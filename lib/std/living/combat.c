/*
 *  /std/living/combat.c
 *
 *  This is a subpart of living.c
 *  All internal combat routines are coded here.
 *
 *  This file is included into living.c
 *
 *  Most of the functionality is moved to the external combat object,
 *  see /std/combat/cbase
 */

#include <comb_mag.h>
#include <files.h>
#include <log.h>
#include <login.h>
#include <macros.h>
#include <money.h>
#include <std.h>
#include <stdproperties.h>
#include <wa_types.h>

static int      time_to_heal;   /* Healing counter */
static mapping  quickness;      /* Objects responsible for quickness */
static float    speed;          /* The current speed modifier */
static int      run_alarm;      /* Alarm used for panic code */
static int      is_whimpy;      /* Automatically flee when low on HP */

static string   whimpy_dir;     /* Direction to wimpy in if needed */

static object   my_leader,      /* Pointer to team leader if exist */
                *my_team,       /* Array of team members if leader */
                combat_extern;  /* The external combat object */

static mixed    leftover_list;  /* The list of leftovers */

#define CEX if (!combat_extern) combat_reload()
varargs public mixed query_leftover(string organ);
public int remove_leftover(string organ);
public void run_away();
public mixed query_enemy(int arg);

/*
 * Function name:   query_combat_file
 * Description:     Gives the name of the file to use for combat.
 * Returns:         The name
 */
public string
query_combat_file()
{
    return COMBAT_FILE;
}

/*
 * Function name:   query_combat_object
 * Description:     Gives the object that is currently acting as external
 *                  combat object
 * Returns:         the combat object
 */
public object
query_combat_object()
{
    return combat_extern;
}

static void
combat_reload()
{
    if (combat_extern)
        return;

    combat_extern = clone_object(query_combat_file());

    /*
     * We can define our own combat object, but it must inherit
     * the original.
     */
    if (function_exists("create", combat_extern) != COMBAT_FILE)
    {
        write("ILLEGAL COMBAT OBJECT: " +
              function_exists("create_object", combat_extern) +
              " should be: " + COMBAT_FILE + "\n");
        destruct();
    }

    combat_extern->cb_link(); /* Link me to the combat object */

    /* 
     * Configure this living object. For humanoids this includes adding
     * the hand attacks and head, torso etc hitlocations.
     */
    combat_extern->cb_configure(); 
}

/*
 * Function name:   combat_reset
 * Description:     Reset the combat functions of the living object.
 */
static nomask void
combat_reset()
{
    my_team = ({});
}

/*
 * Function name: combat_reward
 * Description:   Reward attackers for a kill
 * Arguments:     attacker:   The object that killed me
 *                dam:        The amount of damage in hitpoints made
 *                kill:       True if the attack killed me
 */
public void
combat_reward(object attacker, int dam, int kill)
{
    int exp;
    int align;
    int size;
    int index;
    int average;
    object *team;
    object *team2;

    if (!kill)
    {
#ifdef CB_HIT_REWARD
        attacker->add_exp(dam, 1);
#endif CB_HIT_REWARD
        return;
    }

    /* Change the alignment of the killer. */
    align = attacker->query_alignment();
    attacker->set_alignment(align +
        F_KILL_ADJUST_ALIGN(align, this_object()->query_alignment()));

    /* Let the team share the experience. We use the average team stat to
     * calculate the experience and give a bonus for extra people in the
     * team. Only members of the team present share in the experience.
     */
    team = ({ attacker }) + attacker->query_team_others();
    team = team & all_inventory(environment());
    size = sizeof(team);
    if (size > 1)
    {
        average = 0;
        index = -1;
        while(++index < size)
        {
            average += team[index]->query_average_stat();
        }
        average /= size;
    }
    else
    {
        average = attacker->query_average_stat();
    }

    /* Adjust panic values. Killing the enemy reduces my panic, and that of
     * the members of my team. However, it raises the panic level of the team
     * of the victim.
     */
    attacker->add_panic(-10);
    team->add_panic(-15);
    team2 = (object*)this_object()->query_team_others() & 
        all_inventory(environment());
    team2->add_panic(25);

    /* Calculate the reward based on the average stat of the killer and that
     * of the victim. If you kill something too small, you get no experience.
     */
    exp = F_EXP_ON_KILL(average, this_object()->query_average_stat());
    if (!exp)
    {
        return;
    }

    /* Some NPC's may be worth a little more. Others a little less. */
    if (query_npc())
    {
        exp = this_object()->query_exp_factor() * exp / 100;
    }

    /* Distribute the experience. If you are in a team, then you get a little
     * extra to promote teamplay.
     */
    if (size > 1)
    {
        exp = F_EXP_TEAM_BONUS(size) * exp / 100 / size;
        team->add_exp_combat(exp);
    }
    else
    {
        attacker->add_exp_combat(exp);
    }
}

/*
 * Function name: notify_death
 * Description:   Notify onlookers of my death
 * Arguments:     object killer - the object that killed me
 */
public void
notify_death(object killer)
{
    tell_room(environment(this_object()), 
        QCTNAME(this_object()) + " died.\n", this_object());

    if (!living(killer))
    {
        return;
    }
 
    /*
     * Give specific information about who killed this poor soul.
     */
    tell_object(killer, 
        "You killed " + this_object()->query_the_name(killer) + ".\n");
    tell_room(environment(this_object()),  QCTNAME(killer) + " killed " +
        this_object()->query_objective() + ".\n", ({ this_object(), killer }));
}

/*
 * Function name: notify_pseudo_death
 * Description  : Notify onlookers of my pseudo-death.
 * Arguments    : object killer - the object that pseudo-killed me.
 */
public void
notify_pseudo_death(object killer)
{
    tell_object(killer, "You are victorious over " +
        this_object()->query_the_name(killer) + ".\n");
    tell_object(this_object(), killer->query_The_name(this_object()) +
        " is victorious over you.\n");
    tell_room(environment(this_object()),  QCTNAME(killer) + " is victorious over " +
        QTNAME(this_object()) + ".\n", ({ this_object(), killer }));
}

/*
 * Function name: log_player_death
 * Description  : This function is called when 
 * Arguments    : object killer - the object responsible for our death.
 */
static void
log_player_death(object killer)
{
    string log_msg;
    string extra;

    log_msg = sprintf("%s %-11s (%3d) by ", ctime(time()),
        capitalize(this_object()->query_real_name()),
        this_object()->query_average_stat());

    if (interactive(killer) ||
        IS_PLAYER_OBJECT(killer))
    {
        log_msg += sprintf("%-11s (%3d)",
            capitalize(killer->query_real_name()),
            killer->query_average_stat());
    }
    else
    {
        log_msg += MASTER_OB(killer);
    }
    log_msg += "\n";

    /* Allow the killer to give out extra information about itself. */
    if (strlen(extra = killer->log_player_death_extra_info()))
    {
        log_msg += extra;
    }

#ifdef LOG_PLAYERKILLS
    if (interactive(killer) ||
        IS_PLAYER_OBJECT(killer))
    {
        log_file(LOG_PLAYERKILLS, log_msg, -1);
    }
    else
#endif LOG_PLAYERKILLS
    {
#ifdef LOG_KILLS
        log_file(LOG_KILLS, log_msg, -1);
#endif LOG_KILLS
    }
}

/*
 * Function name:   do_die
 * Description:     Called from enemy combat object when it thinks we died.
 * Arguments:       killer: The enemy that caused our death.
 */
public void
do_die(object killer)
{
    object *sparring;
    object corpse;

    /* Did I die ? */
    if ((query_hp() > 0) ||
        query_wiz_level() ||
        query_ghost())
    {
        return;
    }

    /* Stupid wiz didn't give the objectp to the killer. */
    if (!objectp(killer))
    {
        killer = previous_object();
    }
    /* Bad wiz, calling do_die in someone. */
    if ((MASTER_OB(killer) == WIZ_CMD_NORMAL) ||
        (MASTER_OB(killer) == TRACER_TOOL_SOUL))
    {
        killer = this_interactive();
    }

    /* If the players are sparring, then stop the fight without killing the
     * weaker party.
     */
    sparring = query_prop(LIVE_AO_SPARRING);
    if (IN_ARRAY(killer, sparring))
    {
        killer->stop_fight(this_object());
        this_object()->stop_fight(killer);
        killer->remove_sparring_partner(this_object());
        this_object()->remove_sparring_partner(killer);
        this_object()->notify_pseudo_death(killer);
        return;
    }

    combat_extern->cb_death_occured(killer);

    this_object()->notify_death(killer);

    if (living(killer))
    {
        combat_reward(killer, 0, 1);
    }

    killer->notify_you_killed_me(this_object());

    if (interactive() ||
        IS_PLAYER_OBJECT(this_object()))
    {
        log_player_death(killer);
    }

    /* If there is a coin property, make them into real coins. */
    MONEY_EXPAND(this_object());

    /* Fix the corpse and possibly the ghost */
    if (this_object()->query_prop(LIVE_I_NO_CORPSE))
    {
        move_all_to(environment(this_object()));
    }
    else
    {
        if (!objectp(corpse = (object)this_object()->make_corpse()))
        {
            corpse = clone_object("/std/corpse");
            corpse->set_name(query_name());
            corpse->change_prop(CONT_I_WEIGHT, query_prop(CONT_I_WEIGHT));
            corpse->change_prop(CONT_I_VOLUME, query_prop(CONT_I_VOLUME));
            corpse->add_prop(CORPSE_S_RACE, query_race_name());
            corpse->add_prop(CONT_I_TRANSP, 1);
            corpse->change_prop(CONT_I_MAX_WEIGHT,
                                query_prop(CONT_I_MAX_WEIGHT));
            corpse->change_prop(CONT_I_MAX_VOLUME,
                                query_prop(CONT_I_MAX_VOLUME));
            corpse->set_leftover_list(query_leftover());
        }

        corpse->add_prop(CORPSE_AS_KILLER,
            ({ killer->query_real_name(), killer->query_nonmet_name() }) );
	corpse->add_prop(CORPSE_S_LIVING_FILE, MASTER_OB(this_object()));
        corpse->move(environment(this_object()), 1);
        move_all_to(corpse);
    }
    
    set_ghost(GP_DEAD);

    if (!this_object()->second_life(killer))
    {
        this_object()->remove_object();
    }
}

/*
 * Function name: move_all_to
 * Description  : Move the entire inventory of this_object to dest.
 * Arguments    : object dest - destination of the inventory.
 */
static nomask void
move_all_to(object dest)
{
    object *oblist;
    object room;
    int index;
    int size;
    int ret;

    /* Find the room we are in. */
    room = environment();
    while(objectp(environment(room)))
    {
        room = environment(room);
    }

    oblist = all_inventory(this_object());
    if (oblist && sizeof(oblist) > 0)
    {
        index = -1;
        size = sizeof(oblist);
        while(++index < size)
        {
            /* Remove poisons. They should not bother you in your new body. */
            if (function_exists("create_object", oblist[index]) ==
                POISON_OBJECT)
            {
                oblist[index]->remove_object();
                continue;
            }

            /* Mark in which room we were killed. */
            oblist[index]->add_prop(OBJ_O_LOOTED_IN_ROOM, room);

            if (catch(ret = oblist[index]->move(dest)))
                log_file("DIE_ERR", ctime(time()) + " " +
                    this_object()->query_name() + " (" +
                    file_name(oblist[index]) + ")\n");
            else if (ret)
                oblist[index]->move(environment(this_object()));
        }
    }
}

#if 0
/*
 * Function name: notify_you_killed_me
 * Description  : This routine is called in the killer when it causes the death
 *                of the victim. It can be used for additional processing.
 *
 *                This routin does not actually exist. It is a trick to fool
 *                the document maker.
 * Arguments    : object victim - the victim we caused to perish.
 */
void
notify_you_killed_me(object victim)
{
}
#endif

#if 0
/*
 * Function name: notify_enemy_leaves
 * Description  : This routine is called when an enemy leaves the room, that
 *                is, when someone leaves us. It is not called when we walk
 *                away from someone who is subsequently hunting us.
 *
 *                This routin does not actually exist. It is a trick to fool
 *                the document maker.
 * Arguments    : object enemy - the enemy who left us.
 */
void
notify_enemy_leaves(object enemy)
{
}
#endif

/*************************************************
 *
 * Whimpy routines
 *
 */

/*
 * Function name:   set_whimpy_dir
 * Description:     Sets the favourite direction of the whimpy escape routine
 * Arguments:       str: the direction string
 */
public void
set_whimpy_dir(string str)
{
    whimpy_dir = str;
}

/*
 * Function name: set_whimpy
 * Description  : When a living gets too hurt, it might try to run from
 *                the combat it is engaged in. This will happen if the
 *                percentage of hitpoints left is lower than the whimpy
 *                level, ie: (100 * query_hp() / query_max_hp() < flag)
 * Arguments    : int flag - the whimpy level. Must be in range 0-99.
 */
public void
set_whimpy(int flag)
{
    if ((flag >= 0) && (flag <= 99))
    {
        is_whimpy = flag;
    }
}

/*
 * Function name: query_whimpy
 * Description  : This function returns the whimpy state of this living.
 *                If the percentage of hitpoints the living has left is
 *                lower than the whimpy level, the player will try to
 *                whimp, ie: (100 * query_hp() / query_max_hp() < level).
 * Returns      : int - the whimpy level.
 */
public int
query_whimpy()
{
    return is_whimpy;
}

/*
 * Function name:   query_whimpy_dir
 * Description:     Gives the current favourite whimpy escape direction
 * Returns:         The direction string
 */
public string
query_whimpy_dir()
{
    return whimpy_dir;
}


/*************************************************
 *
 * Team routines
 *
 */

/*
 * Function name: set_leader
 * Description  : Sets this living as a member in a team by assigning a leader.
 *                It will fail if this living is a leader of a team already.
 * Arguments    : object leader - the leader of the team.
 * Returns      : int - 1/0 success/failure.
 */
public int
set_leader(object leader)
{
    if (sizeof(my_team))
        return 0;

    my_leader = leader;
    return 1;
}

/*
 * Function name: query_leader
 * Description  : Find the living who is the leader of our team. If this
 *                returns an objectpointer, it means we are a team member.
 * Returns      : object - the leader of my team, or 0 if we are not lead.
 */
public object
query_leader()
{
    return my_leader;
}

/*
 * Function name: team_join
 * Description  : Make someone a member of our team.
 *                Fails if we have a leader, then we can't lead others.
 * Arguments    : object member - The new member of my team.
 * Returns      : int - 1/0 success/failure.
 */
public int
team_join(object member)
{
    if (my_leader)
        return 0;

    if (!member->set_leader(this_object()))
        return 0;

    if (IN_ARRAY(member, query_team()))
        return 1;

    my_team += ({ member });
    return 1;
}

/*
 * Function name: query_team
 * Description  : Find out the members of our team (if we are the leader).
 * Returns      : object * - the array with team members.
 */
public object *
query_team()
{
    my_team = filter(my_team, objectp);

    return my_team + ({ });
}

/*
 * Function name: team_leave
 * Description  : Someone leaves my team.
 * Arguments    : object member - the member leaving my team.
 */
public void 
team_leave(object member)
{
    member->set_leader(0);
    my_team -= ({ member });
}

/*
 * Function name: query_team_others
 * Description  : Gives all members/leader that we are joined up with,
 *                regardless of whether we are the leader or a member.
 * Returns      : object * - the array with all other members.
 */
public mixed
query_team_others()
{
    object *team;

    if (my_leader)
    {
        team = my_leader->query_team();
        team += ({ my_leader });
        team -= ({ this_object() });
    }
    else
    {
        team = query_team();
    }

    return team;
}


/************************************************************
 * 
 * Redirected functions to the external combat object
 *
 */

/*
 * Function name:   hit_me
 * Description:     Called to make damage on this object. The actually
 *                  made damage is returned and will be used to change
 *                  the score of the aggressor.
 * Arguments:       wcpen         - The wc-penetration
 *                  dt            - damagetype, use MAGIC_DT if ac will not
 *                                  help against this attack.
 *                  attacker      - Object hurting us
 *                  attack_id     - Special id saying what attack hit us. If 
 *                                  you have made a special attack, let the 
 *                                  id be -1
 *                  target_hitloc - Optional argument specifying a hitloc
 *                                  to damage.  If not specified or an
 *                                  invalid hitloc is given, a random
 *                                  one will be used.
 * Returns:         The hitresult as given by the external combat object.
 *                  For details, see 'sman cb_hit_me'.
 */
varargs public mixed
hit_me(int wcpen, int dt, object attacker, int attack_id, int target_hitloc = -1)
{
    mixed hres;
    int wi;

    /*
     * Start nonplayers when attacked
     */
    start_heart();

    CEX;
    hres = (mixed)combat_extern->cb_hit_me(wcpen, dt, attacker, 
                                           attack_id, target_hitloc);

    if (!(wi = query_whimpy()))
        return hres;

    if (((100 * query_hp()) / query_max_hp()) < wi)
    {
        if (run_alarm != 0)
            remove_alarm(run_alarm);
        run_alarm = set_alarm(1.0, 0.0, run_away);
    }

    return hres;
}

/*
 * Function name:   attack_object
 * Description:     Start attacking, the actual attack is done in heart_beat
 * Arguments:       The object to attack
 */
public void
attack_object(object ob)
{
    /* For monsters, start the heart beat. */
    start_heart();

    /* Get the combat started in the combact object. */
    CEX; combat_extern->cb_attack(ob);

    /* Check for sparring, and give the appropriate message if necessary. */
    if (IN_ARRAY(ob, query_prop(LIVE_AO_SPARRING)))
    {
        tell_object(this_object(), "You are sparring with " +
            ob->query_the_name(this_object()) + ".\n");
    }
}

/*
 * Function name:   attacked_by
 * Description:     This routine is called when we are attacked.
 * Arguments:       ob: The attacker
 */
public void
attacked_by(object ob)
{
    /* Get the combat started in the combact object. */
    CEX; combat_extern->cb_attacked_by(ob);

    /* Check for sparring, and give the appropriate message if necessary. */
    if (IN_ARRAY(ob, query_prop(LIVE_AO_SPARRING)))
    {
        tell_object(this_object(), "You are sparring with " +
            ob->query_the_name(this_object()) + ".\n");
    }
}

/*
 * Function name:   query_not_attack_me
 * Description:     The intended victim may force a fail when attacked.
 *                  If fail, the cause must produce explanatory text himself.
 * Arguments:       who: The attacker
 *                  aid: The attack id
 * Returns:         True if the attacker fails hitting us, false otherwise.
 */
public int
query_not_attack_me(object who, int aid)
{
    return 0;
}

/*
 * Function name:   combat_init
 * Description:     Notes when players are introduced into our environment
 *                  Used to attack known enemies on sight.
 */
nomask void
combat_init()
{
    /*
     * Is this_player() in list of known enemies ?
     * Use attacked_by() so that not forced to swap current enemy
     */
    CEX;

    /* Can't attack people you can't see */
    if (!CAN_SEE(this_object(), this_player()))
        return;

    if (IN_ARRAY(this_player(), query_enemy(-1)) &&
        !NPATTACK(this_player()))
    {
        this_object()->reveal_me(1);
        this_player()->reveal_me(1);
        this_object()->attacked_by(this_player());
    }
}



/*
 * Function name: run_away
 * Description:   Runs away from the fight
 */
public void
run_away()
{
    object here;
    int    i, j;

    if (run_alarm != 0)
    {
        remove_alarm(run_alarm);
        run_alarm = 0;
    }
    CEX; combat_extern->cb_run_away(whimpy_dir);
}

/*
 * Function name: stop_fight
 * Description  : Makes this living stop fighting others.
 * Arguments    : mixed elist - the enemy or enemies to stop fighting.
 */
public void
stop_fight(mixed elist)
{
    CEX; combat_extern->cb_stop_fight(elist);
}

/* 
 * Function name: query_enemy 
 * Description  : Gives information of recorded enemies. If you want to know
 *                the currently fought enemy (if any) call query_attack().
 * Arguments    : See "sman cb_query_enemy"
 * Returns      : See "sman cb_query_enemy"
 */
public mixed
query_enemy(int arg)
{
    CEX; return combat_extern->cb_query_enemy(arg);
}
 
/*
 * Function name: update_combat_time
 * Description  : Mark that on this moment a hit was made, either by us or on
 *                us.
 */
public void
update_combat_time()
{
    CEX; combat_extern->cb_update_combat_time();
}

/*
 * Function name: query_combat_time
 * Description  : Find out when the last hit was made, either by us or on us.
 * Returns      : int - the time() value when the last hit was made.
 */
public int
query_combat_time()
{
    CEX; return combat_extern->cb_query_combat_time();
}

/*
 * Function name: query_relaxed_from_combat
 * Description  : If true, the player is relaxed from combat so he can quit or
 *                linkdie.
 * Returns      : int 1/0 - if true, the player can quit/linkdie.
 */
public int
query_relaxed_from_combat()
{
    int tme = query_combat_time();

    /* No combat, means relaxed. */
    if (!tme)
    {
        return 1;
    }

    /* Return TRUE if the time + the relax time after combat is lower than the
     * current time.
     */
    return (F_RELAX_TIME_AFTER_COMBAT(tme) < time());
}

/*
 * Function name: query_attack
 * Description  : Return the object we are currently fighting. This does not
 *                include hunted enemies. Use query_enemy() for that.
 * Returns      : object - the currently attacked object.
 */
public object
query_attack()
{
    CEX; return (object)combat_extern->cb_query_attack();
}

/*******************************************
 *
 * Weapon and Armour routines. 
 *
 * These are merely registration routines for objects used in combat.
 * The actual management of their function is done in the external combat
 * object. The terminology of weapons, armours, wield and wear remain only
 * for backwards compatibility and confusion.
 */

/*
 * Function name: update_weapon
 * Description:   Call this function if the stats or skills of a weapon has
 *                changed.
 * Arguments:     wep - the weapon 
 */
public void
update_weapon(object wep)
{
    CEX; combat_extern->cb_update_weapon(wep);
}

/*
 * Function name: update_armour
 * Description:   Call this function when the ac of an armour has changed
 * Arguments:     arm - the armour
 */
public void
update_armour(object arm)
{
    CEX; combat_extern->cb_update_armour(arm);
}

/*
 * Function name:   adjust_combat_on_move
 * Description:     Called to let movement affect the ongoing fight. This
 *                  is used to print hunting messages.
 * Arguments:       True if leaving else arriving
 */
public void
adjust_combat_on_move(int leave)
{
    CEX; combat_extern->cb_adjust_combat_on_move(leave);
}

/*
 * Function name:   add_panic
 * Description:     Adjust the panic level.
 * Arguments:       dpan: The panic increase/decrease
 */
public void
add_panic(int dpan)
{
    CEX; combat_extern->cb_add_panic(dpan);
}

/*
 * Function name:   query_panic
 * Description:     Give panic value
 * Returns:         The panic value
 */
public int
query_panic()
{
    CEX; return (int)combat_extern->cb_query_panic();
}

/*
 * Function name:   combat_status
 * Description:     Let the combat object describe the combat status
 * Returns:         Description as string
 */
public string
combat_status() 
{
    CEX; return combat_extern->cb_status(); 
}

/*
 * Function name:   combat_data
 * Description:     Let the combat object describe some combat data
 * Returns:         Description as string
 */
public string
combat_data()
{
    CEX; return combat_extern->cb_data();
}

/*
 * Function name:   tell_watcher
 * Description:     Send a string to people who wants to see fights
 * Arguments:       str   - The string to send
 *                  enemy - The enemy we fought
 *                  arr   - Array of objects not to send this message to
 *                          If not used, message sent to all spectators who
 *                          wants to see blood.
 */
varargs void
tell_watcher(string str, mixed enemy, mixed arr)
{
    CEX; combat_extern->tell_watcher(str, enemy, arr);
}

/*
 * Function name:   tell_watcher_miss
 * Description:     Send a string to people who wants to see fights. Since
 *                  there is a miss, exclude those that do not want to see
 *                  the misses.
 * Arguments:       str   - The string to send
 *                  enemy - The enemy we fought
 *                  arr   - Array of objects not to send this message to
 *                          If not used, message sent to all spectators who
 *                          wants to see blood.
 */
varargs void
tell_watcher_miss(string str, object enemy, mixed arr)
{
    CEX; combat_extern->tell_watcher_miss(str, enemy, arr);
}

/*
 * Function name: add_leftover
 * Description:   Add leftovers to the body.
 * Arguments:     string file - The path to the object to leave behind.
 *                string organ - The actual leftover name
 *                int nitems - Number of leftovers of this kind. (-1 = infinite)
 *                string vbfc - VBFC to check.
 *                int hard - Hard remains (left after total decay).
 *                int cut - If this has to be "cut" loose or if "tear" is enough.
 *                int relweight - the weight of the leftover in 1/1000 of the
 *                    weight of the corpse, so setting 50 = 5% corpse weight.
 */
varargs public void
add_leftover(string file, string organ, int nitems, string vbfc,
             int hard, int cut, int relweight)
{
    if (!sizeof(leftover_list))
        leftover_list = ({ });

    remove_leftover(organ);

    leftover_list += ({ ({ file, organ, nitems, vbfc, hard, cut, relweight }) });
}

/*
 * Function name: query_leftover
 * Description:   Return the leftover list. If an organ is specified, that
 *                actual entry is looked for, otherwise, return the entire
 *                list.
 *                The returned list contains the following entries:
 *                ({ objpath, organ, nitems, vbfc, hard })
 * Arguments:     organ - The organ to search for.
 */
varargs public mixed
query_leftover(string organ)
{
    int i;

    if (!sizeof(leftover_list))
        return ({ });

    if (!strlen(organ))
        return leftover_list;

    for (i = 0 ; i < sizeof(leftover_list) ; i++)
        if (leftover_list[i][1] == organ)
            return leftover_list[i];
}

/*
 * Function name: remove_leftover
 * Description:   Remove a leftover entry from a body.
 * Arguments:     organ - Which entry to remove.
 * Returns:       1 - Ok, removed, 0 - Not found.
 */
public int
remove_leftover(string organ)
{
    int index;

    index = sizeof(leftover_list);
    while(--index >= 0)
    {
        if (leftover_list[index][1] == organ)
        {
            leftover_list = exclude_array(leftover_list, index, index);
        }
    }
}

/*
 * Function name: add_attack_delay
 * Description:   Set the LIVE_I_ATTACK_DELAY prop properly.
 *                Use this function if possible instead of altering the prop.
 * Arguments:     secs - How many seconds
 *                type - How it should be added.
 *                       0 - Just add it.
 *                       1 - Superimpose the delay; make it at least 'secs'
 *                           seconds counting from now, not counting from the
 *                           end of an existing delay. In formula:
 *                           new_delay = MAX(old_delay, secs)
 */
public void
add_attack_delay(int secs, int type)
{
    int old, new;

    if (!query_attack())
    {
        return;
    }

    old = new = query_prop(LIVE_I_ATTACK_DELAY);
    if (type)
    {
        if (secs > old)
            new = secs;
    }
    else
        new += secs;

    if (new != old) add_prop(LIVE_I_ATTACK_DELAY, new);
}

/*
 * Function name: add_stun
 * Description:   Stun the living with the LIVE_I_STUNNED prop.
 *                Use this function if possible instead of altering the prop.
 */
public void
add_stun()
{
    add_prop(LIVE_I_STUNNED, query_prop(LIVE_I_STUNNED) + 1);
}

/*
 * Function name: remove_stun
 * Description:   Remove a stun made by add_stun.
 *                Use this function if possible instead of altering the prop.
 */
public void
remove_stun()
{
    int tmp = query_prop(LIVE_I_STUNNED) - 1;
    if (tmp <=  0)
        remove_prop(LIVE_I_STUNNED);
    else
        add_prop(LIVE_I_STUNNED, tmp);
}

/*
 * Function name: add_sparring_partner
 * Description  : This will add a sparring partner to the list of sparring
 *                partners of the living.
 * Arguments    : object partner - the sparring partner to add.
 */
public void
add_sparring_partner(object partner)
{
    object *sparring = query_prop(LIVE_AO_SPARRING);

    /* Can only spar with livings. */
    if (!living(partner))
    {
        return;
    }

    /* Add the partner to the list, or make a new list. */
    if (!sizeof(sparring))
    {
        sparring = ({ partner });
    }
    else
    {
        sparring = filter(sparring, objectp);
        sparring -= ({ partner });
        sparring += ({ partner });
    }
    add_prop(LIVE_AO_SPARRING, sparring);
}

/*
 * Function name: remove_sparring_partner
 * Description  : This will remove a sparring partner from the list of
 *                sparring partners of the living.
 * Arguments    : object partner - the sparring partner to remove.
 */
public void
remove_sparring_partner(object partner)
{
    object *sparring = query_prop(LIVE_AO_SPARRING);

    /* If there is a list, remove the person from it. */
    if (sizeof(sparring))
    {
        sparring = filter(sparring, objectp);
        sparring -= ({ partner });
        if (sizeof(sparring))
        {
            add_prop(LIVE_AO_SPARRING, sparring);
        }
        else
        {
            remove_prop(LIVE_AO_SPARRING);
        }
    }
}

/*
 * Function name: query_sparring_partner
 * Description  : Call this to find out whether a particular living is a
 *                sparring partner of this person.
 * Arguments    : object partner - the partner to check.
 * Returns      : int 1/0 - is a partner / is not a partner.
 */
public int
query_sparring_partner(object partner)
{
    return IN_ARRAY(partner, query_prop(LIVE_AO_SPARRING));
}

#if 0
/*
 * Function name: hook_stop_fighting_offer
 * Description  : When a living makes the offer to stop fighting, this hook
 *                is called in the target. It allows you to code an NPC that
 *                can actually accept the offer, or it can be used in a guild,
 *                for instance, to force the person to accept the offer under
 *                certain circumstances.
 *                All messages have been printed by the time this function is
 *                called, so the reaction may be instant and without alarm. To
 *                let this living accept the offer, make him do:
 *                    command("stop fighting " + OB_NAME(attacker));
 * Arguments    : object attacker - the living making us the offer to stop
 *                    fighting.
 */
public void
hook_stop_fighting_offer(object attacker)
{
    /* This is a pseudo-function. It does not exist. Do not refer to it by
     * making a call to ::hook_stop_fighting_offer(attacker);
     */
}
#endif

/*
 * Function Name: query_speed
 * Description  : Returns the given speed modified by the quickness
 *                of the living. 
 * Arguments    : int / float - the base time to modify.
 * Returns      : float       - the modified speed
 */
public float
query_speed(mixed round_time)
{
    if (!floatp(speed))
        speed = 1.0;

    if (intp(round_time))
        round_time = itof(round_time);
    
    return round_time * speed;
}

/*
 * Function Name: query_speed_map
 * Description  : Returns the entire mapping of quickness items and
 *                their corresponding value.
 * Returns      : mapping - string / value pairs
 */
public mapping
query_speed_map()
{
    if (!mappingp(quickness))
        quickness = ([ ]);
    return quickness;
}

/*
 * Function Name: update_speed
 * Description  : Called automatically when the quickness prop has
 *                been changed in the player. Updates the speed and
 *                informs the combat system that there has been a change.
 */
void
update_speed()
{    
    if (!mappingp(quickness))
    {
        speed = 1.0;
        return;
    }

    speed = 0.0;
    foreach (mixed obj, int value : quickness)
    {
        speed += (1.0 / (F_SPEED_MOD(value) + 0.0001)) - 1.0;
    }

    speed = 1.0 / (1.0 + speed);
    CEX; combat_extern->cb_update_speed();
}

/*
 * Function Name: add_prop_live_i_quickness
 * Description  : Intercepts all modifications to LIVE_I_QUICKNESS
 *                and tracks them based on source object.
 *
 * Returns      : 0 - All changes are allowed.
 */
int
add_prop_live_i_quickness(mixed val)
{
    mixed prev;
    int i;
    string *functions = ({ "add_prop", "remove_prop", "change_prop", 
	  "remove_prop_live_i_quickness", "inc_prop", "dec_prop" });    

    if (!intp(val))
        return 0;
    
    if (!mappingp(quickness))
        quickness = ([ ]);
    
    i = 0;
    while (member_array(calling_function(i), functions) >= 0)
        i--;    
    prev = file_name(calling_object(i));
    
    quickness[prev] += val - query_prop(LIVE_I_QUICKNESS);    

    /* Temporary */
    if (query_interactive(this_object()))
        log_file("quickness", file_name(this_object()) + " gained " +
            (val - query_prop(LIVE_I_QUICKNESS)) + " from " + prev + " [" +
            quickness[prev] + "]\n", 50000);
    
    if (!quickness[prev])
        m_delkey(quickness, prev);
    
    update_speed();
    
    return 0;
}

/*
 * Function Name: remove_prop_live_i_quickness
 * Description  : Catches removal of the LIVE_I_QUICKNESS
 *                prop to keep track of quickness based on source.
 */
int
remove_prop_live_i_quickness()
{
    add_prop_live_i_quickness(0);
}

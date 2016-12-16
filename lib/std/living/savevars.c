/*
 *  /std/living/savevars.c
 *
 *  This file contains all player variables that are saved in the
 *  player save file. The corresponding set- and query functions
 *  can also be found here.
 *
 *  This file is included into /std/living.c
 */

#include <composite.h>
#include <const.h>
#include <filter_funs.h>
#include <formulas.h>
#include <macros.h>
#include <ss_types.h>
#include <stdproperties.h>
#include <std.h>

private string m_in,            /* Messages when entering or leaving a room. */
               m_out,           /* Messages when entering or leaving a room. */
               mm_in,           /* Message when arriving by teleport. */
               mm_out,          /* Message when leaving by teleport. */
               race_name,       /* The name of the race */
               title,           /* Title of the living */
#ifndef NO_ALIGN_TITLE
               al_title,        /* Alignment title of the living */
#endif NO_ALIGN_TITLE
               *cmdsoul_list,   /* The command souls */
               *tool_list;      /* Names of tool souls I want */

private int    hit_points,      /* The hitpoints of this lifeform. */
               mana,            /* The magic points of this lifeform */
               fatigue,         /* How easily this lifeform is fatigued */
               exp_points,      /* Amount of quest experience */
               exp_combat,      /* Amount of exp gained in combat */
               exp_general,     /* Amount of general experience */
               exp_max_total,   /* Maximum amount of exp the player ever had */
               is_ghost,        /* If lifeform is dead */
               alignment,       /* Depends on good or chaotic lifeform */
               gender,          /* G_MALE, G_FEMALE or G_NEUTER (0-2) */
               appearance,      /* What we look like (0-99) */
               opinion,         /* What we think of others appearance */
               intoxicated,     /* How drunk are we? */
               stuffed,         /* Are we fed or not */
               soaked,          /* How soaked are we ? */
               *learn_pref,     /* Prefered % to learn / stat */
               *acc_exp,        /* Accumulated exp / stat */
               scar;            /* The marks of death */
private mapping
               skillmap;        /* Our skills in a mapping */
static mapping
               skill_extra_map; /* Extra skills added by items for example */

/*
 * Prototypes
 */

int query_stuffed();
int query_intoxicated();
int query_stat(int stat);
int query_skill(int skill);
int query_stat_pref_total();
int query_guild_pref_total();
void heal_hp(int hp);
void add_mana(int sp);
int query_fatigue();
int query_mana();
nomask public int remove_cmdsoul(string soul);
nomask public int remove_toolsoul(string soul);

/*
 *  These vars keep track of the last time the appropriate var was updated.
 */
static private int intoxicated_time;
static private int stuffed_time;
static private int soaked_time;
static private int hp_time;
static private int mana_time;
static private int fatigue_time;
static private int last_intox;
static private int last_con;
static private int last_stuffed;

static nomask void
savevars_delay_reset()
{
    last_stuffed = query_stuffed();
    last_intox = query_intoxicated();
    last_con = query_stat(SS_CON);
}

/*
 * Function name:   save_vars_reset
 * Description:     Resets some variables which are used to keep track
 *                  of how variables change with time.
 */
static nomask void
save_vars_reset()
{
    int t = time();

    intoxicated_time  = t;
    stuffed_time      = t;
    soaked_time       = t;
    hp_time           = t;
    mana_time         = t;
    fatigue_time      = t;

    set_alarm(1.0, 0.0, savevars_delay_reset);
}

/*
 * Function name: skill_extra_map_reset
 * Description  : Reset the skill_extra_map at initialization.
 */
private void
skill_extra_map_reset()
{
    skill_extra_map = ([ ]);
}

/*
 * Function name: set_m_in
 * Description  : Set the normal entrance message of this living. This text
 *                should fit into the phrase "<name> <m-in> <from direction>."
 *                so there should not be a closing period to this message.
 * Arguments    : string m - the message string.
 * Example      : "waddles into the room"
 */
public void
set_m_in(string m)
{
    m_in = implode(explode(m, "   "), " ");
}

/*
 * Function name: query_m_in
 * Description  : Gives the normal entrance message of this living.
 * Returns      : string - the message string.
 */
public string
query_m_in()
{
    return m_in;
}

/*
 * Function name: set_m_out
 * Description  : Set the normal exit message of this living. This text should
 *                fit into the sentence "<name> <m-out> <to direction>." so
 *                there should not be a closing period to this message.
 * Arguments    : string m - the message string.
 * Example      : "waddles"
 */
public void
set_m_out(string m)
{
    m_out = implode(explode(m, "   "), " ");
}

/*
 * Function name: query_m_out
 * Description  : Gives the normal exit message of this living.
 * Returns      : string - the message string.
 */
public string
query_m_out()
{
    return m_out;
}

/*
 * Function name: set_mm_in
 * Description  : Set the magical (teleportation) entrance message of this
 *                living. This text should fit into "<name> <mm-in>." so there
 *                should be a closing period or exclamation mark to this
 *                message.
 * Arguments    : string m - the message string.
 * Example      : "falls out of the sky with his mouth full of spam."
 */
public void
set_mm_in(string m)
{
    mm_in = implode(explode(m, "  "), " ");
}

/*
 * Function name: query_mm_in
 * Description  : Gives the magical (teleportation) entrance message of this
 *                living.
 * Returns      : string - the message string.
 */
public string
query_mm_in()
{
    return mm_in;
}

/*
 * Function name: set_mm_out
 * Description  : Set the magical (teleportation) exit message of this living.
 *                This text should fit into the sentence "<name> <mm-out>." so
 *                there should be a closing period or exclamation mark to this
 *                message.
 * Arguments    : string m - the message string.
 * Example      : "disappears into a local black hole."
 */
public void
set_mm_out(string m)
{
    mm_out = implode(explode(m, "  "), " ");
}

/*
 * Function name: query_mm_out
 * Description  : Gives the magical (teleportation) exit message of this
 *                living.
 * Returns      : string - the message string
 */
public string
query_mm_out()
{
    return mm_out;
}

/*
 * Function name: query_wiz_level
 * Description  : Gives the wizard level of the living. This function is
 *                kept here since there are various calls to it from the
 *                living object. The real function is moved to the player
 *                object.
 *
 *                WARNING! This function is NOT nomasked! People can
 *                redefine this and make themselves appear to be a wizard.
 *                In case you need to be certain of the level of the person,
 *                call the following function.
 *
 *                SECURITY->query_wiz_rank(string name);
 *
 * Returns      : int - always 0.
 */
public int
query_wiz_level()
{
    return 0;
}

/*
 * Function name: set_race_name
 * Description  : Sets the race name of this living. The race name will also
 *                be set as add_name too.
 * Arguments    : string str - the race string.
 */
public void
set_race_name(string str)
{
    if (id(race_name))
    {
        remove_name(race_name);
    }

    race_name = str;
    add_name(race_name);
}

/*
 * Function name: query_race_name
 * Description  : Gives the race (species) name of this living. This may
 *                be set with set_race_name(). For players, the value
 *                returned for query_race() will always return one of the
 *                default races defined by the mud. For NPC's it is the same
 *                as query_race().
 * Returns      : string - the race name.
 */
public string
query_race_name()
{
    return race_name;
}

/*
 * Function name: query_race
 * Description  : If you define different player objects for different
 *                races you should mask this function in those objects to
 *                always return the true race of the living even though
 *                query_race_name gives the current race of the living.
 *                You should nomask the redefinition of this function.
 * Returns      : string - the race name.
 */
public string
query_race()
{
    return race_name;
}

/*
 * Function name: reset_race_name
 * Description  : Called in order to re-set the race name of a player to
 *                the base race. This function is irrelevant for NPC's.
 */
public void
reset_race_name()
{
    set_race_name(query_race());
}

/*
 * Function name:   query_npc
 * Description:     Checks whether the living is a non-player character
 * Returns:         True if non-player character
 */
public int
query_npc()
{
    return 1;
}

/*
 * Function name:   set_title
 * Description:     Sets the title of a living to something else.
 * Arguments:       t: The new title string
 */
public void
set_title(string t)
{
#ifdef LOG_TITLE
    if (this_player() != this_object() && interactive(this_object()))
        SECURITY->log_syslog(LOG_TITLE, ctime(time()) + " " +
            query_real_name() + " new title " + t + " by " +
            this_player()->query_real_name() + "\n");
#endif
    title = t;
}

#ifndef NO_ALIGN_TITLE
/*
 * Function name:   set_al_title
 * Description:     Sets the alignment title of a living to something else
 * Arguments:       t: The new alignment title string
 */
public void
set_al_title(string t)
{
#ifdef LOG_AL_TITLE
    if (this_player() != this_object() && interactive(this_object()))
        SECURITY->log_syslog(LOG_AL_TITLE, ctime(time()) + " " +
            query_real_name() + " new title " + t + " by " +
            this_player()->query_real_name() + "\n");
#endif
    al_title = t;
}
#endif

/*
 * Function name:   query_title
 * Description:     Gives the title of a living.
 * Returns:         The title string
 */
public nomask string
query_title()
{
    string dom, name, *titles = ({ });
    int    family_name = 0;

    if (query_wiz_level())
    {
        if (!strlen(title))
            title = "";

        return title;
    }

    /* This MUST be with this_object()-> or it will not work for we are
     * accessing the function in the shadows of the player!
     */
    if (strlen(name = this_object()->query_guild_title_race()))
    {
        titles += ({ name });

        /* If the player is in a racial guild that gives him a family name,
         * we do not add the article before the race-title, but we add it
         * after the title.
         */
        family_name = this_object()->query_guild_family_name();
    }

    if (strlen(name = this_object()->query_guild_title_occ()))
        titles += ({ name });

    if (strlen(name = this_object()->query_guild_title_lay()))
        titles += ({ name });

    if (strlen(name = this_object()->query_guild_title_craft()))
        titles += ({ name });

    /* An NPC may have guild-titles and set titles.
     */
    if (query_npc())
    {
        if (!strlen(title))
            title = "";

        if (!sizeof(titles))
            return title;

        if (strlen(title))
            titles += ({ title });
    }

    /* A mortal player cannot have a title set by a wizard! */
    if (!sizeof(titles))
        return "";

    /* If the player has a family name, we add the article after the family
     * name, else we add it before the possible racial title.
     */
    if (family_name)
    {
        if (sizeof(titles) == 1)
            return titles[0];
        else
            return titles[0] + ", " + LD_THE + " "
		+ COMPOSITE_WORDS(titles[1..]);
    }
    else
        return LD_THE + " " + COMPOSITE_WORDS(titles);
}

#ifndef NO_ALIGN_TITLE
/*
 * Function name:   query_al_title
 * Description:     Gives the alignment title of a living
 * Returns:         The alignment title string
 */
public string
query_al_title()
{
    return al_title;
}
#endif

/*
 * Function name: calculate_hp
 * Description  : Compute the current number of hitpoints of the living,
 *                taking into account the constitution and intoxication.
 */
void
calculate_hp()
{
    int n, con, intox;
    int tmpcon, tmpintox;

    n = (time() - hp_time) / F_INTERVAL_BETWEEN_HP_HEALING;
    if (n > 0)
    {
        con = query_stat(SS_CON);
        intox = query_intoxicated();
        tmpcon = (con + last_con) / 2;
        tmpintox = (intox + last_intox) / 2;

        hp_time += n * F_INTERVAL_BETWEEN_HP_HEALING;
        heal_hp(n * F_HEAL_FORMULA(tmpcon, tmpintox));
        last_con = con;
        last_intox = intox;
    }
}

/*
 * Function name:   query_hp
 * Description:     Gives the number of hitpoint left for the living
 * Returns:         The number
 */
public int
query_hp()
{
    this_object()->calculate_hp();
    return hit_points;
}

/*
 * Function name:   query_max_hp
 * Description:     Calculates the maximum number of hitpoints that the
 *                  living can achieve.
 * Returns:         The maximum
 */
public int
query_max_hp()
{
    return F_MAX_HP(query_stat(SS_CON));
}

/*
 * Function name:   set_hp
 * Description:     Sets the number of hitpoints of a living. The number
 *                  can not exceed the maximum calculated by query_max_hp.
 * Arguments:       hp: The new number of hitpoints
 */
void
set_hp(int hp)
{
    this_object()->calculate_hp();
    hit_points = 0;
    heal_hp(hp);
}

/*
 * Function name:   heal_hp
 * Description:     Increase the number of hitpoints with a few.
 * Arguments:       hp: The difference
 */
void
heal_hp(int hp)
{
    string text;
    object o;

    this_object()->calculate_hp();

#ifdef LOG_REDUCE_HP
    o = previous_object();
    if (!query_npc() && (hp < 0) && (-hp >= hit_points))
    {
        text = sprintf("%s %s %d->%d by ", ctime(time()), query_name(),
	    hit_points, hit_points + hp);

	text += (this_interactive() ? this_interactive()->query_name() : "?");

        if (o)
            text += " " + file_name(o) + ", " + o->short() +
		" (" + getuid(o) + ")\n";
        else
            text += " ??\n";

	/* Don't log CHUMLOCK heal-hp. */
	if (objectp(o) && (MASTER_OB(o) == CHUMLOCK_OBJECT))
	    text = "";

	if (strlen(text))
	    SECURITY->log_syslog(LOG_REDUCE_HP, text);
    }
#endif

    hit_points = min(max(hit_points + hp, 0), query_max_hp());
}

/*
 * Function name:   reduce_hit_point
 * Description:     Reduce the number of hitpoints with a few.
 * Arguments:       dam: The number of damage hitpoints.
 */
public int
reduce_hit_point(int dam)
{
    heal_hp(-dam);
}

/*
 * Function name:   query_max_mana
 * Description:     Calculates that maximum of mana points that a living
 *                  can get.
 * Returns:         The maximum.
 */
public int
query_max_mana()
{
    return query_stat(SS_INT) * 10;
}

/*
 * Function name:   set_mana
 * Description:     Set the number of mana points that a player has. Mana
 *                  points are more commonly known as spellpoints. The
 *                  mana points can not bet set to more than the amount
 *                  that is calculated by query_max_mana.
 * Arguments:       sp: The new amount of mana points.
 */
void
set_mana(int sp)
{
    query_mana(); // heal mana *before* setting the value
    mana = 0;
    add_mana(sp);
}

/*
 * Function name:   add_mana
 * Description:     Add a certain amount of mana points
 * Arguments:       sp: The number of mana points to change.
 */
void
add_mana(int sp)
{
    query_mana();
    mana = min(max(mana + sp, 0), query_max_mana());
}

/*
 * Function name:   query_mana
 * Description:     Gives the number of mana points that the living has
 * Returns:         The number of mana points.
 */
public int
query_mana()
{
    int n;
    int wis;
    int sc;
    int pintox;

    n = (time() - mana_time) / F_INTERVAL_BETWEEN_MANA_HEALING;
    if (n > 0)
    {
        wis = query_stat(SS_WIS);
        pintox = query_intoxicated();
        pintox = ((((pintox < 0) ? 0 : pintox) * 100) / query_prop(LIVE_I_MAX_INTOX));
        sc = query_skill(SS_SPELLCRAFT);

        mana_time += n * F_INTERVAL_BETWEEN_MANA_HEALING;
        add_mana(n * F_MANA_HEAL_FORMULA(sc, pintox, wis));
    }

    return mana;
}

/*
 * Function name:   query_max_fatigue
 * Description:     Calculates the maximum number of fatigue points that
 *                  the living can have.
 * Returns:         The maximum.
 */
public int
query_max_fatigue()
{
    return query_stat(SS_CON) + 50;
}

/*
 * Function name:   add_fatigue
 * Description:     Add an amount of fatigue points to the current amount
 *                  of the living. Observe, negative argument makes a player
 *                  more tired.
 * Arguments:       f: the amount of change
 */
public void
add_fatigue(int f)
{
    query_fatigue();

    fatigue += f;
    if (fatigue < 0)
        fatigue = 0;
    if (fatigue > query_max_fatigue())
        fatigue = query_max_fatigue();
}

/*
 * Function name: calculate_fatigue
 * Description  : Compute the present fatigue value of the player, taking
 *                the stuffed value into account.
 */
void
calculate_fatigue()
{
    int n, stuffed, tmpstuffed;

    n = (time() - fatigue_time) / F_INTERVAL_BETWEEN_FATIGUE_HEALING;
    if (n > 0)
    {
        stuffed = query_stuffed();
        tmpstuffed = (stuffed + last_stuffed) / 2;
        fatigue_time += n * F_INTERVAL_BETWEEN_FATIGUE_HEALING;
        add_fatigue(n *
            F_FATIGUE_FORMULA(tmpstuffed, query_prop(LIVE_I_MAX_EAT)));
        last_stuffed = stuffed;
    }

}

/*
 * Function name:   set_fatigue
 * Description:     Set the fatigue points of the living to a certain amount.
 * Arguments:       f: The amount to set.
 */
public void
set_fatigue(int f)
{
    query_fatigue(); // heal fatigue *before* setting the value
    fatigue = 0;
    add_fatigue(f);
}

/*
 * Function name:   query_fatigue
 * Description:     Gives the amount of fatigue points of a living
 * Returns:         The number of fatigue points
 */
public int
query_fatigue()
{
    this_object()->calculate_fatigue();
    return fatigue;
}

/*
 * Function name: refresh_living()
 * Description  : This function is called to give the living full mana,
 *                full hitpoints and full fatigue.
 *                NOTE that this function can only be used for NPC's.
 */
void
refresh_living()
{
    if (!(this_object()->query_npc()))
        return;

    heal_hp(query_max_hp());
    add_mana(query_max_mana());
    add_fatigue(query_max_fatigue());
}

/*
 * Function name: set_acc_exp
 * Description  : Set the accumulated experience for each of the stats.
 *                WARNING: Since this is an internal call, we do not check
 *                whether the argument 'stat' points to a valid stat. You
 *                must make sure that the internal consistency of the mudlib
 *                is not compromised.
 * Arguments    : int stat - The stat to set.
 *                int val  - The amount of experience to set the stat to.
 */
static void
set_acc_exp(int stat, int val)
{
    if (val < 1)
        val = 1;

    acc_exp[stat] = val;
}

/*
 * Function name:   query_acc_exp
 * Description:     Get the accumulated experience points for a given stat.
 * Arguments:       stat: The stat to check
 * Returns:         The amount of experience belonging to the stat.
 */
public int
query_acc_exp(int stat)
{
    if ((stat < 0) ||
        (stat >= SS_NO_STATS))
        return -1;

    return acc_exp[stat];
}

/*
 * Function name: query_exp
 * Description  : Gives the total amount of experience of the living,
 *                i.e. quest, combat and general experience together.
 * Returns      : int - the experience points.
 */
public int
query_exp()
{
    return (exp_points + exp_combat + exp_general);
}

/*
 * Function name: query_max_exp
 * Description  : Gives the total amount of experience the living ever had,
 *                i.e. before death took any.
 * Returns      : int - the maximum experience points.
 */
public int
query_max_exp()
{
    return exp_max_total;
}

/*
 * Function name: update_max_exp
 * Description  : Remember the maximum amount of experience the living ever
 *                had, i.e. before death took any.
 */
public void
update_max_exp()
{
    exp_max_total = max(query_exp(), exp_max_total);

#if 0
    int new_max_exp = F_DEATH_MAX_EXP_PLATFORM(exp_max_total);

    /* When a player dies, he is helped to regain his former self. However,
     * when he dies a second time before he is fully recovered, we only help
     * him to a lower platform. Otherwise help him to regain his former self.
     * This is also true if there is no platform yet. */
    if (query_exp() < new_max_exp)
    {
        exp_max_total = new_max_exp;
    }
    else
    {
        exp_max_total = query_exp();
    }
#endif
}

/*
 * Function name: set_exp_combat
 * Description  : Set the amount of combat experience the living has.
 * Arguments    : int exp - the amount of combat experience.
 */
static void
set_exp_combat(int exp)
{
    exp_combat = exp;
}

/*
 * Function name: query_exp_combat
 * Description  : Gives the amount of combat experience the living has.
 * Returns      : int - the combat experience points
 */
public int
query_exp_combat()
{
    return exp_combat;
}

/*
 * Function name: query_brute_factor
 * Description  : Calculates the brutality factor, that is, the factor between
 *                the combat+general experience and the total experience. The
 *                best possible brute factor is 0 and the worst is 1. This is
 *                the fraction of the experience that is not awarded to the
 *                player when he gains new combat/general experience.
 * Arguments    : int base - if true, do not apply the death relaxation.
 * Returns      : float - the brute factor.
 */
varargs public float
query_brute_factor(int base = 0)
{
    float brute;
    int min_exp, current_exp;

    current_exp = query_exp();
    brute = itof(exp_combat + exp_general) / itof(current_exp + 1);

    /* When recovering from death, we can limit the brute penalty. Use
     * linear extrapolation to find the actual brute fraction within the
     * experience range where brute lowering is applied.
     *
     * Read carefully:
     *   Brute is a fraction on the exp gained, but here we scaling the
     *   the brute itself; a fraction on the fraction of exp, if you will.
     *
     * The following scaling takes place:
     * - current > maximum: full application of brute.
     * - current < minimum: brute scaled with minumum relative brute.
     * - else: brute scaled to [ minimum relative brute - 1.0 ]
     */
    if (!base && (current_exp < query_max_exp()))
    {
        min_exp = F_DEATH_MIN_EXP_PLATFORM(query_max_exp());
        if (current_exp < min_exp)
        {
	    brute = brute * F_DEATH_MIN_RELATIVE_BRUTE;
        }
        else
        {
	    brute = brute * (F_DEATH_MIN_RELATIVE_BRUTE +
	       (F_DEATH_RELATIVE_BRUTE_RANGE * (itof(current_exp - min_exp) / itof(query_max_exp() - min_exp))));
	}
    }

    /* For small players, we can limit the brute factor. */
#ifdef MAX_EXP_RED_FRIENDLY
    if ((current_exp < MAX_EXP_RED_FRIENDLY) && (brute > MAX_COMB_EXP_RED))
    {
        brute = MAX_COMB_EXP_RED;
    }
#endif

    return brute;
}

/*
 * Function name: add_exp_combat
 * Description  : This function should be called to add combat experience
 *                to the living. To remove combat experience, add a negative
 *                amount.
 * Arguments    : int exp - the amount of general experience to add.
 */
public void
add_exp_combat(int exp)
{
    float fact;
    int   taxed;

    /* Positive combat experience. */
    if (exp > 0)
    {
	/* Modify the added experience with the brute factor. */
        exp -= ftoi(itof(exp) * query_brute_factor());

        /* Add the experience to the total. */
        exp_combat += exp;

        /* Deduct the tax. */
        taxed = query_guild_pref_total();
        if (taxed > 0)
            exp_combat -= ((exp * taxed) / 100);
    }
    /* Negative combat experience. */
    else
    {
        /* Don't remove more than the player has. */
        if (exp < -exp_combat)
            exp = 1 - exp_combat;

        /* Deduct the experience from the total. */
        update_max_exp();
        exp_combat += exp;
    }

    /* Report the experience added and distribute over the stats. */
    SECURITY->bookkeep_exp("combat", exp);
    update_acc_exp(exp);
    /* Give player growth message. */
    check_last_stats();
}

/*
 * Function name: set_exp_general
 * Description  : Set the amount of general experience the living has.
 * Arguments    : int exp - the amount of general experience.
 */
static void
set_exp_general(int exp)
{
    exp_general = exp;
}

/*
 * Function name: query_exp_general
 * Description  : Gives the amount of general experience the living has.
 * Returns      : int - the general experience points.
 */
public int
query_exp_general()
{
    return exp_general;
}

/*
 * Function name: add_exp_general
 * Description  : This function should be called to add general experience
 *                to the living. To remove general experience, add a negative
 *                amount.
 * Arguments    : int exp - the amount of general experience to add.
 */
public void
add_exp_general(int exp)
{
    int taxed;

    /* Positive general experience. */
    if (exp > 0)
    {
	/* Modify the added experience with the brute factor. */
        exp -= ftoi(itof(exp) * query_brute_factor());

        /* Add the experience to the total. */
        exp_general += exp;

        /* Deduct the tax. */
        taxed = query_guild_pref_total();
        if (taxed > 0)
            exp_general -= ((exp * taxed) / 100);
    }
    /* Negative general experience. */
    else
    {
        /* Don't remove more than the player has. */
        if (exp < -exp_general)
            exp = 1 - exp_general;

        /* Deduct the experience from the total. */
        update_max_exp();
        exp_general += exp;
    }

    /* Report the experience added and distribute over the stats. */
    SECURITY->bookkeep_exp("general", exp);
    update_acc_exp(exp);
    /* Give player growth message. */
    check_last_stats();
}

/*
 * Function name: set_exp_quest
 * Description  : Set the amount of quest experience the living has.
 * Arguments    : int exp - the amount of quest experience.
 */
static void
set_exp_quest(int exp)
{
    exp_points = exp;
}

/*
 * Function name: query_exp_quest
 * Description  : Gives the amount of quest experience the living has.
 * Returns      : int - the quest experience points.
 */
public int
query_exp_quest()
{
    return exp_points;
}

/*
 * Function name: add_exp_quest
 * Description  : This function should be called to add quest experience
 *                to the living. Removing quest experience is not possible.
 * Arguments    : int exp - the amount of quest experience to add.
 */
public void
add_exp_quest(int exp)
{
    /* Negative points experience is impossible. */
    if (exp <= 0)
        return;

    /* Report the experience added before applying the quest-factor. */
    SECURITY->bookkeep_exp("quest", exp);

    /* Positive quest experience. Take the quest factor into account and then
     * add the experience to the total.
     */
    exp = (QUEST_FACTOR * exp) / 10;
    exp_points += exp;

    /* Distribute the experience over the stats using the tax-free method. */
    update_acc_exp(exp, 1);
    /* Give player growth message. */
    check_last_stats();
}

/*
 * Function name: add_exp
 * Description  : OBSOLETE FUNCTION
 *                This function adds an amount of experience to the living. It
 *                should not be used anymore. Instead, use add_exp_combat() to
 *                add combat experience and add_exp_quest() to add quest
 *                experience!
 * Arguments    : int exp    - the amount of experience to be added.
 *                int battle - true if the experience was gained in battle, else
 *                             add quest experience.
 */
public void
add_exp(int exp, int battle)
{
    if (battle)
        this_object()->add_exp_combat(exp);
    else
        this_object()->add_exp_quest(exp);
}

/*
 * Function name: modify_exp
 * Description  : Called from the wizard command soul for the express purpose
 *                of altering the total experience of a player.
 * Arguments    : string type   - the type of experience to adjust.
 *                int amount    - the amount of change, positive or negative.
 *                string reason - the reason for changing the experience.
 */
public nomask void
modify_exp(string type, int amount, string reason)
{
    int old;
    int new;

    /* Access failure; may only be called from the 'normal' soul. */
    if (file_name(previous_object()) != WIZ_CMD_NORMAL)
    {
        write("Invalid call.\n");
        return;
    }

    /* Adjust the appropriate type of experience. */
    switch(type)
    {
    case "quest":
        old = exp_points;
        new = max(0, (old + amount));
        exp_points = new;
        break;

    case "combat":
        old = exp_combat;
        new = max(0, (old + amount));
        exp_combat = new;
        break;

    case "general":
        old = exp_general;
        new = max(0, (old + amount));
        exp_general = new;
        break;

    default:
        write("Invalid experience type.\n");
        return;
    }

    /* Update the stats. Then log the change. */
    update_acc_exp(new - old);

    SECURITY->log_syslog("MODIFY_EXP",
        sprintf("%s %-11s by %-11s %-1s %8i (%8d) %8i: %s\n", ctime(time()),
        capitalize(query_real_name()), capitalize(geteuid(this_interactive())),
        capitalize(type[0..0]), old, (new - old), new, reason));
}

/*
 * Function name: modify_stat
 * Description  : Called from the wizard command soul for the express purpose
 *                of altering the a single stat of a player.
 * Arguments    : string type   - the type of experience to adjust.
 *                int stat      - the number of the valid stat to alter.
 *                int amount    - the new stat value, must be > 0.
 *                string reason - the reason for changing the stat.
 */
public nomask void
modify_stat(string type, int stat, int amount, string reason)
{
    int old;
    int delta;

    /* Access failure; may only be called from the 'normal' soul. */
    if (file_name(previous_object()) != WIZ_CMD_NORMAL)
    {
        write("Invalid call.\n");
        return;
    }

    /* Calculate the change in experience. */
    delta = stat_to_exp(amount) + 1 - query_acc_exp(stat);

    /* Adjust the appropriate type of experience (for real stats only). */
    if (stat < SS_NO_EXP_STATS)
    {
        switch(type)
        {
        case "quest":
            if (delta < -exp_points)
            {
                write("Not enough quest experience alone to go down to a stat " +
                    "value of " + amount + ". Only partially adjusted.\n");
                delta = 1 - exp_points;
            }
            exp_points += delta;
            break;

        case "combat":
            if (delta < -exp_combat)
            {
                write("Not enough combat experience alone to go down to a stat " +
                    "value of " + amount + ". Only partially adjusted.\n");
                delta = 1 - exp_combat;
            }
            exp_combat += delta;
            break;

        case "general":
            if (delta < -exp_general)
            {
                write("Not enough general experience alone to go down to a " +
                    "stat value of " + amount + ". Only partially adjusted.\n");
                delta = 1 - exp_general;
            }
            exp_general += delta;
            break;

        default:
            write("Invalid experience type.\n");
            return;
        }
    }

    /* Update and recalculate the stat. Then log the change. */
    old = query_base_stat(stat);
    set_acc_exp(stat, (query_acc_exp(stat) + delta));
    update_stat(stat);

    SECURITY->log_syslog("MODIFY_STAT",
        sprintf("%s %-11s by %-11s %-1s %-3s %3i -> %3i: %s\n",
        ctime(time()), capitalize(query_real_name()),
        capitalize(geteuid(this_interactive())), capitalize(type[0..0]),
        SS_STAT_DESC[stat][0..2], old, query_stat(stat), reason));
}

/*
 * Function name:   set_ghost
 * Description:     Change the living into a ghost or change the ghost-status
 *                  of a player.
 * Arguments:       flag: A flag to recognise the ghost-status. If flag is 0,
 *                        make the ghost a living again.
 */
set_ghost(int flag)
{
    is_ghost = flag;

    if (flag)
    {
        set_m_in(F_GHOST_MSGIN);
        set_m_out(F_GHOST_MSGOUT);

        add_name(LD_GHOST);
    }
    else
    {
        set_m_in(F_ALIVE_MSGIN);
        set_m_out(F_ALIVE_MSGOUT);

        remove_name(LD_GHOST);
    }
}

/*
 * Function name:   query_ghost
 * Description:     Return the ghost-status of a living.
 * Returns:         0 if the living is not a ghost, the status otherwise.
 */
public int
query_ghost()
{
    return is_ghost;
}

/*
 * Function name:   set_invis
 * Description:     Change the visibility of the living
 * Arguments:       flag: If true turn the living invisible, else make the
 *                        living visible again.
 */
public void
set_invis(int flag)
{
    if (!flag)
        add_prop(OBJ_I_INVIS, 0);
    else if (query_wiz_level())
        add_prop(OBJ_I_INVIS, 100);
    else
        add_prop(OBJ_I_INVIS, flag);
}

/*
 * Function name:   query_invis
 * Description:     Gives back the current visibility of the living
 * Returns:         True if invisible
 */
public int
query_invis()
{
    return this_object()->query_prop(OBJ_I_INVIS);
}

/*
 * Function  name: slow_remove_prop_obj_i_invis
 * Description   : Called to make the livings in the room attack a person that
 *                 just became visible.
 */
public void
slow_remove_prop_obj_i_invis()
{
    object *list;
    object tp;

    list = FILTER_LIVE(all_inventory(environment()) - ({ this_object() }) );

    tp = this_player();
    set_this_player(this_object());

    list->combat_init();
    list->init_attack();

    set_this_player(tp);
}

/*
 * Function name: remove_prop_obj_i_invis
 * Description  : When a player is no longer invisible, make aggressive
 *                monsters attack using a tiny alarm.
 * Returns      : int 0 - always.
 */
public int
remove_prop_obj_i_invis()
{
    set_alarm(0.0, 0.0, slow_remove_prop_obj_i_invis);

    return 0;
}

/*
 * Function name: set_alignment
 * Description  : Set the amount of alignment points of the living. There is
 *                a maximum alignment a player can get. There is a Dutch
 *                proverb about trying to be more Roman-Catholic than the
 *                pope himself. We don't need that.
 * Arguments    : int a - the new alignment.
 */
public void
set_alignment(int a)
{
    if (ABS(a) > F_MAX_ABS_ALIGNMENT)
        a = ((a > 0) ? F_MAX_ABS_ALIGNMENT : -F_MAX_ABS_ALIGNMENT);

    alignment = a;

#ifndef NO_ALIGN_TITLE
    if (!query_wiz_level())
        al_title = query_align_text();
#endif NO_ALIGN_TITLE
}

/*
 * Function name:   query_alignment
 * Description:     Gives the current amount of alignment points of a living
 * Returns:         The amount.
 */
public int
query_alignment()
{
    return alignment;
}

/*
 * Function name: adjust_alignment
 * Description  : When a player has solved a quest, his alignment may be
 *                adjusted if the quest is considered good or evil. This
 *                may only be done when the player receives experience and
 *                the quest bit is subsequently being set. When a quest is
 *                considered solvable for all players in the game, ie both
 *                'good' and 'evil' players, no alignment should be given
 *                out.
 * Arguments    : int align - the alignment of the quest. this should be
 *                            a value in the range -1000 .. 1000 and acts
 *                            the same as alignment in combat, though in
 *                            this case, 'good' players should naturally
 *                            receive positive alignment (ie solve good
 *                            quests).
 */
public void
adjust_alignment(int align)
{
    if (ABS(align) > F_MAX_ABS_ALIGNMENT)
        align = ((align > 0) ? F_MAX_ABS_ALIGNMENT : -F_MAX_ABS_ALIGNMENT);

    set_alignment(alignment + F_QUEST_ADJUST_ALIGN(alignment, align));
}

/*
 * Function name:   set_gender
 * Description:     Set the gender code (G_MALE, G_FEMALE or G_NEUTER)
 * Arguments:       g: The gender code
 */
public void
set_gender(int g)
{
    if (g == G_MALE || g == G_FEMALE || g == G_NEUTER)
    {
        gender = g;
    }
}

/*
 * Function name:   query_gender
 * Description:     Returns the gender code of the living.
 * Returns:         The code. (0 - male, 1 - female, 2 - netrum)
 */
public int
query_gender()
{
    return gender;
}

/*
 * Function name:   set_appearance
 * Description:     Set the appearance variable
 * Arguments:       a: The new value
 */
public void
set_appearance(int a)
{
    if (a < 0 || a > 99)
        a = random(99) + 1;
    appearance = a;
}

/*
 * Function name:   query_appearance
 * Description:     Gives the current value of the appearance variable.
 * Returns:         The appearance value
 */
public int
query_appearance()
{
    if (!appearance)
        set_appearance(0);
    return appearance;
}

/*
 * Function name:   set_opinion
 * Description:     Set the value of the opinion variable
 * Arguments:       o: The new value.
 */
public void
set_opinion(int o)
{
    if (o < 0 || o > 100)
        o = 0;
    opinion = o;
}

/*
 * Function name:   query_opinion
 * Description:     Gives the current value of the opinion variable
 * Returns:         The opinion value
 */
public int
query_opinion()
{
    return opinion;
}

/*
 * Function name:   query_intoxicated
 * Description:     Gives the level of intoxication of a living.
 * Returns:         The intoxication level.
 */
public int
query_intoxicated()
{
    int n;

    n = (time() - intoxicated_time ) / F_INTERVAL_BETWEEN_INTOX_HEALING;

    if (n == 0)
        return intoxicated;

    if (intoxicated > 0)
    {
        intoxicated -= n * F_SOBER_RATE;
        intoxicated = MAX(0, intoxicated);
    }
    intoxicated_time += n * F_INTERVAL_BETWEEN_INTOX_HEALING;

    return intoxicated;
}

/*
 * Function name:   query_stuffed
 * Description:     Gives the level of stuffedness of a living.
 * Returns:         The level of stuffedness.
 */
public int
query_stuffed()
{
    int t, n;

    n = (time() - stuffed_time) / F_INTERVAL_BETWEEN_STUFFED_HEALING;

    if (n == 0)
        return stuffed;

    stuffed -= F_UNSTUFF_RATE * n;
    stuffed = MAX(0, stuffed);

    stuffed_time += n * F_INTERVAL_BETWEEN_STUFFED_HEALING;

    return stuffed;
}

/*
 * Function name:   query_soaked
 * Description:     Gives the level of soakedness of  a living.
 * Returns:         The level of soakedness.
 */
public int
query_soaked()
{
    int n;

    n = (time() - soaked_time) / F_INTERVAL_BETWEEN_SOAKED_HEALING;

    if (n == 0)
        return soaked;

    soaked -= F_UNSOAK_RATE * n;
    soaked = MAX(0, soaked);

    soaked_time += n * F_INTERVAL_BETWEEN_SOAKED_HEALING;

    return soaked;
}

/*
 * Function name:   set_intoxicated
 * Description:     Set the level of intoxication of a living.
 * Arguments:       i: The level of intoxication.
 */
public void
set_intoxicated(int i)
{
    this_object()->calculate_hp();
    intoxicated = (i < 0 ? 0 : i);
}

/*
 * Function name:   set_stuffed
 * Description:     Set the level of stuffedness of a living
 * Arguments:       i: The level of stuffedness
 */
static void
set_stuffed(int i)
{
    this_object()->calculate_fatigue();
    stuffed = i;
}

/*
 * Function name:   set_soaked
 * Description:     Set the level of soakedness of a living
 * Arguments:       i: The level of soakedness
 */
static void
set_soaked(int i)
{
    soaked = i;
}

/*
 * Function name: query_guild_pref_total
 * Description  : This function returns the total percentage of experience
 *                that is taxed (i.e. that goes into the guild stats).
 * Returns      : int - the percentage of taxed experience.
 */
int
query_guild_pref_total()
{
    return (learn_pref[SS_RACE] + learn_pref[SS_LAYMAN] + learn_pref[SS_OCCUP]);
}

/*
 * Function name: query_stat_pref_total
 * Description  : This function returns the total percentage of experience
 *                that is not taxed (i.e. that goes into the stats).
 * Returns      : int - the percentage of untaxed experience.
 */
int
query_stat_pref_total()
{
    /* It is quicker to add three numbers than to add six numbers. */
    return (100 - query_guild_pref_total());
}

/*
 * Function name:   set_learn_pref
 * Description:     Calculate learning preferences summing up to 100
 *                  from an array containing arbitrary numbers.
 * Arguments:       pref_arr: An array with relative preference settings
 */
void
set_learn_pref(int *pref_arr)
{
    int sum;
    int i;
    int mval;
    int tmp;

    mval = 100;

    /* Make sure there's a proper value variable to use. */
    if (!pointerp(pref_arr))
	pref_arr = ({});

    if (!pointerp(learn_pref))
	learn_pref = ({});

    /* Make sure the arrays are large enough. */
    if (sizeof(pref_arr) < SS_NO_STATS)
        pref_arr += allocate(SS_NO_STATS - sizeof(pref_arr));

    if (sizeof(learn_pref) < SS_NO_STATS)
        learn_pref += allocate(SS_NO_STATS - sizeof(learn_pref));

    /* Take away the tax */
    for(i = SS_NO_EXP_STATS; i < SS_NO_STATS; i++)
        mval -= learn_pref[i];

    if (mval < 1)
    {
        for(i = 0; i < SS_NO_EXP_STATS; i++)
            learn_pref[i] = 0;

        return;
    }

    for(i = 0, sum = 0; i < SS_NO_EXP_STATS; i++)
        sum += pref_arr[i];

    if (sum > 0)
    {
        /* Try to avoid some rounding errors using this tmp */
        tmp = sum / 2;
        for (i = 0; i < SS_NO_EXP_STATS; i++)
            learn_pref[i] = (mval * pref_arr[i] + tmp) / sum;
    }
    else
    {
        tmp = mval / SS_NO_EXP_STATS;
        for (i = 0; i < SS_NO_EXP_STATS; i++)
            learn_pref[i] = tmp;
    }

    for(i = 0, sum = 0; i < SS_NO_EXP_STATS; i++)
        sum += learn_pref[i];

    sum = mval - sum;
    i = 0;

    if (sum > 0)
    {
        while (sum--)
        {
            learn_pref[i++]++;
            if (i >= SS_NO_EXP_STATS)
                i = 0;
        }
    }
    else if (sum < 0)
    {
        while (sum++)
        {
            if (learn_pref[i] > 0)
                learn_pref[i]--;

            if (++i >= SS_NO_EXP_STATS)
                i = 0;
        }
    }
}

/*
 * Function name: query_learn_pref
 * Description  : Return one or all values of the learn preferences, this
 *                  includes the 'guildtax' learn_prefs of race, occup, layman
 * Arguments    : int stat - Index of the stat to return, or -1 for all prefs.
 * Returns      : mixed - (int)  the learn pref for the stat required.
 *                        (int*) all learn prefs if stat == -1.
 */
public mixed
query_learn_pref(int stat)
{
    /* Return the learn prefs in a safe array. */
    if (stat < 0)
        return ({ }) + learn_pref;

    /* No such stats. */
    if (stat >= SS_NO_STATS)
        return -1;

    return learn_pref[stat];
}

/*
 * Function name:   set_guild_pref
 * Description:     Sets the guild tax/learn_pref for a specific guild.
 * Arguments:       guildstat: SS_RACE / SS_OCCUP / SS_LAYMAN
 *                  tax: taxrate for guild (in %)
 */
public void
set_guild_pref(int guildstat, int tax)
{
    if (guildstat >= SS_NO_EXP_STATS &&
        guildstat < SS_NO_STATS &&
        tax >= 0)
    {
        learn_pref[guildstat] = tax;
        set_learn_pref(query_learn_pref(-1));
    }
}

/*
 * Function name: update_skill
 * Description  : Called by set_skill when a skill changes to notify the
 *                combat object.
 * Arguments    : int skill - the skill number
 */
void
update_skill(int skill)
{
    switch (skill)
    {
    case SS_WEP_FIRST..(SS_WEP_FIRST + 10):
        map(this_object()->query_weapon(-1), this_object()->update_weapon);
        break;
        
    case SS_2H_COMBAT:
        this_object()->query_combat_object()->cb_calc_attackuse();
        break;
    }
}


/*
 * Function name: set_skill
 * Description  : Set a specific skill to a specific value.
 * Arguments    : int skill - the skill-number to set.
 *                int val   - the value to set the skill to.
 * Returns      : int 1/0   - true if successfull, else 0.
 */
public int
set_skill(int skill, int val)
{
    if (!mappingp(skillmap))
        skillmap = ([]);

    if (!intp(skill))
        return 0;

#ifdef LOG_SET_SKILL
    if (interactive(this_object()) &&
        (this_interactive() != this_object()))
    {
        SECURITY->log_syslog(LOG_SET_SKILL,
            sprintf("%s %-11s: %6d %3d -> %3d by %s\n", ctime(time()),
                capitalize(query_real_name()), skill, skillmap[skill], val,
                (objectp(this_interactive()) ?
                    capitalize(this_interactive()->query_real_name()) :
                    MASTER_OB(previous_object()))));
    }
#endif LOG_SET_SKILL

    /* Mudlib skills must be in the range 0 - 100 */
    if ((skill >= 0) && (skill <= SS_MUDLIB_SKILL_END))
        val = max(min(100, val), 0);

    skillmap[skill] = val;
    update_skill(skill);
    return 1;
}

/*
 * Function name: set_skill_extra
 * Description:   This is the function to call if you have an object that wants
 *                to temporarily change the skill of someone. Perhaps some
 *                grappling hooks to help climbing skill, or a fumble spell to
 *                lower fighting skills?
 * Arguments:     skill - Number of skill
 *                val   - The new val of the extra variable
 */
public void
set_skill_extra(int skill, int val)
{
    if (val == 0)
    {
        m_delkey(skill_extra_map, skill);
        return;
    }

    skill_extra_map[skill] = val;
    update_skill(skill);
}

/*
 * Function name: query_skill_extra
 * Description:   Query how much extra skill (or less) someone has in a
 *                particular skill.
 * Arguments:     skill - What skill to query
 * Returns:       The extra modifying value
 */
public int
query_skill_extra(int skill)
{
    return skill_extra_map[skill];
}

/*
 * Function name:   remove_skill
 * Description:     Remove a specific skill from a player
 * Arguments:       skill: The skill number to remove
 */
void
remove_skill(int skill)
{
    if (mappingp(skillmap))
    {
#ifdef LOG_SET_SKILL
        if (interactive(this_object()) &&
            (this_interactive() != this_object()))
        {
            SECURITY->log_syslog(LOG_SET_SKILL,
                sprintf("%s %-11s: %6d %3d ->    0 by %s\n", ctime(time()),
                    capitalize(query_real_name()), skill, skillmap[skill],
                    (objectp(this_interactive()) ?
                        capitalize(this_interactive()->query_real_name()) :
                        MASTER_OB(previous_object()))));
        }
#endif LOG_SET_SKILL

        m_delkey(skillmap, skill);
        update_skill(skill);
    }
}

/*
 * Function name:   query_base_skill
 * Description:     Gives the value of a specific skill. If there is need to
 *                  know the true skill of the player call this since it is
 *                  unshadowable.
 * Arguments:       skill: The number of the skill to check
 * Returns:         The true value of the skill
 */
nomask int
query_base_skill(int skill)
{
    if (!mappingp(skillmap))
        return 0;

    return skillmap[skill];
}

/*
 * Function name:   query_skill
 * Description:     Gives the value of a specific skill.
 * Arguments:       skill: The number of the skill to check
 * Returns:         The value of the skill
 */
public int
query_skill(int skill)
{
    if (!mappingp(skillmap))
        return 0;

    return skillmap[skill] + skill_extra_map[skill];
}

/*
 * Function name:   query_all_skill_types
 * Description:     Gives list of all current skills != 0
 * Returns:         an array with all skill-values
 */
public int *
query_all_skill_types()
{
    if (!mappingp(skillmap))
        return 0;
    return m_indexes(skillmap);
}

/*
 * Function name:   query_skill_descriptor
 * Description:     Gives the path to an object which defines the function
 *                  sk_rank, which assigns names to the skill levels.
 * Returns:         The path
 */
public string
query_skill_descriptor()
{
    return "/lib/skill_raise";
}

/*
 * Function name:   query_scar
 * Description:     Gives the scar bitmask of the living
 * Returns:         The scar bitmask
 */
public int
query_scar()
{
    return scar;
}

/*
 * Function name:   set_scar
 * Description:     Set the scar bitmask of the living
 * Arguments:       s: The scar bitmask
 */
public void
set_scar(int s)
{
    scar = s;
}

/*
 * Function name: valid_change_soul
 * Description  : This function checks whether the soul of a wizard may
 *                may be added or removed.
 * Returns      : 1/0; 1 = change allowed.
 */
nomask static int
valid_change_soul()
{
    object wizard;

    /* May always alter the soul of a mortal player. */
    if (!query_wiz_level())
        return 1;

    /* You may alter your own souls */
    if (geteuid(previous_object()) == geteuid(this_object()))
        return 1;

    /* Root may change everyones souls */
    if (geteuid(previous_object()) == ROOT_UID)
        return 1;

    if (!objectp(wizard = this_interactive()))
        return 0;

    if (wizard != this_player())
        return 0;

    /* You may change someones soul if you are allowed to snoop him. This
     * means Lords change their members, arch changed everyone < arch and
     * you may use snoop sanction to allow someone to patch your souls.
     */
    if (SECURITY->valid_snoop(wizard, wizard, this_object()))
        return 1;

    return 0;
}

/*
 * Function name:   add_cmdsoul
 * Description:     Add a command soul to the list of command souls. Note
 *                  that adding a soul is not enough to get the actions
 *                  added as well. You should do player->update_hooks()
 *                  to accomplish that.
 * Arguments:       soul: String with the filename of the command soul.
 * Returns:         1 if successfull,
 *                  0 otherwise.
 */
nomask public int
add_cmdsoul(string soul)
{

    if (!valid_change_soul())
        return 0;

    if (!((int)soul->query_cmd_soul()))
        return 0;

    /*
     * There can only be one!
     */
    remove_cmdsoul(soul);

    if (!sizeof(cmdsoul_list))
        cmdsoul_list = ({ soul });
    else
        cmdsoul_list += ({ soul });
    return 1;
}

/*
 * Function name:   remove_cmdsoul
 * Description:     Remove a command soul from the list.
 * Arguments:       soul: De filename of the soul to remove
 * Returns:         1 if successfull,
 *                  0 otherwise.
 */
nomask public int
remove_cmdsoul(string soul)
{
    int index;

    if (!valid_change_soul())
        return 0;

    if ((index = member_array(soul, cmdsoul_list)) < 0)
        return 0;

    cmdsoul_list = exclude_array(cmdsoul_list, index, index);
    return 1;
}

/*
 * Function name:   update_cmdsoul_list
 * Description:     Update the list of command souls
 * Arguments:       souls: The new filenames
 */
nomask static void
update_cmdsoul_list(string *souls)
{
    cmdsoul_list = souls;
}

/*
 * Function name:   query_cmdsoul_list
 * Description:     Give back the array with filenames of command souls.
 * Returns:         The command soul list.
 */
nomask public string *
query_cmdsoul_list()
{
    return secure_var(cmdsoul_list);
}

/*
 * Function name:   add_toolsoul
 * Description:     Add a tool soul to the list of tool souls. Note that
 *                  adding a soul is not enough to get the actions added
 *                  as well. You should do player->update_hooks() to
 *                  accomplish that.
 * Arguments:       soul: String with the filename of the tool soul.
 * Returns:         1 if successfull,
 *                  0 otherwise.
 */
nomask public int
add_toolsoul(string soul)
{
    if (!((int)SECURITY->query_wiz_rank(geteuid())))
        return 0;

    if (!((int)soul->query_tool_soul()))
        return 0;

    if (!valid_change_soul())
        return 0;

    /*
     * There can only be one!
     */
    remove_toolsoul(soul);

    if (!sizeof(tool_list))
        tool_list = ({ soul });
    else
        tool_list += ({ soul });
    return 1;
}

/*
 * Function name:   remove_toolsoul
 * Description:     Remove a tool soul from the list.
 * Arguments:       soul: De filename of the tool soul to remove
 * Returns:         1 if successfull,
 *                  0 otherwise.
 */
nomask public int
remove_toolsoul(string soul)
{
    int index;

    if (!valid_change_soul())
        return 0;

    if ((index = member_array(soul, tool_list)) < 0)
        return 0;

    tool_list = exclude_array(tool_list, index, index);
    return 1;
}

/*
 * Function name:   update_tool_list
 * Description:     Update the list of tool souls
 * Arguments:       souls: The new filenames
 */
nomask static void
update_tool_list(string *souls)
{
    tool_list = souls;
}

/*
 * Function name:   query_tool_list
 * Description:     Give back the array with filenames of tool souls.
 * Returns:         The tool soul list.
 */
nomask public string *
query_tool_list()
{
    return secure_var(tool_list);
}

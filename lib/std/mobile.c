/*
  /std/mobile.c

  This is the base for all nonplayer living objects. This file holds the
  code that is common to both non humanoid creatures and humanoid npc's.

*/
#pragma save_binary
#pragma strict_types

inherit "/std/living";
inherit "/std/act/seqaction" ;
inherit "/std/act/trigaction";

#include <ss_types.h>
#include <stdproperties.h>
#include <macros.h>
#include <std.h>
#include <options.h>

/* Private static global reactions.
 */
private	static object mobile_link;	 /* The linked master if controlled */
private static int    mobile_exp_factor; /* The xp modifier to give to killer */
private	static int    no_accept;	 /* Flag when mobile stops accepting */
#ifdef HEART_NEEDED
private	static int    test_alarm;	 /* Alarm used to check for presence */
#endif

/*
 * Prototypes
 */
varargs void default_config_mobile(int lvl);
mixed mobile_deny_objects();

void
create_mobile()
{
    /* 
     * Observe that we do not continue the 'default' calls here
     * All default values are already set.
     */
    set_name("mobile");
}

nomask void
create_living()
{
    set_skill(SS_BLIND_COMBAT, 40); /* Default skill for NPC's. */
    add_prop(CONT_I_HEIGHT, 160); /* Default height for monsters, 160 cm */
    add_prop(LIVE_M_NO_ACCEPT_GIVE, mobile_deny_objects);
    mobile_exp_factor = 100;
    this_object()->seq_reset();
    default_config_mobile();
    create_mobile();
    stats_to_acc_exp();
}

void
reset_mobile()
{
    ::reset_living();
}

nomask void
reset_living()
{
    reset_mobile();
}

/*
 * Function name: team_join
 * Description:   Sets this living as the leader of another
 * Arguments:	  member: The objectpointer to the new member of my team
 * Returns:       True if member accepted in the team
 */
int
team_join(object member)
{
    if (query_interactive(member))
	return 0;		/* mobile leaders overplayers */
    return ::team_join(member);
}

#ifdef HEART_NEEDED
static int   
test_live_here(object ob)
{
    return (living(ob) && interactive(ob) && (ob != this_object()));
}

/*
 * Function name: test_if_any_here
 * Description:   Turn of heart_beat if we are alone.
 * Returns:
 */
void
test_if_any_here()
{
    if (environment(this_object()) &&
	(!sizeof(filter(all_inventory(environment(this_object())), 
			test_live_here))))
    {
	stop_heart();
	test_alarm = 0;
    }
    else
	test_alarm = set_alarm(50.0, 0.0, test_if_any_here);
}

/* 
 * start_heart
 * Description:  When a mobile has an active heartbeat we must test
 *		 now and then if we can turn it off.
 */
static void
start_heart()
{
    ::start_heart();
    if (test_alarm != 0)
	remove_alarm(test_alarm);
    test_alarm = set_alarm(50.0, 0.0, test_if_any_here);
}
#endif

public string
query_race_name()
{
    mixed str;

    str = ::query_race_name();

    if (!str)
	return (string) DEFAULT_PLAYER->query_race();
    else 
	return str;
}

/*
 * Function name:  default_config_mobile
 * Description:    Sets all neccessary values for this mobile to function
 */
varargs void
default_config_mobile(int lvl)
{
    int i;

    for (i = 0; i < SS_NO_EXP_STATS ; i++)
	set_base_stat(i, (lvl ? lvl : 5));

    set_hp(500 * 500); /* Will adjust for CON above */

    stats_to_acc_exp();

    add_prop(CONT_I_WEIGHT, 70 * 1000);
    add_prop(CONT_I_VOLUME, 70 * 1000);

    mobile_exp_factor = 100;
    set_fatigue(query_max_fatigue());
}

/*
 * Function name: set_link_remote
 * Description:   Links a player to the output of the mobile
 * Arguments:	  player: Player to link mobile to
 */
void
set_link_remote(object player)
{
    object control;

    if (!player)
	player = this_player();

    if (!living(player) || mobile_link)
	return;

    control = clone_object(REMOTE_NPC_OBJECT); /* Get the remote control */
    control->set_npc(this_object());
    mobile_link = control;
    control->move(player);
}

/*
 * Description: Return the pointer to the current controller
 */
object
query_link_remote() { return mobile_link; }

/*
 *  Description: For monster link purposes
 */
void
catch_tell(string str)
{
    if (mobile_link)
	mobile_link->link_intext(str);
    ::catch_tell(str);
}

/*
 * Function name: catch_vbfc
 * Description  : This function is called for every normal message sent
 *                to this living. This used to be called catch_msg().
 * Arguments    : mixed str - the message to tell the player. The message
 *                    can be a string or an array in the form
 *                    ({ "met message", "unmet message", "unseen message" })
 *                object from_player - the originator of the message in case
 *                    the message is in array form.
 */
public void
catch_vbfc(mixed str, object from_player = 0)
{
    if (!query_interactive(this_object()) && !query_tell_active())
    {
        return;
    }
 
    if (pointerp(str))
    {
        if (!from_player)
        {
            from_player = this_player();
        }
        if ((sizeof(str) > 2) &&
            (!CAN_SEE_IN_ROOM(this_object()) ||
                !CAN_SEE(this_object(), from_player)))
        {
            catch_tell(str[2]);
        }
        else if (this_object()->query_met(from_player))
        {
            catch_tell(str[0]);
        }
        else
        {
            catch_tell(str[1]);
        }
    }
    else
    {
        catch_tell(process_string(str, 1));
    }
}

/*
 * Function name: catch_msg
 * Description  : See catch_vbfc.
 */
public void
catch_msg(mixed str, object from_player = 0)
{
    catch_vbfc(str, from_player);
}

/*
 * Function name: set_stats
 * Description  : Allows to set all stats of an NPC in a single call. The
 *                order is the same as the default stat order (see ss_types.h)
 *                and both general stats and guild stats may be set. It is
 *                possible to give a deviation from the basic values to add
 *                randomness.
 * Arguments    : int *stats - Array of stat values in order of the basic stat
 *                    list. The array may include both base stats and guild
 *                    stats. The length of the array may vary.
 *              : int deviation - The deviation in percentage from the value
 *                    that you set which will be randomly applied per stat.
 *                    A stat value of 50 and a deviation of 10% leads to a
 *                    stat value in the range 45-55. Maximum deviation: 50%
 *                    Default: 0%
 */
varargs public void
set_stats(int *stats, int deviation = 0)
{
    int index;
    int size;

    index = -1;
    size = ((sizeof(stats) > SS_NO_STATS) ? SS_NO_STATS : sizeof(stats));

    while(++index < size)
    {
	set_base_stat(index, stats[index], deviation);
    }
}

/*
 * Function name: set_exp_factor
 * Description  : Using this function you can set a percentage on the basic
 *                experience that is given out when killing this living. For
 *                instance, an NPC with special (magical) abilities might be
 *                more difficult to kill than one would guess based on the
 *                stats alone. Works only on NPC's. Range: 50% to 200%
 * Arguments    : int proc_xp - the percentage of the base experience that is
 *                    awarded. I.e. to give out 40% extra exp, set to 140.
 */
public void
set_exp_factor(int proc_xp)
{
    if ((proc_xp > 49) && (proc_xp < 201))
    {
	mobile_exp_factor = proc_xp;
    }
}

/*
 * Function name: query_exp_factor
 * Description  : This routine returns the percentage on the basic experience
 *                that is given out when killing this living. For instance. An
 *                NPC with special (magical) abilities might be more difficult
 *                to kill than one would assume based on the stats alone. The
 *                exp-factor is only valid for NPC's. Range: 50% to 200%
 * Returns      : int - the exp-factor, if it returns 140, that means that 40%
 *                    bonus is added.
 */
public int
query_exp_factor()
{
    return mobile_exp_factor;
}
   
/*
 * Function name: init_living
 * Description:   A patch for the automatic attack if this mobile can do that
 */
public void
init_living()
{
    this_object()->init_attack();
}

/*
 * Function name: special_attack
 * Description:   Called from the external combat object's heart_beat
 *                routine. By redefining this, monsters can easily attack
 *                with spells or take special actions when engaged
 *                in combat.
 * Arguments:     victim (the one we are fighting right now)
 * Returns:       0 - if we want the round to continue
 *                1 - if we are done with this round
 * Note:          By always returning 1 the mobile is unable
 *                to do an ordinary attack.
 */
public int
special_attack(object victim)
{
    return 0;
}
 
/*
 * Function name: mobile_deny_objects
 * Description:   This function is called from VBFC and NPC_M_NO_ACCEPT_GIVE
 *		  prop checking. If this mobile is intelligent enough he might
 *		  recognize that someone is trying to load him down before
 *		  attacking and then he doesn't accept any more objects. This
 *		  is merely default behaviour. Feel free to code something
 *		  different
 * Returns:       A message that will be printed to the player or 0
 */
mixed
mobile_deny_objects()
{
    string str;

    if (no_accept || random(100) < query_stat(SS_INT))
    {
	str = " doesn't accept any gifts from you.\n";
	no_accept = 1;
	return str;
    }
    return 0;
}

/*
 * Function name: refresh_mobile
 * Description:   This function is kept here for backwards compatibility,
 *                but is obsolete itself.
 */
void
refresh_mobile()
{
    refresh_living();
}

/*
 * Function name: query_option
 * Description:   Returns default option settings for mobiles
 * Arguments:     int opt - the option to check
 * Returns:       the setting for the specified option
 */
public int
query_option(int opt)
{
    switch(opt)
    {
    case OPT_NO_FIGHTS:
        return 1;

    case OPT_UNARMED_OFF:
	return query_prop(NPC_I_NO_UNARMED);

    case OPT_WHIMPY:
	return query_whimpy();
    }

    return 0;
}

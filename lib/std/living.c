/* 
 * /std/living.c
 *
 * Contains all routines relating to living objects of any kind.
 *
 * If you are going to copy this file, in the purpose of changing
 * it a little to your own need, beware:
 *
 * First try one of the following:
 *
 * 1. Do clone_object(), and then configure it. This object is specially
 *    prepared for configuration.
 *
 * 2. If you still is not pleased with that, create a new empty
 *    object, and make an inheritance of this object on the first line.
 *    This will automatically copy all variables and functions from the
 *    original object. Then, add the functions you want to change. The
 *    original function can still be accessed with '::' prepended on the name.
 *
 * The maintainer of this LPmud might become sad with you if you fail
 * to do any of the above. Ask other wizards if you are doubtful.
 *   
 * The reason of this, is that the above saves a lot of memory.
 */

#pragma save_binary
#pragma strict_types

inherit "/std/container";

#include <formulas.h>
#include <language.h>
#include <living_desc.h>
#include <log.h>
#include <macros.h>
#include <std.h>
#include <stdproperties.h>
#include <wa_types.h>

#include "/std/living/living.h"
#include "/std/living/savevars.c"
#include "/std/living/combat.c"
#include "/std/living/gender.c"
#include "/std/living/stats.c"
#include "/std/living/carry.c"
#include "/std/living/heart_beat.c"
#include "/std/living/drink_eat.c"
#include "/std/living/cmdhooks.c"
#include "/std/living/description.c"
#include "/std/living/move.c"
#include "/std/living/tasks.c"
#include "/std/living/wizstat.c"
#include "/std/living/spells.c"
#include "/std/living/possess.c"
#include "/std/living/notify_meet.c"
#include "/std/living/tool.c"
#include "/std/living/hold.c"
#include "/std/living/wear.c"
#include "/std/living/wield.c"
#include "/std/living/width_height.c"

static int tell_active_flag;      /* Flag to check in catch_vbfc() */

/*
 * Function name: query_init_master
 * Description:   Should return true if create_living shall be called
 *                in the master object of a living.
 */
public int
query_init_master()
{
    return 0;
}

/*
 * Function name: create_container
 * Description:   Create the living object. (constructor)
 */
nomask void
create_container()
{
    if (!(IS_CLONE ||
	  query_init_master()))
    {
	return;
    }

    if (!geteuid(this_object()))   /* Get our own uid if not prepared */
    { 
	setuid(); 
	seteuid(getuid(this_object()));
    }

    add_prop(LIVE_I_IS, 1);
    add_prop(CONT_I_ATTACH, 1);
    add_prop(CONT_I_HIDDEN, 1);
    add_prop(CONT_I_REDUCE_WEIGHT, 200);
    add_prop(CONT_I_REDUCE_VOLUME, 200);

    set_random_size_descs();
    save_vars_reset();
    skill_extra_map_reset();
    notify_meet_reset();
    gender_reset();
    spells_reset();
    ss_reset(); 
    carry_reset();
    drink_eat_reset();
    move_reset(); 
    wear_reset();
    wield_reset();
    combat_reset();
    hold_reset();

    enable_commands();
    cmdhooks_reset();

    create_living();

    combat_reload();

    /* An NPC has full hitpoints, full mana and full fatigue by default.
     * Update NPC hooks again to get proper racial sounds.
     */
    if (query_npc())
    {
        refresh_living();
        cmdhooks_reset();
    }
}

/*
 * Function name: create_living
 * Description:   Create the living object. (standard)
 */
public void
create_living()
{
    set_name("living");	/* Default name for living objects */
}

/*
 * Function name: reset_container
 * Description:   Reset the living object. 
 */
public nomask void
reset_container() { reset_living(); }

/*
 * Function name: reset_living
 * Description:   Reset the living object. (standard)
 */
public void
reset_living() { ::reset_container(); }

/*
 * Function name: init
 * Description:   Tells us of new players in our neigbourhood
 */
nomask void
init()
{
    ::init();
    combat_init();
#ifdef HEART_NEEDED
    start_heart();
#endif
    notify_meet_init();
    this_object()->init_living();
}

/*
 * Function name: encounter
 * Description:   Called when encountering an object
 */
public void
encounter(object obj)
{
    obj->init();
}

/*
 * Function name: command
 * Description  : Makes the living object execute a command, as if it was typed
 *                on the command line. For players, this function is redefined
 *                in /std/player_sec.c.
 * Arguments    : string cmd - the command with arguments to perform. For players
 *                    this should always be prefixed with a "$".
 * Returns      : int - the amount of eval-cost ticks if the command was
 *                    successful, or 0 if unsuccessfull.
 */
public int
command(string cmd)
{
    return efun::command(cmd);
}

/*
 * Function name: can_see_in_room
 * Description  : This function will return whether this object can see
 *                in the room he/she is in. It is used from filters, among
 *                other things.
 * Returns      : int 1/0 - the result from CAN_SEE_IN_ROOM()
 */
public nomask int
can_see_in_room()
{
    return CAN_SEE_IN_ROOM(this_object());
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
	    write_socket(str[2]);
	}
	else if (this_object()->query_met(from_player))
	{
	    write_socket(str[0]);
	}
	else
	{
	    write_socket(str[1]);
	}
    }
    else
    {
	write_socket(process_string(str, 1));
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
 * Function name: remove_object
 * Description:   Destruct this object, but check for possessed first
 */
public void
remove_object()
{
    possessed_remove();
    if (query_combat_object())
	catch(query_combat_object()->remove_object());
    ::remove_object();
}

/*
 * Function name: modify_command
 * Description:	 This is here so that a possessing wizard will get commands.
 *    Technically, it should be part of /std/living/possess.c, but since
 *    the lfun is more general, it is left here.
 */
string
modify_command(string cmd)
{
    return cmd;
}

/*
 * Function name: local_cmd()
 * Description:   Return a list of all add_actioned commands
 */

nomask string *
local_cmd()
{
    return get_localcmd();
}

/*
 * Function name: query_option
 * Description  : Livings by default have no options.
 * Arguments    : int opt - the option to query.
 * Returns      : int - 0
 */
int
query_option(int opt)
{
    return 0;
}

/*
 * Function name: set_tell_active
 * Description:   Sets the tell_active_flag so that catch_msg() will send
 *                all messages to us.
 * Arguments:     i - a number, 1 or 0, on or off
 */
void set_tell_active(int i) { tell_active_flag = i; }

/*
 * Functione name: query_tell_active
 * Description:    Query the tell_active_flag
 * Returns:        The flag
 */
int query_tell_active() { return tell_active_flag; }

public string
show_subloc(string subloc, object on, object for_obj)
{
    if (subloc == SUBLOC_HELD)
    {
        return show_held(for_obj);
    }

    if (subloc == SUBLOC_WORNA)
    {
        return show_worn(for_obj);
    }

    if (subloc == SUBLOC_WIELD)
    {
        return show_wielded(for_obj);
    }

    return "";
}

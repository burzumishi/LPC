/*
 * /d/Standard/obj/arma_tell.c
 *
 * A little object to supply mortals with the tell command when the game
 * is about to go down. They can use it to tell Armageddon that they like
 * to be sent home. Mortals can also communicate with each other, though.
 *
 * Revision history:
 * /Mercade, August 5th 1994: General revision of the armageddon system.
 * /Mercade, September 9 1997: Made it impossible to tell wizards.
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <config.h>
#include <options.h>
#include <std.h>
#include <stdproperties.h>

#define ARMA_TELL_ID "_arma_tell_"

/*
 * Function name: create_object
 * Description  : Called when the object is created.
 */
void
create_object()
{
    set_name(ARMA_TELL_ID);

    set_no_show();

    add_prop(OBJ_M_NO_DROP,     1);
    add_prop(OBJ_M_NO_STEAL,    1);
    add_prop(OBJ_M_NO_TELEPORT, 1);

    remove_prop(OBJ_I_VALUE);
    remove_prop(OBJ_I_VOLUME);
    remove_prop(OBJ_I_WEIGHT);
}

/*
 * Function name: tell
 * Descroption  : When armageddon is active, all players can tell to all
 *                other players, excluding wizards.
 * Arguments    : string str - the command line argument.
 * Returns      : int - 1/0 - success/failure.
 */
int
tell(string str)
{
    object ob;
    string who;
    string msg;
    int busy;

    if (!strlen(str) ||
	(sscanf(str, "%s %s", who, msg) != 2))
    {
	notify_fail("Tell who what?\n");
	return 0;
    }

    who = lower_case(who);
    if (who == "armageddon")
    {
	(ARMAGEDDON)->teleledningsanka();
	ob = find_object(ARMAGEDDON);
    }
    else
    {
	ob = find_player(who);
    }

    if (!objectp(ob))
    {
	notify_fail("No player named " + capitalize(who) + " present.\n");
	return 0;
    }

    if (!this_player()->query_met(ob))
    {
	notify_fail("There is no player called " + capitalize(who) +
	    " that you recall to have met.\n");
	return 0;
    }

    if (ob->query_wiz_level())
    {
	write("If you desparately need to speak with " +
	    capitalize(SECURITY->query_wiz_pretitle(ob)) + " " +
	    capitalize(who) + ", seek an audience with the 'commune' " +
	    "command. Make sure you read the help page first.\n");
	return 1;
    }

    if ((who != "armageddon") && (!query_ip_number(ob)))
    {
	write(capitalize(who) + " is link dead right now.\n");
	return 1;
    }

    busy = ob->query_prop(WIZARD_I_BUSY_LEVEL);
    /* 152 = 8 (P) + 16 (S) + 128 (F) */
    if (busy & 152)
    {
	write(capitalize(who) + " seems to be busy at the moment.\n");
	return 1;
    }

    tell_object(ob, this_player()->query_Art_name(ob) + " tells you: " +
	msg + "\n");

    if (this_player()->query_option(OPT_ECHO))
    {
	tell_object(this_player(), "You tell " + capitalize(who) + ": " +
	    msg + "\n");
    }
    else
    {
        write("Ok.\n");
    }

    return 1;
}

/*
 * Function name: init
 * Description  : Link the command to the player.
 */
void
init()
{
    ::init();

    add_action(tell, "tell");
}

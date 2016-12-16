/*
 * /cmd/wiz/possob.c
 *
 * This object is the object that controls the release mechanism during
 * possessions as well as the possessed player in case of player possessions.
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

#include <macros.h>
#include <std.h>
#include <stdproperties.h>

inherit "/std/object";

/*
 * Global variables.
 */
static object demon;
static object possessed;
static int    is_player;

/*
 * Prototypes.
 */
static varargs int release(string str);
static int think(string arg);
static void check_env();

/*
 * Function name: create_object
 * Description:	  Initialize the object
 */
public void
create_object()
{
    set_name("possob");
    add_name("player");
    add_adj("possessed");

    set_short("possessed player");
    set_long("This is the sadly abused soul of a possessed player. " +
	"However, you shouldn't be able to see this, so something bad " +
	"must have happened. Contact an Arch or Keeper immediately!\n");
    set_no_show();
    is_player = 0;
}

/*
 * Function name: init
 * Description  : Initialize some commands.
 */
public void
init()
{
    ::init();

    add_action(release, "quit");
    add_action(think, "think");
}

/*
 * Function name: release
 * Description  : Stop possessing.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
static int
release(string str)
{
    if (strlen(str))
    {
    	notify_fail("Quit what?\n");
	return 0;
    }

    /* Get rid of the snoop. */
    if (is_player)
    {
	snoop(this_object());
    }

    /* Move the demon back into his own body. */
    exec(demon, possessed);

    /* If the possessed object is a player, move him back. */
    if (is_player)
    {
	exec(possessed, this_object());
    }

    tell_object(possessed, "You feel the alien presence withdraw and " +
	"you regain control of your body.\n");
    tell_object(demon, "You release " +
	capitalize(possessed->query_real_name()) + ".\n");
    possessed->set_possessed("");
    remove_object();

    return 1;
}

/*
 * Function name: possess
 * Description  : Set up the possession of a player or living complete with
 *                initialization of the possess object and messages to both
 *                parties.
 * Arguments    : object ob1 - the demon.
 *                object ob2 - the possessed.
 * Returns      : int 1/0 - true if successful.
 */
public int
possess(object ob1, object ob2)
{
    if (obj_no_change)
    {
	return 0;
    }

    /* This function may only be called from the command soul of normal
     * wizards.
     */
    if (previous_object() != find_object(WIZ_CMD_NORMAL))
    {
	return 0;
    }

    demon = ob1;
    possessed = ob2;

    /* If the possessed object is a player, move him here. */
    if (interactive(possessed))
    {
	is_player = 1;
	exec(this_object(), possessed);
    }

    exec(possessed, demon);
    possessed->set_possessed(demon->query_real_name());

    tell_object(this_object(), "You feel your very soul being forced " +
	"together and crammed into a remote\n" +
	"corner of your brain. Something alien and powerful takes command " +
	"of your\n" +
	"body. You can only watch from a distance in helpless terror. " +
	"All you seem\n" +
	"to be able to do is to 'think' unhappy thoughts.\n");
    tell_object(possessed, "You are now possessing " +
	capitalize(possessed->query_real_name()) + ". Use 'quit' to release " +
        "the possessee and return to the control of your own body.\n");

    /* Give the possessed the ability to 'think'. It parses all commands
     * executed by the possessed to the function think in order to generate
     * proper fail message when the player wants to do anything else.
     */
    enable_commands();
    add_action(think, "", 1);

    /* Start a periodic environment checker. */
    set_alarm(60.0, 60.0, check_env);

    return 1;
}

/*
 * Function name: check_env
 * Description  : Periodically check that the object that is possessed
 *                hasn't been destructed or the possession object moved.
 *                If this happens, stop possessing.
 */
static void
check_env()
{
    /*
     * In this case the possessed body has been destroyed which means
     * that the demon has been thrown out as well. Better remove his
     * body as well in that case.
     */
    if (!possessed && demon)
	demon->remove_object();

    /*
     * This object has been moved out of the possessed body. 
     * Stop possessing.
     */
    if (environment(this_object()) != possessed)
	release(possessed->query_real_name());
}

/*
 * Function name: think
 * Description  : Allows the sadly possessed to think, the only way to
 *                communicated with the demon in charge of his body.
 * Arguments    : string arg - the thought.
 */
public int
think(string arg)
{
    /* The player can only think, every other command will return in a
     * proper fail message.
     */
    if (query_verb() == "think")
    {
	if (!strlen(arg))
	{
	    tell_object(possessed, "Cogito ergo sum.\n");
	}
	else
	{
	    tell_object(possessed, "You think: " + arg + "\n");
	}

	return 1;
    }

    notify_fail("It is impossible for you to \"" + query_verb() +
	(strlen(arg) ? (" " + arg) : "") + "\" as your body is possessed " +
	"by a forceful magician. All you can do is 'think' unhappy " +
	"thoughts.\n");
    return 0;
}

/*
 * Function name: catch_tell
 * Description  : This function is called when a message is printed to
 *                the poor soul crammed into this object. It prints the
 *                message to the connected socket.
 * Arguments    : string msg - the message to print.
 */
public void
catch_tell(string msg)
{
    write_socket(msg);
}

/*
 * Function name: query_prevent_shadow
 * Description  : Prevents shadowing of this object.
 * Returns      : int 1 - always.
 */
public nomask int
query_prevent_shadow()
{
    return 1;
}

/*
 * Function name: query_possessed
 * Description:	  Return the possessed object.
 */
public string
query_possessed()
{
    return (string)possessed->query_real_name();
}

/*
 * Function name: query_demon
 * Description  : Return the name of the possessing object.
 * Returns      : string - the name of the demon.
 */
public string
query_demon()
{
    return demon->query_real_name();
}

/*
 * Function name: query_real_name
 * Description  : This function returns the name of the wizard possessing
 *                the poor living whose soul is crammed into this object.
 *                The return value of this function should be identical to
 *                the result of the function query_demon().
 * Returns      : string - the name.
 */
public string
query_real_name()
{
    return geteuid(this_object());
}

/*
 * Function name: modify_command
 * Description  : This function modifies the command the possessed living
 *                enters before it is executed. Normally this would resolve
 *                aliases et cetera, but we just return the command as it
 *                was entered.
 * Arguments    : string cmd - the command the player entered.
 * Returns      : string - the same as the argument 'cmd'.
 */
public string
modify_command(string cmd)
{
    return cmd;
}

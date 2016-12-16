/*
 * /cmd/live/info.c
 *
 * General commands for giving and getting game information.
 * The following commands are defined:
 *
 * - bug
 * - date
 * - done
 * - help
 * - idea
 * - praise
 * - sysbug
 * - sysidea
 * - syspraise
 * - systypo
 * - typo
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/cmd/std/command_driver";

#include <cmdparse.h>
#include <composite.h>
#include <language.h>
#include <log.h>
#include <macros.h>
#include <std.h>
#include <time.h>

/* These properties are only internal to this module, so we define them
 * here rather than in the properties file stdproperties.h.
 */
#define PLAYER_I_LOG_TYPE   "_player_i_log_type"
#define PLAYER_O_LOG_OBJECT "_player_o_log_object"

/* **************************************************************************
 * The constructor.
 */
void 
create()
{
    setuid();
    seteuid(getuid()); 
}

/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
string
get_soul_id()
{
    return "info";
}

/* **************************************************************************
 * This is a command soul.
 */
int
query_cmd_soul()
{
    return 1;
}

/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
mapping
query_cmdlist()
{
    return ([
	"bug"      : "report",

	"date"     : "date",
	"done"     : "report_done",

	"help"     : "help",

	"idea"     : "report",

	"praise"   : "report",

	"sysbug"   : "report",
	"sysidea"  : "report",
	"syspraise": "report",
	"systypo"  : "report",

	"typo"     : "report" ]);
}

/*
 * Function name: using_soul
 * Description:   Called once by the living object using this soul. Adds
 *		  sublocations responsible for extra descriptions of the
 *		  living object.
 */
public void 
using_soul(object live)
{
}

/* **************************************************************************
 * Here follow some support functions. 
 * **************************************************************************/

/*
 * Function name: done_reporting
 * Descripiton  : This function is called from the editor when the player
 *                has finished the report he or she was writing. If any
 *                text was entered it is logged and the player is thanked.
 * Arguments    : string str - the text entered into the editor.
 */
public void
done_reporting(string str)
{
    int type = this_player()->query_prop(PLAYER_I_LOG_TYPE);

    if (!strlen(str))
    {
	write(LOG_ABORT_MSG(LOG_MSG(type)));
	return;
    }

    /* Log the note, thank the player and then clean up after ourselves. */
    SECURITY->note_something(str, type,
        this_player()->query_prop(PLAYER_O_LOG_OBJECT));
    write(LOG_THANK_MSG(LOG_MSG(type)));

    this_player()->remove_prop(PLAYER_I_LOG_TYPE);
    this_player()->remove_prop(PLAYER_O_LOG_OBJECT);
}

/* **************************************************************************
 * Now the actual commands. Please add new in the alphabetical order.
 * **************************************************************************/

/*
 * date - get local time & date + uptime information
 */
int
date()
{
    int runlevel;
    int delay, interval;

    write("Local time    : " + ctime(time()) + "\n");
    write("Start time    : " + ctime(SECURITY->query_start_time()) + "\n");
    write("Up time       : " + CONVTIME(time() -
	SECURITY->query_start_time()) + "\n");
    write("Memory usage  : " + SECURITY->query_memory_percentage() + "%\n");
#ifdef REGULAR_REBOOT
    write("Regular reboot: " + "Every day after " + REGULAR_REBOOT + ":00\n");
#endif REGULAR_REBOOT

#ifdef REGULAR_UPTIME
    delay = SECURITY->query_irregular_uptime() +
        SECURITY->query_start_time() - time();
#endif REGULAR_UPTIME

    /* Information about the reboot status. */
    if (ARMAGEDDON->shutdown_active())
    {
	write("Armageddon    : Shutdown in " +
	     CONVTIME(ARMAGEDDON->shutdown_time()) + ".\n");
	write("Shutdown by   : " + capitalize(ARMAGEDDON->query_shutter()) +
	     ".\n");
	write("Reason        : " + ARMAGEDDON->query_reason() + "\n");
    }
#ifdef REGULAR_UPTIME
    else if (delay <= 0)
    {
        write("Regular reboot: Announced within the next 15 minutes.\n");
    }
    else if (this_player()->query_wiz_level())
    {
        write("Regular reboot: " + CONVTIME(delay) + " to go.\n");
    }
    else
    {
        /* Round down <= 1h30 -> 0h15, <= 1day -> 1h30,
                      <= 2day -> 3h00, >= 2day -> 6h00 */
        if (delay <= 86400)
        {
            interval = (delay <= 5400) ? 900 : 5400;
        }
        else
        {
            interval = (delay <= 129600) ? 10800 : 21600;
        }
        delay = ((delay / interval) * interval);
        if (delay > 0)
        {
            write("Regular reboot: between " + CONVTIME(delay) + " and " +
                CONVTIME(delay + interval) + " to go.\n");
        }
        else
        {
            /* Note: 15 minutes because of being in the last interval and
             * another 15 minutes because Armageddon needs to wake up. */
            write("Regular reboot: Announced within the next 30 minutes.\n");
        }
    }
#endif REGULAR_UPTIME

    /* Tell wizards some system data. */
    if (this_player()->query_wiz_level())
    {
        if (runlevel = SECURITY->query_runlevel())
        {
            write("Runlevel      : " + WIZ_RANK_NAME(runlevel) +
                " (and higher).\n");
        }

	write(HANGING_INDENT("System data   : " +
	    SECURITY->do_debug("load_average"), 16, 0) + "\n");
    }

    return 1;
}

/*
 * help - Get help on a subject
 */
int
help(string what)
{
    string dir = "general/";

    if (!stringp(what))
    {
        what = "help";
    }

    /* Wizards get to see the wizard help pages by default. */
    if ((this_player()->query_wiz_level()) &&
    	(this_player() == this_interactive()))
    {
	/* ... unless they want to see the general page. */
	if (wildmatch("g *", what))
	{
	    what = extract(what, 2);
	}
	else if (file_size("/doc/help/wizard/" + what) > 0)
	{
	    dir = "wizard/";
	}
    }

    if (file_size("/doc/help/" + dir + what) > 0)
    {
    	setuid();
    	seteuid(getuid());

	this_player()->more(("/doc/help/" + dir + what), 1);
	return 1;
    }

    notify_fail("No help on \"" + what + "\" available.\n");
    return 0;
}

/*
 * Report - make a report of some kind.
 */
int
report(string str)
{
    object *oblist;
    object target;
    int type = LOG_TYPES[query_verb()];

    /* This should never happen for it means the LOG_TYPES mapping has not
     * been properly setup.
     */
    if (!type)
    {
	notify_fail("Illegal log type \"" + query_verb() +
	    "\". Please report this to an archwizard.\n");
	return 0;
    }

    /* Player may describe the object to make a report about. */
    if (stringp(str))
    {
	/* If there is an argument to the 'done' or 'sys..' commands, take
         * it as the message.
         */
	if ((type >= LOG_SYSBUG_ID) ||
	    (type == LOG_DONE_ID))
	{
	    this_player()->add_prop(PLAYER_I_LOG_TYPE, type);
            this_player()->add_prop(PLAYER_O_LOG_OBJECT,
                environment(this_player()));

	    done_reporting(str + "\n");
	    return 1;
	}

	/* Find the target. */
	if (!parse_command(str, environment(this_player()), "[the] %i",
			   oblist) ||
	    (!sizeof(oblist = NORMAL_ACCESS(oblist, 0, 0))))
	{
	    notify_fail("Make a " + query_verb() + " report about what?\n");
	    return 0;
	}

	/* One target at a time. */
	if (sizeof(oblist) > 1)
	{
	    notify_fail("Select only one target to make a " + query_verb() +
		" report about.\n");
	    return 0;
	}

	target = oblist[0];
	write("Making a " + query_verb() + " report about " +
	    LANG_THESHORT(target)+ ".\n");
    }
    else
    {
	target = environment(this_player());
    }

    /* Add the relevant data to the player. */
    this_player()->add_prop(PLAYER_I_LOG_TYPE, type);
    this_player()->add_prop(PLAYER_O_LOG_OBJECT, target);

    setuid();
    seteuid(getuid());

    clone_object(EDITOR_OBJECT)->edit("done_reporting", "");
    return 1;
}

/*
 * report_done - Report something as done (wizards only)
 */
int
report_done(string str)
{
    /* To mortal players, the command done does not exist so we do not
     * have to give a notify_fail message. Apprentices, pilgrims and
     * retired wizards cannot 'do' anything either.
     */
    if (WIZ_CHECK < WIZ_NORMAL)
    {
	return 0;
    }

    return report(str);
}

/*
 * Clone and move this object to a player if you want to paralyze him.
 */
#pragma strict_types

inherit "/std/object";

#include <cmdparse.h>
#include <stdproperties.h>

/*
 * Variables
 */
static string	stop_verb,	/* What verb to stop this paralyze ? */
		stop_fun,	/* What function to call when stopped */
		fail_message,   /* Message to write when command failed */
		stop_message;	/* Message to write when paralyze stopped */
static int	remove_time;	/* Shall it go away automatically? */
static object	stop_object;	/* Object to call stop_fun in when stopped */

/*
 * Prototypes
 */
void set_standard_paralyze(string str);
int stop(string str);
varargs void stop_paralyze();

/*
 * Function name: create_paralyze
 * Description:	  Set up standard paralyze
 */
void
create_paralyze()
{
    set_standard_paralyze("paralyze");
}

/*
 * Function name: create_object
 * Description:   The standard create routine.
 */
nomask void
create_object()
{
    create_paralyze();

    set_no_show();

    add_prop(OBJ_M_NO_GET, 1);
    add_prop(OBJ_M_NO_DROP, 1);
    add_prop(OBJ_M_NO_STEAL, 1);
    add_prop(OBJ_M_NO_TELEPORT, 1);
}

/*
 * Function name: init
 * Description:   Called when meeting an object
 */
void
init()
{
    ::init();

    if (remove_time)
    {
        set_alarm(itof(remove_time), 0.0, stop_paralyze);
    }

    add_action(stop, "", 1);
}

/*
 * Function name: stop
 * Description  : Here all commands the player gives comes.
 * Argument     : string str - The command line argument.
 * Returns      : int 1/0    - success/failure.
 */
int
stop(string str)
{
    /* Only paralyze our environment */
    if (environment() != this_player())
    {
        return 0;
    }

    /* Some commands may always be issued. */
    if (CMDPARSE_PARALYZE_CMD_IS_ALLOWED(query_verb()))
    {
        return 0;
    }

    /* If there is a verb stopping the paralyze, check it. */
    if (stringp(stop_verb) &&
	(query_verb() == stop_verb))
    {
        /* If a stop_fun is defined it MUST return 1 in order not stop the
         * paralyze.
         */
        if (objectp(stop_object) &&
            call_other(stop_object, stop_fun, str))
        {
            return 1;
        }

	if (stringp(stop_message))
        {
            this_player()->catch_msg(stop_message);
        }

        remove_object();
        return 1;
    }

    /* We allow VBFC, so here we may use catch_msg(). */
    if (stringp(fail_message))
    {
        this_player()->catch_msg(fail_message);
    }

    /* Only paralyze mortals. */
    if (!this_player()->query_wiz_level())
    {
        return 1;
    }

    write("Since you are a wizard this paralyze won't affect you.\n");
    return 0;
}

/*
 * Function name: dispel_magic
 * Description  : If this paralyze should be able to lift magical, define this
 *		  function.
 * Arguments    : int i   - a number indicating how strong the dispel spell is.
 * Returns      : int 1/0 - 1 if dispelled; otherwise 0.
 */
int
dispel_magic(int i)
{
    return 0;
}

/*
 * Function name: set_stop_verb
 * Description  : Set the verb to stop paralyze, if possible.
 * Arguments    : string verb - the verb to use.
 */
void
set_stop_verb(string verb)
{
    stop_verb = verb;
}

/*
 * Function name: query_stop_verb
 * Description  : Return the stopping verb.
 * Returns      : string - the verb.
 */
string
query_stop_verb()
{
    return stop_verb;
}

/*
 * Function name: set_stop_fun
 * Description  : Set function to call when paralyze stops if there is such
 *                a function.
 * Arguments    : string fun - the function to call.
 */
void
set_stop_fun(string fun)
{
    stop_fun = fun;
}

/*
 * Function name: query_stop_fun
 * Description  : Returns the function to call when paralyze stops.
 * Returns      : string - the function name.
 */
string
query_stop_fun()
{
    return stop_fun;
}

/*
 * Function name: set_stop_object.
 * Description  : Set which object to call the stop function in.
 * Arguments    : object ob - the object.
 */
void
set_stop_object(object ob)
{
    stop_object = ob;
}

/*
 * Function name: query_stop_object
 * Description  : Returns which object to call the stop function in.
 * Returns      : object - the object.
 */
object
query_stop_object()
{
    return stop_object;
}

/*
 * Function name: set_fail_message
 * Description  : Set the fail message when player tries to do something.
 *                This supports VBFC and uses this_player().
 * Arguments    : string - the fail message.
 */
void
set_fail_message(string message)
{
    fail_message = message;
}

/*
 * Function name: query_fail_message
 * Description  : Returns the fail message when player tries to do something.
 *                This returns the real value, not resolved for VBFC.
 * Returns      : string - the message.
 */
string
query_fail_message()
{
    return fail_message;
}

/*
 * Function name: set_remove_time
 * Description  : Set how long time player should be paralyzed (in seconds).
 * Arguments    : int time - the time to set.
 */
void
set_remove_time(int time)
{
    remove_time = time;
}

/*
 * Function name: query_remove_time
 * Description  : Returns the paralyze time (in seconds).
 * Returns      : int - the time.
 */
int
query_remove_time()
{
    return remove_time;
}

/*
 * Function name: set_stop_message
 * Description  : Set the message written when paralyze stops. This may
 *                support VBFC and uses this_player().
 * Arguments    : string - the message.
 */
void
set_stop_message(string message)
{
    stop_message = message;
}

/*
 * Function name: query_stop_message
 * Description  : Returns the message written when paralyze stops. It returns
 *                the real value, not solved for VBFC.
 * Returns      : string - the message.
 */
string
query_stop_message()
{
    return stop_message;
}

/*
 * Function name: set_standard_paralyze
 * Description  : Set up standard settings for a paralyze.
 * Arguments    : string str - When the player uses the stop-verb, 'stop',
 *                             the message 'You stop <str>.\n' is printed
 *                             to the player.
 */
void
set_standard_paralyze(string str)
{
    set_stop_verb("stop");
    set_stop_fun("stop_paralyze");
    set_stop_object(previous_object());
    set_stop_message("You stop " + str + ".\n");
    set_fail_message("You are busy with other things right now. You must " +
        "'stop' to do something else.\n");
}

/*
 * Function name: stop_paralyze
 * Description  : This function is called if the paralyze shall stop due to 
 *		  the time running out.
 */
void
stop_paralyze()
{
    if (!objectp(environment()))
    {
        remove_object();
        return;
    }

    set_this_player(environment());

    if (objectp(stop_object) && stringp(stop_fun) &&
        (stop_object != this_object()))
    {
        call_other(stop_object, stop_fun, environment());
    }
    else if (strlen(stop_message))
    {
        environment()->catch_msg(stop_message);
    }

    remove_object();
}

/*
 * Function name: stat_object
 * Description  : Function called when wiz tries to stat this object.
 * Returns      : string - the extra information to print.
 */
string
stat_object()
{
    string str = ::stat_object();

    if (strlen(stop_verb))
    {
        str += "Stop verb: " + stop_verb + "\n";
    }
    if (strlen(stop_fun))
    {
        str += "Stop fun:  " + stop_fun + "\n";
    }
    if (strlen(stop_message))
    {
        str += "Stop mess: " + stop_message + "\n";
    }
    if (strlen(fail_message))
    {
        str += "Fail mess: " + fail_message + "\n";
    }
    if (remove_time)
    {
        str += "Duration:  " + remove_time + "\n";
    }
    if (objectp(stop_object))
    {
        str += "Stop obj:  " + file_name(stop_object) + "\n";
    }

    return str;
}

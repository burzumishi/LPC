/*
 * /cmd/std/command_driver.c
 *
 * These routines handles the actual finding and execution of commands.
 * It is called from cmdhooks.c in the living object that performs the
 * command.
 */

#pragma no_clone
#pragma save_binary
#pragma strict_types

inherit "/lib/commands";
inherit "/std/callout";

#include <std.h>

/*
 * Prototype.
 */
public mapping query_cmdlist();

/*
 * Global variable.
 *
 * cmdlist   - the list of verbs and functions.
 */
static mapping cmdlist = query_cmdlist();

/*
 * Function name: query_cmdlist
 * Description  : You must redefine this function in your soul. This
 *                function returns a mapping with the verbs and functions
 *                that are defined in the soul. The format of the mapping
 *                should be ([ "verb" : "function" ]).
 * Returns      : mapping - the verb & function - mapping.
 */
public mapping
query_cmdlist()
{
    return ([ ]);
}

/*
 * Function name: update_commands
 * Description  : This function is called from the spell object when a
 *                new spell is added.
 */
public void
update_commands()
{
    cmdlist = query_cmdlist();
}

/* 
 * Function name: do_command
 * Description  : Perform the given command, if present.
 * Arguments    : string verb - the verb the player executes.
 *                string arg  - the command line argument.
 * Returns      : int - 1/0 depending on success.
 */
nomask public int
do_command(string verb, string arg)
{
    return call_other(this_object(), cmdlist[verb], arg);
}

/* 
 * Function name: exist_command
 * Description  : Check if a command exists.
 * Returns      : int - 1/0 depending on success.
 */
nomask public int
exist_command(string verb)
{
    return stringp(cmdlist[verb]);
}

/*
 * Function name: open_soul
 * Description  : Set the euid of the soul to 0.
 */
nomask public void
open_soul(int state)
{
    if (state)
    {
        seteuid(getuid(this_object()));
    }
    else
    {
        SECURITY->remote_setuid();
    }
}

/*
 * Function name: teleledningsanka
 * Description  : This function is used to load a soul and in command
 *                souls also to set the uid, euid to "backbone". If you
 *                don't speak Swedish, rest assured that the name of this
 *                function isn't real Swedish either.
 */
void
teleledningsanka()
{
    SECURITY->remote_setuid();
}

/*
 * Function name: using_soul
 * Description  : this function is called from /std/living.c when it first
 *                starts to use the soul and can be used for initialization.
 * Arguments    : object live - the living that is going to use this soul.
 */
public void
using_soul(object live)
{
}

/*
 * Function name: query_prevent_shadow
 * Description  : Used to make it impossible to shadow any soul.
 * Returns      : int 1 - always
 */
nomask public int
query_prevent_shadow()
{
    return 1;
}
 
/*
 * Function name: query_alarms
 * Description:   This function gives all alarms set in this object.
 * Returns:       The list as given by get_all_alarms.
 */
mixed
query_alarms()
{
    return get_all_alarms();    
}

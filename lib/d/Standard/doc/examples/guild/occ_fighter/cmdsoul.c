/*
 * NOTE:  See /cmd/std/soul_cmd.c for more examples of coding emotes.
 */

#pragma strict_types

/* Inherit this for guild souls */
inherit "/cmd/std/command_driver";

#include "guild.h"

#include <macros.h>

/* 
 * Function name: get_soul_id
 * Description:   Give a name for this soul
 * Returns:       string - the soul's id
 */
string
get_soul_id() 
{ 
    return GUILD_NAME + " soul"; 
}

/*
 * Function name: query_cmd_soul
 * Description:   identify this as a valid cmdsoul
 * Returns:       1
 */
int 
query_cmd_soul() 
{ 
    return 1; 
}

/*
 * Function name: query_cmdlist
 * Description:   Get the commands available on this soul and
 *                their corresponding function names.
 * Returns:       mapping - a mapping that maps command names
 *                          to function names.
 */
mapping
query_cmdlist()
{
    return ([
            /* Command name : Function name */
              "guildyell"   : "guildyell",
              "guildkick"   : "guildkick",
              "guildlaugh"  : "guildlaugh",
              "guildpunch"  : "guildpunch",
           ]);
}

/*
 * Function name: guildyell
 * Description:   This is the code for the "guildyell" command.
 *                This is a very simple emote, which takes no
 *                arguments.
 * Arguments:     The arguments the player gave to the "guildyell"
 *                command.
 * Returns:       1 / 0 - command succeeded / command failed
 */
int
guildyell(string str)
{
    if (strlen(str))
    {
        /* The player tried to specify an argument for the
         * command.  Return 0 since the command failed.  This
         * Will result in a "What?" message to the player.
	 */
        return 0;
    }

    /* message to the actor */
    write("You yell in anger.\n");

    /* message to others in the room */
    allbb(" yells in anger.");

    /* Command successful, so return 1 */
    return 1;
}

/*
 * Function name: guildkick
 * Description:   This is the code for the "guildkick" command.
 * Arguments:     The arguments the player gave to the "guildkick"
 *                command.
 * Returns:       1 / 0 - command succeeded / command failed
 */
int
guildkick(string str)
{
    object *oblist;

    if (!strlen(str) || !sizeof(oblist = parse_this(str, "[the] %l")))
    {
        notify_fail("Guildkick whom?\n");
        return 0;
    }

    /* message to the actor.  Use actor() instead of write() when
     * The emote has a target
     */
    actor("You kick", oblist, " as only a member of the " + GUILD_NAME +
        "could.");

    /* message to the target */
    target(" kicks you.", oblist);

    /* message to others present.  Use all2act() or all2actbb()
     * instead of all() or allbb() when the emote has a target. 
     */
    all2actbb("kicks", oblist, ".");

    return 1;
}

/* 
 * Function name: guildlaugh
 * Description:   This is the code for the "guildlaugh" command.
 * Arguments:     The arguments the player gave to the "guildlaugh"
 *                command.
 * Returns:       1 / 0 - command succeeded / command failed
 */
int
guildlaugh(string str)
{
    object *ob;

    if (!strlen(str)) /* No arguments given */
    {
        /* message to the player who performed the command */
	write("You laugh.\n");

        /* message to all others */
        all(" laughs.");

        /* command succeeded, so return 1 */
	return 1;
    }

    /* An argument has been specified */

    /* Use parse_this() to find the living objects in the player's
     * environment that match the argument given for the command.
     * Note that this only returns those livings that the actor
     * can see.
     */
    ob = parse_this(str, "[at] [the] %l");

    if (!sizeof(ob)) /* No-one found that matches the argument given */
    {
        /* Set the failure message.  If this is not set, the default
         * message is "What?\n".
         */
        notify_fail("Guildlaugh at whom?\n");

        /* command failed, so return 0 */
	return 0;
    }

    /* message to the actor */
    actor("You laugh at", ob, ".");
    /* message to the target(s) */
    target(" laughs at you.", ob);
    /* message to others present */
    all2act("laughs at", ob, ".");

    /* Command succeeded, so return 1 */
    return 1;
}

/*
 * Function name: guildpunch
 * Description:   This is the code for the "guildpunch" command.
 *                This is a special attack.
 * Arguments:     The arguments the player gave to the "guildpunch"
 *                command.
 * Returns:       1 / 0 - command succeeded / command failed
 */
int
guildpunch(string str)
{
    object ob, *obj;
    string how;
 
    /* What are we attacking anyway? */
    if (strlen(str))
    {
        if (!sizeof(obj = parse_this(str, "[the] %l")))
        {
            notify_fail("Choose a more appropriate target.\n");
            return 0;
        }

        ob = obj[0];
    }
    /* if the player doesn't specify a target, we assume the player's
     * current enemy.
     */
    else if (!(ob = this_player()->query_attack()))
    {
        notify_fail("But you aren't in battle with anyone!\n");
        return 0;
    }

    /* Does attacker have the skill? */
    if (!(this_player()->query_skill(SS_GUILD_SPECIAL_SKILL)))
    {
        write("First you should learn how.\n");
        return 1;
    }
 
    /* Is the attacker already busy? */
    if (this_player()->query_punch())
    {
        write("You are already preparing such an attack.\n");
        return 1;
    }
 
    /* Are we even allowed to attack it?  Use can_attack_with_occ_special(),
     * which is defined in the guild shadow, to find out.
     */
    if (stringp(how = this_player()->can_attack_with_occ_special(ob, "punch")))
    {
        write(how);
        return 1;
    }
 
    /* The shadow takes care of the rest */
    this_player()->punch(ob);

    return 1;
}

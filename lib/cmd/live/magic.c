/*
 * /cmd/live/magic.c
 *
 * General magic-related commands
 *
 * - abort
 * - cast
 * - spells
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types
#pragma no_shadow

inherit "/cmd/std/command_driver";

nomask mapping
query_cmdlist()
{
    return ([
	        "abort"  : "abort",
                "cast"   : "cast",
	        "spells" : "spells",
           ]);
}

/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
string
get_soul_id()
{
    return "magic";
}

/* **************************************************************************
 * This is a command soul.
 */
int
query_cmd_soul()
{
    return 1;
}

/*
 * Function name: abort
 * Description:   The "abort" command to abort casting of a spell
 * Arguments:     string str - arguments to the "abort" command
 * Returns:       1/0 - command success/failure
 */
int
abort(string str)
{
    if (str != "spell")
    {
        notify_fail("Abort what?\n", 0);
        return 0;
    }

    if (this_player()->abort_spell())
    {
        return 1;
    }

    notify_fail("You aren't casting a spell, though.\n");
    return 0;
}
    
/*
 * Function name: cast
 * Description:   The "cast" command.
 * Arguments:     string str - arguments to "cast" command
 * Returns:       1/0 - spell command executed/not executed
 */
int
cast(string str)
{
    object ob;
    string spell, arg;

    if (!strlen(str))
    {
        notify_fail("Cast what?\n", 0);
        return 0;
    }

    if (!sscanf(str, "%s %s", spell, arg))
    {
        spell = str;
    }

    if (ob = this_player()->find_spell(spell))
    {    
        this_player()->start_spell(spell, arg, ob);
        return 1;
    }

    notify_fail("Cast what?\n", 0);
    return 0;
}

/*
 * spells - Show what spells we know
 */
/*
 * Function name: spells
 * Description:   List the active spells.
 */
int
spells(string str)
{
    object *spellobjs = this_player()->query_spellobjs();

    if (!sizeof(spellobjs))
    {
        write("You have no spells.\n");
        return 1;
    }

    map(spellobjs, &->list_spells());
    return 1;
}

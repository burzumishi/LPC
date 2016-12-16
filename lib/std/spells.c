/* 
 * /std/spells.c
 *
 * This is the standard spell object. It should be inherited into any
 * object that is supposed to define spells.
 *
 * Please read the documentation on spells if you haven't done so
 * already. (Use "man spells" in lpmud).
 *
 * User function in this object:
 * -----------------------------
 *
 * create_spells()	- The function that initates the object. Should be
 *			  used exactly as create_object() when you inherit
 *			  the "/std/object" file.
 *
 * add_spell()		- This function is called to add a spell to the 
 *			  list of spells a player has access to. You should
 *			  never used "add_action" for this purpose, since
 *			  that will void most of the built-in functionality
 *			  of spells in the player.
 *
 * remove_spell()	- This function si called to remove a spell from the
 * 			  list of spells a player has access to. It might be
 *			  that you want to give limited access of a spell
 *			  (for example a limited number of times it can be
 *			  cast per session) and then want to remove it after
 *			  a while.
 *
 * Typical usage in your spell object:
 *
 *   inherit "/std/spells";
 *
 *   void
 *   create_spells()
 *   {
 *	 set_name("book");
 *	 set_short("black book");
 *       add_spell("verb1", "function1", "name1");
 *       add_spell("verb2", "function2", "name2");
 *       .....
 *       add_spell("verbN", "functionN", "nameN");
 *   }
 * 
 *   int function1() { }
 *   int function2() { }
 *   .....
 *   int functionN() { }
 *
 * where functionX should return 1 for success, a fail-message or 0 to get
 * the standard fail-message
 */

#pragma save_binary
#pragma strict_types

inherit "/std/object";
inherit "/cmd/std/command_driver";

#include <std.h>

static string	*verb_list = ({ });	/* The list of spell verbs */
static string	*func_list = ({ });	/* The list of functions to call */
static string	*name_list = ({ });	/* The list of spell names */

void create_spells();

/*
 * Function name: create_object
 */
void
create_object()
{
    create_spells();
}

/*
 * Function name: add_spell
 * Description:   Add the spell to the list of active spells.
 * Arguments:	  verb - The activating verb string
 *		  function - The function to call
 *		  name - The name of the spell (to be displayed in
 *			 the player spell list).
 */
nomask void
add_spell(string verb, string func, string name)
{
    /* The spell was already added once */
    if (member_array(verb, verb_list) >= 0)
	return;

    SECURITY->add_spell(verb, func, this_object(), name);
    verb_list += ({ verb });
    func_list += ({ func });
    name_list += ({ name });

    update_commands();
}

/*
 * Function name: remove_spell
 * Description:   Remove the spell with the given verb string.
 * Arguments:	  verb - The verb of the spell to remove.
 */
nomask int
remove_spell(string verb)
{
    int index;

    index = member_array(verb, verb_list);

    if (index < 0)
	return 0;

    verb_list = exclude_array(verb_list, index, index);
    func_list = exclude_array(func_list, index, index);
    name_list = exclude_array(name_list, index, index);

    update_commands();
}

/*
 * Function name: list_spells
 * Description:   This function is called to list the spells from
 *		  the player soul when the command "spells" is typed.
 */
void
list_spells()
{
    int index = -1, size;
    string space;

    space = "                                                 ";
    size = sizeof(verb_list);
    while(++index < size)
    {
	write(verb_list[index] +
	    extract(space, 0, 17 - strlen(verb_list[index])) +
	    name_list[index] +
	    extract(space, 0, 31 - strlen(name_list[index])) + "\n");
    }
}

/*
 * Function name: create_spells
 * Description:   Default function
 */
public void
create_spells()
{
    set_name("book");
    set_short("spell book");
    set_long("A heavy book bearing the title 'Magica Arcana'.\n");
}

/*
 * Function name: query_cmdlist
 * Description:   Used by the command_driver, called each time the
 *		  command set is reloaded.
 */
mapping
query_cmdlist()
{
    if (sizeof(verb_list))
    {
	return mkmapping(verb_list, func_list);
    }
    else
    {
	return ([ ]);
    }
}

/*
 * Function name: query_verbs
 * Description:   Not used by command_driver, do we need it?
 */
string *
query_verbs()
{
    return verb_list;
}

/*
 * Function name: query_funcs
 * Description:   Not used by command_driver, do we need it?
 */ 
string *
query_funcs()
{
    return func_list;
}

/*
 * Function name: enter_env
 * Description:   Add the object to the list of spell objects.
 */
varargs void
enter_env(object dest, object old)
{
    if (!dest || !living(dest))
	return;

    dest->add_spellobj(this_object());
}

/*
 * Function name: leave_env
 * Description:   Remove the object from the list of spell objects
 */
varargs void
leave_env(object old, object dest)
{
    if (!old || !living(old))
	return;

    old->remove_spellobj(this_object());
}

/*
 * Function name: query_spell_time
 * Description:   How long time will it take to cast a spell?
 * Arguments:	  verb - The spell verb
 * Returns:       The time it will take. Real casting time will be time + 2 
 */
int query_spell_time(string verb) { return 0; }

/*
 * Function name: query_spell_mess
 * Description:   If you want own messages to be written when a spell is cast
 *		  instead of the standard messages you shuold define this
 *		  funktion.
 * Arguments:     verb - The spell verb
 * Returns:       1 if I took care of the messages
 */
int query_spell_mess(string verb) { return 0; }

/*
 * Function name:
 * Description:
 * Arguments:
 * Returns:
 */


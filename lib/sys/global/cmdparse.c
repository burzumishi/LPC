/*
  Standard commands

  This object gives you standard handling of commands.


  Available standard forms:

  "verb %i"              Normal access
  "verb %i %s %i"        Access in containers
  "verb %i %s %i"        Both %i normal access


  For details on %i and %s  see /doc/efun/parse_command
*/
#pragma save_binary

#include <cmdparse.h>
#include <composite.h>
#include <macros.h>
#include <std.h>
#include <stdproperties.h>

#define PARSE_ENV (objectp(environment(this_player()))	\
		   ? environment(this_player()) : this_player())

/*
 * Prototypes
 */
static mixed *order_num(mixed *arr, int num);
nomask int filter_first(object ob);

/* Global variables. */
static int gNum;
static int gParseCommandSize = 0;
static mixed *gContainers;
static string *gParalyzeCommands = CMDPARSE_PARALYZE_ALLOWED;

/*
 * Function name: visible_access
 * Description  : Provides a numerical context to a selection by a player.
 *                This is especially made for the results of parse_command().
 * Arguments    : mixed *arr - an array with as first element the number that
 *                    designates the objects to match and further the items
 *                    to test against.
 *                string ascfunc - the function to filter with.
 *                object ascobj - the object to filter with
 *                int normflag - if true, exclude this_player() from the results.
 *                int include_invis - if true, invis objects are matched.
 * Returns      : object * - the objects that matched, or ({ }).
 */
varargs mixed *
visible_access(mixed *arr, string acsfunc, object acsobj, int normflag,
	       int include_invis)
{
    int num;
    object *invenv;
    object *items;
    
    /* Access failure. */
    if (!pointerp(arr) || !sizeof(arr) || !this_player())
	return  ({ });

    /* The number designation. */
    num = arr[0];

    /* Sort items by short description. */
    items = COMPOSITE_SORT(arr[1..], "short");
    items = filter(items, objectp);
    if (!sizeof(items))
    {
        return ({ });
    }

    if (acsfunc)
    {
	items = filter(items, acsfunc, acsobj);
    }
    else
    {
        /* Items must be in the inventory or environment of the player. */
        invenv = all_inventory(this_player());
        if (environment(this_player()))
        {
            invenv += all_inventory(environment(this_player()));
        }
        items &= invenv;
    }

    if (!include_invis)
        items = filter(items, &->check_seen(this_player()));

    /* Most often we do not want to return this_player() */
    if (normflag)
    {
        items -= ({ this_player() });
    }

    /* All items required, or none found. */
    if (!num || !sizeof(items))
	return items;

    /* Select a certain item, eg. first or third */
    if (num < 0)               
	return order_num(items, -num);

    /* Select a number of items, eg. one or three */
    /* Get the first so many we want */
    gNum = num;
    return filter(items, filter_first);
}

/*
 * Function name: normal_access
 * Description  : See visible_access(), but with normflag = 1 set.
 */
varargs mixed *
normal_access(mixed *arr, string acsfunc, object acsobj, int include_invis)
{
    return visible_access(arr, acsfunc, acsobj, 1, include_invis);
}

/*
 * Function name: parse_command_access
 * Description  : Basically a combination of parse_command and NORMAL_ACCESS
 * Arguments    : string str - the string to parse.
 *                mixed env - the items to look through. if a singular object,
 *                    it will take env + the deep inventory of env. If an array
 *                    of objects, it will match only against those objects. If
 *                    not given, use inventory + environment of this_player().
 *                string pattern - the pattern to parse against.
 * Returns      : object * - the matching objects, ({ }) or 0.
 */
object *
parse_command_access(string str, mixed invenv, string pattern)
{
    object *oblist;

    if (!strlen(str) || !strlen(pattern))
    {
        return 0;
    }
    /* In case no object was given, then by default take the inventory and
     * environment of this_player().
     */
    if (!invenv)
    {
        if (!this_player())
        {
            return 0;
        }
        invenv = all_inventory(this_player());
        if (environment(this_player()))
        {
            invenv += all_inventory(environment(this_player()));
        }
    }
    if (parse_command(str, invenv, pattern, oblist))
    {
        return NORMAL_ACCESS(oblist, 0, 0);
    }
    else
    {
        return 0;
    }
}

/*
 * Function name: parse_command_one
 * Description  : Basically a combination of parse_command and NORMAL_ACCESS
 *                and returns only an object if exactly one was found. If more
 *                are found it also returns 0. Use PARSE_COMMAND_SIZE to find
 *                out how many were meant.
 * Arguments    : string str - the string to parse.
 *                mixed env - the items to look through. if a singular object,
 *                    it will take env + the deep inventory of env. If an array
 *                    of objects, it will match only against those objects.
 *                string pattern - the pattern to parse against.
 * Returns      : object the matching object - or 0.
 */
object
parse_command_one(string str, mixed env, string pattern)
{
    object *oblist = parse_command_access(str, env, pattern);

    if ((gParseCommandSize = sizeof(oblist)) == 1)
    {
        return oblist[0];
    }
    else
    {
        return 0;
    }
}

/*
 * Function name: parse_command_size
 * Description  : When using parse_command_one() and it fails, use this
 *                routine to find out how many were actually found.
 * Returns      : int - the number of items actually found.
 */
int
parse_command_size()
{
    return gParseCommandSize;
}

/*
 * Function name: filter_first
 * Description  : Support function to fund 'gNum' items. It will reduce the
 *                global variable 'gNum' with the number of items found until
 *                there are none left.
 * Arguments    : object ob - the object to test.
 * Returns      : int - if true, the object is selected.
 */
nomask int 
filter_first(object ob)
{
    if (gNum < 1)
	return 0;
    else if (ob->query_prop(HEAP_I_IS)) 
	gNum -= ob->split_heap(gNum);
    else
	gNum -= 1;
    return 1;
}

static mixed *
order_num(mixed *objs, int num)
{
    int cnt = 1;

    foreach(mixed obj: objs)
    {
	if (obj->query_prop(HEAP_I_IS))
	{
	    cnt += obj->num_heap();
	    if (cnt > num)
	    {
		obj->split_heap(1);
		return ({ obj });
	    }
	}
	else
	    cnt++;
	if (cnt > num)
	    return ({ obj });
    }
    return ({ });
}

/* General command routine for "verb %i"

   Access objects inside player or in players environment.
   Parses command and calls 'single_func' for each object to do the command
   upon. Returns an array of these objects for which 'single_func' returned 1.
*/
varargs object *
do_verb_1obj(string cmd, string single_func, string acsfunc, object cobj,
	     int include_invis)
{
    mixed * itema;
    object call_obj;
    
    if (!cmd)
	return 0;
    if (!cobj)
	call_obj = previous_object();
    else
	call_obj = cobj;
    
    if (!parse_command(cmd, PARSE_ENV, "[the] %i", itema))
	return 0;
    
    itema = normal_access(itema, acsfunc, cobj, include_invis);
    if (!itema)
	return 0;
        
    return filter(itema, single_func, call_obj);
}

/* General command routine for "verb %i %s %i" (typical : get from )

   Access objects located inside containers.
   Access containers inside player or in players environment.
   Calls 'prep_func' for acknowledge of word returned by %w
   
   Parses command and calls 'single_func' for each object to do the command
   upon. Returns array of these objects for which 'single_func' returned 1,
*/

varargs object *
do_verb_inside(string cmd, string prep_func, string single_func,
	       string afunc, mixed cobj)
{
    mixed *itema1, *itema2;
    object call_obj;
    string str2;
    
    if (!cmd)
	return 0;
    if (!cobj)
	call_obj = previous_object();
    else
	call_obj = cobj;
    
    if (!parse_command(cmd, PARSE_ENV, "[the] %i %w [the] %i", itema1, str2, itema2))
	return 0;
    
    if (!call_other(call_obj, prep_func, str2))
	return 0;
    
    itema2 = normal_access(itema2, afunc, cobj);  
    if (!itema2)
	return 0;
    
    gContainers = itema2;
    
    itema1 = normal_access(itema1, "in_containers", this_object());
    if (!itema1)
	return 0;
    
    return filter(itema1, single_func, call_obj);
}

/*
 * Function name: in_containers
 * Description:   test if object is in one of a set of containers
 * Arguments:	  ob: object
 * Returns:       1: is in one of the conatiners
                  0: not in any of the containers
 * globals        gContainers: the array of containers
 */
int
in_containers(object ob) 
{
    if (!objectp(ob))
	return 0;
    if (!environment(ob))
	return 0;

    return IN_ARRAY(environment(ob), gContainers);
}

/* General command routine for "verb %i %w %i" (typical hit xx with yy )

   Access objects inside player or in players environment.
   Access a single object inside player or in players environment.
   Calls 'check_func' for acknowledge of word returned by %w and second %i
   
   Parses command and calls 'single_func' for each object to do the command
   upon. Returns an array of these objects for which 'single_func' returned 1.
*/

do_verb_with(string cmd, string check_func, string single_func,
	     string acsfunc1, string acsfunc2, mixed cobj)
{
    mixed *itema1, *itema2;
    object call_obj;
    string str2;
    
    if (!cmd)
	return 0;
    if (!cobj)
	call_obj = previous_object();
    else
	call_obj = cobj;
    
    if (!parse_command(cmd, PARSE_ENV, "[the] %i %w [the] %i", itema1, str2, itema2))
	return 0;
    
    itema2 = normal_access(itema2, acsfunc2, cobj);  
    if (!itema2)
	return 0;
    
    gContainers = itema2;
    
    itema1 = normal_access(itema1, acsfunc1, cobj);
    if (!itema1)
	return 0;
    
    if (!call_other(call_obj, check_func, str2, itema2))
	return 0;
    
    return filter(itema1, single_func, call_obj);
}

/*
 * Function name: find_str_in_arr
 * Description:   Will return the array of objects corresponding to the str 
 *		  that this_player can see, picked from the given array.
 * Arguments:	  str - the string to search for
 *		  ob  - An array with objects to look through
 * Returns:	  the object array.
 */
object *
find_str_in_arr(string str, object *ob)
{
    int num;
    object *items;
    mixed *arr;

    if (!str)
	return ({});

    if (parse_command(str, ob, "[the] %i", arr))
    {
        /* Numeric designation */
	num = arr[0];

        items = COMPOSITE_SORT(arr[1..], "short");
 	items = filter(items, &->check_seen(this_player()));

	if (sizeof(items))
	{
            /* Select all items */
	    if (num == 0)
		return items;

	    /* Select a certain item, eg. first or third */
	    if (num < 0)               
		return order_num(items, -num);

	    /* Select a number of items, eg. one or three */
	    /* Get the first so many we want */
	    gNum = num;
	    return filter(items, filter_first);
	}
    }

    return ({});
}

/*
 * Function name: find_str_in_object
 * Description:   Will search through an object and try to find objects matching
 *		  the given string.
 * Arguments:	  str - The string to look for
 *		  ob  - The object to look in
 * Returns:	  An array with corresponding objects, empty if no match
 */
object *
find_str_in_object(string str, object ob)
{
    object *arr, obj;

    arr = find_str_in_arr(str, all_inventory(ob));

    /* Fall back if we cannot find it the official way. */
    if (!sizeof(arr) && (obj = present(str, ob)) && CAN_SEE(this_player(), obj))
	return ({ obj });

    return filter(arr, objectp);
}

/*
 * Parses a string on the form:
 *
 *    <prep> <item> <prep> <item> <prep> <item> ....
 *
 * item can be a subitem, sublocation or a normal object.
 *
 * Returns an array with four elements:
 *
 * ret[0]		 The prepositions 
 * ret[1]		 The items, a normal parse_command %i return value 
 *			 or a string (subitem/sublocation)
 * ret[2]		 True if last was not a normal object 
 * ret[3]		 True if no normal objects 
 *
 */
mixed *
parse_itemlist(string str)
{
    string    	*preps, *trypreps, itemstr, nrest, rest;
    mixed	*itemlists, *itemlist;
    int		last_sub, only_sub, bef;

    preps = ({});
    itemlists = ({});
    last_sub = 0;
    only_sub = 1;
    rest = str;

    trypreps = SECURITY->parse_command_prepos_list();

    trypreps += ({ "at", "of", "prp_examine" });

    while (strlen(rest))
    {
	notify_fail(break_string("Sorry, In '" + str + "'. The ending: '" + 
				 rest + "', not understood.\n", 76));
	if (!parse_command(rest, ({}), "%p %s", trypreps, nrest))
	    return 0;
	if (!strlen(nrest))
	    return 0;
	
	preps += ({ trypreps[0] });
	
	if (!parse_command(nrest, PARSE_ENV, "[the] %i %s", itemlist, rest))
	{
	    if (!parse_command(nrest, ({}), "%s %p %s", itemstr, 
			       trypreps, rest)) 
	    {
		/* Subitem or sublocation in the room */
		itemlists += ({ nrest });
		last_sub = 1;
		break;
	    }
	    else /* subitem or sublocation with preposition after */
	    {
		itemlists += ({ itemstr });
		rest = trypreps[0] + (strlen(rest) ? " " + rest : "");
		last_sub = 1;
	    }
	    bef = sizeof(itemlists) - 2;
	    
	    /* Two following subitem/sublocations are combined to one */
	    if (bef >= 0 && stringp(itemlists[bef]))
	    {
		itemlists[bef] += " " + preps[bef + 1] + " " +
		    itemlists[bef + 1];
		preps = preps[0..bef];
		itemlists = itemlists[0..bef];
	    }
	}
	else /* Normal item */
	{
	    itemlists += ({ itemlist });
	    last_sub = 0;
	    only_sub = 0;
	}
    }
    return ({ preps, itemlists, last_sub, only_sub });
}

/*
 * Function name: paralyze_allow_cmds
 * Description  : Called to register commands that should be allowed even
 *                while the player is paralyzed. This may only be information
 *                commands and must not be any sort of action, activity or
 *                communication.
 * Arguments    : mixed - a single (string) command, or an array of commands.
 */
void
paralyze_allow_cmds(mixed cmds)
{
    /* Allow a single string command. */
    if (stringp(cmds))
    {
        cmds = ({ cmds });
    }
    /* Otherwise it must be an array. */
    if (!pointerp(cmds))
    {
        return;
    }

    gParalyzeCommands |= cmds;
}

/*
 * Function name: paralyze_cmd_is_allowed
 * Description  : Find out whether a certain command is allowed while the
 *                player is paralyzed.
 * Arguments    : string cmd - the command to check.
 * Returns      : int - if true, it is allowed.
 */
int
paralyze_cmd_is_allowed(string cmd)
{
    return IN_ARRAY(cmd, gParalyzeCommands);
}

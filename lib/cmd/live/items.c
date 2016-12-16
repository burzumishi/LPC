/*
 * /cmd/live/items.c
 *
 * General commands for manipulating items.
 *
 * - drink
 * - eat
 * - extinguish
 * - hold
 * - light
 * - (m)read
 * - release
 * - remove
 * - unwield
 * - wear
 * - wield
 *
 * Except where noted, these commands can be used on any item in the
 * player's possession which correctly defines the "use function"
 * called by a particular command.  For instance, "read" can be used
 * on any item which correctly implements a command_read() function.
 * These "use functions" should return a string value (error message)
 * on failure and 1 on success (see the /std modules for which these
 * commands were implemented for examples.)
 *
 * Commands and "use functions":
 * 
 * Command       Function                Remark
 * =========================================================================
 *   drink       command_drink()
 *   eat         command_eat()
 * + extinguish  command_extinguish()
 *   hold        command_hold()
 * + light       command_light()
 * + (m)read     command_read(int more)  If "more" is true, the item should be
 *                                       read using more().
 *   release     command_release()
 *   remove      command_remove()
 *   unwield     command_unwield()
 *   wear        command_wear()
 *   wield       command_wield()
 * =========================================================================
 * + These commands can be used on items in the living's environment as well.
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/cmd/std/command_driver";

#include <cmdparse.h>
#include <files.h>
#include <composite.h>
#include <language.h>
#include <macros.h>

/* Use an item in the actor's inventory */
#define USE_INV(str, func, silent, one) \
    use_described_items(str, all_inventory(this_player()), func, silent, one)

/* Use an item in the actor's inventory or environment */
#define USE_ENV(str, func, silent, one) \
    use_described_items(str, all_inventory(this_player()) + \
    all_inventory(environment(this_player())), func, silent, one)

/*
 * Use the #if 0 trick to fool the document maker. This way, we can here define
 * a description of all the functions this soul may call in items.
 */

#if 0
/*
 * Function name: command_drink
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that is drinkable. It is by default
 *                built into /std/drink.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully imbided.
 */
public mixed
command_drink()
{
}

/*
 * Function name: command_eat
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that is edible. It is by default
 *                built into /std/food.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully eaten.
 */
public mixed
command_eat()
{
}

/*
 * Function name: command_extinguish
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that contains light and can be
 *                extinguished. It is by default built into /std/torch.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully extinguished.
 */
public mixed
command_extinguish()
{
}

/*
 * Function name: command_hold
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that can be held. It is by default
 *                built into /lib/holdable_item.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully eaten.
 */
public mixed
command_hold()
{
}

/*
 * Function name: command_light
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that can contain light and can be
 *                lit. It is by default built into /std/torch.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully lit.
 */
public mixed
command_light()
{
}

/*
 * Function name: command_read
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that can be read.
 * Arguments    : int more - if true, the command should read using more.
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully read.
 */
public mixed
command_read(int more)
{
}

/*
 * Function name: command_release
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that can be held and thus released.
 *                It is by default built into /lib/holdable_item.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully released.
 */
public mixed
command_release()
{
}

/*
 * Function name: command_remove
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that can be worn and thus removed.
 *                It is by default built into /lib/wearble_item.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully removed.
 */
public mixed
command_remove()
{
}

/*
 * Function name: command_unwield
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that can be wielded and thus
 *                unwielded. It is by default built into /std/weapon.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully unwield.
 */
public mixed
command_unwield()
{
}

/*
 * Function name: command_wear
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that can be worn. It is by default
 *                built into /lib/wearble_item.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully worn.
 */
public mixed
command_wear()
{
}

/*
 * Function name: command_wield
 * Description  : This is a pseudo function, used to fool the document maker.
 *                Define this in an object that can be wielded. It is by
 *                default built into /std/weapon.c
 * Returns      : string - an error message upon failure.
 *                int 1  - when successfully wielded.
 */
public mixed
command_wield()
{
}
#endif /* 0 */

/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
string
get_soul_id()
{
    return "items";
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
 * Function name: using_soul
 * Description:   Called once by the living object using this soul. Adds
 *                sublocations responsible for extra descriptions of the
 *                living object.
 */
public void 
using_soul(object live)
{
}

/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
mapping
query_cmdlist()
{
    return
	([
             "drink"      : "drink",
             "eat"        : "eat",
             "extinguish" : "extinguish",
             "hold"       : "hold",
             "light"      : "light",
             "read"       : "read",
             "mread"      : "read",
             "release"    : "release",
             "remove"     : "remove",
             "unwield"    : "unwield",
  	     "wear"       : "wear",
             "wield"      : "wield",
	]);
}

/*
 * Function name: use_items
 * Description:   Cause the actor to "use" the given items
 * Arguments:     object *items - the items to use
 *                function f - the function to call in the objects found to
 *                             use them.
 *                int silent - suppress the default message given when items
 *                             are successfully used.
 * Returns:       object * - an array consisting of the objects that were
 *                           successfully used
 */
varargs public object *
use_items(object *items, function f, int silent)
{
    mixed res;
    object *used;
    string fail_msg;
    int fail, i;

    for (i = 0, used = ({}), fail_msg = ""; i < sizeof(items); i++)
    {
        /* Call the function to "use" the item */
        res = f(items[i]);
 
        if (!res)
	{
	    /* The item cannot be used in this way */
            continue;
	}

        if (stringp(res))
	{
	    /* The attempt to use the item failed */
            fail = 1;
	    fail_msg += res;
	}
        else
	{
	    /* The item was successfully used */
	    used += ({ items[i] });
	}
    }
        
    if (!sizeof(used))
    {
        /* Nothing could be used.  Say why. */

        if (!fail)
	{
            notify_fail("You cannot seem to " + query_verb() + 
                ((sizeof(items) > 1) ? " those" : " that") + ".\n");
            return 0;
	}

        write(fail_msg);
        return ({});
    }

    if (!silent)
    {
    	write("You " + query_verb() + " " + COMPOSITE_ALL_DEAD(used) + 
            ".\n");
    	say(QCTNAME(this_player()) + " " + LANG_PWORD(query_verb()) + " " +
            QCOMPDEAD + ".\n");
    }

    return used;
}
    
/*
 * Function name: use_described_items
 * Description:   Given a string, cause the actor to "use" the items described
 *                by the string.
 * Arguments:     string str  - the string describing what to use
 *                object *obs - the items to be matched with the string 
 *                function f  - the function to call in the objects found to
 *                              use them.
 *                int silent  - suppress the default message given when items are
 *                              successfully used.
 *                int use_one - Allow only one item to be used
 * Returns:       0 - No items found that matched the string describer
 *                object * - an array consisting of the objects that were
 *                           successfully used
 */
varargs public object *
use_described_items(string str, object *obs, function f, int silent,
    int use_one)
{
    mixed *items;

    if (!strlen(str) ||
        !parse_command(str, obs, "[the] %i", items) ||
        !sizeof(items = NORMAL_ACCESS(items, 0, 0)))
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
        return 0;
    }

    if (use_one && (sizeof(items) > 1))
    {
        notify_fail("Be more specific!  You can only " + query_verb() +
            " one item at a time.\n");
        return 0;
    }

    return use_items(items, f, silent);
}

public int
drink(string str)
{
    object *drinkable;

    if (!(drinkable = USE_INV(str, &->command_drink(), 0, 0)))
    {
        return 0;
    }

    /* Remove the drinks after we are through */
    drinkable->remove_drink();

    return 1;
}

public int
eat(string str)
{
    object *eatable;

    if (!(eatable = USE_INV(str, &->command_eat(), 0, 0)))
    {
        return 0;
    }

    /* Remove the food after we are through */
    eatable->remove_food();

    return 1;
}

public int
extinguish(string str)
{
    return !!USE_ENV(str, &->command_extinguish(), 0, 0);
}

public int
hold(string str)
{
    return !!USE_INV(str, &->command_hold(), 1, 0);
}

public int
light(string str)
{
    return !!USE_ENV(str, &->command_light(), 0, 0);
}

public int
read(string str)
{
    return !!USE_ENV(str, &->command_read(FCHAR(query_verb()) == "m"), 1, 1);
}

public int
release(string str)
{
    mixed *items;

    if (!strlen(str) || !parse_command(str, all_inventory(this_player()), 
        "[the] %i", items))
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
	return 0;
    }

    items = CMDPARSE_STD->normal_access(items, 0, 0, 1);

    return !!use_items(items, &->command_release(), 1);
}

public int
remove(string str)
{
    mixed *items;
    object *inv;

    if (!strlen(str))
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
	return 0;
    }

    inv = this_player()->query_clothing(-1);
    inv += all_inventory(this_player()) - inv;

    if (!parse_command(str, inv, "[the] %i", items))
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
	return 0;
    }

    items = CMDPARSE_STD->normal_access(items, 0, 0, 1);

    return !!use_items(items, &->command_remove(), 1);
}

public int
unwield(string str)
{
    mixed *items;
    object *inv;

    if (!strlen(str))
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
	return 0;
    }

    inv = this_player()->query_weapon(-1);
    inv += all_inventory(this_player()) - inv;

    if (!parse_command(str, inv, "[the] %i", items))
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
	return 0;
    }

    items = CMDPARSE_STD->normal_access(items, 0, 0, 1);

    return !!use_items(items, &->command_unwield(), 1);
}

public int
wear(string str)
{
    return !!USE_INV(str, &->command_wear(), 1, 0);
}

public int
wield(string str)
{
    return !!USE_INV(str, &->command_wield(), 1, 0);
}

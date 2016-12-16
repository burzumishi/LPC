/*
 * /lib/holdable_item.c
 *
 * This library allows you to make an object 'holdable'. When a player holds
 * an item, it will occupy a toolslot and therewith render it useless for
 * combat. Holding can also be used to make an item special, like a holy bowl
 * the player needs to hold in both hands to receive the nectar of Gods.
 *
 * NOTICE
 *
 * When you use this module you MUST also define the function leave_env() and
 * call holdable_item_leave_env() from it as follows. The same goes for
 * defining appraise_object() and calling appraise_holdable_item() as follows:
 *
 * void
 * leave_env(object from, object to)
 * {
 *      ::leave_env(from, to);
 *
 *      holdable_item_leave_env(from, to);
 * }
 *
 * void
 * appraise_object(int num)
 * {
 *     ::appraise_object(num);
 *
 *     appraise_holdable_item();
 * }
 *
 * If you omit this, the object will not be released automatically when the
 * player drops it, for instance. Also, players will not be able to properly
 * appraise this item.
 */

#pragma no_clone
#pragma save_binary
#pragma strict_types

#include <language.h>
#include <macros.h>
#include <wa_types.h>

/*
 * Global variables.
 */
public static int held_in_hand;
public static int slots;
public static object holder;

/*
 * Function name: set_slots
 * Description  : With this function you can set the tool slots that this
 *                item will occupy when held.
 * Arguments    : int - the slots, W_RIGHT, W_BOTH, et cetera.
 */
public void
set_slots(int which)
{
    if (!this_object()->query_lock())
    {
        slots = which;
    }
}

/*
 * Function name: query_slots_setting
 * Description  : This will return exactly what was set with set_slots().
 * Returns      : int - the value set with set_slots().
 */
public int
query_slots_setting()
{
    return slots;
}

/*
 * Function name: query_slots
 * Description  : When held, this will return an array with the actual slot
 *                numbers used for this item.
 * Returns      : int * - the actual slots, i.e. ({ TS_LEFT, TS_RIGHT });
 */
public int *
query_slots()
{
    int index;
    int *slot_arr = ({});

    for (index = 2; index <= held_in_hand; index <<= 1)
    {
        if (index & held_in_hand)
		{
            slot_arr += ({ index });
		}
    }

    return slot_arr;
}

#if 0
/*
 * Function name: hold
 * Description  : Called when the person wants to hold an item. This function
 *                may allow or disallow the object to be held, and it may
 *                print its own messages.
 * Returns      : string / int -
 *                 0 - The item can be held, print default messages.
 *                 1 - The item can be held, print no messages.
 *                     Messages were printed within the function.
 *                -1 - The item cannot be held. Print default messages.
 *                string - The item cannot be held. The string contains the
 *                         fail message.
 */
public mixed
hold()
{
    return 0;
}

/*
 * Function name: release
 * Description  : Called when the person wants to release the item. This
 *                function may allow or disallow the object to be released,
 *                and it may print its own messages.
 * Returns      : string / int -
 *                 0 - The item can be relased, print default messages.
 *                 1 - The item can be relased, print no messages.
 *                     Messages were printed within the function.
 *                -1 - The item cannot be released. Print default messages.
 *                string - The item cannot be relased. The string contains the
 *                         fail message.
 */
public mixed
release()
{
    return 0;
}
#endif

/*
 * Function name: query_slot_desc
 * Description:   Give a description of the slots this item occupies.
 * Arguments:     object for_obj - whom the description is for
 * Returns:       the (string) slot description.
 */
varargs public string
query_slot_desc(object for_obj)
{
    string possessive;

    possessive = (!for_obj ? "the" : ((for_obj == holder) ? "your" :
	holder->query_possessive()));

    switch ((holder ? held_in_hand : slots))
    {
    case W_RIGHT:
	return "in " + possessive + " right hand";

    case W_LEFT:
	return "in " + possessive + " left hand";

    case W_BOTH:
        return "in both hands";

    case W_ANYH:
	return "in either hand";

    case W_FOOTR:
        return "on " + possessive + " right foot";

    case W_FOOTL:
        return "on " + possessive + " left foot";

    case W_FOOTR | W_FOOTL:
	return "on both " + possessive + " feet";
    }

    /* Should never happen. */
    return "somewhere";
}
	    
/*
 * Function name: query_hold_desc
 * Description  : Returns the string describing how the player is holding
 *                this item. It assumes that the object is held indeed.
 *                Example return string: "an oil lamp in his right hand"
 * Arguments:     object for_obj - whom the description is for
 * Returns      : string - the description.
 */
varargs public string
query_hold_desc(object for_obj)
{
    string str;

    if (CAN_SEE(this_player(), this_object()))
    {
	str = LANG_ADDART(this_object()->short(for_obj)) + " ";
    }
    else
    {
	str = "something ";
    }

    return str + query_slot_desc(for_obj);
}

/*
 * Function name: command_hold
 * Description  : Called when someone wants to hold this item. It will make
 *                some checks and actually hold it when they all work out.
 * Returns      : string / int - string fail message or 1 when succesful.
 */
nomask mixed
command_hold()
{
    string str;
    mixed fail;
    mixed hret;

    if (holder)
    {
        return "You are already holding the " + this_object()->short() + ".\n";
    }

    if (this_player() != environment())
    {
        return "You must carry the " + this_object()->short() + 
            " to be able to hold it.\n";
    }

    if (slots == W_ANYH)
    {
        if (this_player()->query_tool(TS_RWEAPON))
	{ 
            held_in_hand = TS_LWEAPON;
	}
        else
	{
            held_in_hand = TS_RWEAPON;
	}
    }
    else
    {
        held_in_hand = slots;
    }

    /* can the weapon be held? */
    if (stringp(fail = this_player()->hold(this_object())))
    {
        return fail;
    }

    holder = this_player();

    /* Check for a hold function */
    if (!(hret = this_object()->hold()))
    {
        write("You hold the " + this_object()->short() + 
	    " " + query_slot_desc(this_player()) + ".\n");
        str = " holds " + LANG_ADDART(this_object()->short()) + ".\n";
        say( ({ this_player()->query_Met_name() + str, 
                "The " + this_player()->query_nonmet_name() + str,
                "" }) );
    }

    if (stringp(hret))
    {
	holder = 0;
        this_player()->release(this_object());
        return hret;
    }

    if (hret < 0)
    {
	holder = 0;
        this_player()->release(this_object());
        return "You cannot hold the " + this_object()->short() + ".\n";
    }

    this_object()->set_adj("held");
    this_object()->remove_adj("unheld");

    return 1;
}

/*
 * Function name: command_release
 * Description  : Function is called when the player wants to release the
 *                item held. It will make some checks and actually release
 *                the item when succesful.
 * Returns      : string / int - string fail message, or 1 when successful.
 */
nomask mixed
command_release()
{
    string str;
    mixed hret;

    if (holder != this_player())
    {
        return "You are not holding the " + this_object()->short() + ".\n";
    }

    if (!(hret = this_object()->release()))
    {
	write("You release the " + this_object()->short() + ".\n");
	str = " releases " + LANG_ADDART(this_object()->short()) + ".\n";
	say( ({ this_player()->query_Met_name() + str, 
		"The " + this_player()->query_nonmet_name() + str,
		"" }) );
    }

    if (stringp(hret))
    {
        return hret;
    }

    if (hret < 0)
    {
        return "You cannot release the " + this_object()->short() + ".\n";
    }

    this_player()->release(this_object());

    holder = 0;
    held_in_hand = 0;

    this_object()->remove_adj("held");
    this_object()->add_adj("unheld");

    return 1;
}

/*
 * Function name: query_held
 * Description  : Returns whether this object is being held or not.
 * Returns      : int 1/0 - held or not.
 */
public nomask int
query_held()
{
    return !!holder;
}

/*
 * Function name: query_holder
 * Description  : Returns the person holding this object, if it is held.
 * Returns      : object - the person holding this item, else 0.
 */
public nomask object
query_holder()
{
    return holder;
}

/*
 * Function name: query_holdable_item
 * Description  : Function to identify this object as being holdable.
 * Returns      : int 1 - always.
 */
public nomask int
query_holdable_item()
{
    return 1;
}

/*
 * Function name: holdable_item_leave_env
 * Description:   This MUST be called from leave_env(). Failing to do
 *                means that this holdable is not properly released.
 * Arguments:     object env - the environment being left
 *                object to  - the environment being entered
 */
public void
holdable_item_leave_env(object env, object to)
{
    mixed hret;

    if (!holder)
    {
        return;
    }

    if (!(hret = this_object()->release()))
    {
        tell_object(holder, "You release the " + this_object()->short() + ".\n");
    }

    this_player()->release(this_object());

    this_object()->remove_adj("held");
    this_object()->add_adj("unheld");
    holder = 0;
    held_in_hand = 0;
}

#if 0
/*
 * Function name: leave_env
 * Description  : When you inherit /lib/holdable_item.c you MUST also define
 *                the function leave_env() as copy of this function. It MUST
 *                make the call to holdable_item_leave_env(from, to) as well.
 *                If you omit this, then the item will not be automatically
 *                released when the player drops it, for instance.
 *                PS: This is just a dummy function. It does not exist. You
 *                    must write your own as copy of this one.
 * Arguments    : object from - the environment we are leaving.
 *                object to   - the environment we are entering.
 */
public void
leave_env(object from, object to)
{
    ::leave_env(from, to);

    holdable_item_leave_env(from, to);
}
#endif

/*
 * Function name: holdable_item_usage_desc
 * Description:   Return a string describing how the item is held
 */
public string
holdable_item_usage_desc()
{
    return ("The " + this_object()->short(this_player()) +
	" can be held " + query_slot_desc() + ".\n");
}    

/*
 * Function name: appraise_holdable_item
 * Description  : This function will print how this wearable is worn. You
 *                must call this function from your redefinition of the
 *                appraise_object() function in your wearable.
 */
void
appraise_holdable_item()
{
    write(holdable_item_usage_desc());
}

#if 0
/*
 * Function name: appraise_object
 * Description  : When you inherit /lib/holdable_item.c you MUST also define
 *                the function appraise_object() as copy of this function. It
 *                MUST make the call to appraise_holdable_item() as well. If
 *                you omit this, then the player will not be able to appraise
 *                this item properly.
 *                PS: This is just a dummy function. It does not exist. You
 *                    must write your own as copy of this one.
 * Arguments    : int num - the semi-randomised appraise skill.
 */
void
appraise_object(int num)
{
    ::appraise_object(num);

    appraise_holdable_item();
}
#endif

/*
 * Function name: query_attack_blocked
 * Description:   Check to see if the item armour blocks attacks for the
 *                slots it is held in (if any are valid attacks).
 * Arguments:     int aid - the attack id being checked.
 * Returns:       1/0 - attack blocked/not blocked
 */
public int
query_attack_blocked(int aid)
{
    /* All attacks with occupied slots are blocked */
    return 1;
}

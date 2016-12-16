/*
 * /lib/wearable_item.c
 *
 * This module should be inherited in all wearable items. It is by default.
 * inherited into /std/armour.c, though can also be inherited by other objects,
 * like /std/object.c, to make a non-armour wearable, i.e. non-protective
 * clothing, rings, a brooch, et cetera.
 *
 * NOTICE
 *
 * When you use this module in any object other than /std/armour.c you MUST
 * also define the function leave_env() and call wearable_item_leave_env()
 * from it as follows. The same goes for defining appraise_object() and
 * calling appraise_wearable_item() as follows:
 *
 * void
 * leave_env(object from, object to)
 * {
 *      ::leave_env(from, to);
 *
 *      wearable_item_leave_env(object from, object to);
 * }
 *
 * void
 * appraise_object(int num)
 * {
 *     ::appraise_object(num);
 *
 *     appraise_wearable_item();
 * }
 *
 * If you omit this, the object will not be removed automatically when the
 * player drops it, for instance. Also, players will not be able to properly
 * appraise this item.
 */

#include <composite.h>
#include <macros.h>
#include <stdproperties.h>
#include <wa_types.h>

static int looseness, layers;

/* Do not change these variable names.  They may be used by /std/armour.c or
 * objects that inherit it.
 */
static object wearer, wear_func;
static int worn, arm_at, worn_on_part;

public void set_wf(object ob);

/* 
 * Function name: set_slots
 * Description:   Indicate which armour slots this item occupies.
 * Arguments:     int i - the armour slots (A_CHEST, A_LEGS, etc.)
 */
public void
set_slots(int i)
{
    if (!this_object()->query_lock())
    {
        arm_at = i;
    }
}

/*
 * Function name: query_slots_setting
 * Returns:       The (int) armour slots that this item occupies
 */
public int
query_slots_setting()
{
    return arm_at;
}

/*
 * Function name: query_slots
 * Description  : Get the armour locations this armour occupies, or can
 *                occupy.
 * Arguments    : int check - optional argument, if true, use the
 *                    slot-setting, else use the currently used slots.
 * Returns      : int * - an array composed of the individual armour slots
 *                    this item occupies / can occupy.
 */
varargs public int *
query_slots(int check)
{
    int abit, *slots;

    check = (check ? arm_at : worn_on_part);

    /* If slots is negative, it means the A_ANY_* setting, so we return
     * just that as it can be either slot.
     */
    if (check < 0)
    {
        return ({ check });
    }

    /* Start at 2 (1 is A_MAGIC) */
    slots = ({ });
    for (abit = 2; abit <= check; abit *= 2)
    {
        if (check & abit)
        {
            slots = slots + ({ abit });
        }
    }

    return slots;
}

/*
 * Function name: set_layers
 * Description:   Indicate how many layers or how thick this item
 *                is.
 * Arguments:     int l - the number of layers
 */
public void
set_layers(int l)
{
    layers = l;
}

/*
 * Function name: query_layers
 * Returns:       The (int) number of layers for this item
 */
public int
query_layers()
{
    return layers;
}

/*
 * Function name: set_looseness
 * Description:   Indicate how loosely this item fits (i.e. how
 *                many layers can be worn under it).
 * Arguments:     int l - the looseness
 */
public void
set_looseness(int l)
{
    looseness = l;
}

/*
 * Function name: query_looseness
 * Returns:       The looseness of this item
 */
public int
query_looseness()
{
     return looseness;
}

/*
 * Function name: config_wearable_item
 * Description:   Configure a wearable item, setting all the major
 *                attributes.
 * Arguments:     int slot  - the armour slots occupied by the item (see
 *                            set_slots())
 *                int la    - the number of layers for the item (see
 *                            set_layers())
 *                int lo    - the looseness of the item (see set_looseness())
 *                object wf - the object that defines the wear/remove functions
 *                            for the item (see set_wf())
 */
varargs public void
config_wearable_item(int slot, int la, int lo, object wf)
{
    set_slots(slot);
    set_layers(la);
    set_looseness(lo);
    set_wf(wf);
}

/*
 * Function name: query_wearable_item
 * Description  : Function to identify this object as being wearable.
 * Returns      : int 1 - always.
 */
public nomask int
query_wearable_item()
{
    return 1;
}

/*
 * Function name: wear_how
 * Description:   Generate the string that is shown when the armour is worn
 * Arguments:     location: the location(s) covered by the armour
 */
public string
wear_how(int location)
{
    string how, pos;

    if (!wearer)
    {
        pos = "the";
    }
    else if (this_player() == wearer)
    {
        pos = "your";
    }
    else
    {
        pos = wearer->query_possessive();
    }

    switch (location)
    {
        case A_CHEST:
            how = " on " + pos + " chest"; break;
        case A_TORSO:
            how = " over " + pos + " shoulders"; break;
        case A_HEAD:
            how = " on " + pos + " head"; break;
        case A_LEGS:
            how = " on " + pos + " legs"; break;
        case W_RIGHT:
        case A_R_ARM:
            how = " on " + pos + " right arm"; break;
        case W_LEFT:
        case A_L_ARM:
            how = " on " + pos + " left arm"; break;
        case A_ANY_ARM:
        case A_SHIELD:
            how = " on " + pos + " arm of choice"; break;
        case A_FEET:
            how = " on " + pos + " feet"; break;
        case A_ROBE:
            how = " around " + pos + " body"; break;
        case A_NECK:
            how = " around " + pos + " neck"; break;
        case A_WAIST:
            how = " around " + pos + " waist"; break;
        case A_R_FINGER:
            how = " on " + pos + " right ring finger"; break;
        case A_L_FINGER:
            how = " on " + pos + " left ring finger"; break;
        case A_ANY_FINGER:
            how = " on " + pos + " ring finger of choice"; break;
        case A_HANDS:
            how = " on both hands"; break;
        case A_R_HAND:
            how = " on " + pos + " right hand"; break;
        case A_L_HAND:
            how = " on " + pos + " left hand"; break;
        case A_ANY_HAND:
            how = " on " + pos + " hand of choice"; break;
        case A_R_FOOT:
            how = " on " + pos + " right foot"; break;
        case A_L_FOOT:
            how = " on " + pos + " left foot"; break;
        case A_ANY_FOOT:
            how = " on " + pos + " foot of choice"; break;
        case A_ARMS:
            how = " on both arms"; break;
        case A_BACK:
            how = " on " + pos + " back"; break;
        case A_WRISTS:
            how = " on " + pos + " wrists"; break;
        case A_L_WRIST:
            how = " on " + pos + " left wrist"; break;
        case A_R_WRIST:
            how = " on " + pos + " right wrist"; break;
        case A_ANY_WRIST:
            how = " on " + pos + " wrist of choice"; break;
        case A_ANKLES:
            how = " on " + pos + " ankles"; break;
        case A_L_ANKLE:
            how = " on " + pos + " left ankle"; break;
        case A_R_ANKLE:
            how = " on " + pos + " right ankle"; break;
        case A_ANY_ANKLE:
            how = " on " + pos + " ankle of choice"; break;
        case A_EARS:
            how = " on " + pos + " ears"; break;
        case A_L_EAR:
            how = " on " + pos + " left ear"; break;
        case A_R_EAR:
            how = " on " + pos + " right ear"; break;
        case A_HIPS:
            how = " on " + pos + " hips"; break;
        case A_L_HIP:
            how = " on " + pos + " left hip"; break;
        case A_R_HIP:
            how = " on " + pos + " right hip"; break;
        case A_ANY_HIP:
            how = " on " + pos + " hip of choice"; break;
        case A_SHOULDERS:
            how = " on " + pos + " shoulders"; break;
        case A_L_SHOULDER:
            how = " on " + pos + " left shoulder"; break;
        case A_R_SHOULDER:
            how = " on " + pos + " right shoulder"; break;
        case A_ANY_SHOULDER:
            how = " on " + pos + " shoulder of choice"; break;
        case A_BROW:
            how = " on " + pos + " brow"; break;
        case A_EYES:
            how = " over " + pos + " eyes"; break;
        default: 
            how = "";
    }

    return how;
}

static mixed
do_wear_item()
{
    return wearer->wear_clothing(this_object());
}

static void
do_remove_item()
{
    wearer->remove_clothing(this_object());
}

static mixed
check_slot(int slot)
{
    int i, layers, flag;
    object *worn_arr;

    /* For each slot, we check to see if the item can fit over
     * the other items already worn there.
     */
    layers = flag = 0;
    worn_arr = this_player()->query_clothing(slot);

    if (sizeof(worn_arr) > 6)
    {
        return "You cannot fit anything else" + wear_how(slot) + ".\n";
    }

    for (i = sizeof(worn_arr) - 1; i >= 0; i--)
    {
        layers += worn_arr[i]->query_layers();
        if (layers > looseness)
        {
            /* The item cannot fit over the other items already worn */
            return "The " + this_object()->short() + " won't fit over the " + 
                worn_arr[sizeof(worn_arr) - 1]->short() + ".\n";
        }
    }

    return 1;
}

/*
 * Function name: wear_me
 * Description:   wear this item
 * Returns:       string - error message (failure)
 *                1 - success
 */
public mixed
wear_me()
{
    string what, how, error_msg;
    mixed wfail;
    int *slots, i;

    what = this_object()->short();
    if (worn)
    {
        return "You already wear the " + what + ".\n";
    }

    if (this_object()->query_prop(OBJ_I_BROKEN))
    {
        return "You can't wear broken items.\n";
    }

    worn_on_part = ABS(arm_at);
    slots = query_slots(0);
    wearer = this_player();

    error_msg = "";

    while (i < sizeof(slots))
    {
        if (stringp(wfail = check_slot(slots[i])))
	{
            if (arm_at >= 0)
	    {
                return wfail;
	    }

            error_msg += wfail;
	}
        else
	{
            if (arm_at < 0)
	    {
                worn_on_part = slots[i];
                break;
	    }
	}
        
        if ((++i == sizeof(slots)) && strlen(error_msg))
	{
            return error_msg;
	}
    }

    if (stringp(wfail = do_wear_item()))
    {
        return wfail;
    }

    /*
     * A wear function in another object.
     */
    if (!wear_func || !(wfail = wear_func->wear(this_object())))
    {
        how = wear_how(worn_on_part);
        write("You wear the " + what + how + ".\n");
        say(QCTNAME(this_player()) + " wears the " + what + ".\n");
    }

    /*
     * If the wearfunc returned a value < 0 then we can not wear the armour
     */
    if (intp(wfail) && wfail >= 0)
    {
        this_object()->set_adj("worn");
        this_object()->remove_adj("unworn");
        worn = 1;
        return 1;
    } 

    do_remove_item();

    this_object()->remove_adj("worn");
    this_object()->add_adj("unworn");
    wearer = 0;
    worn = 0;

    if (stringp(wfail))
    {
        return wfail;
    }

    return "You cannot wear the " + 
        this_object()->short(this_player()) + ".\n";
}

/*
 * Function name: command_wear
 * Description:   Called to wear this item
 * Returns:       see wield_me()
 */
public mixed
command_wear()
{
    return wear_me();
}

/*
 * Function name: remove_me
 * Description:   remove this item
 * Returns:       1 - success
 *                0 - armour not worn
 *                string - an error message (failure)
 */
public mixed
remove_me()
{
    mixed wret;

    if (!worn || !wearer)
    {
        return 0;
    }

    /*
     * A remove function in another object.
     */
    if (!wear_func || !(wret = wear_func->remove(this_object())))
    {
        if (CAN_SEE(this_player(), this_object()))
        {
            write("You remove the " + this_object()->short() + ".\n");
        }
        else
        {
           write("You remove something.\n");
        }
     
        say(QCTNAME(this_player()) + " removes " +
            this_player()->query_possessive() + " " +
            QSHORT(this_object()) + ".\n");
    }

    if (intp(wret) && (wret >= 0))
    {
        do_remove_item();
        this_object()->remove_adj("worn");
        this_object()->add_adj("unworn");
        wearer = 0;
        worn = 0;

        return 1;
    }

    return (stringp(wret) ? wret : "");
}

/*
 * Function name: command_remove
 * Description:   Called to wear this item
 * Returns:       see remove_me()
 */
public mixed
command_remove()
{
    return remove_me();
}

/*
 * Function name: set_wf
 * Description:   Designate an object that defines the wear() and remove()
 *                functions for this item.  Those functions can return
 *                the following:
 *		  0 - No affect the item can be worn / removed
 *		  1 - It can be worn / removed but no text should be printed
 *		      (it was done in the function)
 *		 -1 - It can not be worn / removed default failmsg will be 
 *		      written
 *           string - It can not be worn / removed 'string' is the 
 *		      fail message to print
 */
public void
set_wf(object ob)
{
    if (!this_object()->query_lock())
    {
        wear_func = ob;
    }
}

/*
 * Function name: query_wf
 * Returns:       The object that defines the wear() and remove() functions
 *                for this item
 */
public object
query_wf()
{
    return wear_func;
}

#if 0
/*
 * Function name: wear
 * Description  : This function might be called when someone tries to wear
 *                this armour. To have it called, use set_wf().
 * Arguments    : object obj - The armour we want to wear.
 * Returns      : int  0 - The armour can be worn normally.
 *                     1 - The armour can be worn, but print no messages.
 *                    -1 - The armour can't be worn, use default messages.
 *                string - The armour can't be worn, use this message.
 */
public mixed
wear(object obj)
{
    return 0;
}

/*
 * Function name: remove
 * Description  : This function might be called when someone tries to remove
 *                this armour. To have it called, use set_wf().
 * Arguments    : object obj - The armour to remove.
 * Returns      : int  0 - Remove the armour normally.
 *                     1 - Remove the armour, but print no messages.
 *                    -1 - Do not remove the armour, print default message.
 *                string - Do not remove the armour, use this message.
 */
mixed
remove(object obj)
{
    return 0;
}
#endif

/*
 * Function name: wearable_item_usage_desc
 * Description  : This function returns the usage of this wearable. It is
 *                usually printed from the appraise function. The string
 *                includes the location where it should be worn.
 * Returns      : string - the description.
 */
string
wearable_item_usage_desc()
{
    string *parts;

    parts = map(query_slots(1), &extract(, 1) @ wear_how);

    return ("The " + this_object()->short(this_player()) +
        " is made to be worn " + COMPOSITE_WORDS(parts) + ".\n");
}

/*
 * Function name: appraise_wearable_item
 * Description  : This function will print how this wearable is worn. You
 *                must call this function from your redefinition of the
 *                appraise_object() function in your wearable.
 */
void
appraise_wearable_item()
{
    write(wearable_item_usage_desc());
}

#if 0
/*
 * Function name: appraise_object
 * Description  : When you inherit /lib/wearable_item.c into any object
 *                other than /std/armour.c you MUST also define the function
 *                appraise_object() as copy of this function. It MUST make
 *                the call to appraise_wearable_item() as well. If you omit
 *                this, then the player will not be able to appraise this
 *                item properly.
 *                PS: This is just a dummy function. It does not exist. You
 *                    must write your own as copy of this one.
 * Arguments    : int num - the semi-randomised appraise skill.
 */
void
appraise_object(int num)
{
    ::appraise_object(num);

    appraise_wearable_item();
}
#endif

/*
 * Function name: wearable_item_leave_env
 * Description:   Make sure that the item is removed when it is
 *                moved.  This should normally be called from the
 *                leave_env() function of the inheriting object.
 * Arguments:     object from - the environment left
 *                object to   - the environment entered
 */
public void
wearable_item_leave_env(object from, object to)
{
    if (!worn)
    {
        return;
    }

    if (!wear_func || !wear_func->remove(this_object()) && wearer)
    {
        tell_object(wearer, 
            "You remove the " + this_object()->short() + ".\n");
    }

    do_remove_item();
    this_object()->remove_adj("worn");
    this_object()->add_adj("unworn");
    wearer = 0;
    worn = 0;
}

#if 0
/*
 * Function name: leave_env
 * Description  : When you inherit /lib/wearable_item.c into any object other
 *                than /std/armour.c you MUST also define the function
 *                leave_env() as copy of this function. It MUST make the
 *                call to wearable_item_leave_env(from, to) as well. If
 *                you omit this, then the item will not be automatically
 *                removed when the player drops it, for instance.
 *                PS: This is just a dummy function. It does not exist. You
 *                    must write your own as copy of this one.
 * Arguments    : object from - the environment we are leaving.
 *                object to   - the environment we are entering.
 */
public void
leave_env(object from, object to)
{
    ::leave_env(from, to);

    wearable_item_leave_env(from, to);
}
#endif

/*
 * Function name: query_worn
 * Description:   If this object is worn or not
 * Returns:       The object who wears this object if this object is worn
 */
public object
query_worn()
{
    return (worn ? wearer : 0);
}

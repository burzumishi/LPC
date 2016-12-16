/*
 * /doc/examples/obj/holy_symbol.c
 *
 * This item provides an example for creating a holdable object
 * as well as an object that provides magic protection.
 */

inherit "/std/object";
inherit "/lib/holdable_item";

#include <stdproperties.h>
#include <wa_types.h>

public void
create_object()
{
    set_name("symbol");
    set_adj(({ "gold", "holy" }));

    set_long("A small, golden holy symbol.\n");

    add_prop(OBJ_I_WEIGHT, 1500);
    add_prop(OBJ_I_VOLUME,  500);
    add_prop(OBJ_I_VALUE, 3 * 12 * 12 * 12);

    /* Indicate the type and strength of the magic */
    add_prop(MAGIC_AM_MAGIC, ({ "enchantment", 20 }));

    /* Allow the magical properties to be identified by mortals */
    add_prop(MAGIC_AM_ID_INFO, ({ 
        "The holy symbol is enchanted with life magic.\n", 10,
        "It benefits those of good heart with protection " +
	"from death magic.\n", 25 }));

    /* Wizard information about the item */
    add_prop(OBJ_S_WIZINFO, "This object gives 20 levels of " +
        "MAGIC_I_RES_DEATH to non-undead holders with " +
        "alignment > 500.\n");

    /* The holy symbol can be held in either hand */
    set_slots(W_ANYH);
}

/*
 * Function name: hold
 * Description:   This function is called to see if the item can be held.
 * Returns:       see /lib/holdable_item.c:hold
 */
public mixed
hold()
{
    /* Check to see if the holder is able to hold the item */
    if ((this_player()->query_alignment() > 500) &&
        !this_player()->query_prop(LIVE_I_UNDEAD))
    {
        write("As you hold the " + short() + " in your hand, " +
            "a strange warmth pervades your body.\n");

        /* Add the magic effect (resistance) to the holder */
        this_player()->add_magic_effect(this_object());

        /* Returning 1 indicates that the item can be held, but
         * that default messages should be supressed (we've already
         * given our own message).
         */
        return 1;
    }

    write("You feel the " + short() + " resist your grasp.\n");

    /* Returning -1 indicates that the item cannot be held and
     * to print the default failure message.
     */
    return -1;
}

/*
 * Function name: release
 * Description:   This function is called to see if the item can be released.
 * Returns:       see /lib/holdable_item.c:release
 */
public mixed
release()
{
    /* Remove the magic effect from the holder */
    this_player()->remove_magic_effect(this_object());

    /* Returning 0 indicates that the item can be released normally */
    return 0;
}

/*
 * Function name: query_magic_protection
 * Description:   Indicate how much magic resistance this object gives
 * Arugments:     see /std/object.c:query_magic_protection
 * Returns:       see /std/object.c:query_magic_protection
 */
varargs public mixed
query_magic_protection(string type, object protectee = previous_object())
{
    if ((protectee == holder) && (type == MAGIC_I_RES_DEATH))
    {
        /* We give 20 levels of additive resistance to death magic */
        return ({ 20, 1 });
    }

    return ::query_magic_protection(type, protectee);
}

/* Function name: leave_env
 * Description:   This function is called when the item moves from
 *                one environment to another.
 * Arguments:     object env - the environment being left
 *                object to  - the environment being entered
 */
public void
leave_env(object env, object to)
{
    ::leave_env(env, to);

    /* Call this to make sure that the item is released if it is
     * being held.
     */
    holdable_item_leave_env(env, to);
}

/*
 * Function name: appraise_object
 * Description:   This is called when someone appraises this item.
 *                We want to mask it to add a message telling the
 *                appraiser that the item can be held.
 */
public void
appraise_object(int num)
{
    ::appraise_object(num);
    write(holdable_item_usage_desc());
}

/* /doc/examples/wear/hat.c
 *
 * A simple example of a (non-armour) wearable item.
 */

inherit "/std/object";
/* Inherit /lib/wearable_item to make the item wearable */
inherit "/lib/wearable_item";

/* wa_types.h contains armour slot definitions like A_ANY_ANKLE */
#include <wa_types.h>
#include <stdproperties.h>

public nomask void
create_object()
{
    set_name("hat");

    /* The hat is worn on the head */
    set_slots(A_HEAD);

    /* The hat takes up two layers of space on the head.  That means
     * an item must have a looseness of at least two in order to be
     * worn over this hat.
     */
    set_layers(2);

    /* Only one layer of clothing can be worn under the hat. */
    set_looseness(1);

    add_prop(OBJ_I_WEIGHT, 250);
    add_prop(OBJ_I_VOLUME, 150);
    add_prop(OBJ_I_VALUE,   50);
}

/*
 * We need to override leave_env() so that we can be sure to
 * remove the hat if it gets moved from the wearer while it
 * is still worn.
 */
public void
leave_env(object env, object to)
{
    ::leave_env(env, to);
    wearable_item_leave_env(env, to);
}

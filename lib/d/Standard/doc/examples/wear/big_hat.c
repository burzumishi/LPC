/* /doc/examples/wear/big_hat.c
 * 
 * A simple example of a (non-armour) wearable item.
 * This shows a wearable item that can be worn over
 * another (/doc/examples/wear/hat.c).
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
    set_adj("big");

    /* This is a shorter way to do
     *
     *     set_slots(A_HEAD);
     *     set_layers(2);
     *     set_looseness(3);
     *
     * This hat takes up two layers of space on the head and
     * is large enough to be worn over three layers of clothing.
     */
    config_wearable_item(A_HEAD, 2, 3);

    add_prop(OBJ_I_WEIGHT, 275);
    add_prop(OBJ_I_VOLUME, 200);
    add_prop(OBJ_I_VALUE,   60);
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

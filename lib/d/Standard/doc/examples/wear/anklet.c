/* /doc/examples/wear/anklet.c
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
    set_name("anklet");

    /* The anklet can be worn on either ankle */
    set_slots(A_ANY_ANKLE);

    /* We won't make the anklet take up any space on the ankle.
     * That way, we can wear several and not have to worry about
     * not being able to wear clothing or armour over them.
     */
    set_layers(0);

    /* We allow no layers to be worn under the anklet */
    set_looseness(0);

    add_prop(OBJ_I_WEIGHT, 100);
    add_prop(OBJ_I_VOLUME,   50);
    add_prop(OBJ_I_VALUE,  50);
}

/*
 * We need to override leave_env() so that we can be sure to
 * remove the anklet if it gets moved from the wearer while it
 * is still worn.
 */
public void
leave_env(object env, object to)
{
    ::leave_env(env, to);
    wearable_item_leave_env(env, to);
}

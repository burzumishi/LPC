/*
 * An example bag, made by Nick
 *
 * This bag is based on the receptacle, which means that you can open and
 * close it. For a chest or safe you might also add a lock to it.
 *
 * /Mercade
 */

inherit "/std/receptacle";

#include <stdproperties.h>

void
create_container()
{
    set_name("bag");
    set_adj("small");
    set_long("A small leather bag.\n");

    add_prop(CONT_I_WEIGHT, 250); 	/* It weights 250 grams */
    add_prop(CONT_I_MAX_WEIGHT, 2250); 	/* It can hold 2000 grams of weight. */
    add_prop(CONT_I_VOLUME, 30); 	/* Only 30 ml volume (very small) */
    add_prop(CONT_I_MAX_VOLUME, 2030); 	/* 2 litres of volume */

    add_prop(OBJ_I_VALUE, 40); 		/* Worth 40 cc */
}

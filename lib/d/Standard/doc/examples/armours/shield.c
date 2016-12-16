/* An armour should always inherit /std/armour.c */
inherit "/std/armour";

#include <wa_types.h>      /* contains weapon/armour related definitions */
#include <stdproperties.h> /* contains standard properties */

void
create_armour()
{
    /* Set the name, short description and long description */
    set_name("shield");
    set_long("It looks very fragile.\n");

    /* Now we want to set the armour class */
    set_ac(4);

    /* Set the modifiers for each damage type */
    set_am(({ -2, 1, 1 }));  /* ac 2 vs impale damage, 
                              * ac 3 vs slash damage, 
                              * ac 3 vs bludgeon damage
                              */

    /* The shield breaks easily */
    set_likely_break(20);

    /* We also need to set what type of armour this is */
    set_at(A_SHIELD);

    /* Set the weight, volume and value */
    add_prop(OBJ_I_WEIGHT, 1000);
    add_prop(OBJ_I_VOLUME, 500);
    add_prop(OBJ_I_VALUE,  300);
}

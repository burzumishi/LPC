/* An armour should always inherit /std/armour.c */
inherit "/std/armour";

#include <wa_types.h>      /* contains weapon/armour related definitions */
#include <stdproperties.h> /* contains standard properties */

void
create_armour()
{
    /* Set the name, short description and long description */
    set_name("helmet");
    set_short("small helmet"); /* Observe, not 'a small helmet' */
    set_long("It's small but would probably protect you a little.\n");

    /* Now, a player can refere to this armour as 'armour' and 'helmet'. To
     * distinguish it from other helmets, we want the player to be able to 
     * use 'small helmet' as an id too.
     */
    set_adj("small");

    /* Now we want to set the armour */
    set_ac(4);

    /* Set the modifiers for each damage type */
    set_am(({ -2, 1, 1 }));  /* ac 2 vs impale damage, 
                              * ac 3 vs slash damage, 
                              * ac 3 vs bludgeon damage
                              */

    /* We also need to set what type of armour this is */
    set_at(A_HEAD); /* It protects the head */

    /* Set the weight, volume and value */
    add_prop(OBJ_I_WEIGHT, 800);
    add_prop(OBJ_I_VOLUME, 800);
    add_prop(OBJ_I_VALUE,  100);
}


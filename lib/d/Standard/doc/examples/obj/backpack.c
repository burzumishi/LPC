inherit "/d/Standard/std/pack";
inherit "/lib/keep"; 

#include <stdproperties.h>

void
create_pack()
{
    set_name("backpack");
    add_adj("leather");
    set_long("A leather pack which can be worn on the back.\n");

    set_wearable(1); 
    set_mass_storage(1); 

    set_keep(1); 

    add_prop(CONT_I_WEIGHT, 4500);
    add_prop(CONT_I_MAX_WEIGHT, 150000);
    add_prop(CONT_I_VOLUME, 8000);
    add_prop(CONT_I_MAX_VOLUME, 150000);

    add_prop(OBJ_I_VALUE, 450);
}

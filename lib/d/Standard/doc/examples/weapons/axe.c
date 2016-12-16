/*
 * /d/Standard/doc/examples/weapons/axe.c
 *
 * This is an axe.
 *
 * /Mercade 29 july 1993
 */

inherit "/std/weapon";

#include "/sys/stdproperties.h"
#include "/sys/wa_types.h"
#include "/sys/formulas.h"
#include "/sys/macros.h"
/*
 * If you do #include <macros.h> rather than #include "/sys/macros.h" the
 * command parser will look for the file macros.h in the appropriate
 * directory itself. This will only work with #include files that are in
 * /sys or /secure.
 */

#define WEAPON_HIT 23
#define WEAPON_PEN 29

void
create_weapon()
{
    set_name("axe");
    set_pname("axes");

    set_adj("iron");
    set_adj("broad-bladed");

    set_short("broad-bladed iron axe");
    set_pshort("broad-bladed iron axes");

    set_long(break_string("This axe is used by dwarves to slay their " +
        "enemies with. Its blade is broader than usual and made of iron. " +
        "Due to its broad blade, it will do more damage to your enemies " +
        "when you hit them.", 75) + "\n");

    set_hit(WEAPON_HIT);
    set_pen(WEAPON_PEN);
    set_wt(W_AXE);
    set_dt( (W_SLASH | W_BLUDGEON) );
    set_hands(W_ANYH);

    add_prop(OBJ_I_WEIGHT, 8000);
    add_prop(OBJ_I_VOLUME, 4000);
    add_prop(OBJ_I_VALUE, F_VALUE_WEAPON(WEAPON_HIT, WEAPON_PEN));
}

/*
 * In order to make your weapon recover through a reboot, just add the
 * following code to it and don't forget to #include <macros.h> like I
 * did on top.
 */

string
query_recover()
{
    return MASTER + ":" + query_wep_recover();
}

void
init_recover(string arg)
{
    init_wep_recover(arg);
}

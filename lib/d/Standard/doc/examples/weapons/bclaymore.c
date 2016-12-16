/*
 * /doc/examples/bclaymore.c
 *
 * A magical claymore.  It demonstrates how to create a simple
 * magical weapon and how to make restrictions on wielding a
 * weapon.
 *
 *   Copyright (C) Aridor 11/30/93
 *
 *   Revision history:
 *   Boron  05/30/96  : Added different stuff, pragma strict_types and 
 *                      remarks, updated it for the new mudlib!
 *   Shiva  11/28/98  : general cleanup
 */
#pragma strict_types

inherit "/std/weapon";

#include <ss_types.h>
#include <wa_types.h>
#include <stdproperties.h>
#include <tasks.h>
#include <formulas.h>

void
create_weapon()
{
    set_name("claymore");
    add_name("sword");

    set_adj("draconian");

    set_long("This weapon feels wonderfully balanced and is extremely sharp, " +
        "too! You can't help thinking unearthly forces must have been at " +
        "work when this weapon was created.\n");

    set_default_weapon(47, 38, W_SWORD, W_SLASH | W_IMPALE, W_BOTH);

    set_wf(this_object());

    add_prop(OBJ_I_IS_MAGIC_WEAPON, 1);

    add_prop(OBJ_S_WIZINFO,
        "The extremely well balanced weapon lets the wielder " +
	"feel the weapon as an extension of his arm. This magic " +
	"balance allows a very high hit rate, even though the hits " +
	"itself are not magically enhanced.\n");

    add_prop(MAGIC_AM_ID_INFO, ({
        "This weapon is magically enchanted.\n", 5,
	"You feel a strong will emanating from " +
	"the sword that you must overcome.\n", 25,
	"The weapon's balance is subject to the magic, " +
	"allowing better wieldability and a very high " +
	"hit rate.\n", 51 }));

    add_prop(MAGIC_AM_MAGIC, ({ 80, "enchantment" }));

    add_prop(OBJ_M_NO_BUY, 1);
    add_prop(OBJ_I_VOLUME, 3600);
    add_prop(OBJ_I_WEIGHT, 11500);
    add_prop(OBJ_I_VALUE, F_VALUE_WEAPON(49, 40) + 1000); /* it is magical! */
}

/*
 * Function name: wield
 * Description:   Sets up a nice wield sequence for the players.
 * Arguments:     wep -  the weapon the command is performed upon
 * Returns:       1 if wielded, 0 otherwise
 */
mixed
wield(object what)
{
    object wielder = this_player();

    /* Use a resolve_task to see if the player is able to wield the weapon.
     */
    if (wielder->resolve_task(TASK_DIFFICULT, ({ TS_WIS, TS_INT })))
    {
	wielder->catch_tell("You concentrate on the claymore for a second, " +
            "sinking your mind into the spirit of the sword.\n");
	return 0;
    }

    wielder->add_mana(-random(50) - 10);
    return "A sharp pain runs up your arm and into your head as you try, " +
      "but you are unable to wield the draconian claymore.\n";
}

/*
 * Function name: unwield
 * Description:   Sets up a unwield sequence for the players.
 *                (Not really needed here?)
 * Arguments:     wep -  the weapon the command is performed upon
 * Returns:       0 if unwielded, 1 unwield but no message
 *                -1 do not unwield, print normal message
 */
mixed
unwield(object what)
{
    return 0;
}

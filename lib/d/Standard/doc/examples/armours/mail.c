/*
 * /d/Standard/doc/examples/armours/mail.c
 *
 * This is an example of mail.
 *
 * /Mercade, 3 august 1993
 *
 * Updated 4 April 1997
 * /Shiva
 */
inherit "/std/armour";

#include <wa_types.h>      /* contains weapon/armour related definitions */
#include <stdproperties.h> /* contains standard properties */
#include <formulas.h>      /* contains various formulae */
#include <macros.h>        /* contains some handy macro definitions */

#define ARMOUR_CLASS 25

void
create_armour()
{
    set_name("mail");
    set_pname("mails");

    set_adj("augmented");
    set_adj("brass");

    set_short("brass augmented mail");
    set_pshort("brass mails");

    set_long("This mail is made of severel strips of brass " +
        "linked to eachother and partially overlapping eachother. Behind " +
        "the plates is layer of leather for comfort. On the mail you see " +
        "a crest painted: a blue star, indicating the tribe of the " +
        "original owner.\n");

    set_ac(ARMOUR_CLASS);
    set_at(A_BODY);
    set_am( ({ -2, -2, 4 }) );

    add_prop(OBJ_I_WEIGHT, 8400);
    add_prop(OBJ_I_VOLUME, 2500);
    add_prop(OBJ_I_VALUE, F_VALUE_ARMOUR(ARMOUR_CLASS));
}

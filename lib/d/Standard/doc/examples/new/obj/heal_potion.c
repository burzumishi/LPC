/*
 * HEAL_POTION
 * A Standard, run-of-the-mill healing potion
 * This potion heals both Hit-Points and fatigue,
 * using set_effect and add_effect.
 * by Asmodean 5/20/1995
 * Updated by Xarguul (formerly Asmo) on 5/22/1995
 * NOTE: The price in this file fully complies with the values
 * and formulas in 'man herbs', no matter how ridiculous this sum
 * may seem to some people.
 * DISCLAIMER: This is ONLY a demonstration potion!!
 */

inherit "/std/potion";

#include <ss_types.h>
#include <herb.h>
#include <macros.h>
#include <poison_types.h>

public void
create_potion()
{
    set_short("large red potion"); set_adj("red"); add_adj("large");
    set_unid_long("It is a large flask filled with red liquid.\n");
    set_potion_name("healing potion");
    set_id_long("This is a healing potion.\n");
    set_id_diff(10);
    set_soft_amount(10);
    set_alco_amount(1);
    set_potion_value(42000);
    set_id_smell("The potion smells somewhat spicy.\n");
    set_id_taste("The potion tastes somewhat spicy.\n");
    set_unid_smell("Hmm. You don't smell anything.\n");
    set_unid_taste("Hmm. Tastes rather bland.\n");
    set_effect(HERB_HEALING,"hp",400);
    add_effect(HERB_HEALING,"fatigue",15);
}










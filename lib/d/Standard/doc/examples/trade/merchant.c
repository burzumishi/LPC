/*
 * /d/Standard/doc/examples/trade/merchant.c
 *
 * This is a wandering merchant who does not have a shop, but supposedly
 * sells and buys from his inventory. I explicitly say supposedly. There
 * are some advantages and disadvantages to having him buy/sell from his
 * inventory.
 *
 * Advantages:
 * - it is more realistic to have him put the stuff in his inventory for
 *   there is a maximum of what he can carry;
 * - when you kill him (if that is allowed), he has all his stuff on him,
 *   which is nice for the killer;
 *
 * Disadvantages:
 * - he cannot use any equipment himself, for he cannot distinguish between
 *   merchandize and his own stuff. He might even sell his own axe to his
 *   murderer :-(
 * - an npc is only capable of carrying a limited amount of stuff, which
 *   means that he cannot buy that much from you. This means that he can
 *   only have a limited shop-function.
 *
 * However, there is a solution that has the best of both worlds. We give
 * him a special bag that can carry a lot of stuff without him noticing
 * much of it. That way his equipment does not interfere with the
 * merchandize, but on the other hand, if you kill him, you get all the
 * stuff he wants to sell. If you intend to make the merchant immortal
 * by using the OBJ_M_NO_ATTACK property, I recommend you give him a
 * store-room that consists of a room somewhere, for that is slighly
 * easier. Just don't forget to put the name of the store-room in the
 * OBJ_S_WIZINFO.
 *
 * /Mercade, 7 January 1994
 *
 * If you are going to use this code for your objects (what examples are
 * meant for :-) ), please add a line in your comment header saying that
 * you did so.
 *
 * Revision history:
 */

inherit "/std/monster";
inherit "/lib/shop";

#define STORE "/d/Standard/doc/examples/trade/sack"
#define AXE   "/d/Standard/doc/examples/weapons/axe"
#define MAIL  "/d/Standard/doc/examples/armours/mail"

#include <composite.h>
#include <macros.h>
#include <money.h>
#include <ss_types.h>
#include <stdproperties.h>

/* these macros are used to give him a little random in his stats/skills */
#define BASE              80
#define COMPUTE_STAT      (((BASE * 4) / 3) + random(BASE / 2) + 1)
#define COMPUTE_SKILL     (MAX(COMPUTE_STAT, 100))

/* Global variable, will not be saved. */
static object store;

void
create_monster() 
{
    set_name("traw");
    add_name( ({ "owner", "shopkeeper", "keeper" }) );
    add_name("_wandering_merchant"); /* name to use with the sack/store. */
    set_race_name("dwarf");

    set_adj("proud");
    add_adj("rough-bearded");

    set_long(break_string("You are looking at a rough-bearded dwarf. He is " +
        "a wandering merchant who might have something interesting for " +
        "you to buy or maybe you have something interesting to sell him." +
        " Despite of his rough appearance he is a friendly dwarf and he "+
        "is willing to trade with you.", 75) + "\n");

    set_title("son or Alal, wandering merchant");
    set_gender(G_MALE);

    set_base_stat(SS_STR, COMPUTE_STAT + 25); /* a dwarf is strong */
    set_base_stat(SS_DEX, COMPUTE_STAT);
    set_base_stat(SS_CON, COMPUTE_STAT);
    set_base_stat(SS_WIS, COMPUTE_STAT);
    set_base_stat(SS_INT, COMPUTE_STAT);
    set_base_stat(SS_DIS, COMPUTE_STAT);

    set_alignment(500);

    set_skill(SS_BLIND_COMBAT, 100);
    set_skill(SS_WEP_AXE,      COMPUTE_SKILL);
    set_skill(SS_PARRY,        COMPUTE_SKILL);
    set_skill(SS_DEFENCE,      COMPUTE_SKILL);
    set_skill(SS_AWARENESS,    COMPUTE_SKILL);
    set_skill(SS_UNARM_COMBAT, COMPUTE_SKILL);

    add_prop(LIVE_I_ALWAYSKNOWN, 1);
    add_prop(OBJ_S_WIZINFO, break_string("This merchant carries a special " +
        "sack that is his store-room. Every object he buys ends up in this " +
        "sack and he sells what he has in the sack.", 75) + "\n");

    config_default_trade();
    set_money_give_max(1000);

    /* clone the store-sack into the merchant. */
    seteuid(getuid());
    store = clone_object(STORE);
    store->move(this_object(), 1);
    set_store_room(STORE);

    set_alarm(1.0, 0.0, "arm_shopkeeper");
}

void
arm_shopkeeper()
{
    clone_object(AXE)->move(this_object(), 1);
    clone_object(MAIL)->move(this_object(), 1);

    command("wield all");
    command("wear all");

    /* give him some money. */
    MONEY_MAKE_GC(random(10))->move(this_object(), 1);
    MONEY_MAKE_SC(random(20))->move(this_object(), 1);
    MONEY_MAKE_CC(random(30))->move(this_object(), 1);
}

void
init_living()
{
    ::init_living();

    init_shop();
}

/*
 * Since the normal shop assumes a store-room, we have to adjust some
 * of the hooks in order to give a better message.
 */

/*
 * Function name: shop_hook_sold_items
 * Description:   Hook that is called when player sold something
 * Arguments:	  item - The item array player sold
 * Returns:	  1
 */
int
shop_hook_sold_items(object *item)
{
    write(break_string("You sold " + COMPOSITE_DEAD(item) + ".\n", 75));
    say(QCTNAME(this_player()) + " sold " + QCOMPDEAD + ".\n");
    /* lets tell everyone we put the stuff in our sack. */
    tell_room(environment(), QCTNAME(this_object()) + " puts the things " +
        "in his sack.\n");
    return 1;
}

/*
 * Function name: shop_hook_bought_items
 * Description:   Called when player has bought something
 * Arguments:	  arr - The array of objects
 * Returns: 	  1
 */
int
shop_hook_bought_items(object *arr)
{
    /* lets tell everyone we took the stuff out of our sack. */
    tell_room(environment(), QCTNAME(this_object()) + " takes " +
        (sizeof(arr) <= 1 ? "something" : "some things") +
        " out of his sack.\n");
    write(break_string("You bought " + COMPOSITE_DEAD(arr) + ".\n", 75));
    say(QCTNAME(this_player()) + " bought " + QCOMPDEAD + ".\n");
    return 1;
}

/*
 * Function name: shop_hook_value_no_match
 * Description:   Called if there were no match with what the player asked
 *		  about
 * Arguments:     str - The string player asked about
 * Returns:	  0
 */
int
shop_hook_value_no_match(string str)
{
    /* we have a sack and not a store-room. */
    notify_fail("I have no '" + str + "' in my sack.\n");
}

/*
 * Function name: shop_hook_list_empty_store
 * Description:   If the storeroom is empty when the player wants a list
 *		  of its items
 * Arguments:	  str - If player specified string
 */
void
shop_hook_list_empty_store(string str)
{
    /* we do not have a store-room. */
    notify_fail("I have nothing to sell currently.\n");
}

/*
 * Function name: shop_hook_list_no_match
 * Description:   Called if player specified the list and no matching
 *		  objects where found
 * Arguments:	  str - The string he asked for
 * Returns:	  0
 */
int
shop_hook_list_no_match(string str)
{
    /* we have a sack and no store-room */
    notify_fail("I have no '" + str + "' in my sack.\n");
}

/*
 * This function is called from add_action(.., "read"), but since we
 * are not in a shop, we do not have a sign with instructions either.
 * This blocks that function.
 */
int
do_read(string str)
{
    return 0;
}

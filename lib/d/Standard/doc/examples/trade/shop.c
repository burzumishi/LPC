/*
 * /d/Standard/doc/examples/trade/shop.c
 *
 * This cave is the shop of the dwarven race guild. It is a simple
 * example of a shop.
 *
 * /Mercade, 7 January 1994
 *
 * If you are going to use this code for your objects (what examples are
 * meant for :-) ), please add a line in your comment header saying that
 * you did so.
 *
 * Revision history:
 */

inherit "/std/room";
inherit "/lib/shop";

#include <macros.h>
#include <stdproperties.h>

#define STORE  "/d/Standard/doc/examples/trade/store"
#define BSN(s) break_string(s, 75) + "\n" /* a handy macro. */

void
create_room()
{
    set_short("the supply store of the dwarven race guild");
    set_long(break_string("In this cave the shop of the dwarven race guild " +
        "is situated. Here you may buy useful weapons, armours or gear, " +
        "lamps and oil or whatever else is in stock. There is a counter " +
        "and a cabinet in the cave. The cabinet is used to stock the items " +
        "for sale. To the east you see the inn of the guild and to the " +
        "northeast is the exit to the central guild cave. There is a " +
        "poster with instructions on the wall", 75) + "\n");

    /*
     * The store-room does not need to be a real room, in this case, I
     * did just as if the stock was put into a cabinet. It uses VBFC to
     * let you take a look into the cabinet if you examine it.
     */
    add_item("cabinet", "@@long:" + STORE + "@@");

    add_item("counter", "It is a wooden counter.\n");
    add_item( ({ "door", "doors" }),
        BSN("The doors are made of the same wood as the rest of the " +
        "cabinet. They are secured with iron bands and closed shut!"));
    add_item( ({ "band", "iron band", "bands", "iron bands" }),
        BSN("Some iron bands are fastened to the cabinet and its doors to " +
        "make it secure and to make sure that no-one opens the cabinet " +
        "without the permission of the shopkeeper."));
    add_cmd_item( ({ "cabinet", "door", "doors" }),
        ({ "open", "enter", "unlock" }),
        BSN("The doors of the cabinet are closed and fermly locked. There " +
        "is no way you can open the doors so do not even think about " +
        "entering the cabinet."));

    /*
     * The shop-related initializations.
     */
    config_default_trade();
    set_money_give_max(1000);
    set_store_room(STORE);

    /*
     * Add an exit to the store-room, the first parameter is the path to
     * the store, the second command to enter it and the third parameter
     * is a VBFC call to block all mortals players from using the exit.
     */
    add_exit(STORE, "cabinet", "@@check_wizard");
}

void
init()
{
    ::init();

    /*
     * Do this to add the shop-commands to the player.
     */
    init_shop();
}

/*
 * We do not want mortal players to walk into the cabinet and help
 * themselves. However, wizards might need to go in there if something
 * is wrong. Therefore we block their exit.
 */
int
check_wizard()
{
    /* wizards may use the exit. */
    if (this_player()->query_wiz_level())
    {
        return 0;
    }

    /* however, mortal players cannot... */
    write("The cabinet is locked. You cannot get into it.\n");
    say(QCTNAME(this_player()) + " fails to enter the cabinet.\n");
    return 1;
}

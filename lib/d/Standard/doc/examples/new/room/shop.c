/* 
 * An example shop coded by Nick, some Ideas and part of code taken from 
 * Tricky's shop in hobbitown.
 *
 * This shop inherits /d/Genenesis/lib/shop as all shops in Genesis
 * should do. Currently that file is only a link to /lib/shop but one
 * day we will start calculating the prices on demand and supply. If
 * you aren't happy with the way sell, buy, value or list are defined,
 * feel free to change them as you like. The only functions that is
 * mandatory to use are query_sell_price() and query_buy_price(). You
 * are entitled to change the price somewhat but should at least use
 * those functions as base. 
 */

inherit "/std/room";
inherit "/d/Genesis/lib/shop";
#include <stdproperties.h>
#include "ex.h"

#define STORE_ROOM	EX_ROOM + "store"

/*
 * Function name:   create_room
 * Description:     Initialize this room
 */
void
create_room()
{
    config_default_trade();
    set_short("The local shop");
    set_long(
      	"You are in the local shop. Here you can sell and buy stuff.\n" +
      	"You can also use value or list commands, 'list armours',\n" +
      	"'list weapons', 'list <name>' and 'list' works too.\n" +
      	"There is also a small sign to read with more instructions.\n" +
	"West there is a bridge to the inn.\n" +
      	"");

    add_exit(STORE_ROOM, "north", "@@wiz_check");

    add_item("sign", "A nice looking sign for you to read.\n");

    add_prop(ROOM_I_INSIDE, 1);  /* This is a real room */

    set_store_room(STORE_ROOM);
}

/*
 * Function name:   init
 * Description:     Is called for each living that enters this room
 */
void
init()
{
    ::init();   /* You MUST do this in a room-init */
    init_shop(); /* Get the commands from the shop */
}

/*
 * Example inn with the new pub.c
 */
inherit "/std/room";
inherit "/lib/pub";

#include <stdproperties.h>
#include <macros.h>
#include <money.h>

/*
 * Function name: create_room
 * Description:   Set up default trade and cofigure it if wanted.
 */
void
create_room()
{
    set_short("The Inn of Last Home");
    set_long(break_string(
	"You are in the Inn of Last Home. There is a menu on the bar. " +
	"You find this place rather cozy with a lot of tables and chairs " +
	"and a warm fireplace. You can smell the beer from the kegs " +
	"and the food from the kitchen." +
	"", 75) + "\n");
    
    add_item("bar", "A nice looking wooden bar with a menu on it.\n");
    add_item("menu", "It has a lot of readable words on it.\n");
    add_item("chairs",
	"They are made out of wood as is the rest of the place.\n"); 
    add_item("tables", "They are darkened with spilled beer.\n");
    add_cmd_item("menu", "read", "@@read");

    add_prop(ROOM_I_INSIDE,1);  /* This is a real room */

    add_drink( ({ "beer", "beers", "small", "small beer" }),
	"beer", "small", 100, 4, 12, 0, 0,
	"It's a small but refreshing beer.\n", 0);
    add_drink( ({ "wine", "wines", "sweet", "sweet wine" }),
	"wine", "sweet", 200, 20, 72, 0, 0,
	"It has a deep red colour and smells wonderful.\n", 0);
    add_food( ({ "apple", "apples" }),
	"apple", "red", 30, 25, 0, 0,
	"It looks very tasty.\n", 0);
    add_food( ({ "potato", "potato soup", "soup", "soups" }),
	"soup", "potato", 65, 80, "plate of potato soup", 
	"plates of potato soup",
	"It doesn't smell very good at all.\n", 0);
    add_food( ({ "potatoes", "spiced", "plate of spiced potatoes",
		 "plate", "plates" }),
	"potatoes", "spiced", 110, 110, "plate of spiced potatoes",
	"plates of spiced potatoes",
	"They look very tasty.\n", 0);
}

/*
 * Function name: init
 * Description:   Initalize the pub actions
 */
void
init()
{
    ::init(); /* Since it's a room this must be done. */

    init_pub();
}

/*
 * Function name: read
 * Description:   Read the menu
 * Arguments:     str - hopefully the menu
 */
int
read(string str)
{
    write("" +
	"   Small beer                 12 cc\n" +
	"   Some really sweet wine     72 cc\n" +
	"   An apple                   25 cc\n" +
	"   Potato  soup               80 cc\n" +
	"   Plate of spiced potatoes  210 cc\n\n" +
	"Try 'buy beer with gold' if you wanna specify what to\n" +
	"pay with, or 'test buy beer' to see what would happen\n" +
	"if you typed 'buy beer'. 'buy 10 beer' would get you\n" +
	"ten beers from the bar, but it takes a little while to\n" +
	"give them all to you.\n");
    return 1;
}


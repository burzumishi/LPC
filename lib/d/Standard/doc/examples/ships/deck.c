inherit "/d/Emerald/std/deck";

#include <stdproperties.h>

void
create_deck()
{
    set_short("On the ship");
    set_long(break_string("You're standing on a magnificent ship, watching "+
	     "its great sails catching the wind and bulging as they test "+
	     "themselves against the powerful forces of nature. The ship "+
	     "is all made of the finest oak, making it heavy and expensive "+
	     "but also extremely endurable and a fearsome opponent in "+
	     "battle, its weight accompanied by the enormous bronze battle "+
	     "ram in the bow adding up to something most captains would "+
	     "probably hesitate more than a moment before engaging into "+
	     "battle. On the lower deck, slaves specially chosen for their "+
	     "strength and endurance sit ready to put the 120 great oars "+
	     "into the water and start rowing at the captains command, "+
	     "giving the ship an unimaginable velocity despite its mass "+
	     "and the new bronze cannons, 45 on each side, stand secured "+
	     "behind their closed hatches. This ship is indeed one of the "+
	     "safest to travel with whether it comes to storm or pirates. "+
	     "A set of beautiful mahogny stairs lead to the ship's cabin.\n",76));
    add_item(({"mast", "masts"}), break_string("There are three great masts "+
	     "supporting the enormous sails, and you can see people high up "+
	     "climbing about.\n",76));
    add_item(({"sail", "sails"}), break_string("The sails are made out of "+
	     "thick, incredibly strong canvas, but you can't see that from "+
	     "down here really. What you see is that the sails are set and "+
	     "when a gust of wind fills one or several, you feel a rift going "+
	     "through all of the ship and notice the masts swaying slightly "+
	     "to the enormous pressure.\n",76));
    add_item(({"cannon", "cannons"}), break_string("The cannons are brand new "+
	     "and polished so that they shine like the sun when light falls "+
	     "upon them. You realize this is probably the best weaponry that "+
	     "money can buy. The craftsmanship is excellent.\n",76));
    add_exit("/d/Emerald/plugh/ship/cabin", "down", 0);
}

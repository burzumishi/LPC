inherit "/std/room";

#include <stdproperties.h>

int
number(string id_str)
{
    object *ob;
    int	   n, count;

    ob = all_inventory(this_object());
    for (n=0, count=0 ; n<sizeof(ob) ; n++)
	if (ob[n]->id(id_str))
	    count++;
    return count;
}

void
reset_room()
{
    object ob;
    int    count;

    count=number("slave");
    while (count<4) {
	if (count<2) {		/* Hack, too long evaluation */
	    count=2;
	    call_out("reset_room", 5);
	}
	ob=clone_object("/d/Emerald/plugh/ship/slave");
	if (ob)
	    ob->move(this_object());
	count++;
    }
    while (ob=present("oar")) {
	ob->remove_object();
	say("A slave eats a rowing oar raw.\n");
    }
}

void
create_room()
{
    set_short("The slavedeck");
    set_long(break_string("This is the slavedeck of the great ship. There are "+
	     "a number of benches on each side where the slaves sit when they "+
	     "are rowing, and there is a large beating drum at the far end of "+
	     "the deck that is used to keep the rowers in pace and enforcing "+
	     "greater speeds. A rugged brass lantern hangs in the ceiling, "+
	     "shedding a dim light so that you can barely see the room.\n",76));
    add_item("drum", "This is a large drum with a propbably deafening sound.\n");
    add_item("benches", "Old, wooden benches. No mahorny here!\n");
    add_item("lantern", "It's an old brass lantern, secured in the ceiling.\n");
    add_exit("/d/Emerald/plugh/ship/cabin", "up", 0);
    add_prop(ROOM_I_INSIDE, 1);
    call_out("reset_room", 1);
}

inherit "/lib/area_handler";

#include <stdproperties.h>

create_area()
{
    set_map("/d/Standard/doc/examples/area_handler/ah_map");
    add_desc("m", ({ ({ ROOM_NORMAL, 0, 1 }),
		     ({ "a meadow",
			"You find yourself on a meadow, covered with dark blossoms buzzing with insectoid life. A pungent overwhelming smell almost makes your throw up, but you recover your composure in time.",
			"You are standing in the middle of a black meadow. If it wasn't for the fact that everything looks so... alive... you'd think there had been a fire here recently.",
                         "A black meadow crawling with disgusting bugs surrounds you, get out of here!!!" }),
		     ({ "insect", "A disgusting crawling bug! Blech!" }),
		     ({ }) }));
    add_desc("p", ({ ({ ROOM_NORMAL, 0, 1 }),
		     ({ "a path",
			"You find yourself on an open path crossing the meadows. The insects seems to keep well off the track, perhaps due to the tar that covers it." }),
		     ({ "insect", "You can't find any around here.", "bug", "None, zip, nothing." }),
		     ({ }) }));
    add_desc("f", ({ ({ ROOM_NORMAL, 1, 1 }),
		     ({ "cottage floor",
			"You are standing inside the cottage. The floor is rather plain without any carpets of any kind. Strange... there's no furniture here either....",
			"The cottage floor is completely empty. No carpets. No furniture. Strange..." }),
		     ({ }),
		     ({ }) }));
    add_desc("t", ({ ({ ROOM_NORMAL, 0, 1 }),
		     ({ "a tree",
			"In the middle of the meadows stands a tree. It's a gnarled tree that seems to have been hit by the lightning many times but somehow survived every time. You feel an evil presence looming over you." }),
		     ({ "presence", "It can't be seen, only felt..." }),
		     ({ "/d/Immortal/mrpr/obj/examples/tree" }) }));
    add_bound("A", "/d/Standard/doc/examples/area_handler/ah_entry");
    add_bound("B", "/d/Standard/doc/examples/area_handler/ah_cottage_1");
    add_bound("C", "/d/Standard/doc/examples/area_handler/ah_cottage_2");
}


inherit "/std/room";
#include <macros.h>

create_room()
{
    set_short("a small cottage");
    set_long("You are standing in the doorway of a small cottage.\n");

    if (LOAD_ERR("/d/Standard/doc/examples/area_handler/ah_obj"))
    {
	write("Can't load map handler, please report immediately...\n");
	return;
    }

    add_exit("", "north", "@@enter_map:/d/Standard/doc/examples/area_handler/ah_obj|B@@");
    add_exit("", "south", "@@enter_map:/d/Standard/doc/examples/area_handler/ah_obj|C@@");
}

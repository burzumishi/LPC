inherit "/std/room";
#include <macros.h>

create_room()
{
    set_short("a small open place");
    set_long("You are standing on a small open place. Around you, you can see dark meadows.\n");

    if (LOAD_ERR("/d/Standard/doc/examples/area_handler/ah_obj"))
    {
	write("Can't load map handler, please report immediately...\n");
	return;
    }

    add_exit("", "east", "@@enter_map:/d/Standard/doc/examples/area_handler/ah_obj|A@@");
    add_exit("", "south", "@@enter_map:/d/Standard/doc/examples/area_handler/ah_obj|A@@");
}

/*
  mlink.c
*/

#pragma save_binary

inherit "/std/object";

#include "/secure/std.h"

void create_object()
{
    setuid();
    seteuid(getuid(this_object()));
    set_name("gadget");
    set_short("small map gadget");
    set_long("Something used by Commander to link the wizisland to the map.\n");
}

init()
{
    if (this_player()->query_real_name() != "commander")
	return;

    add_action("maplink", "maplink");
}

int maplink(string str)
{
   SECURITY->add_maplink("/d/Standard/manage_map", 0, ({ "x0y0", "x187y554" }));
   write("Ok.\n");
   return 1;
}
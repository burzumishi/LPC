/*
 * A normal storage room
 */

inherit "/std/room";
inherit "/lib/store_support";

#include "/sys/stdproperties.h"
#include "ex.h"

public void
enter_inv(object ob, object from)
{
    ::enter_inv(ob, from);
    store_update(ob);
}

void
create_room()
{
    set_short("The shop's store room");
    set_long("This is the store room for the shop.\n");

    add_exit(EX_ROOM + "shop", "south");

    add_prop(ROOM_I_INSIDE, 1);  /* This is an inside room */
}


/*
 * Copyright (c) 1991 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */

#pragma save_binary

inherit "/std/scroll";
#include <stdproperties.h>
#include "/config/sys/local.h"

create_scroll()
{
    seteuid(getuid(this_object()));
    add_prop(OBJ_I_NO_DROP, "Don't drop the scroll, it contains valuable " +
	"information!\n");
    set_name("scroll");
    set_short("scroll labeled 'READ ME!'");
    set_long("The scroll is held rolled up with a blue and yellow band tied " +
	"around it.\nOn the band the text 'READ ME!' is written in golden " +
	"letters.\n");
    set_autoload();
    set_file(APPRENTICE_SCROLL_FILE);
}

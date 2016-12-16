#pragma strict_types

inherit "/std/object";

#include "guild.h"

#include <stdproperties.h>

void
create_object()
{
    set_name("horn");
    add_name(GUILD_EMBLEM_ID);
 
    add_adj("big");

    set_long("The horn given to you by the " + GUILD_NAME + ".\n");

    add_prop(OBJ_M_NO_DROP, 1);
    add_prop(OBJ_M_NO_GET,  1);
    add_prop(OBJ_M_NO_SELL, 1);
    add_prop(OBJ_M_NO_BUY,  1);
    add_prop(OBJ_I_WEIGHT,  2000);
    add_prop(OBJ_I_VOLUME,  500);

    remove_prop(OBJ_I_VALUE);
}

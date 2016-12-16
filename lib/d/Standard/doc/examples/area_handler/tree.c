inherit "/std/object";
#include <macros.h>

#include <stdproperties.h>

create_object()
{
    set_name("tree");
    set_adj("gnarled");
    set_long("A gnarled tree, blackened and twisted by several strokes of lightning.\n");
    add_prop(OBJ_I_VOLUME, 500000);
    add_prop(OBJ_I_WEIGHT, 300000);
    add_prop(OBJ_I_VALUE, 10);
}

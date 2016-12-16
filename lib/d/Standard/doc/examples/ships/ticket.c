inherit "/std/object";

#include <stdproperties.h>

void
create_object()
{
    set_name("ticket");
    add_name("the emerald line");
    add_adj("torn");
    set_long("It looks like a ticket to some ship.\n");
    add_prop(OBJ_I_VALUE, 30);
}

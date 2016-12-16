/*
   knife.c

   A sample weapon
*/
inherit "/std/weapon";

#include <macros.h>

void
create_weapon()
{
    set_name("knife"); set_pname("knives");
    set_short("small knife"); set_pshort("small knives");
    set_adj("small");
    set_long("It is a very simple looking knife, quite deadly though.\n");
    
    set_default_weapon();
}

string
query_recover()
{
    return MASTER + ":" + query_wep_recover();
}

void
init_recover(string arg)
{
    init_wep_recover(arg);
}

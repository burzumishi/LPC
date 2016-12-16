#pragma strict_types

inherit "/std/room";

inherit "/d/Genesis/lib/post";

#include "guild.h"

#include <files.h>
#include <mail.h>

void
create_room()
{
    set_short("Guild Post Office");
    set_long("This is the " + GUILD_NAME + " post office.\n");

    add_exit("start", "south");
}

void
init()
{
    ::init();
    post_init();
}

void
leave_inv(object ob, mixed to)
{
    ::leave_inv(ob, to);
    post_leave_inv(ob, to);
}

void
mail_message(string new)
{
    write("\nYou have" + new + " mail for you in the " + GUILD_NAME +
        " post office.\n\n");
}

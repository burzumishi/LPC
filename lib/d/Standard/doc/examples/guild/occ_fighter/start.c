/* This is a guild start room.  Note that default start locations
 * need to be registered with /secure/master.c.  See an archwizard
 * to have your start location registered.
 */

#pragma strict_types

inherit "/std/room";

#include "guild.h"

void
create_room()
{
    set_short("Start room");
    set_long("This is the " + GUILD_NAME + " start room.\n");

    add_exit("joinroom", "west");
    add_exit("train",    "south");
    add_exit("post",     "north");
}

int
start(string str)
{
    if (str != "here") 
    {
        notify_fail("Start here?\n");
        return 0;
    }

    if (!this_player()->query_guild_member(GUILD_NAME))
    {
        write("You are not a member of this guild!\n");
        return 1;
    }

    if (this_player()->set_default_start_location(GUILD_STARTLOC))
    {
        write("Ok.\n");
    }
    else
    {
        write("Sorry, some problem prevents you from starting here.\n");
    }

    return 1;
}

void
init()
{
    ::init();

    add_action(start, "start");
}

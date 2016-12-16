/*
 * Sparkle post office
 * Newbie helper poster added by TAPAKAH, 25/05/2005
 */

#pragma save_binary
#pragma strict_types

inherit "/std/room";
inherit "/d/Standard/lib/post";

#include <stdproperties.h>

void
create_room()
{
    set_short("The post office");
    set_long("\n"+
             "You are in the post office.\n"+
             "This is a place of great magic! From here you can reach people\n"+
             "with a written message, all over the world! Quite amazing.\n"+
             "A few of the commands are:\n" +
             "\tfrom [new]       List all [unread] headers.\n"+
             "\tread             Start up the mailreader command mode.\n"+
             "\tread <message>   Read message number <message> from your mailbox.\n"+
             "\tmail <name>      Mail to player(s) <name>.\n"+
             "Examine the mail reader for more instructions.\n" +
             "\nThere is an office of some sort to the east.\n" +
             "");

    add_item(({"walls","wall" }),"The red walls are quite ordinary.\n"+
    "");
    add_item(({"room" }),"");

    add_exit("church", "north");

    add_prop(ROOM_I_INSIDE, 1);
    add_prop(ROOM_I_TYPE, ROOM_NORMAL);

}

void
init()
{
    ::init();

    post_init();
}

void
leave_inv(object ob, object to)
{
    ::leave_inv(ob, to);

    post_leave_inv(ob, to);
}

void
mail_message(string new)
{
    string sp = "       ";

    if (new == " NEW")
        sp = "   ";
    else if (new == " UNREAD")
        sp = "";
    
    write("\n"+
        "     +-------------------------------------------------------+\n"+
        "     ! There is" + new + " mail for you in Sparkle post office. "+
        sp + " !\n"+
        "     ! (North end of the road just inside the village gate.) !\n"+
        "     +-------------------------------------------------------+\n");
}

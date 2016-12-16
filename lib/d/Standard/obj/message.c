/*
 * A standard message of the type devivered by pigeons, couriers, etc.
 *
 * Simply clone this object, and use the set_sender() and set_message()
 * methods.
 */

#pragma save_binary

inherit "/std/object";

#include <stdproperties.h>
#include <macros.h>

string gMessage, gSender;

public string read_message();

public void
create_object()
{
    set_name("message");
    set_pname("messages");
    set_long("An empty message.\n");
    add_prop(OBJ_I_WEIGHT, 100);
    add_prop(OBJ_I_VOLUME, 100);
    gMessage = "Nothing.\n";
    gSender = "Someone";
}

public void
set_message(string str)
{
    gMessage = str;
    set_long(read_message());
}

public void
set_sender(string str) 
{
    gSender = str;
    set_short("message from " + capitalize(str));
    set_pshort("messages from " + capitalize(gSender));
}

public string
query_sender()
{
    return gSender;
}

public string
read_message()
{
    return ("The message reads:\n" + gMessage);
}

public mixed
command_read(int m)
{
    if (m)
    {
        this_player()->more(read_message());
    }
    else
    {
        write(read_message());
    }

    return 1;
}

public void
destruct_me()
{
    object room;
 
    if (living(room = environment()))
    {
        room = environment(room);
    }
 
    tell_room(room, "The message blows away with the wind.\n");
    remove_object();
}

public void
leave_env(object env, object to)
{
    ::leave_env(env, to);

    if (env)
    {
        set_alarm(0.5, 0.0, destruct_me);
    }
}

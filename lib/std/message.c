/*
 * /std/message.c
 *
 * Standard Message Object
 * -----------------------
 * The original content of this code was from the Frogmorton, but has
 * a long history of creators. The purpose of this new version is to
 * standardize messaging in the Realms by implementing a base messenger
 * library.
 *
 * Functions to be overridden by inheriting objects
 * public void             create_message();
 * public string           message_description();
 * public varargs void     hook_message_destroyed(object room, object player);
 */
#pragma save_binary
#pragma strict_types
 
inherit "/std/object.c";
 
#include <cmdparse.h>
#include <composite.h>
#include <macros.h>
#include <stdproperties.h>

// Variables
private string  gMessage; /* The message contents. */
private string  gSender;  /* The capitalized name of the author. */

// Prototypes
public string   query_sender();
public string   query_message();

/*
 * Function:    create_message
 * Description: Default create method for a message object. Mask this
 *              to set up your own default values
 * Arguments:   none
 * ReturnS:     nothing
 */
public void
create_message()
{
}

/*
 * Function:    message_description
 * Description: This function is what the message looks like when it is
 *              "read" or "examine"d. Mask this function to customize
 *              the presentation to the player.
 * Arguments:   none
 * Returns:     string to be displayed
 */
public string
message_description()
{
    string sender = query_sender();
    if (!strlen(sender))
    {
        sender = "Someone";
    }
    
    return "The message from " + sender + " reads:\n" + EXPAND_LINE("-", 70) +
        "\n\n"+ query_message() + "\n"+ EXPAND_LINE("=", 70) + "\n\n";
}

/*
 * Function:    hook_message_destroyed
 * Description: Hook method that gets called when a message is destroyed.
 *              Mask this function to customize the message that players
 *              will see.
 * Arguments:   room   - room that the message is in
 *              player - if not null, then this is the player that dropped
 *                       the message, causing it to be destroyed.
 * Returns:     nothing
 */
public varargs void
hook_message_destroyed(object room, object player)
{
    tell_room(room, "The " + short() + " crumbles to dust, then blows away.\n");
}

/*
 * Function:    create_object
 * Description: This is the default create function for a message.
 *              Customize the message create functions by masking
 *              create_message instead of this one.
 * Arguments:   none
 * Returns:     nothing
 */
public nomask void
create_object()
{
    set_name("message");
    set_pname("messages");
    set_short("message");
    set_pshort("messages");
    set_long(message_description);
    add_prop(OBJ_I_WEIGHT, 100);
    add_prop(OBJ_I_VOLUME, 100);
    gMessage = "Nothing.\n";
    gSender = "Someone";
    
    create_message();
}

/*
 * Function:    do_read
 * Description: This is the methad that gets called when the player
 *              tries to read or examine the message
 * Arguments:   str - arguments to the read command
 * Returns:     0/1 - failure/success of action
 */
public int
do_read(string str)
{
    object target;

    target = PARSE_COMMAND_ONE(str, 0, "[the] %i");
    if (!objectp(target))
    {
        notify_fail(capitalize(query_verb()) + " what?" +
            (PARSE_COMMAND_SIZE ? " Please select only one item." : "") + "\n");
        return 0;
    }
    else if (target != this_object())
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    /* Reading the message is teh same as examining the long description. */
    write(message_description());
    return 1;
}

/*
 * Function:    init
 * Description: Gives the object the "read" command.
 * Arguments:   none
 * Returns:     nothing
 */
public void
init()
{
    add_action(do_read, "read");
}

/*
 * Function:    destruct_me
 * Description: This function gets called whenever the message is 
 *              dropped or otherwise needs to be destroyed. It
 *              mainly calls the hook_message_destroyed to tell
 *              those in the room about the message's destruction
 * Arguments:   none
 * Returns:     nothing
 */
public void
destruct_me()
{    
    object room, player;

    if (IS_LIVING_OBJECT(environment(this_object())))
    {
        player = environment(this_object());
        room = environment(player);
    }
    else
    {
        room = environment(this_object());
    }
    hook_message_destroyed(room, player);
    remove_object();
}

/*
 * Function name: enter_env
 * Description  : This function is called each time this object enters a
 *                new environment. If you mask it, be sure that you
 *                _always_ call the ::enter_env(dest, old) function.
 * Arguments    : object dest - the destination we are entering.
 *                object old  - the location we came from. This can be 0.
 */
public nomask void
enter_env(object dest, object old) 
{
    ::enter_env(dest, old);

    if (IS_ROOM_OBJECT(dest))
    {
        set_alarm(2.0, 0.0, destruct_me);
    }
}

// SETTERS AND GETTERS BELOW

/*
 * Function:    set_message
 * Description: This sets the message content of the message object.
 * Arguments:   str - content of the message
 * Returns:     nothing
 */
public void
set_message(string str)
{
    gMessage = str;
}

/*
 * Function:    query_message
 * Description: Returns the message content of the message object
 * Arguments:   none
 * Returns:     the content of the message
 */
public string
query_message()
{
    return gMessage;
}

/*
 * Function:    set_sender
 * Description: This sets the message sender of the message object.
 * Arguments:   str - sender's name
 * Returns:     nothing
 */
public void
set_sender(string str)
{
    gSender = capitalize(str);
}

/*
 * Function:    query_sender
 * Description: Returns the message sender of the message object
 * Arguments:   none
 * Returns:     the name of the sender
 */
public string
query_sender()
{
    return gSender;
}

/*
 * /lib/messenger.c
 *
 * Base Messaging Library
 * ======================
 * This library can be inherited to provide messenger functionality. It is
 * implemented as a library rather than a base messenger object to allow
 * for both humanoid and non-humanoid messengers, and also to let domains
 * have their own base NPC's with localised functionality.
 *
 * To add messenger functionality, simply inherit from this library
 * and then add the following hooks and options
 *
 * Options
 * set_can_deliver_indoors(int bCanDeliver);
 * set_can_deliver_unmet(int bCanDeliver);
 * set_message_object_file(string file);
 * set_number_of_uses(int uses);
 * 
 * Hooks
 * hook_cannot_deliver_player_unavailable(string who);
 * hook_cannot_deliver_player_linkdead(string who);
 * hook_cannot_deliver_wizard_busy(object wizard);
 * hook_cannot_deliver_unmet(string who);
 * hook_deliver_message_failure(object sender);
 * hook_deliver_message_success(object receiver);
 * hook_messenger_returns_home();
 * hook_resend_message();
 * hook_send_message();
 * hook_dismiss_messenger();
 * appraise_messenger(int num);
 *
 * The most basic messenger can be done using the following code:
 *
 *      inherit "/std/creature";
 *      inherit "/lib/messenger";
 *
 *      public void
 *      create_messenger()
 *      {
 *          // Customize the options here
 *          set_can_deliver_indoors(1);
 *          set_can_deliver_unmet(1);
 *          set_message_object_file(MESSAGE_OBJ);
 *          set_number_of_uses(5);
 *      }
 *
 *      public nomask void 
 *      create_creature()
 *      {
 *          // Call the initialization stuff
 *          init_messenger();
 *      }
 *
 *      public void 
 *      appraise_object(int num)
 *      {
 *          ::appraise_object(num);
 *          appraise_messenger(num);
 *      }
 */

#include <files.h>
#include <language.h>
#include <macros.h>
#include <stdproperties.h>
#include <flags.h>
#include <cmdparse.h>

// Global Variables
static string  gSender = "";   /* The (lower case) sender of the message. */
static string  gMessage = "";  /* The message text. */
static string  gReceiver = ""; /* The (lower case) receiver of the message. */
static int     gNumSent = 0;   /* Number of times this messenger was used. */

// Options
static int     bCanDeliverIndoors;
static int     bCanDeliverUnmet;
static string  MessageObjectFile;
static int     NumberOfUses = 0;

// Setters for Options
public void    set_can_deliver_indoors(int bCanDeliver);
public void    set_can_deliver_unmet(int bCanDeliver);
public void    set_message_object_file(string file);
public void    set_number_of_uses(int uses);
// Getters for Options
public int     query_can_deliver_indoors();
public int     query_can_deliver_unmet();
public string  query_message_object_file();
public int     query_number_of_uses();

// Hooks to be defined - This is how you customize the messenger's
// message to be displayed to the users
public void    hook_cannot_deliver_player_unavailable(string who);
public void    hook_cannot_deliver_player_linkdead(string who);
public void    hook_cannot_deliver_wizard_busy(object wizard);
public void    hook_cannot_deliver_unmet(string who);
public void    hook_deliver_message_failure(object sender);
public void    hook_deliver_message_success(object receiver);
public void    hook_messenger_returns_home();
public void    hook_resend_message();
public void    hook_send_message();
public void    hook_dismiss_messenger();

// Prototypes
public int     query_delivery_time(object sender, object receiver);
public int     deliver();

#if 0
/*
 * Function:    create_messenger
 * Description: Override this function to define your own messenger. This
 *              routine does not actually exist.
 */
public void
create_messenger()
{
} 
#endif 0

/*
 * Function:    init_messenger
 * Description: This nomask function defines the defaults for the messenger.
 *              Define any overrides in create_messenger.
 */
public nomask void 
init_messenger()
{
    /* Since this does not derive from a monster/creature, we need to
     * use external calls to call the "underlying" routines.
     */
    this_object()->add_name("messenger");
    this_object()->set_short("base messenger");
    this_object()->set_long("A nondescript messenger.\n");
 
    this_object()->add_prop(OBJ_I_WEIGHT,  1000);
    this_object()->add_prop(CONT_I_WEIGHT, 1500);
    this_object()->add_prop(OBJ_I_VOLUME,  2000);
    this_object()->add_prop(CONT_I_VOLUME, 2000);
    this_object()->remove_prop(OBJ_I_NO_GET);
    this_object()->add_prop(NPC_I_NO_LOOKS, 1);
    this_object()->remove_prop(OBJ_I_NO_DROP);
    this_object()->add_prop(LIVE_I_NON_REMEMBER, 1);
    this_object()->add_prop(NPC_M_NO_ACCEPT_GIVE, 1);
    this_object()->add_prop(OBJ_M_NO_ATTACK,
        "One should not blame the messenger for bearing bad news.\n");

    // By default, we allow delivery to indoor rooms, and we allow
    // delivery to people who are unmet.
    set_can_deliver_indoors(1);
    set_can_deliver_unmet(1);
    set_message_object_file(MESSAGE_OBJECT);
    set_number_of_uses(5);

    this_object()->create_messenger();
}

/* 
 * Function:    return_messenger_home
 * Description: This function sends a message to the environment to
 *              alert others that the messenger is going home. In
 *              actuality, the messenger is just deleted.
 * Arguments:   none
 * Returns:     nothing
 */
public nomask void 
return_messenger_home() 
{
    hook_messenger_returns_home();
    this_object()->remove_object();
}

/*
 * Function:    do_resend
 * Description: This is called when the player resends a failed message.
 * Arguments:   none
 * Returns:     nothing
 */
public int
do_resend()
{
    object receiver;

    if (!strlen(gSender) || !strlen(gReceiver)
	|| this_player()->query_real_name() != gSender)
    {
    	notify_fail("You can't resend if you haven't sent first.\n");
	return 0;
    }

    if (!strlen(gMessage))
    {
        notify_fail("There is no message to resend.\n");
        return 0;
    }

    receiver = find_player(gReceiver);
    if (!receiver || receiver->query_linkdead())
    {
        hook_cannot_deliver_player_linkdead(gReceiver);
        return 1;
    }

    hook_resend_message();

    this_object()->move(VOID_OBJECT, 1);
    set_alarm(itof(query_delivery_time(this_player(), receiver)), 0.0, deliver);

    return 1;
}

/*
 * Function:    do_send
 * Description: This is the function that gets called when someone
 *              uses the "send" command.
 * Arguments:   who - the arguments to the send command. Valid ones
 *              are the name of the receiver, or "reply" to reply
 *              to a message that was received.
 * Returns:     0/1 - failure/success in sending
 */
public int 
do_send(string who)
{
    object target;
    object editor;

    if (this_player() != environment())
    {
	return 0;
    }

    if (!strlen(who))
    {
        notify_fail("Send message to whom?\n");
    	return 0;
    }

    who = lower_case(who);
    if (who == this_player()->query_real_name())
    {
	notify_fail("No point in sending a message to yourself.\n");
	return 0;
    }
	
    if (who == "reply")
    {
        if (!strlen(gSender) || gSender == this_player()->query_real_name())
        {
            notify_fail("There is no one to send a reply to!\n");
            return 0;
        }
        who = gSender;
    }

    /* Only deliver if they either know the person, or the setting is
     * set for whether the messenger can deliver to unmet individuals. */
    target = find_player(who);
    if (!query_can_deliver_unmet() && !this_player()->query_met(who) &&
        !target->query_prop(LIVE_I_ALWAYSKNOWN))
    {
        hook_cannot_deliver_unmet(who);
        return 0;
    }

    /* No target, or wizard is invisible. */
    if (!objectp(target) ||
        (target->query_wiz_level() && target->query_invis()))
    {
        hook_cannot_deliver_player_unavailable(who);
        return 0;
    }

    if (target->query_wiz_level())
    { 
        if (target->query_prop(WIZARD_I_BUSY_LEVEL) & BUSY_M)
        {
            hook_cannot_deliver_wizard_busy(target);
            return 0;
        }
    }

    if (target->query_linkdead())
    {
        hook_cannot_deliver_player_linkdead(who);
        return 1;
    }

    gSender = this_player()->query_real_name();
    gReceiver = who;

    say(QCTNAME(this_player()) + " starts to write a message.\n");
    this_player()->add_prop(LIVE_S_EXTRA_SHORT, " is writing a message");
    gMessage = "";

    setuid();
    seteuid(getuid());

    editor = clone_object(EDITOR_OBJECT);
    editor->set_activity("a message.");
    editor->edit("done_editing", "");

    return 1;
}

/*
 * Function:    do_dismiss
 * Description: This is the function that gets called when someone
 *              uses the "dismiss" command.
 * Arguments:   who - the arguments to the dismiss command. This should
 *                    match the messenger that wishes to be dismissed.
 * Returns:     0/1 - failure/success in dismissing
 */
public int 
do_dismiss(string who)
{
    if (!strlen(who))
    {
        notify_fail("Dismiss whom?\n");
    	return 0;
    }

    if (PARSE_COMMAND_ONE(who, 0, "[the] %i") != this_object())
    {
        notify_fail("Dismiss whom?\n");
        return 0;
    }

    hook_dismiss_messenger();
    set_alarm(0.0, 0.0, return_messenger_home);

    return 1;
}

/*
 * Function:    init_living
 * Description: The send command is added whenever this messenger
 *              encounters one of these messengers.
 */
public void 
init_living()
{
    add_action(do_dismiss, "dismiss");
    add_action(do_resend,  "resend");
    add_action(do_send,    "send");
}

/*
 * Function:    query_delivery_time
 * Description: This function defines the basic times that it usually
 *              takes to deliver a message.
 * Arguments:   sender - person sending the message
 *              receiver - person receiving the message
 * Returns:     The number of seconds until the receiver receives the
 *              message
 */
public int
query_delivery_time(object sender, object receiver)
{
    if (!objectp(sender) || !objectp(receiver))
    {
        return 25 + random(25);
    }

    if (environment(sender) == environment(receiver))
    {
        return random(5);
    }

    if (environment(sender)->query_domain() 
        == environment(receiver)->query_domain())
    {
        return 15 + random(15);
    }

    return 25 + random(25);
}

/*
 * Function:    done_editing
 * Description: This function gets called after the message has been composed.
 *              It moves the messenger to the void and kicks off the alarm to
 *              deliver the message.
 * Arguments:   string message - the message text.
 */
public void
done_editing(string message)
{
    object receiver;

    this_player()->remove_prop(LIVE_S_EXTRA_SHORT);

    if (!strlen(message))
    {
        write("Message aborted.\n");
        return;
    }

    gMessage = message;
    receiver = find_player(gReceiver);
    if (!receiver || receiver->query_linkdead())
    {
        hook_cannot_deliver_player_linkdead(gReceiver);
        return;
    }

    hook_send_message();

    this_object()->move(VOID_OBJECT, 1);
    set_alarm(itof(query_delivery_time(this_player(), receiver)), 0.0, deliver);
}
 
/*
 * Function:    deliver
 * Description: Actual function that creates the message object and
 *              moves it to the receiver. If the receiver is not 
 *              available, then it returns to the sender.
 * Argumetns:   none
 * Returns:     Always returns 1
 */
public int 
deliver()
{
    object sender, receiver, message_obj;
 
    setuid();
    seteuid(getuid());

    catch(message_obj = clone_object(query_message_object_file()));
    if (!objectp(message_obj))
    {
        throw("Bad message object file, or failed to load.\n");
        return 1;
    }
    message_obj->set_message(gMessage);
    message_obj->set_sender(gSender);

    /* Does our initial target still exist? */
    receiver = find_player(gReceiver);
    sender = find_player(gSender);
    if (!objectp(receiver)
        || receiver->query_linkdead()
        || (!query_can_deliver_indoors()
            && environment(receiver)->query_prop(ROOM_I_INSIDE)))
    {
        // If the target is gone, linkdead, or inside without being
        // able to deliver inside, we return the messenger to the
        // sender
        if (!objectp(sender))
        {
            message_obj->remove_object();
            this_object()->remove_object();
            return 1;
        }
        this_object()->move(sender, 1);
        message_obj->move(sender, 1);
        hook_deliver_message_failure(sender);
    }
    else
    {
        this_object()->move(receiver, 1);
        message_obj->move(receiver, 1);
        hook_deliver_message_success(receiver);
        ++gNumSent;
        gMessage = "";
    }

    // Check to see if the number of uses has been exceeded.
    if (gNumSent >= query_number_of_uses())
    {
        set_alarm(0.0, 0.0, return_messenger_home);
    }
    return 1;
}

// GETTERS AND SETTERS

/*
 * Function:    set_can_deliver_indoors
 * Description: Set this option if you wish the messenger to be able
 *              to find people indoors
 * Arguments:   int bCanDeliver - messenger can deliver indoors or not.
 */
public void
set_can_deliver_indoors(int bCanDeliver)
{
    bCanDeliverIndoors = bCanDeliver;
}

/*
 * Function:    query_can_deliver_indoors
 * Description: Returns whether the messenger can deliver to someone
 *              who is indoors.
 * Returns:     int 0/1 - cannot/can deliver
 */
public int
query_can_deliver_indoors()
{
    return bCanDeliverIndoors;
}

/*
 * Function:    set_can_deliver_unmet
 * Description: Set this option if you wish the messenger to be able
 *              to deliver to people the sender has not been
 *              introduced to.
 * Arguments:   int bCanDeliver - whether the messenger can deliver
 */
public void
set_can_deliver_unmet(int bCanDeliver)
{
    bCanDeliverUnmet = bCanDeliver;
}

/*
 * Function:    query_can_deliver_unmet
 * Description: Returns whether the messenger can deliver to someone
 *              who has not been introduced to the sender.
 * Returns:     int 0/1 - cannot/can deliver
 */
public int
query_can_deliver_unmet()
{
    return bCanDeliverUnmet;
}

/*
 * Function:    set_message_object_file
 * Description: Set this option to indicate which message object file
 *              should be cloned for the message.
 * Arguments:   string file - the filename of the message object
 */
public void
set_message_object_file(string file)
{
    MessageObjectFile = file;
}

/*
 * Function:    query_message_object_file
 * Description: Returns the filename of the message object that should
 *              be cloned for this message.
 * Returns:     string - the filename of the message object
 */
public string
query_message_object_file()
{
    if (!strlen(MessageObjectFile))
    {
        return MESSAGE_OBJECT;
    }

    return MessageObjectFile;
}

/*
 * Function:    set_number_of_uses
 * Description: Set this option to indicate how many times a messenger
 *              can be used before it returns "home".
 * Arguments:   int uses - the number of uses. 0 if unlimited.
 */
public void
set_number_of_uses(int uses)
{
    /* Messenger must have at least one charge.  ... */
    if (uses > 0) NumberOfUses = uses;
}

/*
 * Function:    query_number_of_uses
 * Description: Returns the number of times that a messenger
 *              can be used before it returns home.
 * Returns:     int 0 if unlimited. Else number of times to be used.
 */
public int
query_number_of_uses()
{
    return NumberOfUses;
}

/*
 * Function name: appraise_messenger
 * Description  : Called when a person appraises the messenger. It gives
 *                information about the available commands. Note that if
 *                you do not inherit /std/messenger.c then you have to
 *                mask appraise_object() yourself and call this routine
 *                from it as described in the file header.
 * Arguments    : int num - the appraise semi-random number (unused here).
 */
public void
appraise_messenger(int num)
{
    write(this_object()->query_The_name(this_player()) + " can be used to "
        + "<send> a message to someone or to <send reply> to whomever "
        + "sent a message to you. One can also <dismiss> " 
        + this_object()->query_objective() + ".\n");
}

// HOOKS TO BE DEFINED

/*
 * Function:    hook_cannot_deliver_unmet
 * Description: Hook that gets called to display that the messenger cannot
 *              deliver to someone who has not been introduced to the
 *              sender. Set a notify_fail() message.
 * Arguments:   string who - name of the recipient.
 */
public void
hook_cannot_deliver_unmet(string who)
{
    notify_fail("You do not remember being introduced to anyone named " +
        capitalize(who) + ".\n");
}

/*
 * Function:    hook_cannot_deliver_player_unavailable
 * Description: Hook that gets called to display that the messenger cannot
 *              deliver to someone who is not in the game. Set a notify_fail()
                message.
 * Arguments:   string who - name of the recipient.
 */
public void
hook_cannot_deliver_player_unavailable(string who)
{
    notify_fail(this_object()->query_The_name(this_player())
       + " does not understand who " 
       + capitalize(who) + " is!\n");
}

/*
 * Function:    hook_cannot_deliver_player_linkdead
 * Description: Hook that gets called to display that the messenger cannot
 *              deliver to someone who is LD
 * Arguments:   string who - the name of the recipient.
 */
public void
hook_cannot_deliver_player_linkdead(string who)
{
    write(this_object()->query_The_name(this_player()) 
        + " looks at you and informs you that "
        + capitalize(who) + " is currently asleep.\n");
}

/*
 * Function:    hook_cannot_deliver_wizard_busy
 * Description: Hook that gets called to display that the messenger cannot
 *              deliver to a wizard who has set the busy flag. Set a
 *              notify_fail() message.
 * Arguments:   object wizard - the object of the wizard who is busy
 */
public void
hook_cannot_deliver_wizard_busy(object wizard)
{
    notify_fail("That wizard does not want to be disturbed just now.\n");
}

/*
 * Function:    hook_send_message
 * Description: Hook that gets called to display the message of the
 *              messenger getting the message and leaving to deliver it
 */
public void
hook_send_message()
{
    write("You give your message to " + this_object()->query_the_name(this_player()) + ". "
        + capitalize(this_object()->query_pronoun()) + " happily takes it and runs off to " 
        + "deliver it.\n");
    say(QCTNAME(this_player()) + " gives a message to " + QTNAME(this_object())
        + ". " + capitalize(this_object()->query_pronoun()) + " happily takes it and runs "
        + "off to deliver it.\n");
}

/*
 * Function:    hook_resend_message
 * Description: Hook that gets called to display the message of the
 *              messenger being asked to re-deliver after a failed attempt.
 */
public void
hook_resend_message()
{
    write("You convince " + this_object()->query_the_name(this_player()) + " to try again. "
        + capitalize(this_object()->query_pronoun()) + " happily takes it and runs off to " 
        + "deliver the message again.\n");
    say(QCTNAME(this_player()) + " convinces " + QTNAME(this_object())
        + " to try again. " + capitalize(this_object()->query_pronoun()) + " happily "
        + "takes it and runs off to deliver the message again.\n");
}

/*
 * Function:    hook_dismiss_messenger
 * Description: Hook that gets called to display the message when the
 *              player dismisses the messenger (to go home).
 */
public void
hook_dismiss_messenger()
{
    write("You dismiss " + this_object()->query_the_name(this_player()) + ".\n");
    say(QCTNAME(this_player()) + " dismisses " + QTNAME(this_object()) + ".\n");
}

/*
 * Function:    hook_deliver_message_failure
 * Description: Hook that gets called to display the message of the
 *              the messenger being unable to deliver the message and
 *              returning to the sender.
 * Arguments:   object sender - the person who sent the message
 */
public void
hook_deliver_message_failure(object sender)
{
    sender->catch_tell("Your " + this_object()->short() + " returns, unable to find "
        + capitalize(gReceiver) + ".\n");
    tell_room(environment(sender), QCTNAME(this_object()) + " runs up to "
        + QTNAME(sender) + ".\n", ({ sender }));   
}

/*
 * Function:    hook_deliver_message_success
 * Description: Hook that gets called to display the message of the
 *              messenger successfully delivering the message to the
 *              recipient.
 * Arguments:   object receiver - the person receiving the message
 */
public void
hook_deliver_message_success(object receiver)
{
    receiver->catch_msg(QCTNAME(this_object()) + " runs up to you with "
        + "a message from " + capitalize(gSender) + ".\n");
    tell_room(environment(receiver), QCTNAME(this_object()) + " runs up to "
        + QTNAME(receiver) + ".\n", ({ receiver }));
}

/*
 * Function:    hook_messenger_returns_home
 * Description: Hook that gets called to display the message of the
 *              messenger returning home after either being used up or
 *              being asked to return home.
 */
public void
hook_messenger_returns_home()
{
    object room;
    if (living(room = environment()))
    {
        room = environment(room);
    }

    tell_room(room, QCTNAME(this_object()) + " runs for home.\n");
}

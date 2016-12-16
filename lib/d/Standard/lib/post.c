/*
 * /d/Standard/lib/post.c
 *
 * Funcions needed to make a room a post office. Moditied from the
 * post office from the Rockfriend guild created by Mercade.
 *
 * Usage:
 *
 * - From within your init() function, make a call to post_init():
 *
 *   void
 *   init()
 *   {
 *       ::init();
 *       post_init();
 *   }
 *
 * - Define the function leave_inv() and call post_leave_inv() from it:
 *
 *   void
 *   leave_inv(object obj, object to)
 *   {
 *       ::leave_inv(obj, to);
 *       post_leave_inv(obj, to);
 *   }
 *
 * - You can define the function add_aliases() to add local aliases to the
 *   mail reader:
 *
 *   void
 *   add_aliases(object reader)
 *   {
 *       reader->add_alias("mayor", ({ "mercade" }) );
 *   }
 *
 * - Add the result from get_std_use() to your long description:
 *
 *   set_long("This is a post office.\n" + get_std_use());
 *
 * Revision history:
 *
 * 10 Oct 1994 Glindor - Modified from Mercade's Rockfriend Guild Post office.
 * 16 Feb 1998 Wisk - Added snoop prevention, except for AoP Helpers.
 * 01 Apr 1998 Mercade - Made post office silent, general revision.
 */

#pragma save_binary
#pragma strict_types

#include <files.h>
#include <mail.h>
#include <std.h>

/*
 * This list keeps all commands that are allowed by mortals in a post office.
 */
#define ALLOWED_COMMANDS ({ \
    "mail", "from", "autocc", "read", "mread", "malias", \
    "bug", "idea", "praise", "typo", \
    "sysbug", "sysidea", "syspraise", "systypo", \
    "alias", "unalias", "do", "resume", "save", "quit", \
    "commune", "reply", "help", "last", "date", "who", \
    "look", "l", "examine", "exa", \
    })

/*
 * Function name: get_std_use
 * Description  : This function returns the text that should be added to the
 *                long description about the usage of the mail reader.
 * Returns      : string - the text.
 */
string
get_std_use()
{
    return "Check the mailreader for instructions.\n";
}

/*
 * Function name: silent_room
 * Description  : This function catches all commands. Players will be allowed
 *                to use various commands related to mail, the game and the
 *                exits, but disallows all others.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - player blocked/player not blocked.
 */
public int
silent_room(string str)
{
    string verb = query_verb();

    /* Wizards are not silenced. */
    if (this_player()->query_wiz_level())
    {
	return 0;
    }

    /* Any of the allowed commands will be allowed ;-) */
    if (member_array(verb, ALLOWED_COMMANDS) >= 0)
    {
	return 0;
    }

    /* Player can always use an exit of the room. */
    if (member_array(verb, this_object()->query_exit_cmds()) >= 0)
    {
	return 0;
    }

    /* Block the command. */
    write("The post office is a place of silence. Please honour it.\n");
    return 1;
}

/*
 * Function name: add_aliases
 * Description  : This function can do things to the mailreader before it
 *                is given to the player, like adding local aliases.
 * Arguments    : reader - the mailreader object.
 */
void
add_aliases(object reader)
{
}

/*
 * Function name: post_init
 * Description  : Call this function from the local init() to add the mail
 *                reader to the player. It will also make the room silent for
 *                all while handling mail.
 */
void
post_init()
{
    object reader;

    /* If the player does not already have a mailreader (ie he is not a
     * wizard), then we clone a reader.
     */
    if (!objectp(reader = present(READER_ID, this_player())))
    {
	setuid();
	seteuid(getuid());
	reader = clone_object(MAIL_READER);

	/* We set the alias to the reader and move it into the player. */
	add_aliases(reader);
	reader->move(this_player(), 1);
    }

    /* All commands here are silenced, except the normal commands. */
    if (!this_player()->query_wiz_level())
    {
	//        add_action(silent_room, "", 1);
    }
}

/*
 * Function name: post_leave_inv
 * Description  : Call this function from your leave_inv() function. It will
 *                destruct the mail reader.
 * Arguments    : object obj - the object leaving.
 *                object to  - where 'obj' is leaving to.
 */
void
post_leave_inv(object obj, object to)
{
    object reader;

    /* A wizard gets to keep his/her mailreader. */
    if (obj->query_wiz_level())
    {
	return;
    }

    /* Remove mailreader from players. */
    if (objectp(reader = present(READER_ID, obj)))
    {
	reader->remove_object();
    }
}

/*
 * Function name: mail_message
 * Description  : This function is called from query_mail to give the player
 *                a message about the (new) mail in his mailbox.
 * Arguments    : string new - Depending on the status of the mailbox.
 */
void
mail_message(string new)
{
    write("There is" + new + " mail for you in the nearest post office.\n");
}

/*
 * Function name: query_mail
 * Description  : This function is called when a player logs on to give him
 *                a message about the (new) mail in his mailbox.
 * Arguments    : silent - Set to 1 to not tell the player about the mail.
 * Returns      : 0 - no mail
 *                1 - mail, all read
 *                2 - new mail
 *                3 - unread mail
 */
public int
query_mail(int silent)
{
    int    mail = MAIL_CHECKER->query_mail();
    string new;

    if (mail == 0)
    {
	return 0;
    }
    if (silent)
    {
	return mail;
    }

    switch(mail)
    {
    case 2:
	new = " NEW";
	break;

    case 3:
	new = " UNREAD";
	break;

    default:
	new = "";
    }

    mail_message(new);

    return mail;
}

/*
 * Function name: query_prevent_snoop
 * Description  : This function prevents snooping by wizards, except by the
 *                AoP-helpers.
 * Returns      : int 1/0 - prevent/allow.
 */
public int
query_prevent_snoop()
{
    string *alias_list;

    setuid();
    seteuid(getuid());

    /* An AoP-helper can snoop a player in a post office. */
    alias_list = SECURITY->query_team_list("aop");
    if (member_array(this_interactive()->query_real_name(), alias_list) >= 0)
    {
	return 0;
    }

    /* All others may not snoop. */
    return 1;
}

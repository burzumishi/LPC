/*
 * /secure/mail_reader.c
 *
 * Version 1.0 by Styles, 910824
 *
 * Mail reader, not yet adapted for CDlib 3.0. I will not continue to adapt
 * this version of the post since it has come to my knowledge that Tintin
 * and Groo have been working on this.
 *
 * Version 2.0 by Groo, 910903
 *
 * Adapted to CDlib 3.0, Using the file defined in PLAYERNAMES to check if a
 * player exists.
 *
 * Version 3.0 by Tricky, 19-4-92
 *
 * Since the original mailreader was very basic, and it seemed a low priority
 * project to the arches, I decided to enhance it a bit so it would fit my
 * purposes and would use the advantages of 3.0 a wee bit more. Also threw
 * out the PLAYERNAMES check. How un-3.0-ish can you get? Followed hints
 * from Tintin and added aliases for wizard types. Also changed the file
 * format to use arrays, saves a lot of work. Adjusted for use with the new
 * /secure/master/fob.c.
 *
 * Version 4.0 by Mercade, March 1 1994
 *
 * Revised the system of mail-save. Now for every mail message a file is
 * created. The mail-file of the player contains a link to the message-files
 * in separate directories. New_mail flag changed into new/unread mail.
 * Added personal aliases and area-related aliases. Done a general revision
 * and recoded the whole reader to get rid of old (slow) code. The more-stuff
 * has been replaced by the general more in players. Auto-more on too large
 * messages. Added "from new" and "from name". Gedit removed and adapted the
 * reader for the new editor. Enabled mail-store for domain wizards++.
 * Players cannot send mail when they have too many read messages. Added mail
 * generation from code. Added mail archiving. Mark important mails with "*".
 */


/*
 * AREA RELATED ALIASES
 *
 * You can make area-related aliases, which are valid only in the post-
 * office that adds them to the mailreader. These aliases allow a guild
 * to for instance define the aliases "council" and "guildmaster"(s) and
 * therewith you ease the communictation within the guild. Before you
 * move the reader into the player, the aliases have to be added with
 * a call to set_alias(string alias, string *names) where <alias> is the
 * name of the alias and <names> are the names to replace the alias with.
 * Note that <names> can only contain player names. If you have a council-
 * alias, you must have the <names> generated from the council-code since
 * it is not legal to hard-code player names. For each alias, you have to
 * make a separate call to the reader. Be advised that players will get
 * the substituted names in the addressee-list, so those names are not
 * secret. See also the header of the function set_alias().
 *
 * void reader->set_alias("alias", ({ "name1", "name2", ... }) );
 *
 * CHECKING THE NEW-MAIL-FLAG FROM THE POST OFFICE.
 *
 * In order to check whether there is new mail for a player, the post
 * office can check a query to /secure/mail_checker. For more information,
 * see that object.
 *
 * GENERATED MAIL
 *
 * It is possible to have a mail-message generated from code. The 'to' and
 * 'cc' strings are like the normal input you use. For more information,
 * see the header of the function create_mail(). This function may only be
 * called in the master mail-reader, defined in MAIL_READER in mail.h
 *
 * int MAIL_READER->create_mail(string subject, string author, string to,
 *                              string cc, string body);
 */

#pragma no_inherit
#pragma no_shadow
#pragma save_binary
#pragma strict_types


inherit "/std/object";


/*
 * Define DEBUG when debugging. This will secure the normal directories
 * and use a test-directory. You will also get some debug-messages on
 * various places.
 */
#undef DEBUG


#include <composite.h>
#include <filepath.h>
#include <files.h>
#include <log.h>
#include <language.h>
#include <macros.h>
#include <mail.h>
#include <std.h>
#include <stdproperties.h>
#include <time.h>


#define LOAD_PLAYER    \
    if (!gLoaded)      \
    {                  \
        load_player(); \
    }
#define LOOP         \
    if (gIs_reading) \
    {                \
        loop();      \
    }
#define CHECK_SO_BUSY \
    if (gBusy)        \
    {                 \
        LOOP;         \
        return 1;     \
    }

#define EMPTY_MAIL              \
    ([ MAIL_MAIL     : ({ }),   \
       MAIL_ALIASES  : ([ ]),   \
       MAIL_NEW_MAIL : FLAG_NO, \
       MAIL_AUTO_CC  : 0 ])
#define EMPTY_MESSAGE(date)                                       \
    ([ MSG_TO      : "Recipient",                                 \
       MSG_CC      : "",                                          \
       MSG_ADDRESS : "",                                          \
       MSG_BODY    :                                              \
"This is an error-mail from Postmaster.\n" +                      \
"You are reading a non-existing message. Please notify the\n" +   \
"administration and include the number " + (date) + ".\n\n" +     \
"Do not delete this message from your mailbox until you have\n" + \
"confirmation from an archwizard or keeper.\n\n" +                \
"Thanks,\n\n" +                                                   \
"Postmaster\n" ])

#define SPECIAL_TYPES ({ "global", "globals" })

#define STRING(arr)   (map(arr, &operator(+)("")))
#define WRITE(text)   (tell_object(environment(), (text)))

#define SAVE_MAIL                               \
    save_mail( ([ MAIL_MAIL     : pMessages,    \
                  MAIL_ALIASES  : pAliases,     \
                  MAIL_NEW_MAIL : pNew_mail,    \
                  MAIL_AUTO_CC  : pAutocc       \
               ]), environment()->query_real_name() )

#define FORWARD_FLAG(i)  ((i) & MSG_IS_FORWARD)
#define RESEND_FLAG(i)   ((i) & MSG_IS_RESEND)
#define REPLY_FLAG(i)    ((i) & MSG_IS_REPLY)
#define DO_EDIT_FLAG(i)  ((i) & MSG_DO_EDIT)
#define READ_FLAG(i)     ((i) & MSG_READ)
#define STARRED_FLAG(i)  ((i) & MSG_STARRED)
#define ANSWERED_FLAG(i) ((i) & MSG_ANSWERED)

/*
 * Global variables. They are private to ensure privacy of the mail and
 * static due to the fact that restore/save_object() was once used. I just
 * do not want anyone messing with someone elses mail. The variables
 * starting with 'p' (for private) are stored in the mail folder. Those with
 * 'g' (for general) are used in the mail reader internally.
 */
static private mixed   pMessages;   /* The players messages        */
static private int     pNew_mail;   /* Flag to indicate new mail   */
static private mapping pAliases;    /* The players aliases         */
static private int     pAutocc;     /* The flag indicating auto-cc */

static private int     gLoaded;     /* Are the messages loaded?    */
static private mapping gAliases;    /* The global aliases          */
static private int     gCurrent;    /* The current selected mail   */
static private int     gPrevious;   /* The previous gCurrent       */
static private int     gBusy;       /* Indicate sending mail       */
static private int     gIs_reading; /* Interactive mail reading    */
static private int     gUse_more;   /* Read message with more      */
static private string *gTo;         /* First mail receivers        */
static private string *gCc;         /* CC: mail receivers          */
static private string  gSubject;    /* Subject of the new mail     */
static private string  gMessage;    /* The new mail message        */
static private int     gIs_reply;   /* Reply/forward/resend/edit   */
static private mapping gTo_send;    /* The mail message to send    */
static private mapping gTo_delete;  /* The messages to delete      */
#ifdef MAX_IN_MORTAL_BOX
static private int     gRead_count; /* The number of messages read */
#endif MAX_IN_MORTAL_BOX


/*
 * Prototypes.
 */
static int alias(string str);
static int autocc(string str);
static int from(string str);
static int mail(string str);
static int read(string str);
static int resend(string str);
static void get_cmd(string str);
public string long_description();


/*
 * Function name: init_mail_reader
 * Description  : Called to initialise the global variables of the mail reader.
 * Arguments    : int first - if true, also initialise the global aliases.
 */
private void
init_mail_reader(int first)
{
    pMessages   = ({ });
    pNew_mail   = FLAG_READ;
    pAliases    = ([ ]);
    pAutocc     = 0;

    gLoaded     = 0;
    gCurrent    = 0;
    gPrevious   = 0;
    gBusy       = 0;
    gIs_reading = 0;
    gUse_more   = 0;
    gTo         = ({ });
    gCc         = ({ });
    gSubject    = "";
    gMessage    = "";
    gIs_reply   = 0;
    gTo_send    = ([ ]);
    gTo_delete  = ([ ]);

    if (first)
    {
        gAliases = ([ ]);
    }
}


/*
 * Function name: create_object
 * Description  : This function is called to create the object.
 */
public void
create_object()
{
    setuid();
    seteuid(getuid());

    set_name("mailreader");
    add_name("reader");
    add_name("mailread");
    add_name("folder");
    add_name("correspondence");
    add_name(READER_ID);

    set_pname("mailreaders");
    add_pname("readers");
    add_pname(READER_ID);

    set_adj("mail");
    add_adj("folder");
    add_adj("with");

    set_short("folder with correspondence");
    set_pshort("folders with correspondence");

    set_long(long_description);

    add_prop(OBJ_M_NO_DROP,     "Please hold on to your correspondence.\n");
    add_prop(OBJ_M_NO_GIVE,     "All correspondence is treated confidentially.\n");
    add_prop(OBJ_M_NO_TELEPORT, 1);
    add_prop(OBJ_M_NO_STEAL,    1);
    add_prop(OBJ_M_NO_SELL,     1);
    add_prop(OBJ_M_NO_BUY,      1);

    remove_prop(OBJ_I_WEIGHT);
    remove_prop(OBJ_I_VOLUME);
    remove_prop(OBJ_I_VALUE);

    init_mail_reader(1);
}


/*
 * Function name: long_description
 * Description  : Make a user-dependant long description. Wizards have
 *                more commands.
 * Returns      : string - the long description
 */
public string
long_description()
{
    return "This is your correspondence. The commands you can use are:\n\n" +
"from [new]               list all [unread] headers in your mailbox.\n" +
"     [<name[s]>]         optional: only messages received from <names>.\n" +
"     [*<filter>]         optional: only with <filter> in the subject.\n" +
"mail <name[s]>           mail something to one or more players.\n" +
"resend <name[s]>         mail the last message again to other people.\n" +
"read [<number>]          read message with number <number>.\n" +
"                         without argument you enter mail reading mode.\n"+
"mread <number>           read message with number <number> using more.\n" +
"malias                   list your mailreader aliases. You have " +
        ((m_sizeof(pAliases) == 0) ? "none" :
            (m_sizeof(pAliases) + " alias(es)")) + ".\n" +
"malias <alias> <names>   add mail alias <alias> with <names>.\n" +
"malias remove <alias>    remove mail alias <alias>.\n" +
"autocc [on / off]        set/unset automatic cc to yourself.\n" +
"                         Current setting: Automatic cc: " +
        (pAutocc ? "on" : "off") + ".\n";
}


/*
 * Function name: restore_mail
 * Description  : Restore the mail-file of a player from disk.
 * Arguments    : string name - the name of the player.
 * Returns      : mapping     - the mail of the player.
 */
static mapping
restore_mail(string name)
{
    mapping mail = restore_map(FILE_NAME_MAIL(name));

    /* If there is no mail for the player, return an empty mail-mapping. */
    if ((!mappingp(mail)) ||
        (m_sizeof(mail) != M_SIZEOF_MAIL))
        return EMPTY_MAIL;

    return mail;
}


/*
 * Function name: save_mail
 * Description  : Save the mail-file of a player to disk.
 * Arguments    : mapping mail - the mail of the player.
 *                string  name - the name of the player.
 */
static void
save_mail(mapping mail, string name)
{
    save_map(mail, FILE_NAME_MAIL(name));
}


/*
 * Function name: restore_message
 * Description  : Restore an individual message from disk.
 * Arguments    : int date - the time of the message.
 * Returns      : mapping  - the message restored.
 */
static mapping
restore_message(int date)
{
    mapping message = restore_map(FILE_NAME_MESSAGE(date, HASH_SIZE));

    /* This should not happen, but somehow the message can be destructed.
     * An empty (error containing) message is returned.
     */
    if ((!mappingp(message)) ||
        (m_sizeof(message) != M_SIZEOF_MSG))
        return EMPTY_MESSAGE(date);

    return message;
}


/*
 * Function name: save_message
 * Description  : Save the individual message to disk.
 * Arguments    : mapping message - the message.
 *                int     date    - the current time to save.
 * Returns      : int     1/0     - success/failure. */
static int
save_message(mapping message, int date)
{
    string file = FILE_NAME_MESSAGE(date, HASH_SIZE);

    save_map(message, file);

    if (file_size(file + ".o") <= 0)
        return 0;

    return 1;
}


/*
 * Function name: load_player
 * Description  : Make sure that the messages array of the player who
 *                going to handle his/her mail, is loaded into memory.
 */
static void
load_player()
{
    mapping mail;
    int     index;

    /* Only the environment of the reader can initiate a read since the
     * mailreader may only be in the direct inventory of the player.
     */
    if (!interactive(environment()))
    {
        remove_object();
        return;
    }

    mail = restore_mail(environment()->query_real_name());

    gLoaded  = 1;

    pMessages = mail[MAIL_MAIL];
    pAliases  = mail[MAIL_ALIASES];
    pNew_mail = mail[MAIL_NEW_MAIL];
    pAutocc   = mail[MAIL_AUTO_CC];

    /* If the player had new mail pending, change it to unread mail now. */
    if (pNew_mail == FLAG_NEW)
    {
        pNew_mail = FLAG_UNREAD;
        SAVE_MAIL;
    }

#ifdef MAX_IN_MORTAL_BOX
    /* Count the number of messages in the mailbox of the player. Messages
     * that are answered, count as read as well.
     */
    gRead_count = 0;
    index = sizeof(pMessages);
    while(--index >= 0)
    {
        gRead_count += (pMessages[index][MAIL_READ] != 0);
    }
#endif MAX_IN_MORTAL_BOX
}


/*
 * Function name: init
 * Description  : If a player encounters the object, add the commands to
 *                him or her.
 */
public void
init()
{
    /* Paranoid check! Don't mess with mail! */
    if ((this_player() != environment()) ||
        (!interactive(environment())))
    {
        return;
    }

#ifdef GUEST_NAME
    if (this_player()->query_real_name() == GUEST_NAME)
    {
        write("You are a guest of " + SECURITY->get_mud_name() + ". It is " +
            "no use for you to send mail to others players. Guest has no " +
            "identity, so you cannot even be mailed back. If you want to " +
            "play this game, you are welcome to create yourself a real " +
            "character.\n");
        remove_object();
        return;
    }
#endif GUEST_NAME

    add_action(alias,  "malias");
    add_action(autocc, "autocc");
    add_action(from,   "from");
    add_action(mail,   "mail");
    add_action(read,   "read");
    add_action(read,   "mread");
    add_action(resend, "resend");

    /* We display the 'unread' messages in the mailbox when the player enters
     * the post office.
     */
    set_alarm(0.1, 0.0, &from("new"));
}


/*
 * Function name: autocc
 * Description  : With this command, people can automatically cc:
 *                themselves everything they send.
 * Arguments    : string str - the command line argument
 * Returns      : int        - 1/0 success/failure
 */
static int
autocc(string str)
{
    LOAD_PLAYER;

    if (!strlen(str))
    {
        notify_fail("No argument to 'autocc'. Currently you " +
            (pAutocc ? "DO " : "do NOT ") +
            "automatically add yourself to the cc:list. Possible options: " +
            "\"on\" and \"off\".\n");
        return 0;
    }

    str = lower_case(str);
    if (str == "on")
    {
        if (pAutocc)
        {
            notify_fail("You are already automatically CCing yourself.\n");
            return 0;
        }
        else
        {
            pAutocc = 1;
            SAVE_MAIL;
            WRITE("Automatic CC to yourself turned ON.\n");
            return 1;
        }
    }

    if (str == "off")
    {
        if (!pAutocc)
        {
            notify_fail("You are not automatically CCing yourself.\n");
            return 0;
        }
        else
        {
            pAutocc = 0;
            SAVE_MAIL;
            WRITE("Automatic CC to yourself turned OFF.\n");
            return 1;
        }
    }

    notify_fail("Illegal argument. Possible arguments \"on\" and \"off\".\n");
    return 0;
}


/*
 * Function name: alias
 * Description  : This command should be used called when a player wants
 *                to add an alias to his/her personal mail fail. Yeah,
 *                I know it is a little large.
 * Arguments    : string str - the alias the player wants to add or remove.
 * Returns      : int        - 1/0 - success/failure
 */
static int
alias(string str)
{
    string *list;

    LOAD_PLAYER;

    if (!mappingp(pAliases))
        pAliases = ([ ]);

    /* Player wants to see his current aliases.  */
    if (!strlen(str))
    {
        if (!m_sizeof(pAliases))
        {
            notify_fail("You do not have any mailreader aliases set.\n");
            return 0;
        }

        WRITE("Current mailreader aliases:\n");
        foreach(string aname: m_indices(pAliases))
        {
            WRITE(HANGING_INDENT(sprintf("%-12s %s", aname,
                COMPOSITE_WORDS(pAliases[aname])), 13, 0));
        }
        return 1;
    }

    list = explode(lower_case(str), " ");
    if (sizeof(list) <= 1)
    {
        notify_fail("Illegal argument to malias.\n");
        return 0;
    }

    /* Player wants to remove an alias.  */
    if (list[0] == "remove")
    {
        if (sizeof(list) != 2)
        {
            notify_fail("Syntax to remove an alias: malias remove <alias>\n");
            return 0;
        }

        if (!IN_ARRAY(list[1], m_indices(pAliases)))
        {
            notify_fail("No such mailreader alias: \"" + list[1] + "\".\n");
            return 0;
        }

        m_delkey(pAliases, list[1]);
        SAVE_MAIL;

        WRITE("Mailreader alias \"" + list[1] + "\" removed.\n");
        return 1;
    }

    /* Player wants to add a new alias. We remove the old alias first if
     * it exists, to make space.
     */
    m_delkey(pAliases, list[0]);

    if (SECURITY->exist_player(list[0]) ||
        IN_ARRAY(capitalize(list[0]), SECURITY->query_domain_list()))
    {
        WRITE("Name \"" + list[0] + "\" is a player or domain name.\n");
        return 1;
    }

    if (m_sizeof(pAliases) >= MAX_ALIASES)
    {
        WRITE("You have already used the maximum number (" + MAX_ALIASES +
            ") of aliases. In order to add a new alias, you have to remove " +
            "one first.\n");
        return 1;
    }

    pAliases += ([ list[0] : list[1..(sizeof(list) - 1)] ]);
    SAVE_MAIL;

    WRITE("Mailreader alias \"" + list[0] + "\" added.\n");
    return 1;
}


/*
 * Function name: check_valid_msg_num
 * Description  : Check if the number is a valid message number, i.e. a
 *                number in the range of 1 to the number of messages.
 * Arguments    : msg_num: the number to check
 * Returns      : True if valid
 */
static int
check_valid_msg_num(int msg_num)
{
    return ((msg_num > 0) && (msg_num <= sizeof(pMessages)));
}


/*
 * Function name: loop
 * Description  : This is the main program loop that is called each
 *                time a procedure returns. It contructs the prompt
 *                and reloads messages if neccesary.
 */
static void
loop()
{
    string tmp;

    LOAD_PLAYER;

    if (!sizeof(pMessages))
    {
        WRITE("No messages.\n");
        gIs_reading = 0;
        return;
    }

    if (!gBusy)
    {
        if (!check_valid_msg_num(gCurrent))
        {
            gCurrent = 0;
            tmp = "(no current) -- ";
        }
        else
            tmp = "(current: " + gCurrent + ") -- ";

        WRITE("[1-" + sizeof(pMessages) + " adefFhHmnpqrRsux.!?] " + tmp);
    }

    environment()->add_prop(LIVE_S_EXTRA_SHORT, " is reading mail");

    input_to(get_cmd);
}


/*
 * Function name: done_reading_more
 * Description  : This function is a security function that checks whether
 *                it was indeed the more function that called us. We do
 *                not want anyone else to call loop().
 */
public void
done_reading_more()
{
    if (previous_object() == environment())
        loop();
}


/*
 * Function name: convert_to_names
 * Description  : Change the list of domain- and player names, together
 *                with the aliases to a list of player names only.
 * Arguments    : string *list - a list of domains and names
 * Returns      : string *     - the list with only names
 */
static string *
convert_to_names(string *list)
{
    string *return_arr = ({ });
    string *tmp_arr;
    string name;
    int    force_check;
    int    type;
    int    is_wizard = environment()->query_wiz_level();

    if (!sizeof(list))
        return ({ });

    foreach(string name: list)
    {
        /* The name may be a normal player. */
        name = lower_case(name);
        if (SECURITY->exist_player(name))
        {
            return_arr |= ({ name });
            continue;
        }

        if (is_wizard)
        {
            /* The name may be a class of wizards. */
            if ((type = member_array(LANG_SWORD(name), WIZ_N)) > -1)
            {
                /* Prevent mail spam on honourary administrators. */
                if (type >= WIZ_ARCH)
                {
                    name = "admin";
                    continue;
                }
 
                if (sizeof(tmp_arr = SECURITY->query_wiz_list(type)))
                    return_arr |= tmp_arr;

                /* When you mail the arches, keepers get it too. */
                if ((type == WIZ_ARCH) &&
                    (sizeof(tmp_arr = SECURITY->query_wiz_list(WIZ_KEEPER))))
                    return_arr |= tmp_arr;
                
                continue;
            }
        }

        /* It may be a domain. We check this first since we do not
         * want people to use domain-names as aliases.
         */
        if (sizeof(tmp_arr = SECURITY->query_domain_members(name)))
        {
            return_arr |= tmp_arr;
            continue;
        }

        /* All of the below aliases require forced checking. */
        force_check = 1;

        /* You may want to select the players that have global read access.
         * (Only for wizards.)
         */
        if (is_wizard &&
            ((name == "global") ||
             (name == "globals")))
        {
            if (sizeof(tmp_arr = m_indices(SECURITY->query_global_read())))
                return_arr |= tmp_arr;
            continue;
        }

        /* It may be a helper group. Treat "playerarch" as "aop". */
        if (name == "playerarch")
            name = "aop";

        if (sizeof(tmp_arr = SECURITY->query_team_list(name)))
        {
            return_arr |= tmp_arr;
            continue;
        }

        /* It may be a global alias. The order of these aliases is
         * such that the one with the highest priority is checked
         * first.
         */
        if (file_size(ALIAS_DIR + name) > 0)
        {
            return_arr |= explode(read_file((ALIAS_DIR + name)), "\n");
            continue;
        }

        /* It may be an area-related alias. */
        if (IN_ARRAY(name, m_indices(gAliases)))
        {
            return_arr |= gAliases[name];
            continue;
        }

        /* It may be a personal alias. */
        if (IN_ARRAY(name, m_indices(pAliases)))
        {
            return_arr |= pAliases[name];
            continue;
        }

        /* By the time we reach this, all aliases should have been
         * resolved.
         */
        WRITE("The alias \"" + name + "\" could not be resolved and was " +
            "discarded. Please report this to a knowledgeble person, " +
            "preferably an archwizard.\n");
    }

    /* Some aliases may return names of players that are not valid. Here
     * we check for that so all addressees will always be valid players.
     */
    if (force_check)
    {
        tmp_arr = ({ });
        foreach(string pname: return_arr)
        {
            if (!SECURITY->exist_player(pname))
            {
                tmp_arr |= ({ pname });
            }
        }

        if (sizeof(tmp_arr))
        {
            return_arr -= tmp_arr;
            WRITE("Alias resolving yielded some invalid names. Those names " +
                "are discarded: " + COMPOSITE_WORDS(sort_array(tmp_arr)) +
                 ".\n");
        }
    }

    /* We capitalize the names of the players to send the mail to. */
    return map(return_arr, capitalize);
}


/*
 * Function name: check_mailing_list
 * Description  : Check if all names on the list are valid. Reports the
 *                first non-valid name that is encountered. Takes aliases
 *                in account.
 * Arguments    : string *list - a list of strings with names. 
 * Returns      : string       - the entry in the array that is not valid
 *                               is returned enquoted.
 *                0            - if all members are valid
 */
public string
check_mailing_list(string *list)
{
    if (!sizeof(list))
        return 0;

    foreach(string name: list)
    {
        name = lower_case(name);
        if (!strlen(name))
            return "''";

        /* Some domains are not allowed. */
        if (IN_ARRAY(name, NON_DOMAINS))
            return ("'" + name + "'");

        /* If it is a player or domain-name, it is oke. */
        if (SECURITY->exist_player(name) ||
            IN_ARRAY(capitalize(name), SECURITY->query_domain_list()))
            continue;

        /* If it is any sort of alias, allow it. */
        if ((environment()->query_wiz_level() &&
                IN_ARRAY(LANG_SWORD(name), WIZ_N)) ||
            IN_ARRAY(name, m_indices(pAliases)) ||
            IN_ARRAY(name, m_indices(gAliases)) ||
            IN_ARRAY(name, SPECIAL_TYPES) ||
            IN_ARRAY(name, SECURITY->query_teams()) ||
            (name == "playerarch") ||
            (file_size(ALIAS_DIR + name) > 0))
            continue;

        /* The name is not valid, return the value. */
        return ("'" + name + "'");
    }

    return 0;
}


/*
 * Function name:   cleanup_string
 * Description:     Clean up the given string and turn it into a nice string
 *                  with only commas separating the substrings.
 *                  Leaves a string like "1-4" alone.
 * Arguments:       str: the string that is going to be cleaned up
 * Returns:         The clean string
 */
public string
cleanup_string(string str)
{
    /* No string, no show.  */
    if (!strlen(str))
        return "";

    /* Change all spaces to commas. */
    str = implode(explode(lower_case(str), " "), ",");

    /* Remove all empty spaces from the string. */
    str = implode((explode(str, ",") - ({ "", 0 }) ), ",");

    return str;
}


#ifdef MAX_IN_MORTAL_BOX
/*
 * Function name: too_many_messages
 * Description  : This function will return true if the number of messages
 *                in the mailbox is too large. If this is the case, a message
 *                is printed. Wizards are not limited in the size of their
 *                mailbox.
 * Returns      : int 1/0 - too many messages or not.
 */
static int
too_many_messages()
{
    int limit = MAX_IN_MORTAL_BOX;

    /* There is no limit for wizards. */
    if (environment()->query_wiz_level())
        return 0;

    /* Players performing a special function in a guild may have a higher
     * limit.
     */
#ifdef EXTRA_IN_GUILD_BOX
    if (environment()->query_guild_leader())
        limit += EXTRA_IN_GUILD_BOX;
#endif EXTRA_IN_GUILD_BOX

    /* Number of read and answered message does not exceed the limit. */
    if (gRead_count <= limit)
        return 0;

    /* Give a message and return true. */
    WRITE("The number of messages in your mailbox that have been read or " +
        "answered exceeds " + limit + ". Therefore, you will not be able " +
        "send mail until you delete some old messages. -- Postmaster\n");
    return 1;
}
#endif MAX_IN_MORTAL_BOX


/*
 * Function name: help
 * Description  : Show all commands.
 * Arguments    : string str - the argument to the help command.
 */
static void
help(string str)
{
    if (!strlen(str))
        str = "general";

    if (file_size(READER_HELP + str) <= 0)
    {
        WRITE("No help page on '" + str + "' available. The available " +
            "topics are 'expr' and 'header' and ? without argument.\n");
        return;
    }

    cat(READER_HELP + str);

    LOOP;
}


/*
 * Function name: from
 * Description  : Show the headers of all messages.
 * Arguments    : string str - the command line argument.
 * Returns      : int        - 1/0 - success/failure
 */
static int
from(string str)
{
    int    index;
    int    size;
    int    filter_new  = 0;
    int    filter_name = 0;
    int    filter_star = 0;
    int    found       = 0;
    string tmp;
    string flag = "";
    string *list;
    string *words;
    string filter_text;

    LOAD_PLAYER;

    if (sizeof(pMessages) == 0)
    {
        WRITE("No messages.\n");
        return 1;
    }

    CHECK_SO_BUSY;

    if (strlen(str))
    {
        words = explode(str, " ");

        if (filter_new = IN_ARRAY("new", words))
            words -= ({ "new" });
        if (filter_star = IN_ARRAY("*", words))
            words -= ({ "*" });
        else if (filter_star = IN_ARRAY("star", words))
            words -= ({ "star" });

        /* List will be all names, i.e. words not starting with an asterisk. */
        list = filter(words, &not() @ &wildmatch("\\**", ));
        str = implode(list, " ");
        /* Words should be the filter. */
        words -= list;
	/* We'll just accept only the first filter text. */
        if (sizeof(words))
            filter_text = lower_case(words[0] + "*");
    }

    if (strlen(str))
    {
        filter_name = 1;
    
        /* Transform the commandline argument to an array of addressees. */
        list = explode(cleanup_string(str), ",");

        /* We'd better check it before we start entering the message. */
        if (strlen(tmp = check_mailing_list(list)))
        {
            WRITE("No such addressee (player, domain, alias): " + tmp + ".\n");
            LOOP;
            return 1;
        }

        list = map(convert_to_names(list), capitalize);
    }

    size  = sizeof(pMessages);
    WRITE("Your mailbox contains " + size + " message" +
        ((size == 1) ? "" : "s") + ".\n");

    index = -1;
    while(++index < size)
    {
        /* If the player only wants to see the unread mail, don't list
         * the mail that was already read.
         */
        if (filter_new &&
            (READ_FLAG(pMessages[index][MAIL_READ]) || ANSWERED_FLAG(pMessages[index][MAIL_READ])))
            continue;

        if (filter_star &&
            !STARRED_FLAG(pMessages[index][MAIL_READ]))
            continue;
        
        if (filter_name &&
            !IN_ARRAY(pMessages[index][MAIL_FROM], list))
            continue;

        if (strlen(filter_text) &&
            !wildmatch(filter_text, lower_case(pMessages[index][MAIL_SUBJ])))
            continue;

        /* We do not want too long subjects. */
        if (strlen(tmp = pMessages[index][MAIL_SUBJ]) > 38)
            tmp = tmp[0..34] + "...";

        if (gTo_delete[(index + 1)])
            flag = "D";
        else if (ANSWERED_FLAG(pMessages[index][MAIL_READ]))
            flag = "A";
        else if (READ_FLAG(pMessages[index][MAIL_READ]))
            flag = "R";
        else
            flag = " ";

        found = 1;
        WRITE(sprintf("%2d: %-11s %-4s: %-38s %1s%1s %2s %6s %4s\n",
            (index + 1),
            pMessages[index][MAIL_FROM],
            SUBJECT_SHORT[pMessages[index][MAIL_REPLY]],
            tmp,
            flag,
            (pMessages[index][MAIL_READ] & MSG_STARRED ? "*" : " "),
            pMessages[index][MAIL_LENGTH],
            MAKE_DATE(pMessages[index][MAIL_DATE]),
            DATE_YEAR(pMessages[index][MAIL_DATE])));
    }

    if (!found)
    {
        if (filter_name)
            WRITE("No " + (filter_new ? "unread " : "") + "messages from " +
		  COMPOSITE_WORDS(list) + " found.\n");
        else if (filter_new)
            WRITE("All messages in your mailbox were read before.\n");
    }

#ifdef WARNING_TOO_MUCH_IN_BOX
    if (size > WARNING_TOO_MUCH_IN_BOX)
        WRITE("\nPOSTMASTER -> It is time to clean up!\n" +
            "POSTMASTER -> You have more than " + WARNING_TOO_MUCH_IN_BOX +
            " messages in your mailbox!\n\n");
#endif WARNING_TOO_MUCH_IN_BOX

    LOOP;
    return 1;
}


/*
 * Function name: update_new_mail_flag
 * Description  : Sets the new mail flag of the mailbox. By default it is
 *                "read", but if we have unread messages, mark it so.
 */
static void
update_new_mail_flag()
{
    int index;

    pNew_mail = FLAG_READ;
    /* We have set the flag to FLAG_READ, but if there is one message unread,
     * set it to FLAG_UNREAD. */
    index = sizeof(pMessages);
    while(--index >= 0)
    {
        if (!pMessages[index][MAIL_READ])
        {
            pNew_mail = FLAG_UNREAD;
            break;
        }
    }
}


/*
 * Function name: store_message
 * Description  : This function may be called from the mail reader when
 *                a wizard wants to WRITE a message to file.
 * Arguments    : string message  - the message to WRITE.
 *                string filename - the filename to WRITE to.
 */
static void
store_message(string message, string filename)
{
    int result;

    /* To save the file, we assume the euid of the person trying to send the
     * message. This way we prevent people from writing there they should
     * not.
     */
    seteuid(geteuid(environment()));
    result = write_file(filename, message);
    seteuid(getuid());

    if (result)
        WRITE("Message stored in " + filename + ".\n");
    else
        WRITE("File name " + filename + " could not be written.\n");
}


/*
 * Function name: construct_message
 * Description  : Construct the message text for printing or storing.
 * Arguments    : int to_print - the message number, index in the array.
 * Returns      : string - the message.
 */
static string
construct_message(int to_print)
{
    mapping message;
    string  tmp;

    /* Retrieve the message from disk. */
    message = restore_message(pMessages[to_print][MAIL_DATE]);

    /* Construct the message. */
    tmp = sprintf("Message: %d\nFrom   : %s\n%-7s: %s\n",
        (to_print + 1),
        pMessages[to_print][MAIL_FROM],
        SUBJECT_LONG[pMessages[to_print][MAIL_REPLY]],
        pMessages[to_print][MAIL_SUBJ]);

    /* Only print the 'to' if there are more recipients. */
    if (message[MSG_TO] != capitalize(environment()->query_real_name()))
        tmp += HANGING_INDENT("To     : " +
            COMPOSITE_WORDS(explode(message[MSG_TO], ",")), 9, 0);

    if (message[MSG_CC] != "")
        tmp += HANGING_INDENT("CC     : " +
            COMPOSITE_WORDS(explode(message[MSG_CC], ",")), 9, 0);

    tmp += "Date   : " + ctime(pMessages[to_print][MAIL_DATE]) +
        "\n\n" + message[MSG_BODY] + "\n";

    return tmp;
}


/*
 * Function name: print_message
 * Description  : This function prints all lines of a message. It can use
 *                more, be printed directly or even written to file.
 * Arguments    : string filename - the file to print it to (wiz-only)
 */
varargs static void
print_message(string filename)
{
    string  header;
    string  message;
    int     to_print = gCurrent - 1;
    int     index;
    int     size;

    /* Check if gCurrent is within the bounds. */
    if (!check_valid_msg_num(gCurrent))
    {
        WRITE("Illegal message number " + gCurrent + ".\n");
        gCurrent = gPrevious;
        LOOP;
        return;
    }

    /* Mark the message as read, unless it was read before. */
    if (!READ_FLAG(pMessages[to_print][MAIL_READ]))
    {
        pMessages[to_print][MAIL_READ] |= MSG_READ;

#ifdef MAX_IN_MORTAL_BOX
        gRead_count++;
#endif MAX_IN_MORTAL_BOX
    }

    /* If the current new-mail-flag is set to FLAG_UNREAD, we check
     * whether the last unread mail was read.
     */
    if (pNew_mail == FLAG_UNREAD)
    {
        update_new_mail_flag();
    }
    SAVE_MAIL;

    /* Construct the message. */
    message = construct_message(to_print);

    /* Wizard may want to write the message to a file.  */
    if (strlen(filename))
    {
        store_message(message, filename);
        loop();
        return;
    }

    /* In order to prevent people from having runtime errors for trying
     * to read a text that is too long, make a check here. It automatically
     * invokes more if the note is larger than 100 lines.
     */
    if (!gUse_more && (pMessages[to_print][MAIL_LENGTH] == MAX_NO_MREAD))
    {
        WRITE("Too long message. More automatically invoked!\n");
        gUse_more = 1;
    }

    /* If the player wants to use more, we call the more-reader with
     * the text and if the mailreader is active, we let it call us
     * "back" when the player is done reading.
     */
    if (gUse_more)
        environment()->more(message, 0, done_reading_more);
    else
    {
        WRITE(message);
        loop();
    }
}


/*
 * Function name: read
 * Description  : Read mail, with more or straight.
 * Arguments    : string str - User input
 * Returns      : int        - 1/0 - success/failure.
 */
static int
read(string str)
{
    LOAD_PLAYER;

    if (!sizeof(pMessages))
    {
        notify_fail("No messages.\n");
        return 0;
    }

    /* If the player didn't give an argument, start up the reader. */
    if (!strlen(str))
    {
        gIs_reading = 1;
        loop();
        return 1;
    }

    /* If the player specified a number, he might be reading a message,
     * else, the command was not meant for us.
     */
    if (sscanf(str, "%d", gCurrent) != 1)
        return 0;

    /* If we do not have such message, someone might be reading something
     * else.
     */
    if (!check_valid_msg_num(gCurrent))
    {
        notify_fail("No mail with message number " + gCurrent + ".\n");
        return 0;
    }

    CHECK_SO_BUSY;

    gIs_reading = 1;
    gUse_more = (query_verb() == "mread");
    environment()->add_prop(LIVE_S_EXTRA_SHORT, " is reading mail");
    print_message(); /* will eventually call loop(). */
    return 1;
}


/*
 * Function name: parse_range
 * Description  : Try to compile a list of message numbers from the string
 *                You should clean the string first with cleanup_string.
 *                The list of numbers is sorted and all doubles are
 *                filtered out. All elements are checked for validity.
 * Arguments    : string str - the string that is going to be parsed
 *                             Typical accepted string: 10,1-4,2
 *                             Would return: ({ 1, 2, 3, 4, 10 });
 *                             No input means process gCurrent.
 * Returns      : int - 0 if the string could not be parsed
 *                      otherwise an array of message numbers
 */
static int *
parse_range(string str)
{
    int    *range = ({ });
    int    *subrange;
    string *parts = explode(str, ",");
    int     left;
    int     right;
    int     index;

    /* If no string was entered, the gCurrent is returned if that is a
     * valid message number.
     */
    if (!strlen(str))
    {
        if (check_valid_msg_num(gCurrent))
            return ({ gCurrent });
        else
            return 0;
    }

    foreach(string part: parts)
    {
        /* The parts can either consist of a single number of a range. */
        if (sscanf(part, "%d-%d", left, right) == 2)
        {
            /* Too large or too small numbers. */
            if ((!check_valid_msg_num(left)) ||
                (!check_valid_msg_num(right)))
                return 0;

            /* A descending range is a range too. */
            if (right < left)
            {
                index = left;
                left  = right;
                right = index;
            }

            /* Add all elements from the selected subrange to the range. */
            index = left - 1;
            while(++index <= right)
            {
                range |= ({ index });
            }
        }
        else
        {
            index = atoi(part);
            /* Too large to too small number. If parts[index] is a string
             * value, index will be zero and therewith eliminated.
             */
            if (!check_valid_msg_num(index))
            {
                return 0;
            }
            /* Add the element to the range, making sure that it is in the
             * range only once.
             */
            range |= ({ index });
        }
    }

    /* Sort the array before returning it. There really is not big deal in
     * this since the range is already unique.
     */
    return sort_array(range);
}


/*
 * Function name: archive
 * Description  : Archive mail messages. The archived messages may optionally
 *                be marked for deletion. They will be actually deleted when
 *                you quit the reader.
 * Arguments    : int * archive_arr - the numbers of the messages to archive.
 *                int mark_del - if true, mark the archived mails for deletion.
 */
static void
archive(int *archive_arr, int mark_del)
{
    int   *marked = ({ });
    int    seq;
    string dir;
    string filename;

    /* Only 'normal' wizards and above can store messages.  */
    if (SECURITY->query_wiz_rank(environment()->query_real_name()) <
        WIZ_NORMAL)
    {
        WRITE("You cannot archive messages since you have no home directory.\n");
        loop();
        return;
    }

    if ((!pointerp(archive_arr)) || (!sizeof(archive_arr)))
    {
        WRITE("No messages specified to archive or syntax error.\n");
        loop();
        return;
    }

    if (sizeof(archive_arr) > 100)
    {
        archive_arr = archive_arr[..99];
        write("Archiving restricted to at most 100 messages at a time.\n");
    }

    dir = SECURITY->query_wiz_path(environment()->query_real_name()) + "/private";
    switch(file_size(dir))
    {
    case -2:
        break;
    case -1:
        write("Creating private directory: " + dir + "\n");
        mkdir(dir);
        break;
    default:
        WRITE("A file " + dir + " exists. Cannot archive mail.\n");
        loop();
        return;
    }

    dir += "/" + MAIL_ARCHIVE_DIR;
    switch(file_size(dir))
    {
    case -2:
        break;
    case -1:
        write("Creating archive directory: " + dir + "\n");
        mkdir(dir);
        break;
    default:
        WRITE("A file " + dir + " exists. Cannot archive mail.\n");
        loop();
        return;
    }

    foreach(int archive: archive_arr)
    {
        filename = dir + "/" + pMessages[archive - 1][MAIL_FROM] + "-" +
            TIME2FORMAT(pMessages[archive - 1][MAIL_DATE], "yyyymmdd");
        if (file_size(filename) > 0)
        {
            seq = 0;
            while(file_size(filename + ALPHABET[seq..seq]) > 0)
                seq++;

            filename = filename + ALPHABET[seq..seq];
        }

        write_file(filename, construct_message(archive - 1));
        marked += ({ archive });

        if (mark_del)
            gTo_delete[archive] = 1;
    }

    if (sizeof(marked))
        WRITE(HANGING_INDENT("Archived" +
            (mark_del ? " (and marked for deletion)" : "") + ": " +
            COMPOSITE_WORDS(STRING(sort_array(marked))) + ".", 4, 0));
    else
        WRITE("No messages archived.\n");

    loop();
}


/*
 * Function name: delete
 * Description  : Marks one or more message for deletion. They will be
 *                actually deleted when you quit the reader.
 * Arguments    : int * del_arr - the numbers of the messages to delete.
 */
static void
delete(int *del_arr)
{
    int *marked = ({ });
    int *double = ({ });

    if ((!pointerp(del_arr)) || (!sizeof(del_arr)))
    {
        WRITE("No messages specified to delete or syntax error.\n");
        loop();
        return;
    }

    foreach(int to_del: del_arr)
    {
        if (gTo_delete[to_del])
            double += ({ to_del });
        else
        {
            gTo_delete[to_del] = 1;
            marked += ({ to_del });
        }
    }

    if (sizeof(double))
        WRITE(HANGING_INDENT("Already marked for deletion: " +
            COMPOSITE_WORDS(STRING(sort_array(double))) + ".", 4, 0));

    if (sizeof(marked))
        WRITE(HANGING_INDENT("Marked for deletion: " +
            COMPOSITE_WORDS(STRING(sort_array(marked))) + ".", 4, 0));
    else
        WRITE("No messages marked for deletion.\n");

    loop();
}


/*
 * Function name: undelete
 * Description  : Unmarks one or more messages for deletion.
 * Arguments    : int * mark_arr - the numbers of the messages to unmark.
 */
static void
undelete(int *mark_arr)
{
    int *marked = ({ });
    int *double = ({ });

    if ((!pointerp(mark_arr)) || (!sizeof(mark_arr)))
    {
        WRITE("No messages specified to unmark or syntax error.\n");
        loop();
        return;
    }

    foreach(int to_unmark: mark_arr)
    {
        if (gTo_delete[to_unmark])
        {
            m_delkey(gTo_delete, to_unmark);
            marked += ({ to_unmark });
        }
        else
            double += ({ to_unmark });
    }

    if (sizeof(double))
        WRITE(HANGING_INDENT("Not marked for deletion: " +
            COMPOSITE_WORDS(STRING(sort_array(double))) + ".", 4, 0));

    if (sizeof(marked))
        WRITE(HANGING_INDENT("Unmarked from deletion: " +
            COMPOSITE_WORDS(STRING(sort_array(marked))) + ".", 4, 0));
    else
        WRITE("No messages unmarked from deletion.\n");

    loop();
}


/*
 * Function name: star
 * Description  : Marks one or more message as special.
 * Arguments    : int * mark_arr - the numbers of the messages to mark.
 */
static void
star(int *mark_arr)
{
    int *marked = ({ });
    int *double = ({ });

    if ((!pointerp(mark_arr)) || (!sizeof(mark_arr)))
    {
        WRITE("No messages specified to star or syntax error.\n");
        loop();
        return;
    }

    foreach(int to_star: mark_arr)
    {
        if (pMessages[to_star - 1][MAIL_READ] & MSG_STARRED)
        {
            double += ({ to_star });
        }
        else
        {
            pMessages[to_star - 1][MAIL_READ] |= MSG_STARRED;
            marked += ({ to_star });
        }
    }
    SAVE_MAIL;

    if (sizeof(double))
        WRITE(HANGING_INDENT("Already starred as special: " +
            COMPOSITE_WORDS(STRING(sort_array(double))) + ".", 4, 0));

    if (sizeof(marked))
        WRITE(HANGING_INDENT("Starred as special: " +
            COMPOSITE_WORDS(STRING(sort_array(marked))) + ".", 4, 0));
    else
        WRITE("No messages starred as special.\n");

    loop();
}


/*
 * Function name: unstar
 * Description  : Unmarks one or more messages as special.
 * Arguments    : int * mark_arr - the numbers of the messages to unmark.
 */
static void
unstar(int *mark_arr)
{
    int *marked = ({ });
    int *double = ({ });

    if ((!pointerp(mark_arr)) || (!sizeof(mark_arr)))
    {
        WRITE("No messages specified to unmark or syntax error.\n");
        loop();
        return;
    }

    foreach(int to_unmark: mark_arr)
    {
        if (pMessages[to_unmark - 1][MAIL_READ] & MSG_STARRED)
        {
            pMessages[to_unmark - 1][MAIL_READ] &= ~MSG_STARRED;
            marked += ({ to_unmark });
        }
        else
        {
            double += ({ to_unmark });
        }
    }
    SAVE_MAIL;

    if (sizeof(double))
        WRITE(HANGING_INDENT("Not unmarked as special: " +
            COMPOSITE_WORDS(STRING(sort_array(double))) + ".", 4, 0));

    if (sizeof(marked))
        WRITE(HANGING_INDENT("Unmarked as sepcial: " +
            COMPOSITE_WORDS(STRING(sort_array(marked))) + ".", 4, 0));
    else
        WRITE("No messages unmarked as special.\n");

    loop();
}


/*
 * Function name: send_mail_safely
 * Description  : This is the function that actually sends the mail in
 *                small portions. We do not want to run into eval-cost
 *                limits.
 * Arguments    : string *dest_array - the people to send the mail to.
 *                string name - the (capitalized) name of the author.
 */
static void
send_mail_safely(string *dest_array, string name)
{
    int     index;
    int     max = MIN(sizeof(dest_array), MAX_CYCLE);
    mapping player_mail;
    string  recipient;
    object  obj;

    /* No recipients (VERY strange). */
    if (max <= 0)
    {
        gBusy = 0;
        WRITE("No mail sent.\n");
        LOOP;
        return;
    }

    index = -1;
    while(++index < max)
    {
        recipient = lower_case(dest_array[index]);

        /* Restore the mailbox of the player. */
        player_mail = restore_mail(recipient);

        /* Add the message to the mailbox of the player and set the
         * new-mail-flag to new mail.
         */
        player_mail[MAIL_MAIL] += ({ gTo_send });
        player_mail[MAIL_NEW_MAIL] = FLAG_NEW;

        /* Save the mailbox of the player. */
        save_mail(player_mail, recipient);

        /* Tell the target he has new mail and if necessary notify the
         * mailreader s/he is carrying.
         */
        if (objectp(obj = find_player(recipient)))
        {
            /* Tell only wizards the subject to refrain mortal players from
             * using the mailreader as a tell-line. Rather, mortals get to
             * see the subject too if the author is a wizard.
             */
            if (environment()->query_wiz_level() || obj->query_wiz_level())
                tell_object(obj,
                    "\nPostmaster tells you that you have new mail (# " +
                    sizeof(player_mail[MAIL_MAIL]) + ") from " + name +
                    ",\nconcerning " +
                    SUBJECT_SHORT[gTo_send[MAIL_REPLY]] + ": " +
                    gSubject + " (" + gTo_send[MAIL_LENGTH] + ")\n\n");
            else
                tell_object(obj,
                    "\nPostmaster tells you that you have new mail from " +
                    name + ".\n\n");

            /* If the player is carrying a mailreader, invalidate it, so
             * that the player cannot destruct the message by accident.
             */
            if (objectp(obj = present(READER_ID, obj)))
                obj->invalidate();
        }
    }

    WRITE(HANGING_INDENT("Batch sent to: " +
        COMPOSITE_WORDS(dest_array[0..(max - 1)]) + ".", 4, 0));

    dest_array = dest_array[max..];

    /* If there are more recipients, put in a new alarm, if not,
     * unset gBusy to allow operation of the reader again.
     */
    if (sizeof(dest_array))
        set_alarm(1.0, 0.0, &send_mail_safely(dest_array, name));
    else
    {
        WRITE("Mail sent.\n");
        gBusy = 0;
        LOOP;
    }
}


/*
 * Function name: send_mail
 * Description  : This function sends the mail to the recipients and
 *                makes sure that if people are reading mail their readers
 *                are notified. To avoid the "evaluation too long" error,
 *                send_mail_safely is called to store the message in the
 *                destinations mail-files.
 * Arguments    : string name - the (capitalized) name of the author.
 */
static void
send_mail(string name)
{
    int     send_time = time();
    int     length    = sizeof(explode(gMessage, "\n"));
    string *addressees;

    /* Set a flag so the main loop won't start yet. It disallows
     * the player to handle the reader until all mail has been sent.
     */
    gBusy = 1;

    /* We have to convert gTo and gCc into strings containing only
     * player names.
     */
    gTo  = convert_to_names(gTo);
    gCc  = convert_to_names(gCc);
    addressees = ((gTo - gCc) + gCc);

    /* If you mail yourself, you will rather CC yourself. */
    if (IN_ARRAY(name, gTo))
    {
        gTo -= ({ name });
        gCc |= ({ name });
    }

    /* We take the current time and check if that time already exists as
     * a message. This can only happen if two players send mail at exactly
     * the same time (matching seconds). When that happens, we go for the
     * next available second.
     */
    while(file_size(FILE_NAME_MESSAGE(send_time, HASH_SIZE) + ".o") > 0)
        send_time++;

    /* Construct the mail-message and save it. */
    gTo_send = ([ MSG_TO      : implode(gTo, ","),
                  MSG_CC      : implode(gCc, ","),
                  MSG_ADDRESS : implode(addressees, ","),
                  MSG_BODY    : gMessage ]);

    /* It is possible that the mail reader cannot save the message into
     * a file. One possibility is that the file system is full. There are
     * other problems possible.
     */
    if (!save_message(gTo_send, send_time))
    {
        WRITE("POSTMASTER -> An error has occured. It has been impossible " +
            "to store the\n" +
            "POSTMASTER -> message into a file. The message has not been " +
            "sent.\n" +
            "POSTMASTER -> Please report this to an archwizard.\n");
        return;
    }

    /* If the mail is a reply, update the information in the players box. */
    if (REPLY_FLAG(gIs_reply))
    {
        /* gCurrent will be 0 if this is a resend */
        if (gCurrent > 0)
        {
            LOAD_PLAYER;
    
#ifdef MAX_IN_MORTAL_BOX
            /* The message was not read before, count it as read now. */
            if (!pMessages[gCurrent - 1][MAIL_READ])
                gRead_count++;
#endif MAX_IN_MORTAL_BOX
    
            pMessages[gCurrent - 1][MAIL_READ] |= MSG_ANSWERED;
            SAVE_MAIL;
        }
    }

    /* Each recipient will get the same message, we can construct it here
     * and keep it global.
     */
    gTo_send = ([ MAIL_FROM   : name,
                  MAIL_SUBJ   : gSubject,
                  MAIL_DATE   : send_time,
                  MAIL_REPLY  : REPLY_FLAG(gIs_reply) | FORWARD_FLAG(gIs_reply),
                  MAIL_LENGTH : ((length >= 100) ? MAX_NO_MREAD :
                                    sprintf("%2d", length)),
                  MAIL_READ   : MSG_UNREAD ]);

    if (sizeof(addressees) > MAX_CYCLE)
        WRITE("Busy sending mail. Please wait.\n");

    send_mail_safely(addressees, name);
}


/*
 * Function name: done_editing
 * Description  : When the player is done editing the message, this
 *                function is called from the edit-object.
 * Arguments    : string text - the message the player typed
 */
public void
done_editing(string text)
{
    environment()->remove_prop(LIVE_S_EXTRA_SHORT);

    if (MASTER_OB(previous_object()) != EDITOR_OBJECT)
    {
        WRITE("MAILREADER: done_editing may only be called by the editor.\n");
        return;
    }

    if (!strlen(text))
    {
        WRITE("Aborted. No text.\n");
        LOOP;
        return;
    }

    gMessage = text;
    send_mail(capitalize(environment()->query_real_name()));
}


/*
 * Function name: get_cc
 * Description  : Gets as input the string of people that should receive a
 *                carbon copy of this message. If elements in the list
 *                are not valid, the player is prompted again.
 * Arguments    : string str - the string with the list
 */
static void
get_cc(string str)
{
    string error;
    object editor;

    if (str == "~q")
    {
        WRITE("Aborted.\n");
        LOOP;
        return;
    }

    /* Take the string and turn it into the format we use. */
    str = cleanup_string(str);

    if (strlen(str))
    {
        gCc = explode(str, ",");

        /* Check the CC mailing list before we allow the player to
         * type the message.
         */
        if (strlen(error = check_mailing_list(gCc)))
        {
            WRITE("No such addressee (player, domain or alias): " +
                error + ".\nCC: ");
            input_to(get_cc);
            return;
        }
    }
    else
        gCc = ({ });

    /* Do not send a CC to people that already get the message. */
    gCc -= gTo;

    /* If the player has set auto-cc on, we add him/her to the gCc list,
     * but do not autocc forwards.
     */
    if (pAutocc && !FORWARD_FLAG(gIs_reply))
    {
        str = capitalize(environment()->query_real_name());
        gCc |= ({ str });
    }

    /* Forward or resend without editing required. */
    if ((RESEND_FLAG(gIs_reply) || FORWARD_FLAG(gIs_reply)) &&
        !DO_EDIT_FLAG(gIs_reply))
    {
        send_mail(capitalize(environment()->query_real_name()));
        return;
    }

    environment()->add_prop(LIVE_S_EXTRA_SHORT, " is writing mail");

    editor = clone_object(EDITOR_OBJECT);
    editor->set_activity("a mail message.");
    editor->edit("done_editing", gMessage, !!FORWARD_FLAG(gIs_reply));
}


/*
 * Function name: editor_add_to_cc_list
 * Description  : From the editor, we may add names to the CC-list.
 * Arguments    : string *names - the names to add to the CC-list.
 */
public void
editor_add_to_cc_list(string *names)
{
    if (function_exists("create_object", previous_object()) !=
        EDITOR_OBJECT)
    {
        write("Illegal call to editor_add_to_cc_list()\n");
        return;
    }

    gCc |= (names - gTo);
}


/*
 * Function name: get_subject
 * Description  : Let the player type the subgect of the mail. If the
 *                player replies to another message, the input may be
 *                an empty string.
 * Arguments    : string str - the subject from the player.
 */
static void
get_subject(string str)
{
    if (str == "~q")
    {
        gTo = ({ });
        WRITE("Aborted.\n");
        LOOP;
        return;
    }

    /* A subject should be short. The message is the body ;-) */
    if (strlen(str) > MAX_SUBJECT)
    {
        WRITE("Please keep your subject shorter than " + MAX_SUBJECT +
            " characters.\nSubject: ");
        input_to(get_subject);
        return;
    }

    /* If the player didn't specify a subject, he may be replying to an
     * old message, or else it aborts.
     */
    if (!strlen(str) || (str == "-"))
    {
        if (!REPLY_FLAG(gIs_reply))
        {
            WRITE("No subject, aborting.\n");
            LOOP;
            return;
        }
    }
    else
    {
        /* If we edit the subject, it's not a reply anymore. */
        gIs_reply &= ~MSG_IS_REPLY;
        gSubject  = str;
    }

    WRITE("CC: ");
    input_to(get_cc);
}


/*
 * Function name: mail
 * Description  : Send mail to one or more players.
 * Arguments    : string str - the commandline argument that contains
 *                             a list of the players to mail.
 * Returns      : int        - 1/0 - success/failure.
 */
static int
mail(string str)
{
    string *list;
    string  error;

    if (!strlen(str))
    {
        WRITE("No destination.\n");
        LOOP;
        return 1;
    }

#ifdef MAX_IN_MORTAL_BOX
    /* Mortals with too many messages in this box cannot send mail. */
    if (too_many_messages())
    {
        LOOP;
        return 1;
    }
#endif MAX_IN_MORTAL_BOX

    /* Transform the commandline argument to an array of addressees. */
    list = explode(cleanup_string(str), ",");

    /* We'd better check it before we start entering the message. */
    if (strlen(error = check_mailing_list(list)))
    {
        WRITE("No such addressee (player, domain, alias): " + error + ".\n");
        LOOP;
        return 1;
    }

    /* The player cannot enter new mail while the previous message is
     * still being sent.
     */
    CHECK_SO_BUSY;

    gMessage  = "";
    gTo       = list;
    gIs_reply = 0;

    WRITE("Subject: ");

    input_to(get_subject);

    return 1;
}


/*
 * Function name: resend
 * Description  : Send the last entered message to some more people.
 * Arguments    : string str - the people to send the message go.
 * Returns      : int        - 1/0 - success/failure.
 */
static int
resend(string str)
{
    string *list;
    string  error;

    if (!strlen(gMessage))
    {
        WRITE("There is no last message to resend!\n");
        LOOP;
        return 1;
    }

#ifdef MAX_IN_MORTAL_BOX
    /* Mortals with too many messages in this box cannot send mail. */
    if (too_many_messages())
    {
        LOOP;
        return 1;
    }
#endif MAX_IN_MORTAL_BOX

    if (!strlen(str))
    {
        WRITE("Resend message to whom? No destination.\n");
        LOOP;
        return 1;
    }

    /* Transform the commandline argument to an array of addressees. */
    list = explode(cleanup_string(str), ",");

    /* We'd better check it before we start entering the message. */
    if (strlen(error = check_mailing_list(list)))
    {
        WRITE("No such addressee (player, domain, alias): " + error + ".\n");
        LOOP;
        return 1;
    }

    CHECK_SO_BUSY;

    gTo = list;
    gIs_reply |= MSG_IS_RESEND;

    WRITE("CC: ");

    input_to(get_cc);

    return 1;
}


/*
 * Function name: forward
 * Description  : This function enables the user to redirect incoming
 *                mail to other people without retyping it. Technically,
 *                the forward functionality is a front to resend.
 * Arguments    : string str   - the list of people to forward the mail to
 *                int    alter - if true, edit the outgoing message
 */
static void
forward(string str, int alter)
{
    int     to_forward = gCurrent - 1;
    mapping message;

#ifdef MAX_IN_MORTAL_BOX
    /* Mortals with too many messages in this box cannot send mail. */
    if (too_many_messages())
    {
        loop();
        return;
    }
#endif MAX_IN_MORTAL_BOX

    if (!strlen(str))
    {
        WRITE("Forward message to whom? No destination.\n");
        loop();
        return;
    }

    /* Check if gCurrent is within the bounds. */
    if (!check_valid_msg_num(gCurrent))
    {
        WRITE("Illegal message number " + gCurrent + ". Cannot forward.\n");
        gCurrent = gPrevious;
        loop();
        return;
    }

    /* Set variables up for forwarded mail. We assign some global values
     * and then call the the resend function to handle the rest. First,
     * however, we restore the message itself.
     */
    message   = restore_message(pMessages[to_forward][MAIL_DATE]);
    gSubject  = pMessages[to_forward][MAIL_SUBJ];
    gIs_reply = MSG_IS_FORWARD;
    gMessage  = "Author : " + pMessages[to_forward][MAIL_FROM] + "\n";

    /* Only print the 'to' if there are more recipients. */
    if (message[MSG_TO] != capitalize(environment()->query_real_name()))
    {
        gMessage += HANGING_INDENT("To     : " +
            COMPOSITE_WORDS(explode(message[MSG_TO], ",")), 9, 0);
    }

    if (message[MSG_CC] != "")
    {
        gMessage += HANGING_INDENT("CC     : " +
            COMPOSITE_WORDS(explode(message[MSG_CC], ",")), 9, 0);
    }

    gMessage += "Date   : " +
        ctime(pMessages[to_forward][MAIL_DATE]) + "\n\n" + message[MSG_BODY];

    if (alter)
    {
        gIs_reply |= MSG_DO_EDIT;
        gMessage = "\n> " + implode(explode(gMessage, "\n"), "\n> ") + "\n";
    }

    resend(str);
}


/*
 * Function name: reply_to_cc
 * Description  : Ask confirmation about replying to Cc: people as well.
 * Arguments    : string str - abort with ~q or else y/n.
 */
static void
reply_to_cc(string str)
{
    if (str == "~q")
    {
        gTo = ({ });
        WRITE("Aborted.\n");
        loop();
        return;
    }

    if (!strlen(str) ||
        (lower_case(str[0..0]) != "y"))
        gTo = ({ pMessages[(gCurrent - 1)][MAIL_FROM] });
    else
        gTo = ({ pMessages[(gCurrent - 1)][MAIL_FROM] }) + gTo;

    WRITE("Press return (or '-') to get 'Re: " + gSubject + "'.\nSubject: ");

    input_to(get_subject);
}


/*
 * Function name: reply
 * Description  : Send a reply to the a message.
 * Arguments    : int include - if true, include the message we are
 *                              replying to.
 */
static void
reply(int include)
{
    mapping message;
    int     reply_to = gCurrent - 1;

    /* Check if the current message number is valid. */
    if (!check_valid_msg_num(gCurrent))
    {
        WRITE("Illegal message number " + gCurrent + ".\n");
        gCurrent = gPrevious;
        loop();
        return;
    }

#ifdef MAX_IN_MORTAL_BOX
    /* Mortals with too many messages in this box cannot send mail. */
    if (too_many_messages())
    {
        loop();
        return;
    }
#endif MAX_IN_MORTAL_BOX

    /* We should check whether the author of the mail still exists. */
    if (!SECURITY->exist_player(lower_case(pMessages[reply_to][MAIL_FROM])))
    {
        WRITE("The author of message " + gCurrent + ", " +
            pMessages[reply_to][MAIL_FROM] +
            ", is no longer valid as return address.\n");
        loop();
        return;
    }

    /* We need to have the message for the cc-list and possibly
     * the message itself if the player wants it included.
     */
    message = restore_message(pMessages[reply_to][MAIL_DATE]);

    gSubject  = pMessages[reply_to][MAIL_SUBJ];
    gIs_reply = MSG_IS_REPLY;

    if (include)
    {
        gMessage = "On " + MAKE_LONG_DATE(pMessages[reply_to][MAIL_DATE]) +
            ", " + pMessages[reply_to][MAIL_FROM] + " writes:\n\n> " +
            implode(explode(message[MSG_BODY], "\n"), "\n> ") + "\n";
    }
    else
        gMessage = "";

    gTo = ({ });
    if (strlen(message[MSG_CC]) ||
        (message[MSG_TO] != capitalize(environment()->query_real_name())))
    {
        gTo = explode(message[MSG_TO], ",") +
            explode(message[MSG_CC], ",");
        gTo -= ({ capitalize(environment()->query_real_name()) });
        gTo -= ({ pMessages[reply_to][MAIL_FROM] });

        foreach(string pname: gTo)
        {
            if (!SECURITY->exist_player(lower_case(pname)))
            {
                WRITE("At least on of the CC-ed people, " + pname +
                    " is no longer valid as addressee. You cannot include " +
                    "the original CC-list, but you can naturally include " +
                    "individual players to CC to.\n");
                gTo = ({ });
                break;
            }
        }
    }

    if (sizeof(gTo))
    {
        WRITE("The original message was also sent to " +
            COMPOSITE_WORDS(gTo) + ".\nSend the reply to these people as " +
            "well (y/n)? [default: no] ");

        input_to(reply_to_cc);
    }
    else
        reply_to_cc("n");
}


/*
 * Function name: erase
 * Description  : This function will actually erase the messages. It can
 *                be called by the player of will automatically be called
 *                when the player leaves the reader.
 */
static void
erase()
{
    int     index;
    int     size;
    int     to_del;
    int     *del_arr = m_indices(gTo_delete);
    string  name     = environment()->query_real_name();
    mapping message;

    if (!sizeof(del_arr))
    {
        WRITE("No messages selected for deletion.\n");
        LOOP;
        return;
    }

    gTo_delete = ([ ]);
    WRITE(HANGING_INDENT("Deleting: " +
        COMPOSITE_WORDS(STRING(sort_array(del_arr))) + ".", 4, 0));

    index = sizeof(del_arr);
    while(--index >= 0)
    {
        to_del = del_arr[index] - 1;

        /* If you delete a message with a number smaller than gCurrent,
         * gCurrent has to be adjusted.
         */
        if (del_arr[index] < gCurrent)
        {
            gCurrent--;
        }

        /* Restore the message and remove the player from the list of
         * addressees.
         */
        message = restore_message(pMessages[to_del][MAIL_DATE]);
        message[MSG_ADDRESS] = implode(
            (explode(message[MSG_ADDRESS], ",") -
                ({ capitalize(name), "" }) ), ",");

        /* If the last addressee is removed, the message is deleted, else
         * the message file is saved again.
         */
        if (strlen(message[MSG_ADDRESS]) == 0)
        {
            rm(FILE_NAME_MESSAGE(pMessages[to_del][MAIL_DATE], HASH_SIZE) +
                ".o");
#ifdef DEBUG
            WRITE("DEBUG: Deleting message and removing file: " +
                del_arr[index] + ".\n");
#endif DEBUG
        }
        else
        {
            save_message(message, pMessages[to_del][MAIL_DATE]);
#ifdef DEBUG
            WRITE("DEBUG: Deleting message: " + del_arr[index] + ".\n");
#endif DEBUG
        }

#ifdef MAX_IN_MORTAL_BOX
        /* If the message was read or answered, uncount it. */
        if (pMessages[to_del][MAIL_READ])
            gRead_count--;
#endif MAX_IN_MORTAL_BOX

        /* Set the contents of the mail-element to zero. */
        pMessages[to_del] = 0;
    }

    /* Remove all mail-elements that were deleted, i.e. set to zero. */
    pMessages -= ({ 0 });

    /* If the player has messages left, of if he has personal aliases
     * set, save the mail-file.
     */
    if (sizeof(pMessages) || m_sizeof(pAliases))
    {
        /* If the player has no mail, but only aliases, we set the
         * new-mail-flag to FLAG_NO.
         */
        if (!sizeof(pMessages))
            pNew_mail = FLAG_NO;
        else
            update_new_mail_flag();

        SAVE_MAIL;
    }
    else
        rm(FILE_NAME_MAIL(name) + ".o");

    /* If the current message is being deleted, reset gCurrent. */
    if (IN_ARRAY(gCurrent, del_arr))
        gCurrent = 0;

    LOOP;
}


/*
 * Function name: quit
 * Description  : This function is called when the player wants to quit
 *                using the mail reader. It removes all messages that
 *                the player has marked for deletion.
 */
static void
quit()
{
    /* We first reset gIs_reading to ensure that loop() isn't called
     * at the end of erase().
     */
    gIs_reading = 0;

    /* If there are messages marked for deletion, we automatically
     * delete them.
     */
    if (m_sizeof(gTo_delete))
    {
        WRITE("Deleting marked messages before quitting...\n");
        erase();
    }

    /* Erase the important global variables. */
    gCurrent   = 0;
    gPrevious  = 0;
    gBusy      = 0;
    gUse_more  = 0;
    gTo_delete = ([ ]);
    gTo        = ({ });
    gCc        = ({ });
    gUse_more  = 0;

    environment()->remove_prop(LIVE_S_EXTRA_SHORT);
}


/*
 * Function name: store
 * Description  : Enable a wizard to store a message into a file in his
 *                or her private directory.
 * Arguments    : string str - the filename to store the message in.
 * Returns      : int        - 1/0 - success/failure
 */
static void
store(string str)
{
    string result;

    /* Only 'normal' wizards and above can store messages.  */
    if (SECURITY->query_wiz_rank(environment()->query_real_name()) <
        WIZ_NORMAL)
    {
        WRITE("You cannot store messages since you have no home directory.\n");
        loop();
        return;
    }

    /* Check if gCurrent is within the bounds */
    if (!check_valid_msg_num(gCurrent))
    {
        WRITE("Illegal message number " + gCurrent + ". Cannot store.\n");
        gCurrent = gPrevious;
        loop();
        return;
    }

    /* We need a filename to store the message.  */
    if (!strlen(str))
    {
        WRITE("No file name given. The message cannot be stored.\n");
        loop();
        return;
    }

    /* People can write anywhere they have access. */
    str = FTPATH(environment()->query_path(), str);

    /* We are nice people and don't allow people to over-write stuff. */
    if (file_size(str) != -1)
    {
        WRITE("The message file " + str + " already exists.\n");
        loop();
        return;
    }

    print_message(str);
}


/*
 * Function name: get_cmd
 * Description  : The main program loop sets up an input_to to this one.
 *                It selects the right function.
 * Arguments    : string str - player input through input_to()
 */
static void
get_cmd(string str)
{
    int     message;
    string  cmd;

    /* Entering commands while busy? Tsss. */
    if (gBusy)
    {
        WRITE("Still busy sending mail. Please wait.\n");
        loop();
        return;
    }

    /* Default command: read the next message. */
    if (!strlen(str))
        str = "n";

    /* If gCurrent is changed, we like to remember the previous active
     * message. Also, we check the more-option up front.
     */
    gPrevious = gCurrent;
    gUse_more = (extract(str, 0, 0) == "m");

    /* Check for commands we cannot trigger in the switch. */
    if ((sscanf(str, "%d", message) == 1) ||
        (sscanf(str, "m%d", message) == 1))
    {
        /* This is a flag-command to indicate that the player entered a
         * number of a message. If people give the command _read manually,
         * it will probably result in an illegal gCurrent == 0.
         */
        cmd = "_read";
    }
    else
    {
        cmd  = explode(str, " ")[0];
        str  = str[(strlen(cmd) + 1)..];
    }

    /* If new mail has arrived, we have to re-load the messages and we
     * check for destructive commands, which are ignored.
     */
    if (!gLoaded)
    {
        load_player();

        /* Any destructed command will be filtered first. */
        if ((cmd == "q") ||
            (cmd == "x") ||
            (cmd == "d"))
        {
            WRITE("New mail has arrived. Command ignored.\n");
            loop();
            return;
        }
    }

    switch(cmd)
    {
    /* Read the next message [with more]. */
    case "":
    case "n":
    case "mn":
        gCurrent++;
        print_message();
        return;

    /* Read the current message [with more]. */
    case ".":
    case "m.":
        print_message();
        return;

    /* Read a numbered message [with more]. */
    case "_read":
        gCurrent = message;
        print_message();
        return;

    /* Read the previous message [with more]. */
    case "p":
    case "mp":
        gCurrent--;
        print_message();
        return;

    /* List all [unread] headers [send by someone]. */
    case "h":
    case "H":
        from(((cmd == "h") ? "" : "new ") + str);
        return;

    /* Reply to a message [R: including the original]. */
    case "r":
    case "R":
        sscanf(str, "%d", gCurrent);
        reply(cmd == "R");
        return;

    /* Print the help-page. */
    case "?":
        help(str);
        return;

    /* Mail someone. */
    case "m":
        mail(str);
        return;

    /* Erase the messages marked for deletion. */
    case "e":
        erase();
        return;

    /* Quit the mailreader. */
    case "q":
    case "x":
        quit();
        return;

    /* Archive messages (and possibly mark for deletion). */
    case "ad":
    case "ak":
        archive(parse_range(cleanup_string(str)), (cmd == "ad"));
        return;

    /* Mark messages for deletion. */
    case "d":
        delete(parse_range(cleanup_string(str)));
        return;

    /* Forward a message to someone. */
    case "f":
    case "F":
        sscanf(str, "%d %s", gCurrent, str);
        forward(str, (cmd == "F"));
        return;

    /* Store a message file in a file in your directory. */
    case "s":
        sscanf(str, "%d %s", gCurrent, str);
        store(str);
        return;

    /* Mark messages as special. */
    case "*":
        star(parse_range(cleanup_string(str)));
        return;

    /* Resend a message to (other) people. */
    case "rs":
        resend(str);
        return;

    /* Unmark messages from deletion. */
    case "u":
        undelete(parse_range(cleanup_string(str)));
        return;

    /* Unmark messages as special. */
    case "u*":
        unstar(parse_range(cleanup_string(str)));
        return;

    /* Illegal command. */
    default:
        WRITE("No such command. Do \"?\" for help, \"q\" to quit.\n");
        loop();
    }
}


/*
 * Function name: invalidate
 * Description  : Allows other mailreaders to warn this one that it has
 *                to reload the messages, since there is new mail.
 */
public void
invalidate()
{
    gLoaded = 0;
}


/*
 * Function name: set_alias
 * Description  : This function may be called from a room to set aliases
 *                that are specific for this room, for instance a guild
 *                may define the alias "council" for the guildmembers.
 * Arguments    : string alias  - the name of the alias
 *                string *names - a list with names of individual players
 */
public void
set_alias(string alias, string *names)
{
    string *indices;

    if (!mappingp(gAliases))
        gAliases = ([ ]);

    if (!pointerp(names))
    {
        if (objectp(this_interactive()))
        {
            tell_object(this_interactive(),
                "\nPOSTMASTER -> Illegal alias added to the mailreader!\n" +
                "POSTMASTER -> Not an array: \"" + alias + "\"!\n" +
                "POSTMASTER -> Please make a bugreport about this!\n");
        }
        return;
    }

    /* Check whether the alias name is not already in use. */
    alias = lower_case(alias);
    if (SECURITY->exist_player(alias) ||
        IN_ARRAY(capitalize(alias), SECURITY->query_domain_list()) ||
        IN_ARRAY(LANG_SWORD(alias), WIZ_N) ||
        IN_ARRAY(alias, SECURITY->query_teams()) ||
        (alias == "playerarch") ||
        (file_size(ALIAS_DIR + alias) >= 0))
    {
        if (objectp(this_interactive()))
        {
            tell_object(this_interactive(),
                "\nPOSTMASTER -> Illegal alias-name \"" + alias + "\"!\n" +
                "POSTMASTER -> Please make a bugreport about this!\n");
        }
        return;
    }

    /* make sure we don't get double aliases. */
    m_delkey(gAliases, alias);

    /* Check if the alias only uses player names. */
    foreach(string pname: names)
    {
        if (!(SECURITY->exist_player(pname)))
        {
            if (objectp(this_interactive()))
            {
                tell_object(this_interactive(),
                    "\nPOSTMASTER -> Non-player name in alias \"" +
                    alias + "\" : \"" + pname + "\"!\n" +
                    "POSTMASTER -> Please make a bugreport about this!\n");
            }
            return;
        }
    }

#ifdef DEBUG
    if (objectp(this_interactive()))
    {
        tell_object(this_interactive(),
            "DEBUG: Mailreader alias added: " + alias + ".\n");
    }
#endif DEBUG

    gAliases[alias] = names;
}


/*
 * Function name: query_aliases
 * Description  : Returns the names to which one or more local aliases will
 *                expand.
 * Arguments    : string alias - the optional alias name.
 * Returns      : mixed - either array of string, or mapping.
 */
public varargs mixed
query_aliases(string alias)
{
    if (!stringp(alias))
        return ([ ]) + gAliases;
 
    if (!pointerp(gAliases[alias]))
        return 0;
 
    return ({ }) + gAliases[alias];
}
 
 
/*
 * Function name: create_mail
 * Description  : With this function, you can have mail generated from code.
 *                It may only be called in the master mail reader. All calls
 *                to this function are logged. No misuse of this function
 *                will be tolerated. Abuse -> tar, feathers and demotion!
 * Arguments    : string subject - the subject of the mail.
 *                string author  - the author of the mail.
 *                string to      - the recipients of this mail.
 *                string cc      - possible cc-recipients.
 *                    (Notice that it is the responsibility of the sender to
 *                    make sure the 'to' and 'cc' fields contain only valid
 *                    names. Otherwise the function will simply fail!)
 *                string body    - the body of the mail.
 * Returns      : int 1/0 - true if the mail was sent.
 */
public int
create_mail(string subject, string author, string to, string cc, string body)
{
    if (IS_CLONE)
        return 0;

    if (!strlen(subject) ||
        !strlen(to) ||
        !strlen(body))
        return 0;

    /* We add a line about the generating object to the mail-body. */
    body = "This mail message has been automatically generated.\n\n" +
        body + "\n";
    author = author[..10];
    to = cleanup_string(to);
    cc = cleanup_string(cc);

    /* Prepare the mail reader for sending. */
    gSubject = subject;
    gMessage = body;
    gTo = explode(to, ",");
    gCc = explode(cc, ",");
    gIs_reply = 0;

    /* All recipients must exist. */
    if (strlen(check_mailing_list(gTo)) ||
        strlen(check_mailing_list(gCc)))
        return 0;

#ifdef LOG_GENERATED_MAIL
    write_file(LOG_GENERATED_MAIL, "Date   : " + ctime(time()) +
        "\nObject : " + file_name(previous_object()) + "\nTI / TP: " +
        capitalize(this_interactive()->query_real_name()) + " / " +
        capitalize(this_player()->query_real_name()) + "\nAuthor : " +
        capitalize(author) + "\nSubject: " + subject + "\nTo     : " + to);
    if (strlen(cc))
        write_file(LOG_GENERATED_MAIL, "\nCC     : " + cc);

    write_file(LOG_GENERATED_MAIL, "\nBody:\n" + body + "<end of note>\n\n");
#endif LOG_GENERATED_MAIL

    send_mail(capitalize(author));
    return 1;
}


/*
 * Function name: remove_object
 * Description  : Guard for removal while the mailreader is busy.
 */
public void
remove_object()
{
    if (gBusy)
    {
#ifdef DEBUG
        if (interactive(environment()))
            WRITE("DEBUG: Call to remove_object in mailreader intercepted.\n");
#endif DEBUG

        /* But let it be removed the moment it gets available. */
        set_alarm(2.0, 0.0, remove_object);

        return;
    }

    ::remove_object();
}


/*
 * Function name: move
 * Description  : This function is a mask on the original function in
 *                order to ensure security. People cannot have more than
 *                one mailreader in them and mailreaders may only be
 *                placed in the direct inventory of a player. Any other
 *                environment will lead to the descruction of the reader.
 * Arguments    : mixed dest   - the destination of the reader
 *                mixed subloc - the sublocation of the move
 * Returns      : int          - false if moved, 5 if rejected
 */
public varargs int
move(mixed dest, mixed subloc)
{
    /* We do not allow the MASTER to be moved. This is done since the
     * auto-mail funcionality counts on it being in the void.
     */
    if (!IS_CLONE)
        return 7;

    /* In order to put it into a player, you need to pass the objectpointer
     * as pointer. The object you put it in, must be an interactive player.
     * Also, we cannot accept NPC's that are being possessed.
     */
    if ((!objectp(dest)) ||
        (!interactive(dest)) ||
        (function_exists("enter_game", dest) != PLAYER_SEC_OBJECT))
    {
        set_alarm(0.1, 0.0, remove_object);
        return 5;
    }

    /* One mailreader is enough. */
    if (present(READER_ID, dest))
    {
        tell_object(dest, "You are already carrying a mail reader.\n");
        set_alarm(0.1, 0.0, remove_object);
        return 5;
    }

    /* Initialise the mail reader when it is moved. */
    init_mail_reader(0);

    return ::move(dest, subloc);
}


/*
 * Function name: query_prevent_shadow
 * Description  : This function is called before the object is shadowed.
 *                We do not want that to happen ... ever!
 * Returns      : int - 1 - always.
 */
nomask public int
query_prevent_shadow()
{
    return 1;
}
 
/*
 * Function name: query_auto_load
 * Description  : The mail reader will autoload for wizards.
 * Returns      : string - the path of this object.
 */
nomask public string
query_auto_load()
{
    if (environment()->query_wiz_level())
        return MASTER;
 
    return 0;
}

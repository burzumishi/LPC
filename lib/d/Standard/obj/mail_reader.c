inherit "/std/object";

/*
 *  Mail reader, (not yet) adapted for 3.0  
 *  I will not continue to adapt this version of the post since it has
 *  come to my kowledge that Tintin and Groo have been working on this.
 *  Please remove this code once there is a working mailer.
 *
 *  910824.
 *	      /Styles.
 *
 *  Adapted to 3.0 by Groo 910903. Using the file defined
 *  in PLAYERNAMES to check if a player exists.
 *
 *  Since the original mailreader was very basic, and it seemed a low-
 *  priority project to the arches, I decided to enhance it a bit so
 *  it would fit my purposes and would use the advantages of 3.0 a wee
 *  bit more. Also threw out the PLAYERNAMES check. How un-3.0-ish can
 *  you get?
 *					Tricky, 25-1-92
 *
 *  Followed hints from Tintin and added aliases for pilgrims, mages, lords
 *  arches and keepers. Also changed the file format to use arrays, saves a
 *  lot of work. Added a converter for old .o files. Adjusted for use with
 *  the new secure/master/fob.c.
 *					Tricky, 19-4-92
 */

/*
 *  TO DO (very low priority):
 *	- Enable saving of mail.
 *      - Automatic mail removal
 *	- Aliases for anyone (including savable prefs)
 */

#include "/secure/std.h"
#include "/sys/stdproperties.h"
#include "/sys/bb_gedit.h"

/* Define debug when debugging */
#undef DEBUG

#ifdef DEBUG
#define POST_DIR  "/d/Standard/post_tmp/"
#else
#define POST_DIR  "/d/Standard/post_dir/"
#endif

#define IS_WIZARD (environment()->query_wiz_level())
#define MARKER    " *read*"	/* Used to mark read letters		   */
#define MAX_SEND_AT_ONCE 10	/* Maximum number of mail sent in one loop */
#define MY_UNIQUE_ID "_reader_"	/* Id for use with present()		   */
#define MSG_FROM  0		/* Index for sendername in message-array   */
#define MSG_SUBJ  1		/* Index for subject in message-array	   */
#define MSG_CC    2		/* Index for cc in message-array	   */
#define MSG_DATE  3		/* Index for date in message-array	   */
#define MSG_BODY  4		/* Index for message body in message array */

/* These aliases will not be recognised as player- or domainnames */
#define ALIASES ({"keeper","keepers","arch","arches","lord","lords",\
		  "mage","mages","pilgrim","pilgrims"})


/*
 * These global variables are to be saved in a .o file
 */

/*
 * The array messages contains arrays of strings of the form:
 *          ({ sender, subject, cc, date, message })
 */
mixed		*messages;	/* The array with all messages.		*/
int		 new_mail;	/* Flag if there is any new mail.	*/

/*
 * These globals are not to be saved with save_object, therefore static
 */

static mixed	*gArr_messages;	/* The result of explode on 'messages'	*/
static string	*gMore_array,	/* Same for explode on 'gCurr_message'	*/
		 gNew_message,	/* New message string			*/
		 gNew_subject,	/* New message subject			*/
		*gNew_dest,	/* All strings of addressed people	*/
		 gOld_cc;	/* A string with the old cc people	*/

static int	 gMore_pos,	/* Position in 'gMore_array'		*/
		 gLoaded,	/* Flag if this users mailbox is loaded */
		 gIs_reading,	/* Flag if the user is reading		*/
		 gCurr_message,	/* Current message number		*/
		 gPrev_message,	/* Same, before it changed		*/
                 gUse_gedit,	/* Flag for Gedit			*/
		*gDel_arr,	/* Array of message-numbers to delete	*/
		 gDel_cnt,	/* Counter keeping track of index	*/
		 gIndex,	/* Index of next mail to send		*/
		 gBusy,		/* Flag to indicate if we are busy	*/
		 gReprompt,	/* Flag to signal loop not to input_to	*/
		 gUse_more;	/* Flag to signal if more is to be used */



/*
 * Prototypes, to keep the driver happy ;-)
 */
int do_nmail(string str);
int do_gmail(string str);
int do_mail(string str);
void load_player();
void loop();
void get_cmd(string str);
void print_message();
void delete(int *del_arr);
int do_forward(string str);
void mail_to_continue();
void get_subject(string str);
void more_mail(string str);
void get_cc(string str);
void send_mail();
void send_mail_safely(mixed *to_split);
void notify_new_mail(string dest, string subj, string name);
void reply_to_message();
void reply_to_cc(string str);
void resend_message();
void help_msg();
mixed *convert_old_format(mixed *messages);
int print_more(string str);
void mark_message(int msg_num);
string *convert_to_names(string *list);
mixed *parse_range(string str);
string cleanup_string(string str);
mixed check_mailing_list(string *list);
int check_valid_msg_num(int msg_num);
void set_use_gedit(int num);
void gedit_msg(string str);
int do_from(string str);
int do_read(string str);
int do_mread(string str);
void check_environment();

void
create_object()
{
    seteuid(getuid(this_object()));
    set_name(({"mailreader", "reader", "mailread", MY_UNIQUE_ID}));
    set_pname(({"mailreaders", "readers", "mailreads", MY_UNIQUE_ID}));
    set_adj("new");
    set_short("new mailreader");
    set_pshort("new mailreaders");
    set_long(break_string(
	"This device is made of the finest of elven materials from Lorien, "
      + "put together by Lady Galadriel herself. It has a magical air over "
      + "it.\n",70));

  /* Some properties */
    add_prop(OBJ_I_NO_DROP,"@@slow_destruct");
    add_prop(OBJ_I_WEIGHT,0);
    add_prop(OBJ_I_VOLUME,0);
}


void
init()
{
    add_action(do_from,  "from");
    add_action(do_read,  "read");
    add_action(do_nmail, "mail");
    add_action(do_mread, "mread");

    if (IS_WIZARD)
	add_action(do_gmail, "gmail");
}


/*
 * Function name:   do_read_mail
 * Description:     This function is used to start up the mailreader.
 * Arguments:	    str: User input
 * Returns:	    0 if str is a text
 *		    1 otherwise
 */
int
do_read_mail(string str)
{
    int msg_num;

    gIs_reading = 1;

    if (!gLoaded)
    {
	load_player();
	if (!sizeof(gArr_messages))
	{
	    notify_fail("No messages.\n");
	    return 0;
	}
    }

    /* Do we have input? */
    if (str && str != "")
    {
	/* Check if the input makes sense */
	if (sscanf(str, "%d", msg_num))
	{
	    gCurr_message = msg_num;
	    print_message(); /* print_message will eventually call loop() */
	    return 1;
	}
	else
	    /* The "read" probably wasn't meant for us; e.g. "read note" */
	    return 0;
    }
    loop();
    return 1;
}

/*
 * Function name:   do_mread
 * Description:     This is a frontend for do_read_mail, sets the more flag
 * Arguments:	    str: User input
 * Returns:	    0 if str is a text
 *		    1 otherwise
 */
int
do_mread(string str)
{
    gUse_more = 1;
    return do_read_mail(str);
}

/*
 * Function name:   do_read
 * Description:     This is a frontend for do_read_mail, resets the more flag
 * Arguments:	    str: User input
 * Returns:	    0 if str is a text
 *		    1 otherwise
 */
int
do_read(string str)
{
    gUse_more = 0;
    return do_read_mail(str);
}


/*
 * Function name:   load_player
 * Description:     This function will load the mailbox of the player.
 */
void
load_player()
{
    string my_name;

    my_name = (string)this_player()->query_real_name();
    gLoaded = 1;
    messages = ({ });

    if (!restore_object(POST_DIR + my_name))
    {
	gArr_messages = ({ }); 
	messages = ({ });
	return;
    }
    else if (stringp(messages))		/* Still old format? Tssss. */
    {
#ifdef DEBUG
	write("Converting old message-format to new format...\n");
#endif
	messages = convert_old_format(messages);
	new_mail = 1;		/* Make sure that the new format is saved */
    }
    
    if (messages == 0)
	messages = ({ });

    /* If there was new mail, erase the flag */
    if (new_mail)
    {
	new_mail = 0;
	save_object(POST_DIR + my_name);
    }
    gArr_messages = messages;
}


/*
 * Function name:   do_from
 * Description:     Show the headers of all messages
 */
int
do_from(string str)
{
    int i, n;
    string tmp, tmp2, mess, subj, name, marked, to_print;

    if (!gLoaded)
	load_player();

    if (!sizeof(gArr_messages))
    {
	write("No messages.\n");
	return 1;
    }

    to_print = "";

    for (i=0; i < sizeof(gArr_messages); i++)
    {
	subj = "";
	name = gArr_messages[i][MSG_FROM];
	subj = gArr_messages[i][MSG_SUBJ];

	marked = 0;
	/* Remove the marker from the subject */
	if (sscanf(subj + " ","%s" + MARKER + "%s", tmp, tmp2) == 2)
	{
	    subj = tmp;
	    marked = "*read*";
	}
	else
	    marked = "      ";

	/* Cut off long subjects */
	if (strlen(subj) > 40)
	    subj = extract(subj, 0, 36) + "...";

	to_print += sprintf("%2d: %11-s %40-s %-s %-6s\n", i+1, name, subj,
					marked, gArr_messages[i][MSG_DATE]);
    }

    /* And print it all nicely */

    write(to_print);

    if (gIs_reading)
	loop();
    return 1;
}


/*
 * Function name:   loop
 * Description:     This is the main program loop that is called each
 *		    time a procedure returns. It contructs the prompt
 *		    and reloads messages if neccesary.
 */
void
loop()
{
    string tmp;

    gIs_reading = 1;

    if (!gLoaded)
	load_player();

    if (!sizeof(gArr_messages))
    {
	write("No messages.\n");
	gIs_reading = 0;
	return;
    }
    if (!check_valid_msg_num(gCurr_message))
    {
	gCurr_message = 0;
	tmp = " (no current) ";
    }
    else
	tmp = " (current: " + gCurr_message + ") ";
    write("[1-" + sizeof(gArr_messages) + " adfhmnprx.?]" + tmp);

    if (gBusy)
	write("\nBusy sorting out the mail, please wait.\n");

    /* If this is not a call to rewrite the prompt, do input_to */
    if (!gReprompt)
	input_to("get_cmd");
    else
	gReprompt = 0;
}


/*
 * Function name:   get_cmd
 * Description:     The main program loop sets up an input_to to this one.
 *		    It selects the right function.
 * Arguments:	    str: String from input_to()
 */
void
get_cmd(string str)
{
    int n, new_has_arrived;
    int *list;
    string tmp;

    /* Entering commands while busy? Tsss. */
    if (gBusy)
    {
	loop();
	return;
    }

    /*
     * First, test all nondestructive commands. The destructive commands
     * will be tested after check if new mail has arrived.
     */
    if (!gLoaded)
    {
	load_player();
	new_has_arrived = 1;
    }
    if (!str || str == "")
    {
	str = "n";
    }
    if (str == "r")	/* reply */
    {
	set_use_gedit(0);
	reply_to_message();
	return;
    }
    if (sscanf(str, "r %d", gCurr_message))
    {
	set_use_gedit(0);
	reply_to_message();
	return;
    }
    if (str == "gr")	/* reply using Gedit */
    {
	set_use_gedit(1);
	reply_to_message();
	return;
    }
    if (sscanf(str, "gr %d", gCurr_message))
    {
	set_use_gedit(1);
	reply_to_message();
	return;
    }
    if (str == "h")			/* headers */
    {
	do_from(0);
	return;
    }
    if (str == "mn" || str == "m+")	/* next */
    {
	gPrev_message = gCurr_message;
	gCurr_message++;
	gUse_more = 1;
	print_message();
	return;
    }
    if (str == "n" || str == "+")	/* next */
    {
	gPrev_message = gCurr_message;
	gCurr_message++;
	gUse_more = 0;
	print_message();
	return;
    }
    if (str == ".")			/* current */
    {
	gUse_more = 0;
	print_message();
	return;
    }
    if (str == "m.")			/* mread current message */
    {
	gUse_more = 1;
	print_message();
	return;
    }
    if (str == "a")			/* resend */
    {
	resend_message();
	return;
    }
    if (str == "?")			/* help */
    {
	help_msg();
	loop();
	return;
    }
    if (str == "mp" || str == "m-")	/* previous, with more */
    {
	gPrev_message = gCurr_message;
	gCurr_message--;
	gUse_more = 1;
	print_message();
	return;
    }
    if (str == "p" || str == "-")	/* previous */
    {
	gPrev_message = gCurr_message;
	gCurr_message--;
	gUse_more = 0;
	print_message();
	return;
    }
    if (sscanf(str, "m %s", tmp) == 1)	/* mail */
    {
	set_use_gedit(0);
	do_nmail(tmp);
	return;
    }
    if (sscanf(str, "gm %s", tmp) == 1)	/* mail using Gedit */
    {
	set_use_gedit(1);
	do_gmail(tmp);
	return;
    }
    if (sscanf(str, "f %s", tmp) == 1)	/* forward */
    {
	do_forward(tmp);
	return;
    }
    if (sscanf(str, "m%d", n) == 1)	/* mread number */
    {
	gPrev_message = gCurr_message;
	gCurr_message = n;
	gUse_more = 1;
	print_message();
	return;
    }
    if (sscanf(str, "%d", n) == 1)	/* read number */
    {
	gPrev_message = gCurr_message;
	gCurr_message = n;
	gUse_more = 0;
	print_message();
	return;
    }
    /* Check before doing destructive commands */
    if (new_has_arrived)
    {
	write("New mail has arrived. Command ignored\n");
	loop();
	return;
    }
    if (str == "x")	/* exit */
    {
	/* Erase everything */
	gIs_reading = 0;
	gNew_message = 0;
	gNew_subject = 0;
	gNew_dest = 0;
	gCurr_message = 0;
	gPrev_message = 0;
	return;
    }
    if (str == "d")	/* delete current */
    {
	delete(({ gCurr_message }));
	return;
    }
    if (sscanf(str, "d %s", tmp))	/* delete range */
    {
	tmp = cleanup_string(tmp);
	list = parse_range(tmp);
	delete(list);
	return;
    }
    loop();
}


/*
 * Function name:   print_message
 * Description:     This function prints all lines of a message.
 *		    The messagenumber is stored in gCurr_message and
 *		    the message itself is stored in gArr_messages.
 */
void
print_message()
{
    string tmp, tmp1, tmp2;

    /* Check if gCurr_message is within the bounds */
    if (!check_valid_msg_num(gCurr_message))
    {
	write("Illegal message number " + gCurr_message + ".\n");
	gCurr_message = gPrev_message;
	if (gIs_reading)
	    loop();
	return;
    }

    /* Mark the message as read */
    mark_message(gCurr_message);

    /* Do not print the mark */
    sscanf(gArr_messages[gCurr_message-1][MSG_SUBJ], "%s" + MARKER + "%s",tmp1,tmp2);

    /* Construct the message */
    tmp = "Message " + gCurr_message + ":\nFrom: "
	+ gArr_messages[gCurr_message-1][MSG_FROM] + "\n"   /* Add sender  */
	+ tmp1 + "\n";					    /* Add subject */
    if (gArr_messages[gCurr_message-1][MSG_CC] != "")
	tmp += break_string(gArr_messages[gCurr_message-1][MSG_CC] + "\n",79);

    tmp	+= gArr_messages[gCurr_message-1][MSG_DATE] + "\n\n" /* Add date   */
	+ gArr_messages[gCurr_message-1][MSG_BODY] + "\n";  /* Add message */

    if (gUse_more)
    {
	gMore_array = explode(tmp,"\n");
	gMore_pos = 0;
	print_more("");
    }
    else
    {
	write(tmp);
	if (gIs_reading)
	    loop();
    }
}


/*
 * Function name:   delete
 * Description:     Deletes the current message or a whole range of messages
 * Arguments:	    del_arr: If not specified delete current message,
 *			     if an array delete all members
 */
void
delete(int *del_arr)
{
    int i, j, curr;
    string *messages_copy;
    mixed *msg_copy;

    if (!del_arr)
	del_arr = ({ });

    msg_copy = ({ });
    curr = gCurr_message;

    for (i=1; i <= sizeof(gArr_messages); i++)
    {
	/* Delete messages by not adding them */
	if (member_array(i, del_arr) < 0)
	{
	    /* Only add messages that are not to be deleted */
	    msg_copy += ({ gArr_messages[i-1] });
	}
	else
	{
	    if (i == gCurr_message)
		curr = 0;
	    else if (i < gCurr_message)
		curr--;
#ifdef DEBUG
	    write("Deleting message " + i + ".\n");
#endif
	}
    }

    gArr_messages = msg_copy;
    gCurr_message = curr;

    new_mail = 0;

    if (!sizeof(gArr_messages))
    {
	messages = ({ });
	rm(POST_DIR + this_player()->query_real_name() + ".o");
    }
    else
    {
	messages = gArr_messages;
        /* Save the messages left */
        save_object(POST_DIR + this_player()->query_real_name());
     }

    do_from(0);
}


/*
 * Function name:   do_forward
 * Description:     This function enables the user to redirect incoming
 *		    mail to other people without retyping it.
 * Arguments:	    a string with the adressed people, separated by commas,
 *		    spaces or a combination of the two. Domains may also
 *		    be included. Note that people should have lowercase
 *		    names, and domains capitalized names.
 * Returns:	    1
 */
int
do_forward(string str)
{
    string *list, *wizlist;
    string error, tmp, tmp1, tmp2;
    int i;

    if (!str)
    {
	write("No destination.\n");
	if (gIs_reading)
	    loop();
	return 1;
    }

    /* Make the string readable */
    str = cleanup_string(lower_case(str));

    /* Now we need a list of addressable names. */
    list = explode(str + ",", ",");

    /* We'd better check it before we start entering the message. */
    if (error = check_mailing_list(list))
    {
	write("No such player");
	if (IS_WIZARD)
	    write(" or domain");
	write(": " + error + ".\n");
	if (gIs_reading)
	    loop();
	return 1;
    }

    /* Set variables up for forwarded mail */
    gNew_dest = list;

    /* Do not send the mark */
    sscanf(gArr_messages[gCurr_message-1][MSG_SUBJ], "%s" + MARKER + "%s",tmp1,tmp2);
    gNew_subject = tmp1 + " (forwarded)";

    /* Construct the forwarded message */
    tmp = "Original from: "
	+ gArr_messages[gCurr_message-1][MSG_FROM] + "\n"   /* Add sender  */
	+ tmp1 + "\n";					    /* Add subject */
    if (gArr_messages[gCurr_message-1][MSG_CC] != "")
	tmp += break_string(gArr_messages[gCurr_message-1][MSG_CC] + "\n",79);

    tmp	+= gArr_messages[gCurr_message-1][MSG_DATE] + "\n\n" /* Add date   */
	 + gArr_messages[gCurr_message-1][MSG_BODY] + "\n"; /* Add message */

    gNew_message = tmp;

    if (sizeof(gNew_dest))
    {
	/* Start sending the mail */
	set_alarm(1.0, 0.0, send_mail);

	/* Set a flag so the main loop won't start yet */
	gBusy = 1;

	if (gIs_reading)
	    loop();
	else
	write("Please be patient while your mail is being sorted.\n");
    }
    else
    {
	write("No people to send mail to, mail aborted.\n");
	if (gIs_reading)
	    loop();
    }
    return 1;
}


/*
 * Function name:   do_gmail
 * Description:	    Frontend for do_mail(), use Gedit.
 */
int
do_gmail(string str)
{
    set_use_gedit(1);
    return do_mail(str);
}


/*
 * Function name:   do_nmail
 * Description:	    Frontend for do_mail(). Don't use Gedit.
 */
int
do_nmail(string str)
{
    set_use_gedit(0);
    return do_mail(str);
}

/*
 * Function name:   do_mail
 * Description:     This function will start the writing of a mail to the
 *		    specified people. It will notify the user if it notices
 *		    a wrongness in the fabric of existance.
 * Arguments:	    a string with the adressed people, separated by commas,
 *		    spaces or a combination of the two. Domains may also
 *		    be included. Note that people should have lowercase
 *		    names, and domains capitalized names.
 * Returns:	    1
 */
int
do_mail(string str)
{
    string *list, *wizlist;
    string error;
    int i;

    if (!str)
    {
	write("No destination.\n");
	if (gIs_reading)
	    loop();
	return 1;
    }

    /* Make the string readable */
    str = cleanup_string(lower_case(str));

    /* Now we need a list of addressable names. */
    list = explode(str + ",", ",");

    /* We'd better check it before we start entering the message. */
    if (error = check_mailing_list(list))
    {
	write("No such player");
	if (IS_WIZARD)
	    write(" or domain");
	write(": " + error + ".\n");
	if (gIs_reading)
	    loop();
	return 1;
    }

    gNew_dest = list;		/* Set variables up for new mail */
    gNew_subject = "";
    mail_to_continue();
    return 1;
}


/*
 * Function name:   mail_to_continue
 * Description:     Ask the subject. Take in account gNew_subject.
 */
void
mail_to_continue()
{
    gNew_message = "";
    if (gNew_subject != "")
	write("Return to get '" + gNew_subject + "'\n");
    write("Subject: ");
    input_to("get_subject");
}


/*
 * Function name:   get_subject
 * Description:     This function will be called by filter_array to determine
 *		    whether a string is to be kept in gArr_messages.
 * Arguments:	    str: Input from player
 */
void
get_subject(string str)
{
    if (str == "~q")
    {
	write("Aborted.\n");
	if (gIs_reading)
	    loop();
	return;
    }
    if (str && str != "")
	gNew_subject = "Subj: " + str;
    if (gNew_subject == "")
    {
	write("No subject, aborting.\n");
	if (gIs_reading)
	    loop();
	return;
    }
    /* Check if Gedit has to be used */
    if (gUse_gedit)
    {
	init_ed_it(this_player());
	write("Entering Gedit. Type 'help' or '?' for help.\n"
	    + "To abort the message, type '~q'.\n");
	input_to("gedit_msg");
	GPROMPT();
	return;
    }
    /* If not, use the normal way. */
    write("Give message.  Finish message with '**', or '~q' to cancel\n");
    write("]");
    input_to("more_mail");
    return;
}


/*
 * Function name:   more_mail
 * Description:     Keep asking a next line of mail until "~q" or "**"
 *		    was entered by the player.
 * Arguments:	    str: Input from the player
 */
void
more_mail(string str)
{
    if (str == "~q")
    {
	write("Aborted.\n");
	if (gIs_reading)
	    loop();
	return;
    }
    if (str == "**")
    {
	write("Cc: ");
	input_to("get_cc");
	return;
    }
    gNew_message += str + "\n";
    write("]");
    input_to("more_mail");
}


/*
 * Function name:   get_cc
 * Description:     Gets as input the string of people that should receive a
 *		    carbon copy of this message. Warns if there are
 *		    wrongnesses in that list.
 * Arguments:	    str: A string with the list
 */
void
get_cc(string str)
{
    string *list, error;

    if (str == "~q")
    {
	write("Aborted.\n");
	if (gIs_reading)
	    loop();
	return;
    }
    if (str && str != "" && str != "**")
    {
	/* Make the string readable */
	str = cleanup_string(lower_case(str));

	/* Now we need a list of addressable names. */
	list = explode(str + ",", ",");

	/* We'd better check it before we start entering the message. */
	if (error = check_mailing_list(list))
	{
	    write("No such player");
	    if (IS_WIZARD)
		write(" or domain");
	    write(": " + error + ".\n");
	    write("Cc: ");
	    input_to("get_cc");
	    return;
	}
	else
	    gNew_dest += list;
    }

    if (sizeof(gNew_dest))
    {
        /* Set a flag so the main loop won't start yet */
        gBusy = 1;

        /* Start sending the mail */
	set_alarm(1.0, 0.0, send_mail);

        /* Call the loop, to keep the player with this object */
	if (gIs_reading)
	    loop();
    }
    else
    {
	write("No people to send mail to, mail aborted.\n");
	if (gIs_reading)
	    loop();
    }
}


/*
 * Function name:   send_mail
 * Description:     This function send the mail to a number of users and
 *		    makes sure that if people are reading mail their readers
 *		    will be notified. To avoid the "evaluation too long"
 *		    error, send_mail_safely is called.
 */
void
send_mail()
{
    gIndex = 0;

    /* Note that by now, gNew_dest is a list with both domains and */
    /* names in it. Now convert it to one array with names only.   */

    gNew_dest = convert_to_names(gNew_dest);

    /* This used to be a call_out for safety reasons */
    send_mail_safely(({ gNew_dest, gNew_subject, gNew_message }));
}


/*
 * Function name:   send_mail_safely
 * Description:     See send_mail.
 */
void
send_mail_safely(mixed *to_split)
{
    int i, size, cnt;
    string cc, target, my_name, subj, mess;
    string *dest_arr;
    /* Split the argument */
    dest_arr = (string *) to_split[0];
    subj = (string) to_split[1];
    mess = (string) to_split[2];

    cnt = 0;
    my_name = capitalize((string) this_player()->query_real_name());
    size = sizeof(dest_arr);

    if (!size)
    {
	gBusy = 0; /* Clear the busy flag for the main loop */
	write("No mail sent.\n");
	if (gIs_reading)
	{
	    /* Rewrite the prompt, using loop() without input_to */
	    gReprompt = 1;
	    loop();
	}
	return;
    }

    while (gIndex < size && cnt < MAX_SEND_AT_ONCE)
    {
	i = gIndex;

	/* Compile a list of people to Cc: */
	if (size > 1)
	    cc = "Cc: " + implode(exclude_array(dest_arr, i, i),", ");
	else
	    cc = "";

	target = dest_arr[i];

	/* Tell the target she has new mail */
	notify_new_mail(target, subj, my_name);

	messages = ({ });

	/* Load the targets mailbox */
	restore_object(POST_DIR + target);

	if (stringp(messages))		/* Still old format? Tssss. */
	{
	    messages = convert_old_format(messages);
	    messages = ({ });
	}

	if (messages == 0)
	    messages = ({ });

	/* Construct the mail in standard form */
	messages += ({ ({ my_name,
			  subj,
			  cc,
			  "Date: " + extract(ctime(time()), 4, 9),
			  mess }) });

	/* Set new mail flag */
	new_mail = 1;

	/* Save the targets mailbox */
	save_object(POST_DIR + target);

	gIndex++;
	cnt++;
    }

    /* Pause to avoid error */
    if (gIndex < size)
	set_alarm(1.0, 0.0, &send_mail_safely(to_split));
    else
    {
	gBusy = 0; /* Clear the busy flag for the main loop */
	write("All mail sent.\n");
	if (gIs_reading)
	{
	    /* Rewrite the prompt, using loop() without input_to */
	    gReprompt = 1;
	    loop();
	}
    }
}


/*
 * Function name:   notify_new_mail
 * Description:     Notifies the destination that there is new mail for her.
 * Arguments:	    dest: The targets name
 *		    subj: The subject of the mail
 *		    name: The name of the sender of the mail
 */
void
notify_new_mail(string dest, string subj, string name)
{
    object ob;

    write("Sending mail to " + capitalize(dest) + ".\n");
    if (ob = find_player(dest))
    {
	tell_object(ob,"\nYou have new mail from " + capitalize(name) + ", "
		  + subj + "\n");
	ob = present(MY_UNIQUE_ID, ob);
	if (ob)
	    ob->invalidate();
    }
}


/*
 * Function name:   invalidate
 * Description:     Allows other mailreaders to warn this one that it has
 *		    to reload the messages, since there is new mail.
 */
public void
invalidate()
{
    gLoaded = 0;
}


/*
 * Function name:   reply_to_message
 * Description:     Send a reply to the sender of gCurr_message.
 */
void
reply_to_message()
{
    int n;
    string tmp_str, tmp_str2, mess;
    string name;

    /* Check if the current message number is valid */
    if (!check_valid_msg_num(gCurr_message))
    {
	write("Illegal message number " + gCurr_message + ".\n");
	return;
    }

    name = lower_case(gArr_messages[gCurr_message-1][MSG_FROM]);

    /* Was the old subject a reply? */
    if (sscanf(gArr_messages[gCurr_message-1][MSG_SUBJ]," Re:  %s",tmp_str))
    {
	gNew_subject = " Re:  " + tmp_str;
    }
    else
    {
	sscanf(gArr_messages[gCurr_message-1][MSG_SUBJ],"Subj: %s",tmp_str);
	gNew_subject = " Re:  " + tmp_str;
    }

    /* Remove the read-marker, if present */
    if (sscanf(gNew_subject, "%s" + MARKER, tmp_str))
	gNew_subject = tmp_str;

    /* A reply can be sent without check. */
    gNew_dest = ({ name });

    if ((tmp_str = gArr_messages[gCurr_message-1][MSG_CC]) != "")
    {
        sscanf(tmp_str, "Cc: %s", tmp_str2);
	write("The original message was also sent to:\n"
	    + break_string(tmp_str2,70) + ".\n"
	    + "Send reply to all these people as well (y/n)?  [Y] ");
	input_to("reply_to_cc");
	gOld_cc = cleanup_string(tmp_str2);
    }
    else
    {
	reply_to_cc("n");
    }
}


/*
 * Function name:   reply_to_cc
 * Description:     Ask confirmation about replying to Cc: people as well.
 * Arguments:	    str: if "~q", abort, if "Y" or "y" cc old cc-people.
 */
void
reply_to_cc(string str)
{
    if (str == "~q")
    {
	write("Aborted.\n");
	if (gIs_reading)
	    loop();
	return;
    }

    if (!str || str == "" || str == "y" || str == "Y")
    {
	/* The Cc: people can be added without check */
	gNew_dest += explode(gOld_cc + "," , ",");
    }
    mail_to_continue();
}


/*
 * Function name:   resend_message
 * Description:     Send the last entered message to some more people
 */
void
resend_message()
{
    gNew_dest = ({ });
    if (!gNew_message || gNew_message == "")
    {
	write("There is no last message!\n");
	if (gIs_reading)
	    loop();
	return;
    }
    write("To whom would you like to resend the last message you sent?\n"
	+ "Cc: ");
    input_to("get_cc");
}


/*
 * Function name:   help_msg
 * Description:     Show all commands
 */
void
help_msg()
{
    write("x          Exit from mail reading mode.\n"
	+ "d          Delete current message.\n"
	+ "d <range>  Delete messages in range.\n"
	+ "?          This help message.\n"
	+ "r [<num>]  Reply to current message [<num>].\n"
	+ "m <name>   Mail a message to player <name>.\n"
	+ "f <name>   Forward current message to <name>.\n"
	+ "a          Resend last sent message again to other people.\n"
	+ "h          Print all headers.\n"
	+ "[m].       Show current message [using more].\n"
	+ "[m]<num>   Show message number <num> [using more].\n"
	+ "[m]p/[m]-  Show previous message [using more].\n"
	+ "[m]n/[m]+  Show next message [using more].\n"
	+ "\n");
    if (IS_WIZARD)
	write("Note:      Use \"gmail\", \"gm\" or \"gr\" to use Gedit as editor.\n\n");
    write("   [xxx]   xxx is optional, can be left out.\n"
	+ "  <range>  Any range of numbers, eg. \"1,9,5-7\".\n"
	+ "   <num>   A single number, a <range> can be <num>, eg. \"1\".\n"
	+ "   <name>  A list of player");
    if (IS_WIZARD)
	write("/domain");
    write(" names, eg. \"fatty, mrpr, tintin\".\n"
	+ "           Cc: can be a list as well.\n");
    return;
}


/*
 * Function name:   print_more
 * Description:     This function is usually called by input_to, and
 *		    handles the reading of a message. The exploded message
 *		    should be in gMore_array, and the current line position
 *		    is kept in gMore_pos.
 */
int
print_more(string str)
{
    int more_jump, from, to;
    string *slice;
    string to_write;

    /* Try to use the players favourite more length */
    more_jump = (int)environment()->query_prop(PLAYER_I_MORE_LEN);
    if (!more_jump)
	more_jump = 20;

    if (str == "" | !str)	/* Show new page */
    {
	from = gMore_pos;
    }
    else if (str == "?")	/* Show help */
    {
	write("\nControl commands:\n"
	    + "   <return>    Show next page\n"
	    + "       q       Quit\n"
	    + "       u       One page up\n"
	    + "       t       Go to top of document\n"
	    + "       b       Go to bottom of document\n"
	    + "   <number>    Go to line <number>\n"
	    + "       ?       Show this help\n\n");
	write("--- " + (gMore_pos + 1) + " --- [<ret> q u t b # ?]   ");
	input_to("print_more");
	return 1;
    }
    else if (str == "u")	/* Show previous page */
    {
	from = gMore_pos - 2*more_jump;
	if (from < 0)
	    from = 0;
    }
    else if (str == "t")	/* Show top */
    {
	from = 0;
    }
    else if (str == "b")	/* Show bottom */
    {
	from = sizeof(gMore_array) - 1 - more_jump;
	if (from < 0)
	    from = 0;
    }
    else if (str == "q")
    {
	if (gIs_reading)
	    loop();
	return 1;
    }
    else if (sscanf(str,"%d",from))
    {
	from--;
	if (from < 0)
	    from = 0;
	else if (from > (sizeof(gMore_array) - more_jump))
	    from = sizeof(gMore_array) - more_jump;
    }

    to = from + more_jump - 1;
    
    if (to >= sizeof(gMore_array))
	to = sizeof(gMore_array) - 1;

    if (from <= to)
    {
	write(implode(gMore_array[from..to], "\n")); /* Write the part */
    }
    if (to < (sizeof(gMore_array) - 1))
    {
	gMore_pos = to + 1;
	write("\n--- More: " + (gMore_pos + 1) + " --- [<ret> q u t b # ?]   ");
	input_to("print_more");
	return 1;
    }

    if (gIs_reading)
	loop();

    return 1;
}


/*
 * Function name:   mark_message
 * Description:     Mark a message as read.
 * Arguments:	    msg_num: The number of the message to be marked.
 *		    Note that the number is supposed to be valid.
 */
void
mark_message(int msg_num)
{
    string my_name, subj, rest, tmp;

    if (sscanf(gArr_messages[msg_num-1][MSG_SUBJ], "Subj:%s", subj))
    {
	/* Is the message already marked? */
	if (subj && subj != "" && sscanf(subj, "%s" + MARKER, tmp))
	{
	    return;
	}

	/* Reconstruct the message with a marked subject */
	gArr_messages[msg_num-1][MSG_SUBJ] = "Subj:" + subj + MARKER;

	messages = gArr_messages;	/* Prepare to save */
	my_name = (string) environment()->query_real_name();

	/* Update the mailbox */
	save_object(POST_DIR + my_name);
    }
    else if (sscanf(gArr_messages[msg_num-1][MSG_SUBJ], " Re: %s", subj))
    {
	/* Is the message already marked? */
	if (subj && subj != "" && sscanf(subj, "%s" + MARKER, tmp))
	{
	    return;
	}
	/* Reconstruct the message with a marked subject */
	gArr_messages[msg_num-1][MSG_SUBJ] = " Re: " + subj + MARKER;

	messages = gArr_messages;	/* Prepare to save */
	my_name = (string) environment()->query_real_name();

	/* Update the mailbox */
	save_object(POST_DIR + my_name);
    }
}


/*
 * Function name:   convert_to_names
 * Description:     Change the mixed list of domains and names into a
 *		    names-only list that can be processed by send_mail.
 *		    Here we also convert aliases like "keepers", "arches",
 *		    "lords" etc. to real names. 
 * Arguments:	    list: a list of domains and names
 * Returns:	    the names-only list
 */
string *
convert_to_names(string *list)
{
    int i,j, is_wiz, domain_num;
    int *warr;
    string *return_arr, *a, str;

    if (!list || !sizeof(list))
	return ({ });

    is_wiz = (int) IS_WIZARD;
    return_arr = ({ });

    for (i=0; i < sizeof(list); i++)
    {
	if (str = list[i])
	{

	    /* The integrity of the list is already checked, therefore */
	    /* either str is a player...			       */
	    if (SECURITY->exist_player(str))
	    {
		return_arr += ({ str });
	    }

	    /* ... or str is a domain or a class of wizards. */
	    else
	    {
		str = lower_case(str);

		/* Perhaps it is an alias! */
		if (str == "keepers" || str == "keeper")
		{
#ifdef DEBUG
		    write("Recognised alias \"" + str + "\".\n");
#endif
		    if (sizeof(a = SECURITY->query_wiz_list(WIZ_KEEPER)))
			return_arr += a;
		}
		else if (str == "arches" || str == "arch")
		{
#ifdef DEBUG
		    write("Recognised alias \"" + str + "\".\n");
#endif
		    if (sizeof(a = SECURITY->query_wiz_list(WIZ_ARCH)))
			return_arr += a;
		    if (sizeof(a = SECURITY->query_wiz_list(WIZ_KEEPER)))
			return_arr += a;
		}
		else if (str == "lords" || str == "lord")
		{
#ifdef DEBUG
		    write("Recognised alias \"" + str + "\".\n");
#endif
		    if (sizeof(a = SECURITY->query_wiz_list(WIZ_LORD)))
			return_arr += a;
		}
		else if (str == "mages" || str == "mage")
		{
#ifdef DEBUG
		    write("Recognised alias \"" + str + "\".\n");
#endif
		    if (sizeof(a = SECURITY->query_wiz_list(WIZ_MAGE)))
			return_arr += a;
		}
		else if (str == "pilgrims" || str == "pilgrim")
		{
#ifdef DEBUG
		    write("Recognised alias \"" + str + "\".\n");
#endif
		    if (sizeof(a = SECURITY->query_wiz_list(WIZ_PILGRIM)))
			return_arr += a;
		}
		/* Forbid domains that are too big */
		else if (str != "standard" && str != "wiz")
		{
	    	    if (is_wiz)
		    {
			return_arr += (a = SECURITY->query_domain_members(capitalize(str))) ?
					a : ({ });
		    }
		}
	    }
	}
    }

#ifdef DEBUG
    write("Addressees: ");
    if (sizeof(return_arr) == 0)
	write("none.\n");
    else
    {
	for (i=0; i < sizeof(return_arr); i++)
	    write(capitalize(return_arr[i]) + ",");
	write("\n");
    }
#endif

    return return_arr;
}


/*
 * Function name:   parse_range
 * Description:     Try to compile a list of message numbers from the string
 *		    You should clean the string first with cleanup_string.
 *		    The list of numbers is sorted and all doubles are
 *		    filtered out.
 * Arguments:	    str: the string that is going to be parsed
 *			 Typical accepted string:
 *				10,1-4,2
 *			 Would return: ({ 1, 2, 3, 4, 10 });
 * Returns:	    0 if the string could not be parsed
 *		    otherwise an array of message numbers
 */
mixed *
parse_range(string str)
{
    int i, left_val, right_val, tmp_val, val, last;
    mixed *parse_arr, tmp_arr;
    string tmp, left, right;
    string *substr_arr;

    parse_arr = ({ });

    while (sscanf(str,"%s-%s", left, right) == 2)
    {
	/* Try to obtain the value left of the "-" */
	substr_arr = explode(left + ",", ",");

        /* Convert the string to a value */
	if (!sscanf(substr_arr[sizeof(substr_arr)-1], "%d", left_val))
	    return 0;

	/* Try to obtain the value right of the "-" */
	substr_arr = explode(right + ",", ",");

        /* Convert the string to a value */
	if (!sscanf(substr_arr[0], "%d", right_val))
	    return 0;

	/* Glue the leftovers together */
	str = left + "," + right;

	/* Make sure left_val is smaller than or equal to right_val */
	if (right_val < left_val)
	{
	    tmp_val = left_val;
	    left_val = right_val;
	    right_val = tmp_val;
	}
        
	for (i = left_val; i <= right_val; i++)
	{
	    parse_arr += ({ i });
	}
    }

    substr_arr = explode(str + ",", ",");

    /* If there are still numbers left in the string, convert them to	*/
    /* integers and add them to the parse_arr.				*/
    for (i=0; i<sizeof(substr_arr); i++)
    {
	sscanf(substr_arr[i], "%d", val);
	parse_arr += ({ val });
    }

    /* Let's sort the parse_array */
    parse_arr = sort_array(parse_arr,"lesseq", this_object());

    /* Try to filter out the doubles */
    last = parse_arr[0];
    tmp_arr = ({ parse_arr[0] });

    for (i=1; i<sizeof(parse_arr); i++)
    {
	if (parse_arr[i] != last)
        {
	    tmp_arr += ({ parse_arr[i] });
	    last = parse_arr[i];
	}
	str = tmp;
    }

    return tmp_arr;
}


/*
 * Function name:   lesseq
 * Description:	    Called by sort_array, is simply <= for integers.
 * Arguments:	    a,b: integers to compare.
 * Returns:	    True if a <= b, otherwise false.
 */
public int
lesseq(int a, int b)
{
    return a <= b;
}


/*
 * Function name:   cleanup_string
 * Description:     Clean up the given string and turn it into a nice string
 *		    with only commas separating the substrings.
 *		    Leaves a string like "1-4" alone.
 * Arguments:	    str: the string that is going to be cleaned up
 * Returns:	    The clean string
 */
string
cleanup_string(string str)
{
    int i, val;
    string tmp, last, chr;

    if (str == "" || !str)
	return "";

    tmp = "";

    /* Let's first change all spaces to commas */
    for (i=0; i<strlen(str); i++)
    {
	tmp += (((chr = extract(str,i,i)) == " ") ? "," : chr);
    }
    str = tmp;

    /* Now we delete all multiple commas */
    if (strlen(str) > 1)
    {
	last = extract(str,0,0);
	tmp = last;
	for (i=1; i<strlen(str); i++)
	{
	    if ((chr = extract(str,i,i)) != "," || last != ",")
		tmp += chr;
	    last = chr;
	}
	str = tmp;
    }

    /* Check for a single comma left */
    if (str == ",")
	return "";

    /* Check for starting comma */
    if (extract(str,0,0) == ",")
	str = extract(str, 1, strlen(str) - 1);

    /* Check for ending comma */
    if (extract(str,val = (strlen(str)-1),val) == ",")
	str = extract(str,0,val-1);

    /* Et voila! */
    return str;
}


/*
 * Function name:   check_mailing_list
 * Description:     Check if all names on the list are valid. Reports the
 *		    first non-valid name that is encountered. Takes aliases
 *                  in account.
 * Arguments:	    list: a list of strings with names. 
 * Returns:	    enquoted array string if the list contained a
 *		    non-valid name
 *		    0 if all members are valid
 */
mixed
check_mailing_list(string *list)
{
    int i;
    string str;

    if (!list || !sizeof(list))
	return "''";


    for (i=0; i < sizeof(list); i++)
    {
        str = list[i];

	if (!str || str == "")
	    return ("''");

	/* Check if it is no alias */
	if (member_array(str, ALIASES) < 0)
	{
	    /* Is this name a name of a player? */
	    if (!(SECURITY->exist_player(str)))
	    {
		/* Only wizards can mail domains */
		if (IS_WIZARD)
		{
		    /* Then perhaps it was a domain? */
		    str = capitalize(str);
		    if (member_array(str,SECURITY->query_domain_list()) < 0)
		    {
			return ("'" + str + "'");
		    }
		}
		else
		{
		    if (!str || str == "")
			return ("''");
		    return ("'" + str + "'");
	        }
	    }
	}
    }
    return 0;
}


/*
 * Function name:   check_valid_msg_num
 * Description:     Check if the number is a valid message number, ie. a
 *		    number in the range of 1 to the number of messages.
 * Arguments:	    msg_num: the number to check
 * Returns:	    True if valid
 */
int
check_valid_msg_num(int msg_num)
{
    return (msg_num > 0 && msg_num <= sizeof(gArr_messages));
}


/*
 * Function name:   slow_destruct
 * Description:     This function is called before the object is dropped
 *		    by the OBJ_I_NO_DROP property.
 * Returns:	    0 (the object can be dropped)
 */
int
slow_destruct()
{
  /* Let this object be dropped initially, but check afterwards */
    set_alarm(1.0, 0.0, check_environment);
    return 0;
}


/*
 * Function name:   check_environment
 * Description:     Don't destroy this object if it is held by a person.
 */
void
check_environment()
{
  /* Is this object still carried by a person? */
    if (!living(environment()))
    {
	tell_room(environment(), 
	    "The mailreader falls on the floor and shatters to pieces.\n");
	remove_object();
    }
}


/*
 * Function name:   convert_old_format
 * Description:	    Convert a given string of messages into an array with
 *		    message arrays of the form ({ from, cc, date, msg }).
 *		    This function is typically called just after you tested
 *		    the output of restore_object() and found that it was no
 *		    array of arrays, but a string.
 * Arguments:	    messages: The string with messages
 * Returns:	    An arrays of message-arrays.
 */
mixed *
convert_old_format(mixed messages)
{
    int i;
    string from, subj, cc, date, body, tmp1, tmp2, tmp3;
    string *tmp_msg_arr;
    mixed *return_arr;

    tmp_msg_arr = explode(messages, "\n**\n");
    return_arr = ({ });

    for (i=0; i < sizeof(tmp_msg_arr)/2; i++)
    {
	from = capitalize(tmp_msg_arr[2*i]);

	if (sscanf(tmp_msg_arr[2*i+1]," Re:   %s\n%s",tmp2,tmp3) == 2)
	    subj = " Re:  " + tmp2;

	if (sscanf(tmp_msg_arr[2*i+1],"Subj: %s\n%s",tmp2,tmp3) == 2)
	    subj = "Subj: " + tmp2;

	if (sscanf(tmp_msg_arr[2*i+1],"%sCc: %s\n%s",tmp1,tmp2,tmp3) == 3)
	    cc = "Cc: " + tmp2;
	else
	    cc = "";

	if (sscanf(tmp_msg_arr[2*i+1],"%sDate: %s\n\n%s",tmp1,tmp2,tmp3) == 3)
	    date = "Date: " + tmp2;
	else
	    date = "Date: ";

	body = tmp3;

	return_arr += ({ ({ from, subj, cc, date, body }) });
    }
    return return_arr;
}


/***************************************************************************
 *
 *  Gedit functions, taken from /std/board.c
 */


/*
 * Function name:   set_use_gedit
 * Description:     Make the mailreader use Gedit to edit messages
 * Arguments:	    num: true if you want to use Gedit, else false
 */
public void set_use_gedit(int num)
{
    gUse_gedit = num;
}


/*
 * Function name:   gedit_msg
 * Description:     Catches input_to and feeds it to Gedit.
 * Arguments:	    str: user commands
 */
void
gedit_msg(string str)
{
    string *ret_buf;

    /* I don't know if ed_it is happy with this way of quitting */
    if (str == "~q")
    {
	write("Aborted.\n");
	if (gIs_reading)
	    loop();
	return;
    }

    /* If ed_it returns a buffer, the user has quit from Gedit */
    if ((ret_buf = ed_it(str)))
    {
	/* Check if buffer is empty */
	if (sizeof(ed_it_buffer) == 0 || !strlen(ed_it_buffer[0]))
	{
	    write("No text entered. Message aborted.\n");
	    if (gIs_reading)
		loop();
	}
	else
	{
	    write("Ok.\n");
	    gNew_message = implode(ret_buf, "\n") + "\n";
	    write("Cc:");
	    input_to("get_cc");
	}
	return;
    }
    else
	input_to("gedit_msg");
}

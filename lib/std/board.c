/*
 * /std/board.c
 *
 * The standard board code. Several options can be set:
 *
 * set_board_name(name)	 Path to the save directory for the board (no
 *			 default, this variable MUST be set) Old notes
 *			 will be stored in a directory with the same name,
 *			 with _old appended, if that option is checked.
 * set_num_notes(n)	 Set max number of notes. (default 30)
 * set_silent(n)	 0 = Tell room about read and examine of the board.
 *			 1 = silent read & examine. (default)
 * set_remove_rank(n)	 Minimum rank required to remove other players
 *			 notes. (default: WIZ_LORD, ie lords++)
 * set_remove_str(str)	 The string to send to the player if a remove
 *			 failed. (default "Only a Lord or higher can remove
 *			 other peoples notes.\n")
 * set_show_lvl(n)	 0 = Don't show wizard-rank in note header.
 *			 1 = show rank of writer in note header. (default)
 * set_no_report(n)	 0 = Keep the central board notified. (default)
 *			 1 = Don't notify the central board.
 *                           This hides the board for tools.
 * set_keep_discarded(n) 0 = don't keep old notes (default).
 *			 1 = keep old notes.
 *
 * There are three functions you can use to restrict usage of the board.
 * Independantly of the 'normal' access rules these functions can be used
 * to block/allow additional access. The administration and the owners of
 * the board are always allowed to manipulate the boards. In addition,
 * everyone is allowed to read or remove his/her own note. Usually read
 * and write access are blocked the same way. The right to remove other
 * peoples notes should be allowed separately. Hence the different names
 * and uses of these three functions:
 *
 * int block_reader(int note)
 * int block_writer()
 * int allow_remove(int note)
 *
 * You should only use these functions for board-internal use. To check the
 * access rights for people externally, there are three other functions.
 * This is to ensure some basic access rights for people who need to have
 * that and prevent that those rights are affected by the three forementioned
 * functions. Never call the block_ and allow_ functions externally. Always
 * call check_ externally. The functions return true to block access.
 *
 * int check_reader()
 * int check_writer()
 * int check_remove()
 *
 * The header format is:
 * subject (41)  0..40
 * length  ( 3) 42..44 ( 3 characters)
 * author  (11) 46..56 (11 characters in lower case)
 * rank    (10) 58..67 (10 characters, this field is optional)
 * date    ( 6) 69..74 ( 3 characters month, 2 characters day, i.e. "30 Jun")
 *
 * During display, the rank length is abbreviated to 7 characters.
 */

#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <files.h>
#include <macros.h>
#include <std.h>
#include <stdproperties.h>
#include <time.h>

/* It is not allowed to read notes larger than 100 lines without more. This
 * is done to prevent errors when trying to write too much text to the screen
 * of the player at once.
 */
#define MAX_NO_MREAD      100
#define MIN_HEADER_LENGTH  10
#define MAX_HEADER_LENGTH  41
#define MIN_NAME_LENGTH     2
#define MAX_NAME_LENGTH    11
#define AUTHOR_BEGIN       46
#define AUTHOR_END         56

#define GUEST_NAME        "guest"

#define READ_STAT	   0
#define WRITE_STAT	   1

/*
 * Global variables. They are not savable, the first two are private too,
 * which means that people cannot dump them.
 */
static private mixed   headers = ({ });
static private mapping writing = ([ ]);
static private int     *stats = ({ 0, 0 });

static string  board_name = "";
static string  remove_str = "Only a Lord or higher can remove other peoples notes.\n";
static int     keep_discarded = 0;
static int     notes = 30;
static int     silent = 1;
static int     msg_num;
static int     remove_rank = WIZ_LORD;
static int     show_lvl = 1;
static int     no_report = 0;
static int     fuse = 0;

/*
 * Prototypes.
 */
public nomask  int list_notes(string str);
public nomask  int new_msg(string msg_head);
public varargs int read_msg(string what_msg, int mr);
public nomask  int remove_msg(string what_msg);
public nomask  int rename_msg(string str);
public nomask  int store_msg(string str);
nomask private string *extract_headers(int number);

/*
 * Function name: set_num_notes
 * Description  : Set the maximum number of notes on the board.
 * Arguments    : int n - the number of notes on the board. (default: 30)
 */
public nomask void
set_num_notes(int n)
{
    notes = (fuse ? notes : n);
}

/*
 * Function name: set_silent
 * Description  : Set this to make the board silent. That means that
 *                bystanders are not noticed of people examining the
 *                board and reading notes.
 * Arguments    : int n - true if silent (default: true)
 */
public nomask void
set_silent(int n)
{
    silent = (fuse ? silent : (n ? 1 : 0));
}

/*
 * Function name: set_no_report
 * Description  : If you set this to true, the central board master is
 *                not notified of new notes. The only thing this does is
 *                preventing MBS from showing the board.
 * Arguments    : int n - true if the central board master should not
 *                        be notified. (default: false)
 */
public nomask void
set_no_report(int n)
{
    no_report = (fuse ? no_report : (n ? 1 : 0));
}

/*
 * Function name: set_board_name
 * Description  : This function sets the path in which the notes on the
 *                board are kept. This must _always_ be set!
 * Arguments    : strint str - the pathname of the board. It should not
 *                             end with a /.
 */
public nomask void
set_board_name(string str)
{
    board_name = (fuse ? board_name : str);
}

/*
 * Function name: query_board_name
 * Description  : Return the name of the board. This is the path of the
 *                directory in which the notes are stored.
 * Returns      : string - the name of the board (path-name).
 */
public nomask string
query_board_name()
{
    return board_name;
}

/*
 * Function name: set_remove_rank
 * Description  : With this function you can set the minimum rank people
 *                must have to be able to remove other peoples notes.
 *                NOTE: The argument to this function must be a define
 *                      from std.h, ie WIZ_NORMAL, WIZ_LORD, etc.
 * Arguments    : int n - the minimum level (default WIZ_LORD, ie lords++).
 */
public nomask void
set_remove_rank(int n)
{
    remove_rank = (fuse ? remove_rank : n);
}

/*
 * Function name: set_remove_str
 * Description  : Set the string to be printed when a player tries to
 *                remove a note while he is not allowed to do so.
 * Arguments    : string str - the message (default: Only a Lord is
 *                             allowed to remove other peoples notes.)
 */
public nomask void
set_remove_str(string str)
{
    remove_str = (fuse ? remove_str : str);
}

/*
 * Function name: set_show_lvl
 * Description  : Set this if you want the rank of the authors of the notes
 *                added to the header of the notes.
 * Arguments    : int n - true if the rank should be show (default: true)
 */
public nomask void
set_show_lvl(int n)
{
    show_lvl = (fuse ? show_lvl : (n ? 1 : 0));
}

/* 
 * Function name: query_show_lvl
 * Description  : Find out whether the rank of authors is shown.
 * Returns      : int - if true, the rank is show.
 */
public int
query_show_lvl()
{
    return show_lvl;
}

/*
 * Function name: set_keep_discarded
 * Description  : Set this if you want old notes to be kept. They will be
 *                kept if in a directory with the same name as the
 *                actual notes, though with _old appended to it.
 * Arguments    : int n - true if old notes should be kept (default: false)
 */
public nomask void
set_keep_discarded(int n)
{
    keep_discarded = (fuse ? keep_discarded : (n ? 1 : 0));
}

/*
 * Function name: query_author
 * Description  : Return the name of the author of a certain note. Observe
 *                that the note-numbers start counting at 1, like the way
 *                they appear on the board.
 * Argument     : int note - the number of the note to find the author of.
 * Returns      : string - the name of the author or 0 if the note doesn't
 *                         exist.
 */
public nomask string
query_author(int note)
{
    if ((note <= 0) ||
	(note > msg_num))
    {
	return 0;
    }

    return lower_case(implode(explode(
	headers[note - 1][0][AUTHOR_BEGIN..AUTHOR_END], " "), ""));
}

/*
 * Function name: query_stats
 * Description  : Return an array with the current statistics
 * Returns      : An array of stats, read & write 
 */
nomask public int *
query_stats()
{
    return stats + ({});
}

/*
 * Function name: no_special_fellow
 * Description  : Some people can always handle the board. If you own the
 *                board (same euid), if the board is in your domain, or
 *                if you are a member of the administration or the Lord of
 *                the domain the board it in, you can always meddle.
 * Returns      : int - 1/0 - true if (s)he is not a special fellow.
 */
nomask public int
no_special_fellow()
{
    string name = this_interactive()->query_real_name();
    string euid = geteuid();

    /* I'm an arch or keeper, so I can do anything. */
    if (SECURITY->query_wiz_rank(name) >= WIZ_ARCH)
	return 0;

    /* The board is mine */
    if (name == euid)
	return 0;

    /* The board is my domains */
    if (euid == SECURITY->query_wiz_dom(name))
	return 0;

    /* I am Lord and the board is of one of my wizards. */
    if ((SECURITY->query_wiz_rank(name) == WIZ_LORD) &&
	(SECURITY->query_wiz_dom(name) == SECURITY->query_wiz_dom(euid)))
	return 0;

    return 1;
}

/*
 * Function name: block_reader
 * Description  : If this_player() is not allowed to read notes of other
 *                people on this board, this function should be used to
 *                block access. If you print an error message on failure,
 *                please check whether this_player() is in the environment
 *                of the board.
 *                Redefine this function in your board code. It gets called
 *                automatically when someone tries to read.
 * Arguments      int note - Optional argument with the number of the note
 *                    the person wants to read. When 0, basic access to the
 *                    board.
 * Returns      : int - true if the player is NOT allowed to read.
 */
public varargs int
block_reader(int note = 0)
{
    return 0;
}

/*
 * Function name: check_reader
 * Description  : This function will return true if this_player() is allowed
 *                to read on the board. Note that players may always read
 *                their own notes.
 * Arguments    : int note - an optional argument for a particular note to
 *                           be checked. If omitted give general access to
 *                           the board.
 * Returns      : int 1/0 - true if the player is prohibited to read.
 */
public nomask varargs int
check_reader(int note = 0)
{
    /* No attempts to block readers. */
    if (!block_reader(note))
        return 0;

    /* People can always read their own notes. */
    if (note &&
	(this_player()->query_real_name() == query_author(note)))
	return 0;

    /* People with read access to the files can read the board, too. */
    if (SECURITY->valid_read(board_name + "/note", this_player()->query_real_name()))
        return 0;

    return no_special_fellow();
}

/*
 * Function name: block_writer
 * Description  : If this_player() is not allowed to write notes on this
 *                board, this function should be used to block access.
 *                If you print an error message on failure, please check
 *                whether this_player() is in the environment of the board.
 *                Redefine this function in your board code. It gets called
 *                automatically when someone tries to write.
 * Returns      : int 1/0 - true if the player is NOT allowed to write.
 */
public int
block_writer()
{
    return 0;
}

/*
 * Function name: check_writer
 * Description  : This function will return true if this_player() is allowed
 *                to write on the board.
 * Returns      : int 1/0 - true if the player may not write.
 */
public nomask int
check_writer()
{
    /* No attemps to block writers. */
    if (!block_writer())
        return 0;

    /* People with write access to the files can write notes, too. */
    if (SECURITY->valid_write(board_name + "/note", this_player()->query_real_name()))
        return 0;

    return no_special_fellow();
}

/*
 * Function name: allow_remove
 * Description  : This function checks whether this_player() is allowed
 *                to remove notes from this board. If you print an error
 *                message on failure, please check whether this_player()
 *                is in the environment of the board. This function works
 *                independant of the set_remove_rank function.
 * Arguments    : int note - the optional number of the note to be removed.
 *                    When 0, just general access is meant.
 * Returns      : int - true if the player is allowed to remove notes.
 */
public varargs int
allow_remove(int note)
{
    return 0;
}

/*
 * Function name: check_remove
 * Description  : This function will return true if this_player() is allowed
 *                to remove (other peoples) notes from the board.
 * Arguments    : int note - an optional argument for a particular note to
 *                           be checked. If omitted give general access to
 *                           the board.
 * Returns      : int 1/0 - true if the player may not remove the note.
 */
public nomask varargs int
check_remove(int note = 0)
{
    if (note &&
	(this_player()->query_real_name() == query_author(note)))
	return 0;

    return (no_special_fellow() &&
	    (SECURITY->query_wiz_rank(this_player()->query_real_name()) <
	     remove_rank) &&
            !allow_remove(note));
}

/*
 * Function name: load_headers
 * Description  : Load the headers when the board is created. This is done
 *                in a little alarm since it is also possible to configure
 *                the board by cloning it and then calling the set-functions
 *                externally. This function also sets the fuse that makes
 *                it impossible to alter the board-specific properties.
 */
private nomask void
load_headers()
{
    string *notes;

    /* Set the fuse to make it impossible to alter any of the board-specific
     * properties.
     */
    fuse = 1;

    seteuid(getuid());

    notes = get_dir(board_name + "/b*");
    msg_num = sizeof(notes);
    if (msg_num)
    {
        headers = map(sort_array(
            map(notes, &atoi() @ &extract(, 1))), extract_headers);
    }
    else
        headers = ({ });
}

/*
 * Function name: create_board
 * Description  : Since create_object() is nomasked, you must redefine this
 *                function in order to create your own board.
 */
void
create_board()
{
}

/*
 * Function name: create_object
 * Description  : Create the object. Use create_board() to set up the
 *                board yourself.
 */
nomask void
create_object()
{
    set_name("board");
    add_name("bulletinboard");
    set_adj("bulletin");
    set_short("bulletin board");
    set_long("It is a bulletin board.\n");

    add_prop(OBJ_M_NO_GET, "It's firmly secured to the ground.\n");

    create_board();

    /* We must do this with an alarm because it is also possible to clone
     * the /std/board.c and confugure it with the set_functions.
     */
    set_alarm(0.5, 0.0, load_headers);

    enable_reset();
}

/*
 * Function name: reset_board
 * Description  : Since reset_object() is nomasked, you must mask this
 *                function at reset time. Enable_reset() is already
 *                called, so you do not have to do that yourself.
 */
void
reset_board()
{
    if (!random(5))
	tell_room(environment(),
		  "A small gnome appears and secures some notes on the " +
		  short() + 
		  " that were loose.\nThe gnome then leaves again.\n");
}

/*
 * Function name: reset_object
 * Description  : Every half hour or about, the object resets. Use
 *                reset_board() for user reset functionality.
 */
nomask void
reset_object()
{
    reset_board();
}

/*
 * Function name: long
 * Description  : This function returns the long description on the board.
 *                If you don't have access to the board, you won't see the
 *                headers. NOTE that the arguments to this function are NOT
 *                the normal arguments.
 *                If the long description as set by set_long() contains ##,
 *                the text following ## will be posted at the bottom of the
 *                long, below the message headers. Mind the \n.
 * Arguments    : int start - the first note to print.
 *                int end   - the last note to print.
 * Returns      : string - the long description.
 */
public nomask varargs string
long(int start = 1, int end = msg_num)
{
    string name;
    string str;
    string post_long = "";
    string *parts;
    int allowed;

    str = check_call(query_long(), this_player());
    if (wildmatch("*##*", str))
    {
        parts = explode(str, "##");
        str = parts[0];
	if (sizeof(parts) > 1) post_long = parts[1];
    }
    str += "Usage: note <headline>, remove [note] <number>, " +
	"list [notes] <range>\n" +
	"       read/mread [note] <number>, " +
	"read/mread previous/current/next [note]\n";

    if (this_player()->query_wiz_level())
	str += "       store [note] <number> <file name>\n";

    if (!msg_num)
	return str + "The " + short() + " is empty.\n";

    str += "The " + short() + " contains " + msg_num +
	(msg_num == 1 ? " note" : " notes") + " :\n\n";

    if (!silent && present(this_player(), environment()) &&
        !this_player()->query_prop(OBJ_I_INVIS))
        say(QCTNAME(this_player()) + " studies the " + short() + ".\n");

    /* Check whether the range is valid. */
    if (end > msg_num)
	end = msg_num;

    if (start > end)
	start = end;

    if (start < 1)
	start = 1;

    start -= 2;

    /* If the player is not allowed to read the board, only display the
     * notes the player wrote him/herself.
     */
    allowed = !check_reader();
    name = this_player()->query_real_name();
    while (++start < end)
    {
	if (allowed || (name == query_author(start + 1)))
	{
	    str += sprintf("%2d: %s\n", (start + 1), headers[start][0] + " " +
	        TIME2FORMAT(atoi(headers[start][1][1..]),
	            (show_lvl ? "yy" : "yyyy")));
	}
    }

    return str + post_long;
}

/*
 * Function name: init
 * Description  : Link the commands to the player.
 */
public void
init()
{
    ::init();

    /* Only interactive players can write notes on boards. */
    if (!query_interactive(this_player()))
	return;

    add_action(list_notes, "list");
    add_action(new_msg,    "note");
    add_action(read_msg,   "read");
    add_action(read_msg,   "mread");
    add_action(remove_msg, "remove");
    add_action(rename_msg, "rename");
    add_action(store_msg,  "store");
}

/*
 * Function name: abbreviate_rank
 * Description  : Small support function to abbreviate the rank to 7 chars.
 * Arguments    : string - the title.
 * Returns      : string - the abbreviated title.
 */
string
abbreviate_rank(string title)
{
    if (!show_lvl)
    {
        return title;
    }
    if (title[65..65] != " ")
    {
        return title[..63] + "." + title[68..];
    }
    else
    {
        return title[..64] + title[68..];
    }
}

/*
 * Function name: extract_headers
 * Description  : This is a map function that reads the note-file and
 *                extracts the headers of the note.
 * Arguments    : int number - the number (time) of the note.
 * Returns      : string* - ({ header-string, note-name })
 */
private nomask string *
extract_headers(int number)
{
    string title;
    string file;

    if (!number)
        return 0;

    seteuid(getuid());

    file = "b" + number;
    if (!stringp(title = read_file(board_name + "/" + file, 1, 1)))
        return 0;

    /* Remove the newline from the title and abbreviate the rank. */
    title = abbreviate_rank(title[..-2]);

    return ({ title, file });
}

/*
 * Function name: query_latest_note
 * Description  : Find and return the name of the last note on the board.
 * Returns      : string - the filename (without path) of the last note
 *                         on the note or 0 if no notes are on the board.
 */
public nomask string
query_latest_note()
{
    return (msg_num ? headers[msg_num - 1][1] : 0);
}

/*
 * Function name: query_num_messages
 * Description  : Find a return the number of messages on this board.
 * Returns      : int - the number of messages on the board.
 */
public nomask int
query_num_messages()
{
    return msg_num;
}

/*
 * Function name: list_notes
 * Description  : With this command a player can list only a portion of
 *                the board.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
public nomask int
list_notes(string str)
{
    string *parts;
    int end;

    if (!stringp(str))
    {
	notify_fail("If you wish to list all headers you must either " +
	    "examine the board or type \"list notes\" in order not to " +
	    "get confused with other list commands.\n");
	return 0;
    }

    if (str == "notes")
    {
	write(long());
	return 1;
    }

    if (wildmatch("notes *", str))
	str = extract(str, 6);

    if (!wildmatch("*-*", str))
    {
	notify_fail("Error in range. Valid are \"start-end\", \"start-\" " +
	    "and \"-end\".\n");
	return 0;
    }

    /* Remove the possible spaces. */
    str = implode(explode(str, " "), "");

    /* Explode the range in sub-parts. */
    parts = explode(str, "-");
    switch(sizeof(parts))
    {
    case 1:
	end = msg_num;
	break;

    case 2:
	end = atoi(parts[1]);
	break;

    default:
	notify_fail("Error in range. Valid are \"start-end\", \"start-\" " +
	    "and \"-end\".\n");
	return 0;
    }

    write(long(atoi(parts[0]), end));
    return 1;
}

/*
 * Function name: new_msg
 * Description  : Write a note.
 * Arguments    : string msg_head - the header of the header.
 * Returns      : int 1/0 - success/failure.
 */
public nomask int
new_msg(string msg_head)
{
    string date;
    int    rank;
    object editor;

    if (this_player()->query_real_name() == GUEST_NAME)
    {
	write(break_string("You are a guest of " +
	    SECURITY->query_mud_name() + " and as such you cannot write on " +
	    "bulletin boards. If you want to participate in the discussion " +
	    "you should create yourself a real character. We are always " +
	    "pleased to welcome a new player to the game.", 75) + "\n");
	return 1;
    }

    if (this_player()->query_prop(PLAYER_I_NO_NOTES))
    {
        write("You have lost the privilige of posting notes on boards in " +
            "these realms.\n");
        return 1;
    }

    /* Player is not allowed to write a note on this board. */
    if (check_writer())
    {
        notify_fail("You are not allowed to write here.\n");
	return 0;
    }

    if (!stringp(msg_head))
    {
	notify_fail("Please add a header.\n");
	return 0;
    }
    if (strlen(msg_head) > MAX_HEADER_LENGTH)
    {
	write("Message header too long (limit = " + MAX_HEADER_LENGTH +
	    ", your length = " + strlen(msg_head) + ").\n");
	return 1;
    }

    if (present(this_player(), environment()) &&
        !this_player()->query_prop(OBJ_I_INVIS))
	say(QCTNAME(this_player()) + " starts writing a note.\n");

    this_player()->add_prop(LIVE_S_EXTRA_SHORT, " is writing a note");

    /* We use an independant editor and therefore we must save the header
     * the player has typed. When the player is done editing, the header
     * will be used again to save the message. Don't you just love these
     * kinds of statments.
     */
    rank = SECURITY->query_wiz_rank(this_player()->query_real_name());
    date = (show_lvl ? sprintf("%-10s ",
	capitalize(WIZ_RANK_NAME(rank))) : " ") +
	TIME2FORMAT(time(), "-d mmm");
    writing[this_player()] = sprintf("%-*s     %-11s %s", MAX_HEADER_LENGTH,
	msg_head, capitalize(this_player()->query_real_name()), date);

    seteuid(getuid());

    editor = clone_object(EDITOR_OBJECT);
    editor->set_activity("a note" +
        (environment() == environment(this_player()) ?
            (" on the " + short()) : "") + ".");
    editor->edit("done_editing", "");
    return 1;
}

/*
 * Function name: block_discard
 * Description  : If this function returns 1, it will prevent the
 *		  removal of a note.
 * Arguments	: file - the file to remove
 */
public int
block_discard(string file)
{
    return 0;
}

/*
 * Function name: discard_message
 * Description  : When there are too many notes on the board, the oldest
 *                is discarded. It is either moved or deleted.
 * Arguments    : string file - the file (without path) to save.
 */
private nomask void
discard_message(string file)
{
    seteuid(getuid());

    if (block_discard(file))
	return;

    if (keep_discarded)
    {
	if (file_size(board_name + "_old") == -1)
	    mkdir(board_name + "_old");

	rename(board_name + "/" + file, board_name + "_old/" + file);
    }
    else
	rm(board_name + "/" + file);
}

/*
 * Function name: post_note_hook
 * Description  : This function is called when someone posts a note.
 * Arguments    : string head - the header of the note.
 */
public void
post_note_hook(string head)
{
    if (present(this_player(), environment()))
        say(QCTNAME(this_player()) + " has completed a note (#" +
            msg_num + ") :\n" + head + "\n");
}

/*
 * Function name: post_note
 * Description  : This function actually posts the note on the board,
 *                stores it to disk and notifies the board master.
 * Arguments    : string head    - the header of the note.
 *                string message - the message body.
 */
private nomask void
post_note(string head, string message)
{
    string fname;
    int    t;

    /* If there are too many notes on the board, remove them. */
    while(msg_num >= notes)
    {
	discard_message(headers[0][1]);
	headers = exclude_array(headers, 0, 0);
	msg_num--;
    }

    seteuid(getuid());

    /* If the directory doesn't exist, create it. */
    if (file_size(board_name) == -1)
	mkdir(board_name);

    /* Check that the message file isn't used yet. */
    fname = "b" + (t = time());
    while(file_size(board_name + "/" + fname) != -1)
	fname = "b" + (++t);

    /* Write the message to disk and update the headers. */
    write_file(board_name + "/" + fname, head + "\n" + message);
    headers += ({ ({ abbreviate_rank(head), fname }) });
    msg_num++;

    /* Update the master board central unless that has been prohibited. */
    if (!no_report)
	BOARD_CENTRAL->new_note(board_name, fname, MASTER_OB(environment()));

    stats[WRITE_STAT]++;

    post_note_hook(head);
}

/*
 * Function name: done_editing
 * Description  : When the player is done editing the note, this function
 *                will be called with the message as parameter. If all
 *                is well, we already have the header.
 * Arguments    : string message - the note typed by the player.
 * Returns      : int - 1/0 - true if the note was added.
 */
public nomask int
done_editing(string message)
{
    string head;

    this_player()->remove_prop(LIVE_S_EXTRA_SHORT);

    if (!strlen(message))
    {
	write("No message entered.\n");
	if (present(this_player(), environment()) &&
          !this_player()->query_prop(OBJ_I_INVIS))
	    say(QCTNAME(this_player()) + " quits writing a note.\n");

	m_delkey(writing, this_player());
	return 0;
    }

    if (!stringp(writing[this_player()]))
    {
    	write("Your header has been lost! No note posted. " +
    	    "Please report this!\n");
	return 0;
    }

    head = writing[this_player()][..MAX_HEADER_LENGTH] + sprintf("%3d ",
	sizeof(explode(message, "\n"))) +
	writing[this_player()][AUTHOR_BEGIN..];
    m_delkey(writing, this_player());

    post_note(head, message);

    write("Ok.\n");
    return 1;
}

/*
 * Function name: create_note
 * Description  : This function can be called externally in order to write
 *                a note on a board generated by code. The note will then
 *                appear on the board just like any other note. There are a
 *                few restrictions to the note:
 *                - the euid of the object generating the note must be the
 *                  same as the euid of the board;
 *                - the name of the author may _not_ be the name of a player
 *                  in the game, mortal or wizard.
 *                In addition to that, you should ensure that the note is
 *                properly formatted and that it has newlines inserted at
 *                regular intervals, preferably before the 80th char/line.
 * Arguments    : string header - the header of the note (min: 10; max: 40)
 *                string author - the alleged author of the note  (max: 11)
 *                string body   - the note itself.
 * Returns      : int 1/0 - success/failure.
 */
public nomask int
create_note(string header, string author, string body)
{
    string head;
    string name;
    int    index;

    if (geteuid() != geteuid(previous_object()))
	return 0;

    /* The author may not be a real player and the length of the name
     * must be in the valid range. Remember the original capitalization.
     */
    name = lower_case(author);
    if ((strlen(name) > MAX_NAME_LENGTH) ||
	(strlen(name) < MIN_NAME_LENGTH) ||
	(SECURITY->exist_player(name)))
    {
	return 0;
    }

    if (this_interactive())
    {
	SECURITY->log_syslog("BOARD", ctime(time()) + ": " +
            capitalize(this_interactive()->query_real_name()) + 
            " posted on the board '" + 
            query_board_name() + 
            "' as '" + author + "'.\n");
    }

    /* Header size must be correct and there must be a body too. */
    if ((strlen(header) < MIN_HEADER_LENGTH) ||
	(strlen(header) > MAX_HEADER_LENGTH) ||
	(!strlen(body)))
    {
	return 0;
    }

    /* Author's name may only be letters, space or dash (-). */
    index = strlen(author);
    while(--index >= 0)
    {
	if ((name[index] < 'a') &&
	    (name[index] > 'z') &&
	    !IN_ARRAY(name[index], ({ '-', ' ' }) ))
	{
	    return 0;
	}
    }

    head = sprintf("%-*s %3d %-11s ", MAX_HEADER_LENGTH, header,
	sizeof(explode(body, "\n")), capitalize(author)) +
	(show_lvl ? "           " : " ") + TIME2FORMAT(time(), "-d mmm");

    post_note(head, body);

    tell_room(environment(), "You suddenly notice a note on the " +
	short() + " that you did not see before.\n");
    return 1;
}

/*
 * Function name: read_msg
 * Description  : Read a message on the board
 * Arguments    : string what_msg - the message number.
 *                int    mr       - read with more if true.
 * Returns      : int 1/0 - success/failure.
 */
public nomask varargs int
read_msg(string what_msg, int mr)
{
    int note;
    string text;

    if (!stringp(what_msg))
    {
	notify_fail("You read a while but as you can't decide exactly what " +
	    "to read, you don't find much of interest.\n");
	return 0;
    }

    if ((what_msg == "next") ||
	(what_msg == "next note"))
	note = this_player()->query_prop(PLAYER_I_LAST_NOTE) + 1;

    else if ((what_msg == "current") ||
             (what_msg == "current note"))
        note = this_player()->query_prop(PLAYER_I_LAST_NOTE);

    else if ((what_msg == "previous") ||
             (what_msg == "previous note"))
        note = this_player()->query_prop(PLAYER_I_LAST_NOTE) - 1;

    else if (!sscanf(what_msg, "note %d", note) &&
	!sscanf(what_msg, "%d", note))
    {
	notify_fail("Read which note?\n");
	return 0;
    }

    if ((note < 1) ||
	(note > msg_num))
    {
	notify_fail("Cannot read note " + note + ". There are " +
            msg_num + " messages on the " + short() + ".\n");
	return 0;
    }

    /* See whether the player can read the note. */
    if (check_reader(note))
    {
    	notify_fail("You cannot read that note.\n");
	return 0;
    }

    this_player()->add_prop(PLAYER_I_LAST_NOTE, note);
    if (present(this_player(), environment()))
        write("Reading note number " + note + ".\n");

    note--;
    if (!silent &&
        present(this_player(), environment()) &&
        !this_player()->query_prop(OBJ_I_INVIS))
        say(QCTNAME(this_player()) + " reads a note titled:\n" +
            headers[note][0] + "\n");

    seteuid(getuid());

    if (!mr)
	mr = (query_verb() == "mread");

    if ((!mr) &&
	(atoi(headers[note][0][42..44]) > MAX_NO_MREAD))
    {
	write("Too long note. More automatically invoked.\n");
	mr = 1;
    }

    text = headers[note][0] + " " +
        TIME2FORMAT(atoi(headers[note][1][1..]), "yyyy") + "\n\n";
    if (mr == 1)
	this_player()->more(text + read_file(board_name + "/" + headers[note][1], 2));
    else
    {
	write(text);
	cat(board_name + "/" + headers[note][1], 2);
    }

    /* Update the master board central unless that has been prohibited. */
    if (!no_report)
	BOARD_CENTRAL->read_note(board_name);

    stats[READ_STAT]++;

    return 1;
}

/*
 * Function name: remove_msg
 * Description  : Remove a note from the board.
 * Arguments    : string what_msg - the message to remove.
 * Returns      : int 1/0 - success/failure.
 */
public nomask int
remove_msg(string what_msg)
{
    int note;

    if (!stringp(what_msg))
    {
	notify_fail("Remove which note?\n");
	return 0;
    }

    if (!sscanf(what_msg, "note %d", note) &&
	!sscanf(what_msg, "%d", note))
    {
	notify_fail("Remove which note?\n");
	return 0;
    }

    if ((note < 1) ||
	(note > msg_num))
    {
	write("Message " + note + " does not seem to exist.\n");
	return 1;
    }

    if (check_remove(note))
    {
	write(remove_str);
	return 1;
    }

    note--;
    if (present(this_player(), environment()))
        say(QCTNAME(this_player()) + " removes a note:\n" +
            headers[note][0] + "\n");

    SECURITY->log_syslog("BOARD", ctime(time()) + ": " +
        capitalize(this_interactive()->query_real_name()) + 
        " removed a note on: " + query_board_name() +
        "\n--: " + headers[note][0] + "\n");
        
    discard_message(headers[note][1]);
    
    headers = exclude_array(headers, note, note);
    msg_num--;

    if ((note == msg_num) &&
	(!no_report))
        BOARD_CENTRAL->remove_note(board_name);

    stats[WRITE_STAT]--;

    write("Ok.\n");
    return 1;
}

/*
 * Function name: rename_msg
 * Description  : With this command, the wizard who wrote a note on his
 *                'own' board can rename the note to another players name.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
public nomask int
rename_msg(string str)
{
    string name;
    string char;
    string note;
    int    num;
    int    index = -1;
    int    len;

    if (no_special_fellow())
    {
        notify_fail("You are not allowed to rename messages on this board.\n");
	return 0;
    }

    if (!stringp(str))
    {
	notify_fail("Rename what?\n" +
	    "Syntax: rename <message number> <new author>\n");
	return 0;
    }

    if ((sscanf(str, "note %d %s", num, name) != 2) &&
	(sscanf(str, "%d %s", num, name) != 2))
    {
	notify_fail("Rename what?\n" +
	    "Syntax: rename <message number> <new author>\n");
	return 0;
    }

    if ((num < 1) ||
	(num > msg_num))
    {
	notify_fail("There are only " + msg_num + " notes.\n");
	return 0;
    }

    if (this_player()->query_real_name() != query_author(num))
    {
	write("You are not the author of that note.\n");
	return 1;
    }

    len = strlen(name = lower_case(name));
    if ((len < MIN_NAME_LENGTH) ||
	(len > MAX_NAME_LENGTH) ||
	(SECURITY->exist_player(name)))
    {
	write("The name of the alleged new author is not valid.\n");
	return 1;
    }

    while(++index < len)
    {
	char = name[index..index];
	if ((char < "a") &&
	    (char > "z") &&
	    (char != "-"))
	{
	    write("The name of the alleged new author contains " +
		"invalid characters\n");
	    return 1;
	}
    }

    seteuid(getuid());

    num--;
    headers[num][0] = headers[num][0][..45] +
	sprintf("%-11s", capitalize(name)) + headers[num][0][57..];
    note = headers[num][0] + "\n" +
	read_file(board_name + "/" + headers[num][1], 2);
    rm(board_name + "/" + headers[num][1]);
    write_file(board_name + "/" + headers[num][1], note);

    write("Author on note " + (++num) + " changed from " +
	capitalize(this_player()->query_real_name()) + " to " +
	capitalize(name) + ".\n");
    return 1;
}

/*
 * Function name: store_msg
 * Description  : Store a message to disk.
 * Arguments    : string str - the message to store and the filename.
 * Returns      : int 1/0 - success/failure.
 */
nomask public int
store_msg(string str)
{
    int    note;
    string file;

    if (SECURITY->query_wiz_rank(this_player()->query_real_name()) <
	WIZ_NORMAL)
    {
	notify_fail("Sorry, you can't save messages.\n");
	return 0;
    }

    if (!stringp(str))
    {
	notify_fail("Store which note?\n");
	return 0;
    }

    if ((sscanf(str, "note %d %s", note, file) != 2) &&
	(sscanf(str, "%d %s", note, file) != 2))
    {
	notify_fail("Store which note where?\n");
	return 0;
    }

    if ((note < 1) ||
	(note > msg_num))
    {
	write("That message doesn't exist.\n");
	return 1;
    }

    if (check_reader(note))
    {
	write("You are note allowed to store that note.\n");
	return 1;
    }

    note--;
    str = (headers[note][0] + "\n\n" +
	   read_file((board_name + "/" + headers[note][1]), 2));
    EDITOR_SECURITY->board_write(file, str);

    /* We always return 1 because we don't want other commands to be
     * evaluated.
     */
    return 1;
}

/*
 * Function name: query_headers
 * Description  : Return the headers if this_player() is allowed to see
 *                them.
 * Returns      : mixed - the headers in the following format:
 *                        ({ ({ string header, string filename }) })
 *                        or just ({ }) if no read access.
 */
public nomask mixed
query_headers()
{
    return (check_reader() ? ({ }) : secure_var(headers) );
}

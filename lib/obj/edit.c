/*
 * /obj/edit.c
 *
 * This object is a basic line editor. It supports some simple functions
 * that allow you to manipulate text you are writing. If you want to allow
 * a player to edit a text, clone this object and make the following call:
 *
 * clone_object(EDITOR_OBJECT)->edit(mixed ffun, string text, int begin);
 *
 * All parameters are optional and have the following meaning:
 *
 * mixed ffun  - the name of the function called in the calling object when
 *               done editing. If omitted or 0, the function done_editing()
 *               is called. This can be a string or a functionpointer.
 * string text - if this parameter is added, this text will be the default
 *               text the player can edit and append to.
 * int begin   - start editing at this line number. Start counting at line 1.
 *
 * You can optionally give information on what the player is editing using the
 * routine set_activity().
 *
 * When the player is done editing, the function done_editing(string text)
 * is called in the object that called us. If you want another function to
 * be called, use the first parameter to set something different.
 *
 * NOTE: edit() operates on this_player(). If this_player() is not valid
 *       when edit() is called, nothing can be edited.
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <files.h>
#include <macros.h>
#include <options.h>
#include <stdproperties.h>

#define EDITOR_SUBLOC   "editor_subloc"
#define RETURN_FUNCTION "done_editing"
#define EDITOR_ID       "_editor_"
#define EDIT_END        "**"

/* Prototype.
 */
static void input(string str);

/* Global variables. They are secure since we do not want people snooping.
 * The variable 'line' keeps the number of the next line in terms of code.
 * The player will type the first line (1) and the game has 'line' at 0.
 */
static private object  calling_ob;   /* the object that called us.   */
static private mixed   finished_fun; /* function to call when done.  */
static private string *lines;        /* the lines of the text.       */
static private int     line;         /* the number of the next line. */
static private string  activity;     /* the thing we are writing.    */

/*
 * Function name: create_object
 * Description  : Called to create the object.
 */
public void
create_object()
{
    set_name("editor");
    add_name(EDITOR_ID);

    set_adj("basic");
    add_adj("line");

    set_short("basic line editor");
    set_long("It is a basic line editor. You should never see this. In " +
        "order to get information on it, examine the sourcecode. " +
        "/Mercade.\n");

    remove_prop(OBJ_I_VALUE);
    remove_prop(OBJ_I_VOLUME);
    remove_prop(OBJ_I_WEIGHT);
    add_prop(OBJ_M_NO_DROP,     1);
    add_prop(OBJ_M_NO_GET,      1);
    add_prop(OBJ_M_NO_GIVE,     1);
    add_prop(OBJ_M_NO_STEAL,    1);
    add_prop(OBJ_M_NO_TELEPORT, 1);

    set_no_show();

    setuid();
    seteuid(getuid());

    /* "<player> is writing something." */
    activity = "something.";
}

/*
 * Function name: set_activity
 * Description  : Sets the activity of the person that is using the editor.
 *                This should fit in the text "You are writing <activity>"
 *                and include a period or exclamation mark.
 * Arguments    : string str - the activity description.
 */
void
set_activity(string str)
{
    activity = str;
}

/*
 * Function name: query_activity
 * Description  : Get the description of the activity. It completes the
 *                sentence "You are writing <activity>".
 * Returns      : string - the activity.
 */
string
query_activity()
{
    return activity;
}

/*
 * Function name: done_more
 * Description  : Called when the player is done reviewing the text with more.
 */
void
done_more()
{
    input("~done_more");
}

/*
 * Function name: display
 * Description  : Shows the previous lines before the current line.
 * Arguments    : int num - the number of lines to show.
 *                          -1 - show the whole text with line numbers.
 *                          -2 - show the whole text without line numbers.
 *                int use_more - if true, show with more.
 */
static varargs void
display(int num, int use_more = 0)
{
    int index;
    int start;
    int end;
    int size;
    string text = "";

    /* Not possible to list negative line numbers. 0 Lines is nonsense too. */
    if ((num == 0) ||
        (num < -2) ||
        (!(size = sizeof(lines))))
    {
        return;
    }

    if (num < 0)
    {
        start = 0;
        end = size - 1;
    }
    else
    {
        end = (line - 1);
        start = ((num > end) ? 0 : (end - num + 1));
    }

    if (num == -2)
    {
        for (index = start; index <= end; index++)
        {
            text += (lines[index] + "\n");
        }
    }
    else
    {
        for (index = start; index <= end; index++)
        {
            text += (sprintf("%2d]%s\n", (index + 1), lines[index]));
        }
    }

    if (use_more || (strlen(text) > 3000))
    {
        this_player()->more(text, 0, done_more);
    }
    else
    {
        write(text);
    }
}

/*
 * Function name: finished
 * Description  : Called when the user is finished editing a text. It will call
 *                the report function in the calling object.
 * Arguments    : string str - the text entered.
 */
static void 
finished(string str)
{
    if (functionp(finished_fun))
    {
        function f = finished_fun;
        f(str);
    }
    else
    {
        call_other(calling_ob, finished_fun, str);
    }
}

/*
 * Function name: edit
 * Description  : Lets the player edit a text. All three paramters described
 *                below are optional.
 *                Note that the function operates on this_player(). If there
 *                is no this_player(), then nobody can edit anything.
 * Argmuents    : mixed ffun - the name or function pointer of the function to
 *                    call in the object that called us when the players is
 *                    done editing. If omitted, the function "done_editing" is
 *                    called by default.
 *                string str - the text the players wants to edit or append
 *                    to. If you want to start on a clean sheet, then this
 *                    argument may be omitted.
 *                int begin - start editing at line x of the text added with
 *                    the second argument. If you want to append to the end of
 *                    the text, this argument may be omitted. Otherwise start
 *                    counting the lines at line 1.
 */
public varargs void
edit(mixed ffun, string str = "", int begin = 0)
{
    calling_ob = previous_object();

    if (functionp(ffun))
    {
        finished_fun = ffun;
    }
    else if (!stringp(ffun) || !strlen(ffun))
    {
        finished_fun = RETURN_FUNCTION;
    }
    else
    {
        finished_fun = ffun;
    }

    /* If this_player does not exist, nothing can be edited. */
    if (!objectp(this_player()))
    {
        finished(str);
        remove_object();
        return;
    }

    /* We move the editor into the player to save on linkdeath. */
    move(this_player(), 1);

    lines = (strlen(str) ? explode(str, "\n") : ({ }) );
    line  = sizeof(lines);

    if (begin &&
        (begin < line))
    {
        line = begin - 1;
    }

    write("Extended editor! Type ~? for help and " + EDIT_END +
        " to finish editing.\n");
    display(10);

    write(sprintf("%2d]", (line + 1)));
    input_to(input);
}

/*
 * Function name: add_cc
 * Description  : When writing mail, add people to the CC-list.
 * Arguments    : string arg - the command line argument.
 */
static void
add_cc(string arg)
{
    string *names;

    if (function_exists("create_object", calling_ob) != MAIL_READER)
    {
        write("You are not writing mail, so you cannot add the CC-list.\n");
        return;
    }

    if (!strlen(arg))
    {
        write("No names given to add to the CC-list.\n");
        return;
    }

    names = explode(calling_ob->cleanup_string(arg), " ");
    if (strlen(arg = calling_ob->check_mailing_list(names)))
    {
        write("The name " + arg +
            " is not a valid recipient. No CC-named added.\n");
        return;
    }

    write("Names added to CC-list.\n");
    calling_ob->editor_add_to_cc_list(names);
}

/*
 * Function name: delete
 * Description  : Player wants to delete one or more lines. The format
 *                is '3' or '3-5' or '3,5' or mixed: '3,5-7'.
 * Arguments    : string arg - the command line argument.
 */
static void
delete(string arg)
{
    int    *range = ({ });
    string *parts;
    int     left;
    int     right;
    int     index;
    int     index2;

    if (!strlen(arg))
    {
        write("No lines specified for deletion.\n");
        return;
    }
 
    arg   = implode(explode(arg, " "), "");
    parts = explode(arg, ",");

    for (index = 0; index < sizeof(parts); index++)
    {
        if (sscanf(parts[index], "%d-%d", left, right) == 2)
        {
            if ((left < 1) || (left > sizeof(lines)) ||
                (right < 1) || (right > sizeof(lines)))
            {
                range = 0;
                break;
            }

            if (right < left)
            {
                index2 = left;
                left   = right;
                right  = index2;
            }

            for (index2 = left; index2 <= right; index2++)
            {
                range -= ({ index2 });
                range += ({ index2 });
            }
        }
        else
        {
            index2 = atoi(parts[index]);

            if ((index2 < 1) || (index2 > sizeof(lines)))
            {
                range = 0;
                break;
            }

            range -= ({ index2 });
            range += ({ index2 });
        }
    }

    if ((!pointerp(range)) ||
        (!sizeof(range)))
    {
        write("Syntax error with delete. No lines removed.\n");
        return;
    }

    range = sort_array(range);
    for(index = (sizeof(range) - 1); index >= 0; index--)
    {
        if (range[index] <= line)
        {
            line--;
        }
        lines[range[index] - 1] = 0;
    }

    lines -= ({ 0 });
}

/*
 * Function name: restore
 * Description  : When your link dies and you are editing something, the
 *                edit buffer may have been saved. The restore command
 *                restores it for you.
 */
#ifdef EDITOR_SAVE_OBJECT
static void
restore()
{
    string message;

    setuid();
    seteuid(getuid());

    /* The function restore() in the EDITOR_SAVE_OBJECT operates on
     * this_player().
     */
    message = EDITOR_SAVE_OBJECT->restore();

    if (!strlen(message))
    {
        write("Sorry, but there was no message stored for you.\n");
        return;
    }

    lines = explode(message, "\n");
    line  = sizeof(lines);

    display(10);
}
#endif EDITOR_SAVE_OBJECT

/*
 * Function name: replace
 * Description  : When you want to replace a single line with new text,
 *                it is easier to use this command than to delete the old
 *                line(s), possibly change the point of entry and then
 *                type the new text.
 * Arguments    : string arg - the line that needs to be replaced in the
 *                             format: <num> <text>
 */
static void
replace(string arg)
{
    int num;

    if (!strlen(arg) ||
        (sscanf(arg, "%d %s", num, arg) != 2))
    {
        write("Syntax: ~c <num> <new text>\n");
        return;
    }

    if ((num < 1) ||
        (num > sizeof(lines)))
    {
        write("No line " + num + " in the text.\n");
        return;
    }

    lines[num - 1] = arg;
    display(10);
}

/*
 * Function name: import
 * Description  : "Normal" wizards may ftp a file into Genesis and then
 *                import it into the editor.
 * Arguments    : string arg - the filename of the file to import.
 * Returns      : int        - if true, the player added an EDIT_END
 *                             in his file, so we finish editing.
 */
static int
import(string arg)
{
    string *import_lines;
    string  import_text = EDITOR_SECURITY->import(arg);

    /* If this happens, an error message is printed by the EDITOR_SECURITY. */
    if (!strlen(import_text))
    {
        return 0;
    }

    import_lines = explode(import_text, "\n");

    /* Append line at the end of the text, or if there is no text yet,
     * just append it to the empty array.
     */
    if (line == sizeof(lines))
    {
        lines += import_lines;
    }
    /* Insert before the first line. */
    else if (line == 0)
    {
        lines = import_lines + lines;
    }
    /* Insert somewhere between lines. */
    else
    {
        lines = lines[..(line - 1)] + import_lines + lines[line..];
    }

    /* If the player included the EDIT_END in the imported file, the
     * editor will finish editing.
     */
    if (member_array(EDIT_END, import_lines) != -1)
    {
        lines -= ({ EDIT_END });

        if (objectp(calling_ob))
        {
            arg = (sizeof(lines) ? (implode(lines, "\n") + "\n") : "");
            finished(arg);
        }
        else
        {
            write("\nEDIT ERROR. No calling object. Edit discarded.\n");
        }

        remove_object();
        return 1;
    }

    /* Adjust the number of the next line to be added and display the
     * last part of the message imported.
     */
    line += sizeof(import_lines);
    display(10);

    return 0;
}

/*
 * Function name: checkrange
 * Description  : Checks whether the current line is valid after the
 *                player used ~i or ~a. If not, set it to something valid.
 */
static void
checkrange()
{
    if (line < 0)
    {
        write("Invalid argument. Changed to line 1.\n");
        line = 0;
    }

    if (line > sizeof(lines))
    {
        line = sizeof(lines);
        write("Invalid argument. Changed to line " + (line + 1) + ".\n");
    }
}

/*
 * Function name: autowrap
 * Description  : People who have used 80 characters or more in their text
 *                are prompted to have their message autowrapped for optimal
 *                readability
 * Arguments    : string str - the input.
 */
static void
autowrap(string str)
{
    str = (strlen(str) ? lower_case(str) : "y");
    switch(str[0])
    {
    case '~':
    case 'q':
        if ((str == "~q") || (str == "q"))
        {
            finished("");
            remove_object();
            return;
        }

    case 'n':
        write("The text will not be auto-wrapped.\n");
        break;

    case 'a':
        write("Auto-wrapping switched on. The text will be auto-wrapped.\n");
        this_player()->set_option(OPT_AUTOWRAP, 1);
        lines = map(lines, &break_string(, 79));
        break;

    case 'y':
        write("The text will be auto-wrapped.\n");
        lines = map(lines, &break_string(, 79));
        break;

    default:
        write("Please answer with y[es], n[o], [a]lways or q[uit]. Autowrap? [yes] ");
        input_to(autowrap);
        return;
    }

    str = implode(lines, "\n") + "\n";
    finished(str);
    remove_object();
}

/*
 * Function name: input
 * Description  : Called with input from the player.
 * Arguments    : string str - the text typed by the player.
 */
static void
input(string str)
{
    string arg;
    int    cmd;

    /* Player finished editing. Lets wrap it up and return operation
     * to the calling object.
     */
    if (str == EDIT_END)
    {
        if (!objectp(calling_ob))
        {
            write("\nEDIT ERROR. No calling object. Edit discarded.\n");
            remove_object();
            return;
        }

        /* If people type too long lines, ask them to auto-wrap. */
        if (sizeof(filter(lines, &operator(>=)(, 80) @ strlen)))
        {
            if (this_player()->query_option(OPT_AUTOWRAP))
            {
                write("Auto-wrapping the text.\n");
                lines = map(lines, &break_string(, 79));
            }
            else
            {
                write("The text you entered contains lines with a length " +
                    "of 80 characters or more. This may make it difficult " +
                    "to read using more. Would you like the editor to " +
                    "auto-wrap your text? yes/no/always/quit [default: yes] ");
                input_to(autowrap);
                return;
            }
        }

        str = (sizeof(lines) ? (implode(lines, "\n") + "\n") : "");
        finished(str);
        remove_object();
        return;
    }

    /* Player may start a line with ~~ to indicate that he wants to start
     * the line with a tilde '~'.
     */
    cmd = wildmatch("~*", str);
    if (cmd &&
        wildmatch("~~*", str))
    {
        cmd = 0;
        str = extract(str, 1);
    }

    /* Player entered yet another line, ie. not a special command. Lets
     * add it to the text.
     */
    if (!cmd)
    {
        /* Append line at the end of the text, or if there is no text yet,
         * just append it to the empty array.
         */
        if (line == sizeof(lines))
        {
            lines += ({ str });
        }
        /* Insert before the first line. */
        else if (line == 0)
        {
            lines = ({ str }) + lines;
        }
        /* Insert somewhere between lines. */
        else
        {
            lines = lines[..(line - 1)] + ({ str }) + lines[line..];
        }

        line++;
        write(sprintf("%2d]", (line + 1)));
        input_to(input);
        return;
    }

    /* Player gave a special command. Lets extract the argument. */
    sscanf(str, "%s %s", str, arg);
    switch(str)
    {
    /* Append after line <arg>. Default: append after end of text. */
    case "~a":
        if (strlen(arg))
        {
            line = atoi(arg);
            checkrange();
        }
        else
        {
            line = sizeof(lines);
        }
        display(10);
        break;

    /* Replace one line with new text. */
    case "~c":
        replace(arg);
        break;

    /* Add people to your CC list when sending mail. */
    case "~cc":
        add_cc(arg);
        break;

    /* Delete line(s) <arg>. */
    case "~d":
        delete(arg);
        break;

    /* Done displaying more. Don't do anything. */
    case "~done_more":
        break;

    /* Player wants help on the commands.
     */
    case "~h":
    case "~?":
        write(
"Editor Commands:\n\n" +
"<text>     Yet another line to add to the text.\n" +
EDIT_END + "         Finish text.\n" +
"~? or ~h   Print this help message.\n" +
"~q         Cancel editing and discard text.\n" +
"~l (~L)    Show complete text with(out) line numbers.\n" +
"~s [<num>] Show last <num> lines. Default: 10.\n" +
"~a [<num>] Append at the end of the text. [After line <num>].\n" +
"~i [<num>] Insert before the text. [Before line <num>].\n" +
"~c <num> <text>  Replace the text in line <num> with <text>.\n" +
"~d <range> Delete <range>. Format: '3', '3-5', '3,5' or mixed '3,5-6'.\n" +
    (this_player()->query_wiz_level() ?
"~m <file>  Import the file <file> into the editor.\n" : "") +
#ifdef EDITOR_SAVE_OBJECT
"~r         Restore a message you were editing while you went linkdead.\n" +
#endif EDITOR_SAVE_OBJECT
"~cc <name> Add the person/people in <name> to the CC list when mailing.\n" +
"!<command> Execute command <command> and then return to the editor.\n" +
"The auto-wrap option will wrap long lines if you want it to.\n\n");
        break;

    /* Insert in front of line <arg>. Default: insert before text. */
    case "~i":
        if (strlen(arg))
        {
            line = atoi(arg);
            line--;
            checkrange();
        }
        else
        {
            line = 0;
        }
        display(10);
        break;

    /* Show the complete text with line numbers. */
    case "~l":
    case "~lm":
        display(-1, (str == "~lm"));
        break;

    /* Show the complete text without line numbers. */
    case "~L":
    case "~Lm":
        display(-2, (str == "~Lm"));
        break;

    /* Import a file into the editor. (Wizards only.) */
    case "~m":
        if (this_player()->query_wiz_level() &&
            import(arg))
        {
            return;
        }
        break;

    /* Cancel editing. No text is returned. */
    case "~q":
        if (objectp(calling_ob))
        {
            finished("");
        }
        else
        {
            write("\nEDIT ERROR. No calling object. Edit discarded.\n");
        }

        remove_object();
        return;
        break;

#ifdef EDITOR_SAVE_OBJECT
     /* Restore a message you were editing while you went linkdead. */
     case "~r":
        restore();
        break;
#endif EDITOR_SAVE_OBJECT

    /* Show <arg> lines. Default: 10 lines. */
    case "~s":
        display(strlen(arg) ? atoi(arg) : 10);
        break;

    /* We do not recognize the command. Give an error message. */
    default:
        write("Unknown command. Type ~? for help or " + EDIT_END +
            " to end editing.\n");
        break;
    }

    write(sprintf("%2d]", (line + 1)));
    input_to(input);
}

/*
 * Function name: linkdie
 * Description  : When the player holding this editor linkdies, the
 *                edit-buffer is saved by the EDITOR_SAVE_OBJECT.
 *                When (s)he re-links, the buffer can be restored and
 *                the message continued.
 */
#ifdef EDITOR_SAVE_OBJECT
public void
linkdie()
{
#ifdef OWN_STATUE
    if (!CALL_BY(OWN_STATUE))
    {
        return;
    }
#endif OWN_STATUE

    if (objectp(calling_ob))
    {
        finished("");
    }

    if (!sizeof(lines))
    {
        remove_object();
        return;
    }

    setuid();
    seteuid(getuid());

    EDITOR_SAVE_OBJECT->linkdie(implode(lines, "\n"));
    remove_object();
}
#endif EDITOR_SAVE_OBJECT

/*
 * Function name: enter_env
 * Description  : When we enter an interactive environment, add a subloc to
 *                indicate that the person is writing something.
 * Arguments    : object to - the object we are entering.
 *                object from - the object we come from.
 */
void
enter_env(object to, object from)
{
    ::enter_inv(to, from);

    if (interactive(to))
    {
        to->add_subloc(EDITOR_SUBLOC, this_object());
    }
}

/*
 * Function name: leave_env
 * Description  : When we leave an environment, remove the possibly added
 *                subloc.
 * Arguments    : object from - the object we are leaving.
 *                object to - the object we go to.
 */
void
leave_env(object from, object to)
{
    ::leave_env(from, to);

    from->remove_subloc(EDITOR_SUBLOC);
}

/*
 * Function name: show_subloc
 * Description  : Shows the subloc of the editor, it shows that the player is
 *                writing something when you examine him.
 * Arguments    : string subloc - the subloc identifier.
 *                object carrier - the person on which the subloc should be
 *                    displayed.
 *                object for_obj - the person that is looking.
 */
string
show_subloc(string subloc, object carrier, object for_obj)
{
    if (carrier->query_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS) ||
        (subloc != EDITOR_SUBLOC))
    {
        return "";
    }

    if (for_obj == carrier)
    {
        return "You are writing " + activity + "\n";
    }

    return capitalize(carrier->query_pronoun()) + " is writing " + activity +
        "\n";
}

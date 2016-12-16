/*
 * /cmd/wiz/normal/edit.c
 *
 * This is a subpart of /cmd/wiz/normal.c
 * 
 * The commands in this sub-part all have to do with editing stuff.
 *
 * Commands in this file:
 * - exec
 * - execr
 * - pad
 */

#include <files.h>
#include <macros.h>
#include <options.h>
#include <std.h>

#define EXEC_DONE	("exec_done_editing")
#define PAD_DONE	("pad_done_editing")
#define REOPEN_SOUL	("reopen_soul")

#define WIZARD_S_PAD_MESSAGE ("_wizard_s_pad_message")

/*
 * Global variables. They are not savable.
 *
 * exec_code - this mapping contains the last exec-code executed by a
 *             particular wizard to allow for the repeat function. It is
 *             kept locally in here so no-one can tamper with it.
 * pad_notes - the notepad of the wizard currently handling the pad.
 */
static private mapping exec_code = ([ ]);
static private mapping pad_notes = 0;

/* **************************************************************************
 * exec - Execute one or more lines of code
 * execr - Built in alias for 'exec return'.
 */

/*
 * Function name: exec_write
 * Description  : Write an object file with a function exec_fun()
 *		  that is to be executed. The filename of that file
 *		  is ~/exec_obj.c.
 * Arguments    : string str - the body of the function exec_fun().
 * Returns      : 1 - always.
 * Caveats      : If "str" contains non-executable code, the object
 *		  cannot be loaded, and the gamedriver will give an
 *		  error.
 */
private nomask int
exec_write(string str)
{
    object ob;
    string file, master;
    string name;
    mixed error;
    mixed result;

    seteuid(getuid());

    if (!strlen(str))
    {
	write("Nothing to execute.\n");
	return 1;
    }

    name = this_interactive()->query_real_name();
    exec_code[name] = str;

    file = SECURITY->query_wiz_path(name);
    if (!strlen(file))
    {
	write("Cannot write file.\n");
	return 1;
    }

    master = file + "/exec_obj";
    file += "/" + name + ".h";

    /* Remove the old copy */
    rm(master + ".c");

    /* Write the new file */
    if (!write_file(master + ".c",
	"#pragma no_clone\n\n" +
	"#include <adverbs.h>\n" +
	"#include <alignment.h>\n" +
	"#include <cmdparse.h>\n" +
	"#include <comb_mag.h>\n" +
	"#include <composite.h>\n" +
	"#include <config.h>\n" +
	"#include <const.h>\n" +
	"#include <drink_eat.h>\n" +
	"#include <filepath.h>\n" +
	"#include <files.h>\n" +
	"#include <filter_funs.h>\n" +
	"#include <formulas.h>\n" +
	"#include <herb.h>\n" +
	"#include <language.h>\n" +
	"#include <living_desc.h>\n" +
	"#include <log.h>\n" +
	"#include <login.h>\n" +
	"#include <macros.h>\n" +
	"#include <mail.h>\n" +
	"#include <math.h>\n" +
	"#include <mbs.h>\n" +
	"#include <money.h>\n" +
	"#include <obflags.h>\n" +
	"#include <options.h>\n" +
	"#include <poison_types.h>\n" +
	"#include <seqaction.h>\n" +
	"#include <ss_types.h>\n" +
	"#include <state_desc.h>\n" +
	"#include <std.h>\n" +
	"#include <stdproperties.h>\n" +
	"#include <subloc.h>\n" +
	"#include <tasks.h>\n" +
	"#include <terrain.h>\n" +
	"#include <time.h>\n" +
	"#include <udp.h>\n" +
	"#include <wa_types.h>\n\n" +
	"#include \"" + file + "\"\n" +
	"#include <exec.h>\n\n" +
	"object\nparse(string str)\n{\n    return " +
	"TRACER_TOOL_SOUL->parse_list(str);\n}\n\n" +
	"mixed\nexec_fun()\n{\n" +
	"    mixed a, b, c, d, e, f, g, h, i, j, k, l, m;\n\n" +
	"    setuid();\n    seteuid(getuid());\n    {\n" +
	str + "\n    }\n\n    destruct();\n}\n"))
    {
	write("Cannot write file.\n");
	return 1;
    }

    /* See if the player already has a personal .h file. */
    if (file_size(file) <= 0)
    {
	write("You do not have a file \"" + name +
	      ".h\" file in your home directory.\n" +
	      "One will be created for you. " +
	      "Adjust that file as you please.\n");

	if (!write_file(file,
	    "/*\n * " + file + "\n *\n * This file contains your personal " +
	    "definitions to be used in exec.\n * Check out <exec.h> " +
	    "before adding definitions to this file.\n */\n\n"))
	{
	    write("Could not create \"" + file + "\"\n.");
	    return 1;
	}
    }

    /* Update the object, so it will be reloaded. This should not happen
     * normally, but it is always better to be sure ;-)
     */
    if (objectp(ob = find_object(master)))
    {
	SECURITY->do_debug("destroy", ob);
    }
    
    /* Load the object and beware of errors. */
    if (error = catch(call_other(master, "teleledningsanka")))
    {
	write("\nError when loading: " + error + "\n");
	return 1;
    }

    /* Call the function, but beware of errors. */
    error = catch(result = call_other(master, "exec_fun"));

    /* When set_this_player() has been used, reset it. */
    if (this_player() != this_interactive())
    {
	set_this_player(this_interactive());
    }

    if (stringp(error))
    {
	write("\nError when executing: " + error + "\n");
    }
    else
    {
        write("Result: ");
        print_value(result);
    }

    return 1;
}

/*
 * Function name: exed_done_editing_reloaded
 * Description  : When the euid of this object has been set to the euid
 *                of the wizard again, we can execute the lines.
 */
public nomask void
exec_done_editing_reloaded()
{
    if ((previous_object() != this_interactive()) ||
	(calling_function() != REOPEN_SOUL))
    {
	write("Illegal call to exec_done_editing_reloaded().\n");
	return;
    }

    exec_write(exec_code[this_interactive()->query_real_name()]);
}

/*
 * Function name: exec_done_editing
 * Description  : When the wizard is done editing the exec-text, this
 *                function is called with the text. We call the function
 *                reload_soul in the player to get an euid into the soul
 *                and then execute the stuff.
 * Arguments    : string str - the 
 */
public nomask void
exec_done_editing(string str)
{
    if (MASTER_OB(previous_object()) != EDITOR_OBJECT)
    {
	write("Illegal call to exec_done_editing().\n");
	return;
    }

    exec_code[this_interactive()->query_real_name()] = str;
    call_other(this_interactive(), REOPEN_SOUL);
}

nomask int
exec_code(string str)
{
    string name = this_interactive()->query_real_name();

    /* Wizard wants to edit the previous exec-code before execution. */
    if ((str == "e") ||
	(str == "edit"))
    {
	if (!strlen(exec_code[name]))
	{
	    write("You have no previous exec-code to edit.\n");
	    /* Intentional no return 1; The player can enter new code. */
	}
	else
	{
	    clone_object(EDITOR_OBJECT)->edit(EXEC_DONE, exec_code[name],
		sizeof(explode(exec_code[name], "\n")));
	    return 1;
	}
    }
    /* Wizard wants to repeat the execution of the previous exec-code. */
    else if ((str == "r") ||
	     (str == "repeat"))
    {
	if (!strlen(exec_code[name]))
	{
	    notify_fail("You have no previous exec-code to repeat.\n");
	    return 0;
	}

	return exec_write(exec_code[name]);
    }
    /* Execute the command line. */
    else if (stringp(str))
    {
        if (query_verb() == "execr")
        {
            str = "return " + str;
        }
	return exec_write(str);
    }

    /* Clone an editor to enter the code to exec. */
    clone_object(EDITOR_OBJECT)->edit(EXEC_DONE);
    return 1;
}

/* **************************************************************************
 * pad - Write notes on your notepad.
 */

/*
 * The command "pad" uses disk cashing. For each notepad, a mapping is
 * maintained with the following outline:
 *
 * ([
 *    (int) number : (string) note-text
 * ])
 *
 * Because it is impossible to save a mapping with save_map that contains
 * integer indices, the actual mapping is stored as:
 *
 * ([
 *    "pad": (mapping) the actual mapping
 * ])
 */

#define PAD_FILE ("/.notepad")
#define PAD_MAX  (20)
#define PAD_PAD  ("pad")

/*
 * Function name: read_pad_file
 * Description  : This function reads the pad-file of a particular wizard.
 *                If such a file does not exist, the mapping is cleared.
 */
private void
read_pad_file()
{
    pad_notes = read_cache(SECURITY->query_wiz_path(geteuid()) + PAD_FILE);
    pad_notes = pad_notes[PAD_PAD];

    /* No such file, set to defaults. */
    if (!m_sizeof(pad_notes))
    {
	pad_notes = ([ ]);
    }
}

/*
 * Function name: save_pad_file
 * Description  : This function saves the pad-file of a particular wizard.
 *                When empty, the save-file is deleted.
 */
private void
save_pad_file()
{
    /* No notes left, remove the file completely. */
    if (!m_sizeof(pad_notes))
    {
	/* We do not need to appent the .o as that is done in rm_cache(). */
	rm_cache(SECURITY->query_wiz_path(geteuid()) + PAD_FILE);
	return;
    }

    save_cache( ([ PAD_PAD : pad_notes ]),
	(SECURITY->query_wiz_path(geteuid()) + PAD_FILE));
    pad_notes = 0;
}

/*
 * Function name: pad_write
 * Description  : This function actually adds the note to the pad.
 * Arguments    : string str - the text to write on the note.
 */
private nomask void
pad_write(string str)
{
    int index = 0;
    
    if (!strlen(str))
    {
    	write("No note written on your notepad.\n");
    	return;
    }

    read_pad_file();

    /* Find the first 'free page' on the notepad. */
    while(++index)
    {
    	if (!pad_notes[index])
    	{
    	    break;
    	}
    }

    /* Give a warning, but don't do anything yet. */
    if (index > PAD_MAX)
    {
        write("Too many notes on your notepad. Please clean up a little!\n");
    }

    pad_notes[index] = str;
    write("Note added to your notepad with number '" + index + "'.\n");
    save_pad_file();
}

/*
 * Function name: pad_done_editing_reloaded
 * Description  : When the euid of this object has been set to the euid
 *                of the wizard again, we can keep the lines.
 */
public nomask void
pad_done_editing_reloaded()
{
    if ((previous_object() != this_interactive()) ||
	(calling_function() != REOPEN_SOUL))
    {
	write("Illegal call to pad_done_editing_reloaded().\n");
	return;
    }

    pad_write(this_interactive()->query_prop(WIZARD_S_PAD_MESSAGE));
}

/*
 * Function name: pad_done_editing
 * Description  : When the wizard is done editing the pad-text, this
 *                function is called with the text. We call the function
 *                reload_soul in the player to get an euid into the soul
 *                and then clip the text to the notepad.
 * Arguments    : string str - the 
 */
public nomask void
pad_done_editing(string str)
{
    if (MASTER_OB(previous_object()) != EDITOR_OBJECT)
    {
	write("Illegal call to pad_done_editing().\n");
	return;
    }

    this_interactive()->add_prop(WIZARD_S_PAD_MESSAGE, str);
    call_other(this_interactive(), REOPEN_SOUL);
}

nomask int
pad(string str)
{
    string *args;
    int *indices;
    int size;
    int index;
    int width;

    if (!strlen(str))
    {
    	str = "r";
    }

    args = explode(str, " ");
    size = sizeof(args);
    switch(args[0])
    {
    case "d":
        if (size != 2)
        {
            notify_fail("Syntax for 'pad d': pad d <number> or pad d all\n");
            return 0;
        }

        if (args[1] == "all")
        {
            pad_notes = 0;
            save_pad_file();
            write("Deleted all notes from your notepad.\n");
            return 1;
        }

	read_pad_file();
        index = atoi(args[1]);
        if (!pad_notes[index])
        {
	    notify_fail("No note '" + index + "' on your notepad.\n");
	    return 0;
        }

	write("You rip page '" + index + "' from your notepad.\n");
	pad_notes = m_delete(pad_notes, index);
	save_pad_file();
        return 1;
    
    case "r":
        if (size != 1)
        {
            notify_fail("Syntax: pad r\n");
            return 0;
        }

        read_pad_file();
        if (!m_sizeof(pad_notes))
        {
            write("No notes on your notepad.\n");
            return 1;
        }

        indices = sort_array(m_indices(pad_notes));
        index = -1;
        size = sizeof(indices);
        width = this_interactive()->query_option(OPT_SCREEN_WIDTH) - 15;
        while(++index < size)
        {
            args = explode(pad_notes[indices[index]], "\n");
            str = args[0];
            if (strlen(str) > width)
            {
                str = str[..width] + "...";
            }
            if (sizeof(args) > 1)
            {
            	str += "[+]";
            }

            write(sprintf("%3d: %s\n", indices[index], str));
        }

        return 1;

    case "w":
        if (size != 1)
        {
            notify_fail("Syntax: pad w\n");
            return 0;
        }
        
	/* Clone an editor to enter the note text. */
	clone_object(EDITOR_OBJECT)->edit(PAD_DONE);
	return 1;

    default:
        /* Wizard may read a particular note number on his/her notepad. */
	index = atoi(args[0]);
	if (index &&
	    (size == 1))
	{
	    read_pad_file();
	    if (!pad_notes[index])
	    {
		notify_fail("No note '" + index + "' on your notepad.\n");
		return 0;
	    }

	    write("Reading note '" + index + "' on your notepad:\n");
	    write(pad_notes[index] + "\n");
	    return 1;
	}

	/* Treat the text as new note. */
	pad_write(str);
	return 1;
    }

    write("Impossible end of switch() in pad command. Please report this!\n");
    return 1;
}

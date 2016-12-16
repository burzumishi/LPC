/*
 * /std/data_edit.c
 *
 * With this object, wizards can load a data-file, edit it and save it again.
 * Since it assumes the euid of the person who cloned it, there are some
 * provisions that guard against other people messing with it.
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <composite.h>
#include <filepath.h>
#include <macros.h>
#include <options.h>
#include <std.h>
#include <stdproperties.h>

#define DATA_EDIT_EXEC    "data_exec.c"
#define DATA_EDIT_PROMPT  "DataEdit> "
#define DATA_EDIT_VERSION "1.1"

/*
 * Global variables. They are not saved.
 *
 * filename - the name of the file being edited.
 * data     - the mapping keeping the contents of the data-file.
 */
static private string  filename = 0;
static private mapping data     = 0;

/*
 * Function name: create_object
 * Description  : Constructor. It is called to create this object.
 */
nomask public void
create_object()
{
    set_name("editor");
    set_pname("editors");
    add_name("edit");
    add_name("date_edit");
    set_adj("data");

    set_short("data editor");
    set_pshort("data editors");

    set_long(break_string("With this data editor it is possible to list " +
	"and manipulate LPC-datafiles. One command, data_edit, is linked to " +
	"this editor. There is a general help-page on the command. Also, " +
	"within the editor, you can get help by typing ? or h[elp]. The " +
	"syntax for data_edit is 'data_edit <filename>'.", 75) + "\n");

    remove_prop(OBJ_I_VALUE);
    remove_prop(OBJ_I_VOLUME);
    remove_prop(OBJ_I_WEIGHT);

    add_prop(OBJ_I_NO_STEAL,    1);
    add_prop(OBJ_I_NO_TELEPORT, 1);
    add_prop(OBJ_S_WIZINFO,
	"Just examine the data editor for information. /Mercade\n");
}

int data_edit(string str);

/*
 * Function name: init
 * Description  : When a wizard 'comes close' to this object, the commands
 *                of this object are linked to the player.
 */
public nomask void
init()
{
    ::init();

    add_action(data_edit,   "data_edit");
}

/*
 * Function name: data_edit
 * Description  : This function initializes the data-editing process.
 *                The command line should be the the filename of the file
 *                to edit.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
static nomask int
data_edit(string str)
{
    string name;

    if (this_player() != this_interactive())
    {
	notify_fail("Illegal interactive player. data_edit refused.\n");
	return 0;
    }

    if (environment() != this_interactive())
    {
	notify_fail("You must carry the data editor in order to use it.\n");
	return 0;
    }

    if (mappingp(data))
    {
	notify_fail("This data-editor is already in use.\n");
	return 0;
    }

    if (!strlen(str))
    {
        if (!strlen(filename))
        {
	    notify_fail("Syntax: data_edit [<filename>]\n" +
	        "No previous filename edited this session.\n");
	    return 0;
	}
	write("Attempting to re-open: " + filename + "\n");
	str = filename;
    }

    name = this_interactive()->query_real_name();
    if (SECURITY->query_wiz_rank(name) < WIZ_NORMAL)
    {
	notify_fail("You must be a full wizard to use the data editor.\n");
	return 0;
    }

    if (getuid() != name)
    {
	notify_fail("Only the owner of this object, ie the person who " +
	    "cloned it, may use it since it carries his/her euid.\n");
	return 0;
    }

    /* Expand the filename to the complete path and strip the possible
     * trailing extension ".o".
     */
    filename = FTPATH(this_interactive()->query_path(), str);
    sscanf(filename, "%s.o", filename);

    seteuid(getuid());

    /* If such a file exists, try to load it, else create a new file. */
    if (file_size(filename + ".o") > 0)
    {
	catch(data = restore_map(filename));
	if (!mappingp(data))
	{
	    notify_fail("Failed to restore file: " + filename + ".o\n");
	    return 0;
	}
    }
    else
    {
	write("No such file: " + filename + ".o\n");
	write("A new datafile will be created.\n");

	data = ([ ]);
    }

    write("Data editor version " + DATA_EDIT_VERSION + ".\n");
    write(DATA_EDIT_PROMPT);
    input_to("edit");

    return 1;
}

/*
 * Function name: data_add
 * Description  : This function allows you to add value to an array. The
 *                format is
 *                    <variable> <index> <expression>
 *                where <variable> is the array to assign the value to.
 *                It may contain the index to a mapping. The <index> is the
 *                index of the array before which the new element is added.
 *                To add something at the end of the array, use 'end' as
 *                index. The value <expression> may include all types or
 *                expressions as long as it does not contain variable-
 *                references.
 * Arguments    : string str - the argument to the command set.
 */
static nomask void
data_add(string str)
{
    string *words;
    string var;
    string expr;
    int    index;
    string indices;
    string add;
    string path;
    object obj;

    if (!strlen(str))
    {
	write("Syntax: a[dd] <variable> <index> <expression>\n");
	return;
    }

    words = explode(str, " ");

    if (sizeof(words) < 3)
    {
	write("Syntax: a[dd] <variable> <index> <expression>\n");
	return;
    }

    expr = implode(words[2..], " ");

    /* See if there are indices for a mapping or array involved. */
    if (sscanf(words[0], "%s[%s", var, indices) == 2)
    {
	indices = "[" + indices;
    }
    else
    {
	indices = "";
	var = words[0];
    }

    /* We need to have an array or mapping that contains an array in order
     * to be able to continue.
     */
    if (!mappingp(data) ||
	((!pointerp(data[var])) && (!mappingp(data[var]))))
    {
	write("No array or mapping in the datafile called '" + var + "'.\n");
	return;
    }

    /* Compute the desired index. */
    if (sscanf(words[1], "%d", index) != 1)
    {
	if (words[1] != "end")
	{
	    write("Invalid index '" + words[1] + "'. Must be a number or " +
		"the marker 'end' to add the element at the end of the " +
		"array.\n");
	    return;
	}

	index = -1;
    }

    seteuid(getuid());

    path = SECURITY->query_wiz_path(this_interactive()->query_real_name()) +
	"/" + DATA_EDIT_EXEC;

    if (file_size(path) > 0)
    {
	if (!rm(path))
	{
	    write("Failed to remove previous copy: " + path +
		"\nErgo it is impossible to add the element to the array.\n");
	    return;
	}
    }

    switch(index)
    {
    case -1:
	add = "data[\"" + var + "\"]" + indices + " + ({ " + expr + " })";
	break;

    case  0:
	add = "({ " + expr + " }) + data[\"" + var + "\"]" + indices;
	break;

    default:
	add = "data[\"" + var + "\"]" + indices + "[.." + (index - 1) +
	    "] + ({ " + expr + " }) + data[\"" + var + "\"]" + indices +
	    "[" + index + "..]"; 
	break;
    }

    if (!write_file(path,
	"#pragma no_clone\n\nvoid\ndata_exec(mapping data)\n{\n    data[\"" +
	var + "\"]" + indices + " = " + add + ";\n\n    destruct();\n}\n"))
    {
	write("Failed to write file: " + path + "\nImpossible to execute.\n");
	return;
    }

    /* Update the object if necessary. */
    if (objectp(obj = find_object(path)))
	SECURITY->do_debug("destroy", obj);

    if (str = catch(call_other(path, "teleledningsanka")))
    {
	write("Could not load: " + path +
	    "\nImpossible to add element '" + expr + "' " +
	    ((index == -1) ? "at the end" :
		("before element '" + index + "'")) +
	    " of array '" + var + indices + "'.\n");
	return;
    }

    /* Since runtime errors are not caught by catch(), we must do the
     * input_to() before the actual execution since we want to continue
     * editing even if there is an error.
     */
    input_to("edit");

    if (str = catch(call_other(path, "data_exec", data)))
    {
	write("Error in execution of adding element '" + expr + "' " +
	    ((index == -1) ? "at the end" :
		("before element '" + index + "'")) +
	    " of array '" + var + indices + "'.\n");
	write("Error: " + str);
	return;
    }

    write("Added element '" + expr + "' " +
	((index == -1) ? "at the end" :
	    ("before element '" + index + "'")) +
	" of array '" + var + indices + "'.\n");

    if (!rm(path))
	write("Error removing '" + path + "' after execution.\n");

    return;
}

/*
 * Function name: data_done
 * Description  : When the wizard is done editing the datafile, this function
 *                writes the information back to disk.
 * Returns      : int 1/0 - success/failure
 */
static nomask int
data_done()
{
    seteuid(getuid());

    /* Check if there already is a file with the intended name. */
    if (file_size(filename + ".o") >= 0)
    {
	/* Check if there already is a file with the name of the backup. */
	if (file_size(filename + ".o.old") >= 0)
	{
	    /* Try to remove the old backup. */
	    if (!rm(filename + ".o.old"))
	    {
		write("Failed to remove the old backup: " + filename +
		    ".o.old\n");
		write("Use q[uit] to quit without saving.\n");
		return 0;
	    }
	}

	/* Rename the old file to the backup. */
	if (!rename((filename + ".o"), (filename + ".o.old")))
	{
	    write("Failed to make backup from " + filename +
		".o to its .o.old counterpart.\n");
	    write("Use q[uit] to quit without saving.\n");
	    return 0;
	}

	write("Backup made to " + filename + ".o.old\n");
    }

    /* Save the datafile back to disk. */
    save_map(data, filename);
    write("Datafile saved to " + filename + ".o\n");
    return 1;
}

/*
 * Function name: element_type
 * Description  : Get the type of a certain variable.
 * Arguments    : mixed - the variable to test.
 * Returns      : string - a description of the type.
 */
static nomask string
element_type(mixed element)
{
    if (intp(element))
	return "integer";

    if (stringp(element))
	return "string";

    if (floatp(element))
	return "float";

    if (pointerp(element))
	return sprintf("array   (%2d)", sizeof(element));

    if (mappingp(element))
	return sprintf("mapping (%2d)", m_sizeof(element));

    return "unknown";
}

/*
 * Function name: data_list
 * Description  : This function is used to list the contents of the datafile.
 *                The following arguments are allowed:
 *                <none>   - list all variables with their type.
 *                <list>   - display the contents of all variables in <list>.
 *                *        - display the contents of all variables.
 * Arguments    : string str - the argument to the command list.
 */
static nomask void
data_list(string str)
{
    string *vars;
    string *data_vars;
    int    index;
    int    size;

    if (!(size = m_sizeof(data)))
    {
	write("Currently no variables in the datafile.\n");
	return;
    }

    /* No arguments indicates the wizard only wants to see which variables
     * the datafile has.
     */
    data_vars = m_indices(data);
    if (!strlen(str))
    {
	index = -1;
	size = sizeof(data_vars);
	data_vars = sort_array(data_vars);
	while(++index < size)
	{
	    data_vars[index] = sprintf("%-16s %-1s", data_vars[index],
		element_type(data[data_vars[index]]));
	}

	index = this_interactive()->query_option(OPT_SCREEN_WIDTH);
	if (index ==  0)
	    index = 80;

	write("The datafile contains " + size + " variables:\n" +
	    sprintf("%-*#s\n", (index - 4), implode(data_vars, "\n")));
	return;
    }

    /* 'list *' lists all variables. */
    if (str == "*")
	vars = data_vars;
    else
    {
	str = implode(explode(str, ","), " ");
	vars = explode(str, " ") - ({ "" });

	if (sizeof(vars - data_vars))
	{
	    write("Not in the datafile as variable: " +
		COMPOSITE_WORDS(vars - data_vars) + ".\n");
	    return;
	}
    }

    size = sizeof(vars);
    index = -1;
    while(++index < size)
    {
	write(sprintf("%2d: %-20s ", (index + 1), vars[index]));
	dump_array(data[vars[index]]);
	write("\n");
    }

    return;
}

/*
 * Function name: data_remove
 * Description  : This function allows you to remove variables from the
 *                datafile, or elements from an array or mapping in the
 *                datafile. The following arguments are valid:
 *                <var>         - remove variable <var> from the datafile.
 *                <var> <index> - remove the element <index> from the array
 *                                or mapping <var>. For an array, it is the
 *                                index-number of the element and for a
 *                                mapping, it is the index-name.
 * Arguments    : string str - the argument to the command remove.
 */
static nomask void
data_remove(string str)
{
    string *words;
    string var;
    string element;
    int    index;

    if (!strlen(str))
    {
	write("Syntax: r[emove] <variable> [<index>]\n");
	return;
    }

    words = explode(str, " ");
    var = words[0];

    /* Check whether the desired variable is in the datafile. */
    if (member_array(var, m_indices(data)) == -1)
    {
	write("No variable '" + var + "' in the datafile.\n");
	return;
    }

    /* Only one argument means remove that variable. */
    if (sizeof(words) == 1)
    {
	m_delkey(data, var);

	write("Variable '" + var + "' removed from the datafile.\n");
	return;
    }

    element = words[1];
    if (pointerp(data[var]))
    {
	/* Remove an element from an array, so we need to have a numerical
	 * index.
	 */
	if (sscanf(element, "%d", index) != 1)
	{
	    write("Cannot remove element '" + element + "' from array '" +
		var + "'.\n");
	    return;
	}

	/* Check whether the index is within the bounds. */
	if (index >= sizeof(data[var]))
	{
	    write("Array '" + var + "' contains only " + sizeof(data[var]) +
		" elements. Cannot remove element #" + index + ".\n");
	    return;
	}

	/* Remove the element from the array. */
	data[var] = exclude_array(data[var], index, index);
	write("Removed element #" + index + " from array '" + var + "'.\n");
	return;
    }

    if (mappingp(data[var]))
    {
	/* Check whether the element with the required index exists. */
	if (member_array(element, m_indices(data[var])) == -1)
	{
	    write("The mapping '" + var + "' contains no element '" +
		element + "'.\n");
	    return;
	}

	m_delkey(data[var], element);
	write("Element '" + element + "' removed from mapping '" + var +
	    "'.\n");
	return;
    }

    write("Variable '" + var + "' is neither an array nor a mapping. " +
	"Cannot remove element '" + element + "' from it.\n");
    return;
}

/*
 * Function name: data_set
 * Description  : This function allows you to assign a variable to a certain
 *                value. The format is
 *                    <variable> <expression>
 *                where <variable> is the variable to assign the value to.
 *                It may contain the index to a mapping. The value <expression>
 *                may include all types or expressions as long as it does not
 *                contain variable-references.
 * Arguments    : string str - the argument to the command set.
 */
static nomask void
data_set(string str)
{
    string *words;
    string var;
    string expr;
    string indices;
    string path;
    object obj;

    if (!strlen(str))
    {
	write("Syntax: s[et] <variable> <expression>\n");
	return;
    }

    words = explode(str, " ");
    expr = implode(words[1..], " ");

    /* See if there are indices for a mapping or array involved. */
    if (sscanf(words[0], "%s[%s", var, indices) == 2)
    {
	indices = "[" + indices;
    }
    else
    {
	indices = "";
	var = words[0];
    }

    seteuid(getuid());

    path = SECURITY->query_wiz_path(this_interactive()->query_real_name()) +
	"/" + DATA_EDIT_EXEC;

    if (file_size(path) > 0)
    {
	if (!rm(path))
	{
	    write("Failed to remove previous copy of " + path +
		"\nErgo it is impossible to set the variable.\n");
	    return;
	}
    }

    /* If the mapping is not a mapping for some reason, make it one. */
    if (!mappingp(data))
	data = ([ ]);

    if (!write_file(path,
	"#pragma no_clone\n\nvoid\ndata_exec(mapping data)\n{\n    data[\"" +
	var + "\"]" + indices + " = " + expr + ";\n\n    destruct();\n}\n"))
    {
	write("Failed to write " + path + "\nImpossible to set variable.\n");
	return;
    }

    if (objectp(obj = find_object(path)))
	SECURITY->do_debug("destroy", obj);

    if (str = catch(call_other(path, "teleledningsanka")))
    {
	write("Could not load: " + path +
	    "\nProbably due to an error in the set expression:\n    " + var +
	    indices + " = " + expr + ";\n");
	return;
    }

    /* We have to add this input_to() here since a runtime error is not
     * caught by the catch() statment and we do want to continue editing.
     */
    input_to("edit");

    if (str = catch(call_other(path, "data_exec", data)))
    {
	write("Error in set expression: " + var + indices + " = " + expr +
	    ";\n");
	write("Error: " + str);
	return;
    }

    write("Executed: " + var + indices + " = " + expr + ";\n");

    if (!rm(path))
	write("Error removing: " + path + " after execution.\n");

    return;
}

/*
 * Function name: edit
 * Description  : This function is called with the input from the player.
 *                It is the actual editor.
 * Arugments    : string str - the input from the wizard.
 */
static nomask void
edit(string str)
{
    string *words;

    if (!strlen(str))
    {
	write(DATA_EDIT_PROMPT);
	input_to("edit");
    }

    words = explode(str, " ");
    str = implode(words[1..], " ");

    switch(words[0])
    {
    case "a":
    case "add":
	data_add(str);
	break;

    case "d":
    case "done":
    case "**":
	if (data_done())
	{
	    data = 0;

	    return;
	}

	break;

    case "h":
    case "help":
    case "?":
	write(
"Data editor commands:\n\n" +
"!<command>       Execute command <command> and return to the editor.\n" +
"a[dd] <var> <idx> <expr> Add a value to an array before index <idx> To\n" +
"                         add at the end of the array use index 'end'.\n" +
"d[one] or **     Save the data-file and leave the data editor.\n" +
"h[elp] or ?      Display this help text.\n" +
"l[ist] [<vars>]  List all variables in the datafile by name and type or\n" +
"                 show the contents of variable(s) <vars>. '*' lists all.\n" +
"n[ame] [<path>]  Set datafile-filename to <path> or show current path.\n" +
"q[uit]           Quit editing without saving the datafile.\n" +
"r[emove] <var>   Remove variable <var> from the datafile.\n" +
"r[emove] <var> <index>   Remove element with index <index> from an array\n" +
"                         or mapping.\n" +
"s[et] <var> <expr>       Assign variable <var> to <expr>. Here <expr> may\n" +
"                         be of any type or expression as long as it does\n" +
"                         not contain variable references. The variable\n" +
"                         <var> may also include mapping/array indices.\n");
	break;

    case "l":
    case "list":
	data_list(str);
	break;

    case "n":
    case "name":
	if (!strlen(str))
	{
	    write("Current filename: " + filename + ".o\n");
	    break;
	}

	write("Old filename: " + filename + ".o\n");

	filename = FTPATH(this_interactive()->query_path(), str);
	sscanf(filename, "%s.o", filename);

	write("New filename: " + filename + ".o\n");
	break;

    case "q":
    case "quit":
	data = 0;

	write("Aborted editing. Nothing saved.\n");
	return;

    case "r":
    case "remove":
	data_remove(str);
	break;

    case "s":
    case "set":
	data_set(str);
	break;

    default:
	write("Invalid command. Use ? or h[elp] for help.\n");
	break;
    }

    write(DATA_EDIT_PROMPT);
    input_to("edit");
}

/*
 * Function name: query_prevent_shadow
 * Description  : This function prevents all other objects from shadowing
 *                this object.
 * Returns      : int 1 - always.
 */
public nomask int
query_prevent_shadow()
{
    return 1;
}

/*
 * Function name: query_auto_load
 * Description  : Returns the path to the master of this module. It will
 *                only autoload to wizards.
 * Returns      : string - the path.
 */
nomask string
query_auto_load()
{
    if (environment()->query_wiz_level())
         return MASTER;

    return 0;
}


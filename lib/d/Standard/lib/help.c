/* help.c created by Shiva@Standard
 *
 * Simple management of help files.
 *
 * Adds a help command using the syntax
 *
 *     "help <help type> <subject>"
 *
 * Allows simple addition of new help subjects by creating help files
 * in a specified directory with a name that corresponds to the subject
 * and a special extension that designates it as a help file (".help" by
 * default).
 * 
 *
 * Example usage:
 * 
 * inherit "/cmd/std/command_driver";
 * inherit "/d/Wiz/shiva/open/help";
 *
 * void
 * create()
 * {
 *     // The type of help we're giving.
 *     set_help_type(({ "shiva", "siva" }));
 *
 *     // Find the help files in my help directory
 *     read_help_dir("/d/Wiz/shiva/help/");
 * }
 *
 * // standard soul stuff
 * int
 * query_cmd_soul()
 * {
 *     return 1;
 * }
 *
 * // standard soul stuff
 * string
 * get_soul_id()
 * {
 *     return "My soul";
 * }
 *
 * // Gives the cmdlist mapping
 * mapping
 * query_cmdlist()
 * {
 *     // Return our command mapping plus the help command mapping 
 *     return ([ "dummy" : "dummy" ]) + help_cmdlist();
 * }
 *
 * // A dummy command; it does nothing.
 * int
 * dummy(string str)
 * {
 *     return 0;
 * }
 *
 * Given the soul above, one could execute the command
 *
 *     "help shiva xxx"
 *
 * and the contents of /d/Wiz/shiva/help/xxx.help would be displayed.
 *
 * Simply executing
 *
 *     "help shiva"
 *
 * gives a listing of possible help subjects.
 */

static string *help_type = ({});
static mapping help_map = ([]);

/*
 * Function name: help_commands
 * Description:   Returns the filename passed to it, minus the extension if
 *                the extension matches the specified help file extension.
 * Arguments:     string file - a filename
 * Returns:       mixed - the filename minus it's extension or 0 if the
 *                         extension doesn't match the help file extension.
 */
mixed
help_commands(string file, string ext)
{
    string name;
    if (file[(strlen(file) - strlen(ext) - 1)..] != ("." + ext))
    {
        return 0;
    }

    return file[..(strlen(file) - strlen(ext) - 2)];
}

/*
 * Function name: query_subjects
 * Description:   get the subject names currently available
 * Returns:       an array of subject names
 */
string *
query_subjects()
{
    return m_indices(help_map);
}

string
get_text(mixed val)
{
    int size;

    if (stringp(val))
    {    
        if ((size = file_size(val)) == -1) // file does not exist
	{
            return val;
	}

        if (size < 1) // empty file or directory
	{
            return "";
	}

        return read_file(val);
    }

    if (functionp(val))
    {
        return val();
    }

    return "";
}

/*
 * Function name: get_help
 * Description: get the help data for a given subject
 * Arguments:   string subject - the subject name
 * Returns:     The help data or "" if none could be found
 */
string
get_help(string subject)
{
    mixed val = help_map[subject], help;

    if (!pointerp(val))
    {    
        return "";
    }

    val = val[0];

    return get_text(val);
}

string
get_summary(string subject)
{
    mixed val = help_map[subject];

    if (!pointerp(val))
    {
        return 0;
    }

    val = val[1];

    return get_text(val);
}
        
string
format_subjects(string *subjects)
{
    return sprintf("%-#60s\n", implode(subjects, "\n"));
}

string
format_subjects_and_summaries(string *subjects, string *summaries)
{
    string str = "";
    int i;

    for (i = 0; i < sizeof(subjects); i++)
    {        
        str += sprintf("- %-15s   %-=40s\n", subjects[i], summaries[i]);
    }

    return str;
}

void
show_help_subjects()
{
    string *subjects, *summaries;

    if (!sizeof(subjects = query_subjects()))
    {
        return 0;
    }

    summaries = map(subjects, get_summary);

    write("Help is available on these subjects:\n\n");

    if (!sizeof(summaries - ({ "" })))
    {
        write(format_subjects(subjects));
    }
    else
    {
        write(format_subjects_and_summaries(subjects, summaries));
    }

    write("\nUse 'help " + help_type[0] + " <subject>' to access help files.\n\n");
}

/*
 * Function name: _help
 * Description:   The "help" command.  Give a listing of help subjects or
 *                print the contents of a help file.
 * Arguments:     string str - arguments given to the "help" command.
 * Returns:       1 / 0 - success / failure
 */
int
_help(string str)
{
    string help;
    int i;

    setuid();
    seteuid(getuid());
  
    if (member_array(str, help_type) >= 0)
    {
        show_help_subjects();
        return 1;
    }

    if (!strlen(str))
    {
        return 0;
    }

    for (i = 0; i < sizeof(help_type); i++)
    {
        if (sscanf(str, help_type[i] + " %s", help))
	{
            if (strlen(help = get_help(help)))
	    {
        	write("\n");
        	this_player()->more(help);
            
        	return 1;
            }
  
            return 0;
	}
    }
  
    return 0;
}

/*
 * Function name: read_help_dir
 * Description:   Check the specified directory to find help files
 * Arguments:     string dir         - the directory
 *                string ext         - the extension used to identify help
 *                                     files in the specified directory.
 *                string summary_ext - the extentsion used to identify
 *                                     summary files in the specified
 *                                     directory (if any)
 */
varargs void
read_help_dir(string dir, string ext = "help", string summary_ext = "summary")
{
    string *files, help_cmd, summary_file;
    int i;
    
    setuid();
    seteuid(getuid());

    files = get_dir(dir);

    for (i = 0; i < sizeof(files); i++)
    {
        if (help_cmd = help_commands(files[i], ext))
	{        
  	    help_map[help_cmd] = ({ dir + files[i], 0 });

            if (summary_ext &&
                (file_size(summary_file = dir + help_cmd + "." + 
                summary_ext) > 0))
	    {
                help_map[help_cmd][1] = summary_file;
	    }
	}
    }
}

/*
 * Function name: add_help
 * Description:   Add a help subject.
 * Arguments:     string subject - the subject name
 *                mixed src     - A filename if the data should be read from
 *                                disk, a string to be used as the data, or a
 *                                function pointer to a function which will
 *                                return the data.
 *                mixed summary - A filename if the summary should be read from
 *                                disk, a string to be used as the data, or a
 *                                function pointer to a function which will
 *                                return the data.*
 */
varargs void
add_help(string subject, mixed src, mixed summary)
{
    help_map[subject] = ({ src, summary });
}

/*
 * Function name: set_help_type
 * Description:   Specify the type of help we're giving.  This will be
 *                used to determine correct syntax for the "help" command.
 * Arguments:     string *type - an array of strings.  Any of these strings
 *                               may be used as the second argument to the
 *                               "help" command.
 */
void
set_help_type(string *type)
{
    help_type = type;
}

/*
 * Function name: help_cmdlist
 * Description:   Returns a mapping that maps help command names to help
 *                function names
 * Returns:       See above
 */
mapping
help_cmdlist() 
{
    return ([ "help" : "_help" ]);
}

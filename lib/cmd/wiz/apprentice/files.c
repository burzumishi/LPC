/*
 * /cmd/wiz/apprentice/files.c
 *
 * This is a sub-part of /cmd/wiz/apprentice.c
 *
 * This file contains the commands that allow wizard to handle files and
 * directories.
 *
 * Commands currently included:
 * - cat
 * - cd
 * - dirs
 * - head
 * - ls
 * - more
 * - popd
 * - pushd
 * - pwd
 * - tail
 * - tree
 */

#include <filepath.h>
#include <stdproperties.h>
#include <options.h>

/* These properties are used by this object only. */
#define WIZARD_AS_DIRPATH   "_wizard_as_dirpath"
#define WIZARD_S_LAST_DIR   "_wizard_s_last_dir"
#define WIZARD_S_TAIL_PATH  "_wizard_s_tail_path"
#define WIZARD_I_TAIL_LIMIT "_wizard_i_tail_limit"

#define MAX_TREE_SIZE   ( 60)
#define TAIL_READ_CHUNK (800)

#define REOPEN_SOUL	("reopen_soul")

#define SPACES ("                              ")

/* **************************************************************************
 * cat - display the contents of a file
 */
int
cat_file(string path)
{
    CHECK_SO_WIZ;

    if (!path)
    {
	notify_fail("What file do you want to cat?\n");
	return 0;
    }

    path = FTPATH((string)this_interactive()->query_path(), path);
    if (!stringp(path))
    {
	notify_fail("Bad file name format.\n");
	return 0;
    }

    if (!(SECURITY->valid_read(path, geteuid(), "cat")))
    {
	notify_fail("You have no read access to: " + path + "\n");
	return 0;
    }

    if (file_size(path) <= 0)
    {
	notify_fail("No such file: " + path + "\n");
	return 0;
    }

    cat(path);
    write("EOF\n");
    return 1;
}

/* **************************************************************************
 * cd - change current directory
 */
int
cd(string str)
{
    string old_path;
    string new_path;
    string *parts;
    object ob;
    int    auto;

    auto = this_player()->query_option(OPT_AUTO_PWD);

    CHECK_SO_WIZ;

    old_path = this_interactive()->query_path();

    if (!stringp(str))
    {
	new_path = SECURITY->query_wiz_path(this_player()->query_real_name());
    }
    else switch(str)
    {
    case ".":
	new_path = old_path;
	if (auto)
	    write("CWD: " + new_path + "\n");
	else
	    write("Ok.\n");
	return 1;

    case "-":
	if (!(new_path = this_player()->query_prop(WIZARD_S_LAST_DIR)))
	{
	    new_path = SECURITY->query_wiz_path(
			this_player()->query_real_name());
	}
	break;

    default:
	new_path = FTPATH(old_path, str);
	break;
    }

    if (file_size(new_path) != -2)
    {
	if (!objectp(ob = parse_list(str)))
	{
	    notify_fail("No such directory '" + str + "'.\n");
	    return 0;
	}

	parts = explode(file_name(ob), "/");
	new_path = implode(parts[0..(sizeof(parts) - 2)], "/");
    }

    if (auto)
	write("CWD: " + new_path + "\n");
    else
	write("Ok.\n");
    this_player()->add_prop(WIZARD_S_LAST_DIR, old_path);
    this_player()->set_path(new_path);

    return 1;
}

/* **************************************************************************
 * dirs - display your dirs list.
 */
varargs int
dirs(string str)
{
    string	*paths;
    int		size;

    CHECK_SO_WIZ;

    if (stringp(str))
    {
	notify_fail("There should not be an argument to 'dirs'.\n");
	return 0;
    }

    paths = this_player()->query_prop(WIZARD_AS_DIRPATH);
    size = sizeof(paths);
    write(this_player()->query_path());

    if (size)
	write(" " + implode(paths, " ") + "\n");
    else
	write("\n");
    return 1;
}

/* **************************************************************************
 * head - display the header of a file
 */
int
head(string path)
{
    int lines = 10;
    int size;
    string text;

    CHECK_SO_WIZ;

    if (!strlen(path))
    {
	notify_fail("Syntax: head [lines] <filename>\n");
	return 0;
    }

    /* User may want to display a different number of lines. */
    sscanf(path, "%d %s", lines, path);
    if (lines <= 0)
    {
        notify_fail("Number of lines (" + lines + ") must be positive.\n");
        return 0;
    }

    path = FTPATH(this_interactive()->query_path(), path);
    if (!strlen(path))
    {
	notify_fail("Bad file name format.\n");
	return 0;
    }

    if (!(SECURITY->valid_read(path, geteuid(), "head")))
    {
	notify_fail("You have no read access to: " + path + "\n");
	return 0;
    }

    size = file_size(path);
    if (size <= 0)
    {
	notify_fail("No such file: " + path + "\n");
	return 0;
    }

    text = read_file(path, 1, lines);
    if (!strlen(text))
    {
        notify_fail("Failed to read " + lines + " lines from: " + path + "\n");
        return 0;
    }

    write(text);
    if (size == strlen(text))
    {
        write("EOF\n");
    }
    return 1;
}

/* **************************************************************************
 * ls - list the files in a directory
 */
int
ls_sort_t(mixed *item1, mixed *item2)
{
    return item2[1] - item1[1];
}

int
list_files(string path)
{
    int     i;
    int     j;
    int     ml, mf;
    int     size, len;
    mixed   tmp;
    string  mode;
    string  prefix;
    string *files;
    string *items;
    int     scrw = this_player()->query_option(OPT_SCREEN_WIDTH);

    CHECK_SO_WIZ;

    scrw = ((scrw >= 40) ? (scrw - 1) : 79);
    mode = "";

    if (!stringp(path))
    {
	path = ".";
    }
    else
    {
	tmp  = explode(path, " ");
	j    = sizeof(tmp);
	i    = -1;
	path = "";

	while (++i < j)
	{
	    if (tmp[i][0] == '-')
	    {
		mode += tmp[i][1..];
	    }
	    else
	    {
		path  = tmp[i];
	    }
	}

	if (!strlen(path))
	{
	    path = ".";
	}
    }

    path = FTPATH((string)this_interactive()->query_path() + "/", path);

    /* List a single file. */
    if (file_size(path) > 0)
    {
        files = explode(path, "/")[-1..];
    }
    /* See if there is no asterisk in the path. */
    else if (!wildmatch("*\\**", path))
    {
        /* Add a trailing / if necessary. */
	if (!wildmatch("*/", path))
	{
	    path += "/";
	}
	else
	{
	    path = "/";
	}
    }

    if (!sizeof(files))
    {
        if (!sizeof(files = get_dir(path)))
        {
            notify_fail("No files in: " + path + "\n");
            return 0;
        }
    }

    files -= ({ ".", ".." });
    size   = sizeof(files);
    items  = ({});
    path   = implode(explode(path + "/", "/")[..-2], "/") + "/";

    /* Do we dump files with a leading period? */
    if (!wildmatch("*a*", mode))
    {
	files = filter(files, &not() @ &wildmatch(".*", ));
	size  = sizeof(files);
    }

    /* Do we do sorting on time/date? Note, order is: recent-oldest */
    if (wildmatch("*t*", mode))
    {
	i   = -1;
	tmp = ({});

	while (++i < size)
	{
	    tmp += ({ ({ files[i], file_time(path + files[i]) }) });
	}

	tmp   = sort_array(tmp, "ls_sort_t", this_object());
	files = ({});
	i     = -1;

	while (++i < size)
	{
	    files += ({ tmp[i][0] });
	}
    }

    /* Directories on top. Tintin finds 'O' more logical for that than 'd'. */
    if (wildmatch("*O*", mode))
    {
        tmp = filter(files, &operator(==)(-2, ) @ &file_size() @ &operator(+)(path, ));
        files = tmp + (files - (string *)tmp);
    }

    /* Do we do a long display? */
    if (wildmatch("*l*", mode))
    {
        mf    = !wildmatch("*f*", mode);
        ml    = 1;
	items = files + ({});
	files = ({});
	/* Allow for long file names, but at least 20 chars. */
        int len = max(20, min((scrw-32), applyv(max, map(items, strlen))) + 1);

	foreach(string fname: items)
	{
	    if ((j = file_size(path + fname)) == -2)
	    {
		if (mf)
		{
		    fname += "/";
		}
		prefix =  "d";
		j      = 0;
	    }
	    else if (find_object(path + fname))
	    {
		prefix =  "*";
	    }
	    else
	    {
		prefix =  "-";
	    }

	    tmp = ctime(file_time(path + fname));
	    tmp = tmp[4..9] + tmp[19..23] + tmp[10..15];

	    files += ({ sprintf("%1s %-*s%10d  %s", prefix, len, fname, j, tmp) });
	}
    }

    /* Distinguish dirs and loaded files?
     * Used to be a -F check, but I reversed that to a negative -f check.
     */
    if (!ml &&
        !wildmatch("*f*", mode))
    {
	i = -1;
	while (++i < size)
	{
	    if (file_size(path + files[i]) == -2)
	    {
		files[i] += "/";
	    }
	    else if (objectp(find_object(path + files[i])))
	    {
		files[i] += "*";
	    }
	}
    }

    /* Do we reverse sort? */
    if (wildmatch("*r*", mode))
    {
	items = files + ({});
	i     = sizeof(items);
	files = ({});

	while (i--)
	{
	    files += ({ items[i] });
	}
    }

    if (ml)
    {
	/* If long option chosen, display one line at a time. */
	i = -1;
	while (++i < size)
	{
	    write(files[i] + "\n");
	}
    }
    else
    {
	tmp = sprintf("%-*#s\n", scrw, implode(files, "\n"));
	if (strlen(tmp) > 4000)
	{
	    this_player()->more(tmp);
	}
	else
	{
	    write(tmp + "\n");
	}
    }

    return 1;
}

/* **************************************************************************
 * more - display the contents of a file
 */
int
more_file(string path)
{
    CHECK_SO_WIZ;

    if (!stringp(path))
    {
	notify_fail("More what file?\n");
	return 0;
    }

    path = FTPATH((string)this_interactive()->query_path(), path);
    if (!strlen(path))
    {
	notify_fail("Bad file name format.\n");
	return 0;
    }

    if (!(SECURITY->valid_read(path, geteuid(), "cat")))
    {
	notify_fail("You have no read rights to: " + path + "\n");
	return 0;
    }

    if (file_size(path) <= 0)
    {
	notify_fail("No such file: " + path + "\n");
	return 0;
    }

    this_player()->more(path, 1);
    return 1;
}

/* **************************************************************************
 * popd - pop a directory from the stack.
 */
int
popd(string str)
{
    string	*paths;
    int		which;

    CHECK_SO_WIZ;

    if (stringp(str) &&	(sscanf(str, "+%d", which) != 1))
    {
	notify_fail("Syntax: popd [+<number>]\n");
	return 0;
    }
    else
	which = 1;

    paths = this_player()->query_prop(WIZARD_AS_DIRPATH);
    if (!sizeof(paths))
    {
	notify_fail("Directory stack empty.\n");
	return 0;
    }

    which -= 1;
    if (which >= sizeof(paths))
    {
	notify_fail("Directory stack not that deep.\n");
	return 0;
    }

    this_player()->add_prop(WIZARD_S_LAST_DIR, this_player()->query_path());
    this_player()->set_path(paths[which]);
    paths = exclude_array(paths, which, which);

    if (sizeof(paths))
	this_player()->add_prop(WIZARD_AS_DIRPATH, paths);
    else
	this_player()->remove_prop(WIZARD_AS_DIRPATH);

    dirs();
    return 1;
}

/* **************************************************************************
 * pushd - push a directory to stack and 'cd' to a directory.
 */
int
pushd(string str)
{
    string	*paths;
    int		which = -1;

    CHECK_SO_WIZ;

    paths = this_player()->query_prop(WIZARD_AS_DIRPATH);
    paths = (sizeof(paths) ? paths : ({ }) );

    if (!strlen(str))
	str = "+1";

    if (sscanf(str, "+%d", which) != 0)
    {
	which -= 1;

	if (which < 0)
	{
	    notify_fail("Directory index < 1.\n");
	    return 0;
	}

	if (which >= sizeof(paths))
	{
	    notify_fail("Directory stack not that deep.\n");
	    return 0;
	}

	this_player()->add_prop(WIZARD_S_LAST_DIR,
	  this_player()->query_path());
	this_player()->set_path(paths[which]);
	paths[which] = this_player()->query_prop(WIZARD_S_LAST_DIR);
    }
    else
    {
	str = FTPATH(this_player()->query_path(), str);
	if (file_size(str) != -2)
	{
	    notify_fail("No such directory: " + str + "\n");
	    return 0;
	}

	paths = ({ this_player()->query_path() }) + paths;
	this_player()->set_path(str);
    }

    this_player()->add_prop(WIZARD_AS_DIRPATH, paths);
    dirs();
    return 1;
}

/* **************************************************************************
 * pwd - display the current working directory.
 */
int
pwd()
{
    string path;

    if (stringp((path = this_player()->query_prop(WIZARD_S_LAST_DIR))))
	write("LWD: " + path + "\n");
    write("CWD: " + this_player()->query_path() + "\n");

    return 1;
}

/* **************************************************************************
 * tail - display the end of a file.
 */

/*
 * Function name: tail_input_player
 * Description  : Input function for the "tail -r" command. We call the
 *                function reload_soul in the player to get an euid into
 *                the soul again and then display more text.
 * Arguments    : string str - the input-argument.
 */
nomask void
tail_input_player(string str)
{
    str = (strlen(str) ? lower_case(str) : "u");

    switch(str[0])
    {
    case 'q':
    case 'x':
	/* Clean up after ourselves. */
	this_interactive()->remove_prop(WIZARD_S_TAIL_PATH);
	this_interactive()->remove_prop(WIZARD_I_TAIL_LIMIT);
	return;

    case 'u':
	call_other(this_interactive(), REOPEN_SOUL);
	return;

    default:
	write("Invalid command. \"q/x\" to quit or RETURN to continue --- ");
	input_to(tail_input_player);
	return;
    }

    write("Impossible end of tail_input_player() in the apprentice soul.\n" +
      "Notify the administration.\n");
}

/*
 * Function name: tail_lines
 * Description  : This function will actually print a part of the file that
 *                the wizard wants to tail.
 */
private void
tail_lines()
{
    string path  = this_interactive()->query_prop(WIZARD_S_TAIL_PATH);
    int    size  = file_size(path);
    int    limit = this_interactive()->query_prop(WIZARD_I_TAIL_LIMIT);
    int    begin = limit - TAIL_READ_CHUNK;
    string text;
    string *lines;

    /* If we reach the begin of the file, stop. */
    if (begin <= 0)
    {
	text = read_bytes(path, 0, limit);
	write(text + "BOF\n");
	this_interactive()->remove_prop(WIZARD_S_TAIL_PATH);
	this_interactive()->remove_prop(WIZARD_I_TAIL_LIMIT);
	return;
    }

    text = read_bytes(path, begin, TAIL_READ_CHUNK);
    lines = explode(text, "\n");

    /* If there is at least one line, only print the complete lines. */
    if (sizeof(lines) > 1)
    {
	text = implode(lines[1..], "\n");
    }

    write(text + "\n");
    limit -= (strlen(text) + 1);
    write("TAIL " + limit + "/" + size + " (" + (100 * limit / size) + "%)" +
      " --- \"q/x\" to quit, RETURN to continue --- ");

    this_interactive()->add_prop(WIZARD_I_TAIL_LIMIT, limit);
    input_to(tail_input_player);
}

/*
 * Function name: tail_input_player_reloaded
 * Description  : When the euid of this object has been set to the euid
 *                of the wizard again, print more of the file.
 */
public nomask void
tail_input_player_reloaded()
{
    if ((previous_object() != this_interactive()) ||
      (calling_function() != REOPEN_SOUL))
    {
	write("Illegal call to tail_input_player_reloaded().\n");
	return;
    }

    tail_lines();
}

nomask int
tail_file(string path)
{
    int reverse;
    int size;

    CHECK_SO_WIZ;

    if (!stringp(path))
    {
	notify_fail("Syntax: tail [-r] <filename>\n");
	return 0;
    }

    reverse = sscanf(path, "-r %s", path);
    path = FTPATH(this_interactive()->query_path(), path);
    if (!strlen(path))
    {
	notify_fail("Bad file name format.\n");
	return 0;
    }

    if (!(SECURITY->valid_read(path, geteuid(), "tail")))
    {
	notify_fail("You have no read access to: " + path + "\n");
	return 0;
    }

    size = file_size(path);
    if (size <= 0)
    {
	notify_fail("No such file: " + path + "\n");
	return 0;
    }

    /* The normal tail command without the -r argument. */
    if (!reverse)
    {
	/* Tail the file the normal way. */
	if (tail(path))
	{
	    return 1;
	}

	notify_fail("No such file: " + path + "\n");
	return 0;
    }

    /* If the file is too small, print the whole file. */
    if (size <= TAIL_READ_CHUNK)
    {
	cat(path);
	return 1;
    }

    /* Add the relevant information to the player in properties. */
    this_interactive()->add_prop(WIZARD_S_TAIL_PATH, path);
    this_interactive()->add_prop(WIZARD_I_TAIL_LIMIT, size);
    tail_lines();    
    return 1;
}

/* **************************************************************************
 * tree - display the directory tree.
 */

/*
 * Function name: build_tree
 * Description  : This function prints a sub-part of the tree for a particular
 *                directory on said level.
 * Arguments    : string path - the directory to parse.
 *                int spaces  - the number of spaces preceiding this level.
 *                int printed - the number of directories printed.
 * Returns      : int - the total number of directories printed so far.
 */
int
build_tree(string path, int spaces, int printed)
{
    string *files;
    int    index = -1;
    int    size;
    int    this_level = 0;

    if (printed == -1)
    {
	return -1;
    }

    if (!pointerp(files = get_dir(path + "*")))
    {
	return printed;
    }

    files = map((files - ({ ".", ".." }) ), &operator(+)(path, ));
    files = filter(files, &operator(==)(-2) @ file_size);

    if (!(size = sizeof(files)))
    {
	return printed;
    }

    while(++index < size)
    {
	write(extract(SPACES, 1, spaces) + files[index] + "\n");

	if (++printed >= MAX_TREE_SIZE)
	{
	    write("Too large tree. Maximum of " + MAX_TREE_SIZE +
	      " reached. Interrupted...\n");
	    return -1;
	}

	printed = build_tree(files[index] + "/", (spaces + 3), printed);

	if (printed == -1)
	{
	    return -1;
	}
    }

    return printed;
}

int
tree(string str)
{
    CHECK_SO_WIZ;

    if (!stringp(str))
    {
	str = ".";
    }

    str = FTPATH(this_interactive()->query_path(), str);
    if (!strlen(str))
    {
	notify_fail("No such path found.\n");
	return 0;
    }

    if (file_size(str) != -2)
    {
	notify_fail("Not a directory: " + str + "\n");
	return 0;
    }

    write(str + "\n");
    build_tree(str + "/", 3, 1);
    return 1;
}

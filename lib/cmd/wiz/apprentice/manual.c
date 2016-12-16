/*
 * /cmd/wiz/apprentice/manual.c
 *
 * This is a sub-part of /cmd/wiz/apprentice.c
 *
 * This file contains the commands that gives you access to the manuals.
 *
 * Commands currently included:
 * - lman
 * - man
 * - sman
 */

#define MANHEAD  "/doc/man/"
#define SMANHEAD "/doc/sman"

/* Global variable. */
static int g_mannum;

/* Prototype. */
nomask string man_num() { return sprintf("\n#%d:", g_mannum++); }

nomask int
find_source(string docfile, string argv)
{
    string          docs,
                    str,
                    src;
    int             line;

    if (file_size(docfile) < 0)
	return 0;

    docs = read_file(docfile);
    if (sscanf(docs, "%d:%s %s", line, src, str) == 3)
    {
	write("File: " + src + " Line: " + line + "\n\n");
	if (argv == "-s")
	{
	    this_player()->more(src, line);
	    return 1;
	}

	return 1;
    }

    return 0;
}

/* **************************************************************************
 * lman/sman - Give information on source code in domain or mud.
 */
nomask varargs int
lman(string entry, string docdir)
{
    mixed *argv, *man_arr;
    int argc, i, num, flag;
    string *sdirarr, path, *p_parts, man_chapt, str;

    CHECK_SO_WIZ;

    if (!entry)
    {
        notify_fail("Syntax error, for instructions on usage, " +
	    "do 'help lman'.\n");
	return 0;
    }

    if (!strlen(docdir))
	docdir = this_player()->query_path();

    if (!SRCMAN->valid_docdir(docdir))
    {
        notify_fail("The directory: " + docdir + " is not a valid documentation directory\n");
	return 0;
    }

    argv = explode(entry, " ");
    argc = sizeof(argv);

    switch(argv[0])
    {
    case "-k":

        if (argc == 2)
        {
            sdirarr = (string *)SRCMAN->get_subdirs(docdir);
	    if (member_array(argv[1], sdirarr) < 0)
	    {
	        for (i = 0 ; i < sizeof(sdirarr) ; i++)
	        {
		    man_arr = SRCMAN->get_keywords(docdir, sdirarr[i], argv[1])[1];
		    if (sizeof(man_arr))
		    {
		        write("--- " + sdirarr[i] + ":\n" +
			    sprintf("%-*#s\n", 76, implode(man_arr, "\n")) + "\n");
		        flag = 1;
		    }
	        }
	        if (!flag)
		    write("No match.\n");
	    }
	    else
	    {
	        man_chapt = argv[1];
	        man_arr = SRCMAN->get_index(docdir, man_chapt)[1];
	        this_interactive()->add_prop(WIZARD_S_SMAN_SEL_DIR, man_chapt);
	        this_interactive()->add_prop(WIZARD_AM_SMAN_SEL_ARR, man_arr);
	        g_mannum = 2;
	        str = process_string("#1:" + implode(man_arr, "@@man_num@@"));
	        str = "Available functions:\n" + sprintf("%-*#s\n", 76, str);
	        if (strlen(str) > 5000)
	        {
	            this_player()->more(str + "\n");
	        }
	        else
	        {
	            write(str);
	        }
	    }
        }

        if (argc == 3)
        {
            if (member_array(argv[1], SRCMAN->get_subdirs(docdir)) < 0)
	    {
	        write("No such subdir '" + argv[1] + "' available.\n");
	        break;
	    }
	    man_chapt = argv[1];
	    man_arr = SRCMAN->get_keywords(docdir, man_chapt, argv[2])[1];
	    this_interactive()->add_prop(WIZARD_S_SMAN_SEL_DIR, man_chapt);
	    this_interactive()->add_prop(WIZARD_AM_SMAN_SEL_ARR, man_arr);
	    g_mannum = 2;
	    str = process_string("#1:" + implode(man_arr, "@@man_num@@"));
	    str = "Available subjects:\n" + sprintf("%-*#s\n", 76, str);
	    if (strlen(str) > 5000)
            {
                this_player()->more(str + "\n");
            }
            else
            {
                write(str);
            }
        }
        break;

    case "-?":
        man_chapt = this_interactive()->query_prop(WIZARD_S_SMAN_SEL_DIR);
        if (!man_chapt)
        {
            write("You haven't made any selection yet.\n" +
	        "Do 'lman -k <dir>' to select a directory.\n");
	    break;
        }
        man_arr = this_interactive()->query_prop(WIZARD_AM_SMAN_SEL_ARR);

        g_mannum = 2;
        str = process_string("#1:" + implode(man_arr, "@@man_num@@"));
        write("Selected functions (" + man_chapt + "):\n" +
	    sprintf("%-*#s\n", 76, str));
        break;

    case "-c":
        write("Available subdirs:\n" + sprintf("%-*#s\n", 76,
            implode(SRCMAN->get_subdirs(docdir), "\n")));
        break;

    case "-u":
        SRCMAN->update_index(docdir);
        write("Ok.\n");
        break;

    case "-s":
        if (argc > 1)
        {
	    sscanf(argv[1], "#%d", num);
	    if (num)
	    {
	        man_chapt = this_interactive()->query_prop(WIZARD_S_SMAN_SEL_DIR);
	        man_arr = this_interactive()->query_prop(WIZARD_AM_SMAN_SEL_ARR);

	        if (!man_chapt)
	        {
		    write("You haven't made any selection yet.\n");
		    break;
	        }
	        if (num < 1 || num > sizeof(man_arr))
	        {
		    write("The possible interval is 1-" + sizeof(man_arr) + ".\n");
		    break;
	        }
	        path = docdir + man_chapt + "/" + man_arr[num - 1];

	        if (!find_source(path, argv[0]))
	        {
	            write("No such source code found.\n");
		    break;
	        }
	        break;
	    }

	    if (file_size(docdir +  argv[1]) >= 0)
	    {
	        find_source(docdir + argv[1], argv[0]);
	        break;
	    }

	    sdirarr = SRCMAN->get_subdirs(docdir);
	    if (argc == 3)
	    {
	        if (member_array(argv[1], sdirarr) < 0)
		{
		    write("No such subdir '" + argv[1] + "' available.\n");
		    break;
		}
		man_chapt = argv[1];
		man_arr = SRCMAN->get_keywords(docdir, man_chapt, argv[2])[1];
	    }
	    else
	    {
	        for (i = 0; i < sizeof(sdirarr); i++)
		{
		    man_chapt = sdirarr[i];
		    man_arr = SRCMAN->get_keywords(docdir, man_chapt, argv[1])[1];
		    if (sizeof(man_arr) > 0)
		        break;
		}
	    }

	    if (sizeof(man_arr) == 0)
		write("No command: " + argv[1] + "\n");
	    else
	    {
		path = docdir + man_chapt + "/" + man_arr[0];

		if (file_size(path) < 0)
		    write("Function not found.\n");
		else
		    find_source(path, argv[0]);
	    }
	    break;
        }

    default:
        sscanf(argv[0], "#%d", num);
        if (num)
        {
            man_chapt = this_interactive()->query_prop(WIZARD_S_SMAN_SEL_DIR);
            man_arr = this_interactive()->query_prop(WIZARD_AM_SMAN_SEL_ARR);

            if (!man_chapt)
	    {
	        write("You haven't made any selection yet.\n");
	        break;
	    }
            if (num < 1 || num > sizeof(man_arr))
	    {
	        write("The possible interval is 1-" + sizeof(man_arr) + ".\n");
	        break;
	    }
	    path = docdir + man_chapt + "/" + man_arr[num - 1];

	    write("File: " + path + "\n");
            this_player()->more(path, 1);
	    break;
        }

        if (file_size(docdir + argv[0]) >= 0)
        {
	    write("File: " + docdir + argv[0] + "\n");
	    this_player()->more((docdir + argv[0]), 1);
	    break;
	}

        sdirarr = (string *)SRCMAN->get_subdirs(docdir);
	if (argc == 2)
	{
	    if (member_array(argv[0], sdirarr) < 0)
	    {
	        write("No such subdir '" + argv[0] + "' available.\n");
		break;
	    }
	    man_chapt = argv[0];
	    man_arr = (mixed *)SRCMAN->get_keywords(docdir, man_chapt, argv[1])[1];
	    this_interactive()->add_prop(WIZARD_S_SMAN_SEL_DIR, man_chapt);
	    this_interactive()->add_prop(WIZARD_AM_SMAN_SEL_ARR, man_arr);
	}
	else
	{
	    for (i = 0 ; i < sizeof(sdirarr) ; i++)
	    {
	        man_chapt = sdirarr[i];
		man_arr = (mixed *)SRCMAN->get_keywords(docdir, man_chapt, argv[0])[1];
		if (sizeof(man_arr) > 0)
		    break;
	    }
	}
	if (sizeof(man_arr) == 0)
	    write("No command: " + argv[0] + "\n");
	else
	{
	    path = docdir + man_chapt + "/" + man_arr[0];
            if (file_size(path) < 0)
	        write("No command: " + argv[0] + "\n");
	    else
	    {
	        write("File: " + path + "\n");
		this_player()->more(path, 1);
	    }
	}
	break;
    }

    return 1;
}

/* **************************************************************************
 * man - display a manual page
 */
nomask int
man(string entry)
{
    mixed *argv, *man_arr;
    int argc, i, num, wr_flag;
    string *chaparr, path, *p_parts, man_chapt, str;

    CHECK_SO_WIZ;

    if (!entry)
    {
        notify_fail("Syntax error, for instructions on usage, do 'help man'.\n");
	return 0;
    }

    argv = explode(entry, " ");
    argc = sizeof(argv);

    switch(argv[0])
    {
    case "-k":
        if (argc == 2)
        {
            chaparr = (string *)MANCTRL->get_chapters();
	    if (member_array(argv[1], chaparr) < 0)
	    {
	        for (i = 0 ; i < sizeof(chaparr) ; i++)
	        {
		    man_arr = MANCTRL->get_keywords(chaparr[i], argv[1])[1];
		    if (sizeof(man_arr))
		    {
		        write("--- " + chaparr[i] + ":\n" +
		            sprintf("%-*#s\n", 76, implode(man_arr, "\n")) + "\n");
		        wr_flag = 1;
		    }
	        }
	        if (!wr_flag)
		    write("No such chapter.\n");
	    }
	    else
	    {
	        man_chapt = argv[1];
	        man_arr = MANCTRL->get_index(man_chapt)[1];
	        this_interactive()->add_prop(WIZARD_S_MAN_SEL_CHAPT, man_chapt);
	        this_interactive()->add_prop(WIZARD_AM_MAN_SEL_ARR, man_arr);
	        g_mannum = 2;
	        str = process_string("#1:" + implode(man_arr, "@@man_num@@"));
	        write("Available subjects:\n" + sprintf("%-*#s\n", 76, str));
	    }
	    break;
        }
        if (argc == 3)
        {
            if (member_array(argv[1], MANCTRL->get_chapters()) < 0)
	    {
	        write("No such chapter '" + argv[1] + "' available.\n");
	        break;
	    }
	    man_chapt = argv[1];
	    man_arr = MANCTRL->get_keywords(man_chapt, argv[2])[1];
	    this_interactive()->add_prop(WIZARD_S_MAN_SEL_CHAPT, man_chapt);
	    this_interactive()->add_prop(WIZARD_AM_MAN_SEL_ARR, man_arr);
	    g_mannum = 2;
	    str = process_string("#1:" + implode(man_arr, "@@man_num@@"));
	    write("Available subjects:\n" + sprintf("%-*#s\n", 76, str));
	    break;
        }
        write("Try 'man -c' to see possible chapters.\n");
        break;

    case "-?":
        man_chapt = this_interactive()->query_prop(WIZARD_S_MAN_SEL_CHAPT);
        if (!man_chapt)
        {
            write("You haven't made any selection yet.\n");
	    break;
        }
        man_arr = this_interactive()->query_prop(WIZARD_AM_MAN_SEL_ARR);

        g_mannum = 1;
        str = process_string("#1:" + implode(man_arr, "@@man_num@@"));
        write("Selected subjects (" + man_chapt + "):\n" + sprintf("%-*#s\n", 76, str));
        break;

    case "-c":
        write("Available chapters:\n" +
	    sprintf("%-*#s\n", 76, implode(MANCTRL->get_chapters(), "\n")));
        break;

    case "-u":
        if (find_object(MANCTRL))
	    find_object(MANCTRL)->remove_object();
        write("Ok.\n");
        break;

    default:
        sscanf(argv[0], "#%d", num);
        if (num)
        {
            man_chapt = this_interactive()->query_prop(WIZARD_S_MAN_SEL_CHAPT);
            man_arr = this_interactive()->query_prop(WIZARD_AM_MAN_SEL_ARR);

            if (!man_chapt)
	    {
	        write("You haven't made any selection yet.\n");
	        break;
	    }
            if (num < 1 || num > sizeof(man_arr))
	    {
	        write("The possible interval is 1-" + sizeof(man_arr) + ".\n");
	        break;
	    }
	    path = MANHEAD + man_chapt + "/" + man_arr[num - 1];

	    write("File: " + path + "\n");
	    this_player()->more(path, 1);
	    break;
        }

        if (file_size(MANHEAD + argv[0]) >= 0)
        {
	    write("File: " + MANHEAD + argv[0] + "\n");
	    this_player()->more((MANHEAD + argv[0]), 1);
	    break;
        }

        chaparr = (string *)MANCTRL->get_chapters();
	if (argc == 2)
	{
	    if (member_array(argv[0], chaparr) < 0)
	    {
		write("No such chapter '" + argv[0] +
		      "' available.\n");
		break;
	    }
	    man_chapt = argv[0];
	    man_arr = (mixed *)MANCTRL->get_keywords(man_chapt, argv[1])[1];
	    this_interactive()->add_prop(WIZARD_S_MAN_SEL_CHAPT, man_chapt);
	    this_interactive()->add_prop(WIZARD_AM_MAN_SEL_ARR, man_arr);
	}
	else
	{
	    for (i = 0 ; i < sizeof(chaparr) ; i++)
	    {
		man_chapt = chaparr[i];
		man_arr = (mixed *)MANCTRL->get_keywords(man_chapt, argv[0])[1];
		if (sizeof(man_arr) > 0)
		    break;
	    }
	    if (!sizeof(man_arr) && member_array(argv[0], chaparr) >= 0)
	    {
		man_arr = MANCTRL->get_index(argv[0])[1];
		g_mannum = 2;
		str = process_string("#1:" + implode(man_arr, "@@man_num@@"));
		write("Available subjects:\n" +	sprintf("%-*#s\n", 76, str));
		return 1;
	    }
	}
	if (sizeof(man_arr) == 0)
	    write("No command: " + argv[0] + "\n");
	else
	{
	    path = MANHEAD + man_chapt + "/" + man_arr[0];

	    if (file_size(path) < 0)
		write("No command: " + argv[0] + "\n");
	    else
	    {
		write("File: " + path + "\n");
		this_player()->more(path, 1);
	    }
	}
	break;
    }

    return 1;
}

/* **************************************************************************
 * sman - source manual, give information on functions in the standard mudlib
 */
nomask int
sman(string entry)
{
    CHECK_SO_WIZ;

    if (!entry)
    {
        notify_fail("Syntax error, do 'help sman' for instructions.\n");
	return 0;
    }

    return lman(entry, SMANHEAD);
}

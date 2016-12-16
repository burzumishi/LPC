/*
 * /secure/master/sanctions.c
 *
 * Subpart of /secure/master.c
 *
 * This module contains all sanction related code.
 */

#define SANCTION_READ      ("/read-sanction")
#define SANCTION_READ_ALL  ("/read-all-sanction")
#define SANCTION_WRITE     ("/write-sanction")
#define SANCTION_WRITE_ALL ("/write-all-sanction")
#define SANCTION_SNOOP     ("/snoop-sanction")
#define PERSONAL_SANCTIONS ( ({ "r", "w", "s" }) )
#define DOMAIN_SANCTIONS   ( ({ "r", "R", "w", "W" }) )
#define PATH_SANCTIONS     ( ({ "r", "w" }) )

#define SANCTION_FILE_TO_TOKEN ([ \
				 SANCTION_READ      : "r", \
				 SANCTION_READ_ALL  : "R", \
				 SANCTION_WRITE     : "w", \
				 SANCTION_WRITE_ALL : "W", \
				 SANCTION_SNOOP     : "s" ])

#define SANCTION_TOKEN_TO_FILE ([ \
				 "r" : SANCTION_READ,      \
				 "R" : SANCTION_READ_ALL,  \
				 "w" : SANCTION_WRITE,     \
				 "W" : SANCTION_WRITE_ALL, \
				 "s" : SANCTION_SNOOP ])

/*
 * Function name: valid_snoop_sanction
 * Description  : This function can be used to query whether there is a
 *                snoop sanction set that allows 'snooper' to snoop on
 *                'snoopee'. Note that there is no check on the arguments,
 *                which means the arguments _must_ be in lower case.
 * Arguments    : string snooper - the person that wants to snoop.
 *                string snoopee - the person that is to be snooped.
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int
valid_snoop_sanction(string snooper, string snoopee)
{
    return (file_time(SANCTION_DIR + snoopee + "/" + snooper +
			SANCTION_SNOOP) > 0);
}

/*
 * Function name: valid_read_sanction
 * Description  : This function can be used to query whether there is a
 *                read sanction set that allows 'reader' to read in 'euid'.
 *                Note that there is no check on the arguments, which means
 *                the first argument _must_ be in lower case.
 * Arguments    : string reader - the euid that wants to read.
 *                string euid   - the euid that 'reader' wants to read.
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int
valid_read_sanction(string reader, string euid)
{
    return (file_time(SANCTION_DIR + euid + "/all" +
		       SANCTION_READ) ||
	    file_time(SANCTION_DIR + euid + "/" + reader +
		       SANCTION_READ) ||
	    file_time(SANCTION_DIR + euid + "/" + query_wiz_dom(reader) +
		       SANCTION_READ));
}

/*
 * Function name: valid_read_all_sanction
 * Description  : This function can be used to query whether there is a
 *                'read all' sanction set that allows 'reader' to read in
 *                'euid'. Note that there is no check on the arguments,
 *                which means the first argument _must_ be in lower case.
 * Arguments    : string reader - the euid that wants to read.
 *                string euid   - the euid that 'reader' wants to read.
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int
valid_read_all_sanction(string reader, string euid)
{
    return (file_time(SANCTION_DIR + euid + "/" + reader +
		       SANCTION_READ_ALL) ||
	    file_time(SANCTION_DIR + euid + "/" + query_wiz_dom(reader) +
		       SANCTION_READ_ALL) ||
	    file_time(SANCTION_DIR + euid + "/all" +
		       SANCTION_READ_ALL));
}

/*
 * Function name: valid_write_sanction
 * Description  : This function can be used to query whether there is a
 *                read sanction set that allows 'writer' to write in 'euid'.
 *                Note that there is no check on the arguments, which means
 *                the first argument _must_ be in lower case.
 * Arguments    : string writer - the euid that wants to write.
 *                string euid   - the euid that 'writer' wants to write.
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int
valid_write_sanction(string writer, string euid)
{
    return (file_time(SANCTION_DIR + euid + "/all" +
		       SANCTION_WRITE) ||
	    file_time(SANCTION_DIR + euid + "/" + writer +
		       SANCTION_WRITE) ||
	    file_time(SANCTION_DIR + euid + "/" + query_wiz_dom(writer) +
		       SANCTION_WRITE));
}

/*
 * Function name: valid_write_all_sanction
 * Description  : This function can be used to query whether there is a
 *                'write all' sanction set that allows 'writer' to write in
 *                'euid'. Note that there is no check on the arguments,
 *                which means the first argument _must_ be in lower case.
 * Arguments    : string writer - the euid that wants to write.
 *                string euid   - the euid that 'writer' wants to write.
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int
valid_write_all_sanction(string writer, string euid)
{
    return (file_time(SANCTION_DIR + euid + "/" + writer +
		       SANCTION_WRITE_ALL) ||
	    file_time(SANCTION_DIR + euid + "/" + query_wiz_dom(writer) +
		       SANCTION_WRITE_ALL) ||
	    file_time(SANCTION_DIR + euid + "/all" +
		       SANCTION_WRITE_ALL));
}

/*
 * Function name: valid_read_path_sanction
 * Description  : This function can be used to query whether there is a
 *                read sanction set that allows 'reader' to read the
 *                directory 'path' in 'euid'. Note that there is no check
 *                on the arguments, which means the first argument _must_
 *                be in lower case.
 * Arguments    : string reader - the euid that wants to read.
 *                string euid   - the euid that 'reader' wants to read.
 *                string path   - the path that 'reader' wants to read within
 *                                the directory of 'euid' starting with "/".
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int
valid_read_path_sanction(string reader, string euid, string path)
{
    return (file_time(SANCTION_DIR + euid + "/all" +
		       path + SANCTION_READ) ||
	    file_time(SANCTION_DIR + euid + "/" + reader +
		       path + SANCTION_READ) ||
	    file_time(SANCTION_DIR + euid + "/" + query_wiz_dom(reader) +
		       path + SANCTION_READ));
}

/*
 * Function name: recursive_valid_read_path_sanction
 * Description  : This function will in a loop test the complete path in
 *                'parts' to see whether 'reader' has the right to read that
 *                directory in 'dname'. It doesn't really recurse, but loop.
 * Arguments    : string reader - the euid that wants to read.
 *                string euid   - the euid that 'reader' wants to read.
 *                string *parts - the parts that compose the path.
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int
recursive_valid_read_path_sanction(string reader, string dname, string *parts)
{
    int index;
    int size;

    index = 0;
    parts = ({ "" }) + parts;
    size = sizeof(parts);
    while(++index < size)
    {
	/* If this sanction is valid, go ahead. */
	if (valid_read_path_sanction(reader, dname,
				     implode(parts[..index], "/")))
	{
	    return 1;
	}
    }

    return 0;
}

/*
 * Function name: valid_write_path_sanction
 * Description  : This function can be used to query whether there is a
 *                write sanction set that allows 'writer' to read the
 *                directory 'path' in 'euid'. Note that there is no check
 *                on the arguments, which means the first argument _must_
 *                be in lower case.
 * Arguments    : string writer - the euid that wants to write.
 *                string euid   - the euid that 'writer' wants to write.
 *                string path   - the path that 'writer' wants to write within
 *                                the directory of 'euid' starting with "/".
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int
valid_write_path_sanction(string writer, string euid, string path)
{
    return (file_time(SANCTION_DIR + euid + "/all" +
		       path + SANCTION_WRITE) ||
	    file_time(SANCTION_DIR + euid + "/" + writer +
		       path + SANCTION_WRITE) ||
	    file_time(SANCTION_DIR + euid + "/" + query_wiz_dom(writer) +
		       path + SANCTION_WRITE));
}

/*
 * Function name: recursive_valid_write_path_sanction
 * Description  : This function will in a loop test the complete path in
 *                'parts' to see whether 'writer' has the right to write that
 *                directory in 'dname'. It doesn't really recurse, but loop.
 * Arguments    : string writer - the euid that wants to write.
 *                string euid   - the euid that 'writer' wants to write.
 *                string *parts - the parts that compose the path.
 * Returns      : int 1/0 - allowed/disallowed.
 */
static int
recursive_valid_write_path_sanction(string writer, string dname, string *parts)
{
    int index;
    int size;

    index = 0;
    parts = ({ "" }) + parts;
    size = sizeof(parts);
    while(++index < size)
    {
	/* If this sanction is valid, go ahead. */
	if (valid_write_path_sanction(writer, dname,
				      implode(parts[..index], "/")))
	{
	    return 1;
	}
    }

    return 0;
}

/*
 * Function name: query_sanction
 * Description  : Returns whether a particular sanction exists.
 * Arguments    : string giver    - the euid giving the sanction.
 *                string receiver - the receiver of the sanction.
 *                string type     - the sanction type.
 *                string path     - the path in case of a directory sanction.
 * Returns      : int 1/0 - exists/does not exist.
 */
static varargs int
query_sanction(string giver, string receiver, string type, string path)
{
    return (file_time(SANCTION_DIR + giver + "/" + receiver +
			(strlen(path) ? ("/" + path) : "") + type) > 0);
}

/*
 * Function name: create_sanction
 * Description  : This function will actually create a sanction in the form
 *                of a file in a directory structure. Note that the fourth
 *                argument is optional. This is a static function, ergo all
 *                validity checks on the arguments must be made prior to
 *                calling this function.
 * Arguments    : string giver    - the euid giving the sanction.
 *                string receiver - the receiver of the sanction.
 *                string type     - the sanction type.
 *                string path     - the path in case of a directory sanction.
 * Returns      : int 1/0 - success/failure.
 */
static varargs int
create_sanction(string giver, string receiver, string type, string path)
{
    string *parts;
    int    index;
    int    size;

    set_auth(this_object(), "root:root");

    /* This is the file we are supposed to write. */
    path = SANCTION_DIR + giver + "/" + receiver +
	(strlen(path) ? ("/" + path) : "");

    /* Loop over the parts of the path and build the tree. We do not have to
     * create the first part of the tree because that should be there
     * already. Note that the first element of 'parts' is an empty string.
     */
    index = 2;
    parts = explode(path, "/");
    size = sizeof(parts);
    while(++index < size)
    {
	path = implode(parts[..index], "/");
	switch(file_size(path))
	{
	case -2:
	    continue;

	case -1:
	    if (!mkdir(path))
	    {
		return 0;
	    }
	    continue;

	default:
	    return 0;
	}
    }

    return write_file((path + type),
		      (objectp(this_player()) ?
		       this_player()->query_real_name() : "sanction"));
}

/*
 * Function name: recursive_rmdir
 * Description  : This function will recursively remove a directory with all
 *                its contents, files or directories. If the argument is
 *                a file rather than a directory, this file will be removed
 *                as well.
 * Arguments    : string path - the path to remove.
 * Returns      : int 1/0 - success/failure.
 */
static int
recursive_rmdir(string path)
{
    string *files;
    int    size;

    switch(file_size(path))
    {
    case -2:
	files = get_dir(path + "/*") - ({ ".", ".." });
	size = sizeof(files);
	while(--size >= 0)
	{
	    if (!recursive_rmdir(path + "/" + files[size]))
	    {
		return 0;
	    }
	}
	return rmdir(path);
    
    case -1:
	return 0;

    default:
	return rm(path);
    }

    /* Should never happen. */
    return 0;
}

/*
 * Function name: remove_sanction
 * Description  : This function will actually remove a sanction from the
 *                directory structure. All but the first argument are
 *                optional. This is a static function, ergo all validity
 *                checks on the arguments must be made prior to calling
 *                this function.
 * Arguments    : string giver    - the euid giving the sanction.
 *                string receiver - the receiver of the sanction.
 *                string type     - the sanction type.
 *                string path     - the path in case of a directory sanction.
 * Returns      : int 1/0 - success/failure.
 */
static varargs int
remove_sanction(string giver, string receiver, string type, string path)
{
    string *parts;
    int    size;

    set_auth(this_object(), "root:root");

    /* Construct the path to remove. */
    path = SANCTION_DIR + giver +
	(strlen(receiver) ?
	 ("/" + receiver +
	  (strlen(path) ? ("/" + path) : "") +
	  (strlen(type) ? type : "")) : "");

    /* Now, if it is a directory, we will first recursively remove everything
     * that is in the directory. If it is a file, just remove the file.
     */
    if (file_size(path) == -2)
    {
	if (!recursive_rmdir(path))
	{
	    return 0;
	}
    }
    else
    {
	if (!rm(path))
	{
	    return 0;
	}
    }

    /* Then we will go up the tree as far as we can and remove empty
     * directories.
     */
    parts = explode(path, "/");
    size = sizeof(parts) - 1;
    while(--size > 2)
    {
	if (sizeof(get_dir(implode(parts[..size], "/") + "/*")) == 2)
	{
	    if (!rmdir(implode(parts[..size], "/")))
	    {
		return 0;
	    }
	}
    }

    return 1;
}

/*
 * Function name: remove_all_sanctions
 * Description  : This function can be used to remove all sanctions that have
 *                been given out by a wizard or domain and that have been
 *                received from a wizard or domain. It is automatically called
 *                when a domain is removed or when a wizard is demoted out of
 *                wizardhood.
 * Arguments    : string name - the domain or wizard loosing its sanction.
 */
static void
remove_all_sanctions(string name)
{
    string *files;
    int    index;
    int    size;

    set_auth(this_object(), "root:root");

    /* Remove the sanctions this wizard or domain has given out. */
    remove_sanction(name);

    /* Remove all sanctions this wizard or domain has received. */
    files = get_dir(SANCTION_DIR + "*");
    index = -1;
    size = sizeof(files);
    while(++index < size)
    {
	/* Only remove it if there is such a sanction indeed. */
	if (file_size(SANCTION_DIR + files[index] + "/" + name) == -2)
	{
	    remove_sanction(files[index], name);
	}
    }
}

/*
 * Function name: display_sanction_recursively
 * Description  : This function will actually print the sanctions that a
 *                particular domain or wizard has given out. For each
 *                sanction receiver the tree will be followed and printed.
 * Arguments    : string giver - the name of the sanction giver.
 *                string root  - the root directory for the sanction.
 *                string name  - the name of the sanction receiver.
 *                string path  - the path in case of a directory sanction.
 */
static void
display_sanction_recursively(string giver, string root, string name,
			     string path)
{
    string *files;
    string code, rank;
    int    index;
    int    size;

    /* Here we loop over the sanctions of the receiver. There are two cases.
     * Either this is a normal sanction, in which case the directory contains
     * only files, the sanction types. Add them to a nice string and print
     * them. The other case is if a directory is found. This would mean that
     * we deal with a special directory sanction, in which case we recurse
     * into the directory.
     */
    code = "";
    files = get_dir(root + name + path + "*") - ({ ".", ".." });
    index = -1;
    size = sizeof(files);
    while(++index < size)
    {
	if (file_size(root + name + path + files[index]) == -2)
	{
	    display_sanction_recursively(giver, root, name,
					 (path + files[index] + "/"));
	}
	else
	{
	    code += SANCTION_FILE_TO_TOKEN["/" + files[index]];
	}
    }

    if (strlen(code))
    {
	if (query_domain_number(name) < 0)
	    rank = "[" + WIZ_RANK_NAME(query_wiz_rank(name)) + "]";
	else
	    rank = "[domain]";

	write(sprintf("%-12s %-12s %-5s %s\n", capitalize(name), rank, code,
		      ((path == "/") ? "" : ("~" + giver + path))));
    }
}

/*
 * Function name: display_sanctions
 * Description  : This function can be used display the sanctions a wizard
 *                or domain has given out.
 * Arguments    : string name - the wizard or domain who gave the sanction.
 */
static void
display_sanctions(string name)
{
    string path;
    string *files;
    int    index = -1;
    int    size;

    path = SANCTION_DIR + name;
    if (file_size(path) == -1)
    {
	write("No one received a sanction from " +
	      ((name == this_player()->query_real_name()) ? "you" :
	       capitalize(name)) + ".\n");
	return;
    }

    write("Sanction list of " + capitalize(name) + ".\n");

    /* We read the top level directory in the sanction structure for this
     * domain or wizard. These are the receivers of the sanctions. From
     * there we proceed by recursively displaying those sanctions.
     */
    path += "/";
    files = get_dir(path + "*") - ({ ".", ".." });
    size = sizeof(files);
    while(++index < size)
    {
	display_sanction_recursively(name, path, files[index], "/");
    }
}

/*
 * Function name: sanction
 * Description  : This function contains the code for the sanction command.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
public int
sanction(string str)
{
    string *parts;
    string name = this_player()->query_real_name();
    int    index;
    int    size;
    int    remove;

    /* May only be called from the apprentice soul. */
    if (!CALL_BY(WIZ_CMD_APPRENTICE))
    {
	return 0;
    }

    /* No argument argument, display your own sanctions. */
    if (!strlen(str))
    {
	display_sanctions(name);
	return 1;
    }

    /* Arches, keepers and Lord of their wizards may view the sanction list
     * of a particular wizard.
     */
    if (sscanf(str, "display %s", str) == 1)
    {
	if (!query_wiz_rank(str))
	{
	    notify_fail("There is no wizard named \"" + str + "\".\n");
	    return 0;
	}

	if ((query_wiz_rank(name) < WIZ_ARCH) &&
	    (query_domain_lord(query_wiz_dom(str)) != name))
	{
	    notify_fail("You are neither a member of the administration, " +
			"nor the Lord over " + capitalize(str) + ".\n");
	    return 0;
	}

	display_sanctions(str);
	return 1;
    }

    /* Clear your personal sanction list. */
    if ((str == "clear") ||
	(str == "c"))
    {
	write("Removing all your personal sanctions.\n");
	recursive_rmdir(SANCTION_DIR + name);
	return 1;
    }

    /* If the argument is not valid, the sanction may only be removed. */
    parts = explode(str, " ") - ({ "" });
    if (query_domain_number(parts[0]) > -1)
    {
	parts[0] = capitalize(parts[0]);
    }
    else if ((parts[0] != "all") &&
	(query_wiz_rank(parts[0]) < WIZ_APPRENTICE))
    {
	remove = 1;
    }

    /* Remove the sanction given to a particular domain or wizard. */
    if (sizeof(parts) == 1)
    {
	if (file_size(SANCTION_DIR + name + "/" + parts[0]) != -2)
	{
	    write("You have not given a sanction to \"" +
		  parts[0] + "\".\n");
	    return 1;
	}

	write("Removing sanction(s) given to \"" + parts[0] + "\".\n");
	recursive_rmdir(SANCTION_DIR + name + "/" + parts[0]);
	rmdir(SANCTION_DIR + name);
	return 1;
    }

    /* Set or clear sanctions. Must be two arguments now. */
    if (sizeof(parts) != 2)
    {
	notify_fail("Syntax: sanction <name> <type>");
	return 0;
    }

    str = parts[0];
    parts = explode(parts[1], "");
    index = -1;
    size = sizeof(parts);
    while(++index < size)
    {
	if (member_array(parts[index], PERSONAL_SANCTIONS) == -1)
	{
	    write("Sanction \"" + parts[index] +
		  "\" is not possible as personal sanction.\n");
	    continue;
	}

	/* If the sanction is not there yet, if may only be set if the
	 * argument is valid.
	 */
	if (!query_sanction(name, str, SANCTION_TOKEN_TO_FILE[parts[index]]))
	{
	    if (remove)
	    {
		write("Invalid receiver \"" + str + "\". Sanction \"" +
		      parts[index] + "\" may only be removed.\n");
		continue;
	    }

	    if (create_sanction(name, str,
				SANCTION_TOKEN_TO_FILE[parts[index]]))
	    {
		write("Sanction \"" + parts[index] + "\" given to " +
		      capitalize(str) + ".\n");
	    }
	    else
	    {
		write("Error setting sanction \"" + parts[index] + "\" to " +
		      capitalize(str) + ".\n");
	    }
	}
	else
	{
	    if (remove_sanction(name, str,
				SANCTION_TOKEN_TO_FILE[parts[index]]))
	    {
		write("Sanction \"" + parts[index] + "\" removed from " +
		      capitalize(str) + ".\n");
	    }
	    else
	    {
		write("Error removing sanction \"" + parts[index] +
		      "\" from " + capitalize(str) + ".\n");
	    }
	}
    }

    return 1;
}

/*
 * Function name: domainsanction
 * Description  : This function contains the code for the domainsanction
 *                command.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
public int
domainsanction(string str)
{
    string *parts;
    string name = this_player()->query_real_name();
    string domain;
    string path;
    int    remove;
    int    index;
    int    size;

    /* May only be called from the apprentice soul. */
    if (!CALL_BY(WIZ_CMD_APPRENTICE))
    {
	return 0;
    }

    /* No argument. Display your domain's sanctions. */
    if (!strlen(str))
    {
	if (!strlen(str = query_wiz_dom(name)))
	{
	    notify_fail("You are not a member of any domain.\n");
	    return 0;
	}

	display_sanctions(str);
	return 1;
    }

    parts = explode(str, " ") - ({ "" });
    domain = capitalize(lower_case(parts[0]));
    if (query_domain_number(domain) == -1)
    {
	notify_fail("There is no domain named \"" + domain + "\".\n");
	return 0;
    }

    /* Display the sanctions of that particular domain. */
    if (sizeof(parts) == 1)
    {
	display_sanctions(domain);
	return 1;
    }

    /* Must be either a member of the administration or the Lord of the
     * domain to continue.
     */
    if (!((query_wiz_rank(name) >= WIZ_ARCH) ||
          ((query_wiz_rank(name) == WIZ_LORD) &&
           (query_wiz_dom(name) == domain))))
    {
	notify_fail("You are neither a member of the administration, nor " +
		    "the Lord of the domain " + domain + ".\n");
	return 1;
    }

    parts[1] = lower_case(parts[1]);
    
    /* Clear all sanctions from the domain. */
    if ((parts[1] == "clear") ||
	(parts[1] == "c"))
    {
	write("Removing all sanctions from domain " + domain + ".\n");
	recursive_rmdir(SANCTION_DIR + domain);
	return 1;
    }

    if (sizeof(parts) > 4)
    {
	notify_fail("Too many arguments to domainsanction.\n");
	return 0;
    }

    /* If the argument is not valid, the sanction may only be removed. */
    if (query_domain_number(parts[1]) > -1)
    {
	parts[1] = capitalize(parts[1]);
    }
    else if ((parts[1] != "all") &&
	(query_wiz_rank(parts[1]) < WIZ_APPRENTICE))
    {
	remove = 1;
    }

    /* Remove the sanction given to a particular domain or wizard. */
    if (sizeof(parts) == 2)
    {
	if (file_size(SANCTION_DIR + domain + "/" + parts[1]) != -2)
	{
	    write("The domain " + domain + " has not given a sanction to \"" +
		  parts[1] + "\".\n");
	    return 1;
	}

	write("Removing sanction(s) given to \"" + parts[1] + "\" in " +
	     domain + ".\n");
	recursive_rmdir(SANCTION_DIR + domain + "/" + parts[1]);
	rmdir(SANCTION_DIR + domain);
	return 1;
    }

    str = parts[1];
    path = ((sizeof(parts) == 4) ? parts[3] : "");
    parts = explode(parts[2], "");
    index = -1;
    size = sizeof(parts);
    while(++index < size)
    {
	if (member_array(parts[index],
			 (strlen(path) ? PATH_SANCTIONS :
			  DOMAIN_SANCTIONS)) == -1)
	{
	    write("Sanction \"" + parts[index] +
		  "\" is not possible as domain " +
		  (strlen(path) ? "path " : "") + "sanction.\n");
	    continue;
	}

	/* If the sanction is not there yet, if may only be set if the
	 * argument is valid.
	 */
	if (!query_sanction(domain, str, SANCTION_TOKEN_TO_FILE[parts[index]],
		 	    path))
	{
	    if (remove)
	    {
		write("Invalid receiver \"" + str + "\". Sanction \"" +
		      parts[index] + "\" may only be removed from " +
		      domain + ".\n");
		continue;
	    }
	    
	    if (create_sanction(domain, str,
				SANCTION_TOKEN_TO_FILE[parts[index]], path))
	    {
		write("Sanction \"" + parts[index] + "\" given to " +
		      capitalize(str) + " for " + domain +
		      (strlen(path) ? (" for " + path) : "") + ".\n");
	    }
	    else
	    {
		write("Error setting sanction \"" + parts[index] +
		      "\" to " + capitalize(str) + " for " + domain +
		      (strlen(path) ? (" for " + path) : "") + ".\n");
	    }
	}
	else
	{
	    if (remove_sanction(domain, str,
				SANCTION_TOKEN_TO_FILE[parts[index]], path))
	    {
		write("Sanction \"" + parts[index] + "\" to " + domain +
		      " removed from " + capitalize(str) +
		      (strlen(path) ? (" for " + path) : "") + ".\n");
	    }
	    else
	    {
		write("Error removing sanction \"" + parts[index] +
		      "\" to " + domain + " from " + capitalize(str) +
		      (strlen(path) ? (" for " + path) : "") + ".\n");
	    }
	}
    }

    return 1;
}

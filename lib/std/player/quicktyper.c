/*
 * /std/player/quicktyper.c
 *
 * This module is the quicktyper. It takes care of aliases and do
 * sequences.
 */

#include <macros.h>
#include <std.h>
#include <options.h>

#define ALIAS_LENGTH (query_wiz_level() ? 45 : 30)
#define NICK_LENGTH  (query_wiz_level() ? 30 : 30)

/*
 * Global variables, all static, i.e. non-savable.
 */
static private string last_command = "";
static private int    paused       = 0;
static private string do_sequence  = "";
static private int    do_alarm     = 0;

/* 
 * Prototypes.
 */
static nomask int alias(string str);
static nomask int nick(string str);
static nomask int doit(string str);
static nomask int resume(string str);
static nomask int unalias(string str);
static nomask int unnick(string str);

/*
 * Function name: init_cmdmodify
 * Description  : The quicktyper commands are added to the player.
 */
static nomask void
init_cmdmodify()
{
    add_action(alias,   "alias");
    add_action(nick,    "nick");
    add_action(doit,    "do");
    add_action(resume,  "resume");
    add_action(unalias, "unalias");
    add_action(unnick,  "unnick");
}

/*
 * Function name: modify_command
 * Description  : This function is called when it is time to modify a
 *                command. It resolves aliases and takes care of the
 *                remaining history functionality.
 * Arguments    : string str - The command to modify.
 * Returns      : string     - The modified command.
 */
nomask public string
modify_command(string str)
{
    string *words;
    string *subst_words;

    if (!strlen(str))
	return str;

    /* Player wants to repeat the last command. */
    if (str == "%%")
    {
 	if (this_player()->query_option(OPT_ECHO))
	    write("Doing: " + last_command + "\n");

	return last_command;
    }

    words = explode(str, " ");

    /* Resolve for aliases. */
    if (m_alias_list[words[0]])
    {
	/* Replace the first word with the aliased string. */
	words[0] = m_alias_list[words[0]];

	/* If the aliased string containts the text '%%', this means to
	 * replace that '%%' with the remaining words of the command line.
	 */
	if (wildmatch("*%%?*", words[0]))
	    subst_words = explode(words[0], "%%");
    }

    /* Resolve nicknames. */
    if (m_sizeof(m_nick_list) &&
        (words[0] != "unnick") && (words[0] != "nick"))
    {
	int size = sizeof(words);

	while(size--)
	{
	    if (m_nick_list[words[size]])
		words[size] = m_nick_list[words[size]];
	}
    }

    /* Reconstruct the command. */
    if (subst_words)
    {
	str = subst_words[0] + implode(words[1..], " ") +
	    implode(subst_words[1..], "");
    }
    else
    {
	str = implode(words, " ");
    }

    /* Save the last command given to be retrieved with %%. */
    last_command = str;
    return str;
}

/*
 * Function name: alias
 * Description  : Make an alias, or display one or all current alias(es).
 * Arguments    : string str - the command line argument.
 * Returns      : int - 1/0 - success/failure.
 */
static nomask int
alias(string str)
{
    int    index;
    int    size;
    string a;
    string cmd;
    string *list;
    object player;
    mapping a_list;

    /* No-one can be forced to make an alias. */
    if (this_interactive() != this_object())
    {
	return 0;
    }

    /* List all aliases. */
    if (!stringp(str))
    {
	list = sort_array(m_indices(m_alias_list));
	size = sizeof(list);
	index = -1;

	write("You have " + LANG_WNUM(size) +
	    ((size == 1) ? " alias" : " aliases") + " and " +
	    ((size >= ALIAS_LENGTH) ? "NO room for more." :
	    ("room for " + LANG_WNUM(ALIAS_LENGTH - size) + " more.")) +
 	    "\n\n");

	while(++index < size)
	    write(sprintf("%-8s: %s\n", list[index],
		m_alias_list[list[index]]));

	return 1;
    }

    /* List one alias. */
    if (m_alias_list[str])
    {
	write(sprintf("%-8s: %s\n", str, m_alias_list[str]));
	return 1;
    }

    /* Add a new alias, must consist of a name and a value, else we assume
     * the player wanted to display a non-existant alias.
     */
    if (sscanf(str, "%s %s", a, cmd) != 2)
    {
	notify_fail("No such alias: \"" + str + "\".\n");
	return 0;
    }

    /* Wizard may see other people's aliases. */
    if (a == "-l")
    {
	if (SECURITY->query_wiz_rank(query_real_name()) < WIZ_NORMAL)
	{
	    notify_fail("You can only see your own aliases.\n");
	    return 0;
	}

        if (!objectp(player = find_player(lower_case(cmd))))
	{
	    notify_fail("Player " + capitalize(cmd) + " is not present.\n");
	    return 0;
	}

	a_list = player->query_aliases();
	list = sort_array(m_indices(a_list));
	size = sizeof(list);
	index = -1;

	write("The aliases of " + capitalize(cmd) + " are:\n\n");
	while(++index < size)
	    write(sprintf("%-8s: %s\n", list[index], a_list[list[index]]));

	return 1;
    }

    /* Replace the alias if is already exists.*/
    if (m_alias_list[a])
    {
	write("Replacing previous alias \"" + a + "\" which was aliased " +
	      "to \"" + m_alias_list[a] + "\".\n");
    }
    /* See whether there is room for yet another alias. */
    else if (m_sizeof(m_alias_list) >= ALIAS_LENGTH)
    {
	write("Sorry, the alias list is full! The maximum is " +
	      ALIAS_LENGTH + " Use \"unalias <cmd>\" to make some space.\n");
	return 1;
    }

    m_alias_list[a] = cmd;
    write("Alias \"" + a + "\" added to \"" + cmd + "\"\n");
    return 1;
}

/*
 * Function name: remove_do_alarm
 * Description  : This is a service function. Call it to remove the do-alarm
 *                from a player if necessary.
 */
public nomask void
remove_do_alarm()
{
    remove_alarm(do_alarm);
    do_alarm = 0;
}

/*
 * Function name: do_chain
 * Description  : Do next command in the do chain.
 */
static nomask void
do_chain()
{
    int index;
    string cmd;
    string rest;

    /* Player is in combat, no do sequences allowed. */
    if (objectp(query_attack()))
    {
	remove_alarm(do_alarm);
	paused = 1;

	tell_object(this_object(),
	    "Your do-chain is paused since you are now in combat.\n" +
	    "When you are no longer in combat, you can 'resume' it.\n");
	return;
    }

    /* Get the first part of the command and execute it. */
    if (sscanf(do_sequence, "%s,%s", cmd, rest) == 2)
    {
	if (strlen(rest))
	{
            index = -1;
            while (cmd[++index] == ' ');
            cmd = cmd[index..];
	    tell_object(this_object(), "Doing: " + cmd + "\n");
	    do_sequence = rest;
	    this_object()->command(cmd);
	    return;
	}
	do_sequence = cmd;
    }

    index = -1;
    while (do_sequence[++index] == ' ');
    do_sequence = do_sequence[index..];

    remove_alarm(do_alarm);
    tell_object(this_object(), "Doing: " + do_sequence + "\n");
    this_object()->command(do_sequence);
    tell_object(this_object(), "Done.\n");
    do_sequence = "";
}

/* 
 * Function name: doit
 * Description  : Do a sequence of commands.
 * Arguments    : string str - the command line argument.
 * Returns      : int - 1/0 - success/failure.
 */
static nomask int
doit(string str)
{
    /* Access failure. You cannot be forced to 'do' anything. */
    if (this_interactive() != this_object())
	return 0;

    /* No argument. If a 'do' is going on, pause it. */
    if (!strlen(str))
    {
	if (!strlen(do_sequence))
	{
	    notify_fail("Usage: do cmd1,cmd2,cmd3,...\n");
	    return 0;
	}

	write("Paused the execution of \"do\".\n" +
	      "Use \"resume\" to continue.\n");
	remove_alarm(do_alarm);
	paused = 1;
	return 1;
    }

    /* If a 'do' is going on and it is not paused, reject the call. */
    if (strlen(do_sequence) &&
	!paused)
    {
	write("Busy executing commands:\n  " + do_sequence + "\n");
	return 1;
    }

    /* There is a 'do', but it has been paused, skip the paused commands. */
    if (strlen(do_sequence))
    {
	write("Skipping paused commands:\n  " + do_sequence + "\n");
	paused = 0;
    }

    /* Disallow do-commands to be executed while the player is in combat. */
    if (objectp(query_attack()))
    {
	write("You cannot use the command 'do' since you are in combat.\n");
	return 1;
    }

    /* Start the new 'do'. The first alarm is in 0.0 seconds since we want
     * that to be executed immediately.
     */
    do_sequence = str;
    do_alarm = set_alarm(0.0, 2.0, do_chain);

    return 1;
}

/* 
 * Function name: resume
 * Description  : Resume the processing of the commands in the do chain.
 * Arguments    : string str - the command line argument.
 * Returns      : int - 1/0 - success/failure.
 */
static nomask int
resume(string str)
{
    /* Access failure. You cannot be forced to resume. */
    if (this_interactive() != this_object())
	return 0;

    /* No argument possible. */
    if (stringp(str))
    {
	notify_fail("Resume what? " +
	    "Just \"resume\" to continue a paused \"do\".\n");
	return 0;
    }

    /* There is no 'do' going on or it isn't paused. */
    if (!strlen(do_sequence) ||
	!paused)
    {
	write("Nothing to resume.\n");
	return 1;
    }

    /* Disallow players to resume a do while they are in combat. */
    if (objectp(query_attack()))
    {
	write("You cannot resume a do-sequence since you are in combat.\n");
	return 1;
    }

    /* Resume the 'do'. */
    paused = 0;
    do_alarm = set_alarm(0.0, 2.0, do_chain);
    return 1;
}

/*
 * Function name: unalias
 * Description  : Remove an alias.
 * Arguments    : string str - the command line argument.
 * Returns      : int - 1/0 - success/failure.
 */
static nomask int
unalias(string str)
{
    if (!stringp(str))
    {
	notify_fail("Syntax: unalias <alias-name>\n");
	return 0;
    }

    if (!m_alias_list[str]) 
    {
	notify_fail("Alias \"" + str + "\" does not exist!\n");
	return 0;
    }

    write("Alias \""+ str +"\" removed. Used to be: "+ m_alias_list[str] +".\n");
    m_delkey(m_alias_list, str);
    return 1;
}

/*
 * Function name: nick
 * Description  : Make a nick, or display one or all current nickname(s).
 * Arguments    : string str - the command line argument.
 * Returns      : int - 1/0 - success/failure.
 */
static nomask int
nick(string str)
{
    int    index;
    int    size;
    string a;
    string cmd;
    string *list;
    object player;
    mapping n_list;

    /* No-one can be forced to make an alias. */
    if (this_interactive() != this_object())
	return 0;

    /* List all nicks. */
    if (!strlen(str))
    {
	list = sort_array(m_indices(m_nick_list));
	size = sizeof(list);
	index = -1;

	write("You have " + LANG_WNUM(size) +
	    ((size == 1) ? " nickname" : " nicknames") + " and " +
	    ((size >= NICK_LENGTH) ? "NO room for more." :
		("room for " + LANG_WNUM(NICK_LENGTH - size) + " more.")) +
	    "\n\n");

	while(++index < size)
	    write(sprintf("%-8s: %s\n", list[index], m_nick_list[list[index]]));

	return 1;
    }

    /* List one alias. */
    if (m_nick_list[str])
    {
	write(sprintf("%-8s: %s\n", str, m_nick_list[str]));
	return 1;
    }

    /* Add a new nickname, must consist of a name and a value, else we assume
     * the player wanted to display a non-existant alias.
     */
    if (sscanf(str, "%s %s", a, cmd) != 2)
	return notify_fail("No such nickname: \"" + str + "\".\n");

    /* Wizard may see other people's aliases. */
    if (a == "-l")
    {
	if (SECURITY->query_wiz_rank(query_real_name()) < WIZ_NORMAL)
	    return notify_fail("You can only see your own nicknames.\n");

	if (!objectp(player = find_player(lower_case(cmd))))
	    return notify_fail("Player "+ capitalize(cmd) +" is not present.\n");

	n_list = player->query_nicks();
	list = sort_array(m_indices(n_list));
	size = sizeof(list);
	index = -1;

	write("The nicknames of " + capitalize(cmd) + " are:\n\n");
	while(++index < size)
	    write(sprintf("%-8s: %s\n", list[index], n_list[list[index]]));

	return 1;
    }

    /* Replace the nickname if is already exists.*/
    if (m_nick_list[a])
    {
	write("Replacing previous nick \"" + a + "\" which was nicknamed " +
	    "to \"" + m_nick_list[a] + "\".\n");
    }
    /* See whether there is room for yet another nickname. */
    else if (m_sizeof(m_nick_list) >= NICK_LENGTH)
    {
	write("Sorry, the nickname list is full! The maximum is " +
	    NICK_LENGTH + " Use \"unnick <cmd>\" to make some space.\n");
	return 1;
    }

    m_nick_list[a] = cmd;
    write("Nickname \"" + a + "\" added to \"" + cmd + "\"\n");
    return 1;
}

/*
 * Function name: unnick
 * Description  : Remove a nickname.
 * Arguments    : string str - the command line argument.
 * Returns      : int - 1/0 - success/failure.
 */
static nomask int
unnick(string str)
{
    if (!strlen(str))
	return notify_fail("Syntax: unnick <nickname>\n");

    if (!m_nick_list[str]) 
	return notify_fail("Nickname \""+ str +"\" does not exist!\n");

    write("Nickname \""+ str +"\" removed. Used to be: "+ m_nick_list[str] +".\n");
    m_delkey(m_nick_list, str);
    return 1;
}

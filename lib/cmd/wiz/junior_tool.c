/*
 * /cmd/wiz/junior_tool.c
 *
 * This object holds the commands that can be executed by juniors to
 * wizards. This allows them them to execute some commands without having
 * to have their wizard character logged in, which is even impossible to
 * some players.
 *
 * Some commands are copied from the apprentice soul and some are coded
 * specificly. Too bad the commands cannot be accessed with call_other
 * to the appropriate soul for this player is not a wizard.
 *
 * NOTE: For information about the help-file and index generation, see
 * the remark at the ALL_COMMANDS define below and the function
 * get_help_index() defined at the end of this object.
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <cmdparse.h>
#include <composite.h>
#include <filepath.h>
#include <files.h>
#include <language.h>
#include <macros.h>
#include <std.h>
#include <stdproperties.h>
#include <options.h>

#define CHECK_SO_JUNIOR if (!check_so_junior()) return 0
#define PARSE_LIST(s)   ("/cmd/std/tracer_tool_base"->parse_list(s))
#define HELP_FILE	"/doc/help/wizard/junior_help"
#define ENTRY_HEADER	"SYNTAX: "

/* This should list the possible commands, supported by this tool. The
 * indices to the mapping are the possible commands. It returns an array
 * with the beginning lines of the entries of the commands in the help-
 * file. The second number is the number of lines of the help entry.
 *
 * To get the figures, you can call the function get_help_index() in
 * this junior tool. After you altered the help file, you have to call
 * the function and manually update this mapping! The numbers printed
 * by the forementioned function can be easily pasted into this mapping.
 *
 * Be advised that only the commands in this mapping are displayed with
 * the allcmd command, which is a good stimulant to create a help-entry
 * for each command, how trivial it may seem.
 */
#define ALL_COMMANDS ([ "allcmd": ({  25,  6 }), \
			"clone" : ({  31, 13 }), \
			"death" : ({  44, 12 }), \
			"goto"  : ({  56, 13 }), \
			"help"  : ({  69, 10 }), \
			"home"  : ({  79, 12 }), \
			"list"  : ({  91, 10 }), \
			"peace" : ({ 101,  7 }), \
			"stat"  : ({ 108, 15 }), \
			"tell"  : ({ 123, 10 }), \
			"update": ({ 133, 12 }) ])

/*
 * Prototypes.
 */
nomask static int allcmd(string str);
nomask static int clone(string str);
nomask static int death(string str);
nomask static int goto(string dest);
nomask static int help(string str);
nomask static int home(string str);
nomask static int list(string str);
nomask static int peace(string str);
nomask static int stat(string str);
nomask static int tell(string str);
nomask static int update(string str);

/*
 * Function name: create_object
 * Description  : When the object is created, this function is called to
 *                configure it.
 */
nomask public void
create_object()
{
    set_name("tool");
    set_pname("tools");

    set_adj("junior");

    set_short("junior tool");
    set_pshort("junior tools");

    set_long(break_string("In order to be able to operate independantly " +
	"from wizards, this tool was developed to give junior characters " +
	"access to some wizard-commands. To see which commands are " +
	"supported by this tool you should do <allcmd>. Do <help junior " +
	"topic> to get help on a command.", 74) + "\n");

    add_prop(OBJ_I_VALUE,       0);
    add_prop(OBJ_I_VOLUME,      0);
    add_prop(OBJ_I_WEIGHT,      0);

    add_prop(OBJ_M_NO_BUY,      1);
    add_prop(OBJ_M_NO_DROP,     "@@not_wizard");
    add_prop(OBJ_M_NO_GET,      "@@not_junior");
    add_prop(OBJ_M_NO_GIVE,     "@@not_wizard");
    add_prop(OBJ_M_NO_SELL,     1);
    add_prop(OBJ_M_NO_STEAL,    1);
    add_prop(OBJ_M_NO_TELEPORT, 1);

    add_prop(OBJ_S_WIZINFO,
	break_string("The junior tool allowes juniors to real wizards to " +
	"operate independantly from their wizard-characters. Any junior " +
	"character having the same name as its wizards with \"jr\" appended " +
	"to it, may use it. However, the normal wizard-rules apply.",
	74) + "\n");

    setuid();
    seteuid(getuid());

    set_lock();
}

/*
 * Function name: not_wizard
 * Description  : Called to see if the player is not a wizard.
 * Returns      : 1 - the player is not a wizard
 *                0 - the player is a wizard
 */
nomask public int
not_wizard()
{
    return (!(this_player()->query_wiz_level()));
}

/*
 * Function name: not_junior
 * Description  : Called to see if the player is not a junior nor a wizard.
 * Returns      : 1 - the player is not a junior nor a wizard
 *                0 - the player is a junior or a wizard
 */
nomask public int
not_junior()
{
    string name;

    if (sscanf(this_player()->query_real_name(), "%sjr", name) == 1)
    {
	if (SECURITY->query_wiz_rank(name) > WIZ_RETIRED)
	{
	    return 0;
	}
    }

    return not_wizard();
}

/***************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
nomask public void
init()
{
    ::init();

    add_action(allcmd, "allcmd");
    add_action(clone , "clone");
    add_action(death , "death");
    add_action(goto  , "goto");
    add_action(help  , "help");
    add_action(home  , "home");
    add_action(list  , "list");
    add_action(peace , "peace");
    add_action(stat  , "stat");
    add_action(tell  , "tell");
    add_action(update, "update");
}

/***************************************************************************
 * Return 1 if the player who executes the command is a junior second
 * to a real wizard. (not apprentice or retired)
 */
nomask static int
check_so_junior()
{
    string name;

    if (this_interactive() != this_player())
    {
	notify_fail("You cannot be forced into doing this.\n");
	return 0;
    }

    if (environment() != this_player())
    {
	notify_fail("You do not hold the tool.\n");
	return 0;
    }

    if (this_player()->query_wiz_level())
    {
	notify_fail("Junior tool command ignored.\n");
	return 0;
    }

    if (sscanf((string)this_player()->query_real_name(), "%sjr", name) != 1)
    {
	/* Should never happen! */
	remove_object();
	notify_fail("You are not a junior character!\n");
	return 0;
    }

    if (SECURITY->query_wiz_rank(name) <= WIZ_RETIRED)
    {
	remove_object();
	notify_fail("You are not the junior to a normal wizard.\n");
	return 0;
    }

    return 1;
}

/*
 * Function name: log_usage
 * Description  : Makes a log of the fact that the junior tool was used.
 * Arguments    : string cmd - the command issued.
 */
nomask static void
log_usage(string cmd)
{
    SECURITY->log_syslog("JUNIOR_TOOL", sprintf("%24s %-12s %-1s\n",
	ctime(time()), capitalize(this_interactive()->query_real_name()),
	cmd));
}

/***************************************************************************
 * Here follows the actual functions. Please add new functions in the
 * same order as in the function name list. To minimuze the use of
 * prototypes, put service functions before the actual functions.
 */

/***************************************************************************
 * allcmd - list your commands
 *
 * Function designed for this tool.
 */
nomask static int
allcmd(string str)
{
    CHECK_SO_JUNIOR;
    log_usage("allcmd");

    if (stringp(str))
    {
	notify_fail("Allcmd what? You can only use \"allcmd\" to get your " +
	    "own commands.\n");
	return 0;
    }

    write("The following commands are possible for juniors:\n\n" +
	break_string(COMPOSITE_WORDS(sort_array(m_indices(ALL_COMMANDS))),
	74) + "\n");
    return 1;
}

/***************************************************************************
 * clone - clone an object
 *
 * Function taken from WIZ_CMD_NORMAL and adjusted for junior usage.
 */
nomask static object
clone_ob(string str)
{
    object ob;
    string name;

    if (!stringp(str))
    {
	notify_fail("Invalid file.\n");
	return 0;
    }

    if (sscanf(this_player()->query_real_name(), "%sjr", name) != 1)
    {
	notify_fail("Your name does not end with \"jr\". Impossible!\n");
	return 0;
    }

    /* A junior does not have a path, so we have to construct one using
     * the name of the wizard.
     */
    str = FPATH(SECURITY->query_wiz_path(name), TPATH(name, str));

    if (file_size(str + ".c") < 0 && file_size(str) < 0)
    {
	notify_fail("No such file: " + str + "\n");
	return 0;
    }
    
    seteuid(getuid());
    ob = clone_object(str);
    if (!objectp(ob))
    {
	notify_fail("You can not clone: " + str + "\n");
	return 0;
    }

    say(QCTNAME(this_interactive()) + " fetches @@clone_message:" +
	WIZ_CMD_NORMAL + "|" + file_name(ob) +
	"@@ from another dimension.\n");

    return ob;
}

nomask static int
clone(string str)
{
    object ob;
    int    num;
    string *argv;

    CHECK_SO_JUNIOR;
    log_usage("clone (" + str + ")");

    if (!stringp(str))
    {
	notify_fail("Clone what object ?\n");
	return 0;
    }

    argv = explode(str, " ");

    switch (argv[0])
    {
    case "-i":
	ob = clone_ob(argv[1]);
	if (!ob)
	    return 0;
	ob->move(this_interactive(), 1);
	write("Ok.\n");
	break;

    case "-e":
	ob = clone_ob(argv[1]);
	if (!ob)
	    return 0;
	ob->move(environment(this_interactive()), 1);
	write("Ok.\n");
	break;

    default:
	ob = clone_ob(argv[0]);
	if (!ob)
	    return 0;

	num = (int)ob->move(this_interactive());
	switch (num)
	{
	case 0:
	    write("Ok.\n");
	    break;
	    
	case 1:
	    write("Too heavy for destination.\n");
	    break;
	    
	case 2:
	    write("Can't be dropped.\n");
	    break;
	    
	case 3:
	    write("Can't take it out of it's container.\n");
	    break;
	    
	case 4:
	    write("The object can't be inserted into bags etc.\n");
	    break;
	    
	case 5:
	    write("The destination doesn't allow insertions of objects.\n");
	    break;
	    
	case 6:
	    write("The object can't be picked up.\n");
	    break;
	    
	case 7:
	    write("Other (Error message printed inside move() function).\n");
	    break;
	    
	case 8:
	    write("Too big volume for destination.\n");
	    break;
	    
	default:
	    write("Strange, very strange error in move: " + num + "\n");
	    break;
	}

	ob->move(environment(this_interactive()), 1);
	break;
    }
    return 1;
}

/***************************************************************************
 * death - prevent yourself from death.
 *
 * Function created for this tool.
 */
nomask static int
death(string str)
{
    object junior_shadow;

    CHECK_SO_JUNIOR;
    log_usage("death (" + str + ")");

    if (str == "prevent")
    {
	if (this_interactive()->query_death_protection() == 1)
	{
	    write("You are already protected from Death.\n");
	    return 1;
	}

	if (catch(junior_shadow = clone_object(JUNIOR_SHADOW)))
	{
	    write("Could not load the Junior Shadow!\n");
	    return 1;
	}

	write("Trying to add the protecting shadow to you ...\n");
	return junior_shadow->shadow_me(this_interactive());
    }

    if (str == "allow")
    {
	if (!(this_interactive()->query_death_protection()))
	{
	    write("You are not protected from Death.\n");
	    return 1;
	}

	return this_interactive()->remove_death_protection();
    }

    notify_fail("There are two possible parameters for the command " +
	"\"death\": \"prevent\" and \"allow\". The first prevents you " +
	"from dying and the second re-allows you to die again. Currently " +
	"you are " + 
	((this_interactive()->query_death_protection() == 1) ? "" : "NOT ") +
	"protected from Death!\n");
    return 0;
}

/***************************************************************************
 * goto - teleport somewhere
 *
 * Function taken from WIZ_CMD_APPRENTICE and adjusted for use by juniors.
 */
nomask static int
goto(string dest)
{
    object ob;
    string name;

    CHECK_SO_JUNIOR;
    log_usage("goto (" + dest + ")");

    if (!stringp(dest))
    {
	notify_fail("Goto where ?\n");
	return 0;
    }

    if (!(ob = find_player(dest)))
	ob = find_living(dest);

    if ((objectp(ob)) && (objectp(environment(ob))))
    {
	this_interactive()->move_living("X", environment(ob));
	return 1;
    }

    /* no maps, not path-extention. A mortal doesn't have a path */

    if (sscanf(this_player()->query_real_name(), "%sjr", name) != 1)
    {
	notify_fail("Your name does not end with \"jr\". Impossible!\n");
	return 0;
    }

    /* A junior does not have a path, so we have to construct one using
     * the name of the wizard.
     */
    dest = FPATH(SECURITY->query_wiz_path(name), TPATH(name, dest));

    if (catch(this_interactive()->move_living("X", dest)))
    {
	notify_fail("Invalid monster name or file name: " + dest + "\n");
	return 0;
    }
    return 1;
}

/***************************************************************************
 * help - get help on a junior command.
 *
 * Function created for this tool.
 */
nomask static int
help(string str)
{
    string topic;

    CHECK_SO_JUNIOR;

    if (!stringp(str))
    {
	/* Handled by general help command. */
	return 0;
    }

    log_usage("help (" + str + ")");
    if (str == "junior")
    {
	str = "junior help";
    }

    if (sscanf(str, "junior %s", topic) != 1)
    {
	/* Handled by general help command. */
	return 0;
    }

    if (member_array(topic, m_indices(ALL_COMMANDS)) == -1)
    {
	write("Not a junior command      : " + topic + "\n");
	write("For all possible topics do: allcmd\n");
	write("Information on help       : help junior\n");
	return 1;
    }

    if (file_size(HELP_FILE) <= 0)
    {
	write("The junior help file is unfortunately not available.\n");
	return 1;
    }

    write(read_file(HELP_FILE, ALL_COMMANDS[topic][0],
	ALL_COMMANDS[topic][1]));

    return 1;
}

/***************************************************************************
 * home - teleport home.
 *
 * Function taken from WIZ_CMD_APPRENTICE and slightly adjusted for use
 * by juniors.
 */
nomask static int
home(string str)
{
    CHECK_SO_JUNIOR;
    log_usage("home (" + str + ")");

    if (str == "admin")
    {
	str = ADMIN_HOME;
    }
    else
    {
	if (!stringp(str))
	{
	    if (sscanf(this_player()->query_real_name(), "%sjr", str) != 1)
	    {
		write("Your name does not end with \"jr\"! Impossible!\n");
		return 1;
	    }
	}

	str = (string)SECURITY->wiz_home(str);
    }

    if (this_player()->move_living("X", str))
    {
	write("Unable to teleport there! No such workroom.\n");
	return 1;
    }

    return 1;
}

/***************************************************************************
 * list wizards - get a list of all present wizards
 *
 * Function created for this tool.
 */
nomask static int
list(string str)
{
    object *wizards;
    string *descs = ({ });
    string  domain;
    int     index;

    CHECK_SO_JUNIOR;

    if (str != "wizards")
    {
	notify_fail("List what? wizards?\n");
	return 0;
    }

    log_usage("list");
    wizards = filter(users(), &->query_wiz_level());

    if (!sizeof(wizards))
    {
	write("No wizards logged in.\n");
	return 1;
    }

    for (index = 0; index < sizeof(wizards); index++)
    {
	domain = SECURITY->query_wiz_dom(wizards[index]->query_real_name());
	domain = (strlen(domain) ? domain : "no domain");

	descs += ({ sprintf("%-12s (%2d, %-11s",
	    capitalize(wizards[index]->query_real_name()),
	    wizards[index]->query_wiz_level(), (domain + ")")) });
    }

    write("The following wizard" +
	((sizeof(wizards) == 1) ? " is" : "s are") + " logged in.\n");
    write(sprintf("%-*#s\n", 74, implode(sort_array(descs), "\n")));
    return 1;
}

/***************************************************************************
 * peace - make peace with all my enemies
 *
 * Function created for this tool.
 */
nomask static int
peace(string str)
{
    object *enemies;

    CHECK_SO_JUNIOR;

    if (stringp(str))
    {
	notify_fail("Peace with whom or what ?\n");
	return 0;
    }

    log_usage("peace");
    enemies = (object *)this_player()->query_enemy(-1);

    if (!pointerp(enemies))
    {
	enemies = ({ });
    }
    else
    {
	enemies -= ({ 0 });
    }

    if (!sizeof(enemies))
    {
	notify_fail("You are not fighting anyone.\n");
	return 0;
    }

    this_player()->stop_fight(enemies);
    enemies->stop_fight(this_player());

    write("You make peace with all your enemies.\n");
    return 1;
}

/***************************************************************************
 * stat - get information about an object
 *
 * Function taken from WIZ_CMD_APPRENTICE.
 */

#define STAT_PLAYER 0x1
#define STAT_LIVING 0x2
#define STAT_OBJECT 0x4

nomask static int
show_stat(object ob, int what)
{
    string str = "";

/*
    if (what & STAT_PLAYER)
	str += (string)ob->stat_playervars();
 */
    if (what & STAT_LIVING)
	str += (string)ob->stat_living();
    if (what & STAT_OBJECT)
	str += (string)ob->stat_object();

    write(str);
    return 1;
}

nomask static int
stat(string str)
{
    object ob;
    int    result;

    CHECK_SO_JUNIOR;
    log_usage("stat (" + str + ")");

    if (!stringp(str))
	return show_stat(this_interactive(), STAT_PLAYER | STAT_LIVING);

    if (ob = find_player(str))
	return show_stat(ob, STAT_PLAYER | STAT_LIVING);

    if ((ob = PARSE_LIST(str)) ||
	(ob = present(str, environment(this_interactive()))))
    {
	if (living(ob))
	    return show_stat(ob, STAT_LIVING);
	return show_stat(ob, STAT_OBJECT);
    }

    if (ob = find_living(str))
	return show_stat(ob, STAT_LIVING);

    if (ob = present(str, this_interactive()))
	return show_stat(ob, STAT_OBJECT);

    if (ob = SECURITY->finger_player(str))
    {
	result = show_stat(ob, STAT_PLAYER | STAT_LIVING);
	ob->remove_object(); /* We should destruct those phony players */
	return result;
    }

    notify_fail("No such thing.\n");
    return 0;
}

/***************************************************************************
 * tell - tell someone something
 *
 * Function taken from WIZ_CMD_APPRENTICE and slightly adjusted for use
 * by juniors.
 */
nomask static int
tell(string str)
{
    object ob;
    string who, msg;
    string name;
    string *names;
    int busy;

    CHECK_SO_JUNIOR;
    log_usage("tell (" + str + ")");

    if (!stringp(str) ||
	sscanf(str, "%s %s", who, msg) != 2)
    {
	notify_fail("Tell who what ?\n");
	return 0;
    }

    ob = find_player(lower_case(who));
    if (!objectp(ob))
    {
	notify_fail("No player with that name.\n");
	return 0;
    }

    if (!query_interactive(ob))
    {
	notify_fail(ob->query_The_name(this_player()) +
	    " is link dead right now.\n");
	return 0;
    }

    if (!(ob->query_wiz_level()))
    {
	notify_fail("You can only use tell to communicate with a wizard!\n");
	return 0;
    }

    busy = (int)ob->query_prop(WIZARD_I_BUSY_LEVEL);
    /* 152 = 8 + 16 + 128 */
    if (busy & 152)
    {
	write(break_string(capitalize(ob->query_real_name()) +
	    " seems busy at the moment. You can try again later or contact " +
	    "someone else.", 74));
	return 1;
    }

    name = this_player()->query_real_name();
    names = ob->query_prop(PLAYER_AS_REPLY_WIZARD);
    if (pointerp(names))
    {
	names = ({ name }) + (names - ({ name }) );
    }
    else
    {
	names = ({ name });
    }
    ob->add_prop(PLAYER_AS_REPLY_WIZARD, names);
    tell_object(ob, capitalize(name) + " tells you: " + msg + "\n");

    if (this_player()->query_option(OPT_ECHO))
    {
	write("You tell " + capitalize(ob->query_real_name()) + ": " + msg +
	    "\n");
    }
    else
    {
	write("Ok.\n");
    }

    return 1;
}

/***************************************************************************
 * update - update an object
 *
 * Function taken from WIZ_CMD_NORMAL and slightly adjusted for use by
 * juniors.
 */
nomask static int
update_ob(string str)
{
    object ob, *obs;
    int kick_master, i, error;
    string name;

    CHECK_SO_JUNIOR;
    log_usage("update (" + str + ")");

    if (!stringp(str))
    {
	ob = environment(this_player());
	str = MASTER_OB(ob);

	if (!ob)
	{
	    notify_fail("Update what object?\n");
	    return 0;
	}

	obs = filter(all_inventory(ob),
	    find_object(WIZ_CMD_NORMAL)->is_player);

	error = 0;
	for (i = 0; i < sizeof(obs); i++)
	{
	    if (obs[i]->query_default_start_location() == str)
	    {
		error = 1;        
		write("Cannot update the start location of "
		    + capitalize(obs[i]->query_real_name()) + ".\n");
	    }
	}

	if (error == 1)
	{
	    notify_fail("Update failed.\n");
	    return 0;
	}

	write("Updating environment.\n");
	say(QCTNAME(this_player()) + " updates the room.\n");

	/* Move all objects out of the room */
	for (i = 0; i<sizeof(obs); i++)
	{
	    obs[i]->move(obs[i]->query_default_start_location());
	}

	ob->remove_object();
	ob = find_object(str);
	if (ob)
	    SECURITY->do_debug("destroy", ob);

	for (i = 0; i < sizeof(obs); i++)
	    obs[i]->move(str);
	return 1;
    }
    else
    {
	if (sscanf(this_player()->query_real_name(), "%sjr", name) != 1)
	{
	    notify_fail("Your name does not end with \"jr\". Impossible!\n");
	    return 0;
	}

	/* A junior does not have a path, so we have to construct one using
	 * the name of the wizard.
	 */
	str = FPATH(SECURITY->query_wiz_path(name), TPATH(name, str));

	if (!strlen(str))
	{
	    notify_fail("Invalid file name: " + str + "\n");
	    return 0;
	}
	ob = find_object(str);
	if (!ob)
	{
	    notify_fail("No such object: " + str + "\n");
	    return 0;
	}
    }

    if (ob == find_object(SECURITY))
	kick_master = 1;

    if (ob != this_object())
    {
	ob->remove_object();
	ob = find_object(str);
	if (ob)
	    SECURITY->do_debug("destroy", ob);

	/* When updating the master object it must be reloaded at once
	   and from within the GD
	 */
	if (kick_master)
	{
	    write(SECURITY + " was updated and reloaded.\n");
	    get_dir("/*");
	}

	else if (!ob)
	    write(str + " will be reloaded at next reference.\n");
	else
	{
	    notify_fail("Object could not be updated.\n");
	    return 0;
	}
    }
    else
    {
	write(str + " will be reloaded at next reference.\n");
/*
 * This is a clonable object. No need to destruct it.
	write("Destructing this object.\n");
	destruct();
 */
	return 1;
    }
    return 1;
}

nomask static int
update(string str)
{
    int i;
    string *dir, *args, path, fpath, name;
    
    /* catch 'update' without args */

    if (!strlen(str))
	return update_ob(str);
    
    /* check if user specifies the -d option */

    args = explode(str, " ");
    if (args[0] != "-d")
	return update_ob(str);

    /* remove '-d' and keep pattern */

    str = args[1];

    if (sscanf(this_player()->query_real_name(), "%sjr", name) != 1)
    {
	notify_fail("Your name does not end with \"jr\". Impossible!\n");
	return 0;
    }

    /* A junior does not have a path, so we have to construct one using
     * the name of the wizard.
     */
    path = FPATH(SECURITY->query_wiz_path(name), TPATH(name, str));

    /* read directory */ 

    dir = get_dir(path);

    /* remove file pattern from end of path to expand path in update loop */

    args = explode(path, "/");
    args = exclude_array(args, sizeof(args) - 1, sizeof(args));
    path = implode(args, "/");
  
    /* move through all files returned by 'dir' */

    for (i = 0; i < sizeof(dir); i++)
    {
	/* get full filepath to objects */
	fpath = FPATH(path, TPATH(name, dir[i]));
	
	/* just update existing objects. skip anything names workroom.c to
	    avoid accidents */
	if (find_object(fpath) &&
	    (dir[i] != "workroom.c"))
	{
	    if (!update_ob(fpath)) /* standard update function */
		return 0;
	}
    }

    return 1;
}

/*
 * Function name: query_auto_load
 * Description  : In order to let the object autoload, this function should
 *                return its path-name.
 * Returns      : string - the path-name
 */
nomask public string
query_auto_load()
{
    return MASTER + ":" +
	((environment()->query_death_protection() == 1) ?
	"PREVENT" : "ALLOW");
}

/*
 * Function name: slow_init_arg
 * Description  : The original init_arg() is called _before_ the
 *                object is in the player, so we have to use this
 *                alarm to check wether the player is allowed to
 *                use the tool.
 * Arguments    : string - either "PREVENT" or "ALLOW"
 */
nomask static void
slow_init_arg(string arg)
{
    string name;
    object junior_shadow;

    if (sscanf(environment()->query_real_name(), "%sjr", name) != 1)
    {
	remove_object();
	return;
    }

    if (arg != "PREVENT")
    {
	return;
    }

    if (environment()->query_death_protection() == 1)
    {
	return;
    }

    if (catch(junior_shadow = clone_object(JUNIOR_SHADOW)))
    {
	environment()->catch_msg("Could not load the Junior Shadow!\n");
	return;
    }

    environment()->catch_msg("Trying to add the protecting shadow to " +
	"you ...\n");
    junior_shadow->shadow_me(environment());
}

/*
 * Function name: init_arg
 * Description  : When the player re-logs in, the death protection-shadow
 *                is added to the player if necessary.
 * Arguments    : string - either "PREVENT" or "ALLOW"
 */
nomask public void
init_arg(string arg)
{
    set_alarm(0.5, 0.0, &slow_init_arg(arg));
}

/*
 * Function name: get_help_index
 * Description  : The help file consists of a text file with the commands
 *                sequencially listed in it. The define ALL_COMMANDS in
 *                the header of this object contains the index to the
 *                entries in the help file. If you call this function, a
 *                list of supported commands is printed to this_player().
 *
 *                In order for the function to recognize the help entries,
 *                the entries must start with the following:
 *
 *                SYNTAX: name options
 *
 *                beginning on a new line and with one space between the
 *                word SYNTAX: (capitalized) and the name of the command.
 *                The command will be recognized as the first word after
 *                SYNTAX: and will continue till a new entry is found or
 *                the file ends.
 *
 *                CAVEAT: If two entries in the help file have exactly the
 *                same header-line, the second one will be disregarded and
 *                if they are successive in the file, the first one, will
 *                return length 0 lines. All other entries will be printed
 *                correctly though.
 */
nomask public void
get_help_index()
{
    string *help_file;
    string *entries;
    string command;
    int    index;

    if (file_size(HELP_FILE) <= 0)
    {
	write("Junior help file not found at path: " + HELP_FILE + "\n");
	return;
    }

    help_file = explode(read_file(HELP_FILE), "\n");

    if (sizeof(entries = regexp(help_file, (ENTRY_HEADER + ".*"))) <= 1)
    {
	write("No entries detected in: " + HELP_FILE + "\n");
	return;
    }

    write("Junior tool commands help index:\n");
    write("Command      Begin   #lines\n\n");

    while(sizeof(entries) > 1)
    {
	if (sscanf(entries[0], (ENTRY_HEADER + "%s"), command) != 1)
	{
	    write("Illegal line in help file (line " +
		member_array(entries[0], help_file) + "):\n");
	    write(entries[0] + "\n");
	    return;
	}

	index = member_array(entries[0], help_file);

	write(sprintf("%-12s %5d   %6d\n", explode(command, " ")[0], index,
	    (member_array(entries[1], help_file) - index)));

	entries -= ({ entries[0] });
    }
}

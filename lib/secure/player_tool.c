/*
 * /secure/player_tool.c
 *
 * This object can be used by the helpers of the playerarch to access
 * playerfiles. It is autoloadable.
 *
 * /Mercade, April 5 1995
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <filepath.h>
#include <macros.h>
#include <std.h>
#include <stdproperties.h>

#define PRIVATE		 ("/private")
#define PRIVATE_DIR(p)	 ((p) + PRIVATE)
#define PLAYERS		 ("/players")
#define PLAYERS_DIR(p)	 (PRIVATE_DIR(p) + PLAYERS)
#define BACKUP		 ("/player_tool")
#define BACKUP_DIR	 (PLAYERS + BACKUP)
#define PREDEATH	 (".predeath")
#define PREDEATH_FILE(p) (PLAYER_FILE(p) + PREDEATH) 

#define CHECK_SO_ACCESS if (!valid_access()) return 0

/*
 * Global variable. This variable is not saved.
 *
 * alphabet - array with the letters of the alphabet.
 */
private static string *alphabet = explode(ALPHABET, "");

/*
 * Prototypes.
 */
static nomask int playerget(string str);
static nomask int playerput(string str);
static nomask int playerrestore(string str);

/*
 * Function name: create_object
 * Description  : Constructor. Called when this object is created.
 */
nomask void
create_object()
{
    set_name("tool");
    add_name("playertool");
    add_name("player_tool");
    set_pname("tools");
    set_adj("player");

    set_short("player tool");
    set_pshort("player tools");

    set_long(break_string("It is a player tool that can be used to copy " +
	"playerfiles from the secured directory of the playerfiles to the " +
	"directory players in your private directory. Later, the modified " +
	"files can be put back. There is a help-page on all supported " +
	"commands. Apart from the archwizards and keepers, only people who " +
	"have been granted special access to this tool can use this tool.",
	75) + "\nThe commands supported are:\n" +
"    playerget     - copy a playerfile to your private directory\n" +
"    playerput     - copy a playerfile back from your private directory\n" +
"    playerrestore - copy a playerfile from its last predeath state\n");

    remove_prop(OBJ_I_VALUE);
    remove_prop(OBJ_I_VOLUME);
    remove_prop(OBJ_I_WEIGHT);

    add_prop(OBJ_I_NO_STEAL,    1);
    add_prop(OBJ_I_NO_TELEPORT, 1);
    add_prop(OBJ_S_WIZINFO,
	"Examine the player tool for information. /Mercade.\n");

    setuid();
    seteuid(getuid());
}

/*
 * Function name: init
 * Description  : This function is called when someone 'comes close' to
 *                this object to add the necessary commands to him/her.
 */
nomask public void
init()
{
    ::init();

    add_action(playerget,     "playerget");
    add_action(playerput,     "playerput");
    add_action(playerrestore, "playerrestore");
}

/*
 * Function name: valid_access
 * Description  : This function checks whether the currently interactive
 *                player is allowed to execute commands on this tool.
 * Returns      : int 1/0 - true if the player may execute commands.
 */
static nomask int
valid_access()
{
    string name;

    /* Sanity check. Only truely interactive people can use this tool. */
    if (this_player() != this_interactive())
    {
	notify_fail("Invalid interactive player. No access.\n");
	return 0;
    }

    /* Wizard must carry the tool in order to use it. */
    if (environment() != this_interactive())
    {
	notify_fail("You are not carrying the player tool. No access.\n");
	return 0;
    }

    name = this_interactive()->query_real_name();

    /* Arches and keepers have access. */
    if (SECURITY->query_wiz_rank(name) >= WIZ_ARCH)
    {
	return 1;
    }

    /* AoP team members may use it too. */
    if (SECURITY->query_team_member("aop", name))
    {
	return 1;
    }

    /* This tool will self-destruct in 5 seconds ... */
    set_alarm(5.0, 0.0, remove_object);

    notify_fail("You are neither a member of the administration, nor a " +
	"registered helper to the playerarch, so you may not use this tool.\n");
    return 0;
}

/*
 * Function name: valid_filename
 * Description  : This function tests whether the filename of the playerfile
 *                contains only valid characters, i.e. letters of the
 *                alphabet.
 * Arguments    : string name - the name to test.
 * Returns      : int 1/0 - success/failure.
 */
static nomask int
valid_filename(string name)
{
    if (!strlen(name))
    {
	return 0;
    }

    return (!sizeof(explode(name, "") - alphabet));
}

/*
 * Function name: backup_playerfile
 * Descriptions : Copies an original playerfile into the player tool backup
 *                directory before overwriting it. It uses sequential numbers
 *                to avoid overwriting any files.
 * Arguments    : string
 * Returns      : int 1/0 - success/failure.
 */
nomask static int
backup_playerfile(string str)
{
    string backup;
    string file;
    int number = 1;

    backup = BACKUP_DIR + "/" + str + ".o.";

    if (file_size(BACKUP_DIR) != -2)
    {
	if (!mkdir(BACKUP_DIR))
	{
	    notify_fail("Failed to create backup directory " + BACKUP_DIR + "\n");
	    return 0;
        }

	write("Backup directory created " + BACKUP_DIR + "\n");
    }

    while(file_size(backup + number) > 0)
    {
	number++;
    }

    file = PLAYER_FILE(str) + ".o";
    backup = backup + number;
    if (!rename(file, backup))
    {
	notify_fail("Failed to write backup file " + backup + "\n");
	return 0;
    }

    write("Current playerfile backed up to " + backup + "\n");
    return 1;
}

/*
 * Function name: playerget
 * Description  : This function can be used to transfer/copy a playerfile
 *                from the secure directory for the playerfiles to the
 *                private directory of the wizard.
 * Arguments    : string str - the name of the player whose file to copy.
 * Returns      : int 1/0 - success/failure.
 */
nomask static int
playerget(string str)
{
    string name;
    string path;
    string file;
    string buffer;

    CHECK_SO_ACCESS;

    if (!strlen(str))
    {
	notify_fail("Syntax: playerget <name>\n");
	return 0;
    }

    str = lower_case(str);

    if (!valid_filename(str))
    {
	notify_fail("Not a valid playername '" + capitalize(str) +
	    "' since not all characters are letters.\n");
	return 0;
    }

    file = PLAYER_FILE(str) + ".o";
    if (file_size(file) <= 0)
    {
	notify_fail("No player named " + capitalize(str) + " found.\n");
	return 0;
    }

    name = this_interactive()->query_real_name();
    /* The arch-helpers should not be interested in wizards files. Arches
     * or keepers may do so though.
     */
    if (SECURITY->query_wiz_rank(str) &&
	(SECURITY->query_wiz_rank(name) < WIZ_ARCH))
    {
	notify_fail("No reason to playerget a wizards file.\n");
	return 0;
    }

    path = SECURITY->query_wiz_path(name);
    /* See if the target directory exists. */
    if (file_size(PLAYERS_DIR(path)) != -2)
    {
	/* See if the private directory exists. */
	if (file_size(PRIVATE_DIR(path)) != -2)
	{
	    /* Try to create the private directory. */
	    if (!mkdir(PRIVATE_DIR(path)))
	    {
		notify_fail("Failed to create directory " +
		    RPATH(PRIVATE_DIR(path)) + "\n");
		return 0;
	    }

	    write("Created directory " + RPATH(PRIVATE_DIR(path)) + "\n");
	}

	/* Try to create the players directory in the private directory. */
	if (!mkdir(PLAYERS_DIR(path)))
	{
	    notify_fail("Failed to create directory " +
		RPATH(PLAYERS_DIR(path)) + "\n");
	    return 0;
	}

	write("Created directory " + RPATH(PLAYERS_DIR(path)) + "\n");
    }

    /* Since there is no copy-efun, we have to read the playerfile first. */
    buffer = read_file(file);
    if ((!stringp(buffer)) ||
	(!strlen(buffer)))
    {
	notify_fail("Failed to read " + RPATH(file) + "\n");
	return 0;
    }

    /* Before we try to save the buffer, see whether the target is
     * available.
     */
    file = PLAYERS_DIR(path) + "/" + str + ".o";
    switch(file_size(file))
    {
    case -2:
	notify_fail("Target " + RPATH(file) + " is a directory.\n");
	return 0;

    case -1:
    case  0:
	break;

    default:
	if (!rm(file))
	{
	    notify_fail("Failed to remove previous copy " + RPATH(file) + "\n");
	    return 0;
	}
    }

    if (!write_file(file, buffer))
    {
	notify_fail("Failed to write " + RPATH(file) + "\n");
	return 0;
    }

    write("Playerfile from " + capitalize(str) + " copied to " +
	RPATH(file) + "\n");
    return 1;
}

/*
 * Function name: playerput
 * Description  : This function can be used to transfer/copy a playerfile
 *                from the private directory of a wizard to the secure
 *                directory for the playerfiles.
 * Arguments    : string str - the name of the player whose file to copy.
 * Returns      : int 1/0 - success/failure.
 */
nomask static int
playerput(string str)
{
    string name;
    string file;
    string buffer;

    CHECK_SO_ACCESS;

    if (!strlen(str))
    {
	notify_fail("Syntax: playerput <name>\n");
	return 0;
    }

    name = this_interactive()->query_real_name();
    str = lower_case(str);

    if (!valid_filename(str))
    {
	notify_fail("Not a valid player name '" + capitalize(str) +
	    "' since not all characters are letters.\n");
	return 0;
    }

    /* The arch-helpers should not be interested in wizards files. Arches
     * or keepers may do so though.
     */
    if (SECURITY->query_wiz_rank(str) &&
	(SECURITY->query_wiz_rank(name) < WIZ_ARCH))
    {
	notify_fail("No reason to playerput a wizards file.\n");
	return 0;
    }

    if (objectp(find_player(str)))
    {
	notify_fail("The player " + capitalize(str) + " is logged in. It is " +
	    "useless to playerput now.\n");
	return 0;
    }

    setuid();
    seteuid(getuid());

    file = PLAYERS_DIR(SECURITY->query_wiz_path(name)) + "/" + str + ".o";
    if (file_size(file) <= 0)
    {
	notify_fail("File " + RPATH(file) + " not found.\n");
	return 0;
    }

    /* Since there is no copy-efun, we have to read the playerfile first. */
    buffer = read_file(file);
    if ((!stringp(buffer)) ||
	(!strlen(buffer)))
    {
	notify_fail("Failed to read " + RPATH(file) + "\n");
	return 0;
    }

    file = PLAYER_FILE(str) + ".o";

    /* If there is a file, make a proper backup. */
    if (file_size(file) > 0)
    {
        if (!backup_playerfile(str))
        {
            return 0;
        }
    }

    if (!write_file(file, buffer))
    {
	write("Failed to write " + file + "\n");
	return 1;
    }

    write("Playerfile from " + capitalize(str) +
	" copied back to the playerfiles directory.\n");
    return 1;
}

/*
 * Function name: playerrestore
 * Description  : This function can be used to transfer/copy a playerfile
 *                from the private directory of a wizard to the secure
 *                directory for the playerfiles.
 * Arguments    : string str - the name of the player whose file to copy.
 * Returns      : int 1/0 - success/failure.
 */
nomask static int
playerrestore(string str)
{
    string name;
    string file;
    string buffer;

    CHECK_SO_ACCESS;

    if (!strlen(str))
    {
	notify_fail("Syntax: playerrestore <name>\n");
	return 0;
    }

    name = this_interactive()->query_real_name();
    str = lower_case(str);

    if (!valid_filename(str))
    {
	notify_fail("Not a valid player name '" + capitalize(str) +
	    "' since not all characters are letters.\n");
	return 0;
    }

    /* The arch-helpers should not be interested in wizards files. Arches
     * or keepers may do so though.
     */
    if (SECURITY->query_wiz_rank(str) &&
	(SECURITY->query_wiz_rank(name) < WIZ_ARCH))
    {
	notify_fail("No reason to playerput a wizards file.\n");
	return 0;
    }

    if (objectp(find_player(str)))
    {
	notify_fail("The player " + capitalize(str) + " is logged in. It is " +
	    "useless to playerrestore now.\n");
	return 0;
    }

    setuid();
    seteuid(getuid());

    file = PREDEATH_FILE(str) + ".o";
    if (file_size(file) <= 0)
    {
	notify_fail("File " + RPATH(file) + " not found.\n");
	return 0;
    }

    write("Found " + RPATH(file) + " dated " + ctime(file_time(file)) + ".\n");

    /* Since there is no copy-efun, we have to read the playerfile first. */
    buffer = read_file(file);
    if ((!stringp(buffer)) ||
	(!strlen(buffer)))
    {
	notify_fail("Failed to read " + RPATH(file) + "\n");
	return 0;
    }

    /* If there is a file, make a proper backup. */
    file = PLAYER_FILE(str) + ".o";
    if (file_size(file) > 0)
    {
        if (!backup_playerfile(str))
        {
            return 0;
        }
    }

    if (!write_file(file, buffer))
    {
	write("Failed to write " + file + "\n");
	return 1;
    }

    write("Playerfile from " + capitalize(str) +
	" copied back to the last known predeath state.\n");
    return 1;
}

/*
 * Functio name: query_auto_load
 * Description : This function is called to test whether this object is
 *               autoloadable.
 * Returns     : string - the path to this object.
 */
public nomask string
query_auto_load()
{
    return MASTER + ":";
}

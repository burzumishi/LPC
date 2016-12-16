/*
 * exp_convert.c
 *
 * Created by Mercade @ Genesis, March 17 1998.
 *
 * This object will convert all playerfiles in the game with respect to the
 * amount of experience that is accumulated within the guild stats. It takes
 * the guild-experience out of the total experience. As friendly gesture, we
 * never remove more experience than the player has in combat experience.
 *
 * The program first converts all playerfiles that have been secured in a
 * special directory. Then it loops over all playerfiles in the normal save
 * directories. Players that are logged in or linkdead will be forced to
 * quit before they are processed.
 */

#pragma no_clone
#pragma no_inherit
#pragma strict_types

#include <ss_types.h>
#include <std.h>

/*
 * Prototypes.
 */
nomask static string player_filename(string name);
nomask static string saved_filename(string name);

/*
 * Global variables.
 */
static private object   actor;
static private string   *files;
static private int      alphabet;
static private int      doing_saved;

#define LOOP_SIZE      (100)
#define ALPHABET       ("abcdefghijklmnopqrstuvwxyz")

/*
 * Function name: player_filename
 * Description  : This function return the filename of the playerfile.
 * Arguments    : string name - the name of the player to convert.
 * Returns      : string - the filename.
 */
nomask static string
player_filename(string name)
{
    return ("/players/" + name[0..0] + "/" + name);
}

/*
 * Function name: saved_filename
 * Description  : This function returns the filename of the playerfile if
 *                that playerfile is secured.
 * Arguments    : string name - the name of the player to convert.
 * Returns      : string - the filename.
 */
nomask static string
saved_filename(string name)
{
    return ("/players/saved/" + name);
}

/*
 * Function name: change_file
 * Description  : This function will parse one playerfile.
 * Arguments    : string name - the name of the player.
 */
nomask static void
change_file(string name)
{
    string  filename;
    mapping playerfile;
    object  player;
    int     index;
    int     sum = 0;

    /* If the player is logged in, it is useless to alter the playerfile.
     * Therefore we first force him to quit.
     */
    if (objectp(player = find_player(name)) &&
	!doing_saved)
    {
        tell_object(player, "\nYour playerfile will be adapted now.\n");
        tell_object(player, "After 10 seconds, you can log in again.\n");
        catch(player->quit());

        if (objectp(player))
        {
            catch(player->remove_object());
            SECURITY->do_debug("destruct", player);
        }
    }

    seteuid(getuid());

    filename = (doing_saved ? saved_filename(name) : player_filename(name));
    playerfile = restore_map(filename);

    /* This must be true for true playerfiles. */
    if (playerfile["name"] != name)
    {
	return;
    }

    /* Player didn't make it through the login sequence. Delete. */
    if (playerfile["exp_points"] == 0)
    {
        rm(filename + ".o");
        return;
    }

    /* Take the combat experience out of the quest experience. */
    playerfile["exp_points"] -= playerfile["exp_combat"];
    playerfile["exp_general"] = 0;

    /* Quest experience cannot become negative, though. */
    if (playerfile["exp_points"] < 5000)
    {
        playerfile["exp_points"] = 5000;
    }

    /* Count the total amount of real experience. */
    index = -1;
    while(++index < SS_NO_EXP_STATS)
    {
        sum += playerfile["acc_exp"][index];
    }

    /* Adjust the combat experience. */
    playerfile["exp_combat"] = sum - playerfile["exp_points"];

    /* However, the combat experience may not become negative.  Let us
     * be nice and not remove quest experience. So, the stats are raised.
     */
    if (playerfile["exp_combat"] < 0)
    {
        sum = playerfile["exp_combat"] / -6;
        playerfile["exp_combat"] = 0;

        index = -1;
        while(++index < SS_NO_EXP_STATS)
        {
            playerfile["acc_exp"][index] += sum;
        }
    }

    /* Save the playerfile. */
    save_map(playerfile, filename);
}

/*
 * Function name: change_directory
 * Description  : This function loops over a certain number of files in
 *                one directory. It uses alarms between series to prevent
 *                eval cost problems.
 */
nomask static void
change_directory()
{
    int index = -1;
    int size = ((sizeof(files) > LOOP_SIZE) ? LOOP_SIZE : sizeof(files));

    while(++index < size)
    {
        sscanf(files[index], "%s.o", files[index]);
        change_file(files[index]);
    }

    if (sizeof(files) > LOOP_SIZE)
    {
    	tell_object(actor, ("Last file: " + files[size - 1] + "\n"));
    	files = files[LOOP_SIZE..];
    	set_alarm(2.0, 0.0, change_directory);
    	return;
    }

    if (doing_saved)
    {
    	tell_object(actor, "Finished secured players.\n");
	doing_saved = 0;
    	alphabet = -1;
    }
    else
    {
    	tell_object(actor, "Finished letter " + ALPHABET[alphabet..alphabet] +
            ".\n");
    }

    alphabet++;
    if (alphabet >= strlen(ALPHABET))
    {
        tell_object(actor, "Done converting experience.\n");
        alphabet = 0;
        files = ({ });
        destruct();
        return;
    }

    files = get_dir("/players/" + ALPHABET[alphabet..alphabet] + "/*.o");
    set_alarm(2.0, 0.0, change_directory);
}

/*
 * Function name: change_experience
 * Description  : Call this function to initiate the experience changing.
 * Returns      : int 1/0 - success/failure.
 */
nomask public int
change_experience()
{
    if (SECURITY->query_wiz_rank(this_interactive()->query_real_name()) <
        WIZ_ARCH)
    {
    	write("This may only be called by archwizards and keepers.\n");
    	return 0;
    }

    if (objectp(actor))
    {
    	write("The module is already active. Wait until it is done.\n");
    	return 0;
    }

    write("Started changing playerfiles.\n");
    write("First section: secured playerfiles.\n");

    setuid();
    seteuid(getuid());

    actor = this_interactive();
    doing_saved = 0;
    files = get_dir("/players/saved/*.o");
    change_directory();

    return 1;
}

/*
 * Function name: change_player
 * Description  : Call this manually to change the file of one existing
 *                player. This will only work on playerfiles that are in
 *                the "real" player directory.
 * Arguments    : string name - the name of the player to change.
 * Returns      : int 1/0 - success/failure.
 */
nomask public int
change_player(string name)
{
    if (SECURITY->query_wiz_rank(this_interactive()->query_real_name()) <
        WIZ_ARCH)
    {
    	write("This may only be called by archwizards and keepers.\n");
    	return 0;
    }

    name = lower_case(name);
    if (!SECURITY->exist_player(name))
    {
        write("No player named \"" + name + "\".\n");
        return 0;
    }

    /* Initialise so that active players are changed. */
    doing_saved = 0;

    /* Do it! */
    change_file(name);

    return 1;
}

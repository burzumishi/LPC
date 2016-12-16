/*
 * bit_change.c
 *
 * This is the master module of the bit changer. In order to operate this
 * program you must alter the bit_change.h header file accourdingly and have
 * this module run with a root or archwizards (++) euid.
 *
 * The program first converts all playerfiles that have been secured in a
 * special directory. Then it loops over all playerfiles in the normal save
 * directories. Players that are logged in or linkdead will have the bits
 * changed in them and they are then forced to save. For all players that are
 * not logged in the playerfiles are altered. If a bit-bank is completely
 * cleared, it is removed from the player.
 */

#pragma no_clone
#pragma no_inherit
#pragma strict_types

#include <std.h>
#include "bit_change.h"

/*
 * Prototypes.
 */
nomask static string player_filename(string name);
nomask static string saved_filename(string name);

/*
 * Global variables.
 */
static private string   *files;
static private function file;
static private int      alphabet;
static private int      source_num = SECURITY->query_domain_number(DOMAIN);
static private int      destination_num = source_num;
static private mapping  player;
static private int      *bit_wizlist;
static private int      *bit_bitlist;
static private mixed    change = CHANGE;
static private int      doing_saved;
static private string   source = DOMAIN;
static private string   destination = DOMAIN;

#define ACTIVE_SAVED   (saved_filename)
#define ACTIVE_PLAYERS (player_filename)
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
 * Function name: change_present_player
 * Description  : This function will convert the bits in a player that is
 *                logged in.
 * Arguments    : string name - the name of the player to convert.
 */
nomask static void
change_present_player(string name)
{
    object player = find_player(name);
    string result = sprintf("%11-s IN:", capitalize(player->query_real_name()));
    int    changed = 0;

    foreach(mixed *line: change)
    {
    	switch(line[INDEX_COMMAND])
    	{
    	case "remove":
    	    /* If the bit is set, remove it. */
    	    if (player->test_bit(destination, line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT]))
    	    {
                seteuid(destination);
    	        player->clear_bit(line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT]);
    	        result += " C" + line[INDEX_SOURCE_GROUP] + ":" + line[INDEX_SOURCE_BIT];
    	        changed = 1;
    	    }
    	    break;

    	case "moveall":
    	    /* If the destination bit is set, remove it first. */
    	    if (player->test_bit(destination, line[INDEX_DEST_GROUP], line[INDEX_DEST_BIT]))
    	    {
                seteuid(destination);
    	        player->clear_bit(line[INDEX_DEST_GROUP], line[INDEX_DEST_BIT]);
		result += " C" + line[INDEX_DEST_GROUP] + ":" + line[INDEX_DEST_BIT];
		changed = 1;
    	    }
    	    /* Intentional fallthrough. */

    	case "moveset":
	    /* If the source bit is set, remove it and set the target. */
    	    if (player->test_bit(source, line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT]))
    	    {
                seteuid(source);
    	        player->clear_bit(line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT]);
		result += " C" + line[INDEX_SOURCE_GROUP] + ":" + line[INDEX_SOURCE_BIT];
                seteuid(destination);
    	        player->set_bit(line[INDEX_DEST_GROUP], line[INDEX_DEST_BIT]);
		result += " S" + line[INDEX_DEST_GROUP] + ":" + line[INDEX_DEST_BIT];
		changed = 1;
    	    }
    	    break;

    	case "copyset":
	    /* If the source bit is set, don't remove it, but set the target. */
    	    if (player->test_bit(source, line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT]))
    	    {
                seteuid(destination);
                player->set_bit(line[INDEX_DEST_GROUP], line[INDEX_DEST_BIT]);
		result += " S" + line[INDEX_DEST_GROUP] + ":" + line[INDEX_DEST_BIT];
		changed = 1;
    	    }
    	    break;

    	default:
    	    break;
    	}
    }

    seteuid(getuid());

    if (changed)
    {
    	player->save_me(1);
	write_file(LOGFILE, result + "\n");
    }
}

/*
 * Function name: set_bit
 * Description  : This function sets a bit in the player.
 * Arguments    : int group - the group to set the bit in (0 - 4).
 *                int bit   - the bit to set in the group (0 - 19).
 *                int domain_num - the number of the domain to handle.
 */
nomask static void
set_bit(int group, int bit, int domain_num)
{
    int index;
    int num;

    num = domain_num * 5 + group;
    index = member_array(num, bit_wizlist);
    if (index < 0)
    {
	bit_wizlist += ({ num });
	bit_bitlist += ({ 0 });
	index = sizeof(bit_wizlist) - 1;
    }
    bit_bitlist[index] |= (1 << bit);
}

/*
 * Function name: clear_bit
 * Description  : This function clears a bit in the player.
 * Arguments    : int group - the group to clear the bit in (0 - 4).
 *                int bit   - the bit to clear in the group (0 - 19).
 *                int domain_num - the number of the domain to handle.
 */
nomask static void
clear_bit(int group, int bit, int domain_num)
{
    int index;
    int num;

    num = domain_num * 5 + group;
    index = member_array(num, bit_wizlist);
    if (index < 0)
    {
	return;
    }

    bit_bitlist[index] &= (0xFFFFFFFF - (1 << bit));
}

/*
 * Function name: test_bit
 * Description  : This function tests a bit in the player.
 * Arguments    : int group - the group to test the bit in (0 - 4).
 *                int bit   - the bit to test in the group (0 - 19).
 *                int domain_num - the number of the domain to handle.
 * Returns      : int 1/0 - true if the bit was set.
 */
nomask static int
test_bit(int group, int bit, int domain_num)
{
    int index;
    int num;

    num = domain_num * 5 + group;
    index = member_array(num, bit_wizlist);
    if (index < 0)
    {
	return 0;
    }

    if (bit_bitlist[index] & (1 << bit))
    {
	return 1;
    }
    else
    {
	return 0;
    }
}

/*
 * Function name: pack_bits
 * Description  : This function stuffs the used bits into an array to save
 *                memory. This array is then stored to disk. It only stores
 *                the bit modules that are not clear.
 */
nomask static void
pack_bits()
{
    int index = -1;
    int size = sizeof(bit_wizlist);

    player["bit_savelist"] = ({ });
    while(++index < size)
    {
    	if (bit_bitlist[index])
    	{
	    player["bit_savelist"] +=
		({ bit_wizlist[index] | (bit_bitlist[index] << 12) });
	}
    }
}

/*
 * Function name: unpack_bits
 * Description  : This function takes the stuffed bits out of their array
 *                and puts them in the correct arrays.
 */
nomask static void
unpack_bits()
{
    if (!sizeof(player["bit_savelist"]))
    {
	player["bit_savelist"] = ({ });
	bit_wizlist = ({ });
	bit_bitlist = ({ });
    }
    else
    {
    	bit_wizlist = map(player["bit_savelist"], &operator(&)(, 0xFFF));
    	bit_bitlist = map(player["bit_savelist"], &operator(>>)(, 12));
    }
}

nomask static void
change_file(string name)
{
    string result = sprintf("%11-s SF:", capitalize(name));
    int    changed = 0;

    /* If the player is logged in, it is useless to alter the playerfile.
     * Therefore we have the player object altered.
     */
    if (objectp(find_player(name)) &&
	!doing_saved)
    {
	change_present_player(name);
	return;
    }

    seteuid(getuid());

    player = restore_map(file(name));

    /* This must be true for true playerfiles. */
    if (player["name"] != name)
    {
	return;
    }

    unpack_bits();

    foreach(mixed *line: change)
    {
	switch(line[INDEX_COMMAND])
	{
	case "remove":
	    /* If the bit is set, remove it. */
    	    if (test_bit(line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT], destination_num))
    	    {
    	        clear_bit(line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT], destination_num);
    	        result += " C" + line[INDEX_SOURCE_GROUP] + ":" + line[INDEX_SOURCE_BIT];
    	        changed = 1;
    	    }
    	    break;

	case "moveall":
    	    /* If the destination bit is set, remove it first. */
    	    if (test_bit(line[INDEX_DEST_GROUP], line[INDEX_DEST_BIT], destination_num))
    	    {
    	        clear_bit(line[INDEX_DEST_GROUP], line[INDEX_DEST_BIT], destination_num);
		result += " C" + line[INDEX_DEST_GROUP] + ":" + line[INDEX_DEST_BIT];
		changed = 1;
    	    }
            /* Intentional fallthrough. */

	case "moveset":
	    /* If the source bit is set, remove it and set the target. */
    	    if (test_bit(line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT], source_num))
    	    {
    	        clear_bit(line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT], source_num);
		result += " C" + line[INDEX_SOURCE_GROUP] + ":" + line[INDEX_SOURCE_BIT];
    	        set_bit(line[INDEX_DEST_GROUP], line[INDEX_DEST_BIT], destination_num);
		result += " S" + line[INDEX_DEST_GROUP] + ":" + line[INDEX_DEST_BIT];
		changed = 1;
    	    }
    	    break;

	case "copyset":
	    /* If the source bit is set, don't remove it, but set the target. */
    	    if (test_bit(line[INDEX_SOURCE_GROUP], line[INDEX_SOURCE_BIT], source_num))
    	    {
    	        set_bit(line[INDEX_DEST_GROUP], line[INDEX_DEST_BIT], destination_num);
		result += " S" + line[INDEX_DEST_GROUP] + ":" + line[INDEX_DEST_BIT];
		changed = 1;
    	    }
    	    break;

    	default:
    	    break;
    	}
    }

    if (changed)
    {
        seteuid(getuid());
	pack_bits();
	save_map(player, file(name));
	write_file(LOGFILE, result + "\n");
    }
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
    	if (wildmatch("*.o", files[index]))
    	{
    	    sscanf(files[index], "%s.o", files[index]);
    	    change_file(files[index]);
    	}
    }

    if (sizeof(files) > LOOP_SIZE)
    {
    	tell_object(find_player(ALLOWED),
    	    ("Last file: " + file(files[size - 1]) + "\n"));
    	files = files[LOOP_SIZE..];
    	set_alarm(2.0, 0.0, change_directory);
    	return;
    }

    if (doing_saved)
    {
    	tell_object(find_player(ALLOWED), "Finished secured players.\n");
    	file = ACTIVE_PLAYERS;
	doing_saved = 0;
    	alphabet = -1;
    }
    else
    {
    	tell_object(find_player(ALLOWED), "Finished letter " +
    	    ALPHABET[alphabet..alphabet]+ ".\n");
    }

    alphabet++;
    if (alphabet >= strlen(ALPHABET))
    {
    	tell_object(find_player(ALLOWED), "Done converting bits.\n");
    	file = 0;
    	alphabet = 0;
    	files = ({ });
    	destruct();
    	return;
    }

    seteuid(getuid());
    files = get_dir("/players/" + ALPHABET[alphabet..alphabet] + "/*.o");
    set_alarm(2.0, 0.0, change_directory);
}

/*
 * Function name: change_bits
 * Description  : Call this function to initiate the bit changing.
 * Returns      : int 1/0 - success/failure.
 */
nomask public int
change_bits()
{
    if (this_interactive()->query_real_name() != ALLOWED)
    {
    	write("This function may only be called by " + capitalize(ALLOWED) +
    	    ".\n");
    	return 0;
    }

    if (functionp(file))
    {
    	write("The module is already active. Wait until it is done.\n");
    	return 0;
    }

#ifdef SOURCE
    source = SOURCE;
    source_num = SECURITY->query_domain_number(SOURCE);
#endif SOURCE

    write("Started changing bits.\n");
    write("First section: secured playerfiles.\n");

    setuid();
    seteuid(getuid());

    file = ACTIVE_SAVED;
    doing_saved = 1;
    files = get_dir("/players/saved/*");
    change_directory();

    return 1;
}

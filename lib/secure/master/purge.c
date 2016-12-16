/*
 * /secure/master/purge.c
 *
 * After some time there may be a lot of idle characters around, wasting
 * the space and also taking up names that other people might want to
 * use.
 *
 * Wizards will not be purged. They have to be demoted first. Also, wizards
 * may be absent for a while. From each purge, a log is made. It is
 * recommented to check this log and to see which wizards are idle so long,
 * so they may be manually removed. Also, it lists files that are not normal
 * player characters.
 *
 * Some of these functions may seem a little robust and there indeed are a
 * lot of checks in this object, but then again, purging is serious business.
 */

#pragma no_clone
#pragma no_include
#pragma save_binary
#pragma strict_types

#include <std.h>

#define MAX_PURGE       (50)
#define PURGE_LOG       ("/syslog/log/PURGE")
#define MINIMUM_LIMIT   (6048000) /* seconds, 70 days */
#define SECS_PER_DAY    (86400)   /* seconds          */
#define ONE_DAY         (43200)   /* heartbeats       */
#define ONE_HOUR        (1800)    /* heartbeats       */
#define ONE_MINUTE      (30)      /* heartbeats       */
#define STAT(s)         (pow(itof(s), 0.3333333333))
#define ALPHABET        ("abcdefghijklmnopqrstuvwxyz")
#define PLAYER_FILES(c) ("/players/" + (c) + "/*")

/*
 * Static global variables.
 *
 * These global variables are private to this object. They have nothing to
 * do with the playerfiles that are being checked.
 */
private static object  purger;
private static string *purge_files;
private static string  purged_wizards;
private static string  purged_mortals;
private static string  strange_files;
private static int     num_wizards;
private static int     num_mortals;
private static int     num_strange;
private static int     purge_index;
private static int     tested_files;

/*
 * Non-static global variables.
 *
 * These global variables are a part of the playerfiles that are being
 * checked.
 */
private string  name;       /* the name of the player             */
private int     login_time; /* the last time the player logged in */
private int     age_heart;  /* the of the player in heartbeats    */
private int    *acc_exp;    /* the acc-experience of the player   */

/*
 * Function name: report_purge_done
 * Description  : When the purging is over, we make the final report and
 *                write that to disk. Naturally we also notify the purger
 *                if (s)he is still arround.
 */
static nomask void
report_purge_done()
{
    if (strlen(purged_wizards))
    {
	write_file(PURGE_LOG, "Found " + num_wizards + " wizard(s) that " +
	    "have been idle too long.\nThey have not been purged. You shall " +
	    "have to demote them manually:\n\n" + purged_wizards + "\n");
    }
    else
    {
	write_file(PURGE_LOG, "No overly idle wizards found this time.\n\n");
    }

    if (strlen(purged_mortals))
    {
	write_file(PURGE_LOG, "Found " + num_mortals + " mortal(s) that " +
	    "have have been idle too long.\nThey have been purged:\n\n" +
	    purged_mortals + "\n");
    }
    else
    {
	write_file(PURGE_LOG,
	    "No overly idle mortal players found this time.\n\n");
    }

    if (strlen(strange_files))
    {
	write_file(PURGE_LOG, "In addition, we have found " + num_strange +
	    " files that are no normal playerfiles:\n\n" + strange_files);
    }

    if (objectp(purger))
    {
	tell_object(purger, "PURGER >> Done testing " + tested_files +
	    " files.\nPURGER >> Purged " + num_mortals + " mortal " +
	    "player(s).\nPURGER >> Found " + num_wizards + " overly idle " +
	    "wizard(s).\nPURGER >> The wizards have not been purged. Demote " +
	    "them first.\n");
    }

    destruct();
}

/*
 * Function name: last_date
 * Description  : Returns the date the player was last logged in. It only
 *                returns month, day and year.
 * Arguments    : int    - the time.
 * Returns      : string - the time.
 */
static nomask string
last_date(int last_time)
{
    string tmp = ctime(last_time);

    return extract(tmp, 4, 10) + extract(tmp, 20, 23);
}

/*
 * Function name: player_age
 * Description  : Returns the age of the player in the largest denomination.
 * Returns      : string - the age.
 */
static nomask string
player_age()
{
    /* measure in days. */
    if (age_heart > ONE_DAY)
    {
	return ((age_heart / ONE_DAY) + " d");
    }

    /* measure in hours. */
    if (age_heart > ONE_HOUR)
    {
	return ((age_heart / ONE_HOUR) + " h");
    }

    /* measure in minutes */
    return ((age_heart / ONE_MINUTE) + " m");
}

/*
 * Function name: player_average
 * Description  : Returns the average stat of the player.
 * Returns      : int - the average stat.
 */
static nomask int
player_average()
{
    return (ftoi(STAT(acc_exp[0]) + STAT(acc_exp[1]) + STAT(acc_exp[2]) +
	STAT(acc_exp[3]) + STAT(acc_exp[4]) + STAT(acc_exp[5])) / 6);
}

/*
 * Function name: purge_one
 * Description  : This function actually tests a file and purges it if
 *                necessary.
 * Arguments    : string filename - the name to test and possibly purge.
 */
static nomask void
purge_one(string filename)
{
    string my_name;
    int    level;

    /* Not a playerfile. */
    if (extract(filename, -2) != ".o")
    {
	if ((extract(filename, -6) == ".o.org") &&
	    ((time() - file_time(PLAYER_FILE(filename))) > MINIMUM_LIMIT ))
	{
	    rm(PLAYER_FILE(filename));
	    num_wizards++;
	    purged_wizards += sprintf("%-11s %-13s\n", capitalize(filename),
		last_date(file_time(PLAYER_FILE(filename))));
	}

	strange_files += (filename + "\n");
	num_strange++;
	return;
    }

    /* Reset the information we use. */
    name = 0;
    login_time = 0;

    /* This should be the name of the player. */
    my_name = extract(filename, 0, -3);

    /* If we cannot restore it, it is not a true playerfile. */
    if (!restore_object(PLAYER_FILE(my_name)))
    {
	strange_files += (filename + "\n");
	num_strange++;
	return;
    }

    /* Apparently not a true playerfile. The saved name does not match the
     * filename.
     */
    if (name != my_name)
    {
	strange_files += (filename + "\n");
	num_strange++;
	return;
    }

    /* Player has been logged in recently, so hands off. */
    if ((time() - login_time) < MINIMUM_LIMIT)
    {
	return;
    }

    /* If the player is a wizard, report him but don't hurt him. */
    if (level = SECURITY->query_wiz_rank(name))
    {
	purged_wizards += sprintf("%-11s %-10s %-13s\n", capitalize(name),
	    WIZ_RANK_NAME(level), last_date(login_time));
	num_wizards++;
	return;
    }

    /* If a player is old or has a lot of experience, he can be idle just
     * a little longer than other people.
     */
    level = player_average();
    if ((time() - login_time) <	(MINIMUM_LIMIT +
	    (((level / 3) + (age_heart / ONE_DAY)) * SECS_PER_DAY)))
    {
	return;
    }

    /* Oke.. Player has been idle too long. Lets purge him/her */
    rm(PLAYER_FILE(filename));

    purged_mortals += sprintf("%-11s %-13s %3d %4s\n", capitalize(name),
	last_date(login_time), level, player_age());
    num_mortals++;
}

/*
 * Function name: delayed_purge
 * Description  : We use alarms to prevent problems with eval-cost when
 *                testing a lot of players.
 */
static nomask void
delayed_purge()
{
    int index = -1;
    int limit;

    /* No files left to purge. Lets check the next character. */
    if (!sizeof(purge_files))
    {
	if (++purge_index >= strlen(ALPHABET))
	{
	    report_purge_done();
	    return;
	}

	tell_object(purger, "Purging next letter (" +
	    extract(ALPHABET, purge_index, purge_index) + ").\n");
	purge_files = get_dir(PLAYER_FILES(extract(ALPHABET, purge_index,
	    purge_index))) - ({ ".", ".." });
    }

    limit = ((sizeof(purge_files) > MAX_PURGE) ?
	MAX_PURGE : sizeof(purge_files));
    tested_files += limit;

    while(++index < limit)
    {
	purge_one(purge_files[index]);
    }

    purge_files = purge_files[limit..];

    set_alarm(2.0, 0.0, "delayed_purge");
}

/*
 * Function name: purge
 * Description  : Clean out old and idle playercharacters. You need to be
 *                an archwizard or keeper to be allowed to execute the
 *                purge command or you may find your own character purged.
 */
public nomask void
purge_players()
{
    if (previous_object() != find_object(WIZ_CMD_ARCH))
    {
	write("You are not allowed to use this command.\n");
    }

    if (file_size(PURGE_LOG) > 0)
    {
	rename(PURGE_LOG, (PURGE_LOG + ".old"));
    }

    purger         = this_player();
    purge_files    = ({ });
    num_wizards    = 0;
    num_mortals    = 0;
    num_strange    = 0;
    tested_files   = 0;
    purge_index    = -1;
    purged_wizards = "";
    purged_mortals = "";
    strange_files  = "";

    write("Oke. Purge started. You shall be notified when the purge is " +
	"done. There may be a little lag since we have to use alarms.\n");

    write_file(PURGE_LOG, "Purge executed by " +
	capitalize(purger->query_real_name()) + ".\nDate: " +
	ctime(time()) + ".\n\n");

    delayed_purge();
}

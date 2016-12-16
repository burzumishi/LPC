/* 
 * /secure/master/siteban.c
 *
 * This module maintains the list of banned sites.
 */

#define SITEBAN_LOG "SITEBANS"

#define SITEBAN_TYPE    0
#define SITEBAN_WIZARD  1
#define SITEBAN_DATE    2
#define SITEBAN_COMMENT 3

/*
 * Global variable in the save-file:
 *
 * sitebans = ([ (string) ipmask : ({ (int) type,
 *                                    (string) wizname,
 *                                    (int) date,
 *                                    (string) comment,
 *                                 })
 *            ])
 */
private mapping sitebans;

/*
 * Global variables that are not saved.
 *
 * sitebans_nologin = (string *)ipmasks
 * sitebans_nonew   = (string *)ipmasks
 */
private static string *sitebans_nologin;
private static string *sitebans_nonew;

/*
 * Function name: filter_sitebans
 * Description  : Filter to find all sitebans of a particular type.
 * Arguments    : string ipmask - the ip mask to test.
 *                int type - the type to find.
 * Returns      : int 1/0 - matched / did not match.
 */
static int
filter_sitebans(string ipmask, int type)
{
    return (sitebans[ipmask][SITEBAN_TYPE] == type);
}

/*
 * Function name: init_sitebans
 * Description  : Called at boot-time, and whenever the sitebans list has been
 *                updated to create separate lists of sites that have been
 *                banned nonew or nologin.
 */
static void
init_sitebans()
{
    if (!mappingp(sitebans))
    {
        sitebans = ([ ]);
    }

    sitebans_nologin = filter(m_indices(sitebans),
        &filter_sitebans(, SITEBAN_NOLOGIN));
    sitebans_nonew = filter(m_indices(sitebans),
        &filter_sitebans(, SITEBAN_NONEW));
}

/*
 * Function name: check_newplayer
 * Description  : Call this to find out whether a certain site is locked out.
 * Arguments    : string ipnumber - the ip number to check.
 * Returns      : int - 0 - all connections are allowed.
 *                      1 - no connection is allowed.
 *                      2 - connections are allowed, but no new players.
 */
public int
check_newplayer(string ipnumber)
{
    if (!strlen(ipnumber))
        return 0;

    if (sizeof(filter(sitebans_nologin, &wildmatch(, ipnumber))))
        return SITEBAN_NOLOGIN;

    if (sizeof(filter(sitebans_nonew, &wildmatch(, ipnumber))))
        return SITEBAN_NONEW;

    return 0;
}

/*
 * Function name: add_siteban
 * Description  : Add a new siteban.
 * Arguments    : string cmd - either nonew or nologin.
 *                string ipmask - the ip number to ban.
 *                string reason - the reason for banning.
 */
static int
add_siteban(string cmd, string ipmask, string reason)
{
    string name;
    int    type;

    if (pointerp(sitebans[ipmask]))
    {
        notify_fail("There already is a ban on " + ipmask + " for " +
            ((sitebans[ipmask][SITEBAN_TYPE] == SITEBAN_NOLOGIN) ?
            "all logins" : "new characters") + ". To alter the ban type " +
            "please remove the ban first.\n");
        return 0;
    }

    name = getwho();
    type = ((cmd == "nologin") ? SITEBAN_NOLOGIN : SITEBAN_NONEW);
    sitebans[ipmask] = ({ type, name, time(), reason });
    log_file(SITEBAN_LOG, sprintf("%s %-7s %-15s %-11s %s\n",
        ctime(time()), cmd, ipmask, capitalize(name), reason));
    save_master();
    init_sitebans();

    write("Installing siteban " + cmd + " on: " + ipmask + "\n");
    return 1;
}

static int
list_siteban(string wildcards)
{
    mixed   data;
    string *bans;
    int     index;
    int     size;
    string  comment;

    switch(wildcards)
    {
    case "all":
        bans = sitebans_nologin + sitebans_nonew;
        break;

    case "nologin":
        bans = sitebans_nologin;
        break;

    case "nonew":
        bans = sitebans_nonew;
        break;

    default:
        bans = sitebans_nologin + sitebans_nonew;
        bans = filter(bans, &wildmatch(wildcards, ));
    }

    size = sizeof(bans);
    switch(size)
    {
    case 0:
        write("There are no sitebans found (of this type).\n");
        return 1;

    case 1:
        data = sitebans[bans[0]];
        write("Siteban " + ((data[SITEBAN_TYPE] == SITEBAN_NOLOGIN) ?
            "all logins" : "new characters") + " on ip mask " + bans[0] +
            ".\n" + ctime(data[SITEBAN_DATE]) + " by " +
            capitalize(data[SITEBAN_WIZARD]) + ".\nReason: " +
            data[SITEBAN_COMMENT] + "\n");
        return 1;
    }

    bans = sort_array(bans);
    index = -1;
    while(++index < size)
    {
        data = sitebans[bans[index]];
        comment = ((strlen(data[SITEBAN_COMMENT]) > 37) ?
            (data[SITEBAN_COMMENT][..33] + " ..") : data[SITEBAN_COMMENT]);
        write(sprintf("%1s %-15s %-6s %-4s %-11s %s\n",
            ((data[SITEBAN_TYPE] == SITEBAN_NOLOGIN) ? "A" : "N"), bans[index],
            MAKE_DATE(data[SITEBAN_DATE]), DATE_YEAR(data[SITEBAN_DATE]),
            capitalize(data[SITEBAN_WIZARD]), comment));
    }
    write("A = All logins banned, N = No new logins\n");

    return 1;    
}

/*
 * Function name: remove_siteban
 * Description  : Called to remove the ban of a single site.
 * Arguments    : string ipmask - the ip number to remove.
 * Returns      : int 1/0 success/failure.
 */
static int
remove_siteban(string ipmask)
{
    string name;

    if (!pointerp(sitebans[ipmask]))
    {
        notify_fail("There is no current siteban of the site \"" + ipmask +
            "\".\n");
        return 0;
    }

    write("Removing ban of " + ipmask + " for " +
        ((sitebans[ipmask][SITEBAN_TYPE] == SITEBAN_NOLOGIN) ?
        "all logins" : "new characters") + ".\n");
    write("Installed by " + capitalize(sitebans[ipmask][SITEBAN_WIZARD]) +
        " on " + ctime(sitebans[ipmask][SITEBAN_DATE]) + ".\n");
    write("Reason: " + sitebans[ipmask][SITEBAN_COMMENT] + "\n");

    name = getwho();
    m_delkey(sitebans, ipmask);
    log_file(SITEBAN_LOG, sprintf("%s %-7s %-15s %-11s\n",
        ctime(time()), "removed", ipmask, capitalize(name)));
    save_master();
    init_sitebans();

    return 1;
}

/*
 * Function name: siteban
 * Description  : Main implementation of the command "siteban" for arches.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
public int
siteban(string str)
{
    string *words;

    if (!CALL_BY(WIZ_CMD_ARCH))
	return 0;

    if (!stringp(str))
        str = "list all";

    if (!mappingp(sitebans))
        sitebans = ([ ]);

    words = explode(str, " ");
    switch(words[0])
    {
    case "list":
        if (sizeof(words) == 1)
            words += ({ "all" });

        if (sizeof(words) != 2)
	{
            notify_fail("Syntax: list nologin / nonew / <wildcards>\n");
            return 0;
	}

        return list_siteban(words[1]);

    case "add":
        words = words[1..];
        /* Intentional fallthrough. */

    case "nologin":
    case "nonew":
        if (sizeof(words) < 3)
        {
            notify_fail("Syntax: [add] nologin/nonew <ipmask> <reason>\n");
            return 0;
        }
        return add_siteban(words[0], words[1], implode(words[2..], " "));

    case "remove":
        if (sizeof(words) != 2)
        {
            notify_fail("Syntax: remove <ipmask>\n");
            return 0;
        }
        return remove_siteban(words[1]);

    default:
        notify_fail("No such argument to \"siteban\".\nSyntax: siteban " +
            "list nologin / nonew / <wildcards>\n        siteban [add] " +
            "nologin / nonew <ipmask> <reason>\n        siteban remove " +
            "<ipmask>\n");
        return 0;
    }
}

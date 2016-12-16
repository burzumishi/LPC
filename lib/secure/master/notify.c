/*
 * /secure/master/notify.c
 *
 * This module processes a signal that is generated whenever a player
 * connects or disconnects. It is used to handle notifying wizards about
 * people logging on or off. The second use is keeping some statistics
 * about the number of people in the game.
 *
 * The notify-flag wizards have can take severel levels, binary coded:
 *
 *  1 - A - Notify of all events;
 *  2 - W - Only notify about wizards;
 *  4 - L - Only notify of Lords and higher people;
 *  8 - D - Only notify of people of the same domain as you;
 * 16 - I - Get the ip address of the player in the message;
 * 32 - X - Do not add messages about linkdeath;
 * 64 - B - Try to block notification. Higher wizards still see you.
 */

#include "/sys/flags.h"
#include "/sys/log.h"
#include "/sys/stdproperties.h"
#include "/sys/time.h"

#define NOTIFY_ALL     (1)
#define NOTIFY_WIZARDS (2)
#define NOTIFY_LORDS   (4)
#define NOTIFY_DOMAIN  (8)
#define NOTIFY_IP     (16)
#define NOTIFY_NO_LD  (32)
#define NOTIFY_BLOCK  (64)

#define NOTIFY_LOGIN   (0)
#define NOTIFY_LOGOUT  (1)
#define NOTIFY_LINKDIE (2)
#define NOTIFY_REVIVE  (3)
#define NOTIFY_SWITCH  (4)
#define NOTIFY_REAL_LD (5)

#define GRAPH_PERIODS (24)
#define GRAPH_QUEUE   (WIZ_KEEPER)
#define GRAPH_WIZARDS (WIZ_KEEPER + 1)
#define GRAPH_ALL     (WIZ_KEEPER + 2)
#define GRAPH_SIZE    (WIZ_KEEPER + 3)
#define GRAPH_ROWS    (20)

#define MIN_LOG_AGE   (1728000)

/*
 * Global variables. They are saved in the KEEPERSAVE.
 *
 * The graph_players is a list of 24 arrays. Each array contains the average
 * number of mortals, apprentices, pilgrims, ..., lords, administrators,
 * players in the queue, wizards and total players that have been connected
 * over that particular hour. The arches and keepers are grouped together in
 * this sense as 'admin'.
 *
 * ({ ({ mortals, apprentice, ...., lords, admin, queue, wizards, players }),
 *    ({ ..... }),
 *    .....
 * })
 *
 * The graph_reboots is a list of 20 integers that contain the last so many
 * times the game was rebooted.
 */
private mixed graph_players;
private int  *graph_reboots;

/*
 * Global variables. They are not saved.
 *
 * The graph_tmp_players is a list of arrays just like graph_players. Its
 * contents are the same, though it is not as large. Because we probe a few
 * times per hour, we use this to stack the data until it is processed.
 *
 * The wiz_notify_map is a list of arrays with the individual notification
 * names of the wizards.
 */
private static mixed graph_tmp_players = ({ });
private static mapping wiz_notify_map = ([ ]);

/*
 * Function name: reset_graph
 * Descripiton  : This function will reset the graph related variables.
 */
static void
reset_graph()
{
    int index = -1;

    graph_reboots = allocate(GRAPH_ROWS);
    graph_players = allocate(GRAPH_PERIODS);
    graph_tmp_players = ({ });

    while(++index < GRAPH_PERIODS)
    {
        graph_players[index] = allocate(GRAPH_SIZE);
    }
}

/*
 * Function name: display_player_graph
 * Description  : This function can be used to display a graph about the
 *                number of players in the game.
 * Arguments    : int type - the type to display.
 */
static void
display_player_graph(int type)
{
    int index;
    int row;
    int max;

    /* Find the highest value among the data. */
    index = -1;
    max = 0;
    while(++index < GRAPH_PERIODS)
    {
        if (graph_players[index][type] > max)
        {
            max = graph_players[index][type];
        }
    }

    if (!max)
    {
        write("No data of that type has been gathered yet. It is not " +
            "possible to print this graph.\n");
        return;
    }

    /* Print the graph lines. */
    row = 21;
    while(--row >= 1)
    {
        if (!(row % 4))
        {
            write(sprintf("%4d |", ((row * max) / GRAPH_ROWS)));
        }
        else if (row == 1)
        {
            write("num. |");
        }
        else
        {
            write("     |");
        }

        index = -1;
        while(++index < GRAPH_PERIODS)
        {
            if (((graph_players[index][type] * GRAPH_ROWS) / max) >= row)
            {
                write(" ##");
            }
            else
            {
                write("   ");
            }
        }

        write("\n");
    }

    write("-----+---------------------------------------------------------" +
          "---------------\n");
    write("Hour |");
    index = -1;
    max = (((time() + 3600) % 86400) / 3600) + 1;
    while(++index < GRAPH_PERIODS)
    {
        write(sprintf("%3d", ((max + index) % GRAPH_PERIODS)));
    }
    write("\n");
}

/*
 * Function name: display_reboot_graph
 * Description  : This function will display the reboot-graph.
 */
static void
display_reboot_graph()
{
    int    index;
    int    max;
    int    *tmp;
    int    row;
    string word;
    int    secs;

    /* Fill the array with the uptimes and calculate the longest uptime. */
    tmp = graph_reboots + ({ time() });
    index = -1;
    max = 0;
    while(++index < GRAPH_ROWS)
    {
        /* Here we check for the existance of the data-element since we don't
         * want uptimes to appear as having started at 'time() == 0'
         */
        tmp[index] = tmp[index + 1] - tmp[index];
        if ((graph_reboots[index]) &&
            (tmp[index] > max))
        {
            max = tmp[index];
        }
    }

    /* Get rid of the current time in the array again. */
    tmp = tmp[..(GRAPH_ROWS - 1)];

    /* Get the time denomination, days, hours or minutes. */
    if ((max / 86400) >= 5 )
    {
        secs = (86400 * GRAPH_ROWS);
        word = "d";
    }
    else if ((max / 3600) >= 5)
    {
        secs = (3600 * GRAPH_ROWS);
        word = "h";
    }
    else
    {
        secs = (60 * GRAPH_ROWS);
        word = "m";
    }

    /* Print the graph lines. */
    row = 21;
    while(--row >= 1)
    {
        if (!(row % 4))
        {
            write(sprintf("%3d%1s |", ((row * max) / secs), word));
        }
        else if (row == 1)
        {
            write("time |");
        }
        else
        {
            write("     |");
        }

        index = -1;
        while(++index < GRAPH_ROWS)
        {
            if (((tmp[index] * GRAPH_ROWS) / max) >= row)
            {
                write(" #");
            }
            else
            {
                write("  ");
            }
        }

        index = GRAPH_ROWS - row;
        write(sprintf("     %1s %s\n", ALPHABET[index..index],
                      (graph_reboots[index] ?
                       ctime(graph_reboots[index]) : "-")));
    }

    write("-----+-----------------------------------------------" +
          "-------------------------\n");
    write("     | a b c d e f g h i j k l m n o p q r s t       " +
          "the reboot times\n");
}

/*
 * Function name: graph
 * Description  : This function will display the user graphs.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
int
graph(string str)
{
    int type;

    /* May only be called from the apprentice soul. */
    if (!CALL_BY(WIZ_CMD_APPRENTICE))
    {
        return 0;
    }

    if (!strlen(str))
    {
        str = "all";
    }

    switch(lower_case(str))
    {
    case "all":
        display_player_graph(GRAPH_ALL);
        return 1;

    case "queue":
        display_player_graph(GRAPH_QUEUE);
        return 1;

    case "reset":
        if (query_wiz_rank(this_player()->query_real_name()) < WIZ_ARCH)
        {
            notify_fail("Only arches and keepers may reset the graph.\n");
            return 0;
        }
        
        reset_graph();
        write("Graph reset.\n");
        return 1;
        
    case "reboots":
        display_reboot_graph();
        return 1;

    case "wizards":
        display_player_graph(GRAPH_WIZARDS);
        return 1;

    case "keeper":
    case "keepers":
        str = WIZNAME_ARCH;

    default:
        if ((type = member_array(LANG_SWORD(str), WIZ_N)) > -1)
        {
            display_player_graph(WIZ_R[type]);
            return 1;
        }

        notify_fail("No such graph to display: \"" + str + "\".\n");
        return 0;
    }

    write("Fatal end of switch() in 'graph'! Please report this!\n");
    return 1;
}

/*
 * Function name: mark_graph_reboot
 * Description  : This function is called from start_boot() when the game
 *                is booted to mark the reboot. When the master is updated
 *                this function is not called.
 */
static void
mark_graph_reboot()
{
    /* Something is apparently wrong in the save-file. Reset the graph. */
    if ((sizeof(graph_reboots) != GRAPH_ROWS) ||
        (sizeof(graph_players) != GRAPH_PERIODS))
    {
        reset_graph();
    }

    /* Only update this value if the uptime of the previous driver was
     * longer than 15 minutes, 900 seconds.
     */
    if ((time() - graph_reboots[GRAPH_ROWS - 1]) > 900)
    {
        graph_reboots = graph_reboots[1..] + ({ time() });
    }

    /* Initialize the temporary variables. */
    graph_tmp_players = ({ });
}

/*
 * Function name: graph_process_data
 * Description  : This function is called once per hour to process the data
 *                gathered at the resets.
 */
static void
graph_process_data()
{
    int index;
    int size;
    int index2;
    int *tmp;
    int sum;

    if (!sizeof(graph_tmp_players))
    {
        return;
    }

    /* Compute the average over the samples. */
    tmp = allocate(GRAPH_SIZE);
    size = sizeof(graph_tmp_players);
    index = -1;
    while(++index < GRAPH_SIZE)
    {
        /* I want the average to be rounded upwards, not truncated, so we
         * start with half the value, making someone who logged in only half
         * an hour still count.
         */
        sum = (size / 2);
        index2 = -1;
        while(++index2 < size)
        {
            sum += graph_tmp_players[index2][index];
        }
        tmp[index] = (sum / size);
    }

    /* Add the averages to the graph data and reset the temporary stack. */
    graph_players = graph_players[1..] + ({ tmp });
    graph_tmp_players = ({ });
}

/*
 * Function name: probe_for_graph
 * Description  : This function is called from reset_master() to gather
 *                the necessary information about players.
 */
static void
probe_for_graph()
{
    int    *tmp;
    object *players;
    int    index;
    int    size;
    int    rank;

    tmp = allocate(GRAPH_SIZE);
    /* Get all real players. */
    players = filter(users(), &operator(==)(PLAYER_SEC_OBJECT) @
                     &function_exists("enter_game"));

    /* Count all mortals and wizards in their respective ranks. Note that
     * the keepers are counted with the arches.
     */
    index = -1;
    size = sizeof(players);
    while(++index < size)
    {
        rank = query_wiz_rank(players[index]->query_real_name());
        if (rank == WIZ_KEEPER)
        {
            rank = WIZ_ARCH;
        }

        tmp[rank]++;
    }

    /* Count all wizards, i.e. all non-mortals. */
    tmp[GRAPH_WIZARDS] = size - tmp[WIZ_MORTAL];

    /* Count all people. */
    tmp[GRAPH_ALL] = size;

    /* Find the number of people in the queue. */
    tmp[GRAPH_QUEUE] = QUEUE->query_queue();

    /* Add the information to the data stack. */
    graph_tmp_players += ({ tmp });

    /* Process the stacked data if this is the top of the hour, i.e. if the
     * number of seconds after the top of the hour is smaller than the reset
     * period RESET_TIME.
     */
    if ((time() % 3600) < ftoi(RESET_TIME))
    {
        graph_process_data();
    }
}

/*
 * Function name: notify_try_block
 * Description  : This filter is used when the player tries to block his/
 *                her logins. People with a higher rank will see you though,
 *                just as archwizards and keepers.
 * Arguments    : object player - the player who may want to see the login.
 *                int    rank   - the rank of the player who tries to block.
 * Returns      : int 1/0 - true if the player is allowed to see the player
 *                          who tries to block him/herself.
 */
public int
notify_try_block(object player, int rank)
{
    int wiz_rank = query_wiz_rank(player->query_real_name());

    return ((wiz_rank > rank) ||
            (wiz_rank >= WIZ_ARCH));
}

/*
 * Function name: update_wiz_notify
 * Description  : This function updates the individual notification status
 *                of the wizard by reloading his ~/.notify file.
 * Arguments    : string name - the (lower case) name of the wizard.
 */
public void
update_wiz_notify(string name)
{
    string str = read_file(query_wiz_path(name) + "/.notify");

    if (!strlen(str))
    {
        wiz_notify_map[name] = ({ });
        return;
    }

    /* We only take the first 20 entries so as not to waste too much
     * memory. 50 for arches and AoP.
     */
    wiz_notify_map[name] = (explode(str, "\n") - ({ "" }) )[..49];
}

/*
 * Function name: notify
 * Description  : Notify wizards who want to hear when someone leaves or
 *                enters the game.
 * Arguments    : object ob    - the player object entering or leaving;
 *                int    level - the notification status.
 *
 *                The notify level has several possible values:
 *
 *                0 - player logged in;
 *                1 - player logged out;
 *                2 - player linkdied;
 *                3 - player revived from linkdeath;
 *                4 - player switched terminals.
 */
public void
notify(object ob, int level)
{
    string name = ob->query_real_name();
    string message_text;
    string second_text;
    string log_text;
    string ip_name = query_ip_name(ob);
    string wizname;
    string log;
    object *players = users() - ({ 0, ob });
    string *names;
    string *seconds = this_object()->query_seconds(name);
    int    notify_lvl;
    int    size;
    int    rank = query_wiz_rank(name);
    string domain = query_wiz_dom(name);
    int    ld = (level >= NOTIFY_LINKDIE);

    switch(level)
    {
    case NOTIFY_LOGIN:
        log_text = sprintf("login  %s", ip_name); 
        message_text = "logged in.";
        break;

    case NOTIFY_LOGOUT:
        log_text = "quit";
        message_text = "logged out.";
        break;

    case NOTIFY_LINKDIE:
        if (ob->query_relaxed_from_combat())
        {
            log_text = "linkdeath";
            message_text = "link-died.";
        }
        else
        {
            log_text = "linkdeath in combat";
            message_text = "link-died in combat";
        }
        break;

    case NOTIFY_REVIVE:
        log_text = sprintf("revive %s", ip_name); 
        message_text = "revived from linkdeath";
        break;

    case NOTIFY_SWITCH:
        log_text = sprintf("switch %s", ip_name); 
        message_text = "reconnected.";
        break;

    case NOTIFY_REAL_LD:
        log_text = "actual linkdeath after combat";
        message_text = "link-died after combat";
        break;

    default:
        log_text = "unknown notification type: " + level;
        message_text = "did something unpredictable (level " + level + ").";
    }

#ifdef LOG_ENTER
    /* Cycle the log with the same day number every month. */
    log = "/syslog/log/" + LOG_ENTER + "." + TIME2FORMAT(time(), "mmdd");
    if ((file_size(log) > 0) &&
        ((file_time(log) + MIN_LOG_AGE) < time()))
    {
        rm(log);
    }

    write_file(log, sprintf("%s %-11s: %s\n", ctime(time()),
        capitalize(name), log_text));
#endif LOG_ENTER

    ip_name = (strlen(ip_name) ? (" (" + ip_name + ")") : "");

    /* First we filter the wizards. */
    players = filter(players, &->query_wiz_level());

    /* If the wizard tries to block notification, we filter those who are
     * still allowed to see the players action.
     */
    if (ob->query_notify() & NOTIFY_BLOCK)
    {
        players = filter(players, &notify_try_block(, rank));
    }

    foreach(object wizard: players)
    {
        /* Do not bother busy wizards. */
        if (wizard->query_prop(WIZARD_I_BUSY_LEVEL) & BUSY_F)
        {
            continue;
        }

        /* Cache the wizard notify information for speed. */
        wizname = wizard->query_real_name();
        if (!pointerp(names = wiz_notify_map[wizname]))
        {
            update_wiz_notify(wizname);
            names = wiz_notify_map[wizname];
        }

        second_text = "";
        /* Notify level == notification on wizards. */
        notify_lvl = wizard->query_notify();
        /* However, also check for notified mortals. */
        if (IN_ARRAY(name, names))
        {
            notify_lvl = NOTIFY_ALL | (notify_lvl & (NOTIFY_IP | NOTIFY_NO_LD));
        }
        /* For arches, also check seconds. */
        else if ((query_wiz_rank(wizname) >= WIZ_ARCH) &&
            sizeof(names = seconds & names))
        {
            notify_lvl = NOTIFY_ALL | (notify_lvl & (NOTIFY_IP | NOTIFY_NO_LD));
            foreach(string sname: names)
            {
                second_text += ":" + capitalize(sname);
            }
        }

        if (!notify_lvl ||
            (ld && (notify_lvl & NOTIFY_NO_LD)))
        {
            continue;
        }

        /* These are the different notify possibilities. */
        if ((notify_lvl & NOTIFY_ALL) ||
            (rank && (notify_lvl & NOTIFY_WIZARDS)) ||
            ((notify_lvl & NOTIFY_LORDS) && (rank >= WIZ_LORD)) ||
            ((notify_lvl & NOTIFY_DOMAIN) && (domain == query_wiz_dom(wizname))))
        {
            tell_object(wizard, "[" + capitalize(name) + second_text + " " +
                message_text +
                (((notify_lvl & NOTIFY_IP) && valid_query_ip(wizard)) ? ip_name : "") +
                "]\n");
        }
    }
}

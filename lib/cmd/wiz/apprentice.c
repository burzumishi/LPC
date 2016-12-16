/*
 * /cmd/wiz/apprentice.c
 *
 * This object holds the apprentice wizards commands. The soul has been split
 * over a few modules. Hence, the list of commands below only reflects the
 * commands actually coded in this module:
 * 
 * - adjdesc
 * - allcmd
 * - altitle
 * - applications
 * - apply
 * - bit
 * - club
 * - domainsanction (short: dsanction)
 * - finger
 * - gd
 * - gfinger
 * - goto
 * - graph
 * - gtell
 * - guild
 * - gwiz
 * - gwize
 * - home
 * - last
 * - localcmd
 * - metflag
 * - mudlist
 * - newhooks
 * - notify
 * - ranking
 * - regret
 * - review
 * - rsupport
 * - rwho
 * - sanction
 * - setmin
 * - setmmin
 * - setmmout
 * - setmout
 * - start
 * - title
 * - wizopt
 * - whereis
 * - whichsoul
 */

#pragma no_clone
#pragma no_inherit
#pragma strict_types
#pragma save_binary

inherit "/cmd/std/tracer_tool_base.c";

#include <cmdparse.h>
#include <composite.h>
#include <files.h>
#include <filter_funs.h>
#include <formulas.h>
#include <language.h>
#include <mail.h>
#include <macros.h>
#include <ss_types.h>
#include <std.h>
#include <stdproperties.h>
#include <time.h>
#include <options.h>
#include <living_desc.h>

#define CHECK_SO_WIZ if (WIZ_CHECK < WIZ_APPRENTICE) return 0; \
                     if (this_interactive() != this_player()) return 0
#define WIZARD_S_GOTO_SET "_wizard_s_goto_set"

/* Prototype for finger() because it is used in line() in communication.c */
nomask int finger(string str);

#include "/cmd/wiz/apprentice/communication.c"
#include "/cmd/wiz/apprentice/files.c"
#include "/cmd/wiz/apprentice/manual.c"
#include "/cmd/wiz/apprentice/people.c"

/* **************************************************************************
 * At creation, the save-file of the soul is restored.
 */
nomask void
create()
{
    setuid();
    seteuid(getuid());

    restore_object(WIZ_CMD_APPRENTICE);

    create_lines();
    update_commands();
}

/* **************************************************************************
 * Return a list of which souls are to be loaded.
 * The souls are listed in order of command search.
 */
nomask string *
get_soul_list()
{
    return ({ WIZ_CMD_APPRENTICE,
              MBS_SOUL });
}

/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
nomask string
get_soul_id()
{
    return WIZNAME_APPRENTICE;
}

/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
nomask mapping
query_cmdlist()
{
    mapping cmd = ([
        "adjdesc":"adjdesc",
        "allcmd":"allcmd",
#ifndef NO_ALIGN_TITLE
        "altitle":"altitle",
#endif NO_ALIGN_TITLE
        "applications":"applications",
        "apply":"apply",
        "audience":"audience",

        "bit":"bit",
        "busy":"busy",

        "cat":"cat_file",
        "cd":"cd",
        "club":"club",

        "dirs":"dirs",
        "domainsanction":"domainsanction",
        "dsanction":"domainsanction",

        "emote":"emote",
        ":":"emote",

        "finger":"finger",

        "gd":"gd_info",
        "gfinger":"gfinger",
        "graph":"graph",
        "goto":"goto",
#ifdef UDP_ENABLED
        "gtell":"gtell",
#endif UDP_ENABLED
        "guild":"guild",
        "guildtell":"guildtell",
#ifdef UDP_ENABLED
        "gwiz":"gwiz",
        "gwize":"gwiz",
#endif UDP_ENABLED

        "head":"head",
        "home":"home",

        "last":"last",
        "line":"line",
        "linee":"line",
        "lman":"lman",
        "localcmd":"localcmd",
        "ls":"list_files",

        "man":"man",
        "metflag":"metflag",
        "more":"more_file",
        "mpeople":"people",
#ifdef UDP_ENABLED
        "mudlist":"mudlist",
#endif UDP_ENABLED

        "newhooks":"newhooks",
        "next":"next",
        "notify":"notify",

        "people":"people",
        "popd":"popd",
        "pushd":"pushd",
        "pwd":"pwd",

        "ranking":"ranking",
        "regret":"regret",
        "review":"review",
#ifdef UDP_ENABLED
        "rsupport":"rsupport",
        "rwho":"rwho",
#endif UDP_ENABLED

        "sanction":"sanction",
        "setmin":"set_m_in",
        "setmmin":"set_mm_in",
        "setmmout":"set_mm_out",
        "setmout":"set_m_out",
        "sman":"sman",
        "start":"start",

        "tail":"tail_file",
        "tell":"tell",
        "title":"title",
        "tree":"tree",

        "wizopt":"wizopt",
        "whereis":"whereis",
        "whichsoul":"whichsoul",
        "wiz":"wiz",
        "wize":"wiz",
        "wsay":"wsay",
        ]);
    
    return cmd + query_line_cmdlist();
}

/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the
 * same order as in the function name list.
 * **************************************************************************/

/* **************************************************************************
 * adjdesc - display/add descriptional adjectives
 */
nomask int
adjdesc(string str)
{
    string *parts, *old;
    object pl;

    CHECK_SO_WIZ;

    if (str)
    {
        parts = explode(str + " ", " ");
        if (pl = find_player(parts[0]))
        {
            write(pl->query_name() + "'s 'nonmet name' is: " +
                  pl->query_nonmet_name() + "\n");
            return 1;
        }
        old = (string *)this_interactive()->query_adj(1);
        this_interactive()->remove_adj(old);
        if (!(this_interactive()->set_adj(parts)))
        {
            notify_fail("Too many words (max 2) or too long string " +
                        "(< 35 chars total).\n");
            this_interactive()->set_adj(old);
            return 0;
        }
    }

    write("Your 'nonmet name' is: " +
          this_interactive()->query_nonmet_name() + "\n");

    return 1;
}

/* **************************************************************************
 * allcmd - List all commands available to a player
 */

/*
 * Function name: print_soul_list
 * Description  : This function actually prints the commands linked to the
 *                souls in the wizard.
 * Arguments    : string *soul_list - the souls in this section.
 *                string  soul      - then soul the wizard wants to see, ""
 *                                    if (s)he wants to see all.
 */
static nomask void
print_soul_list(string *soul_list, string soul)
{
    int    index = -1;
    int    size = sizeof(soul_list);
    string soul_id;
    string *list;

    while(++index < size)
    {
        soul_id = soul_list[index]->get_soul_id();
        if (strlen(soul) &&
            (soul != soul_id))
        {
            continue;
        }

        write("----- " + capitalize(soul_id) + ":\n");
        list = m_indices(soul_list[index]->query_cmdlist());

        /* To print the list of this soul, don't list the lines. 
        if (soul == get_soul_id())
        {
            list -= m_indices(query_line_cmdlist());
	    } */

        if (sizeof(list))
        {
            write(break_string(implode(sort_array(list), ", "), 76) + "\n");
        }
    }
}

nomask int
allcmd(string soul)
{
    object ob;
    int    specific = 0;
    string *list;

    CHECK_SO_WIZ;

    if (stringp(soul))
    {
        list = explode(soul + " ", " ");
        if (sizeof(list) > 1)
        {
            soul = list[1];
            specific = 1;
        }
        if (list[0] == "me")
        {
            ob = this_interactive();
        }
        else
        {
            ob = find_living(list[0]);
            if (!objectp(ob) && sizeof(list) > 1)
            {
                notify_fail("Syntax: allcmd [name] [soul]\n");
                return 0;
            }
            else if (!objectp(ob))
            {
                specific = 1;
            }
        }
    }

    if (!objectp(ob))
    {
        ob = this_interactive();
    }

    if (!specific ||
        (soul == "base"))
    {
        /* We do not sort the local commands because their order is linked
         * to the order of their execution. The first command listed is
         * tested. This order also reflects the inventory of the player.
         * Objects first in the inventory are evaluated before objects later
         * in the inventory.
         */
        write("----- Base:\n");
        write(break_string(implode(ob->local_cmd(), ", "), 76) + "\n");
    }

    soul = (specific ? soul : "");

    print_soul_list(ob->query_wizsoul_list(), soul);
    print_soul_list(ob->query_tool_list(),    soul);
    print_soul_list(ob->query_cmdsoul_list(), soul);

    return 1;
}

/* **************************************************************************
 * altitle - display/change the alignment title
 */
#ifndef NO_ALIGN_TITLE
nomask int
altitle(string t)
{
    CHECK_SO_WIZ;

    if (!stringp(t))
    {
        write("Your alignment title is '" +
              this_interactive()->query_al_title() + "'.\n");
        return 1;
    }
    this_interactive()->set_al_title(t);
    write("Ok.\n");
    return 1;
}
#endif

/* **************************************************************************
 * applications - display applications to a domain
 */
nomask int
applications(string domain)
{
    CHECK_SO_WIZ;

    return SECURITY->list_applications(domain);
}

/* **************************************************************************
 * apply - apply to a domain
 */
nomask int
apply(string domain)
{
    CHECK_SO_WIZ;

    return SECURITY->apply_to_domain(domain);
}

/* **************************************************************************
 * bit - affect bits
 */
nomask int
bit(string args)
{
    int    argc;
    int    group;
    int    bit;
    int    index;
    int    index2;
    string *argv;
    string result;
    object player;

    CHECK_SO_WIZ;

    if (!stringp(args))
    {
        notify_fail("No argument to 'bit'. Check with the help text.\n");
        return 0;
    }
    argv = explode(args, " ");
    argc = sizeof(argv);

    if (argc < 3)
    {
        notify_fail("Syntax: bit <command> <player> <args>\n");
        return 0;
    }

    if (!objectp(player = find_player(argv[1])))
    {
        notify_fail("Player " + capitalize(argv[1]) + " is not logged in.\n");
        return 0;
    }

    if (argc == 5)
    {
        if (SECURITY->query_wiz_rank(this_player()->query_real_name()) <
            WIZ_ARCH)
        {
            notify_fail("You are not allowed to give a domain name as " +
                "argument. In order to change a bit administered by your " +
                "own domain, you can omit the domain argument.\n");
            return 0;
        }

        argv[2] = capitalize(argv[2]);
        if (SECURITY->query_domain_number(argv[2]) == -1)
        {
            notify_fail("There is no domain named '" + argv[2] + "'.\n");
            return 0;
        }

        if (!seteuid(argv[2]))
        {
            write("Failed to set euid to " + argv[2] + ".\n");
            return 1;
        }

        write("Euid changed to " + argv[2] + " for bit operation!\n");
        argv = exclude_array(argv, 2, 2);
        argc--;
    }

    switch(argv[0])
    {
    case "set":
        if (argc != 4)
        {
            write("Syntax error. Check with the help text.\n");
            return 1;
        }
        if ((sscanf(argv[2], "%d", group) != 1) ||
            (sscanf(argv[3], "%d", bit) != 1))
        {
            write("Syntax error. Check with the help text.\n");
            return 1;
        }
        if (!(player->set_bit(group, bit)))
        {
            write("You were not allowed to set bit " + group + ":" + bit +
                  " in " + capitalize(argv[1]) + ".\n");
            return 1;
        }
        write("Set bit " + group + ":" + bit + " in " + capitalize(argv[1]) +
              ".\n");
        return 1;

    case "clear":
        if (argc != 4)
        {
            write("Syntax error. Check with the help text.\n");
            return 1;
        }
        if ((sscanf(argv[2], "%d", group) != 1) ||
            (sscanf(argv[3], "%d", bit) != 1))
        {
            write("Syntax error. Check with the help text.\n");
            return 1;
        }
        if (!(player->clear_bit(group, bit)))
        {
            write("You were not allowed to clear bit " + group + ":" + bit +
                  " in " + capitalize(argv[1]) + ".\n");
            return 1;
        }
        write("Cleared bit " + group + ":" + bit + " in " +
              capitalize(argv[1]) + ".\n");
        return 1;

    case "list":
        if (argc != 3)
        {
            write("Syntax: bit list <player> <domain>\n");
            return 1;
        }

        argv[2] = capitalize(argv[2]);
        result = "";
        index = -1;
        while(++index < 5)
        {
            index2 = -1;
            while(++index2 < 20)
            {
                if (player->test_bit(argv[2], index, index2))
                {
                    result += " " + index + ":" + index2;
                }
            }
        }
        if (!strlen(result))
        {
            write(argv[2] + " has no bits set in " +
                  capitalize(argv[1]) + ".\n");
            return 1;
        }

        write(argv[2] + " has set bits:" + result + "\n");
        return 1;

    default:
        notify_fail("No subcommand '" + argv[0] + "' for the command bit.\n");
        return 0;
    }

    notify_fail("Should never happen: switch() error in then end of bit()\n");
    return 0;
}

/* **************************************************************************
 * club - manage information about clubs
 */
nomask int
club(string str)
{
    CHECK_SO_WIZ;

    return SECURITY->guild_command(str);
}

/* **************************************************************************
 * domainsanction - manage the domain sanction list
 */
nomask int
domainsanction(string str)
{
    CHECK_SO_WIZ;

    return SECURITY->domainsanction(str);
}

/* **************************************************************************
 * finger - display information about player/domain/wiztype
 */

/*
 * Function name: finger_domain
 * Description  : This function is called to display finger information about
 *                a domain.
 * Arguments    : string domain - the domain name.
 */
nomask static void
finger_domain(string domain)
{
    string *names;
    string name;
    string path;
    int    size;

    names = (string *)SECURITY->query_domain_members(domain);
    name = SECURITY->query_domain_lord(domain);
    if (strlen(name))
    {
        write("The Liege of " + domain + " is " + capitalize(name));
        names -= ({ name });
    }
    else
    {
        write("The domain " + domain + " has no Liege");
    }

    name = SECURITY->query_domain_steward(domain);
    if (strlen(name))
    {
        write(", " + capitalize(name) + " is the steward.\n");
        names -= ({ name });
    }
    else
    {
        write(".\n");
    }

    if (!sizeof(names))
    {
        write("The domain has no members.\n");
    }
    else
    {
        names = sort_array(map(names, capitalize));
        write("The domain has " + LANG_WNUM(sizeof(names)) + " member" +
              ((sizeof(names) == 1) ? "" : "s") + ": " +
              COMPOSITE_WORDS(names) + ".\n");
    }

    size = SECURITY->query_domain_max(domain) -
        sizeof(SECURITY->query_domain_members(domain));
    if (!size)
    {
        write("There are no vacancies.\n");
    }
    else
    {
        write("There " + ((size == 1) ? "is " : "are ") + LANG_WNUM(size) +
              ((size == 1) ? " vacancy" : " vacancies") + ".\n");
    }

    path = SECURITY->query_wiz_path(domain) + "/open/finger_info";
    if (file_size(path) > 0)
    {
        write("--------- Special domain supplied info:\n" +
              read_file(path, 1, 10) + "\n");
    }
}

/*
 * Function name: map_interactive_seconds
 * Description  : For seconds, display whether they are in the game or not,
 *                and if so, see whether they are LD.
 * Arguments    : string name - the name of the player.
 * Returns      : string - the name of the player with possible suffix.
 */
nomask static string
map_interactive_seconds(string name)
{
    object player = find_player(name);
    if (!objectp(player))
    {
        return name;
    }
    if (interactive(player))
    {
        return name + "(In)";
    }
    return name + "(LD)";
}

/*
 * Function name: finger_player
 * Description  : This function is called to display finger information about
 *                a player.
 * Arguments    : string name - the player name.
 *                int show_long - true if the long version is desired
 */
nomask static void
finger_player(string name, int show_long)
{
    int    real;
    int    pinfo;
    object player;
    object env;
    string pronoun;
    string domain;
    string str, line;
    string *names;
    int    chtime;
    int    restricted;

    /* Display the location of the player in the game, get a 'finger_player'
     * if the player is not in the game.
     */
    if (objectp(player = find_player(name)))
    {
        write(capitalize(name) + " is in the game; location: " +
              (objectp(env = environment(player)) ?
               RPATH(file_name(env)) : "VOID") + "\n");
        real = 1;
    }
    else
    {
        player = SECURITY->finger_player(name);
        write(capitalize(name) + " is not in the game.\n");
        real = 0;
    }

    /* Display the long description of the player. */
    if (show_long)
    {
        write(player->long());
    }
    else
    {
        write(LD_PRESENT_TO(player));
    }

    pronoun = ((this_player()->query_real_name() == name) ? "You are " :
               capitalize(player->query_pronoun()) + " is ");

    /* Display the rank/level of the player. */
    if (SECURITY->query_wiz_rank(name) >= WIZ_APPRENTICE)
    {
        line = pronoun +
            LANG_ADDART(WIZ_RANK_NAME(SECURITY->query_wiz_rank(name))) +
#ifdef USE_WIZ_LEVELS
            " (level " + SECURITY->query_wiz_level(name) + ")" +
#endif USE_WIZ_LEVELS
            ", set by " +
            (strlen(str = SECURITY->query_wiz_chl(name)) ?
             capitalize(str) : "root");
#ifdef FOB_KEEP_CHANGE_TIME
        line += ((chtime = SECURITY->query_wiz_chl_time(name)) ?
             (" on " + ctime(chtime)) : "");
#endif FOB_KEEP_CHANGE_TIME
        line += ".";
    }
    else
    {
        line = pronoun + "a mortal player.";
    }
    /* Display pinfo hint to those who have a need to know. */
    pinfo = (WIZ_CMD_HELPER->valid_user() && (file_size(PINFO_FILE(name)) > 0));
    line += (pinfo ? " PInfo available." : "") + "\n";
    write(line);

    /* Display the domain the player is in. */
    if (strlen(domain = SECURITY->query_wiz_dom(name)))
    {
        write(pronoun +
              ((SECURITY->query_wiz_rank(name) == WIZ_LORD) ? "Liege" :
               "a member") + " of the domain " + domain + ", added by " +
              (strlen(str = SECURITY->query_wiz_chd(name)) ?
               capitalize(str) : "root") +
#ifdef FOB_KEEP_CHANGE_TIME
              ((chtime = SECURITY->query_wiz_chd_time(name)) ?
               (" on " + ctime(chtime)) : "") +
#endif FOB_KEEP_CHANGE_TIME
              ".\n");
    }

    /* Arch team memberships. */
    if (sizeof(names = SECURITY->query_team_membership(name)))
    {
        names = sort_array(names);
        write("Arch team membership" +
            ((sizeof(names) == 1) ? "" : "s") + ": " +
            COMPOSITE_WORDS(map(names, capitalize)) + ".\n");
    }

    /* Display the seconds the player has listed. */
    if (sizeof(names = SECURITY->query_seconds(name)))
    {
        names = map(names, map_interactive_seconds);
        write("Listed second" + ((sizeof(names) == 1) ? "" : "s") + ": " +
            COMPOSITE_WORDS(map(names, capitalize)) + ".\n");
    }

    /* Display mentor information */
    if (strlen(str = SECURITY->query_mentor(player->query_real_name())))
    {
        write("Registered mentor: " + capitalize(str) + "\n");
    }
    if (sizeof(names = SECURITY->query_students(player->query_real_name())))
    {
        write("Registered student" + ((sizeof(names) == 1) ? "" : "s") + ": " +
            COMPOSITE_WORDS(map(sort_array(names), capitalize)) + ".\n");
    }

    /* Display suspension / self-restriction information. */
    if (restricted = player->query_restricted())
    {
        /* Negative means administrative restriction. */
        if (restricted < 0)
        {
            write(pronoun + "suspended from playing until " +
                ctime(-restricted) + ".\n");
        }
        else
        {
            write(capitalize(player->query_pronoun()) + " has restricted " +
                player->query_objective() + "self from playing until " +
                ctime(restricted) + ".\n");
        }
    }

    /* Display login information. */
    if (real)
    {
        write(pronoun + "logged on for " +
            CONVTIME(time() - player->query_login_time()) +
            (SECURITY->valid_query_ip(geteuid(), player) ?
            (" from " + player->query_login_from() +
            SITEBAN_SUFFIXES[SECURITY->check_newplayer(player->query_login_from())]) :
            ".") + "\n");
        if (interactive(player))
        {
            if (query_idle(player) > 0)
            {
                write("Idle time: " + CONVTIME(query_idle(player)) + ".\n");
            }
        }
        else
        {
            write(pronoun + "link dead for " +
                CONVTIME(time() - player->query_linkdead()) + ".\n");
        }
    }
    else
    {
        write("Last login " + CONVTIME(time() - player->query_login_time()) +
            " ago" + (SECURITY->valid_query_ip(geteuid(), player) ?
            (" from " + player->query_login_from() +
            SITEBAN_SUFFIXES[SECURITY->check_newplayer(player->query_login_from())]) :
            ".") + "\n");
        chtime = player->query_logout_time() - player->query_login_time();
        if (chtime < 86400) /* 24 hours, guard against patched files */
        {
            write("Duration of stay was " + CONVTIME(chtime) + ".\n");
        }
    }

    /* Display the age and email address of the player. */
    write("Age  : " + CONVTIME(player->query_age() * 2) + ".\n");
    if (strlen(str = player->query_mailaddr()) && (str != "none"))
    {
        write("Email: " + str + "\n");
    }
    player->finger_info();

    /* Clean up after ourselves if the player is not logged in. */
    if (!real)
    {
        player->remove_object();
    }
}

/*
 * Function name: finger_wizards
 * Description  : This function is called to display finger information about
 *                a group of wizards.
 * Arguments    : string *names - the lower case names of group members.
 *                int details - if true, give presence info.
 */
nomask static void
finger_wizards(string *names, int details)
{
    string *present;
    int     length;

    if (!sizeof(names))
    {
        write(" - none - \n");
        return;
    }

    names = sort_array(names);

    /* No details, just a quick table. */
    if (!details)
    {
        write(sprintf("%-75#s\n", implode(map(names, capitalize), "\n")));
        return;
    }

    /* Find out the longest name, so we can synchronise the two tables. */
    length = applyv(max, map(names, strlen));

    /* Must do this in two steps because map() returns (mixed *). */
    present = map(users(), geteuid);
    present &= names;
    names -= present;

    if (sizeof(present))
    {
        /* Synchronise the two tables by padding the first name. */
        present[0] = extract((present[0] + SPACES), 0, length-1);
        write(sprintf("%-75#s\n", implode(map(present, capitalize), "\n")));
    }
    if (sizeof(names))
    {
        /* Synchronise the two tables by padding the first name. */
        names[0] = extract((names[0] + SPACES), 0, length-1);
        write(" - nonpresent - \n");
        write(sprintf("%-75#s\n", implode(map(names, capitalize), "\n")));
    }
}

nomask int
finger(string str)
{
    int    index;
    int    size;
    int    arg_l;
    int    pinfo;
    string *names;
    mapping gread;
    mixed *banished;

    CHECK_SO_WIZ;

    /* This function never uses notify_fail() and return 0 since I do not
     * want it to fall back to the 'mortal' finger command. This is not
     * because of the emote as such (which isn't used anymore), but also
     * to not mess up the fail message which might contain useful
     * information.
     */
    str = lower_case(str);
    if (!stringp(str))
    {
        write("Syntax: finger <something>\n");
        return 1;
    }

    /* Argument -l is used. */
    if (arg_l = !!wildmatch("-l *", str))
    {
        str = extract(str, 3);
    }

    /* Wizard wants to finger a player. */
    if (SECURITY->exist_player(str))
    {
        finger_player(str, arg_l);
        return 1;
    }

    /* Wizard wants to list all domains. */
    if (str == "domains")
    {
        write(sprintf("The domains of this mud are:\n%-60#s\n",
            implode(sort_array(SECURITY->query_domain_list()), "\n")));
        return 1;
    }

    /* Wizard wants to list a particular domain. */
    if (SECURITY->query_domain_number(str) > -1)
    {
        if (arg_l)
        {
            finger_wizards(SECURITY->query_domain_members(str), arg_l);
        }
        else
        {
            finger_domain(capitalize(str));
        }
        return 1;
    }

    /* Wizard wants to list the people in the queue. */
    if ((str == "queue") ||
        (str == "q"))
    {
        names = QUEUE->queue_list(1);
        if (!(size = sizeof(names)))
        {
            write("There are no players in the queue right now.\n");
            return 1;
        }

        index = -1;
        while(++index < size)
        {
            names[index] = sprintf("%2d: %s", (index + 1), names[index]);
        }
        write("The following people are in the queue:\n" +
            sprintf("%-70#s\n", implode(names, "\n")));
        return 1;
    }

    /* Wizard wants to list those with global read. */
    if ((str == "global read") ||
        (str == "global") ||
        (str == "globals"))
    {
        gread = SECURITY->query_global_read();
        if (!m_sizeof(gread))
        {
            write("There are no wizards with global read rights.\n");
            return 1;
        }
        
        write("Wizard      Added by    Reason\n");
        write("----------- ----------- ------\n");
        names = sort_array(m_indices(gread));
        index = -1;
        size = sizeof(names);
        while(++index < size)
        {
            write(sprintf("%-11s %-11s %s\n", capitalize(names[index]),
                capitalize(gread[names[index]][0]), gread[names[index]][1]));
        }
        return 1;
    }

    /* Wizard wants to see a particular class of wizards. */
    if ((index = member_array(LANG_SWORD(str), WIZ_N)) > -1)
    {
        write("The following " + LANG_PWORD(WIZ_N[index]) + " are registered:\n");
        finger_wizards(SECURITY->query_wiz_list(WIZ_R[index]), arg_l);
        return 1;
    }

    /* Wizard wants to list an alias group */
    if (IS_MAIL_ALIAS(str))
    {
        write("The following are registered for " + capitalize(str) + ":\n");
        finger_wizards(EXPAND_MAIL_ALIAS(str), arg_l);
        return 1;
    }

    /* Wizard wants to list an admin team */
    if (member_array(str, SECURITY->query_teams()) > -1)
    {
        write("The following wizards are registered:\n");
        finger_wizards(SECURITY->query_team_list(str), arg_l);
        return 1;
    }

    banished = SECURITY->banish(str, 0);
    pinfo = (WIZ_CMD_HELPER->valid_user() && (file_size(PINFO_FILE(str)) > 0));
    if (sizeof(banished) == 2)
    {
        write("The name " + capitalize(str) + " was banished by " +
            capitalize(banished[0]) + " on " + ctime(banished[1]) +
            "." + (pinfo ? " PInfo available." : "") + "\n");
        return 1;
    }
 
    write("There is no such player, domain, category, etcetera." +
        (pinfo ? " PInfo available." : "") + "\n");
    return 1;
}

/* **************************************************************************
 * gd_info - Get some relevant Game driver information
 */
nomask int
gd_info(string icmd)
{
    string inf, *p;
    object ob;
    int f;

    if (!stringp(icmd))
    {
        notify_fail("No argument to 'gd'.\n");
        return 0;
    }

    inf = SECURITY->do_debug(icmd);
    if (!inf)
    {
        if (sizeof(p = explode(icmd, " ")) > 1)
        {
            if (sizeof(p) > 2)
            {
                f = 0;
                if (sscanf(p[1],"%d", f) != 1)
                    f = 0;
                ob = parse_list(p[2]);
                inf = SECURITY->do_debug(p[0], f, ob);
            }
            else
            {
                ob = parse_list(p[1]);
                inf = SECURITY->do_debug(p[0], ob);
            }
        }
    }

    if (stringp(inf))
        write(icmd + ":\n" + inf + "\n");
    else
        dump_array(inf);
    write("\n");
    return 1;
}

/* **************************************************************************
 * gfinger - finger showone at another mud
 */
nomask int
gfinger(string str)
{
    CHECK_SO_WIZ;
#ifdef UDP_MANAGER
    return UDP_MANAGER->cmd_gfinger(str);
#else
    notify_fail("No udp manager active.\n");
    return 0;
#endif
}

/* **************************************************************************
 * graph - display user graphs
 */
nomask int
graph(string str)
{
    return SECURITY->graph(str);
}

/* **************************************************************************
 * goto - teleport somewhere
 */
nomask int
goto(string dest)
{
    object loc;

    CHECK_SO_WIZ;

    if (!stringp(dest))
    {
        if (objectp(loc = this_interactive()->query_prop(LIVE_O_LAST_ROOM)))
        {
            this_interactive()->move_living("X", loc);
            return 1;
        }

        notify_fail("Goto where? You have no last room.\n");
        return 0;
    }

    if (!objectp(loc = find_player(dest)))
    {
        loc = find_living(dest);
    }

    if (objectp(loc) &&
        objectp(environment(loc)))
    {
        this_interactive()->move_living("X", environment(loc));
        return 1;
    }

    if (sscanf(dest, "set %s", dest) == 1)
    {
        if (!objectp(loc = parse_list(dest)))
        {
            notify_fail("Room '" + dest + "' not found.\n");
            return 0;
        }

        dest = file_name(loc);
        this_interactive()->add_prop(WIZARD_S_GOTO_SET, dest);
        write("Goto set to '" + dest + "'.\n");

        return 1;
    }

    if (dest == "set")
    {
        if (!stringp(dest = this_interactive()->query_prop(WIZARD_S_GOTO_SET)))
        {
            notify_fail("You have no previous location set.\n");
            return 0;
        }
    }
    else
    {
        dest = FTPATH(this_interactive()->query_path(), dest);
    }

    if (!objectp(loc = find_object(dest)))
    {
        if (LOAD_ERR(dest))
        {
            notify_fail("Destination '" + dest + "' cannot be loaded.\n");
            return 0;
        }

        if (!objectp(loc = find_object(dest)))
        {
            notify_fail("Destination '" + dest + "' not found.\n");
            return 0;
        }
    }

    this_interactive()->move_living("X", loc);
    return 1;
}

/* **************************************************************************
 * gtell - tell showone at another mud
 */
#ifdef UDP_ENABLED
nomask int
gtell(string str)
{
    CHECK_SO_WIZ;
#ifdef UDP_MANAGER
    return UDP_MANAGER->cmd_gtell(str);
#endif UDP_MANAGER
    notify_fail("No udp manager active.\n");
    return 0;
}
#endif UDP_ENABLED

/* **************************************************************************
 * guild - manage information about guilds
 */
nomask int
guild(string str)
{
    CHECK_SO_WIZ;

    return SECURITY->guild_command(str);
}

/* **************************************************************************
 * gwiz - intermud wizline
 */
#ifdef UDP_ENABLED
nomask int
gwiz(string str)
{
    CHECK_SO_WIZ;
#ifdef UDP_MANAGER
    return UDP_MANAGER->cmd_gwiz(str, query_verb() == "gwize");
#endif UDP_MANAGER
    notify_fail("No udp manager active.\n");
    return 0;
}
#endif UDP_ENABLED

/* **************************************************************************
 * home - go home, to a domain workroom, or to an other wizards home
 */
nomask int
home(string str)
{
    CHECK_SO_WIZ;

    if (str == "admin")
    {
        str = ADMIN_HOME;
    }
    else
    {
        if (!stringp(str))
        {
            str = this_interactive()->query_real_name();
        }

        str = SECURITY->wiz_home(str);
    }

    if (this_interactive()->move_living("X", str))
    {
        write("There is no such workroom.\n");
    }
    return 1;
}

/* **************************************************************************
 * last - give information on the players login-time
 */

/*
 * Function name: last_check
 * Description  : This function is used by the last-command. It prints the
 *                individual lines about all players the wizard selected
 *                to gather information about.
 * Arguments    : string who   - the name of the wizard to check.
 *                int    login - true for login-info rather than logout-info.
 * Returns      : string - the line to print.
 */
static nomask string
last_check(string who, int login)
{
    string result;
    object pl;
    int    tmp;
    int    t_in;
    int    t_out;

    if (objectp(pl = find_player(who[1..])))
    {
        if (interactive(pl))
        {
            tmp = time() - pl->query_login_time();
            result = "Logged on    " + TIME2STR(tmp, 2);

            /* Only list idle time if more than one minute. */
            if ((tmp = query_idle(pl)) >= 60)
            {
                result += "   " + TIME2STR(tmp, 2);
            }
        }
        else
        {
            tmp = time() - pl->query_login_time();
            result = "Linkdead     " + TIME2STR(tmp, 2);
            tmp = time() - pl->query_linkdead();
            result += "   " + TIME2STR(tmp, 2);
        }
    }
    else
    {
        /* Get a finger-player to get the login time, then clean out the
         * finger-player again. We do not want to waste the memory.
         */
        pl = SECURITY->finger_player(who[1..]);
        if (!pl)
        {
            if (who[0..0] == "<")
                return sprintf("%-14s No such player", capitalize(who[1..]));
            else
                return sprintf("  %-12s No such player", capitalize(who[1..]));
        }
        t_in = pl->query_login_time();
        t_out = pl->query_logout_time();
        pl->remove_object();

        /* This test checks whether the alleged duration of the last
         * visit of the wizard does not exceed two days. If the wizard
         * was reported to have been logged in for more than two days,
         * this probably means that the playerfile was adjusted after
         * the player last logged out. Ergo, the duration time would
         * range from the moment the wizard logged in until the moment
         * his/her playerfile was changed and that is naturally not
         * something we want.
         */
        if ((t_out - t_in) < 172800)
        {
            tmp = time() - (login ? t_in : t_out);
            result = TIME2STR(tmp, 2);

            tmp = t_out - t_in;
            result += "   " + TIME2STR(tmp, 2);
        }
        else
        {
            /* If the player's file has been changed, we use the login
             * time since the logout time (ie the file time) is not
             * correct.
             */
            tmp = time() - t_in;
            result = TIME2STR(tmp , 2) + "    (unknown)";
        }
    }

    if (who[0..0] == "<")
        return sprintf("%-14s ", capitalize(who[1..])) + result;
    else
        return sprintf("  %-12s ", capitalize(who[1..])) + result;
}

nomask int
last(string str)
{
    int    i, sz, index, login = 0, all = 0;
    object player;
    string *args, *plist = ({ });

    CHECK_SO_WIZ;

    if (stringp(str))
    {
        args = explode(lower_case(str), " ") - ({ "" });
        for (i = 0, sz = sizeof(args) ; i < sz ; i++)
        {
            switch (args[i])
            {
            case "-i":
                login = 1;
                break;

            case "-a":
                all = 1;
                break;
                
            default:
                plist += ({ args[i] });
                break;
            }
        }
    }

    /* Get the wizards currently logged in. We have to test for strlen
     * again because the wizard may have given only '-i' as argument.
     */
    args = ({});
    if (!sizeof(plist)) // No arguments
        args = filter(users(), &->query_wiz_level())->query_real_name();
    else
    {
        for (i = 0, sz = sizeof(plist) ; i < sz ; i++)
        {
            /* The name may be a class of wizards. */
            if ((index = member_array(plist[i], WIZ_N)) >= 0)
            {
                args += SECURITY->query_wiz_list(WIZ_R[index]);

                /* Ask for arches and get the keepers too. */
                if (WIZ_R[index] == WIZ_ARCH)
                    args += SECURITY->query_wiz_list(WIZ_KEEPER);
            }
            /* The name may be the name of a domain. */
            else if (SECURITY->query_domain_number(plist[i]) > -1)
            {
                args += SECURITY->query_domain_members(plist[i]);
            }
            /* The name may be a mail alias. */
            else if (IS_MAIL_ALIAS(plist[i]))
            {
                args += EXPAND_MAIL_ALIAS(plist[i]);
            }
            /* The name may be an admin team */
            else if (member_array(plist[i], SECURITY->query_teams()) > -1)
            {
                args += SECURITY->query_team_list(plist[i]);
            }
            /* The list of one or more players. */
            else
                args += ({ plist[i] });
        }
    }

    plist = ({});
    if (all)
    {
        args = sort_array(args);
        for (i = 0, sz = sizeof(args) ; i < sz ; i++)
        {
            plist += ({ "<" + args[i] });
            plist += map(sort_array(SECURITY->query_seconds(args[i])), &operator(+)(">"));
        }
    }
    else
        plist = map(sort_array(args), &operator(+)("<"));

    if (login)
    {
        write("Who            Last login    Duration     Idle time\n");
        write("---            ----------    ---------    ---------\n");
    }
    else
    {
        write("Who            Last logout   Duration     Idle time\n");
        write("---            -----------   ---------    ---------\n");
    }

    write(implode(map(plist, &last_check(, login)), "\n") + "\n");
    return 1;
}

/* **************************************************************************
 * localcmd - list my available commands
 */
nomask int
localcmd(string arg)
{
    return allcmd((stringp(arg) ? arg : "me") + " base");
}

/* **************************************************************************
 * metflag - Set the metflag, a wizard can decide if met or unmet with everyone
 */

#define METFLAG_CMDS  ([ "all": 0, "none": 1, "players": 2 ])
#define METFLAG_TEXTS ({ "\"all\" = all livings are met", "\"none\" = nobody is met", "\"players\" = players are met, but npc's are not" })

nomask int
metflag(string str)
{
    CHECK_SO_WIZ;

    if (!strlen(str))
    {
        write("Current metflag: " + METFLAG_TEXTS[this_player()->query_wiz_unmet()] + ".\n");
        return 1;
    }

    switch(str)
    {
    case "all":
    case "none":
    case "players":
        this_interactive()->set_wiz_unmet(METFLAG_CMDS[str]);
        return metflag("");

    default:
        notify_fail("Syntax: metflag [all/none/players]");
        return 0;
    }

    write("Illegal end of metflag.\n");
    return 1;
}

/* **************************************************************************
 * mudlist - List the muds known to us
 */
#ifdef UDP_ENABLED
nomask int
mudlist(string arg)
{
    CHECK_SO_WIZ;
#ifdef UDP_MANAGER
    return UDP_MANAGER->cmd_mudlist(arg);
#endif UDP_MANAGER
    notify_fail("No udp manager active.\n");
    return 0;
}
#endif UDP_ENABLED

/* **************************************************************************
 * newhooks - get new command hooks
 */
nomask int
newhooks(string str)
{
    object pl;

    CHECK_SO_WIZ;

    if ((WIZ_CHECK < WIZ_ARCH) ||
        (!stringp(str)) ||
        (find_player(str) == this_interactive()))
    {
        write("Will update your command hooks.\n");
        this_interactive()->update_hooks();
        return 1;
    }

    pl = find_player(str);
    if (!objectp(pl))
    {
        pl = present(str, environment(this_interactive()));
    }
    if (!objectp(pl) ||
        !living(pl))
    {
        pl = find_living(str);
    }
    if (!objectp(pl))
    {
        notify_fail("No such living object.\n");
        return 0;
    }
    pl->update_hooks();
    write("Updated command hooks for " + str + "(" + file_name(pl) + ").\n");
    return 1;
}

/* **************************************************************************
 * notify - Set whether wizard is to be notified of logins or not
 */

/*
 * Function name: notify_string
 * Description  : This function will return the notify flag in string form.
 * Arguments    : int nf - the notify level in int form.
 * Returns      : string - the notify level in string form.
 */
nomask string
notify_string(int nf)
{
    string nstring;

    nstring = "";

    if (nf & 1)
        nstring += "A";
    if (nf & 2)
        nstring += "W";
    if (nf & 4)
        nstring += "L";
    if (nf & 8)
        nstring += "D";
    if (nf & 16)
        nstring += "I";
    if (nf & 32)
        nstring += "X";
    if (nf & 64)
        nstring += "B";

    return nstring;
}

nomask int
notify(string str)
{
    int i, nf;
    int limit;
    string *words;
    string name;
    string wname = this_player()->query_real_name();
    string file;

    CHECK_SO_WIZ;

    nf = this_player()->query_notify();
    file = SECURITY->query_wiz_path(wname) + "/.notify";

    if (!stringp(str))
    {
        if (!nf)
        {
            write("You have the notifier turned off.\n");
        }
        else
        {
            write("Your notification level is: " + notify_string(nf) + ".\n");
        }

        if (file_size(file) > 0)
        {
            words = explode(read_file(file), "\n");
            write("You have individual notification on " +
                  COMPOSITE_WORDS(map(words, capitalize)) + ".\n");
        }
        else
        {
            write("You have no individual notification set.\n");
        }
        return 1;
    }

    words = explode(str, " ");
    switch(words[0])
    {
    case "clear":
    case "O":
    case "off":
        if (!nf)
        {
            write("The notifier was already shut off.\n");
            return 1;
        }
        this_interactive()->set_notify(0);
        write("Notifier switched off.\n");
        return 1;

    case "a":
    case "add":
        if (SECURITY->query_wiz_rank(wname) < WIZ_NORMAL)
        {
            notify_fail("Only domain wizards can have a personal notify " +
                "list.\n");
            return 0;
        }
        if (sizeof(words) != 2)
        {
            notify_fail("Notify add whom?\n");
            return 0;
        }
        name = lower_case(words[1]);
        if (!SECURITY->exist_player(name))
        {
            notify_fail("There is no player named " + name + "?\n");
            return 0;
        }
        words = ((file_size(file) > 0) ? explode(read_file(file), "\n") :
            ({ }) ) - ({ name });
        limit = 20;
        if ((SECURITY->query_wiz_rank(wname) >= WIZ_ARCH) ||
            SECURITY->query_team_member("aop", wname))
        {
            limit = 50;
        }
        if (sizeof(words) >= limit)
        {
            notify_fail("You can track only a maximum of 50 players " +
                "individually.\n");
            return 0;
        }
        words = sort_array(words + ({ name }) );
        rm(file);
        write_file(file, implode(words, "\n"));
        SECURITY->update_wiz_notify(wname);
        write("Added individual notification for " + capitalize(name) + ".\n");
        return 1;

    case "r":
    case "remove":
        if (SECURITY->query_wiz_rank(wname) < WIZ_NORMAL)
        {
            notify_fail("Only domain wizards can have a personal notify " +
                "list.\n");
            return 0;
        }
        if (sizeof(words) != 2)
        {
            notify_fail("Notify remove whom?\n");
            return 0;
        }
        name = lower_case(words[1]);
        words = ((file_size(file) > 0) ? explode(read_file(file), "\n") :
            ({ }) ) - ({ name });
        rm(file);
        if (sizeof(words))
        {
            write_file(file, implode(words, "\n"));
        }
        SECURITY->update_wiz_notify(wname);
        write("Removed individual notification for " + capitalize(name) + ".\n");
        return 1;
    }

    for (i = 0; i < strlen(str); i++)
    {
        switch (str[i])
        {
        case 'A':
            nf ^= 1;
            break;
        case 'W':
            nf ^= 2;
            break;
        case 'L':
            nf ^= 4;
            break;
        case 'D':
            nf ^= 8;
            break;
        case 'I':
            nf ^= 16;
            break;
        case 'X':
            nf ^= 32;
            break;
        case 'B':
            nf ^= 64;
            break;
        default:
            write("Strange notify state: " + extract(str, i, i) + ".\n");
            break;
        }
    }

    this_interactive()->set_notify(nf);
    write("Ok.\n");
    return 1;
}

static mixed *old_rank = 0;
static int old_time = -1;
#define MIN_DIFF_TIME 300

/* **************************************************************************
 * ranking - Print a ranking list of the domains
 */

nomask int
rank_sort(mixed *elem1, mixed *elem2)
{
    if (elem1[1] < elem2[1]) return -1;
    if (elem1[1] == elem2[1]) return 0;
    return 1;
}

nomask int
ranking(string dom)
{
    string *doms, sg;
    mixed *mems; /* Array of array of members */
    int il, q, c, s, cmd, aft;

    CHECK_SO_WIZ;

    if (!sizeof(old_rank) || (time() - old_time) > MIN_DIFF_TIME)
    {
        doms = SECURITY->query_domain_list();
        for (mems = ({}), il = 0; il < sizeof(doms); il++)
        {
            mems += ({ ({ doms[il] }) +
                       SECURITY->query_domain_members(doms[il]) });
        }

        for (mems = ({}), il = 0; il < sizeof(doms); il++)
        {
            q = SECURITY->query_domain_qexp(doms[il]);
            c = SECURITY->query_domain_cexp(doms[il]);
            cmd = SECURITY->query_domain_commands(doms[il]);
#ifdef DOMAIN_RANKWEIGHT_FORMULA
            s = DOMAIN_RANKWEIGHT_FORMULA(q, c);
#else
            s = 100 + (q / 25) + ((-c) / 10000);
#endif
            mems += ({ ({ doms[il], (cmd * s) / 100,
                          s, cmd, q, c }) });
        }

        mems = sort_array(mems, rank_sort);
    }
    else
        mems = old_rank;

    if (stringp(dom))
    {
        if (sscanf(dom, "%d", aft) != 1)
        {
            dom = capitalize(dom);
            for (aft = (sizeof(mems) - 5), il = 0; il < sizeof(mems); il++)
                if (dom == mems[il][0])
                    aft = il;
        }
    }

    write("Ranking of domains:     Rank#     Weight   Base Rank    Quest   Combat\n-------------------\n");

    for (il = sizeof(mems) -1; il >= 0; il--)
    {
        q = sizeof(mems) - il;
        if ((!stringp(dom) && (q < 11)) ||
            (stringp(dom) && (ABS(il - aft) < 3)) ||
            dom == "All")
        {
            sg = (mems[il][2] < 0 ? "-" + ABS(mems[il][2] / 100) : " " +
                  ABS(mems[il][2] / 100));
            write(sprintf("%2d: %-12s     %8d     %4s.%02d   %8d %8d %8d\n",
                          q,
                          mems[il][0], mems[il][1],
                          sg, ABS(mems[il][2] % 100),
                          mems[il][3], mems[il][4], mems[il][5]));
        }
    }
    old_rank = mems;
    old_time = time();

    return 1;
}

/* **************************************************************************
 * regret - regret an application to a domain
 */
nomask int
regret(string dom)
{
    CHECK_SO_WIZ;

    return SECURITY->regret_application(dom);
}

/* **************************************************************************
 * review - review move messages
 */
nomask int
review(string str)
{
    object tp;

    CHECK_SO_WIZ;

    if (!stringp(str))
        tp = this_interactive();
    else
        tp = find_player(str);

    if (!objectp(tp))
    {
        notify_fail("No such player: " + str +".\n");
        return 0;
    }

    write("mout:\t" + tp->query_m_out() +
          "\nmin:\t" + tp->query_m_in() +
          "\nmmout:\t" + tp->query_mm_out() +
          "\nmmin:\t" + tp->query_mm_in() + "\n");
    return 1;
}

/* **************************************************************************
 * rsupport - show what another mud support
 */
nomask int
rsupport(string str)
{
    CHECK_SO_WIZ;
#ifdef UDP_MANAGER
    return UDP_MANAGER->cmd_support(str);
#endif UDP_MANAGER
    notify_fail("No udp manager active.\n");
    return 0;
}

/* **************************************************************************
 * rwho - show people on other muds
 */
nomask int
rwho(string str)
{
    CHECK_SO_WIZ;
#ifdef UDP_MANAGER
    return UDP_MANAGER->cmd_rwho(str);
#endif UDP_MANAGER
    notify_fail("No udp manager active.\n");
    return 0;
}

/* **************************************************************************
 * sanction - sanction actions
 */
nomask int
sanction(string str)
{
    CHECK_SO_WIZ;

    return SECURITY->sanction(str);
}

/* **************************************************************************
 * setmout - set move out message.
 */
nomask int
set_m_out(string m)
{
    CHECK_SO_WIZ;

    if (!strlen(m))
    {
        write("Your m-out: " + this_interactive()->query_m_out() + "\n");
        return 1;
    }

    this_interactive()->set_m_out(m);
    write("M-out changed to: " + m + "\n");
    return 1;
}

/* **************************************************************************
 * setmin - set move in message.
 */
nomask int
set_m_in(string m)
{
    CHECK_SO_WIZ;

    if (!strlen(m))
    {
        write("Your m-in: " + this_interactive()->query_m_in() + "\n");
        return 1;
    }

    if (wildmatch("*[.!]", m))
    {
        notify_fail("Please observe that there should not be a period to " +
                    "this text. Consult the help page if you are doubtful.\n");
        return 0;
    }

    this_interactive()->set_m_in(m);
    write("M-in changed to: " + m + "\n");
    return 1;
}

/* **************************************************************************
 * setmmout - set teleport out message.
 */
nomask int
set_mm_out(string m)
{
    CHECK_SO_WIZ;

    if (!strlen(m))
    {
        write("Your mm-out: " + this_interactive()->query_mm_out() + "\n");
        return 1;
    }

    this_interactive()->set_mm_out(m);
    write("MM-out changed to: " + m + "\n");
    return 1;
}

/* **************************************************************************
 * setmmin - set teleport in message.
 */
nomask int
set_mm_in(string m)
{
    CHECK_SO_WIZ;

    if (!strlen(m))
    {
        write("Your mm-in: " + this_interactive()->query_mm_in() + "\n");
        return 1;
    }

    this_interactive()->set_mm_in(m);
    write("MM-in changed to: " + m + "\n");
    return 1;
}

/* **************************************************************************
 * start - set start point.
 */
nomask int
start(string str)
{
    CHECK_SO_WIZ;

    if (!stringp(str))
    {
        write("Your default starting location: " +
            this_interactive()->query_default_start_location() + "\n");
        return 1;
    }

    if (str == "valid")
    {
        str = implode(SECURITY->query_list_def_start(), ", ");
        write("Available starting locations:\n" + break_string(str, 76, 3) +
            "\n");
        return 1;
    }

    if (str == "here")
    {
        str = file_name(environment(this_interactive()));
    }

    if (!this_interactive()->set_default_start_location(str))
    {
        write("<" + str + "> is not a valid starting location.\n");
        return 1;
    }
    write("Ok.\n");
    return 1;
}

/* **************************************************************************
 * title - change the title
 */
nomask int
title(string t)
{
    CHECK_SO_WIZ;

    if (!stringp(t))
    {
        write("Your title is '" + this_interactive()->query_title() + "'.\n");
        return 1;
    }

    this_interactive()->set_title(t);
    write("Ok.\n");
    return 1;
}

/* **************************************************************************
 * wizopt - Change/view the wizard options
 */
nomask int
wizopt(string arg)
{
    if (!stringp(arg))
    {
        /* These names directly link to the options arguments. */
        return CMD_LIVE_STATE->options("autolinecmd");
        return CMD_LIVE_STATE->options("autopwd");
        return CMD_LIVE_STATE->options("timestamp");
    }
    else
    {
        return CMD_LIVE_STATE->options(arg);
    }
}

/* **************************************************************************
 * whereis - Display the location of a player or living.
 */
nomask int
whereis(string str)
{
    object obj;
    object *live;
    object *dead;
    int    verbose;

    if (!stringp(str))
    {
        notify_fail("Whereis who?\n");
        return 0;
    }

    verbose = sscanf(lower_case(str), "-v %s", str);

    if (!objectp(obj = find_player(str)))
    {
        if (!objectp(obj = find_living(str)))
        {
            notify_fail("No player or NPC '" + capitalize(str) + "' found.\n");
            return 0;
        }

        write("No player '" + capitalize(str) +
            "' logged in. We found an NPC though.\n");
    }

    if (!objectp(obj = environment(obj)))
    {
        write(capitalize(str) + " is sitting in the void.\n");
        return 1;
    }

    write("File: " + file_name(obj) + "\n");
    if (!verbose)
    {
        write("Room: " + obj->short(this_player()) + "\n");
        return 1;
    }

    write(obj->long(0));

    live = FILTER_LIVE(dead = all_inventory(obj));
    dead = FILTER_SHOWN(dead - live);
    if (sizeof(dead) &&
        strlen(str = COMPOSITE_DEAD(dead)))
    {
        write(break_string((capitalize(str) + "."), 76) + "\n");
    }
    if (sizeof(live) &&
        strlen(str = COMPOSITE_LIVE(live)))
    {
        write(break_string((capitalize(str) + "."), 76) + "\n");
    }
    return 1;
}

/* **************************************************************************
 * whichsoul - Find out which soul defines a particular command.
 */

/*
 * Function name: print_whichsoul
 * Description  : Searches a type of souls for the command desired.
 * Arguments    : string *soul_list - the souls to search through.
 *                string cmd - the command to look for.
 *                string type - the type of soul.
 * Returns      : int - number of times the command was found.
 */
static nomask string
print_whichsoul(string *soul_list, string cmd, string type)
{
    int     index = -1;
    int     size = sizeof(soul_list);
    string  soul_id;
    mapping cmd_list;
    string  text = "";

    while(++index < size)
    {
        cmd_list = soul_list[index]->query_cmdlist();
        if (cmd_list[cmd])
        {
            soul_id = soul_list[index]->get_soul_id();
            soul_id = (strlen(soul_id) ? soul_id : "noname");
            text += sprintf("%-7s  %-15s  %-15s  %-s\n", type, soul_id,
                cmd_list[cmd] + "()", soul_list[index] + ".c");
        }
    }

    return text;
}

int
whichsoul(string str)
{
    object target;
    mixed *cmds;
    string text = "";
    int    index = -1;
    int    size;
    string extra;

    if (!stringp(str))
    {
        notify_fail("Syntax: whichsoul [<person>] <command>\n");
        return 0;
    }

    cmds = explode(str, " ");
    switch(sizeof(cmds))
    {
    case 1:
        target = this_player();
        break;

    case 2:
        target = find_player(lower_case(cmds[0]));
        if (!objectp(target))
        {
            notify_fail("There is no player called " +
                capitalize(cmds[0]) + " in the game.\n");
            return 0;
        }
        str = cmds[1];
        break;

    default:
        notify_fail("Too many arguments. Syntax: whichsoul " +
            "[<person>] <command>\n");
        return 0;
    }

    cmds = commands(target);
    size = sizeof(cmds);
    while (++index < size)
    {
        if (str == cmds[index][0])
        {
            extra = (cmds[index][1] ? ("(part = " + cmds[index][1] + ")") : "");
            text += sprintf("%-15s  %-s\n",
                cmds[index][3] + "()", file_name(cmds[index][2]) + extra);
        }
    }

    if (strlen(text))
    {
        write("Function         Filename\n");
        write("--------         --------\n");
        write(text + "\n");
    }
    else
    {
        write("Command " + str + " not found among local commands. (add_action's)\n\n");
    }

    text = print_whichsoul(target->query_wizsoul_list(), str, "wizard");
    text += print_whichsoul(target->query_tool_list(),    str, "tool");
    text += print_whichsoul(target->query_cmdsoul_list(), str, "command");

    if (strlen(text))
    {
        write("Type     Soulname         Function         Filename\n");
        write("----     --------         --------         --------\n");
        write(text);
    }
    else
    {
        write("Command " + str + " not found in any souls.\n");
    }

    return 1;
}

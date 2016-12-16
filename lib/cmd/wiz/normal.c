/*
 * /cmd/wiz/normal.c
 *
 * This object holds the normal wizards commands. Since the soul is split
 * over a few modules, the list below only reflects the commands actually
 * coded in this module:
 *
 * - banish
 * - buglog
 * - cbd
 * - cbs
 * - cmdsoul
 * - combatdata
 * - combatstat
 * - control
 * - donelog
 * - dtell
 * - dtelle
 * - du
 * - echo
 * - echoto
 * - errlog
 * - force
 * - idealog
 * - invis
 * - leave
 * - modify
 * - money
 * - msecond
 * - peace
 * - possess
 * - praiselog
 * - restrict
 * - retire
 * - runlog
 * - sdoc
 * - shutdown
 * - skillstat
 * - snoop
 * - stat
 * - teams
 * - tellall
 * - toolsoul
 * - tradestat
 * - trans
 * - typolog
 * - vis
 */

#pragma no_clone
#pragma no_inherit
#pragma strict_types
#pragma save_binary

inherit "/cmd/std/tracer_tool_base.c";

/*
 * This is necessary for AFT.
 */
inherit "/lib/cache";

#include <composite.h>
#include <filepath.h>
#include <files.h>
#include <filter_funs.h>
#include <flags.h>
#include <language.h>
#include <log.h>
#include <macros.h>
#include <money.h>
#include <options.h>
#include <ss_types.h>
#include <std.h>
#include <stdproperties.h>
#include <time.h>

#define CHECK_SO_WIZ    if (WIZ_CHECK < WIZ_NORMAL) return 0; \
                        if (this_interactive() != this_player()) return 0

#include "/cmd/wiz/normal/edit.c"
#include "/cmd/wiz/normal/files.c"

static nomask int somelog(string str, string logname, string log);
static nomask int valid_possess(object demon, object possessed);

/*
 * Function name: create
 * Descritpion  : Constructor. Called when the soul is loaded. We use it
 *                to set the cache size larger than the default. Because the
 *                cache is used for two commands, we doubled it.
 */
nomask void
create()
{
    set_cache_size(20);
}

/* **************************************************************************
 * Return a list of which souls are to be loaded.
 * The souls are listed in order of command search.
 */
nomask string *
get_soul_list()
{
    return ({ WIZ_CMD_NORMAL,
              WIZ_CMD_APPRENTICE,
              MBS_SOUL });
}

/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
nomask string
get_soul_id()
{
    return WIZNAME_NORMAL;
}

/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
nomask mapping
query_cmdlist()
{
    return ([
             "aft":"aft",

             "banish":"banish",
             "buglog":"buglog",

             "cbd":"combatdata",
             "cbs":"combatstat",
             "clone":"clone",
             "cmdsoul":"cmdsoul",
             "combatdata":"combatdata",
             "combatstat":"combatstat",
             "control":"control",
             "cp":"cp_cmd",

             "destruct":"destruct_ob",
             "distrust":"distrust",
             "donelog":"donelog",
             "dtell":"dtell",
             "dtelle":"dtell",
             "du":"du",

             "echo":"echo",
             "echoto":"echo_to",
             "ed":"ed_file",
             "errlog":"errlog",
             "exec":"exec_code",
             "execr":"exec_code",

             "force":"force",

             "idealog":"idealog",
             "invis":"invis",

             "leave":"leave",
             "load":"load",

             "mkdir":"makedir",
             "modify":"modify",
             "money":"money",
             "msecond":"msecond",
             "mv":"mv_cmd",

             "pad":"pad",
             "peace":"peace",
             "possess":"possess",
             "praiselog":"praiselog",

             "remake":"remake_object",
             "restrict":"restrict",
             "retire":"retire",
             "rm":"rm_cmd",
             "rmdir":"removedir",
             "runlog":"runlog",

             "sdoc":"sdoc",
             "shutdown":"shutdown_game",
             "skillstat":"skillstat",
             "snoop":"snoop_on",
             "stat":"stat",

             "teams":"teams",
             "tellall":"tellall",
             "toolsoul":"toolsoul",
             "tradestat":"tradestat",
             "trans":"trans",
             "trust":"trust_ob",
             "typolog":"typolog",

             "update":"dupd",

             "vis":"vis",
             ]);
}
 
/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the 
 * same order as in the function name list.
 * **************************************************************************/

/* **************************************************************************
 * banish - banish a name
 */
nomask int
banish(string arg)
{
    int wtype;
    string *argv, name, what;
    mixed *rval;

    if (stringp(arg) == 0)
    {
        notify_fail("Banish who?\n");
        return 0;
    }

    argv = explode(arg, " ");

    if (sizeof(argv) == 2)
    {
        what = argv[0];
        name = argv[1];
    }
    else
    {
        what = "-i";
        name = argv[0];
    }
    
    if (SECURITY->exist_player(name))
    {
        notify_fail(capitalize(name) + " is a player in the game.\n");
        return 0;
    }

    wtype = SECURITY->query_wiz_rank(this_interactive()->query_real_name());
    switch (what)
    {
    case "-a":
    case "-add":
        /*
         * Arch and keeper can banish anywhere.
         */
        if (wtype < WIZ_ARCH)
        {
            if (function_exists("create_workroom", 
                environment(this_interactive())) != ADMIN_HOME)
            {
                notify_fail("You can only banish names in the " +
                    "administrators workroom. Type \"home admin\" to go " +
                    "there.\n");
                return 0;
            }
        }
            
        /*
         * Try to banish.
         */
        rval = SECURITY->banish(name, 2);
        
        if (sizeof(rval) != 0)
            write(capitalize(name) + " was banished by " + 
                  capitalize(rval[0]) + " at " + ctime(rval[1]) + ".\n");
        else
            write("You have banished: " + capitalize(name) + ".\n");
        
        break;
        
    case "-i":
    case "-info":
        /*
         * Look for information.
         */
        rval = SECURITY->banish(name, 0);
        if (sizeof(rval) != 0)
            write(capitalize(name) + " was banished by " + 
                  capitalize(rval[0]) + " at " + ctime(rval[1]) + ".\n");
        else
            write(capitalize(name) + " has not been banished.\n");
        break;
        
    case "-r":
    case "-remove":
        /*
         * Try to remove.
         */
        rval = SECURITY->banish(name, 0);
        if (sizeof(rval) != 0)
        {
            if (wtype != WIZ_KEEPER && wtype != WIZ_ARCH)
            {
                if (rval[0] != this_interactive()->query_real_name())
                {
                    notify_fail("You have not banished " + 
                                capitalize(name) + ".\n");
                    return 0;
                }
            }
        }
        else
        {
            notify_fail(capitalize(name) + " has not been banished.\n");
            return 0;
        }

        rval = SECURITY->banish(name, 1);
        if (sizeof(rval) != 0)
        {
            write(capitalize(name) + " was banished by " + 
                  capitalize(rval[0]) + " at " + ctime(rval[1]) + ".\n");
            write("You have removed the banishment of " + 
                  capitalize(name) + ".\n");
        }

        break;
        
    default:
        break;
    }
    return 1;
}

/* **************************************************************************
 * buglog - list a buglog
 */
nomask int
buglog(string str) 
{ 
    return somelog(str, "bug log", "/bugs"); 
}

/* **************************************************************************
 * cmdsoul - affect your command souls
 */
nomask int
cmdsoul(string str)
{
    string *cmdsoul_list;
    int index, size;

    CHECK_SO_WIZ;

    cmdsoul_list = (string *)this_interactive()->query_cmdsoul_list();
    if (!stringp(str))
    {
        index = -1;
        size = sizeof(cmdsoul_list);
        write("Current cmdsouls:\n");
        while(++index < size)
        {
            write(sprintf("%2d: %s\n", (index + 1), cmdsoul_list[index]));
        }
        return 1;
    }

    str = FTPATH((string)this_interactive()->query_path(), str);

    if (member_array(str, cmdsoul_list) >= 0)
    {
        if (this_interactive()->remove_cmdsoul(str))
            write("Removed " + str + ".\n");
        else
            write("Failed on " + str + ".\n");
        return 1;
    }

    if (this_interactive()->add_cmdsoul(str))
        write("Added " + str + ".\n");
    else
        write("Failed on " + str + ".\n");
    return 1;
}

/* **************************************************************************
 * combatdata - Show combat statistics for a living object
 */
nomask int
combatdata(string str)
{
    object live;

    CHECK_SO_WIZ;

    if (!str)
        live = this_player();
    else if (live = present(str, environment(this_player())));
    else if (live = find_player(str));
    else if (live = find_living(str));

    if (live)
        write(live->combat_data());
    else
    {
        notify_fail("No such living found: " + str + "\n");
        return 0;
    }
    return 1;
}

/* *************************************************************************
 * combatstat - Show some combat statistics for a living object
 */
nomask int
combatstat(string str)
{
    object live;

    CHECK_SO_WIZ;

    if (!str)
        live = this_player();
    else if (live = present(str, environment(this_player())));
    else if (live = find_player(str));
    else if (live = find_living(str));

    if (live)
        write(live->combat_status());
    else
    {
        notify_fail("No such living found: " + str + "\n");
        return 0;
    }
    return 1;
}

/* **************************************************************************
 * control - Get a 'control wand' for a specific monster
 */
nomask int
control(string npc)
{
    object mon, owner;
    string str;

    CHECK_SO_WIZ;

    notify_fail("Control who?\n");

    if (!npc)
        return 0;

    mon = present(npc, environment(this_player()));

    if (!mon)
        mon = find_living(npc);

    if (!mon)
    {
        notify_fail("I can't seem to locate " + capitalize(npc) + "\n");
        return 0;
    }

    if (!mon->query_npc())
    {
        notify_fail("You feel a strong resistance towards control.\n");
        return 0;
    }

    if (owner = (object)mon->query_link_remote())
    {
        owner = environment(owner);
        if (living(owner))
            str = "is held by " + owner->short();
        else
            str = "seems to be in: " + owner->short();
        write(capitalize(npc) + " is already controlled. The wand " +
              str + "\n");
        return 1;
    }
    mon->set_link_remote();
    write("Ok.\n");
    return 1;
}

/* **************************************************************************
 * donelog - list donelog
 */
nomask int
donelog(string str) 
{ 
    return somelog(str, "done log", "/done"); 
}

/* **************************************************************************
 * dtell  - actually an alias for 'line <my domain>'
 * dtelle - actually an alias for 'linee <my domain>'
 */
nomask int
dtell(string str)
{
    string domain;

    if (!stringp(str))
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    if (!strlen(domain =
        SECURITY->query_wiz_dom(this_interactive()->query_real_name())))
    {
        notify_fail("You are not a member of any domain.\n");
        return 0;
    }

    return WIZ_CMD_APPRENTICE->line((domain + " " + str),
        (query_verb() == "dtelle"));
}

/* **************************************************************************
 * echo - echo something in a room
 */
nomask int
echo(string str)
{
    CHECK_SO_WIZ;

    if (!stringp(str))
    {
        notify_fail("Echo what?\n");
        return 0;
    }

    say(str + "\n");

#ifdef LOG_ECHO
    if (SECURITY->query_wiz_rank(this_player()->query_real_name()) < WIZ_ARCH)
    {
        SECURITY->log_syslog(LOG_ECHO, sprintf("%s %-11s: %s\n", ctime(time()),
            capitalize(this_interactive()->query_real_name()), str));
    }
#endif LOG_ECHO

    if (this_player()->query_option(OPT_ECHO))
    {
        write("You echo: " + str + "\n");
    }
    else
    {
        write("Ok.\n");
    }
    return 1;
}

/* **************************************************************************
 * echoto - echo something to someone
 */
nomask int
echo_to(string str)
{
    object ob;
    string who, msg;

    CHECK_SO_WIZ;

    if (!stringp(str) ||
        (sscanf(str, "%s %s", who, msg) != 2))
    {
        notify_fail("Syntax: echoto <who> <what> ?\n");
        return 0;
    }

    ob = find_player(lower_case(who));
    if (!objectp(ob))
    {
        notify_fail("No player " + capitalize(who) + " in the game.\n");
        return 0;
    }

#ifdef LOG_ECHO
    if (SECURITY->query_wiz_rank(this_player()->query_real_name()) < WIZ_ARCH)
    {
        SECURITY->log_syslog(LOG_ECHO, sprintf("%s %-11s to %-11s: %s\n",
            ctime(time()), capitalize(this_interactive()->query_real_name()),
            capitalize(who), msg));
    }
#endif LOG_ECHO

    tell_object(ob, msg + "\n");
    if (this_player()->query_option(OPT_ECHO))
    {
        write("You echo to " + capitalize(who) + ": " + msg + "\n");
    }
    else
    {
        write("Ok.\n");
    }

    return 1;
}

/* **************************************************************************
 * errlog - list an error log
 */
nomask int
errlog(string str) 
{ 
    return somelog(str, "error log", "/errors"); 
}

/* **************************************************************************
 * force - force a player to do something
 */
nomask int
force(string str)
{
    string who, what;
    object ob;

    CHECK_SO_WIZ;

    if (!stringp(str))
    {
        notify_fail("Force who what?\n");
        return 0;
    }

    if (sscanf(str, "%s to %s", who, what) != 2 &&
        sscanf(str, "%s %s", who, what) != 2)
    {
        notify_fail("Force who to do what?\n");
        return 0;
    }

    ob = find_living(who);
    if (!objectp(ob))
    {
        notify_fail("No player called " + who + ".\n");
        return 0;
    }

#ifdef LOG_FORCE
    if (SECURITY->query_wiz_rank(this_player()->query_real_name()) < WIZ_ARCH)
    {
        SECURITY->log_syslog(LOG_FORCE, sprintf("%s %-11s to %-11s: %s\n",
            ctime(time()), capitalize(this_interactive()->query_real_name()),
            capitalize(who), what));
    }
#endif LOG_FORCE

    if (ob->command(what))
    {
        tell_object(ob, this_interactive()->query_The_name(ob) +
                    " forced you to: " + what + "\n");
        write("Ok.\n");
    }
    else
    {
        tell_object(ob, this_interactive()->query_The_name(ob) +
                    " tried to force you to: " + what + "\n");
        write("You failed.\n");
    }
    return 1;
}

/* **************************************************************************
 * idealog - read the idea log
 */
nomask int
idealog(string str) 
{ 
    return somelog(str, "idea log", "/ideas"); 
}

/* **************************************************************************
 * invis - become invisible
 */
nomask int
invis()
{
    CHECK_SO_WIZ;

    if (this_player()->query_invis())
    {
        notify_fail("You are already invisible.\n");
        return 0;
    }

    write("You are now invisible.\n");
    say(QCTNAME(this_player()) + " " + this_player()->query_mm_out() + "\n");
    this_player()->set_invis(1);
    say( ({ this_player()->query_name() +
                " remains invisible in the room though.\n",
            "The " + this_player()->query_nonmet_name() +
                " remains invisible in the room though.\n", "" }) );
    return 1;
}

/* **************************************************************************
 * leave - leave a domain
 */
nomask int
leave(string str)
{
    CHECK_SO_WIZ;

    if (str != "domain")
    {
        notify_fail("Syntax: leave domain\n");  
        return 0;
    }

    return SECURITY->leave_domain();
}

/* **************************************************************************
 * modify - modify experience or stats of a player.
 */
nomask int
modify(string str)
{
    string *words;
    string reason;
    string name;
    int    amount;
    int    stat;
    int    self;
    object player;

    CHECK_SO_WIZ;

    if (!strlen(str))
    {
        notify_fail("Modify what? Do \"help modify\" for instructions.\n");
        return 0;
    }

    words = explode(str, " ");
    name = lower_case(words[0]);
    self = (name == this_player()->query_real_name()) || wildmatch("*jr", name);
    if (sizeof(words) < (5 - self))
    {
        notify_fail("Syntax: modify <person> <stat>/exp <type> <amount> " +
            "[<reason>]\n");
        return 0;
    }

    if (!objectp(player = find_player(name)))
    {
        notify_fail("Player \"" + capitalize(name) +
            "\" is not found in the game.\n");
        return 0;
    }

    amount = atoi(words[3]);
    reason = (self ? "self" : implode(words[4..], " "));

    words[2] = lower_case(words[2]);
    if (member_array(words[2], ({ "quest", "combat", "general" }) ) < 0)
    {
        notify_fail("Unknown type of experience. Only \"quest\", " +
            "\"combat\" and \"general\" are recognised.\n");
        return 0;
    }

    words[1] = lower_case(words[1]);
    if (words[1] == "exp")
    {
        if (amount == 0)
        {
            notify_fail("The amount of experience to change can be " +
                "positive or negative, but not zero.\n");
            return 0;
        }

        player->modify_exp(words[2], amount, reason);
        write("Changed " + words[2] + " experience of " +
            player->query_name() + " with delta " + amount + ".\n");
        return 1;
    }

    if (amount <= 0)
    {
        notify_fail("The new stat value must be positive.\n");
        return 0;
    }

    stat = member_array(words[1], SS_STAT_DESC);
    if ((stat < 0) ||
        (stat >= SS_NO_STATS))
    {
        notify_fail("Unknown stat name. Valid are: " +
            COMPOSITE_WORDS(SS_STAT_DESC) + ". Or use \"exp\" to change " +
            "the total experience of a player.\n");
        return 0;
    }

    player->modify_stat(words[2], stat, amount, reason);
    write("Altered stat " + words[1] + " using " + words[2] +
        " experience of " + player->query_name() + " with to value " +
        amount + ".\n");
    return 1;
}

/* **************************************************************************
 * money - display, clone or destruct money
 */
nomask int
money(string str)
{
    object coin;
    int *coins;
    int *new_coins;
    int index;
    int size;

    CHECK_SO_WIZ;

    if (stringp(str))
    {
        new_coins = allocate(4);
        if ((size = sscanf(str, "%d %d %d %d", new_coins[0], new_coins[1],
            new_coins[2], new_coins[3])) == 0)
        {
            notify_fail("Usage: money <cc> <sc> <gc> <pc>\n");
            return 0;
        }

        if (size > sizeof(MONEY_TYPES))
        {
            notify_fail("There are only " + sizeof(MONEY_TYPES) +
                " money types.\n");
            return 0;
        }

        index = -1;
        while(++index < size)
        {
            /* If the wizard already has such coins, change the coins, else
             * we only clone new if that is necessary.
             */
            if (objectp(coin = present((MONEY_TYPES[index] + " coin"),
                this_player())))
            {
                /* If we indeed need to keep these coins, change the number
                 * or else remove them.
                 */
                if (new_coins[index])
                {
                    coin->set_heap_size(new_coins[index]);
                }
                else
                {
                    coin->remove_object();
                }
            }
            else
            {
                if (new_coins[index])
                {
                    MONEY_MAKE(new_coins[index],
                        MONEY_TYPES[index])->move(this_player());
                }
            }
        }
    }

    coins = MONEY_COINS(this_player());
    write("You are carrying:\n");
    index = -1;
    size = sizeof(MONEY_TYPES);
    while(++index < size)
    {
        write(sprintf("%-11s: %6d\n", capitalize(MONEY_TYPES[index]),
            coins[index]));
    }
        
    write(sprintf("%11s: %6d\n", "Total value", MONEY_MERGE(coins)));
    return 1;
}

/* ***************************************************************************
 *  msecond - Modify seconds entry in mortals
 */

public int
msecond(string str)
{
    string *args;
    mixed info;

    if (!strlen(str))
    {
	notify_fail("Syntax: msecond <player>\n" +
		    "        msecond a[dd] <name> to <player>\n" +
		    "        msecond r[emove] <name> from <player>\n");
	return 0;
    }

    args = explode(lower_case(str), " ");
    switch (args[0])
    {
    case "a":
    case "add":
	if (sizeof(args) != 4 || args[2] != "to")
	{
	    msecond("");
	    return 0;
	}
	if (!SECURITY->exist_player(args[1]))
	{
	    notify_fail("The player " + capitalize(args[1]) + " does not exist.\n");
	    return 0;
	}
	if (!SECURITY->exist_player(args[3]))
	{
	    notify_fail("The player " + capitalize(args[3]) + " does not exist.\n");
	    return 0;
	}
        if (!SECURITY->add_second(args[3], args[1]))
        {
            return 0;
        }

	write("Added second " + capitalize(args[1]) + " to " + capitalize(args[3]) + ".\n");
	return msecond(args[3]);

    case "r":
    case "remove":
	if (sizeof(args) != 4 || args[2] != "from")
	{
	    msecond("");
	    return 0;
	}
        if (!SECURITY->remove_second(args[3], args[1]))
        {
            return 0;
        }
	
	write("Removed second " + capitalize(args[1]) + " from " + capitalize(args[3]) + ".\n");
	return msecond(args[3]);

    default:
	if (sizeof(args) != 1)
	{
	    msecond("");
	    return 0;
	}
        str = SECURITY->query_find_first(args[0]);
        if (!str)
        {
            notify_fail("No player " + capitalize(args[0]) + " or no seconds.\n");
            return 0;
        }
	args = sort_array(SECURITY->query_seconds(str));
	if (!sizeof(args))
	{
	    write(sprintf("%-11s: ", capitalize(str)) + "No seconds\n");
	    return 1;
	}

        write("Seconds for " + capitalize(str) + ":\n");
        foreach(string second: args)
        {
           info = SECURITY->query_second_info(str, second);
           write(sprintf("- %-11s added %s by %s\n", capitalize(second), ctime(info[1]), capitalize(info[0])));
        }
	return 1;
    }
    write("Impossible end of msecond.\n");
    return 1;
}

/* **************************************************************************
 * peace - stop fighting (in the room)
 */
nomask int
peace(string str)
{
    object *oblist;
    object *targets;
    int     index = -1;
    int size;

    CHECK_SO_WIZ;

    if (!stringp(str))
    {
        oblist = ({ this_player() });

        say(QCTNAME(this_player()) + " makes peace with all " +
            this_player()->query_possessive() + " enemies.\n");
        write("You make peace for yourself.\n");
    }
    else
    {
        oblist = parse_this(str, "[the] %l");

        if (!sizeof(oblist))
        {
            notify_fail("Peace for whom?\n");
            return 0;
        }

        actor("You make peace for", oblist);
        target(" makes peace for you.", oblist);
        all2act(" makes peace for", oblist);
    }

    size = sizeof(oblist);
    while(++index < size)
    {
        targets = oblist[index]->query_enemy(-1);

        oblist[index]->stop_fight(targets);
        targets->stop_fight(oblist[index]);
    }

    return 1;
}

/* **************************************************************************
 * possess - possess a living object
 */
nomask int
possess(string arg)
{
    object victim, possob;
    string *argv, v_name, p_name;
    int argc;

    CHECK_SO_WIZ;

    argv = explode(arg, " ");
    argc = sizeof(argv);

    if (argc == 0)
    {
        notify_fail("Possess what?\n");
        return 0;
    }

    /*
     * Find the victim of this evil spell.
     */
    if (argc == 1)
        v_name = lower_case(argv[0]);
    else
        v_name = lower_case(argv[1]);

    victim = present(v_name, environment(this_interactive()));
        
    if (!victim)
        victim = find_living(v_name);

    if (!victim)
    {
        notify_fail("No living object with that name.\n");
        return 0;
    }

    p_name = (string)victim->query_possessed();

    if (!living(victim))
    {
        notify_fail("You can only possess living beings.\n");
        return 0;
    }

    if (victim == this_interactive())
    {
        notify_fail("Hey! You're already in your head. For more self-control, go see a shrink.\n");
        return 0;
    }

    if (!valid_possess(this_interactive(), victim))
    {
        notify_fail("Sorry, you're not allowed to do that.\n");
        return 0;
    }

    /*
     * Only an Arch or Keeper can break or force a possession.
     */
    if ((argc == 2) &&
        (SECURITY->query_wiz_rank(geteuid(this_interactive())) >= WIZ_ARCH))
    {
        switch (argv[0])
        {
        case "force":
            if (strlen(p_name))
            {
                victim->command("quit");
                write("You exorcised " + capitalize(p_name) + ".\n");
                tell_object(find_player(p_name), "You were exorcised by " + capitalize((string)this_interactive()->query_real_name()) + ".\n");
            }
            else
                write(capitalize(v_name) + " is unpossessed.\n");
            break;

        case "break":
            if (strlen(p_name))
            {
                victim->command("quit");
                write("You exorcised " + capitalize(p_name) + ".\n");
                tell_object(find_player(p_name), "You were exorcised by " + capitalize((string)this_interactive()->query_real_name()) + ".\n");
            }
            else
                write(capitalize(v_name) + " is unpossessed.\n");
            return 1;
            break;

        default:
            notify_fail("Strange command: " + argv[0] + ".\n");
            return 0;
            break;
        }
    }
    else
    {
        if (strlen((string)victim->query_possessed()))
        {
            notify_fail(capitalize(v_name) + " is already possessed by " + capitalize(p_name) + ".\n");
            return 0;
        }
    }

    if (!query_interactive(victim))
    {
        if (SECURITY->exist_player(v_name))
        {
            notify_fail("You can't possess an object with the same name as a player.\n");
            return 0;
        }
    }

    /*
     * Create the control object and do the possession.
     */
    possob = clone_object(POSSESSION_OBJECT);
    possob->move(victim, 1);
    possob->set_name(v_name);

    if (possob->possess(this_interactive(), victim))
    {
        snoop(possob, victim);
        possob->set_lock();
    }
    else
    {
        possob->remove_object();
    }

    return 1;
}

static nomask int
valid_possess(object demon, object possessed)
{
    int rank;
    string dom, by, on;

    rank = SECURITY->query_wiz_rank(geteuid(demon));

    /* Arch & keeper possesses all everywhere */
    if (rank >= WIZ_ARCH)
        return 1;

    by = geteuid(demon);
    dom = SECURITY->query_wiz_dom(by);
    on = geteuid(possessed);

    /* never possess an arch or keeper. */
    if ((rank < WIZ_ARCH) &&
        (SECURITY->query_wiz_rank(on) >= WIZ_ARCH))
    {
        return 0;
    }

    /* Lords can possess members & member's objects everywhere */
    if (rank == WIZ_LORD)
    {
        if (SECURITY->query_domain_lord(dom) == by)
        {
            if (dom == SECURITY->query_wiz_dom(on))
            {
                return 1;
            }
        }
    }

    /* You can possess objects in your domain */
    if (!query_interactive(possessed) &&
        ((dom == on) || (dom == SECURITY->query_wiz_dom(on))))
    {
        return 1;
    }

    return 0;
}

/* **************************************************************************
 * praiselog - list your praise log
 */
nomask int
praiselog(string str) 
{ 
    return somelog(str, "praise log", "/praise"); 
}

/* **************************************************************************
 * restrict - manipulate the restriction settings of a wiz
 */
nomask int
restrict(string str)
{
    string *args, who, dom, *wlist;
    int res, setres, i, sz;

    CHECK_SO_WIZ;

    if (strlen(str) == 0)
    {
        notify_fail("Syntax: restrict list\n" +
                    "        restrict [wiz] <restriction>\n" +
                    "        restrictions: snoop/s, snoopdom/sd, rw, w, log\n");
        return 0;
    }

    args = explode(str, " ");

    // Mortals don't have any powers to restrict
    if (args[0] != "list" && SECURITY->query_wiz_rank(args[0]) < WIZ_NORMAL)
    {
        notify_fail("You can only restrict full wizards.\n");
        return 0;
    }

    // Mentors can only restrict their students
    who = this_interactive()->query_real_name();
    if (SECURITY->query_wiz_rank(who) < WIZ_STEWARD &&
        SECURITY->query_mentor(args[0]) != who)
    {
        notify_fail("You are not the mentor of " + capitalize(args[0]) + ".\n");
        return 0;
    }

    // For lieges, restrict the use of restrict (pun!) to domain members.
    dom = SECURITY->query_wiz_dom(who);
    if (args[0] != "list" &&
        SECURITY->query_wiz_rank(who) < WIZ_ARCH &&
        (dom != SECURITY->query_wiz_dom(args[0])))
    {
        notify_fail("You can only restrict wizards in your own domain.\n");
        return 0;
    }

    if (args[0] == "list")
    {
        wlist = sort_array(filter(SECURITY->query_wiz_list(-1), 
                                  &operator(!=)(0) @ SECURITY->query_restrict));
        if (SECURITY->query_wiz_rank(who) == WIZ_LORD ||
            SECURITY->query_wiz_rank(who) == WIZ_STEWARD)
            wlist = filter(wlist, &operator(==)(dom) @ SECURITY->query_wiz_dom);
        wlist = filter(wlist, &operator(<=)(WIZ_NORMAL) @ SECURITY->query_wiz_rank);
        if (sizeof(wlist) == 0)
            write("No restricted wizards.\n");
        else
        {
            for (i = 0, sz = sizeof(wlist) ; i < sz ; i++)
            {
                write(sprintf("%-11s%-11s", capitalize(wlist[i]), SECURITY->query_wiz_dom(wlist[i])));
                res = SECURITY->query_restrict(wlist[i]);
                if (res & RESTRICT_SNOOP)
                    write("S   ");
                if (res & RESTRICT_SNOOP_DOMAIN)
                    write("SD  ");
                if (res & RESTRICT_RW_HOMEDIR)
                    write("RW  ");
                if (res & RESTRICT_NO_W_DOMAIN)
                    write("W   ");
                if (res & RESTRICT_LOG_COMMANDS)
                    write("LOG ");
                if (res & RESTRICT_STAT)
                    write("STAT");
                write("\n");
            }
        }
        return 1;
    }

    res = SECURITY->query_restrict(args[0]);
    if (sizeof(args) == 1)
    {
        write(sprintf("%-11s%-11s", capitalize(args[0]), SECURITY->query_wiz_dom(args[0])));
        res = SECURITY->query_restrict(args[0]);
        if (res & RESTRICT_SNOOP)
            write("S   ");
        if (res & RESTRICT_SNOOP_DOMAIN)
            write("SD  ");
        if (res & RESTRICT_RW_HOMEDIR)
            write("RW  ");
        if (res & RESTRICT_NO_W_DOMAIN)
            write("W   ");
        if (res & RESTRICT_LOG_COMMANDS)
            write("LOG ");
        if (res & RESTRICT_STAT)
            write("STAT");
        if (res == 0)
            write("No restrictions");
        write("\n");
    }
    else
    {
        switch (lower_case(args[1]))
        {
        case "snoop":
        case "s":
            setres = RESTRICT_SNOOP;
            break;

        case "snoopdom":
        case "sd":
            setres = RESTRICT_SNOOP_DOMAIN;
            break;

        case "rw":
            setres = RESTRICT_RW_HOMEDIR;
            break;

        case "w":
            setres = RESTRICT_NO_W_DOMAIN;
            break;

        case "log":
            setres = RESTRICT_LOG_COMMANDS;
            break;

        case "stat":
            setres = RESTRICT_STAT;
            break;

        case "all":
            setres = RESTRICT_ALL;
            break;

        case "none":
            setres = res;
            break;

        default:
            notify_fail("restrict: Unknown restriction.\n");
            return 0;
        }
            
        if ((res & setres) == 0 &&
            SECURITY->query_wiz_rank(args[0]) > WIZ_MAGE)
        {
            notify_fail("It's not suitable to restrict a wizard of this level.\nPlease use the demote command instead.\n");
            return 0;
        }

        // Handle snoop/snoopdom conflict
        if (setres == RESTRICT_SNOOP && (res & RESTRICT_SNOOP_DOMAIN) != 0)
            SECURITY->remove_restrict(args[0], RESTRICT_SNOOP_DOMAIN);
        if (setres == RESTRICT_SNOOP_DOMAIN && (res & RESTRICT_SNOOP) != 0)
            SECURITY->remove_restrict(args[0], RESTRICT_SNOOP);

        // Handle w/rw conflict
        if (setres == RESTRICT_RW_HOMEDIR && (res & RESTRICT_NO_W_DOMAIN) != 0)
            SECURITY->remove_restrict(args[0], RESTRICT_NO_W_DOMAIN);
        if (setres == RESTRICT_NO_W_DOMAIN && (res & RESTRICT_RW_HOMEDIR) != 0)
            SECURITY->remove_restrict(args[0], RESTRICT_RW_HOMEDIR);

        // Actually toggle the restriction bit
        if (res & setres == 0)
            SECURITY->set_restrict(args[0], setres);
        else
            SECURITY->remove_restrict(args[0], setres);

        // Display the result
        restrict(args[0]);
        write("Ok.\n");
    }

    return 1;
}

/* **************************************************************************
 * retire - Retire from active wizhood.
 */
nomask int
retire(string str)
{
    CHECK_SO_WIZ;

    if (str != "from coding")
    {
        notify_fail("If you want to retire yourself from coding, type " +
            "'retire from coding'.\n");
        return 0;
    }

    return SECURITY->retire_wizard();
}

/* **************************************************************************
 * runlog - list a runtime log
 */
nomask int
runlog(string str)
{
    return somelog(str, "runtime log", "/runtime");
}

/* **************************************************************************
 * sdoc - extract documentation from file(s)
 */
nomask int
sdoc(string str)
{
    string *argv, *files, *parts, path;
    int argc, i;
    mixed *ord;

    if (!str)
    {
        notify_fail("What do you want to document, see 'help sdoc'\n");
        return 0;
    }

    argv = explode(str, " "); 
    argc = sizeof(argv);

    switch (argv[0])
    {
    case "-r":
        DOCMAKER->doc_reset();
        write("Ok.\n");
        return 1;
        break;
    case "-?":
        write(DOCMAKER->doc_query_status());
        ord = DOCMAKER->doc_query_orders();
        for (i = 0; i < sizeof(ord); i++)
        {
            write(ord[i][1] + " in " + ord[i][0] + " by: " +
                  implode(ord[i][2], ", ") + "\n");
        }
        return 1;
        break;
    default:
        if (argc < 2)
        {
            notify_fail("Syntax error, see 'help sdoc'\n");
            return 0;
        }

        argv[1] = FTPATH(this_player()->query_path(), argv[1]);
        if (sizeof(explode(" " + argv[1] + " ", "*")) == 1)
        {
            files = ({ argv[1] });
            path = "";
        }
        else
        {
            files = get_dir(argv[1]);

            if (sizeof(files))
                files -= ({ ".", ".." });

            if (sizeof(parts = explode(argv[1], "/")) > 1)
            {
                path = "/" + implode(parts[0..sizeof(parts) - 2], "/") + "/";
            }
            else 
                path = "/";
        }
        
        if (!sizeof(files))
        {
            notify_fail("No such file(s): " + argv[1] + "\n");
            return 0;
        }
                
        write("The Docscribe will be told of your request.\n");

        for (i = 0; i < sizeof(files); i++)
        {
            if (file_size(path + files[i]) != -2)
                DOCMAKER->doc_file(argv[0], path + files[i]);
            else
                write(path + files[i] + " is a directory.\n");
        }
        return 1;
        
        break;
    }
}

/* **************************************************************************
 * shutdown - shut the game down
 */
nomask int
shutdown_game(string str)
{
    string *argv;
    int     grace;
    
    CHECK_SO_WIZ;

    if (!stringp(str))
    {
        write("No arguments specified. Do 'help shutdown' for help.\n");
        return 1;
    }

    argv = explode(str, " ");
    if (sizeof(argv) == 1)
    {
        switch(argv[0])
        {
        case "abort":
            SECURITY->cancel_shutdown();
            return 1;

        case "runlevel":
            if (grace = SECURITY->query_runlevel())
            {
                write("Runlevel: " + WIZ_RANK_NAME(grace) + " (and higher).\n");
            }
            else
            {
                write("The game is currently open to all players.\n");
            }
            return 1;
        }

        notify_fail("Invalid argument to shutdown: " + str +
            "\nDo 'help shutdown' for help.\n");
        return 0;
    }

    grace = atoi(argv[0]);
    if ((grace >= 1) ||
        (argv[0] == "now"))
    {
        SECURITY->request_shutdown(implode(argv[1..], " "), grace);
    }

    if (argv[0] == "runlevel")
    {
        if (argv[1] == "all")
        {
            argv[1] = WIZNAME_MORTAL;
        }

        if ((grace = member_array(argv[1], WIZ_N)) == -1)
        {
            if ((grace = member_array(argv[1], WIZ_S)) == -1)
            {
                notify_fail("There is no rank called '" + argv[1] + "'.\n");
                return 0;
            }
        }

        /* Runlevel keeper -> runlevel arch. */
        SECURITY->set_runlevel((grace == WIZ_KEEPER) ? WIZ_ARCH : grace);
        write("Runlevel changed to " + argv[1] + " (and higher).\n");
        return 1;
    }

    notify_fail("Invalid argument to shutdown: " + str +
        "\nDo 'help shutdown' for help.\n");
    return 0;
}

/* **************************************************************************
 * snoop - snoop someone
 */
nomask int
snoop_on(string str)
{
    object on, by;
    string *argv;
    int    argc;

    CHECK_SO_WIZ;

    /* No argument, try to terminate the current snoop. */
    if (!stringp(str))
    {
        if (snoop(this_interactive()))
        {
            write("Snoop terminated.\n");
        }
        else
        {
            write("Failed to terminate snoop.\n");
        }
        return 1;
    }

    argv = explode(str, " ");

    if (sizeof(argv) == 1)
    {
        on = find_player(argv[0]);
        if (!on)
        {
            write("No such player.\n");
            return 1;
        }

        if ((object)SECURITY->query_snoop(on))
        {
            write(capitalize(argv[0]) + " is already snooped.\n");
            return 1;
        }

        if (snoop(this_interactive(), on))
        {
            write("Snooping " + capitalize(argv[0]) + ".\n");
            return 1;
        }
        else
        {
            write("Failed to snoop " + capitalize(argv[0]) + ".\n");
        }

        return 1;
    }

    if (argv[0] == "break")
    {
        if (WIZ_CHECK < WIZ_ARCH)
        {
            notify_fail("The command \"snoop break\" is not valid for you. " +
                "To stop snooping, simply type \"snoop\".\n");
            return 0;
        }
        on = find_player(argv[1]);
        if (!on)
        {
            write("No such player.\n");
            return 1;
        }
        if (snoop(on))
        {
            tell_object(on, capitalize((string)this_interactive()->
                query_real_name()) + " broke your snoop.\n");
            write("Broken snoop by " + capitalize(argv[1]) + ".\n");
        }
        else
        {
            write("Failed to break snoop on " + capitalize(argv[1]) + ".\n");
        }
        return 1;
    }

    by = find_player(argv[0]);
    on = find_player(argv[1]);
    if (!on)
    {
        write("No such player: " + capitalize(argv[0]) + ".\n");
        return 1;
    }
    if (!by)
    {
        write("No such player: " + capitalize(argv[1]) + ".\n");
        return 1;
    }
    if (snoop(by, on))
    {
        tell_object(by, "You are now snooping " + capitalize(argv[1]) + ".\n");
        write(capitalize(argv[0]) + " is now snooping " +
              capitalize(argv[1]) + ".\n");
    }
    else
    {
        write("Failed to set snoop on " + capitalize(argv[1]) + " by " +
              capitalize(argv[0]) + ".\n");
    }

    return 1;
}

#define STAT_PLAYER      0x1
#define STAT_LIVING      0x2
#define STAT_OBJECT      0x4

/*
 * Function name: show_stat
 * Description  : This function actually prints the stat-information about
 *                the object to the wizard.
 * Arguments    : object ob   - the object to stat.
 *                int    what - flags to see what information to display.
 * Returns      : int 1 - always.
 */
nomask int
show_stat(object ob, int what)
{
    string tmp, str = "";
    /*
    if (what & STAT_PLAYER)
        str += ob->stat_playervars();
     */
    if (what & STAT_LIVING)
        str += ob->stat_living();
    if (what & STAT_OBJECT)
        str += ob->stat_object();
    write(str);
    return 1;
}

/* **************************************************************************
 * stat - displays stats of something
 */
nomask int
stat(string str)
{
    object ob;
    int res;
    int flags;

    CHECK_SO_WIZ;

    if (!stringp(str))
    {
        return show_stat(this_interactive(), STAT_PLAYER | STAT_LIVING);
    }

    if (ob = find_player(str))
    {
        flags = STAT_PLAYER | STAT_LIVING;
    }
    else if (ob = parse_list(str))
    {
        if (IS_PLAYER_OBJECT(ob))
        {        
             flags = STAT_PLAYER | STAT_LIVING;
        }
        else if (living(ob))
        {
             flags = STAT_LIVING;
        }
        else
        {
             flags = STAT_OBJECT;
        }
    }
    else if (ob = find_living(str))
    {
        flags = STAT_LIVING;
    }
    else if (ob = (object)SECURITY->finger_player(str))
    {
        flags = STAT_PLAYER | STAT_LIVING;
        // Remove the temporary finger player object after we're done with it
        set_alarm(0.0, 0.0, &ob->remove_object());
    }

    if (!ob)
    {
        notify_fail("No such thing.\n");
        return 0;
    }

    res = SECURITY->query_restrict(this_interactive()->query_real_name());
    if (res & RESTRICT_STAT)
    {
        // A restricted wizard may not stat other players, only himself.
        if (flags & STAT_PLAYER)
        {
            if (!ob->query_wiz_level() && !wildmatch("*jr", ob->query_real_name()))
            {
                write("You are currently restricted from using the stat " +
                    "command on mortal players (except juniors).\n");
                return 1;
            }
        }

        // A restricted wizard may only stat objects for which he has read
        // permission
        if (!SECURITY->valid_read(MASTER_OB(ob), this_interactive(), "stat"))
        {
            write("You may only stat objects for which you can read the " +
                "source code.\n");
            return 1;
        }
    }

    return show_stat(ob, flags);
}

nomask int
skillstat(string str)
{
    object ob;

    CHECK_SO_WIZ;

    if (!stringp(str))
    {
        ob = this_interactive();
    }
    else if ((!objectp(ob = find_player(str))) &&
        (!(objectp(ob = present(str, environment(this_interactive())))) ||
            (!living(ob))) &&
        (!objectp(ob = find_living(str))))
    {
        notify_fail("No such living thing.\n");
        return 0;
    }

    write(ob->skill_living());
    return 1;
}

/* **************************************************************************
 * teams - list info about all present teams
 */

/*
 * Function name: team_member_description
 * Description  : Print the name and idle/ld status of a team member.
 * Arguments    : object player - the player object
 * Returns      : string - the name of the player with idle/ld status.
 */
static string
team_member_description(object player)
{
    string name = capitalize(player->query_real_name());
    int idle;

    if (!interactive(player))
    {
        name += " (LD)";
    }
    else if ((idle = query_idle(player)) > 60)
    {
        name += " (" + TIME2STR(idle, 1) + ")";
    }
    return name;
}

nomask int
teams(string str)
{
    object *players;
    object *members;
    int index;
    int num_teams;

    CHECK_SO_WIZ;

    players = users();
#ifdef STATUE_WHEN_LINKDEAD
#ifdef OWN_STATUE
    /* If there is a room where statues of linkdead people can be found,
     * we add that to the list.
     */
    if (objectp(find_object(OWN_STATUE)))
    {
        players += ((object *)find_object(OWN_STATUE)->query_linkdead_players() - players);
    }
#endif OWN_STATUE
#endif STATUE_WHEN_LINKDEAD
    num_teams = 0;
    index = sizeof(players);
    while(--index >= 0)
    {
        members = players[index]->query_team();
        if (!sizeof(members))
        {
            continue;
        }
        num_teams++;
        write(HANGING_INDENT(sprintf("%-11s %d %s %s",
            capitalize(players[index]->query_real_name()), sizeof(members),
            ((sizeof(members) == 1) ? "member :": "members:"),
            COMPOSITE_WORDS(map(members, team_member_description))),
            23, 0));
    }
    write("There " + ((num_teams == 1) ? "is " : "are ") +
        LANG_WNUM(num_teams) + " teams in the realms.\n");
    return 1;
}

/* **************************************************************************
 * tellall - tell something to everyone
 */
nomask int
tellall(string str)
{
    string name = this_player()->query_real_name();
    int index = -1;
    int size;
    object *list;
    string *names;

    /* We do NOT add the wizard-check here since Armageddon should be
     * able to use the function.
     */

    if (!stringp(str))
    {
        notify_fail("Tellall what?\n");
        return 0;
    }

    list = users() - ({ this_player(), 0 });
    list -= (object *)QUEUE->queue_list(0);
    size = sizeof(list);
    while(++index < size)
    {
        if (list[index]->query_wiz_level())
        {
            tell_object(list[index], capitalize(name) +
                " tells everyone: " + str + "\n");
        }
        else
        {
            tell_object(list[index], "An apparition of " + 
                this_player()->query_art_name(list[index]) +
                " appears to you.\n" +
                capitalize(this_player()->query_pronoun()) +
                " tells everyone: " + str +
                "\nThe figure then disappears.\n");
        }

        names = list[index]->query_prop(PLAYER_AS_REPLY_WIZARD);
        if (pointerp(names))
        {
            names = ({ name }) + (names - ({ name }) );
        }
        else
        {
            names = ({ name });
        }
        list[index]->add_prop(PLAYER_AS_REPLY_WIZARD, names);
    }

    if (this_player()->query_option(OPT_ECHO))
        write("You tell everyone: " + str + "\n");
    else
        write("Ok.\n");
    return 1;
}

/* **************************************************************************
 * toolsoul - affect your tool souls
 */
nomask int
toolsoul(string str)
{
    string *tool_list;
    int index, size;

    CHECK_SO_WIZ;

    tool_list = (string *)this_interactive()->query_tool_list();
    if (!stringp(str))
    {
        index = -1;
        size = sizeof(tool_list);
        write("Current tool souls:\n");
        while(++index < size)
        {
            write(sprintf("%2d: %s\n", (index + 1), tool_list[index]));
        }
        return 1;
    }

    str = FTPATH((string)this_interactive()->query_path(), str);

    if (member_array(str, tool_list) >= 0)
    {
        if (this_interactive()->remove_toolsoul(str))
            write("Removed " + str + ".\n");
        else
            write("Failed on " + str + ".\n");
        return 1;
    }

    if (this_interactive()->add_toolsoul(str))
        write("Added " + str + ".\n");
    else
        write("Failed on " + str + ".\n");
    return 1;
}

/* **************************************************************************
 * tradestat - Stat the trade in a room
 */
nomask int
tradestat()
{
    if (!function_exists("stat_trade", environment(this_player())))
    {
        notify_fail("No obvious trade in this room.\n");
        return 0;
    }

    write(environment(this_player())->stat_trade());
    return 1;
}

/* **************************************************************************
 * trans - teleport someone somewhere
 */
nomask int
trans(string str)
{
    object  ob;
    mixed   room;
    string *args;
    int reverse;

    CHECK_SO_WIZ;

    if (!stringp(str))
    {
        notify_fail("Trans who?\n");
        return 0;
    }
    
    args = explode(str, " ");
    if (args[0] == "-r")
    {
        if (sizeof(args) == 1)
        {
            notify_fail("Trans who back?\n");
            return 0;
        }

        args = args[1..sizeof(args)];
        reverse = 1;
    }

    ob = find_living(args[0]);
    if (!objectp(ob))
    {
        notify_fail("No such living thing.\n");
        return 0;
    }

    if (!reverse)
    {
        tell_object(ob, "You feel yourself magically transferred.\n");
        room = MASTER_OB(environment(ob));
        ob->add_prop(PLAYER_S_TRANSED_FROM, room);
        if (this_player()->query_option(OPT_ECHO))
        {
            write("Transed from: " + room + "\n");
        }
        if (sizeof(args) > 1)
        {
            ob->move_living("X",
                FTPATH(this_interactive()->query_path(), args[1]));
        }
        else
        {
            ob->move_living("X", environment(this_interactive()));
        }
    }
    else
    {
        if (!(strlen(room = ob->query_prop(PLAYER_S_TRANSED_FROM))))
        {
            write(capitalize(args[0]) +
                " was not transed. Using previous location instead.\n");
            room = ob->query_prop(LIVE_O_LAST_ROOM);
        }
        if (!room)
        {
            notify_fail(" No location to trans " + capitalize(args[0]) +
                " back to.\n");
            return 0;
        }
        
        tell_object(ob, "You feel yourself magically transferred.\n");
        if (this_player()->query_option(OPT_ECHO))
        {
            write("Transed to: " +
                (objectp(room) ? file_name(room) : room ) + "\n");
        }
        ob->move_living("X", room);
        ob->remove_prop(PLAYER_S_TRANSED_FROM);
    }

    return 1;
}


/* **************************************************************************
 * typolog - read the typolog
 */
nomask int
typolog(string str) 
{ 
    return somelog(str, "typo log", "/typos"); 
}

/* **************************************************************************
 * vis - become visible
 */
nomask int
vis()
{
    CHECK_SO_WIZ;

    if (!(this_player()->query_invis()))
    {
        notify_fail("You are not invisible.\n");
        return 0;
    }

    this_player()->set_invis(0);
    write("You are now visible.\n");
    say(QCNAME(this_player()) + " " + this_player()->query_mm_in() + "\n");
    return 1;
}

/*
 * Function name: somelog
 * Description:   Look at error logs
 */
static int
somelog(string str, string logname, string log)
{
    string file;
    int remove;

    if (!strlen(str))
    {
        str = (string)this_interactive()->query_real_name();
    }
    if (remove = wildmatch("-r*", str))
    {
        str = extract(str, 3);
    }
    
    file = (string)SECURITY->query_wiz_path(str) + "/log";
    if (!strlen(file))
    {
        return (notify_fail("No " + logname + " for: " + str + ".\n"), 0);
    }

    file += log;
    if (file_size(file) < 0)
    {
        return (notify_fail("Can't find: " + file + ".\n"), 0);
    }

    if (remove)
    {
        write("Removing " + capitalize(logname) + ": " + file + "\n");
        this_interactive()->command("rm " + file);
    }
    else
    {
        write(capitalize(logname) + ": " + file + "\n");
        this_interactive()->command("tail " + file);
    }

    /* For the runtime log, tell wizards where to find trace info. */
    if (log == "/runtime")
    {
        write("For trace information, see /" +
            lower_case(SECURITY->get_mud_name()) + ".debug.log\n");
    }
    return 1;
}

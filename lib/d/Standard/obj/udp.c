/*
 * Copyright notice, please read.
 *
 * Copyright Chalmers Datorf|rening, 1992
 *
 * You are welcome to read this file for information & education purposes.
 * However, you are expressly forbidden to copy this code for use in any
 * other mud. This code is part of Standard and we want Standard to remain
 * unique. You must invent your own world on your own mud.
 */

#pragma save_binary
#pragma strict_types

inherit "/sys/global/udp";

#include <composite.h>
#include <files.h>
#include <macros.h>
#include <std.h>
#include <time.h>
#include <udp.h>

string *allowed_muds;
int     is_allowed;

/*
 * These are the muds we allow some things to we do not allow to other
 * muds.
 */
#define GWIZ_ALLOWED \
	({ "Standard_test", "Kingdoms", "Plughed", "GondorMud", \
	   "Angalon", "Synergy", "Muddy_Waters" })
#define CHECK_ALLOWED is_allowed = (member_array(p["NAME"], allowed_muds) >= 0)

void
create()
{
    ::create();

    allowed_muds = GWIZ_ALLOWED;
    is_allowed = 0;
}

string
short()
{
    return "the intermud master";
}

void 
incoming_udp(string host, string msg)
{
    string name, said_host, t;
    int p;

    /* For relayed packages we must extract the correct info
     */
#ifdef Standard
    if (sscanf(msg,"RELAY(%s)%s", said_host, t) == 2)
    {
	::incoming_udp(said_host, t);
    }
    else
#endif
	::incoming_udp(host, msg);
}

int
startup(mapping p)
{
    string a,b;

    /* Just log if the name does not contain 'test'
     */
    if (sscanf(" " + p["NAME"]+ " ", "%stest%s", a, b) != 2)
	log_file("UDP_startups", sprintf("%-25s %s (%s:%s:%s)\n", 
					 p["NAME"], ctime(time()),
					 p["HOSTADDRESS"], p["PORT"],
					 p["PORTUDP"]), -1);
    return ::startup(p);
}

int 
gwizmsg(mapping p)
{
    CHECK_ALLOWED;

    if (!is_allowed)
    {
	return 0;
    }

/*
    log_file("UDP_gwiz", extract(ctime(time()), 4, 18) + " @ " +
	     p["WIZNAME"] + "." + capitalize(lower_case(p["NAME"])) + ": " +
	     p["GWIZ"] + "\n");
*/
    return ::gwizmsg(p);
}

/*
 * Function name: finger_player
 * Description  : This function masks the finger_player() function since
 *                we do not want to give out information about the players
 *                of Standard to others.
 * Arguments    : string player - the player to give information about.
 * Returns      : string - the text to display.
 */
string
finger_player(string player)
{
    object obj;
    string str = "GFinger on " + capitalize(player) + " on " +
	SECURITY->get_mud_name() + " (GameDriver " +
	SECURITY->do_debug("version") + ", Mudlib " + MUDLIB_VERSION + ")\n";

    if (is_allowed)
    {
	player = lower_case(player);
	if (objectp(obj = find_player(player)))
	{
	    str += capitalize(player) + " is currently in the game.\n";
	}
	else
	{
	    str += capitalize(player) + " is not in the game.\nLast logout " +
		CONVTIME(time() - file_time(PLAYER_FILE(player) + ".o")) +
		" ago.\n";
	}
    }
    else
    {
	str += "Sorry. We do not give finger-information on players or " +
	    "wizards.\n";
    }

    return str;
}

int
gfinger_q(mapping p)
{
    CHECK_ALLOWED;

    return ::gfinger_q(p);
}

/*
 * Function name: rwho_message
 * Description  : This function returns all the rwho information we want
 *                to distribute. We do not want to distribute names as they
 *                are subject to a met/nonmet system inside the game.
 * Returns      : string - the message.
 */
public string
rwho_message()
{
    object *players = users();
    string *names;
    string str = "RWho on " + SECURITY->get_mud_name() + " (GameDriver " +
	SECURITY->do_debug("version") + ", Mudlib " + MUDLIB_VERSION + ")\n";
    int size;

    players = filter(players, &->query_wiz_level());
    size = sizeof(players);
    if (size)
    {
	str += "Wizards : " + size + "\n";

	if (is_allowed)
	{
	    names = map(players, &->query_real_name());
	    names = map(sort_array(names), capitalize);
	    str += break_string(COMPOSITE_WORDS(names), 70, "          ") +
		"\n";
	}
    }

    players = users() - players;
    players = filter(players, &->query_average_stat());
    size = sizeof(players) - size;
    if (size > 0)
    {
	str += "Players : " + size + "\n";

	if (is_allowed)
	{
	    names = map(players, &->query_real_name());
	    names = map(sort_array(names), capitalize);
	    str += break_string(COMPOSITE_WORDS(names), 70, "          ") +
		"\n";
	}
    }

    size = QUEUE->query_queue();
    if (size)
    {
	str += "Queueing: " + size + "\n";
    }

    return str;
}

int
rwho_q(mapping p)
{
    CHECK_ALLOWED;

    return ::rwho_q(p);
}

/*
  udp.c

  This file is supposed to be inherited by the udp manager of the mud.
  It implements the basics of the intermud communication protocol.

  It is not supposed to be cloned, but to reside only as the master.

  As from CD.00.26 this file is distributed as an active UDP_MANAGER,
  however as it implements only the basics of the intermud protocol,
  no special services are available.
*/

#pragma save_binary

/*
   Standard udp extensions
*/
inherit "/lib/udp/rwho";		/* show wizards on other muds */
inherit "/lib/udp/support";		/* support check feature */
inherit "/lib/udp/ping";		/* ping another mud */
inherit "/lib/udp/mudlist";		/* get and send mudlist */
inherit "/lib/udp/gfinger";		/* finger wizards on other muds */
inherit "/lib/udp/gtell";		/* tell wizards on other muds */
inherit "/lib/udp/gwiz";		/* intermud wizline */

#include <macros.h>
#include <std.h>
#include <udp.h>
#include <stdproperties.h>

#define TO this_object()

mapping	known_muds;		/* The currently known muds */
mapping reverse;		/* Reverse mapping of known muds */
static string my_name;
static int my_udpport;

public string query_my_name() { return my_name; }
public int query_my_udpport() { return my_udpport; }

/*
 *  Initialize the udp manager
 */
void create()
{
    mapping muds;

#ifdef UDP_ENABLED
    setuid();
    seteuid(getuid());
    known_muds = ([]);
    reverse = ([]);
    muds = SECURITY->query_known_muds();
    if (!mappingp(muds))
	muds = ([]);
    my_name = SECURITY->get_mud_name();
    my_udpport = SECURITY->do_debug("udp_port");
    set_alarm(1800.0, 1800.0, reset);
#ifdef UDP_MUDWHO
    TO->mudwho_mudalive(1); /* 1 - marks startup */
#endif UDP_MUDWHO
    add_list(muds);
#endif UDP_ENABLED
}

#ifdef UDP_ENABLED
/*
 * Regularly check so foreign muds are still up and running
 */
void
reset()
{
    int il;
    string *ix;
    mapping p;

    /* Which muds are still up?
    */
    for (ix = m_indexes(known_muds), il = 0; il < sizeof(ix); il++)
    {
	p = known_muds[ix[il]];
	if (atoi(p["PORTUDP"]) > 0)
	{
	    if (p["MUDLIST_Q"] == 0)
	    {
		TO->send_mudlist_q(p["HOSTADDRESS"], p["PORTUDP"]);
		p["MUDLIST_Q"] = (UDP_NUM_NO_CONTACT / 2) +
		    random(UDP_NUM_NO_CONTACT / 2);
	    }
	    else
	    {
		TO->send_ping_q(p["HOSTADDRESS"], p["PORTUDP"]);
		p["MUDLIST_Q"]--;
	    }
	}

	p[UDP_NO_CONTACT] = p[UDP_NO_CONTACT] + 1;
	if (p[UDP_NO_CONTACT] > UDP_NUM_NO_CONTACT) {
	    m_delkey(reverse, p["HOSTADDRESS"] + ":" + p["PORTUDP"]);
	    m_delkey(known_muds, ix[il]);
	}
	else
	    known_muds[ix[il]] = p;
    }
}

/*
 * send_udp - Send a udp package. All packages goes through this function
 */
int 
send_udp(string host, int port, string message)
{
    if (previous_object() == this_object())
	return SECURITY->send_udp_message(host, port, message);
}

/*
 * Handle incoming udp messages
 * Return 0 if message is unknown
 */
int
incoming_udp(string host, string msg)
{
    string cmd, prm, *parts, a, b;
    mapping params;
    int il;
    
    if (sscanf(msg,"@@@%s||%s@@@", cmd, prm) != 2)
    {
	if (sscanf(msg,"@@@%s@@@", cmd) != 1)
	    return 0;
	else
	    prm = 0;
    }

    params = ([]);
    if (strlen(prm))
    {
	parts = explode(prm, "||");
	for (il = 0; il < sizeof(parts); il++)
	{
	    if (sscanf(parts[il], "%s:%s", a, b) == 2)
		params[a] = b;
	}
    }
    params["HOSTADDRESS"] = host;

    /*
     * We heard from it, reset the no connect counter
     */
    if (mappingp(known_muds[params["NAME"]]))
	known_muds[params["NAME"]][UDP_NO_CONTACT] = 0;

    return this_object()->execute_udp_command(cmd, params);
}

/*
 * set_mud_info		Store information on a mud
 * update_master_list	Update the list of known muds in master
 * query_known_muds	Gives a list of known mud names
 * query_mud_info	Gives info an a specific mud
 * 		    	A mapping with a set of parameters for 
 * 			    each mud typically:
 *         			 "NAME", "HOSTADDRESS", "PORT",
 *				 "PORTUDP", "VERSION", "TIME"
 */
void
set_mud_info(string name, mapping p)
{
    mapping q;

    if (!name || !stringp(name) ||
	!p["NAME"] || !stringp(p["NAME"]) ||
	!p["HOSTADDRESS"] || !stringp(p["HOSTADDRESS"]) ||
	!p["PORTUDP"] || !atoi(p["PORTUDP"]) ||
	!p["PORT"] || !atoi(p["PORT"]))
	return;
    q = reverse[p["HOSTADDRESS"] + ":" + p["PORTUDP"]];
    if (q)
    {
	if (!p["MUDLIB"] && q["MUDLIB"])
	    p["MUDLIB"] = q["MUDLIB"];
	if (!p["HOST"] && q["HOST"])
	    p["HOST"] = q["HOST"];
	if (!p["TIME"] && q["TIME"])
	    p["TIME"] = q["TIME"];
	if (!p["VERSION"] && q["VERSION"])
	    p["VERSION"] = q["VERSION"];
	if (islowercase(name) && name != q["NAME"])
	{
	    m_delkey(known_muds, name);
	    name = q["NAME"];
	    p["NAME"] = name;
	}
    }
    known_muds[name] = p;
    reverse[p["HOSTADDRESS"] + ":" + p["PORTUDP"]] = p;
}

void
add_list(mapping l)
{
    mapping p;
    string *ix;
    int il;

    ix = m_indexes(l);
    for (il = sizeof(ix) - 1; il >= 0; il--)
    {
	p = l[ix[il]];
	if (p["NAME"] == lower_case(my_name))
	{
	    m_delkey(l, p["NAME"]);
	    continue;
	}
	reverse[p["HOSTADDRESS"] + ":" + p["PORTUDP"]] = p;
    }
    known_muds = l;
}

int
update_masters_list()
{
    return SECURITY->set_known_muds(known_muds+([]));
}

string *
query_known_muds()
{
    return m_indexes(known_muds);
}

mixed *
query_mud_info(string mudname)
{
    return known_muds[mudname];
}

int
islowercase(string s)
{
    return lower_case(s) == s;
}

/* 
 * Execute the given commands
 */
int
execute_udp_command(string cmd, mapping params)
{
    if (member_array(cmd, UDP_SUPPORT_ARRAY) >= 0)
	return call_other(this_object(), cmd, params);
    else
	return 0;
}

/* ***************************************************************
 * startup and shutdown features
 *
 * These are called from /secure/master
 */

/*
 * Function name:  send_startup_udp
 * Description:    Send the startup message, usually to the mudserver.
 */
void 
send_startup_udp(string host, int port)
{
    TO->send_udp(host, port,
		 "@@@" + UDP_STARTUP + SECURITY->startup_udp() +
		 "@@@\n");
}


/*
 * startup - received a startup message
 */
int
startup(mapping p)
{
    if (stringp(p["NAME"]))
    {
	this_object()->ping_a(p);
	/* We could note that we had a startup message too */
	this_object()->ping_q(p);
	return 1;
    }
    return 0;
}

/*
 * Function name:  send_shutdown_udp
 * Description:    Send the shutdown message, usually to the mudserver.
 */
void 
send_shutdown_udp(string host, int port)
{
    TO->send_udp(host, port,
		 "@@@" + UDP_SHUTDOWN + SECURITY->startup_udp() +
		 "@@@\n");
#ifdef UDP_MUDWHO
    TO->mudwho_mudalive(-1);
#endif /* UDP_MUDWHO */
}

/*
 * shutdown - received a shutdown message
 */
int
shutdown(mapping p)
{
    if (stringp(p["NAME"]))
    {
	if (mappingp(known_muds) && mappingp(known_muds[p["NAME"]]))
	    m_delkey(known_muds, p["NAME"]);

	return 1;
    }
    return 0;
}

/* ************************************************
 *
 * Code for support of the MUDWHO server
 */
#ifdef UDP_MUDWHO
static 	int	last_users = 0;

public void
mudwho_mudalive(int start)
{
    string pass, name, cmt;
    int i;
    object *u;

    /*
     * This should be a secret password.
     */
    pass = TO->mudwho_get_password();
    name = TO->query_my_name();
    cmt = TO->mudwho_up_info();

    if (start == 1)
	TO->send_udp(UDP_MUDWHO, UDP_MW_PORT, UDP_MW_UP(name, pass, cmt));
    else if (start == -1)
	TO->send_udp(UDP_MUDWHO, UDP_MW_PORT, UDP_MW_DOWN(name, pass, cmt));
    else
    {
	TO->send_udp(UDP_MUDWHO, UDP_MW_PORT, UDP_MW_LIVE(name, pass, cmt));
	u = users();
	if (sizeof(u) > last_users)
	    last_users = sizeof(u);

	for (i = 0; i < last_users; i++)
	{
	    if (i >= sizeof(u))
		TO->send_udp(UDP_MUDWHO, UDP_MW_PORT, 
			 UDP_MW_USER_OUT(name, pass, "User" + i));
	    else
	    {
#ifdef UDP_MW_ANONYMOUS
		TO->send_udp(UDP_MUDWHO, UDP_MW_PORT, 
			     UDP_MW_USER_IN(name, pass, "User" + i, "Anonymous"));
#else
		cmt = capitalize(u[i]->query_real_name());
		if (cmt == 0)
		    cmt = "Unknown";
		TO->send_udp(UDP_MUDWHO, UDP_MW_PORT, 
			     UDP_MW_USER_IN(name, pass, "User" + i, cmt));
#endif
	    }
	}
    }
    if (start == 1)
	TO->mudwho_mudalive(0);
    else
	set_alarm(1.0, 0.0, &mudwho_mudalive(UDP_MW_REPEAT_TIME));
}

/*
 * This should be redefined to return a secret password that is not
 * readable by normal wizards.
 */
public string
mudwho_get_password()
{
    return "passord";
}

public string
mudwho_up_info()
{
    return "(" + SECURITY->do_debug("version") + ":" + MUDLIB_VERSION + ")";
}
#endif /* UDP_MUDWHO */

#endif /* UDP_ENABLED */

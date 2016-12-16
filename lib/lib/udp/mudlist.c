/*
  lib/udp/mudlist.c

  Routines for getting a mudlist from another mud

  This code is meant to be shared among all muds needing his feature.
*/
#pragma strict_types
#pragma save_binary

#include <std.h>
#include <files.h>
#include <udp.h>
#include <composite.h>
#include <options.h>

#define TO this_object()

/*
 * Build a mudlist
 */
static string 
build_mudlist(string *names)
{
    string ret;
    mapping p;
    int il;

    ret = "";

    for (il = 0; il < sizeof(names); il++)
    {
	p = TO->query_mud_info(names[il]);
	ret += "||" + il + ":" +
	    "|NAME:" + names[il] + 
	    "|HOST:" + p["HOST"] +
	    "|HOSTADDRESS:" + p["HOSTADDRESS"] +
	    "|PORT:" + p["PORT"] +
	    "|PORTUDP:" + p["PORTUDP"];
    }
    return ret;
}

/*
 * Send a mudlist query package
 */
void
send_mudlist_q(string host, mixed port)
{
    if (stringp(port))
	port = atoi(port);

    TO->send_udp(host, port,
		 "@@@" + UDP_MUDLIST_Q +
		 "||NAME:" + TO->query_my_name() +
		 "||PORTUDP:" + TO->query_my_udpport() +
		 "@@@\n");
}
    
    
/*
 * mudlist_q
 */
int
mudlist_q(mapping p)
{
    string *names;
    int il;

    if (p["PORTUDP"])
    {
	names = TO->query_known_muds();
	for (il = 0; il < sizeof(names); il+=5)
	    TO->send_udp(p["HOSTADDRESS"], atoi(p["PORTUDP"]), 
			 "@@@" + UDP_MUDLIST_A +
			 build_mudlist(names[il..il+4]) + "@@@\n");
	return 1;
    }
    return 0;
}

/*
 * mudlist_a
 */
int mudlist_a(mapping p)
{
    int il, il2;
    string *ix, *inf, a, b;
    mapping q, r;

    if (!m_sizeof(p))
	return 0;

    ix = m_indexes(p);

    for (il = 0; il < sizeof(ix); il++)
    {
	inf = explode(p[ix[il]], "|");
	q = ([]);
	for (il2 = 0; il2 < sizeof(inf); il2++)
	{
	    if (sscanf(inf[il2], "%s:%s", a, b) == 2)
		q[a] = b;
	}
	if (!stringp(q["NAME"]) ||
	    q["NAME"] == TO->query_my_name() ||
	    q["NAME"] == lower_case(TO->query_my_name()))
	    continue;
	r = TO->query_mud_info(q["NAME"]);
	if (!mappingp(r))
	    TO->set_mud_info(q["NAME"], q);
	else
	{
	    inf = m_indexes(q);
	    for (il2 = 0; il2 < sizeof(inf); il2++)
		r[inf[il2]] = q[inf[il2]];
	    TO->set_mud_info(q["NAME"], r);
	}
    }

    return 1;
}

/* 
 * cmd_mudlist - List the muds known to us
 */
int
cmd_mudlist(string str)
{
    mapping p;
    string *names, flag, mudn, m;
    int il;

    names = sort_array(TO->query_known_muds());

    if (!str)
    {
	if (sizeof(names)) {
	    write(sizeof(names) + " known muds:\n" + 
		  break_string(COMPOSITE_WORDS(names) + ".\n",
			       this_player()->query_option(OPT_SCREEN_WIDTH)-10,
			       3));
	}
	else
	    write("Currently no known other muds.\n");
	return 1;
    }

    if (sscanf(str, "-%s", flag) != 1)
    {
	mudn = str;
	flag = "none";
    }
    else 
    {
	str = flag;
	if (sscanf(str, "%s %s", flag, mudn) != 2)
	{
	    flag = str;
	    mudn = 0;
	}
    }

    switch(flag)
    {
    case "all":
	write(sizeof(names) + " known muds:\n");
	for (il = 0; il < sizeof(names); il++)
	{
	    p = TO->query_mud_info(names[il]);
	    write(names[il] + ": " + p["HOST"] + "(" + p["HOSTADDRESS"] +
		  ") " + p["PORT"] + "\n");
	}
	break;
    case "query":
	if (!this_interactive() ||
	    WIZ_CHECK <= WIZ_LORD)
	    return 0;
	if (member_array(mudn, names) >=0)
	{
	    p = TO->query_mud_info(mudn);
	    TO->send_mudlist_q(p["HOSTADDRESS"], p["PORTUDP"]);
	    write("Ok.\n");
	    return 1;
	}
	else
	{
	    write("No such known mud: " + mudn + "\n");
	    return 1;
	}
	break;
    case "none":
	if (member_array(mudn, names) >=0)
	{
	    p = TO->query_mud_info(str);
	    m = mudn + ": " + p["HOST"] + "(" + p["HOSTADDRESS"] + ") " +
		p["PORT"] + " Ver: " + p["VERSION"] + " : " + p["MUDLIB"] +
		" Start: " + p["TIME"];
	    if (!this_interactive() ||
		WIZ_CHECK > WIZ_LORD)
		m += " Udp: " + p["PORTUDP"];

	    write(break_string(m,
			       this_player()->query_option(OPT_SCREEN_WIDTH) - 4,
			       0) + "\n");
	}
	else
	{
	    write("No such known mud: " + mudn + "\n");
	    return 1;
	}
	break;
    }
    return 1;
}

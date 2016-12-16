/*
  udp/gwiz.c

  Routines for implementing an intermud wizline across muds

  This code is meant to be shared among all muds needing his feature.
*/
#pragma strict_types
#pragma save_binary

#include <udp.h>
#include <stdproperties.h>
#include <composite.h>

#define TO this_object()


/*
 * This can be called from any soul or whatever
 * Returns: List of muds the message was sent to.
 */
string *
send_gwizmsg(string wizname, string msg, int emote)
{
    string *ix, smsg, *muds;
    int il;
    mapping p;

    ix = TO->query_known_muds();
    for (muds = ({}), il = 0; il < sizeof(ix); il++)
    {
	p = TO->query_mud_info(ix[il]);
	if (stringp(p["HOSTADDRESS"]) && atoi(p["PORTUDP"]))
	    TO->send_udp(p["HOSTADDRESS"], atoi(p["PORTUDP"]), 
			 "@@@" + UDP_GWIZMSG +
			 "||NAME:" + TO->query_my_name() +
			 "||PORTUDP:" + TO->query_my_udpport() +
			 "||WIZNAME:" + capitalize(wizname) +
			 "||EMOTE:" + emote +
			 "||GWIZ:" + msg +  "@@@\n");
	muds += ({ p["NAME"] });
    }
    if (emote)
	TO->tell_wizards(capitalize(wizname) + "." + TO->query_my_name()
			 + " " + msg);
    else
	TO->tell_wizards(capitalize(wizname) + "." + TO->query_my_name()
			 + ": " + msg);
    return muds;
}

/*
 * Got a message
 */
int 
gwizmsg(mapping p)
{
    string smsg;
    mapping mi;

    if (stringp(p["NAME"]) && 
	stringp(p["PORTUDP"]))
    {
	if (p["NAME"] == TO->query_my_name())
	    return 0;

	mi = TO->query_mud_info(p["NAME"]);

	/* 
	    If the mud is unknown to us we might as well ping it to get
	    some relevant info. We do not accept messages from unknown
	    muds.
	*/
	if (!mappingp(mi))
	{
	    TO->send_ping_q(p["HOSTADDRESS"], p["PORTUDP"]);
	    return 0;
	}
	else if (!stringp(p["WIZNAME"]))
	    return 0;

    if (p["EMOTE"] && atoi(p["EMOTE"]))
	    smsg = p["WIZNAME"] + "." + p["NAME"] + " " + p["GWIZ"];
	else
	    smsg = p["WIZNAME"] + "." + p["NAME"] + ": " + p["GWIZ"];

	if (mi["HOSTADDRESS"] != p["HOSTADDRESS"])
	{
	    log_file("UDP_fakegwiz", sprintf("%s: %s\n@ %s\n",
					     ctime(time()), p["HOSTADDRESS"],
					     smsg));
	    TO->send_udp(mi["HOSTADDRESS"], atoi(mi["PORTUDP"]),
			 "@@@" + UDP_WARNING +
			 "||MSG:Faked gwiz message, " + smsg +
			 "||FAKEHOST:" + p["HOSTADDRESS"] +
			 "@@@");
	    return 0;
	}
	TO->tell_wizards(smsg);
	return 1;
    }
    return 0;
}

/*
 * Loop through and tell wizards the received message
 */
void 
tell_wizards(string smsg)
{
    object *u;
    int il, busy;

    u = users();
    for (il = 0; il < sizeof(u); il++)
	if (u[il] && u[il]->query_wiz_level())
	{
	    busy = u[il]->query_prop(WIZARD_I_BUSY_LEVEL);
	    if (!((busy & 256) || (busy & 128)))
		tell_object(u[il],
			    "@ " + readable_string(smsg) + "\n");
	}
}

/* **************************************************************************
 * gwiz - gozzip on the intermud wizline
 */
varargs int
cmd_gwiz(string str, int emote)
{
    int             i, cflag;
    string          name, *muds, s;

    if (!strlen(str))
    {
	notify_fail("gwiz what?\n");
	return 0;
    }
    if (sscanf(str,"-c %s", s) == 1)
    {
	str = s;
	cflag = 1;
    }
    notify_fail("Can't reach the intermud wizline, sorry.\n");

    name = capitalize(this_player()->query_real_name());
    muds = TO->send_gwizmsg(name, str, emote);
    if (cflag)
	write("Sent to: " + COMPOSITE_WORDS(muds) + "\n");

    if (!pointerp(muds))
	return 0;

    return 1;
}

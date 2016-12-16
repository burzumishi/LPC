/*
  /std/combat/unarmed.c

  Unarmed initialization routines.

  These routines can inherited by all living objects that need
  unarmed combat values of a standardized type.
 */
#pragma save_binary
#pragma strict_types

#include <formulas.h>
#include <macros.h>
#include <ss_types.h>
#include <wa_types.h>

static	mapping 	ua_attdata,     /* wchit, wcpen, damtype and %use */
			ua_hitdata;     /* acarray, %hit */
static  int 		ua_attuse;      /* %attacked used each turn */

/*
 * Prototypes
 */
public void cr_reset_attack(int aid);
public void cr_reset_hitloc(int hid);

#define QEXC (this_object()->query_combat_object())

/*
 * Description: Set data for a specific attack
 * Arguments:   aid: One of W_something (as defined in /sys/wa_types.h)
 *             wchit: Weapon class to hit
 *             wcpen: Weapon class penetration
 *	       dt:    Damage type
 *             puse:  Chance of use each turn
 *	       adesc: A string desc of the attack, ie "right claw" etc
 * 
 */
public varargs void
set_attack_unarmed(int aid, int wchit, int wcpen, int dt, int puse, 
		   string adesc)
{
    if (!mappingp(ua_attdata))
	ua_attdata = ([]);

    ua_attdata[aid] = ({ wchit, wcpen, dt, puse, adesc });

    /*
     * If we have a combat object and no weapon for this attack then
     * modify direct
     */
    if (QEXC && !objectp(QEXC->cb_query_weapon(aid)))
	this_object()->cr_reset_attack(aid);
}

public mixed *
query_ua_attack(int aid) { return ua_attdata[aid]; }

/*
 * Description: Set data for a specific hitlocation
 * Arguments:   hid: One of A_something (as defined in /sys/wa_types.h)
 *	      ac:    The ac's for a given hitlocation, can be an int
 *	      phit:  The chance that a hit will hit this location
 *	      hdesc: String describing this hitlocation, ie "head", "tail"
 */
public varargs void
set_hitloc_unarmed(int hid, int *ac, int phit, string hdesc)
{
    if (!mappingp(ua_hitdata))
	ua_hitdata = ([]);

    ua_hitdata[hid] = ({ ac, phit, hdesc });
    /*
     * If we have a combat object and no armour for this hitlocation then
     * modify direct
     */
    if (QEXC && !objectp(QEXC->cb_query_armour(hid)))
	this_object()->cr_reset_hitloc(hid);
}

public mixed *
query_ua_hitloc(int hid) { return ua_hitdata[hid]; }

/*
 * Description: Set the %attacks used each turn. 100% is one attack / turn
 * Arguments:   sumproc: %attack used
 */
public void
set_attackuse(int sumproc)
{
    ua_attuse = sumproc;
    if (QEXC)
	QEXC->cb_set_attackuse(sumproc);
}

/*
 * Description: Give the %attacks used each turn. 100% is one attack / turn
 * Returns:     %attack used
 */
public int
query_attackuse() { return ua_attuse; }

/*
 * Function name: cr_configure
 * Description:   Configures basic values for this creature.
 */
public void
cr_configure() 
{
    if (mappingp(ua_attdata))
	map(m_indexes(ua_attdata), cr_reset_attack);
    if (mappingp(ua_hitdata))
	map(m_indexes(ua_hitdata), cr_reset_hitloc);

    if (ua_attuse)
	QEXC->cb_set_attackuse(ua_attuse);
}

/*
 * Function name: cr_reset_attack
 * Description:   Set the values for a specific attack. These are called from
 *		  the external combat object.
 * Arguments:     aid: The attack id
 */
public void
cr_reset_attack(int aid)
{
    mixed att;

    if (!mappingp(ua_attdata))
    {
	ua_attdata = ([]);
    }
    
    att = ua_attdata[aid];

    if (sizeof(att) >= 4)
    {
        QEXC->cb_add_attack(att[0], att[1], att[2], att[3], aid,
            this_object()->query_skill(SS_UNARM_COMBAT));
    }
}

/*
 * Function name: cr_reset_hitloc
 * Description:   Set the values for a specific hitlocation
 * Arguments:     hid: The hitlocation (bodypart) id
 */
public void
cr_reset_hitloc(int hid)
{
    mixed hloc;

    if (!mappingp(ua_hitdata))
	ua_hitdata = ([]);
    
    hloc = ua_hitdata[hid];

    if (sizeof(hloc) >= 3)
	QEXC->cb_add_hitloc(hloc[0], hloc[1], hloc[2], hid);
}

/*
 * Function name: cr_try_hit
 * Description:   Decide if a certain attack fails because of something
 *                related to the attack itself.
 * Arguments:     aid:   The attack id
 * Returns:       True if hit, otherwise 0.
 */
public int
cr_try_hit(int aid) { return 1; }

/*
 * Function name: cr_attack_desc
 * Description:   Gives the description of a certain unarmed attack slot.
 * Arguments:     aid:   The attack id
 * Returns:       string holding description
 */
public string
cr_attack_desc(int aid)
{
    mixed att;
    
    att = ua_attdata[aid];

    if (sizeof(att) >= 5)
	return att[4];
    else
	return "mind";
}

/*
 * Function name: cr_got_hit
 * Description:   Tells us that we got hit. It can be used to reduce the ac
 *                for a given hitlocation for each hit.
 * Arguments:     hid:   The hitloc id
 *                ph:    The %hurt
 *                att:   Attacker
 *		  aid:   The attack id
 *                dt:    The damagetype
 *		  dam:   The damage in hitpoints
 */
public varargs void
cr_got_hit(int hid, int ph, object att, int aid, int dt, int dam)
{
}

/*
 * Function name:  cr_attacked_by
 * Description:    This routine is called when we are attacked or when 
 *                 someone we are hunting appears in our location. This
 *		   routine is simply a notification of the fact. The combat
 *		   system will start a fight without us doing anything here.
 * Arguments:	   ob: The attacker
 */
public void
cr_attacked_by(object ob)
{
}

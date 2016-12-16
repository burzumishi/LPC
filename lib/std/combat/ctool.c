/*
  /std/combat/ctool.c

  This is the externalized combat routines for monsters and other mobiles
  that are able to use tools for aid in combat. Typical tools are weapons
  and armours.

  This object is cloned and linked to a specific individual when
  engaged in combat. The actual object resides in 'limbo'.
*/

#pragma save_binary
#pragma strict_types

inherit "/std/combat/cplain";

#include "combat.h"
#include <wa_types.h>
#include <ss_types.h>
#include <macros.h>
#include <stdproperties.h>

/* Prototypes */
public void cb_remove_arm(object wep);
static void adjust_ac(int hid, object arm, int rm);

/* 
   NOTE
 
     There is a limited number of tool slots and one slot can only be
     occupied by one object at a time. One object may occupy many slots though.
    
     Armours can protect one or more hit locations. What hit locations
     a given armour protects is given by the function 'query_protects' in
     the armour. 

     Weapons can only aid one attack. The attack id is given by the
     function 'query_attack_id' in the weapon.

     The tool slots are used to ensure that the use of a weapon and an armour
     do not conflict. The slots that a weapon or armour occupies are given
     by the function 'query_slots' in weapons and armours.

     Observe that what attacks and hitlocation a weapon and armour is defined
     to aid is independant of what slots it allocates.

     The combat system makes no checks on that relevant tool slots aids
     relevant attacks and hit locations. This is taken care of in
     /std/armour.c and /std/weapon.c

     Tool slots are made as defined by the objects. The only thing that the
     combat system does is to ensure that two tools do not use the same 
     slot.

   MAGICAL armours

     Magical armours can protect one or more hitlocations without allocating
     a tool slot. If the property OBJ_I_IS_MAGIC_ARMOUR is defined the armour
     is considered to be a magic one. The magic armour can of course allocate
     any number of tool slots, just like a normal armour.

   MAGICAL weapons

     Magical weapons work just like normal weapons. A magical 'weapon' that
     allocates no combat slot is not a 'weapon' it is an independant magic
     object and must cause damage onto the enemy on its own. Such magic
     attacks are not supported in the combat system. 

*/

/*
 * Function name: create_cplain
 * Description:   Reset the combat functions
 */
public nomask void
create_cplain()
{
    if (me)
    {
        return;
    }

    this_object()->create_ctool();
}

/*
 * Function name: cb_calc_procuse
 * Description  : Calculates the procuse based on 2H combat skill 
 */
public void
cb_calc_attackuse()
{
    int extra = 0;

    /*
     * If more than two weapons wielded check the 2H combat, only with 2H
     * skill > 20 will it be profitable to wield 2 weapons.
     * Attack use range from 80 to 150
     */
    if (sizeof(cb_query_weapon(-1)) > 1)
    {
        extra = qme()->query_skill(SS_2H_COMBAT);
        extra = extra > 20 ? extra / 2 : extra - 20;
    }
    this_object()->cb_set_attackuse(100 + extra);    
}

/*
 * Function name: cb_wield_weapon
 * Description:   Wield a weapon. Here 'weapon' can be any object.
 * Arguments:     wep - The weapon to wield.
 * Returns:       True if wielded.
 */
public mixed
cb_wield_weapon(object wep)
{
    int il, aid, extra, size;
    object *obs;
    mixed val;

    aid = (int) wep->query_attack_id();

    /* Can we use this weapon ? */    
    if (!query_attack(aid))
    {
        return "It doesn't seem to fit your body very well.\n";
    }

    add_attack(wep->query_hit(), wep->query_modified_pen(), wep->query_dt(), 
               wep->query_procuse(), aid, 0, wep);


    cb_calc_attackuse();
    return 1;
}

/*
 * Function name: unwield
 * Description:   Unwield a weapon.
 * Arguments:     wep - The weapon to unwield.
 * Returns:       None.
 */
public void
cb_unwield(object wep)
{
    int aid = wep->query_attack_id();

    me->cr_reset_attack(aid);
    cb_calc_attackuse();
}

/*
 * Function name: adjust_ac
 * Description:   Adjust relevant hitlocations for a given armour slot
 *                when we wear an armour or remove an armour.
 * Arguments:     hid:   The hitlocation id as given in /sys/wa_types.h
 *                arm:   The armour.
 *                rm:    True if we remove armour
 */
static void
adjust_ac(int hid, object arm, int rm)
{
    int il, ac, *am, size;
    mixed *oldloc;
    object *arms;

    ac = (int)arm->query_ac(hid);
    am = (int*)arm->query_am(hid);

    size = sizeof(am);
    il = -1;
    while(++il < size)
    {
        am[il] += ac;
    }
    
    oldloc = query_hitloc(hid);

    for (il = 0; il < sizeof(am) && il < sizeof(oldloc[HIT_AC]); il++)
    {
        if (rm)
	{
            oldloc[HIT_AC][il] -= am[il];
	}
        else
	{
            oldloc[HIT_AC][il] += am[il];
	}
    }

    arms = oldloc[HIT_ARMOURS];

    if (rm)
    {
        arms = (arms ? arms - ({ arm }) : ({}));
    }
    else
    {
        arms = (arms ? arms + ({ arm }) : ({ arm }));
    }

    add_hitloc(oldloc[HIT_AC], oldloc[HIT_PHIT], oldloc[HIT_DESC], hid, arms);
}

/*
 * Function name: cb_wear_arm
 * Description:   Wear an armour
 * Arguments:     arm - The armour.
 * Returns:       True if worn, text if fail
 */
public mixed
cb_wear_arm(object arm)
{
    int *hid;

    if (arm->query_prop(OBJ_I_IS_MAGIC_ARMOUR))
    {
        hid = arm->query_shield_slots();
    }
    else
    {
        hid = arm->query_protects();   /* The hitlocations */
    }

    /* Can we use this armour ? We must define all the hitlocs it protects
     */
    if (sizeof(hid) && (sizeof(query_hitloc_id() & hid) < sizeof(hid)))
    {
        return "It seems not to fit your body very well.";
    }

    map(hid, &adjust_ac(, arm, 0));
    return 1;
}

/*
 * Function name: cb_remove_arm
 * Description:   Remove an armour
 * Arguments:     arm - The armour.
 */
public void
cb_remove_arm(object arm) 
{
    /*
     * Remove the armours effect for each hit location it protects
     */
    foreach (int hid: query_hitloc_id())
    {
        if (member_array(arm, query_hitloc(hid)[HIT_ARMOURS]) >= 0)
        {
            adjust_ac(hid, arm, 1);

            if (!sizeof(query_hitloc(hid)[HIT_ARMOURS]))
            {
                qme()->cr_reset_hitloc(hid);
            }
        }
    }
}

/*
 * Function name: cb_attack_desc
 * Description:   Gives the description of a certain attack slot.
 * Arguments:     aid:   The attack id
 * Returns:       string holding description on VBFC form. Do not use write()
 */
public string
cb_attack_desc(int aid)
{
    object wep = query_attack(aid)[ATT_OBJ];

    if (wep)
    {
        return QSHORT(wep);
    }
    else
    {
        return qme()->cr_attack_desc(aid);
    }
}

/*
 * Function name: cb_try_hit
 * Description:   Decide if a certain attack fails because of something
 *                related to the attack itself, ie specific weapon that only
 *                works some of the time. 
 * Arguments:     aid:   The attack id
 * Returns:       True if hit, otherwise 0.
 */
public int
cb_try_hit(int aid)
{
    object wep = query_attack(aid)[ATT_OBJ];

    return (wep ? wep->try_hit(cb_query_attack()) : qme()->cr_try_hit(aid));
}

/*
 * Function name: cb_did_hit
 * Description:   Tells us that we hit something. Should produce combat
 *                messages to all relevant parties. 
 * Arguments:     aid:   The attack id
 *                hdesc: The hitlocation description
 *                hid:   The hitlocation id
 *                phurt: The %hurt made on the enemy
 *                enemy: The enemy who got hit
 *                dt:    The current damagetype
 *                phit:  The %success that we made with our weapon
 *                dam:   The damamge made in hit points
 */
public varargs void
cb_did_hit(int aid, string hdesc, int hid, int phurt, object enemy, int dt,
           int phit, int dam)
{
    object wep;

    if ((!enemy) || (!qme()))
    {
        return;
    }
    
    wep = query_attack(aid)[ATT_OBJ];
    
    if (wep)
    {
        if (wep->did_hit(aid, hdesc, phurt, enemy, dt, phit, dam, hid))
    	{
    	    /* Adjust our panic level */
    	    cb_add_panic(((phurt >= 0) ? -3 - phurt / 5 : 1));
            return;
        }
    }

    ::cb_did_hit(aid, hdesc, hid, phurt, enemy, dt, phit, dam, wep);
}

/*
 * Function name: cb_got_hit
 * Description:   Tells us that we got hit. It can be used to reduce the ac
 *                for a given hitlocation for each hit.
 * Arguments:     hid:   The hitloc id
 *                ph:    The %hurt
 *                att:   Attacker
 *                aid:   The attack id
 *                dt:    The damagetype
 *                dam:   The damage done in hitpoints
 */
public varargs void
cb_got_hit(int hid, int ph, object att, int aid, int dt, int dam)
{
    int il, size;
    object *arms;

    /* 
     * Many armours may help to cover the specific bodypart: hid
     */
    arms = query_hitloc(hid)[HIT_ARMOURS];
    if (sizeof(arms))
    {
        arms->got_hit(hid, ph, att, dt, dam);
    }

    qme()->cr_got_hit(hid, ph, att, aid, dt, dam);
}

/*
 * Function namn: cb_update_armour
 * Description:   Call this function if the ac of a shielding object has changed
 * Arguments:     obj - the object which ac has changed
 */
public void
cb_update_armour(object obj)
{
    int *hids, i;
    object *arms;

    if (!obj)
    {
        return;
    }

    foreach (int hid: query_hitloc_id())
    {
        arms = query_hitloc(hid)[HIT_ARMOURS];

        if (pointerp(arms) && (member_array(obj, arms) >= 0))
	{
            me->cr_reset_hitloc(hid);
            map(arms, &adjust_ac(hid, , 0));
        }
    }
}
            
/*
 * Function namn: cb_update_weapon
 * Description:   Call this function when something has caused the weapon
 *                stats to change, skill raise or sharpening the weapon or so.
 * Arguments:     wep - The weapon
 */
public void
cb_update_weapon(object wep)
{
    if (!wep)
    {
        return;
    }

    add_attack(wep->query_hit(), wep->query_modified_pen(), wep->query_dt(),
        wep->query_procuse(), wep->query_attack_id(),
        me->query_skill(wep->query_wt()), wep);
}

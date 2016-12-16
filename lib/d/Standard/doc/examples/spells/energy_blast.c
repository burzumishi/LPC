/* This spells will hit the attacked objet with a ray of raw energy.
 * We will check both the general resistans of magic in the object and
 * the resistance against electricity.
 * With wisdom and skill the spell will be easier to cast.
 * If a success it will cause a penetration of 10.
 *
 * This is an example of a combat spell. This means that its effect are
 * not instant but incorporated into the combat rounds.
 */

#include <ss_types.h>
#include <stdproperties.h>
#include <formulas.h>
#include <macros.h>
#include <comb_mag.h>
#include <cmdparse.h>
#include <filter_funs.h>

#define CAST_LIMIT      20   /* The limit for this to become a success */
#define MANA_LIMIT      10   /* How much mana will be used */

/*
 * Prototype
 */
void tell_watcher(string str, object me, object enemy);

/* Function name: energy_blast_start_fail
 * Description:   Will prepare for casting a ray of energy on the enemy
 * Arguments:     str - string describing the enemy
 * Returns:       1 - failed and printed error, string - the error, 0 - ok
 */
nomask mixed
energy_blast_start_fail(string str)
{
    return 0;
}

/* Function name: energy_blast
 * Description:   Will setup the cast of a ray of energy on the enemy
 * Arguments:     str - string describing the enemy
 * Returns:       string - the error, 1 - ok
 */
nomask mixed
energy_blast(string str)
{
    object obj, *a;

    if (str)
    {
	a = CMDPARSE_ONE_ITEM(str, "kill_access", "kill_access");
	if (sizeof(a))
	    obj = a[0];
    }
    if (!obj)
        obj = this_player()->query_attack();
    if (!obj)
        return "Who do you want to cast this spell upon?\n";

    if (this_player()->query_ghost())
	return "You are a ghost, you cannot kill anything else.\n";

    if (NPMATTACK(obj)) /* Check if magical attack is possible on this object.*/
        return "For some strange reason you cannot cast this spell.\n";

    if (!F_DARE_ATTACK(this_player(), obj))
	return "You don't dare to cast the spell.\n";

    if (this_player()->query_mana() < MANA_LIMIT)
        return "You couldnt concetrate enough to cast this spell.\n";

    if (random(this_player()->query_skill(SS_ELEMENT_DEATH) +
        this_player()->query_stat(SS_WIS)) < random(CAST_LIMIT))
    {
        write("You failed to pronounce the words right.\n");
        this_player()->add_mana(-MANA_LIMIT / 3);
        return 1;
    }

    this_player()->add_mana(-MANA_LIMIT);

    this_player()->attack_object(obj); /* This MUST be called! */
    this_player()->add_prop(LIVE_O_SPELL_ATTACK, this_object());
    return 1;
}

/*
 * This is called from within the combat system
 */
public void
spell_attack(object attacker, object target)
{
    int hurt;
    mixed *hitresult;
    string how;

    hurt = F_PENMOD(10, (attacker->query_skill(SS_SPELL_ATTACK) +
        attacker->query_stat(SS_INT)) / 20 + 15);
    hurt -= hurt * attacker->query_magic_res(MAGIC_I_RES_ELECTRICITY) / 100;

    hitresult = target->hit_me(hurt, 0, attacker, 0);
    how = " without effect";
    if (hitresult[0] > 0)
        how == "";
    if (hitresult[0] > 10)
        how = " hard";
    if (hitresult[0] > 20)
        how = " very hard";

    attacker->catch_msg("You blast " + QTNAME(target) + how + ".\n");
    target->catch_msg(QCTNAME(attacker) + " blasts you" + how +
        " with an energy spell.\n");
    tell_watcher(QCTNAME(attacker) + " blasts " + QTNAME(target) + how + 
	" with an energy spell.\n", attacker, target);

    if (target->query_hp() <= 0)
        target->do_die(attacker);
}

/* Function name: kill_access
 * Description:   This is to filter out not killable things in the CMDPARSE
 *		  macro.
 * Arguments:     ob - Object to test
 * Returns:	  1 - keep object.
 */
int
kill_access(object ob)
{
    if (!living(ob) || ob->query_ghost() || ob == this_player())
	return 0;
    else
	return 1;
}

/*
 * Function name: tell_watcher
 * Description:   Send the string from the fight to people that want them
 * Arguments:     The string to send
 */
static void
tell_watcher(string str, object me, object enemy)
{
    object *ob;
    int i;

    ob = FILTER_LIVE(all_inventory(environment(me))) - ({ me });
    ob -= ({ enemy });
    for (i = 0; i < sizeof(ob); i++)
        if (ob[i]->query_see_blood())
            ob[i]->catch_msg(str);
}


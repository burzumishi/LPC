/* This file is included in the guild shadow.  It contains the code
 * for the guild's special attack.  We keep it in a separate file to
 * keep from cluttering the shadow and for easier debugging.
 */

#include <stdproperties.h>
#include <formulas.h>
#include <macros.h>

/* A global variable to hold the id for the alarm used for the special */
static private int punch_alarm = 0;

/* Prototypes */
void punch2(object target);

/*
 * Function name: can_attack_with_occ_special
 * Description:   See if we are allowed to attack someone
 * Arguments:     object target - the thing we want to attack
 *                string attack - a name for the attack we want to use
 * Returns:       A string describing why we cannot attack or 0 if
 *                we can attack
 */
mixed
can_attack_with_occ_special(object target, string attack)
{
    mixed why;
    object who = query_shadow_who();

    if (!target || !living(target) || !present(target, environment(who)))
    {
        return attack + " whom?\n";
    }
 
    if (who->query_prop(LIVE_I_ATTACK_DELAY) ||
        who->query_prop(LIVE_I_STUNNED))
    {
        return "You are too stunned to " + attack + ".\n";
    }
 
    if (stringp(why = target->query_prop(OBJ_M_NO_ATTACK)) ||
        stringp(why = environment(target)->query_prop(ROOM_M_NO_ATTACK)))
    {
        return why;
    }
    else if (why)
    {
        return "You sense a force protecting " + 
            target->query_the_name(who) + ".\n";
    }

    /* Someone might try defining this to block all special attacks */
    if (target->query_not_attack_me(who, -1))
    {
        /* A message should be given by query_not_attack_me(), so
         * no need to specify one here.
         */
        return "";
    }

    if ((!who->query_npc()) && (who->query_met(target)) &&
        !(who == target->query_attack()) &&
        (who->query_prop(LIVE_O_LAST_KILL) != target))
    {
        who->add_prop(LIVE_O_LAST_KILL, target);
        return "Attack " + target->query_the_name(who) + 
            "?!? Please confirm by trying again.\n";
    }
 
    if (!F_DARE_ATTACK(who, target))
    {
        return "You don't dare attack.\n";
    }

    
    return 0;
}

/*
 * Function name: punch
 * Description:   Start up the guild's special attack
 * Arguments:     object target - the person to attack
 */
void
punch(object target)
{
    object who = query_shadow_who();

    who->catch_msg("You prepare to punch " + target->query_the_name(who) + 
        ".\n");
    
    punch_alarm = set_alarm(
       itof(20 - who->query_skill(SS_GUILD_SPECIAL_SKILL) / 10),
       0.0, &punch2(target));

    /* Initiate battle with the target */
    who->attack_object(target);

    /* reveal the player if he is hiding */
    who->reveal_me(0);
}
 
/*
 * Function name: punch2
 * Description:   Perform the actual special attack.
 * Arguments:     object target - the target of the attack
 */
void
punch2(object target)
{
    int hitsuc, phurt;
    string how, why;
    object attacker = query_shadow_who();

    punch_alarm = 0;
 
    /* make sure the target is still in the same room as the attacker */
    if (!target || (environment(target) != environment(attacker)))
    {
        attacker->catch_msg("The target of your attack seems to " +
            "have slipped away.\n");
        return;
    }

    /* Has target become invalid somehow? */
    if (stringp(why = can_attack_with_occ_special(target, "punch")))
    {
        attacker->catch_msg(why);
        return;
    }
 
    /* Test to see if the attacker hits.  cb_tohit() is the routine
     * used by the combat system to determine if a regular attack
     * hits, but it also works nicely for our purposes.
     */
    if ((hitsuc = attacker->query_combat_object()->cb_tohit(-1, 90,
        target)) > 0)
    {
        /* attack successful! */
        phurt = target->hit_me(F_PENMOD(
            MAX(attacker->query_stat(SS_STR), 150) / 6,
            attacker->query_skill(SS_GUILD_SPECIAL_SKILL)) + 30,
            W_BLUDGEON, attacker, -1)[0];
    }
    else
    {
        /* attack missed. */
        phurt = target->hit_me(hitsuc, 0, attacker, -1)[0];
    }
 
    /* add fatigue */
    if (attacker->query_fatigue() < 5)
    {
        attacker->heal_hp(-(5 - attacker->query_fatigue()));
        attacker->set_fatigue(0);
        attacker->catch_msg("The strain of the attack drains you.\n");
    }
    else
    {
        attacker->add_fatigue(-5);
    }

    if (phurt >= 0) 
    {
        /* remove some of the attacker's panic */
        attacker->add_panic(-3 - phurt / 5);

        /* give combat descriptions based on the percent damage done */
        switch (phurt)
        {
            case 0:
                how = "amazed by such a feeble attack";
                break;
            case 1..15:
                how = "dazed";
                break;
            case 16..30:
                how = "injured";
                break;
            case 31..50:
                how = "hurt";
                break;
            case 51..70:
                how = "severely hurt";
                break;
            default:
                how = "on the verge of collapse";
                break;
        }

        /* message to the attacker */
        attacker->catch_msg("You punch " + target->query_the_name(attacker) +
            ".\n" + capitalize(target->query_pronoun()) + " appears to be  " +
            how + ".\n");
       
        /* message to the target */
        target->catch_msg(attacker->query_The_name(target) +
            " punches you!\nYou are " + how + ".\n");

        /* message to onlookers */
        attacker->tell_watcher(QCTNAME(attacker) + " punches " +
                 QTNAME(target) + ".\n" + capitalize(target->query_pronoun()) +
                 " appears to be " + how + ".\n", target);

        if (target->query_hp() <= 0)
        {
            target->do_die(attacker);
        }

        return;
    }
            
    /* We get here if the attack missed */

    /* add some panic to the attacker */
    attacker->add_panic(1);
    attacker->add_attack_delay(5);

    /* message to attacker */
    attacker->catch_msg("You try to punch " +
        target->query_the_name(attacker) + " but miss.\n");

    /* message to target */
    target->catch_msg(attacker->query_The_name(target) +
        " tries to punch you but misses.\n");

    /* message to onlookers */
    attacker->tell_watcher(QCTNAME(attacker) + " tries to punch " +
        QTNAME(target) + " but misses.\n", target);
}

/*
 * Function name: query_punch
 * Description:   See if we are currently preparing to perform
 *                the special attack
 * Returns:       1 / 0 - preparing / not preparing
 */
int
query_punch()
{
    return !!punch_alarm;
}

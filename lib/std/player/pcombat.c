/*
 * /std/player/pcombat.c
 *
 * This is a subpart of player_sec.c
 *
 * All combat routines that are player specific are handled here. Interactive
 * players are not allowed to be in the same team with NPC's.
 */

#include <log.h>

/*
 * Global variables that are not saved.
 */
private static object   *team_invited;	/* Array of players invited to team */

/* 
 * Prototype.
 */
static nomask void linkdeath_remove_enemy(object enemy);

/*
 * Function name:   team_invite
 * Description:     Invites a new member to my team. This does NOT join the
 *                  member to my team. It only makes it possible for the
 *                  player to join my team.
 * Arguments:	    member: The objectpointer to the invited member.
 *                          If member == 0, the invited list is cleared.
 */
public void
team_invite(object member)
{
    if (!member)
    {
	team_invited = 0;
	return;
    }
    if (member_array(member, team_invited) >= 0)
    {
	return;
    }

    if (!team_invited)
    {
	team_invited = ({ member });
    }
    else
    {
	team_invited = team_invited + ({ member });
    }
}

/*
 * Function name:   query_invited
 * Description:     Give back an array with objects of players who are
 *                  invited.
 * Returns:         An array of objects
 */
public object *
query_invited()
{
    if (!team_invited)
    {
        return ({ });
    }

    team_invited = filter(team_invited, objectp);
    return ({ }) + team_invited;
}

/*
 * Function name:   remove_invited
 * Description:     Remove an object from the invited list.
 * Argumnents:      ob - The object to remove from the list.
 */
public void
remove_invited(object ob)
{
    if (team_invited)
    {
	team_invited -= ({ 0, ob });
    }
}

/*
 * Function name: attacked_by
 * Description  : When someone attacks us, this function is called. It makes
 *                a test, preventing anyone from attacking us while we are
 *                linkdeath.
 * Arguments    : object attacker - who is attacking us.
 */
public void
attacked_by(object attacker)
{
    if (!query_interactive(this_object()))
    {
	tell_object(this_object(), "You are linkdeath, so you cannot " +
	    "be attacked by " +
	    attacker->query_the_name(this_object()) + ".\n");
	tell_object(attacker, "You are not allowed to attack " +
	    this_object()->query_the_name(attacker) +
	    " since " + query_pronoun() +
	    " is not in touch with reality.\n");

	set_alarm(0.5, 0.0, &linkdeath_remove_enemy(attacker));
	return;
    }

    ::attacked_by(attacker);
}

/*
 * Function name: attack_object
 * Description  : When we attack someone, this function is used. It has a
 *                check preventing us from attacking anyone while we are
 *                linkdeath. Also, log when players attack each other.
 * Arguments    : object victim - the intended victim.
 */
public void
attack_object(object victim)
{
    if (!interactive(this_object()))
    {
        this_object()->catch_tell("You are linkdeath, so you cannot" +
            " attack " + victim->query_the_name(this_object()) + ".\n");

        victim->catch_tell(this_object()->query_The_name(victim) +
            " cannot attack you since " + query_pronoun() +
            " is not in touch with reality.\n");

        set_alarm(0.5, 0.0, &linkdeath_remove_enemy(victim));
        return;
    }

#ifdef LOG_PLAYERATTACKS
    if (interactive(victim) &&
        (calling_function(0) != "combat_init") &&
        (!query_wiz_level() || !victim->query_wiz_level()))
    {
        /* we log only when the victim is interactive, calling function */
        /* isnt combat_init (attack_object called from there when enemy */
        /* is reattacked from combat object) & when one of the parties  */
        /* is not a wizard                                              */

        log_file(LOG_PLAYERATTACKS,
            sprintf("%s %-11s (%3d) attacks %-11s (%3d)\n in %s\n",
            ctime(time()), capitalize(query_real_name()),
            query_average_stat(), capitalize(victim->query_real_name()),
            victim->query_average_stat(),
            file_name(environment(this_object()))),  -1);
    }
#endif LOG_PLAYERATTACKS

    ::attack_object(victim);
}

/*
 * Function name: linkdeath_remove_enemy
 * Description  : This function removes a player from the list of enemies
 *                if combat was initiated between this object and the
 *                enemy.
 * Arguments    : object enemy - the enemy we should not fight.
 */
static nomask void
linkdeath_remove_enemy(object enemy)
{
    this_object()->stop_fight(enemy);
    enemy->stop_fight(this_object());
}

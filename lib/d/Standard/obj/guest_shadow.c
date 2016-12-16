/*
 * /d/Standard/obj/guest_shadow.c
 *
 * Copyright (C) Stas van der Schaaf - December 23 1994
 *               Mercade @ Standard
 *
 * This is the shadow of the guest player. Guest is a guest of the game.
 * We do not want him to join guilds, be engaged in combat or gain any
 * experience.
 *
 * Revision history:
 */

inherit "/std/shadow";

#pragma no_inherit
#pragma save_binary
#pragma strict_types

#include <macros.h>

#define QSW   query_shadow_who()
#define GUEST "guest"
#define TITLE "of Standard"

/* Define this if you do not want Guest to use the mail system. */
#undef GUEST_MAIL_BLOCK

/*
 * Function name: query_guild_not_allow_join_gulid
 * Description  : This function is called before a player may join a guild.
 *                It is used to prevent guest from joining any guild.
 * Arguments    : object player - the player who wants to join;
 *                string type   - the type of guild someone wants to join;
 *                string style  - the style of the guild you want to join;
 *                string name   - the name of the guild you want to join.
 * Returns      : int 1 - always.
 */
nomask public int
query_guild_not_allow_join_guild(object player, string type, string style,
	string name)
{
    write("You are a guest of this game. You cannot join any guilds.\n\n");
    return 1;
}

/*
 * Function name: attacked_by
 * Description  : When someone attacks us, this function is called. We
 *                disallow all combat with the guest player, so the call
 *                is intercepted and the combat cancelled.
 * Arguments    : object attacker - the player attacking us.
 */
nomask public void
attacked_by(object attacker)
{
    tell_object(QSW, "\nAs guest of Standard you are protected from any " +
	"hostilities. " + attacker->query_The_name(QSW) + " will have to " +
	"find " + attacker->query_possessive() + "self a different " +
	"victim.\n\n");
    tell_object(attacker, "\nThe guest of Standard is protected by the Gods. " +
	"We do not want anyone to attack " + QSW->query_objective() +
	".\n\n");

    QSW->stop_fight(attacker);
    attacker->stop_fight(QSW);
}

/*
 * Function name: attack_object
 * Description  : When the player attacks someone, this function is called.
 *                The guest of Standard should not get ingaged in combat.
 * Arguments    : object victim - the intended victim.
 */
nomask public void
attack_object(object victim)
{
    tell_object(QSW, "\nAs guest of Standard you should not get involved in " +
	"hostilities. If you like this game, you can create your own " +
	"character to explore the world with.\n\n");
    tell_object(victim, "\nThe guest of Standard is not allowed to get " +
	"involved in active combat. The fight is off.\n\n");

    QSW->stop_fight(victim);
    victim->stop_fight(QSW);
}

/*
 * Function name: do_die
 * Description  : This function is called whenever someone suspects that
 *                we have died. We do not want the guest to die, so the
 *                call is intercepted.
 * Arguments    : killer - the object that killed us.
 */
nomask public void
do_die(object killer)
{
    object *enemies;

    if ((QSW->query_hp() > 0) ||
	(QSW->query_ghost()))
    {
	return;
    }

    if (!objectp(killer))
    {
	killer = previous_object();
    }

    /* Heal him fully, so this function isn't called again. */
    QSW->set_hp(QSW->query_max_hp());

    tell_room(environment(QSW), "\nThe guest of Standard dies ....\n" +
	" .... and yet " + QSW->query_pronoun() + " fails to die!\n" +
	"Even though all life is drained from " + QSW->query_objective() +
	", the Gods refuse to take the guest of Standard away.\n",
	query_shadow_who());
    tell_object(killer, "You killed the guest of Standard, though the Gods " +
	"decide to keep " + QSW->query_objective() + " among the living.\n\n");
    tell_object(QSW, "\nYOU DIE !!!\nHowever, since you are a guest of " +
	"Standard, you are kept among the living.\n\n");

    /* Even though the guest cannot be in combat, we remove all enemies
     * just in case.
     */
    enemies = (object *)QSW->query_enemy(-1) - ({ 0 });
    if (sizeof(enemies))
    {
	QSW->stop_fight(enemies);
	enemies->stop_fight(QSW);
    }
}

/*
 * Function name: add_exp
 * Description  : There is no reason for guest to gain experience. Therefore
 *                we disallow any experience other than the experience
 *                added by the player itself. This is vital since it is done
 *                in the player start-sequence.
 * Arguments    : int experience - the experience to add;
 *                int battle     - true if gained in battle.
 */
nomask public void
add_exp(int experience, int battle)
{
    if (previous_object() == QSW)
    {
	QSW->add_exp(experience, battle);
    }
}

/*
 * Function name: query_guild_title_race
 * Description  : The guest of Standard has his own title.
 * Returns      : string - the title.
 */
nomask public string
query_guild_title_race()
{
    return TITLE;
}

/*
 * Function name: query_guild_family_name
 * Description  : If this function returns true, the article 'the' is
 *                omitted in the title.
 * Returns      : int 1 - always.
 */
nomask public int
query_guild_family_name()
{
    return 1;
}

/*
 * Function name: set_race_name
 * Description  : The guest of Standard is a human being. We do not wants
 *                his race to be set to anything else.
 * Arguments    : string name - the race name to set.
 */
nomask public void
set_race_name(string name)
{
    QSW->set_race_name("human");
}

/*
 * Function name: autoload_shadow
 * Description  : This function is called to ensure that the shadow
 *                autoloads. If the player is anyone else than the guest,
 *                the shadow selfdestructs.
 * Arguments    : mixed arg - the possible arguments.
 */
nomask public void
autoload_shadow(mixed arg)
{
    ::autoload_shadow(arg);

    if (!objectp(QSW) ||
	(QSW->query_real_name() != GUEST))
    {
	QSW->remove_autoshadow(MASTER);

	destruct();
    }
}

#ifdef GUEST_MAIL_BLOCK
/*
 * Function name: enter_inv
 * Description  : This function will mask the enter_inv() function of the
 *                guest to prevent the mail reader from entering.
 * Arguments    : object obj  - the object entering.
 *                object from - the object it came from.
 */
nomask public void
enter_inv(object obj, object from)
{
    if (obj->id("_reader_"))
    {
	set_alarm(1.0, 0.0, &tell_object(QSW, "\nAs guest of Standard you " +
	    "have no business mailing people. If you like this\ngame so " +
	    "much that you want to keep in touch with other players, " +
	    "create\nyourself a character.\n\n"));
	obj->remove_object();
	return;
    }

    QSW->enter_inv(obj, from);
}
#endif GUEST_MAIL_BLOCK

/*
 * Function name: query_prevent_shadow
 * Description  : No additional shadows should be added the player. This
 *                function returns true to prevent addition of shadows to
 *                the player.
 * Returns      : int 1 - always.
 */
nomask public int
query_prevent_shadow()
{
    return 1;
}

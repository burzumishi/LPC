/*
 * /d/Standard/obj/auto_intro.c
 *
 * Add this as autoshadow to players who have been stupid enough to select a
 * name that is embarassing. It will force the player to introduce themselves
 * to everyone they meet.
 *
 * Created by Mercade on August 4, 2001
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/std/shadow";

#include <macros.h>

/*
 * Global variables.
 */
static private int     alarm_id = 0;
static private object *players = ({ });

#define LOGFILE "AUTO_INTRO"

/*
 * Function name: force_introduction
 * Description  : Make the player introduce himself.
 */
public void
force_introduction()
{
    query_shadow_who()->command("$introduce me");
    query_shadow_who()->command("$say Hi, I am " +
        capitalize(query_shadow_who()->query_real_name()) + ".");

    alarm_id = 0;
}

/*
 * Function name: inform_wizard
 * Description  : Tell the wizard what is going on with the player.
 * Arguments    : object wizard - the wizard.
 */
static void
inform_wizard(object wizard)
{
    if (environment(query_shadow_who()) != environment(wizard))
    {
        return;
    }

    wizard->catch_tell("\nThe player " +
        capitalize(query_shadow_who()->query_real_name()) +
        " has been punished with auto-intro because of " +
        query_shadow_who()->query_possessive() +
        " bad taste in chosing names. Effectively, this means that " +
        query_shadow_who()->query_pronoun() + " will introduce " +
        query_shadow_who()->query_objective() + 
        "self every time a non-met player is encountered.\n" +
        "To remove, call remove_auto_intro() in the player.\n\n");
}

/*
 * Function name: init_living
 * Description  : This function is called each time the player encounters a
 *                living. If it is a mortal player, make the player introduce
 *                himself.
 */
public void
init_living()
{
    /* The normal init_living() call continued. */
    query_shadow_who()->init_living();

    /* This module does not operate on NPC's. */
    if (this_player()->query_npc())
    {
    	return;
    }

    /* Tell wizards what to expect. Make sure to tell the wizard only once,
     * and don't bother with the check on multiple players, because the
     * wizard gets an individual message.
     */
    if (this_player()->query_wiz_level())
    {
        if (member_array(this_player(), players) > -1)
        {
            return;
        }
        players += ({ this_player() });
        set_alarm(0.2, 0.0, &inform_wizard(this_player()));
        return;
    }

    /* Let the player introduce only once per reboot. Also, then the player
     * is already met, don't bother with another introduction. And when the
     * player cannot be seen, then we cannot intro either, naturally.
     */
    if ((member_array(this_player(), players) > -1) ||
        this_player()->query_met(query_shadow_who()) ||
        !CAN_SEE(query_shadow_who(), this_player()))
    {
        return;
    }

    /* Add the player to the list of people to we introduce to, so that we do
     * not do it a second time.
     */
    players += ({ this_player() });

    /* Set the alarm, if it is not already running. */
    if (!alarm_id)
    {
	set_alarm(0.2, 0.0, force_introduction);
    }
}

/*
 * Function name: add_auto_intro
 * Description  : For easy access, this call this function in the master to
 *                add the shadow to a player.
 * Arguments    : string name - the name of the player.
 */
void
add_auto_intro(string name)
{
    object player;
    
    player = find_player(name);
    if (!objectp(player))
    {
        write("No such player in the realms.\n");
        return;
    }
    if (player->query_wiz_level())
    {
        write("This does not work on wizards.\n");
        return;
    }

    setuid();
    seteuid(getuid());

    player->add_autoshadow(MASTER + ":");
    clone_object(MASTER)->shadow_me(player);
    write("Shadow and autoshadow added to " + capitalize(name) + ".\n");

    log_file(LOGFILE, sprintf("%s - %s added auto-intro to %s.\n",
        ctime(time()), capitalize(this_player()->query_real_name()),
        capitalize(name)));
}

/*
 * Function name: remove_auto_intro
 * Description  : To relieve the poor soul, call this function to remove the
 *                soul and remove it from autoloading.
 */
public void
remove_auto_intro()
{
    setuid();
    seteuid(getuid());

    query_shadow_who()->remove_autoshadow(MASTER + ":");
    destruct();
    write("Shadow and autoshadow removed from " +
        capitalize(query_shadow_who()->query_real_name()) + ".\n");

    log_file(LOGFILE, sprintf("%s - %s removed auto-intro from %s.\n",
        ctime(time()), capitalize(this_player()->query_real_name()),
        capitalize(query_shadow_who()->query_real_name())));
}

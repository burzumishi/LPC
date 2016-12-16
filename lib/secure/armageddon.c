/*
 * /secure/armageddon.c
 *
 * This object takes the mud down in a gracefull manner.
 *
 * This is supposed to be inherited by the actual Armageddon object
 * for mud-specific actions to be taken when the game closes down.
 * SECURITY is the only object that is allowed to shut down the game.
 * All requests have to go through the master.
 */

#pragma no_clone
#pragma save_binary
#pragma strict_types

inherit "/std/creature";

#include <files.h>
#include <macros.h>
#include <std.h>
#include <stdproperties.h>
#include <time.h>

#define PASS_ARMAGEDDON          300 /*  5 minutes */
#define SHUTDOWN_TELL_HOME_LIMIT 900 /* 15 minutes */

/*
 * Global variables.
 */
static private string shutdown_shutter;
static private string shutdown_reason;
static private int    shutdown_delay; /* time left in minutes */
static private int    shutdown_alarm;
static private int    shutdown_manual;

#define TELLALL(x) WIZ_CMD_NORMAL->tellall(x)

/*
 * Function name: query_init_master
 * Description:   Makes sure that the master object is initialized properly.
 */
public nomask int
query_init_master()
{
    return 1;
}

/*
 * Function name: create_creature
 * Description  : Called to create the statuette.
 */
public void
create_creature()
{
    set_name("armageddon");
    add_name("statuette");
    set_adj("small");

    set_short("small statuette");
    set_long("It is a small statuette of Armageddon!\n");

    set_living_name("armageddon");
    set_tell_active(1);

    add_prop(LIVE_I_ALWAYSKNOWN, 1);

    shutdown_shutter = 0;
    shutdown_reason  = 0;
    shutdown_delay   = 0;
    shutdown_alarm   = 0;
    shutdown_manual  = 0;
}

/*
 * Function name: shutdown_info_domain_link
 * Description  : Inform the domain links of the shutdown status.
 * Arguments    : int level - one of the definitions as per <const.h>
 */
static void
shutdown_info_domain_link(int level)
{
    string *links;
    int index;
		 
    links = SECURITY->query_domain_links();
    index = sizeof(links);
    while(--index)
    {
        if (file_size(links[index] + ".c") != -1)
        {
            catch(links[index]->armageddon(level));
        }
    }
}

/*
 * Function name: shutdown_now
 * Description  : When the game finally goes down, this is the function
 *                that tells the master to do so.
 */
private nomask void
shutdown_now()
{
    set_this_player(this_object());
    TELLALL("I am shutting the game down now!");

    if (!SECURITY->master_shutdown(sprintf("%-11s: %s\n",
	capitalize(shutdown_shutter), shutdown_reason)))
    {
	TELLALL("EEKS! Master did not allow me to shut down the game!\n");

	shutdown_alarm   = 0;
	shutdown_delay   = 0;
	shutdown_reason  = 0;
	shutdown_shutter = 0;
        shutdown_manual  = 0;
    }
}

/*
 * Function name: shutdown_dodelay
 * Description  : This function counts down the minutes until the game
 *                is finally shut down.
 */
private nomask void
shutdown_dodelay()
{
    int period;

#ifdef STATUE_WHEN_LINKDEAD
#ifdef OWN_STATUE
    /* Since people are not allowed to re-link when the game is about to
     * be shut down, we inform the statue-object of the fact so they can
     * save the players and log them out.
     */
    if ((shutdown_delay * 60) <= PASS_ARMAGEDDON)
    {
        OWN_STATUE->shutdown_activated();
    }
#endif OWN_STATUE
#endif STATUE_WHEN_LINKDEAD

    /* No more delay, it is closing time. */
    if (!shutdown_delay)
    {
	shutdown_now();
	return;
    }

    set_this_player(this_object());
    TELLALL("I will shut the game down in " + CONVTIME(shutdown_delay * 60) +
        ".");

    /* If the shutdown period is longer, we will not notify the players
     * each minute, but use a larger delay.
     */
    if (shutdown_delay >= 1080)
    {
	period = 720;
    }
    else if (shutdown_delay >= 90)
    {
        period = 60;
    }
    else if (shutdown_delay >= 25)
    {
	period = 15;
    }
    else if (shutdown_delay >= 10)
    {
	period = 5;
    }
    else
    {
	period = 1;
    }

    shutdown_alarm = set_alarm((itof(period) * 60.0), 0.0,
        shutdown_dodelay);
    shutdown_delay -= period;
}

/*
 * Function name: shutdown_started
 * Description  : This function is called when the game is shut down. It
 *                can be redefined by the local armageddon object at your
 *                mud.
 */
public void
shutdown_started()
{
}

/*
 * Function name: start_shutdown
 * Description  : When the game has to be shut down in a gentle way,
 *                this is the function you are looking for. You should
 *                not try to call it directly, but use the 'shutdown'
 *                command.
 * Arguments    : string reason  - the reason to close the game.
 *                int    delay   - the delay in minutes.
 *                string shutter - who is shutting down the game.
 */
public nomask void
start_shutdown(string reason, int delay, string shutter)
{
    if (previous_object() != find_object(SECURITY))
    {
	return;
    }

    /* When shutdown is started, we destruct the queue and tell the people
     * to get back later.
     */
    QUEUE->tell_queue("\nThe game is going to reboot.\n" +
	"Please reconnect when the game is back up.\n" +
	"It will take about ten minutes to reboot the game.\n\n");
    (QUEUE->queue_list(0))->remove_object();

    set_this_player(this_object());

    if (shutter == ROOT_UID)
    {
	TELLALL("Lo and behold! I am here! " + reason + "\n");
    }
    else
    {
	TELLALL("Lo and behold! I am here! " + capitalize(shutter) +
	    " asked me to shut the game down. " + reason + "\n");
    }

    shutdown_shutter = shutter;
    shutdown_reason  = reason;
    shutdown_delay   = delay;
    shutdown_manual  = (shutter != ROOT_UID);

    if (!shutdown_delay)
    {
	shutdown_now();
	return;
    }

    shutdown_started();

    set_this_player(this_object());

    TELLALL("Tell me (do not commune) if you want to be sent home." +
        (shutdown_manual ? (" NOTE: As favour to " + capitalize(shutter) +
        ", no item will fail to glow.\n") : "\n"));
    shutdown_dodelay();

    shutdown_info_domain_link(ARMAGEDDON_ANNOUNCE);
}

/*
 * Function name: shutdown_stopped
 * Description  : This function is called when the shutdown process is
 *                stopped. It may be redefined by the local armageddon
 *                object at your mud.
 * Arguments    : string stopper - the one who decided not to stop.
 */
public void
shutdown_stopped(string stopper)
{
}

/*
 * Function name: cancel_shutdown
 * Description  : If the wizard who was shutting the game down changed
 *                his mind, this is the way to stop it. Do not call the
 *                function directly, though use: 'shutdown abort'
 * Arguments    : string shutter - the person canceling the shutdown.
 */
public nomask void
cancel_shutdown(string shutter)
{
    if (previous_object() != find_object(SECURITY))
    {
	return;
    }

    set_this_player(this_object());
    TELLALL(capitalize(shutter) +
	" decided not to shut down the game after all.");

    shutdown_stopped(capitalize(shutter));

    remove_alarm(shutdown_alarm);

    shutdown_shutter = 0;
    shutdown_reason  = 0;
    shutdown_alarm   = 0;
    shutdown_delay   = 0;
    shutdown_manual  = 0;

    shutdown_info_domain_link(ARMAGEDDON_CANCEL);
}

/*
 * Function name: execute_shutdown
 * Description  : Called by SECURITY when the game actually shuts down,
 *                whether so desired through Armageddon, or by other means.
 *                We use it to inform the domain links with our last breath.
 */
public nomask void
execute_shutdown()
{
    if (previous_object() != find_object(SECURITY))
    {
        return;
    }

    shutdown_info_domain_link(ARMAGEDDON_SHUTDOWN);
}
     
/*
 * Function name: query_delay
 * Description  : Returns the time we have left in seconds.
 * Returns      : int - the time in seconds, or -1 if we are not shutting
 *                    down at all.
 */
public nomask int
query_delay()
{
    mixed *call;

    if (shutdown_alarm == 0)
    {
        return -1;
    }

    call = get_alarm(shutdown_alarm);
    if (sizeof(call) < 3)
    {
        return -1;
    }

    return ftoi(call[2]) + (shutdown_delay * 60);
}

/*
 * Function name: query_shutter
 * Description  : Return the name of the person shutting us down.
 * Returns      : string - the name.
 */
public nomask string
query_shutter()
{
    return shutdown_shutter;
}

/*
 * Function name: query_manual_reboot
 * Description  : Returns whether the game is being shut down manually or not.
 *                If a wizard called for this reboot, no items will fail to
 *                glow.
 * Returns      : int 1/0 - if true, then a wizard manually rebooted the game.
 */
public nomask int
query_manual_reboot()
{
    return shutdown_manual;
}

/*
 * Function name: query_reason
 * Description  : Return the reason for the shutdown.
 * Returns      : string - the reason.
 */
public nomask string
query_reason()
{
    return shutdown_reason;
}

/*
 * Function name: shutdown_active
 * Description  : Returns true if Armageddon is active.
 * Returns      : int 1/0 - true if Armageddon is active.
 */
public nomask int
shutdown_active()
{
    return (shutdown_alarm != 0);
}

/*
 * Function name: shutdown_time
 * Description  : This function returns how long it will take before the
 *                game is shut down.
 * Returns      : int - the remaining time in seconds.
 */
public nomask int
shutdown_time()
{
    if (!shutdown_active())
    {
	return 0;
    }

    /* Get the remaining time until the next alarm and the time needed
     * after the next alarm is called.
     */
    return ftoi(get_alarm(shutdown_alarm)[2]) + (shutdown_delay * 60);
}

/*
 * Function name: catch_tell
 * Description  : Everything "printed" to us is parsed by this function.
 * Arguments    : string str - the text being sent to us.
 */
public void
catch_tell(string str)
{
    string home;
    object old;

    /* Don't catch on the things we say ourselves. */
    if (this_player() == this_object())
	return;

    if (!shutdown_active())
    {
	tell_object(this_player(), "Armageddon tells you: No reason to " +
	    "teleport you home since I am not shutting the game down.\n");
	return;
    }

    if (sizeof(explode(str, "home")) <= 1)
    {
	tell_object(this_player(), "Armageddon tells you: " +
	    "Tell me 'home' and I will send you there.\n");
	return;
    }

    if (shutdown_time() > SHUTDOWN_TELL_HOME_LIMIT)
    {
	tell_object(this_player(),
	    "Armageddon tells you: No reason to teleport you home since " +
	    "there is enough time still before the shutdown.\n");
	return;
    }

    home = this_player()->query_temp_start_location();
    if (!stringp(home))
    {
	home = this_player()->query_default_start_location();
    }
    if (!stringp(home))
    {
	tell_object(this_player(),
	    "Armageddon tells you: You have no home, sorry!\n");
	return;
    }

    old = environment(this_player());
    tell_object(this_player(), "Armageddon tells you: " +
	"I will try to send you home to where you start!\n");

    /* Third argument idicates group should not try to follow this
     * player.
     */
    this_player()->move_living("X", home, 1);
    if (old == environment(this_player()))
    {
	tell_object(this_player(), "Armageddon tells you: " +
	    "My magic seems not to work too well at teleporting you.\n");
    }
}

/*
 * Function name: query_prevent_shadow
 * Description  : This function prevents anyone from shadowing us.
 * Returns      : int 1 - always.
 */
nomask int
query_prevent_shadow()
{
    return 1;
}

/*
 * /secure/queue.c
 *
 * This object queues people who want to log in when the game is full.
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

#include <files.h>
#include <macros.h>
#include <std.h>
#include <time.h>

/*
 * Prototype.
 */
static void inform_queue();

/*
 * The gloval variables.
 */
private static object *q   = ({ });
private static string *vip = ({ });
private static int    alarm_id;

#define QUEUE_TIME              (150.0) /* 2.5 minutes */
#define WIZARDS_PER_MORTAL_SLOT (  3  )

/*
 * Function name: create
 * Description  : The people who are waiting in the queue are informed of
 *                their position regularly.
 */
public void
create()
{
    alarm_id = set_alarm(QUEUE_TIME, QUEUE_TIME, inform_queue);
}

/*
 * Function name: short
 * Description  : Returns the short description for the queue.
 * Returns      : string - the short description.
 */
public string
short()
{
    return "the login queue";
}

/*
 * Function name: validate_queue
 * Descritpion  : This function will validate the queue in the sense that it
 *                checks whether every object in the queue is by an
 *                interactive object.
 */
static void
validate_queue()
{
    q = filter(q, objectp);
    q = filter(q, interactive);
}

/*
 * Function name: queue_list
 * Description  : Get the queue of the list of names of people who are
 *                in the queue.
 * Arguments    : int arg - if true list the names, else return the array
 *                          of objectpointers.
 * Returns      : *object - the real queue if arg is false.
 *                *string - the names of the people in the queue if arg is
 *                          true.
 */
public mixed
queue_list(int arg)
{
    validate_queue();

    if (!arg)
    {
	return secure_var(q);
    }
    else
    {
	return q->query_pl_name();
    }
}

/*
 * Function name: slots_free
 * Description  : This function returns the number of free slots in the
 *                game. Since wizards do not run around, loading a lot of
 *                rooms and they are not involved in slaughtering many NPC
 *                several wizards fit in one mortal players slot. However,
 *                if there is only one wizard in the slot, the slot is still
 *                considered filled.
 * Returns      : int - the number of slots free for mortal players.
 */
static int
slots_free()
{
    object *list;
    int    wizards;

    list = users() - ({ previous_object() }) - q;
    list = filter(list, objectp);
    wizards = sizeof(filter(list, &->query_wiz_level()));

    return (MAX_PLAY - ((sizeof(list) - wizards) +
	((wizards + WIZARDS_PER_MORTAL_SLOT - 1) / WIZARDS_PER_MORTAL_SLOT)));
}

/*
 * Function name: force_quit_idler
 * Description  : This routine is called through an alarm from should_queue()
 *                to prevent eval-cost errors on large inventories.
 * Arguments    : object player - the player to quit.
 */
public void
force_quit_idler(object player)
{
    SECURITY->log_syslog("IDLE", sprintf("%s %-11s after %s\n", ctime(time()),
        capitalize(player->query_real_name()), CONVTIME(query_idle(player))));
    tell_object(player,
	"You have been idle too long. You are logged out.\n");

    set_this_player(player);
    player->quit();

    /* This may happen if the person is idling while in combat, but it should
     * not happen practically (very long combat while idling ?!?).
     */
    if (objectp(player))
    {
	player->save_character();
	player->remove_object();
    }
}

/*
 * Function name: should_queue
 * Description  : Call this function to see whether the player should queue.
 *                It does not queue the player yet.
 * Arguments    : string name - the name of the player that wants to log in.
 * Returns      : int - true if the player should queue. It returns the
 *                      next queue-number. Else 0.
 */
public int
should_queue(string name)
{
    object *list;

    /* If not called from the login object, return the queue size. */
    if (MASTER_OB(previous_object()) != LOGIN_OBJECT)
    {
	return sizeof(q) + 1;
    }

    /* Wizards above 'normal' level always enter the game without problems.
     * The same goes for the junior wizhelpers of that rank and above.
     */
    if ((SECURITY->query_wiz_rank(name) >= WIZ_NORMAL) ||
	(wildmatch("*jr", name) &&
	 (SECURITY->query_wiz_rank(extract(name, 0, -3)) >= WIZ_NORMAL)))
    {
	return 0;
    }

    /* The player may have VIP access. */
    if (member_array(name, vip) != -1)
    {
	previous_object()->catch_tell("You have VIP access to bypass the " +
	    "queue this time.\n");
	return 0;
    }
   
    /* Begin by getting rid of idlers. This is done EVERY time anyone
     * tries to log in. Some can be idle longer than others.
     */
    validate_queue();
    list = users() - ({ previous_object() }) - q;
    list = filter(list, objectp);
    list = filter(list, interactive);

    foreach(object player: list)
    {
#ifdef NO_WIZARD_IDLE_CHECK
        if (!SECURITY->query_wiz_rank(player->query_real_name()) &&
            query_idle(player) > MAX_IDLE_TIME)
#else
	if (query_idle(player) > (MAX_IDLE_TIME * (1 +
	    SECURITY->query_wiz_rank(player->query_real_name()))))
#endif NO_WIZARD_IDLE_CHECK
	{
            set_alarm(0.0, 0.0, &force_quit_idler(player));
	}
    }

    /* People are already queueing, so you cannot enter. Take a number. */
    if (sizeof(q))
    {
	return sizeof(q) + 1;
    }

    /* There are no slots free for mortals, so start a queue. */
    if (slots_free() < 1)
    {
	return sizeof(q) + 1;
    }

    /* We passed all tests, so we can enter. */
    return 0;
}

/*
 * Function name: enqueue
 * Description  : Called to see whether a (mortal) player can log in and
 *                if necessary add the player to the queue.
 * Arguments    : object ob - the object that wants to log in.
 * Returns      : int - the queue position of the player.
 */
public int
enqueue(object ob)
{
    int pos;

    /* Should only be called from the login object. */
    if (MASTER_OB(ob) != LOGIN_OBJECT)
    {
	return sizeof(q);
    }

    /* If the player is already in the queue, we insert the new object in
     * the queue at the position of the other object, so actually we are
     * very nice ;-)
     */
    if ((pos = member_array(ob->query_pl_name(), queue_list(1))) != -1)
    {
	q[pos]->catch_tell("You entered the queue again, so this copy is " +
	    "removed.\n");
	q[pos]->remove_object();
	q[pos] = ob;

	return (pos + 1);
    }

    validate_queue();
    q = q + ({ ob });

    if (!alarm_id)
    {
        alarm_id = set_alarm(QUEUE_TIME, QUEUE_TIME, inform_queue);
    }

    return sizeof(q);
}

/*
 * Function name: dequeue
 * Description  : Called from SECURITY when an interactive object
 *                leaves the game. It checks how many players are still
 *                in the game and dequeues people if there is space for
 *                them.
 * Arguments    : object ob - the object that leaves the game.
 */
public void
dequeue(object ob)
{
    int size;
    int index;
    int free;

    validate_queue();
    
    /* If a player leaves the queue, we won't update the information of the
     * players in the queue to let them advance. No space was created in the
     * game, so no player can really enter anyway. However, we kick the
     * object out of the queue.
     */
    if (member_array(ob, q) != -1)
    {
	q -= ({ ob });

	return;
    }

    /* See how many slots are free now, and make the players in the queue
     * advance as far as possible. Other players will have their queue
     * status updated.
     */
    free = slots_free();
    if (free > 0)
    {
	index = -1;
	size = sizeof(q);
	while(++index < size)
	{
	    q[index]->advance((index < free) ? 0 : (index - free + 1));
	}
	
	q = ((size > free) ? q[free..] : ({ }) );
    }

    /* Re-start the alarm to give the next update in QUEUE_TIME seconds. */
    remove_alarm(alarm_id);
    alarm_id = set_alarm(QUEUE_TIME, QUEUE_TIME, inform_queue);
}

/*
 * Function name: inform_queue
 * Description  : We advise the people in the queue on their position at
 *                a regular interval to assure them of the fact that they
 *                are still queueing.
 */
static void
inform_queue()
{
    int index;
    int size;

    validate_queue();

    if (!sizeof(q))
    {
        remove_alarm(alarm_id);
        alarm_id = 0;
        return;
    }

    index = -1;
    size = sizeof(q);
    while(++index < size)
    {
	q[index]->advance(index + 1);
    }
}

/*
 * Function name: query_queue
 * Description  : Return the number of players in the queue.
 * Returns      : int - the size of the queue.
 */
public int
query_queue()
{
    validate_queue();

    return sizeof(q);
}

/*
 * Function name: query_position
 * Description  : This function returns the position of a particular player
 *                in the queue.
 * Arguments    : string name - the name of the player to check
 * Returns      : int - the position of the player in the queue or -1 if the
 *                      player is not in the queue.
 */
public int
query_position(string name)
{
    return member_array(name, queue_list(1));
}

/*
 * Function name: set_vip
 * Description  : Give VIP access until the game reboots. The function
 *                requires arch privileges.
 * Arguments    : string v - the [lowcase] name of the player to give access.
 * Returns      : int 1/0 - true if the vip-access was granted.
 */
public int
set_vip(string v)
{
    int pos;

    /* May only be called from the arch-soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
    {
	return 0;
    }

    /* Person also has VIP access. */
    if (member_array(v, vip) != -1)
    {
	return 0;
    }

    /* Player is already in the queue, making him leave it. */
    if ((pos = member_array(v, queue_list(1))) != -1)
    {
	q[pos]->catch_tell("You have been given VIP access to leave the " +
	    "queue by " + capitalize(this_player()->query_real_name()) +
	    ".\n");
	set_this_player(q[pos]);
	q[pos]->advance(0);
	q[pos] = 0;

	return 1;
    }    

    vip += ({ v });
    return 1;
}

/*
 * Function name: unvip
 * Description  : Call this function to revoke VIP access from a player.
 *                The function requires arch privileges.
 * Arguments    : string v - the [lowcase] name of the player.
 * Returns      : int 1/0 - true if the vip-access was granted.
 */
public int
unvip(string v)
{
    /* May only be called from the arch-soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
    {
	return 0;
    }

    /* Player does not have VIP access. */
    if (member_array(v, vip) == -1)
    {
	return 0;
    }

    vip -= ({ v });
    return 1;
}

/*
 * Function name: query_vip
 * Description  : Return the names of the people in the queue.
 * Returns      : string * - the people in the queue.
 */
public string *
query_vip()
{
    return secure_var(vip);
}

/*
 * Function name: tell_queue
 * Description  : Give a message to all people in the queue.
 * Arguments    : string str - the message to print to the people in the queue.
 */
public void
tell_queue(string str)
{
    validate_queue();

    q->catch_tell(str);
}

/*
 * Function name: remove_object
 * Description  : Remove this object from memory. It also destructs the
 *                people in the queue.
 */
public void
remove_object()
{
    tell_queue("The queue is being destructed.\n" +
	"Please connect another time.\n");
    q->remove_object();

    destruct();
}

/*
 * Function name: query_prevent_shadow
 * Description  : We do not want anyone shadowing this object.
 * Returns      : int 1 - always.
 */
public nomask int
query_prevent_shadow()
{
    return 1;
}

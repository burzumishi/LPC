/*
 * /d/Standard/obj/statue.c
 *
 * This is the statue room. Here we keep all players who linkdied. If they do
 * not recover their link within two hours, they are destructed.
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/std/room";

#include <login.h>
#include <macros.h>
#include <std.h>
#include <stdproperties.h>

#define STATUE_ROOM  ("_statue_room")
#define EDITOR_ID    ("_editor_")
#define LD_PERIOD    (7200) /* 2 hours */
#define PASS_ARMAGEDDON (300) /* 5 minutes */

#define PLAYER_S_LD_IN_ROOM "_player_s_ld_in_room"
#define PROPS_TO_SAVE_SIZE  (2)

static string *props_to_save = ({ LIVE_O_LAST_ROOM, LIVE_S_LAST_MOVE });
static object *statues = ({ });
static object *nonpresent = ({ });
static int alarm_id = 0;

/*
 * Function name: may_not_leave
 * Description  : This function prevents linkdead people from leaving the room.
 * Returns      : int 1/0 - true if the player may not leave.
 */
nomask public int
may_not_leave()
{
    if (!interactive(this_player()))
    {
	write("Only interactive players may use this exit.\n");
	return 1;
    }

    return 0;
}

/*
 * Function name: create_room
 * Description  : Called to create the room.
 */
public nomask void
create_room() 
{
    set_short("This is the statue room");
    set_long("You are in a part of the Keepers' art collection. This is " +
        "where they keep the statues of some unfortunate adventurers. " +
        "Rumours have it that the Keepers once fought a great battle " +
        "against revolting Archwizards here and that the room still " +
        "contains strong magic from that time. Some visitors claim that " +
        "they have seen statues turn alive and disappear!\n\n");

    add_exit("/d/Standard/start/human/town/tower", "south", may_not_leave, 0);

    add_prop(ROOM_I_NO_ATTACK,  1);
    add_prop(ROOM_I_NO_CLEANUP, 1);
    add_prop(ROOM_I_LIGHT,   1000);

    setuid();
    seteuid(getuid());
}

/*
 * Function name: query_linkdead_players
 * Description  : Call this to find out all players that are linkdead.
 * Returns      : object * - the array of linkdead players, or ({ })
 */
public nomask mixed
query_linkdead_players()
{
    statues = filter(statues, objectp);
    nonpresent = filter(nonpresent, objectp);
    
    return filter((statues + nonpresent), &not() @ interactive);
}

/*
 * Function name: remove_linkdead_player
 * Description  : 
 */
static nomask void
remove_linkdead_player()
{
    object *players;
    int    size;
    int    delay = LD_PERIOD;
    int    *delays;

    alarm_id = 0;

    /* Get all existing objects still in this room. */
    statues = filter(filter(statues, objectp),
		     &operator(==)(this_object()) @ environment);

    /* Find all statues due for destruction. */
    players = filter(statues, &operator(<=)(, (time() - LD_PERIOD)) @
		     &->query_linkdead() );
    statues -= players;

    /* Find the moment the next statue should be destructed. */
    if (sizeof(statues))
    {
        delays = map(statues, &operator(+)(LD_PERIOD - time()) @
		     &->query_linkdead());
        size = sizeof(delays);
        while(--size >= 0)
            delay = min(delay, delays[size]);

        alarm_id = set_alarm(itof(delay), 0.0, remove_linkdead_player);
    }

    /* Destruct the statues. */
    size = sizeof(players);
    while(--size >= 0)
    {
        catch(players[size]->remove_object());
        if (objectp(players[size]))
        {
            SECURITY->do_debug("destroy", players[size]);
        }
    }
}

/*
 * Function name: nonpresent_linkdie
 * Description  : This function is called from the player when a player
 *                linkdies, but is not moved to this room yet.
 * Arguments    : object ob - the object that linkdies.
 */
public nomask void
nonpresent_linkdie(object ob)
{
    nonpresent -= ({ ob });
    nonpresent += ({ ob });
}

/*
 * Function name: nonpresent_revive
 * Description  : This function is called from the player when a player
 *                revives, but had not moved to this room yet.
 * Arguments    : object ob - the object that revives.
 */
public nomask void
nonpresent_revive(object ob)
{
    nonpresent -= ({ ob });
}

/*
 * Function name: linkdie
 * Description  : This function is called from the player when a player
 *                linkdies. We move him to the statue room and do some
 *                additional stuff.
 * Arguments    : object ob - the object that linkdies.
 */
public nomask void 
linkdie(object ob) 
{
    object editor;
    int    index;

    /* The player linkdied, so remove him from the nonpresent list. */
    nonpresent -= ({ ob });

    /* Save the contents of a possible editor the player carried. */
    if (objectp(editor = present(EDITOR_ID, ob)))
    {
	editor->linkdie();
    }

    /* If Armageddon is active and within 5 minutes of the reboot, make him
     * quit instantly.
     */
    if (ARMAGEDDON->shutdown_active() &&
        (ARMAGEDDON->query_delay() <= PASS_ARMAGEDDON))
    {
	set_this_player(ob);
        ob->catch_tell("Link lost during Armageddon. You are logged out.\n");
        /* If the player is still recovering his autoload string, that means
         * he just logged in. No point in quitting (and thus saving), but
         * let's simply destruct and preserve the playerfile.
         */
        if (ob->query_prop(PLAYER_I_AUTOLOAD_TIME))
        {
            ob->remove_object();
            return;
        }
        ob->quit();
	return;
    }

    if (member_array(ob->query_race(), RACES) < 0)
    {
	tell_room(environment(ob), ({
    	    capitalize(ob->query_real_name()) + " goes link dead.\n",
    	    "The " + ob->query_nonmet_name() + " goes link dead.\n",
    	    "" }),
	    ({ ob }) );
    }
    else
    {
    	tell_room(environment(ob), ({
    	    capitalize(ob->query_real_name()) + LINKDEATH_MESSAGE[ob->query_race()],
    	    "The " + ob->query_nonmet_name() + LINKDEATH_MESSAGE[ob->query_race()],
    	    "" }),
	    ({ ob }) );
    }

    tell_room(this_object(), "From a cloud, the statue of " +
	QNAME(ob) + " appears.\n ");

    /* Use the filename to prevent against reloaded rooms. */
    ob->add_prop(PLAYER_S_LD_IN_ROOM,
        (objectp(environment(ob)) ? file_name(environment(ob)) : 0));

    /* Save the props that will be altered in move_living(). */
    index = -1;
    while(++index < PROPS_TO_SAVE_SIZE)
    {
	ob->add_prop(STATUE_ROOM + props_to_save[index],
	    ob->query_prop_setting(props_to_save[index]));
    }

    /* Force the player to save and then move him here. */
    ob->save_me(1);
    ob->move_living("M", this_object(), 1);
    statues += ({ ob });

    /* If there is no alarm running yet, set the alarm. */
    if (!alarm_id)
    {
	alarm_id = set_alarm(itof(LD_PERIOD), 0.0, remove_linkdead_player);
    }
}

/*
 * Function name: revive
 * Description  : This function is called from the login object if a player
 *                revives from linkdeath. The player object is moved into
 *                his original room and some additional stuff is done.
 * Arguments    : object ob - the object that revives from linkdeath.
 */
public nomask void 
revive(object ob)
{
    int   index;
    mixed room = ob->query_prop(PLAYER_S_LD_IN_ROOM);

    /* Get the room where the player was. */
    if (stringp(room))
    {
	LOAD_ERR(room);
	room = find_object(room);
    }

    ob->remove_prop(PLAYER_S_LD_IN_ROOM);
    statues -= ({ ob });

    /* When the player comes from a destructed cloned room, or when there
     * is a problem moving the player, we try to move the player to his
     * default start location.
     */
    if (!objectp(room) ||
	ob->move_living("M", room, 1))
    {
	write("\nIt was not possible to move you back to the " +
            "location you were in before, so you shall be moved to " +
	    "your normal start location.\n");

	if (ob->move_living("M", ob->query_default_start_location(), 1))
	{
	    write("\nUnable to move you to your starting location. " +
		"This is serious. Let us try on your most default start " +
		"location.\n");

	    if (ob->move_living("M", ob->query_def_start(), 1))
	    {
		write("\nPANIC! Cannot move you!\n" +
		    "Contact a wizard immediately!\n\n");
	    }
	}

	/* We remove the properties that change during the move. */
        index = -1;
	while(++index < PROPS_TO_SAVE_SIZE)
	{
	    ob->remove_prop(props_to_save[index]);
	    ob->remove_prop(STATUE_ROOM + props_to_save[index]);
	}
    }
    else
    {
	/* We restore the properties that change during the move. */
        index = -1;
	while(++index < PROPS_TO_SAVE_SIZE)
	{
	    ob->add_prop(props_to_save[index],
		ob->query_prop_setting(STATUE_ROOM + props_to_save[index]));
	    ob->remove_prop(STATUE_ROOM + props_to_save[index]);
	}
    }

    if (member_array(ob->query_race(), RACES) < 0)
    {
	tell_room(environment(ob), ({
    	    capitalize(ob->query_real_name()) + " revives from link death.\n",
    	    "The " + ob->query_nonmet_name() + " revives from link death.\n",
    	    "" }),
	    ({ ob }) );
    }
    else
    {
	tell_room(environment(ob), ({
    	    capitalize(ob->query_real_name()) + REVIVE_MESSAGE[ob->query_race()] + "\n",
    	    "The " + ob->query_nonmet_name() + REVIVE_MESSAGE[ob->query_race()] + "\n",
    	    "" }),
            ({ ob }) );
    }

    tell_room(this_object(), "The statue of " + QTNAME(ob) +
	" is taken into a cloud and disappears.\n");

    /* If this was the last linkdead player, unset the alarm. */
    if (!sizeof(statues))
    {
        remove_alarm(alarm_id);
        alarm_id = 0;
    }
}       

/*
 * Function name: shutdown_slow_quit
 * Description  : This function will slowly make all players who are in the
 *                statue room quit. It uses a one second alarm to give each
 *                person his own eval-cost slot. If 'quit' does not work,
 *                then force is used.
 * Arguments    : object *statues - the players to destruct.
 */
static nomask void
shutdown_slow_quit(object *statues)
{
    statues = filter(statues, objectp);

    switch(sizeof(statues))
    {
    case 0:
        return;

    case 1:
        break;

    default:
	set_alarm(1.0, 0.0, &shutdown_slow_quit(statues[1..]));
    }

    set_this_player(statues[0]);
    /* We don't want the player to update his save vars during LD. */
    statues[0]->linkdead_save_vars_reset();
    catch(statues[0]->quit());

    if (objectp(statues[0]))
    {
	SECURITY->do_debug("destruct", statues[0]);
    }
}

/*
 * Function name: shutdown_activated
 * Description  : When shutdown is activated, this function is called to
 *                deal with the people who are linkdead. They are
 *                destructed.
 */
public nomask void
shutdown_activated()
{
    /* It may only be called from ARMAGEDDON. */
    if (!CALL_BY(ARMAGEDDON))
    {
	return;
    }

    /* No-one in the room means no show. */
    statues = query_linkdead_players();
    if (!sizeof(statues))
    {
	return;
    }

    /* Kick the players out one by one with a little delay. */
    set_alarm(1.0, 0.0, &shutdown_slow_quit(statues));

    /* If a linkdeath alarm was running, stop it. */
    if (alarm_id)
    {
        remove_alarm(alarm_id);
        alarm_id = 0;
    }
}

/*
 * Function name: enter_inv
 * Description  : This function is called whenever something enters the
 *                inventory of this room. If the object is non-living, it
 *                is destructed immediately.
 * Arguments    : object obj  - the entering object.
 *                object from - the possible old environment.
 */
public nomask void
enter_inv(object obj, object from)
{
    ::enter_inv(obj, from);

    /* Non-living means removal. */
    if (!living(obj))
    {
	catch(obj->remove_object());

	/* If it is not gone yet, hammer hard. */
	if (objectp(obj))
	{
	    SECURITY->do_debug("destruct", obj);
	}
    }

    /* The player linkdied, so remove him from the nonpresent list. Just to
     * make sure in case it wasn't done yet.
     */
    nonpresent -= ({ obj });
}

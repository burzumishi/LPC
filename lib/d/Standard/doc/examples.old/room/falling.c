/*
 * falling.c
 */

/*
 * Anyone enter this room is in trouble. He is falling quickly towards the
 * ground.
 */
#pragma strict_types
#pragma no_clone

inherit "/std/room";

#include "ex.h"		/* Include our definitions */
#include <macros.h>	/* Many nice macros, among them QCTNAME */
#include <stdproperties.h> /* Properties, LIVE_S_EXTRA_SHORT */
#include <language.h>	/* Some nice definitions when playing with words. */

/*
 * Prototypes
 */
int quit(string str);

/*
 * Normal creation of the room
 */
void
create_room()
{
    set_short("Falling");
    set_long("You are falling rapidly towards the ground. You have a nice\n" +
	"view from here though....\n");

    /*
     * Fighting when falling is out of the question :) The ROOM_M_NO_ATTACK
     * prop should see to that. If you set it to 1, then standard messages
     * will be used, but you can also define a message on your own to be
     * printed if a player tries to attack someone. The 'M' stands for 
     * Mixed type, the prop can have both an integer setting as well as
     * a string type setting.
     */
    add_prop(ROOM_M_NO_ATTACK, "You cannot fight while falling.\n");

    /*
     * This room is hardly of normal type (default) but in the air..
     */
    add_prop(ROOM_I_TYPE, ROOM_IN_AIR);
}

/*
 * The init
 */
void
init()
{
    ::init();

    add_action(quit, "quit");	/* Don't let anyone quit himself out. */
}

/*
 * It should not be easy escape the fall
 */
int
quit(string str)
{
    write("No no, you can't escape this fall that easy.\n");
    return 1;
}

/*
 * enter_inv() is called each time an object enters this container. (All rooms
 * are containers since /std/room.c inherits /std/container.c) If an object
 * enters this room, let it fall.
 */
void
enter_inv(object ob, object from)
{
    /*
     * Let the old version of enter_inv() do what it wants to do first,
     * then we'll take over.
     */
    ::enter_inv(ob, from);

    /*
     * living() is an efun and returns true if ob is a living object. No
     * use telling dead objects anything....
     *
     * LIVE_S_EXTRA_SHORT is a nice way to add a string to the short
     * description of a living. If a player looks in this room he should
     * get the impression other players in here are also falling.
     */
    if (living(ob))
    {
	tell_object(ob, "You feel unhappy not standing on solid ground.\n");
	add_prop(LIVE_S_EXTRA_SHORT, " is also falling.\n");
    }

    /*
     * call_out() is a nice efun that makes it possible to easily call a
     * function after a specified amount of seconds. Let's wait a few seconds
     * and then let the player pass halfway down. The first argument is the
     * function name, the second how many seconds to wait and the last is
     * any argument I want to be send to the function named.
     */
    call_out("halfway", 3, ob);
}

/*
 * halfway(), if the obejct is still in this room, then tell people on the 
 * trunk of the tree that something fell past them and tell the object, if
 * living, that it's falling rapidly.
 */
void
halfway(object ob)
{
    /*
     * If the object for some reason is not in the room, quit the falling
     */
    if (!ob || !present(ob, this_object()))
	return;

    tell_room(EX_ROOM + "trunk", "You hear a big noise and suddenly you see ");

    /*
     * We must split up the living and dead objects that are falling since
     * their short descriptions are a little different. QTNAME is a VBFC and
     * if people know the ob it will return the name of ob, else the nonemet
     * description of ob will be written, with a 'the' infront. Notice that
     * we don't use QCTNAME.
     *
     * LANG_ASHORT adds an article to the short description of ob, 'a' or
     * 'an'. If the ob is a heap it has already a full short description.
     * The LANG_ASHORT macros does the job for us and returns a nice short
     * description of the object with 'a' or 'an' infront of it unless it's
     * a heap.
     */
    if (living(ob))
    {
	tell_object(ob, "The ground is getting closer.\n");
	tell_room(EX_ROOM + "trunk", QTNAME(ob));
    }
    else
	tell_room(EX_ROOM + "trunk", LANG_ASHORT(ob));

    tell_room(EX_ROOM + "trunk", "  fall through the leaves and " +
	    "continue down.\n");

    call_out("land", 3, ob);
}

/*
 * land(), called when the object finally reaches the ground.
 */
void
land(object ob)
{
    /*
     * If the object for some reason is not in the room, quit the falling
     */
    if (!ob || !present(ob, this_object()))
	return;

    tell_room(EX_ROOM + "at_tree", "You hear a terrible noise from above, " +
	"suddenly you see ");

    if (living(ob))
    {
	tell_room(EX_ROOM + "at_tree", QTNAME(this_player()) + " fall down.\n");

	/*
	 * Here I choose to move the player with move() even though he is
	 * a living object. Since teams shouldn't follow him in any case
	 * and we print the enter and leave messages and you shouldn't
	 * be able to fight in this room, it's ok to do that. Note that
	 * this is an exception. Use move_living() in 99% of the cases :)
	 */
	ob->move(EX_ROOM + "at_tree");

	tell_object(ob, "You hit the ground with a thump! It hurt!!\n");
	ob->heal_hp(-400);

	/*
	 * Now check if the hp of the object is down to 0, if so kill him!
	 * Muhahahahahahahahahaha :)
	 *
	 * If you call the function do_die() in a player he will die, if he
	 * has 0 hp left, and is no wizard :)
         */
	if (ob->query_hp() < 1)
	    ob->do_die(this_object());
    }
    else
    {
	tell_room(EX_ROOM + "at_tree", LANG_ASHORT(ob) + " fall down.\n");
	ob->move(EX_ROOM + "at_tree");
    }
}

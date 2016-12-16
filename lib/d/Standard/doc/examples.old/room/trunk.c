/*
 * trunk.c
 */

/*
 * Now the player is midway up the tree. The climbing gets tougher if he
 * continues up.
 */
#pragma strict_types
#pragma no_clone

inherit "/std/room";

#include "ex.h"		/* Include our definitions */
#include <ss_types.h>	/* Skill and stat definitions */
#include <macros.h>	/* Many nice macros, among them QCTNAME */

/*
 * Prototypes
 */
int try_climb(string str);

/*
 * Normal creation of the room
 */
void
create_room()
{
    set_short("On trunk");
    set_long("You are halfway up the tree, you can climb up or down.\n");

    add_item( ({ "tree", "big tree" }), 
	"This close it looks huge.\n");
}

/*
 * The init
 */
void
init()
{
    ::init();

    add_action(try_climb, "climb");
}

/*
 * Try to climb.....
 */
int
try_climb(string str)
{
    /*
     * This time we expect the player to 'climb up' or 'climb down'
     */
    notify_fail("Climb up or down?\n");

    if (!str)
	return 0;

    if (str == "up")
    {
	/*
	 * The climbing gets tougher now, and we add some randomness to it.
	 * The efun random() returns a nice random number. Try 'man random'
	 * as usual for efuns...
	 */

	if (this_player()->query_skill(SS_CLIMB) <= 20 ||
		random(this_player()->query_skill(SS_CLIMB)) < 10)
	{
	    /*
	     * Oh, oh. The player tried to climb up but slipped. This time 
	     * he falls much longer but will still not die from it. (Yes he
	     * will be able to die if he falls from the top....)
	     */
	    write("You start to climb but slip and fall all the way down "+
		"and hurt yourself.\n");
	    say(QCTNAME(this_player()) + " starts to climb the tree but " +
	    	"slips and falls all the way down.\n");
	    this_player()->heal_hp(-30);

	    /*
	     * Tell people if they are standing at the base of the tree what
	     * happens.
	     */
	    tell_room(EX_ROOM + "at_tree", "Suddenly you hear a lot of " +
		"noise from above and " + QTNAME(this_player()) + " falls " +
		"down with a thump!");

	    /*
	     * This time we have to move the player to the base of the tree.
	     * We use the "M" as how because we have written all the messages
	     * needed ourselves. If the leader falls we don't force the 
	     * players who choosed him as leader to fall too, aren't we nice?
	     */
	    this_player()->move_living("M", EX_ROOM + "at_tree", 1);
	    return 1;
	}

	/*
	 * Well, guess the player managed to climb up then...
	 */
	this_player()->move_living("climbing up the trunk", EX_ROOM + "top", 1);
	return 1;
    }

    if (str == "down")
    {
	/*
	 * Climbing down is easier and there is no chance of falling there.
	 */
    	this_player()->move_living("climbing down", EX_ROOM + "at_tree", 1);
    	return 1;
    }

    /*
     * If we get here the player tried to specify how to climb but didn't
     * write 'up' or 'down'. Let's return 0 and show him the notify_fail()
     * we set in the beginning of this function.
     */
    return 0;
}

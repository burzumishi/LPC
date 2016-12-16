/*
 * top.c
 */

/*
 * Top of the tree, here is an elf that will push you out of the tree if
 * you attack him. It's also not adviced to be jumping up here. You can
 * throw items down the tree too.
 */
#pragma strict_types
#pragma no_clone

inherit "/std/room";

#include "ex.h"		/* Include our definitions */
#include <macros.h>	/* Many nice macros, among them QCTNAME */
#include <cmdparse.h>	/* FIND_STR_IN_OBJECT macro */
#include <composite.h>	/* COMPOSITE macros, used when describing objects */

/*
 * Prototypes
 */
int try_climb(string str);
int jump(string str);
int throw(string str);

/*
 * reset_room(), add and elf to this room.
 */
void
reset_room()
{
    /*
     * Since there are many elves in the world, we'll have to give this 
     * elf an special name. If we were to test if someone with name 'elf'
     * was here, any elf could have broken the reset, and only our special
     * elf should be able to do that, so let's give him a special name
     */
    if (!present("tree_elf", this_object()))
	clone_object(EX_MON + "tree_elf")->move(this_object());
}

/*
 * Normal creation of the room
 */
void
create_room()
{
    set_short("Top of tree");
    set_long("You are high up in the tree. The ground is far far below.\n" +
	"It would be foolish to jump up here.\n");

    reset_room();
}

/*
 * The init
 */
void
init()
{
    ::init();

    add_action(try_climb, "climb");
    add_action(jump, "jump");
    add_action(throw, "throw");
}

/*
 * Try to climb.....
 */
int
try_climb(string str)
{
    /*
     * This time we expect the player to 'climb down'
     */
    notify_fail("Climb what, down?\n");

    if (!str || str != "down")
	return 0;

    /*
     * Climbing down is easier and there is no chance of falling there.
     */
    this_player()->move_living("climbing down", EX_ROOM + "at_tree", 1);
    this_player()->add_fatigue(-5);
    return 1;
}

/*
 * Jump, like climbing. If some stupid player tries to jump he will fall down.
 * This time the fall is quite long and if you have 0 hp after the fall you 
 * will die *grin*. We don't care what argument player gave to the jump command
 * this time. Just let him fall. this_player() will be moved to a room
 * that controls the fall better.
 */
int
jump(string str)
{
    write("You try to jump as high as possible and you slip. It's seems to " +
	"be a long way down.\n");
    say(QCTNAME(this_player()) + " foolishly jumps and falls from the tree.\n");

    this_player()->move_living("M", EX_ROOM + "falling", 1);
    return 1;
}

/*
 * throw(), throw something off the tree. It will fall the same way a living
 * is falling.
 */
int
throw(string str)
{
    object *arr;	/* Local variables, array of objects */
    int i;		/* Local variable, integer, used in the for-loop */

    if (!str)
    {
	notify_fail("Throw what?\n");
	return 0;
    }

    /*
     * FIND_STR_IN_OBJECT finds all objects matching a str in a certain
     * certain object. str can be 'two swords' or 'third red apple' and so on.
     * If the macro finds no matching objects then it returns an empty array.
     * sizeof() is the efun to use to check the size of an array.
     */
    arr = FIND_STR_IN_OBJECT(str, this_player());

    if (!sizeof(arr))
    {
	notify_fail("You don't have any '" + str + "'.\n");
	return 0;
    }

    /*
     * Ok, let's throw away all the objects the player wanted to the, move
     * them to the falling room in other words. We need to run a small for-
     * loop and move them one by one.
     */
    for (i = 0; i < sizeof(arr); i++)
	arr[i]->move(EX_ROOM + "falling");

    /*
     * COMPOSITE_DEAD returns a string describing the array of objects. It's
     * this kind of array you get when you do 'inventory' or 'i'. Since not
     * all objects are seen the same way by everyone, perhaps a few objects
     * are invis, or look different to guild members or something, we can't
     * use the same string in the say() statement. QCOMPDEAD sets up a VBFC
     * that will fix composite descriptions, depending on who the reciever
     * is and will use the same array as was last used by any composite
     * command. If all objects are unseen to the reciever, 'something' will
     * be returned.
     */
    write("You throw " + COMPOSITE_DEAD(arr) + " off the tree.\n");
    say(QCTNAME(this_player()) + " throws " + QCOMPDEAD + " off the tree.\n");
    return 1;
}

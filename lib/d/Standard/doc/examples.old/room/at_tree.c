/*
 * at_tree.c
 */

/*
 * In this room there is a tree wich you can climb. We make a check for
 * climbing skill and hurt the player if he falls down.
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
    set_short("At big tree");
    set_long("You are standing at a big tree, with road west. The tree is\n" +
	"possible to climb.\n");

    add_item("road", "It looks terrible long\n");
    add_item( ({ "tree", "big tree" }), 
	"Well, maybe the tree is not so easy to climb after all.\n");

    add_exit(EX_ROOM + "road1", "west");
}

/*
 * init() is a nice function. It's called each time objects meet eachothers.
 * If a player enters a room, then init() is called in the room. If there are
 * other objects in the room, then init() is called in them too. The is
 * normally a very good place to add commands to a player. You can do that
 * by redefining the function to suite your needs. Note that in monsters
 * the function to redefine is init_living(). Since there are so many things
 * to do init() has become nomask (= not redefinable), but let you use
 * init_living().
 *
 * Mostly when using init() we just want a few things more to happen when
 * the init() function is called. Therefor we call the old version of init()
 * from our version, so nothing is lost. Failing to call the old version of
 * init() will make it impossible to leave a room, since the normal init()
 * adds the commands north, west, south and so on to the player. (Yes, if
 * you choosed to add those functions as well then you would not have to call
 * the old version, but why do the work when someone has already done it for
 * you? The '::' infront of the function name indicates that you want to
 * call the old version of that function.
 *
 * add_action() is the function that adds a command to the player. The first
 * argument is the function name that should be called when player gives the
 * second argument as command. add_action() is an efun and can be studied by
 * 'man add_action'.
 */
void
init()
{
    ::init(); /* This calls the normal init() function then */

    add_action(try_climb, "climb");
}

/*
 * As explained earlier try_climb() will be called when the player tries
 * the 'climb' command. This function returns 1 to indicate that the command
 * was a success. If we failed to do what the player asked us to do we return
 * 0 instead. Retrning 0 will give any other object defining the verb 'climb'
 * the chance to do something. Perhaps someone has put a huge boulder in
 * the room that's climbable as well..... This function is of int (integer)
 * type then.
 *
 * str is the argument send to this function. If the player gives the command
 * 'climb tree' then str will be the string 'tree'. Therefor the argument is
 * of string type.
 */
int
try_climb(string str)
{
    /*
     * notify_fail() is a neat function. With it we set the string that shall
     * be written in case this command fails. Returning 0 without setting this
     * will result in the popular 'What?'. Since we want to give other objects
     * the chance to handle a 'climb' command, we shouldn't write a fail
     * messege directly either, perhaps the player didn't try to climb the 
     * tree at all.....
     */
    notify_fail("Climb what?\n");

    if (!str)
	return 0;	/* If the player just typed 'climb' */

    if (str != "tree" && str != "big tree")
	return 0;	/* Player didn't typ 'climb tree' or
		 	 * 'climb big tree' */

    /*
     * Ok, player is trying to climb the tree. Let's see if he has the skill
     * to do it. If the player has climb skill > 10 then there is no problem.
     * SS_CLIMB is defined in /sys/ss_types.h, this_player() is the object
     * (player) who gave the command.
     */
    if (this_player()->query_skill(SS_CLIMB) <= 10)
    {
	/*
	 * Oh, oh. The player tried to climb the tree but couldn't. Write
	 * something and hurt him! write() is a efun and the argument is
	 * written exactly as given to this_player(). write(str) is the same
	 * thing as tell_object(this_player(), str); This means that VBFC:s
	 * are not useable with tell_object() and write(). If you want to
	 * send a message with VBFC:s in it to this_player() then use the
	 * lfun catch_msg():  this_player()->catch_msg(str);
	 *
	 * say() is a nice lfun too. It will send a message to every living
	 * object in the room that is not this_player(). You can specify more
	 * objects not to send the message to with a second argument.
	 * 'sman say' and 'man meet_people' both will show you more info 
	 * about the function. say() is a special case of tell_room() just
	 * like write() is a special case of tell_object().
	 *
	 * QCTNAME is a nice macro to use with say(). It sees to it that each
	 * living object gets the met or unmet name of the player depending 
	 * on there status. This is a VBFC call wich is why it won't work in
	 * a write() statement, as said above. If the living object can't see
	 * this_player() then 'Someone' will be written instead of his name.
	 * CT stands for Capitalized The, it means the name will be capitalized
	 * and have a 'The' in front of it if unmet status. There are also
	 * QTNAME, QCNAME and QNAME.....
	 */
	write("You start to climb but slip and fall down and hurt yourself.\n");
	say(QCTNAME(this_player()) + " starts to climb the tree but slips " +
	    "and falls down.\n");

	/*
	 * Hurt the player, don't let him die from the fall though....
	 * If the heal_hp() call would lead to negative hp in the player
	 * the player would get 0 hp *shiver*
	 */
	this_player()->heal_hp(-10);

	return 1; /* Well, perhaps the player doesn't think his command was
		   * a success but we took care of it and performed the task
		   * so we don't want to player shall try to climb the boulder
		   * too with this command... */
    }

    /*
     * If we get here it means the player managed to climb. Then we use the
     * function move_living(). You've seen earlier how we use move() on dead
     * objects but when players move between rooms we want messages to be
     * written and fights to be updated and so on. That's why we use 
     * move_living() on living objects.
     *
     * The first argument to move_living() indicates how the move is done.
     * The text other sees in this case would be '<name> leaves climbing
     * the tree'. If the move is a teleportation use "X" to indicate how and
     * if you prefere to write your own arrive and leave messages instead of
     * the default let it be "M". Second argument is where to move the living.
     * Third argument can be used to indicate if a group shall try to follow
     * their leader. In this case it might be a good idea to let each member
     * in the group try to climb the tree alone, and not auto follow their
     * leader and perhaps fall down and get hurt...
     */
    this_player()->move_living("climbing the tree", EX_ROOM + "trunk", 1);
    return 1;
}

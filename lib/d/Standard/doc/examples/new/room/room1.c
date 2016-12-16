/*
 * This is the first room in the example area. Here you will hopefully 
 * find most of the examples you need to create your own glorious area.
 *
 * /Nick
 */

/*
 * Almost every object must inherit something. We'll make a room now so
 * we inherit /std/room. If it was food we were going to make /std/food
 * would have to be inherited.
 */

inherit "/std/room";

#include "ex.h"		/* Include our definitions */

/*
 * All objects need a create_xxx() function where xxx is the current
 * type of object we are working with. This is a room, so well use
 * create_room(). This function will be called when this object is
 * created, like cloned, or loaded. (If someone walks into a room that
 * room will naturally be loaded.)
 *
 * The 'void' thing that is written in front of this function means
 * that this function has no return value. Some people think using types
 * on functions and arguments is a nuisance, but it helps you find bugs
 * easier since the compiler can follow 'sharper' rules. If you use types
 * all functions must be defined in the right order. I create_object() down
 * below where to call another function. That function must have been
 * defined already, otherwise an error will occur.
 */
void
create_room()
{
   /*
    * Set short description of the room. This text will be shown if
    * players uses 'brief' - mode. As in all short descriptions no
    * ending \n (carrige return)
    */
    set_short("The first room");

    /*
     * Set long description of the room. It is normally shown to players 
     * entering the room and when they do 'look'.
     */
    set_long("You are standing in the first example room. A long road\n" +
	"leads north and there is a small cave west. The way back\n" +
	"south looks blocked.\n");

    /*
     * We mentioned a road in the long description. Let the player be
     * able to 'look at road'
     */
    add_item("road", "It looks terrible long\n");

    /*
     * The cave was also mentioned. Using and array like
     *    ({ "cave", "small cave" })   will make it posible to both
     * 'look at cave' and 'look at small cave'
     */
    add_item( ({ "cave", "small cave" }),
	"You cannot see a thing in there, it's completely dark.\n");

    /*
     * Some exits out of the room are needed. The cave is west from
     * here. The first argument is the file name, second the command,
     * third will be discussed below, and forth is how tiresome it
     * is to walk that way. 1 is the default setting and will be used
     * if not specified.
     *
     * This argument: If it is 0, then everything works as normal.
     * 1 means that the player don't move, and that no other exits
     * are tried. 2 stops the player, but looks if there is another
     * exit in that direction and let the player try to leave that way.
     * 2 is a very good value if you want to make a 'enter mansion'
     * direction. I will show an example of this later at the town
     * square. If it's a negative number, then the player will have to
     * walk through a corridor of rooms. (Like the north exit, try it
     * if you want to see what it is.)
     *
     * EX_ROOM is defined in the file I included at the top (ex.h)
     * I defined this to make it easy to move this directory and
     * not to have to write so long path names all the time. If I
     * hadn't used it, the add_exit() would have looked like this:
     *    add_exit("/d/Genesis/doc/examples/room/cave", "west", 0, 1);
     *
     */
    add_exit(EX_ROOM + "cave1", "west", 0, 1);

    /*
     * This is an example of the corridor room. Try it out and you will
     * hopefully undertsand what this does. You have to walk north 3 times
     * before you reach the road1 room.
     */
    add_exit(EX_ROOM + "road1", "north", -3);

    /*
     * Here we use VBFC (Value By Function Call) that means that instead
     * of having that value set to a number it is now set to a function
     * name that shall be called each time we want to know that setting.
     * The number returned by that function will be used.
     */
    add_exit(EX_ROOM + "block", "south", "@@block");
}

/*
 * This function is called each time someone tries to go south. If we
 * were to return 0 from this function then player would move as 
 * usually. This is a good way to block off a path.
 * 'int' means this function returns an integer.
 */
int
block()
{
    write("The way south is not yet open to the public.\n");
    return 1;

    /*
     * If we were to use the possibility with many exits with the same
     * exit command, we would had written something like this instead.

    notify_fail("The way south is not yet open to the public.\n");
    return 2;

     * notify_fail() sets the message the play will get in case his 
     * command fails. Standard message is 'What?' which you may have
     * seen a number of times as mortal....
     */
}

/*
 * If you wonder what a function does there is a very neat command to use
 * called sman. Try 'help sman' to see all it's options. If you wonder
 * what the function add_exit() does and what its different arguments you
 * simply give the command 'sman add_exit'. All functions defined in
 * objects in the mudlib should be possible to read with the sman command.
 * If someone is missing, or you think the information you get is bad, or
 * not updated, either use the sysbug command, or contact an Arch.
 *
 * Functions like notify_fail() and write() are what we call efuns. They
 * are defined inside the Game Driver. You can get manual pages on them 
 * with the 'man' command.
 *
 * Files found in /doc/man/ can be read with the 'man' command and files
 * found in /doc/sman/ with 'sman'. These two commands will help you a lot
 * if you can learn how to use them.
 */

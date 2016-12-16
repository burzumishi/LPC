/*
 * /d/Standard/login/mirror_1.c
 *
 * This is the first of two mirror rooms where player's select their
 * height and width. In this one, they select their height by choosing
 * from various shaped mirrors.
 *
 * Khail, May 17/96
 */

#pragma strict_types

#include <macros.h>
#include <stdproperties.h>
#include <language.h>
#include <composite.h>
#include <std.h>
#include "login.h"

#define TP this_player()

inherit "/std/room";

public mixed resolve_mirror(string which);
public int check_bodies_exit();
public int read_book();
public int all_cmd(string str);
public int examine_mirrors(string str);
public int enter_something(string str);
public int get_help(string str);

/*
 * Function name: extra_long
 * Description  : Modifies the long description if the player shouldn't
 *                be here.
 * Arguments    : n/a
 * Returns      : The extra string, if any.
 */
public string
extra_long()
{
  /* Give an extra message to the long if the player shouldn't be */
  /* setting height/weight */
    if (!(TP->query_ghost() & GP_MANGLE))
        return "You realize you shouldn't be here, and might as well " +
            "'enter first mirror', as it won't matter which one you " +
            "enter at the moment.\n\n";
    else
        return "";
}

/*
 * Function name: resolve_mirror
 * Description  : Decides which mirror the player is trying to enter
 *                or examine. Allows for a little more variety in the
 *                arguments they can use.
 * Arguments    : which - String which is the argument passed from
 *                        commands like 'exa' or 'enter'
 * Returns      : int - An integer specifying which index of the mirrors
 *                      array the player wants. I.e. exa first mirror -> 0
 *                string - A fail message if the specified mirror does not
 *                         exist. I.e. exa purple mirror -> There is no
 *                         purple mirror.
 */
public mixed
resolve_mirror(string which)
{
    int index;
    string fail;

  /* Try to determine a number, i.e. if the player examined first mirror. */
    index = LANG_ORDW(which) - 1;

  /* If the index is negative, the player probably specified a mirror */
  /* by name, i.e. 'exa tall mirror', look for it in the HEIGHTDESC */
  /* array. */
    if (index < 0)
    {
      /* If the mirror's in the heightdesc array, return the index. */
        if (member_array(which, HEIGHTDESC) >= 0)
            index = member_array(which, HEIGHTDESC);
      /* Not in the heightdesc array, return fail. */
        else
            fail = "There is no " + which + " mirror.\n";
    }

  /* If the player did 'exa seventh mirror', or any other number */
  /* beyond the numberof mirrors, send a fail message. */
    if (index >= sizeof(HEIGHTDESC))
        fail = "There aren't that many mirrors.\n";

  /* Return fail message if one was made. */
    if (fail && strlen(fail))
        return fail;

  /* Otherwise we found a correct mirror, return the value. */
    else
        return index;
}

/*
 * Function name: create_room
 * Description  : Turns this container into a real room.
 * Arguments    : n/a
 * Returns      : n/a
 */
public void
create_room()
{
    set_short("hall of mirrors");
    set_long("You find yourself in a very strange room, surrounded by " +
        "mirrors of all shapes and sizes. You somehow feel that you " +
        "should look at them, and that it might even be possible to " +
        "enter one to leave this place. In the center of the room " +
        "a pedestal supports a heavy book, open to a single, well-" +
        "worn page.\n\nIf you get really stuck, 'help here' might " +
       "provide some help.\n\n" +
       "@@extra_long");
    add_item(({"pedestal"}),
        "A large pedestal stands in the middle of the room. An " +
        "important looking book rests atop it, probably worth " +
        "reading.\n");
    add_item(({"book"}),
        "It's got writing in it, you should probably read it.\n");
    add_cmd_item(({"writing", "book"}), ({"read"}),
        "@@read_book");

    add_exit(PATH + "bodies", "bodies",
        "@@check_bodies_exit");
}

/*
 * Function name: check_bodies_exit
 * Description  : Function called by VBFC from the bodies exit, resets
 *                the players ghost var so they can enter a new body
 *                again.
 * Arguments    : n/a
 * Returns      : 0 - Allow player to proceed through the exit.
 */
public int
check_bodies_exit()
{
    TP->set_ghost(TP->query_ghost() | GP_BODY);
    return 0;
}

/*
 * Function name: read_book
 * Description  : Allows players to read the book here for info on what
 *                to do.
 * Arguments    : n/a
 * Returns      : 1 - Player read this book, stop threading.
 */
public int
read_book()
{
    string ret;

    ret = "\nWelcome to the first Hall of Mirrors. Here you may select the " +
        "height of your new body. You can accomplish this by examining " +
        "the various mirrors, and entering whichever mirror holds the " +
        "form you like. For example, 'enter first mirror' if you want " +
        "to be extremely short.\n";
    ret += "Once you enter a mirror, you will pass beyond with your " +
        "new height, into the second Hall of Mirrors where you will " +
        "choose your width.\n";
    ret += "Note this will be permanent after you have entered the " +
        "world, although you may return here from any point previous " +
        "to that. You will be warned once before the final exit, wherever " +
        "it is.\n";
    write(ret + "\n");
    return 1;
}

/*
 * Function name: init (MASKED)
 * Description  : Called when another object enters either the inventory
 *                or environment of this object. Here, it's masked so 
 *                we can use add_action.
 * Arguments    : n/a
 * Returns      : n/a
 */
public void
init()
{
    ::init();
    add_action(all_cmd, "", 1);
}

/*
 * Function name: all_cmd
 * Description  : This is a command filter that is used by the add_action
 *                in the init() fun. Any command the player attempts to
 *                use is filtered through here, so we can restrict the
 *                things a player tries to do.
 * Arguments    : str - String containing the arguments passed by the
 *                      player.
 * Returns      : 0 - Keep threading.
 *                1 - Stop threading.
 */
public int
all_cmd(string str)
{
    object tp;
    tp = this_player();

    switch(query_verb())
    {
        case "commune":
        case "bug":
        case "typo":
        case "praise":
        case "idea":
        case "'":
        case "say":
        case "quit":
        case "save":
        case "sysbug":
        case "read":
        case "bodies":
            return 0;
            break;
        case "look":
        case "l":
        case "exa":
        case "examine":
            return examine_mirrors(str);
            break;
        case "enter":
            return enter_something(str);
            break;
        case "help":
            return get_help(str);
            break;
        default:
            if (SECURITY->query_wiz_rank(tp->query_real_name()) >= WIZ_NORMAL)
                return 0;
            write("You struggle, but find that is impossible to do here.\n");
            return 1;
    }
}

/*
 * Function name: enter_something
 * Description  : Called when the player's tried to enter something.
 * Arguments    : str - String containing what the player tried to enter.
 * Returns      : 1 - Success.
 *                0 - Failed.
 */
public int
enter_something(string str)
{
    string which;
    int height,
        index,
        *attr;

  /* An array of racial attributes based of this player's race. */
    attr = RACEATTR[TP->query_race()];

    notify_fail("Enter what? One of the mirrors?\n");

  /* Make sure player specified they wanted to enter a mirror somehow. */
    if (!str || !strlen(str) || sscanf(str, "%s mirror", which) != 1)
        return 0;

  /* Fail if the specified mirror does not exist. */
    if (stringp(resolve_mirror(which)))
    {
        notify_fail(resolve_mirror(which));
        return 0;
    }

  /* What index was the specified mirror? */
    index = resolve_mirror(which);

  /* Make double-sure the index is an existing mirror. */
    if (index >= sizeof(HEIGHTDESC))
    {
        notify_fail("Please enter either one of the following mirrors: " +
            COMPOSITE_WORDS(HEIGHTDESC) + ".\n");
        return 0;
    }

  /* Make sure the player is supposed to be here. */
    if (!(TP->query_ghost() & GP_MANGLE))
    {
        write("\nYou suddenly realize that you aren't supposed to be here, " +
            "and find yourself moving along with no changes being " +
            "made.\n\n");
      /* Move the player to whichever room they are supposed to be in, */
      /* based on their ghost_var */
        if (TP->query_ghost() & GP_FEATURES)
        {
            TP->move_living("X", PATH + "features");
            return 1;
        }
        else if (TP->query_ghost() & GP_SKILLS)
        {
            TP->move_living("X", PATH + "features");
            return 1;
        }
        else
        {
            if (MASTER_OB(TP) == LOGIN_NEW_PLAYER)
            {
                TP->set_ghost(0);
                TP->ghost_ready();
            }
            else
                TP->move_living("X", TP->query_default_start_location());
            return 1;
        }
    }

  /* Ok, player entered a mirror. Set their height prop, and send them */
  /* on their way for width. */
    write("\nYou step into the " + which + " mirror, and surprisingly find " +
        "yourself in another room rather than chewing glass, as " +
        LANG_ADDART(HEIGHTDESC[index]) + " " + 
        TP->query_race() + ".\n\n");

    height = (attr[0] * SPREAD_PROC[index]) / 100 - random(attr[0] / 20);

    TP->add_prop(CONT_I_HEIGHT, height);
    TP->move_living("X", PATH + "mirrors_2");

    return 1;
}

/*
 * Function name: examine_mirrors
 * Description  : Lets the players examine the various mirrors.
 * Arguments    : str - String containing what the players were trying
 *                      to examine.
 * Returns      : 1 - Success, found a mirror to look at.
 *                0 - Fail, no mirror to look at.
 */        
public int
examine_mirrors(string str)
{
    int index;
    string ret,
           which,
           temp,
           *arr;

  /* Fail if player didn't send an argument. */
    if (!str || !strlen(str))
        return 0;

  /* If the first arg is 'at', i.e. if the player used 'look at', */
  /* strip it out of the argument string. */
    arr = explode(str, " ");
    if (arr[0] == "at")
        str = implode(arr[1..], " ");

  /* What players see if they 'exa mirrors' */
    if (str == "mirrors")
    {
        write("\nGazing into the various mirrors, you see yourself looking " +
            "into images of yourself in different heights:\n" +
            COMPOSITE_WORDS(HEIGHTDESC) + ".\n\n");
        return 1;
    }

  /* Did they try tp examine a specific mirror? */
    if (sscanf(str, "%s mirror", which) != 1)
    {
        notify_fail("Examine which mirror?\n");
        return 0;
    }

  /* If they did try to examine a specific mirror, which one, if it */
  /* exists? */
    if (stringp(resolve_mirror(which)))
    {
        notify_fail(resolve_mirror(which));
        return 0;
    }

  /* Ok, we've got an index pointer of which mirror to examine. Tell */
  /* the player what they see. */
    index = resolve_mirror(which);

    ret = "Gazing into the mirror, you see yourself looking ";
    ret += HEIGHTDESC[index] + ". ";
    ret += "You get the impression by entering the " + which + " mirror " +
        "you might permanently look that way.\n";

    write("\n" + ret + "\n");

    return 1;
}

/*
 * Function name: get_help
 * Description  : Returns the 'extended' help file.
 * Arguments    : str - String argument player passed to the 'help'
 *                      command. This function only reacts to 'help here'
 * Returns      : 1 - Success, stop thread.
 *                0 - Fail, keep threading.
 */
public int
get_help(string str)
{
    if (!str || !strlen(str))
        return 0;

    if (str == "here")
    {
        TP->more(read_file(HELP + "height_help"));
        return 1;
    }

    return 0;
}

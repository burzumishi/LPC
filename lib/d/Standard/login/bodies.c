/*
 * /d/Standard/login/bodies.c
 *
 * Room where players go to get real bodies, either upon login, or after
 * they die.
 *
 * Khail, May 13/96
 */

#pragma strict_types

#include "login.h"
#include <std.h>
#include <macros.h>
#include <cmdparse.h>
#include <ss_types.h>

inherit "/std/room";

/* Prototypes, alphabetical order */
public int command_lock(string str);
public int can_possess_body(object ob);
public void create_room();
public int do_possess_body(object ob);
public int enter_portal(string str);
public int finish_possess_body(object ob);
public void init();
public void make_the_bodies();
public void possess_body(string str);
public int read_book(string str);
public void replace_body(string race, int gender);

private mapping RaceStatMod = RACESTATMOD;

/*
 * Function name: extra_long
 * Description  : Called by vbfc from the long desc to add an extra message
 *                telling the player to get out if they aren't a ghost.
 * Returns      : str - A string described above.
 */
public string
extra_long()
{
    if (this_player()->query_ghost() & GP_BODY)
        return "You feel these bodies require souls, and you require a " +
            "body. Perhaps you can possess one.";
    else
        return "You feel you have no business in this place of the dead, " +
            "and should leave by 'enter portal'.";
}

/*
 * Function name: create_room
 * Description  : set up the room itself, description, bodies. etc.
 */
public void
create_room()
{
    set_short("solemn chamber");
    set_long("You are in a strange and solemn chamber, made out of " +
        "white marble assembled to form a massive dome. A strange " +
        "light permeates the room, centered about a number of bodies " +
        "laid out upon stone altars throughout the room. In the center " +
        "of the room upon a pedestal rests a large book. In the north " +
        "wall, an arched portal pulses with a brilliant white light. " +
        "@@extra_long@@\n\n");
   
    add_item(({"portal"}),
        "The arched portal glows with an eerie, but brilliant " +
        "white light. You can't see what's on the other side, " +
        "but you have a feeling you'll find out once you have " +
        "a real body again.\n");
    add_item(({"altars","altar"}),
        "Upon each item lies a body, perhaps worth examining, for one " +
        "of them shall be yours.\n");
    add_item(({"pedestal"}),
        "A short marble column in the center of the floor about a meter " +
        "high, the smooth top supports a heavy book which lies open.\n");
    add_item(({"book", "large book", "heavy book"}),
        "It rests atop the pedestal, pages of vellum bound in ancient " +
        "leather, it stands open to a single passage, perhaps worth " +
        "reading.\n");

    set_alarm(0.5, 0.0, make_the_bodies);
}

/*
 * Provide the racestat modification
 * 
 * Arguments: race - the race
 *            stat - the stat
 * Returns:   the race mod
 */
public int
queryRaceMod(string race, int stat)
{
    return RaceStatMod[race][stat];
}

/*
 * Function name: init
 * Description  : Add and remove commands as necessary.
 */
public void
init()
{
    ::init();
    add_action("command_lock", "", 1);
}

/*
 * Function name: command_lock
 * Description  : Check every command the player tries, and decides
 *                what, if anything, to do with them.
 * Arguments    : str - the string argument the player passed with
 *                      the command.
 */
public int
command_lock(string str)
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
        case "exa":
        case "examine":
        case "look":
        case "l":
        case "'":
        case "say":
        case "quit":
        case "save":
        case "sysbug":
            return 0;
            break;
        case "possess":
            possess_body(str);
            return 1;
            break;
        case "enter":
            return enter_portal(str);
            break;
        case "leave":
            return enter_portal("portal");
            break;
        case "read":
            return read_book(str);
            break;
        case "help":
            if (str && str == "here")
                return read_book("book");
            return 0;
            break;
        default:
            if (SECURITY->query_wiz_rank(tp->query_real_name()) >= WIZ_NORMAL)
                return 0;
            write("You struggle, but find that is impossible to do here.\n");
            return 1;
    }
}

/*
 * Function name: enter_portal
 * Description  : The player attempts to enter the portal, and it decides
 *                where the destimation is, if the player can pass.
 * Arguments    : str - string the player used as an argument to the 'enter'
 *                      command.
 * Returns      : 0 - Command failed, continue thread.
 *                1 - Command succeeded.
 */
public int
enter_portal(string str)
{
    object tp;
    string dest;

    tp = this_player();

    notify_fail("Enter what? Enter portal, perhaps?\n");
    if (!str || !strlen(str))
        return 0;

    if (str != "portal" && str != "the portal")
        return 0;

    if (tp->query_ghost() & GP_BODY)
    {
        write("You step towards the portal, but it resists your ghostly " +
            "form. You sense you must possess one of these bodies before " +
            "it will permit you to pass beyond.\n");
        return 1;
    }

    if (tp->query_ghost() & GP_MANGLE)
        dest = PATH + "mirrors_1";
    else if (tp->query_ghost() & GP_FEATURES)
        dest = PATH + "features";
    else if (tp->query_ghost() & GP_SKILLS)
        dest = PATH + "skills";
    else 
        dest = RACESTART[tp->query_race()];

    write("You step towards the portal, and step through the brilliant " +
        "light. You feel a surging warmth, then momentary darkness, and " +
        "you are falling...\n");
    say(QCTNAME(tp) + " steps through the portal, and is consumed in a " +
        "blinding flash of light.\n");
    tp->move_living("M", dest, 1, 0);
    say(QCTNAME(tp) + " materializes in a flash of light.\n");
    return 1;
}

/*
 * Function name: read_book
 * Description  : Allows the player to attempt to read the book, which 
 *                provides instructions on what to do here.
 * Arguments    : str - the argument used to attempt to read the book.
 * Returns      : 0 - Failed, keep threading.
 *                1 - Passed, stop.
 */
public int
read_book(string str)
{
    object tp;
    tp = this_player();

    notify_fail("Read what? Read book, perhaps?\n");
    if (!str || !strlen(str))
        return 0;

    if (str != "book" && str != "passage")
        return 0;

    tp->more(read_file(HELP + "bodies_help"));
    return 1;
}

/*
 * Function name: make_the_bodies
 * Description  : Adds bodies to the room that players can enter.
 *                Generate 2 of each gender of each race.
 */
public void
make_the_bodies()
{
    int i,
        j;
    object ob;

  /* Loop through each race */
    for (i = 0; i < sizeof(RACES); i++)
    {
      /* Loop through male and female, 0 = male, 1 = female */
        for (j = 0; j < 2; j++)
        {
            ob = clone_object(PATH + "body");    // clone a body
            ob->create_body(RACES[i], j);        // set race and gender
            ob->move(this_object());             // bring it here
        }
    }
}

/*
 * Function name: possess_body
 * Description  : Evaluates player's attempt to possess a body.
 * Arguments    : str - arguments the player sent to the 'possess' command.
 */
public void
possess_body(string str)
{
    object *bodies;
    string race, gender;
    /* Need to have mixed * to work with & */
    mixed *words;

    if (!(this_player()->query_ghost() & GP_BODY))
    {
        write("You cannot possess a body when you are not a ghost. You " +
            "may leave this room by 'enter portal' any time now.\n");
        return;
    }

    if (!strlen(str))
    {
	write("Syntax: possess <gender> <race> body\n");
	return;
    }
    words = explode(str, " ");
    if (!sizeof(words & RACES))
    {
	write("Syntax: you must specify a valid race.\n");
	return;
    }
    if (!sizeof(words & ({ "female", "male", "neuter" }) ))
    {
	write("Syntax: you must specify a valid gender.\n");
	return;
    }

    bodies = CMDPARSE_ONE_ITEM(str, "do_possess_body", "can_be_possessed");

    if (!sizeof(bodies))
    {
        write("There doesn't seen to be any possessable " + str +
            " here.\n");
        return;
    }

    if (sizeof(bodies) > 1)
    {
        write("You can only possess one body.\n");
        return;
    }

    if (finish_possess_body(bodies[0]))
    {
        write("You direct your conscience into your new body, the " +
	    bodies[0]->short() + " and take over its awareness, and step " +
	    "down from the altar it rested upon.\n");
        say("The " + capitalize(bodies[0]->query_non_name()) +
	    " stirs slightly, and steps down from the altar it rested " +
	    "upon.\n");
        set_alarm(10.0, 0.0, &replace_body(bodies[0]->query_race(),
            bodies[0]->query_gender()));
        bodies[0]->remove_object();
    }
}

/*
 * Function name: can_possess_body
 * Description  : Verifies that the object being tested is a body.
 * Arguments    : ob - object pointer, hopefully to a new body.
 * Returns      : 0 - object isn't a body.
 *                1 - object is a body.
 */
public int
can_possess_body(object ob)
{
    if (function_exists("create_body", ob) == (PATH + "body"))
        return 1;
    else
        return 0;
}

/*
 * Function name: do_possess_body
 * Description  : A dud function, simply returns one so CMDPARSE_ONE_ITEM
 *                can be used to find the right body to possess.
 * Arguments    : ob - object pointer, hopefully to a body, but it's not
 *                used anyway.
 * Returns      : 1 always.
 */
public int
do_possess_body(object ob)
{
    return 1;
}

/*
 * Function name: can_be_possessed
 * Description  : Checks to ensure that the object being tested
 *                is an actual body.
 * Arguments    : ob - object pointer to object to test.
 * Returns      : 0 - not a real body.
 *                1 - is a real body.
 */
public int
can_be_possessed(object ob)
{
    return (function_exists("create_body", ob) == (PATH + "body"));
}

/*
 * Function name: replace_body
 * Description  : Replaces bodies after someone occupies one.
 *                The new body is the same race and gender as the one
 *                being replaced.
 * Arguments    : race - The race as a string.
 *                gender - The gender as an integer, 0 = male, 1 = female.
 */
public void
replace_body(string race, int gender)
{
    object ob;

    ob = clone_object(PATH + "body");
    ob->create_body(race, gender);
    ob->move(this_object());

    tell_room(this_object(), "Suddenly the light in the room seems to " +
        "coalesce around an empty altar. The light pulses with a searing " +
        "brilliance, and you are forced to look briefly away. A moment " +
        "later you turn back, and see a new body upon the altar.\n");
}

/*
 * Function name: Finish_possess_player
 * Description  : Completes the possession of a new body. Updates player
 *                vars to coincide with those of the new body, and judges
 *                what's necessary depending on why the player is here.
 *                Possibiites are the player died and picks the same race
 *                as he had, player died and picks a new race, and a
 *                completely new player.
 * Arguments    : ob - object pointer to the new body to possess.
 * Returns      : 0 - player failed to possess ob.
 *                1 - player successfully possessed ob.
 */
public int
finish_possess_body(object ob)
{
    int *skill_types,
         il,
         val;
    object tp;

    tp = this_player();

  /* Give this_player() the selected body's appearance. */
    tp->set_appearance(ob->query_appearance());

  /* Things to do if player's here because he died. */
    if (tp->query_ghost() & GP_DEAD)
    {
      /* If the player isn't changing race or gender (unless the player */
      /* is neutral somehow), switch bodies and send them on their way. */
        if (tp->query_race() == ob->query_race() &&
            (tp->query_gender() == ob->query_gender() ||
             tp->query_gender() == G_NEUTER))
        {
            tp->set_ghost(0);
            tp->ghost_ready();
        }
      /* If the player did change race or gender */
        else
        {
          /* Give the player a warning that skills will be slashed. */
            if (!tp->query_prop("confirm_body_change"))
            {
                tp->add_prop("confirm_body_change", 1);
                write("You feel uncomfortable possessing this body, " +
                    "you realize that you would be less skilled in this " +
                    "new form. Try again if you're really sure.\n");
                return 0;
            }

          /* Player decided he wanted a new race or gender after all */
            tp->remove_prop("confirm_body_change");
            write("You feel your fears confirmed, your skills are " +
                "not what they once were.\n");

            skill_types = tp->query_all_skill_types();
            for (il = 0; il < sizeof(skill_types); il++)
            {
                if(skill_types[il] <= SS_MAX)
                {
                val = tp->query_skill(skill_types[il]);
                tp->set_skill(skill_types[il], val / 2);
                }
            }
         
         /* Set the player's new race name, gender, and set their ghost */
         /* variable as requiring mangling (height/width) and features. */
            tp->set_race_name(ob->query_race());
            tp->set_gender(ob->query_gender());
            tp->set_ghost(tp->query_ghost() &~ GP_BODY |
                GP_MANGLE | GP_FEATURES);
        }
    }

  /* Must be a new player, set up his initial stuff, and cut him loose. */
    else
    {
      /* Set race name, gender and ghost variable minus body flag */
        tp->set_race_name(ob->query_race());
        tp->set_gender(ob->query_gender());
        tp->set_ghost(tp->query_ghost() &~ GP_BODY);

      /* Set up stats. */
        for (il == SS_STR; il <= SS_DIS; il++)
            tp->set_base_stat(il, ob->query_base_stat(il));

      /* Set initial learning prefs. */
        tp->set_learn_pref(0);

      /* Convert stats to accumulated exp. */
        tp->stats_to_acc_exp();

      /* Make sure all points are fully stacked. */
        tp->set_hp(tp->query_max_hp());
        tp->set_mana(tp->query_max_mana());
        tp->set_fatigue(tp->query_max_fatigue());
    }
    return 1;
}

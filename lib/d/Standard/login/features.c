/*
 * /d/Standard/login/features.c
 *
 * The room where players get their adjectives set.
 *
 * Khail, May 14/96
 */

#pragma strict_types               // Make sure all variables and functions
// are properly defined

/* Include some useful macros and defines */
#include <macros.h>
#include <std.h>
#include "login.h"

#define ATTRIBS (PATH + "attributes/")
#define TPQRL   this_player()->query_real_name()
#define TP      this_player()
#define TO      this_object()

inherit "/std/room";

/* Global variables */
mapping attribmap;
string *attriblist;

/* Prototypes in order of appearance */
public void create_room();
public int get_help(string str);
public string exa_reflection();
public void setup_features();
public varargs mixed query_features(string str);
public int look_in_eye();
public int do_list(string str);
private int do_unselect(string str);
private int do_select(string str);
public int do_enter(string str);
public int do_go_back();
public void init();
public int all_cmd(string str);
public string get_category_by_attrib(string attrib);


public int
check_bodies_exit()
{
    TP->set_ghost(TP->query_ghost() | GP_BODY | GP_MANGLE);
    return 0;
}

public int
check_height_exit()
{
    TP->set_ghost(TP->query_ghost() | GP_MANGLE);
    return 0;
}

/*
 * Function name: extra_long
 * Description  : Generates an extra bit to the room's long if a player
 *                shouldn't be here.
 * Returns      : The string addition to the long.
 */
public string
extra_long()
{
    if (!(TP->query_ghost() & GP_FEATURES))
	return "You seem to realize that you've absolutely no business " +
	"here, and should 'enter pool' immediately.\n\n";
    else
	return "";
}

/*
 * Function name: create_room
 * Description  : Sets up the features room, initializes the attribmap
 *                and attriblist variables.
 */
public void
create_room()
{
    set_short("eye of souls");
    set_long("You're in dark and strange hall, its walls and ceiling " +
      "hidden deep in the shadows, or maybe it just goes on forever, " +
      "you can't tell. In the middle of the floor stands a wide, " +
      "circular pool of a silvery, waterlike substance. It's highly " +
      "reflective, who knows what you'd see if you 'look in'.\n\n" +
      "If you're stuck, just do 'help here'.\n\n" +
      "@@extra_long");

    add_item("reflection", "@@exa_reflection");

    set_alarm(0.5, 0.0, setup_features);

    add_exit(PATH + "bodies", "bodies", 
      "@@check_bodies_exit");
    add_exit(PATH + "mirrors_1", "height",
      "@@check_height_exit");
}

/*
 * Function name: get_help
 * Description  : Delivers a detailed help message for people who can't
 *                figure it out from the stuff in the room itself.
 */
public int
get_help(string str)
{
    if (!str || !strlen(str) || str != "here")
	return 0;

    setuid();
    seteuid(getuid());

    /* Use the board more to read the idiot-proof help text */
    TP->more(read_file(HELP + "feature_help"));
    return 1;
}

/*
 * Function name: exa_reflection
 * Description  : Generates a message when players examine reflection.
 *                That message contains their current long, plus tips
 *                on what to do next depending on their current status.
 * Returns      : The above-described message as a string.
 */
public string
exa_reflection()
{
    string ret;

    ret = "You gaze deeply into the pool, ignoring the voices and " +
    "concentrating on your reflection. Soon it sharpens into " +
    "clear view...\n";
    ret += TP->long();

    switch(sizeof(TP->query_adjs()))
    {
    case 0:
	ret += "You seem to be rather plain, you sense that you can " +
	"and should change that now.\n";
	break;
    case 1:
	ret += "You seem to be coming along nicely, but you feel that " +
	"you are still not quite perfect.\n";
	break;
    case 2:
	if (get_category_by_attrib(TP->query_adjs()[0]) ==
	  get_category_by_attrib(TP->query_adjs()[1]))
	    ret += "You seem to have two adjectives from the same " +
	    "category. Please change one.\n";
	else
	    ret += "You seem to be complete. If you are happy with " +
	    "the way you look, it is time to leave by 'enter " +
	    "pool'.\n";
	break;
    default:
	ret += "It seems something is wrong with the pool, better " +
	"use 'bug <message about what's wrong>' to report it, " +
	"or try to 'commune all <message about what's wrong' " +
	"if you're really in trouble.\n";
	break;
    }

    return ret;
}

/*
 * Function name: get_category_by_attrib
 * Description  : Returns the name of the category the supplied attribute
 *                matches in (if any).
 * Arguments    : attrib - The attribute to match as a string.
 * Returns      : The category as a string. 
 *                "" if no category is matched.
 */
public string
get_category_by_attrib(string attrib)
{
    int i;
    string *temp;

    for (i = 0; i < m_sizeof(attribmap); i++)
    {
	temp = attribmap[m_indexes(attribmap)[i]];
	if (member_array(attrib, temp) >= 0)
	    break;
    }

    if (i < m_sizeof(attribmap))
	return m_indexes(attribmap)[i];
    else
	return "";
}

/*
 * Function name: setup_features
 * Description  : Generate the attribute mapping based on the files in
 *                /d/Standard/login/attributes/*
 *                Note that with this setup, all that is required to add
 *                new attributes is to just add them to the right category
 *                file (all plain text files, one attribute per line), or
 *                even create a new category. This function will generate
 *                the mapping from whatever's in that directory. After a
 *                change, you can wait for reboot and it will update the
 *                next time it loads, or this function can be called
 *                manually.
 */
public void
setup_features()
{
    mixed *categories;
    int i;

    attribmap = ([]);
    attriblist = ({});

    /* Get an array with all the attribute files */
    categories = get_dir(ATTRIBS);

    /* Go through the attribute files one at a time and add them to the */
    /* attribmap mapping, and attriblist array */
    for (i = 0; i < sizeof(categories); i++)
    {
	attribmap[categories[i]] = explode(read_file(ATTRIBS + categories[i]),"\n");
	attriblist += explode(read_file(ATTRIBS + categories[i]), "\n");
    }
}


/*
 * Function name: query_features
 * Description  : Returns a variety of information regarding the attribmap
 *                variable, depending on what, if anything, was supplied
 *                as an argument.
 * Arguments    : str - An optional string. If left blank, function returns
 *                the whole attribmap. If 'categories' specified, returns
 *                an array of all categories. If a specific category is
 *                supplied, return an array of features in that category.
 *                If 'features' is the argument, the it generates an array
 *                of all features in all categories in one array.
 * Returns      : As described in the 'Arguments' above.
 */
public varargs mixed
query_features(string str)
{
    string *arr;
    int i;

    /* Ensure features are fully loaded and enabled. */
    if (!attribmap)
	setup_features();

    /* If no argument, return the full mapping of categories and features */
    if (!str || !strlen(str))
	return attribmap + ([]);

    /* If 'categories' specified, return an array of available categories */
    if (str == "categories")
	return m_indexes(attribmap);

    /* If 'features' specified, return an array of all features. */
    if (str == "features")
    {
	arr = ({});
	for (i = 0; i < sizeof(m_values(attribmap)); i++)
	    arr += m_values(attribmap)[i];
	return arr;
    }

    /* If the specified argument isn't 'categories' or a valid category */
    if (!attribmap[str])
	write("\nNo such category.\n\n");
    return;

    /* Return the appropriate array of features in the specified category */
    return attribmap[str];
}

/*
 * Function name: look_in_eye
 * Description  : Writes the information on what to do in this room, i.e.
 *                the list of available commands.
 * Returns      : 1
 */
public int
look_in_eye()
{
    write("\nYou stare into the pool, but your reflection is strange, " +
      "a complex pattern of ever-shifting shapes and images. A " +
      "strange voice whispers:\n" +
      "\tI am the eye of souls.\n" +
      "\tYour body is new, and your soul featureless. I can help\n" +
      "\tyou correct that with these abilities.\n" +
      "\tlist - list all available categories and features.\n" +
      "\tlist categories - list only categories.\n" +
      "\tlist <category> - list features in the selected category.\n" +
      "\tselect first <feature> - set your first feature.\n" +
      "\tselect second <feature> - set your second feature.\n" +
      "\tunselect <feature> - unselect an feature you have set.\n" +
      "\tunselect all - unselect all of your current features.\n" +
        "\tYou may also 'examine reflection', to see how you look at the\n "+
      "\tmoment. You may have only two features, but are free to\n " +
      "\tchange as much as you want until you make your final choice.\n" +
      "\tWhen you are done, try to 'enter pool' to leave.\n" +
      "\tFor example 'list', 'list eyes', 'select first green-eyed'.\n\n");
    return 1;
}

/*
 * Function name: do_list
 * Description  : The command which players use to list information on the
 *                attributes available. Results vary depending upon what
 *                was specified as an argument (note this is very similar
 *                to query_features(), but that is for debugging purposes,
 *                and the output of that function is not formatted for
 *                mortals). If no argument, result is a list of all
 *                categories and features. If 'categories' is the argument,
 *                writes a list of all categories. If one of the available
 *                categories is the argument, then a list of the features
 *                in that category is listed.
 * Arguments    : str - An optional string as set by the argument to
 *                the 'list' command. Can be blank, 'categories', or any
 *                one of the existing categories.
 * Returns      : 1 
 */ 
public int
do_list(string str)
{
    int i;
    string ret,
    temp;

    ret = "";

    /* If no argument, return list of all categories (indexes) and their */
    /* corresponding features (values) */
    if (!str || !strlen(str))
    {
	for (i = 0; i < m_sizeof(attribmap); i++)
	{
	    temp = m_indexes(attribmap)[i];
	    ret += "CATEGORY: " + temp + "\n";
	    ret += implode(attribmap[temp], ", ");
	    ret += "\n";
	}
	TP->more(ret);
	return 1;
    }

    /* If argument is 'categories' then write a list of all the categories */
    /* (indexes) available. */
    if (str == "categories")
    {
	write("\n" + implode(m_indexes(attribmap), ", ") + "\n\n");
	return 1;
    }

    /* Unrecognized argument */
    if (!attribmap[str])
    {
	write("\nNo such category. Use 'list categories' to see the ones " +
	  "that are available.\n\n");
	return 1;
    }

    /* Write a list of features in specified category (all other */
    /* possibilities _should_ be exhausted by now */
    ret += "Features in category " + str + ":\n";
    ret += implode(attribmap[str], ", ");
    ret += "\n";

    write("\n" + ret + "\n");

    return 1;
}

/*
 * Function name: do_unselect
 * Description  : Used by players to remove attributes if they've changed
 *                their minds.
 * Arguments    : str - Attribute to be removed or 'all' to remove all of
 *                      them.
 */
private int
do_unselect(string str)
{
    mixed temp;
    int i;

    notify_fail("Unselect <which> or 'all' attributes?\n");
    if (!str || !strlen(str))
	return 0;

    /* Can't unselect if the player isn't supposed to be getting */
    /* features. */
    if (!(TP->query_ghost() & GP_FEATURES))
    {
	write("\nYou have no business here, you'd better 'enter portal' " +
	  "immediately.\n");
	return 1;
    }

    /* Can't unselect if no adjectives exist yet. */
    if (!pointerp(temp = TP->query_adjs()))
    {
	notify_fail("You've no attributes to unselect.\n");
	return 0;
    }

    /* If the player unselected something other than 'all', make */
    /* sure it's an adjective that currently exists in the player. */
    if (str != "all" && member_array(str, temp) < 0)
    {
	notify_fail(capitalize(str) + " is not an attribute you possess " +
	  "to unselect.\n");
	return 0;
    }

    /* If player unselected all, loop through all adjectives and */
    /* clear them out. */
    if (str == "all")
    {
	for (i = 0; i < sizeof(temp); i++)
	{
	    TP->remove_adj(temp[i]);
	    write("\nRemoved " + temp[i] + ".\n\n");
	}
	return 1;
    }

    /* If player has at least 2 adjectives, and is unselecting */
    /* the first one. Remove it, and set their first adjective */
    /* to "" to prevent their second adjective from moving */
    /* to the first position. */
    if (sizeof(temp) >= 2 && temp[0] == str)
    {
	TP->remove_adj(str);
	TP->set_adj(({""}));
    }

    /* Otherwise, just remove the adjective. */
    else
	TP->remove_adj(str);
    write("\nRemoved " + str + ".\n\n");
    return 1;
}

/*
 * Function name: do_select
 * Description  : Sets/changes the attributes of a player. If player
 *                specifies an adjective that they already have, two things
 *                can happen: 1 - if the player specifies an adjective in a
 *                slot where it already exists, ignore it. 2 - if the
 *                player specifies an adjective that already exists in the
 *                other slot, move it (to prevent 2 identical adjectives).
 * Arguments    : str - A string of the form 'first/second <adjective>',
 *                for example 'first blue-eyed'.
 * Returns      : 0 - Fail, keep threading.
 *                1 - Success or fail, stop threading.
 */
private int
do_select(string str)
{
    int i;
    string a,
    b,
    cat1,
    cat2,
    temp,
    *temparr;

    /* First check that they have are supposed to be here */
    if (!(TP->query_ghost() & GP_FEATURES))
    {
	notify_fail("You have no business being here, please leave " +
	  "by entering the pool, now.\n");
	return 0;
    }

    notify_fail("Syntax: 'select first <adjective>' or 'select second " +
      "<adjective>'.\n");

    /* Make sure player specified an argument of some kind */
    if (!str || !strlen(str))
	return 0;

    /* Make sure player's argument was 'first/second <adjective>' */
    if (sscanf(str, "%s %s", a, b) != 2 || (a != "first" && a != "second"))
	return 0;

    /* Make sure specified adjective exists in attriblist */
    if (member_array(b, attriblist) < 0)
    {
	notify_fail(capitalize(b) + " is not a valid feature. Please " +
	  "select another.\n");
	return 0;
    }

    /* By now we've established that the player selected either first or */
    /* second adjective, and has specified an valid adjective */

    /* Prevent players from using two adjectives from the same category. */
    cat1 = get_category_by_attrib(b);
    if (pointerp(TP->query_adjs()))
	temparr = TP->query_adjs();
    else
	temparr = ({});

    for (i = 0; i < sizeof(temparr); i++)
    {
	cat2 = get_category_by_attrib(temparr[i]);
	if ((cat1 == cat2) &&
	  (cat1 != "general"))
	{
	    write("\n" + capitalize(b) + " and an existing attribute, " +
	      temparr[i] + ", are both of the category " +
	      cat1 + ". Please choose a new attribute from another " +
	      "category, or 'unselect " + temparr[i] +
	      "' and then try to select " + b + " again.\n\n");
	    return 1;
	}
    }

    if (!sizeof(temparr) && a == "second")
    {
	notify_fail("You cannot set a second attribute when you do not " +
	  "as yet have a first.\n");
	return 0;
    }

    /* Now we've established that the specified slot and adjective are */
    /* and the adjective is new */

    for (i = 0; i < sizeof(temparr); i++)
	TP->remove_adj(temparr[i]);

    if (a == "first")
    {
	if (!sizeof(temparr))
	    temparr = ({b});
	else
	    temparr[0] = b;
	TP->set_adj(temparr);
    }
    else if (a == "second")
    {
	if (sizeof(temparr) == 1)
	    temparr += ({b});
	else
	    temparr[1] = b;
	TP->set_adj(temparr);
    }

    write("\nSet your " + a + " feature as " + b + ".\n\n");
    return 1;
}

/*
 * Function name: do_enter
 * Description  : Allows the player to attempt to leave by entering the
 *                pool. Only players with properly set adjectives can get
 *                out this way.
 * Arguments    : str - argument player gave with the 'enter' command.
 * Returns      : 0 - failure
 *                1 - success.
 */
public int
do_enter(string str)
{
    string temp;
    int i;

    notify_fail("Enter what? The pool?\n");

    if (!str || !strlen(str))
	return 0;

    if (member_array(str, ({"pool","the pool","eye","the eye"})) < 0)
	return 0;

    if (sizeof(TP->query_adjs()) < 2)
    {
	write("\nYou seem to still require some features, 'look in' the " +
	  "pool, that the souls which dwell within may help.\n\n");
	return 1;
    }

    if (sizeof(TP->query_adjs()) > 2)
    {
	write("\nYou seem to have selected too many features somehow, " +
	  "please re-select them all.\n\n");
	TP->remove_adj(TP->query_adjs());
	return 1;
    }

    for (i = 0; i < 2; i++)
    {
	temp = TP->query_adjs()[i];

	if (!temp || !strlen(temp))
	{
	    write("\nYour " + ({"first", "second"})[i] + " attribute seems " +
	      "to be nothing, the eye of souls will not accept you. Please " +
	      "choose your first attribute and try again. 'look in' the " +
	      "pool for help.\n\n");
	    return 1;
	}
    }

    if (get_category_by_attrib(TP->query_adjs()[0]) ==
      get_category_by_attrib(TP->query_adjs()[1]))
    {
	write("\nYou have selected two attributes from the same category. " +
	  "please change one of them.\n\n");
	return 1;
    }

    write("\nYou hesitantly step down into the eye of souls, and feel " +
      "the clear waters of the pool close over your head. You feel " +
      "yourself being drawn to another place...\n\n");
    say(QCTNAME(TP) + " hesitantly steps down into the eye " +
      "of souls, and vanishes beneath it's surface!\n");
    TP->set_ghost(TP->query_ghost() &~ GP_FEATURES);
    if (!(TP->query_ghost() & GP_SKILLS))
    {
	TP->set_ghost(0);
	TP->ghost_ready();
    }
    else
    {
	TP->move_living("X", PATH + "skills");
	say(QCTNAME(TP) + " steps out of the shadows.\n");
    }
    return 1;
}

/*
 * Function name: do_go_back
 * Description  : Moves the player back to the halls of mirrors if they
 *                want their height and width changed for some reason
 *                before they finally enter the game
 * Returns      : 0 - failure, they didn't come from the halls of mirrors.
 *                1 - success, they were just there, so give them a chance
 *                    to go back.
 */
public int
do_go_back()
{
    /* Make sure player came from the last mirrors room (mirror_2, sets */
    /* their width) */
    if (TP->query_prop("_live_i_last_room") == PATH + "mirror_2")
    {
	write("\nYou suddenly realize you forgot something, and are whisked " +
	  "away by the powers that be to remedy the situation.\n\n");

	/* Record that they're getting width and height again */
	TP->set_ghost(TP->query_ghost() | GP_FEATURES);

	/* Trans them back to the first mirror room, not the second */
	TP->move_living("X", PATH + "mirror_1");
	return 1;
    }
    else
    {
	notify_fail("You can't do that, you don't seem to have come from " +
	  "any place you can be returned.\n");
	return 0;
    }
}

/*
 * Function name: init
 * Description  : Use add_action to catch all commands player attempts
 *                here.
 */
public void
init()
{
    ::init();
    add_action("all_cmd", "", 1);
}

/*
 * Function name: all_cmd
 * Description  : All command attempts are routed to this function. Here
 *                is where we determine what a player can do here, and how
 *                to do it if it's a local command.
 * Arguments    : str - Whatever string the player specified along with the
 *                last command.
 * Returns      : 1 - Hit, stop threading.
 *                0 - Command not blocked, keep threading.
 */
public int
all_cmd(string str)
{
    switch(query_verb())
    {
    case "arrive":
    case "glance":
    case "say":
    case "quit":
    case "Goto":
    case "exa":
    case "examine":
    case "home":
    case "save":
    case "typo":
    case "bug":
    case "praise":
    case "idea":
	return 0;
	break;
    case "help":
	return get_help(str);
	break;
    case "look":
    case "l":
	if (str == "in")
	    return look_in_eye();
	else
	    return 0;
	break;
    case "list":
	return do_list(str);
	break;
    case "select":
	return do_select(str);
	break;
    case "unselect":
	return do_unselect(str);
	break;
    case "enter":
	return do_enter(str);
	break;
    case "back":
    case "return":
	return do_go_back();
	break;
    default:
	if (SECURITY->query_wiz_rank(TPQRL) >= WIZ_NORMAL)
	    return 0;
	write("\nThat is not possible here.\n\n");
	return 1;
    }
}

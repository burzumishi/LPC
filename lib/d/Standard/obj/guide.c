/*
 * /d/Standard/obj/guide.c
 *
 * This player guide is cloned into the inventory of every player when the
 * character is created. It is also available from all adventurers guilds
 * in the game.
 *
 * Specifically, this guide is about "how to play the game" and "where to
 * get help". The intention is not to copy the whole series of help pages
 * into this guide.
 *
 * The template in /d/Standard/obj/guide/template shows the exact format of
 * the pages that can be read in the guide.
 *
 * Author: Mercade
 * Date  : February 20 1998
 */

#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <macros.h>
#include <stdproperties.h>

/*
 * Prototypes.
 */
int read(string str);
int obj_m_no_drop();

/*
 * This definition contains a list of topics to be read. Please add new in
 * alphabetical order and in lower case.
 */
#define TOPICS ({ "conduct", "exploring", "fighting", "guide", "guilds", \
    "help", "index", "money", "questing", "roleplay" })
#define GUIDE_DIR "/d/Standard/obj/guide/"
#define AGE_LIMIT 43200 /* One day in heartbeats */

/*
 * Function name: create_object
 * Description  : Constructor - called to create this object.
 */
void
create_object()
{
    set_short("small paper guide");
    set_pshort("small paper guides");
    set_name("guide");
    set_pname("guides");
    set_adj( ({ "small", "paper", "player" }) );

    set_long("The guide is nothing more than a collection of pages, bound " +
        "together with a thin thread. The cover page contains only three " +
        "words. In red it reads \"IMPORTANT\" and underneath, in black and " +
        "more elegant letters \"PLAYER GUIDE\".\n" +
        "Opening the guide, the index page makes clear read that this is " +
        "player guide. It will give some information on how to play this " +
        "game. Separate pages deal with different issues.\nThe following " +
        "topics can be read with the command \"read guide on <topic>\":\n\n" +
         sprintf("%-78#s\n", implode(TOPICS, "\n")) + "\n");

    add_item( ({ "paper", "page", "pages", "collection of pages" }),
        "Several sheets of paper function as pages of this guide. They are " +
        "made of a cheap sort of wood pulp, but very readable nontheless. " +
        "To read the pages, use the command to \"read guide on <topic>\".\n");
    add_item( ({ "thin thread", "thread" }),
        "This thin thread has a most vital function. It keeps the pages " +
        "of the small paper guide together, and thus makes it a guide, " +
        "rather than a bunch of pages, soon to be lost.\n");
    add_item( ({ "letters", "eloquent letters" }),
        "The base of all written communication, letters. Some are more " +
        "elegantly penned down than others. Letters come in all sizes and " +
        "different shapes, which makes it easier to distinguish between " +
        "them. Some look like \"a\", others like \"b\". Amazing!\n");
    add_item( ({ "index page" }), &read("guide on index"));

    add_prop(OBJ_M_NO_DROP, obj_m_no_drop);
    add_prop(OBJ_M_NO_BUY, 1);
    add_prop(OBJ_M_NO_SELL, 1);
    add_prop(OBJ_M_NO_STEAL, 1);
    add_prop(OBJ_M_NO_TELEPORT, 1);

    remove_prop(OBJ_I_VALUE);
    add_prop(OBJ_I_WEIGHT, 150);
    add_prop(OBJ_I_VOLUME,  50);

    setuid();
    seteuid(getuid());
}

/*
 * Function name: obj_m_no_drop
 * Description  : Called when the player tries to drop the guide.
 */
int
obj_m_no_drop()
{
    if (this_player()->query_age() < AGE_LIMIT)
    {
        return "You are not old enough get rid of the " + short() + ".\n";
    }

    return 0;
}

/*
 * Function name: read
 * Description  : Holds the code for the read command.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
int
read(string str)
{
    if (!strlen(str))
    {
        notify_fail("Read what?\n");
        return 0;
    }

    if (parse_command(str, ({ }), "[the] [small] [paper] [player] 'guide'"))
    {
        write(query_long());
        return 1;
    }

    if (!parse_command(str, ({ }),
        "[the] [small] [paper] [player] 'guide' [on] %w", str))
    {
        /* There is no notify_fail() here because I do not want the
         * trivial "Read what?" to replace a more eloquent fail message
         * from other objects.
         */
        return 0;
    }

    str = lower_case(str);
    if (member_array(str, TOPICS) == -1)
    {
        notify_fail("The " + short() + " has no page on the topic \"" + str +
            "\". Read the index page or examine the guide for a list of " +
            "topics.\n");
        return 0;
    }

    /* All pages should preferably be at most 20 lines, but let's use more
     * functionality anyway for those who have set it to something lower, or
     * for the odd page that actually is longer.
     */
    this_player()->more(read_file(GUIDE_DIR + str));
    return 1;
}

/*
 * Function name: init
 * Description  : Adds the command(s) related to this guide to the player.
 */
void
init()
{
    ::init();

    add_action(read, "read");
}

/*
 * Function name: destroy_guide
 * Description  : Destroy the guide when it is dropped.
 */
void
destroy_guide()
{
    if (environment()->query_prop(ROOM_I_IS))
    {
        tell_room(environment(), "The " + short() +
            " falls apart and slowly disintegrates.\n");
        remove_object();
    }
}

/*
 * Function name: enter_env
 * Description  : Destroy the guide when it is dropped.
 * Arguments    : object to - the object we are entering.
 *                object from - the object we came from.
 */
void
enter_env(object to, object from)
{
    ::enter_env(to, from);

    if (objectp(to) &&
        to->query_prop(ROOM_I_IS))
    {
        set_alarm(30.0, 0.0, destroy_guide);
    }
}

/*
 * Function name: query_auto_load
 * Description  : Returns the path of this file, so that it auto-loads.
 * Returns      : string - the path.
 */
string
query_auto_load()
{
    return MASTER;
}

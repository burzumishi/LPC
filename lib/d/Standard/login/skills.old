#pragma strict_types

inherit "/lib/skill_raise";
inherit "/std/room";

#include <ss_types.h>
#include <stdproperties.h>
#include <macros.h>
#include <language.h>
#include <money.h>
#include "login.h"

#define TP this_player()

void set_up_skills();

/* Function name: check_bodies_exit
 * Description  : Resets necessary ghost vars if players return
 *                to the bodies room.
 * Arguments    : n/a
 * Returns      : 0
 */
public int
check_bodies_exit()
{
    TP->set_ghost(GP_BODY | GP_MANGLE | GP_FEATURES);
    return 0;
}

/*
 * FUnction name: check_height_exit
 * Description  : Resets necessary ghost vars if players return
 *                to the height mirrors.
 * Arguments    : n/a
 * Returns      : 0
 */
public int
check_height_exit()
{
    TP->set_ghost(GP_MANGLE | GP_FEATURES);
    return 0;
}

/*
 * Function name: check_features_exit
 * Description  : Resets necessary ghost vars if players return
 *                to the festures room.
 * Arguments    : n/a
 * Returns      : 0
 */
public int
check_features_exit()
{
    TP->set_ghost(GP_FEATURES);
    return 0;
}

/*
 * Function name: check_south_exit
 * Description  : Allows players to enter the game once they've
 *                cleaned out their ghost vars.
 * Arguments    : n/a
 * Returns      : 1
 */
public int
check_south_exit()
{
  /* If the player is a ghost, and his ghost_var is anything */
  /* other than GP_SKILLS, don't let them leave. */
    if (TP->query_ghost() && 
        (TP->query_ghost() - GP_SKILLS))
    {
        write("\nSome force prevents you from leaving, " +
            "as if there is something you have not yet done.\n\n");
    }
    else
    {
        TP->set_ghost(0);
        TP->ghost_ready();
    }
    return 1;
}

/*
 * Function name: create_room
 * Description  : Sets up this object as a room.
 * Arguments    : n/a
 * Description  : n/a
 */
void
create_room()
{
    set_short("Hall of Wisdom and Knowledge");
    set_long("You find yourself in a large hall, quite mundane compared " +
        "to the ones you've seen so far since choosing your new body. " +
        "You come to realize that contained herein is the elementary " +
        "knowledge of all who have come before you, and some of that " +
        "knowledge can become yours, should you choose to spend the " +
        "funds you've been given on that here. You may leave at any " +
        "time, however, and save those funds to improve in the adventurer " +
        "guilds scattered across the lands. Or, you can spend them all " +
        "here, the choice now lies with you. A large door stands open " +
        "to the south, which you can leave by when you're ready. Know, " +
        "however, that door is the end of your journey through creation. " +
        "Once you pass through it, you can only return to these rooms " +
        "should you die in your travels.\n\n" +
        "If you wish to spend your funds here, you may <learn> and " +
        "then <improve> them. Defense, knife and sword skills are " +
        "quite common skills to learn first.\n\n" +
        "As always, you may 'help here' for more detail.\n\n");

   add_prop(ROOM_I_INSIDE, 1);
   add_prop(ROOM_I_TYPE, ROOM_NORMAL);
   
   create_skill_raise();
   set_up_skills();

    add_exit(PATH + "bodies", "bodies", "@@check_bodies_exit");
    add_exit(PATH + "mirrors_1", "height", "@@check_height_exit");
    add_exit(PATH + "features", "features", "@@check_features_exit");
    add_exit(PATH + "skills", "south", "@@check_south_exit");
}

/*
 * Function name: give_me_cash
 * Description  : Gives the player their cash for training
 *                initial skills, and removes the GP_SKILLS
 *                flag from their body.
 * Arguments    : who - Object point to the player to give
 *                      the cash to.
 * Returns      : n/a
 */
public void
give_me_cash(object who)
{
    if (who->query_ghost() & GP_SKILLS &&
        !present("silver coin", who) &&
        !m_sizeof(who->query_all_skill_types()))
    {
        tell_object(who, "Suddenly a voice whispers in your ear: May " +
            "these coins aid you in your journeys.\n");
        MONEY_MAKE(16 + random(5), "silver")->move(who, 1);
    }
}

/*
 * Function name: do_help
 * Description  : Provides the help for this room.
 * Arguments    : here - Argument player provided to 'help' command.
 * Returns      : 0 - Didn't 'help here', do nothing.
 *                1 - Did 'help here'.
 */
public int
do_help(string str)
{
    if (!str || !strlen(str) || str != "here")
        return 0;

    TP->more(read_file(HELP + "skills_help"));
    return 1;
}

/*
 * Function name: all_cmd
 * Description  : A command filter to lock out all but a few
 *                commands.
 * Argument     : str - Any possible arguments to any given
 *                      command.
 * Returns      : 0 - Command's legal, let it through.
 *                1 - Illegal command, do nothing.
 */
public int
all_cmd(string str)
{
    switch(query_verb())
    {
        case "exa":
        case "examine":
        case "look":
        case "l":
        case "improve":
        case "learn":
        case "bodies":
        case "features":
        case "height":
        case "south":
        case "s":
        case "commune":
        case "quit":
        case "bug":
        case "idea":
        case "typo":
        case "praise":
            return 0;
            break;
        case "help":
            return do_help(str);
            break;
        default:
            return 0;
    }
}

/*
 * Function name: init
 * Description  : initializes some commands for players.
 * Arguments    : n/a
 * Returns      : n/a
 */
void
init()
{
   ::init();
   add_action(all_cmd, "", 1);
   init_skill_raise();
   set_alarm(1.0, 0.0, &give_me_cash(TP));
}

/*
 * Function name: set_up_skills
 * Description  : Sets up the skill limits, names, etc. for the
 *                skills available in this room.
 * Arguments    : n/a
 * Returns      : n/a
 */
public void
set_up_skills()
{
    string me, ot;
   
    me = "use an axe"; ot = me;
    sk_add_train(SS_WEP_AXE,    ({ me, ot }), 0, 0, 30 );
    me = "defend yourself"; ot = me;
    sk_add_train(SS_DEFENCE, ({ me, ot }), 0, 0, 27);
    me = "parry attacks"; ot = me;
    sk_add_train(SS_PARRY, ({ me, ot }), 0, 0, 33);
    me = "use a knife"; ot = me;
    sk_add_train(SS_WEP_KNIFE,  ({ me, ot }), 0, 0, 31 );
    me = "use a club"; ot = me;
    sk_add_train(SS_WEP_CLUB, ({ me, ot }), 0, 0, 32);
    me = "use a sword"; ot = me;
    sk_add_train(SS_WEP_SWORD, ({ me, ot }), 0, 0, 32);
    me = "use a polearm"; ot = me;
    sk_add_train(SS_WEP_POLEARM, ({ me, ot }), 0, 0, 29);
    me = "become more aware of the surroundings"; ot = me;
    sk_add_train(SS_AWARENESS, ({ me, ot }), 0, 0, 48);
    me = "perform magic and spells"; ot = me;
    sk_add_train(SS_SPELLCRAFT, ({ me, ot }), 0, 0, 30);
    me = "make better trades and deals"; ot = me;
    sk_add_train(SS_TRADING, ({ me, ot }), 0, 0, 50);
    me = "handle animals"; ot = me;
    sk_add_train(SS_ANI_HANDL, ({ me, ot }), 0, 0, 50);
    me = "understand various languages"; ot = me;
    sk_add_train(SS_LANGUAGE, ({ me, ot }), 0, 0, 50);
    me = "climbing"; ot = me;
    sk_add_train(SS_CLIMB, ({ me, ot }), 0, 0, 54);
    me = "appraising different objects"; ot = me;
    sk_add_train(SS_APPR_OBJ, ({ me, ot }), 0, 0, 51);
    me = "appraising monsters"; ot = me;
    sk_add_train(SS_APPR_MON, ({ me, ot }), 0, 0, 53);
    me = "appraising values"; ot = me;
    sk_add_train(SS_APPR_VAL, ({ me, ot }), 0, 0, 50);
    me = "finding tracks"; ot = me;
    sk_add_train(SS_TRACKING, ({ me, ot }), 0, 0, 50);
    me = "hunt wild game"; ot = me;
    sk_add_train(SS_HUNTING,    ({ me, ot }), 0, 0, 49 );
    me = "swim"; ot = me;
    sk_add_train(SS_SWIM, ({me, ot}), 0, 0, 50); 
}

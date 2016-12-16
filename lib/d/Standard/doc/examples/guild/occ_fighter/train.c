/* A simple guild trainer */

#pragma strict_types

/* Base file for skill trainers */
inherit "/lib/skill_raise";
inherit "/std/room";

#include "guild.h"

#include <ss_types.h>

/*
 * Function name: set_up_skills
 * Description:   Initialize the trainer and set the skills we train
 */
void
set_up_skills()
{
    create_skill_raise();
  
    sk_add_train(SS_TRADING, "make deals", "trading", 0, 50);
    sk_add_train(SS_AWARENESS, "perceive", "awareness", 0, 60);
    sk_add_train(SS_CLIMB, "climb", "climb", 0, 40);
    sk_add_train(SS_LOC_SENSE, "avoid getting lost", "location sense", 0, 60);
    sk_add_train(SS_DEFENSE, "dodge attacks", "defense", 0, 90);
    sk_add_train(SS_BLIND_COMBAT, "fight while blind",
	"blindfighting", 0, 50);
    sk_add_train(SS_APPR_MON, "appraise enemies", "appraise enemy", 0, 70);
    sk_add_train(SS_UNARM_COMBAT, "fight unarmed", "unarmed combat",
	0, 70);
    sk_add_train(SS_WEP_SWORD, "fight with a sword", "sword", 0, 90);
    sk_add_train(SS_GUILD_SPECIAL_SKILL, "do a special skill", "special skill",
	80, 100);
}

void
create_room()
{
    set_short("Guild Trainer");
    set_long("This is the " + GUILD_NAME + " training hall.\n");

    add_exit("start", "north");

    /* configure the trainer */
    set_up_skills();
}

void
init()
{
    ::init();

    /* add the trainer's commands */
    init_skill_raise();
}

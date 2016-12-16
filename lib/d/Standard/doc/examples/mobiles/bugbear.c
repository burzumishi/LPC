/*
 * This is an example of a monster with leftovers. Please read the file
 * /doc/man/objects/leftovers for further explanations.
 */

inherit "/std/monster.c";
#include <stdproperties.h>
#include <ss_types.h>
#include <macros.h>

#define BS(message)	break_string(message, 75)
#define TP 		this_player()
#define TO 		this_object()

void
create_monster()
{
    if (!IS_CLONE)
	return;

    set_name("ellen");
    set_race_name("bugbear");
    set_adj("fanged");
    set_gender(G_NEUTER);
    set_aggressive(1);

    set_long(BS("This is probably the most unpleasant looking dinner-partner you can imagine. Especially so since it seems to consider YOU as dinner.\n"));

    set_stats(({ 80, 80, 80, 5, 5, 20 }));
    set_hp(10000); /* Heal fully */

    set_skill(SS_DEFENCE, 100);
    set_skill(SS_UNARM_COMBAT, 80);
    set_skill(SS_BLIND_COMBAT, 80);
    set_skill(SS_PARRY, 80);
    
    add_prop(CONT_I_WEIGHT, 85000);
    add_prop(CONT_I_HEIGHT, 1830);
    add_prop(CONT_I_VOLUME, 84000);

    /* Leftover 1 */
    add_leftover("/std/leftover", "nose", 1, 0, 0, 0);
    /* Leftover 2 */
    add_leftover("/std/leftover", "ear", 2, "@@check_ear:" +  
		 MASTER + "@@", 0, 0);
    /* Leftover 3 */
    add_leftover("/doc/examples/mobiles/bb_heart.c", "heart", 1, 0, 0, 1);
    /* Leftover 4 */
    add_leftover("/std/leftover", "skull", 1, 0, 1, 1);
}

string
check_ear()
{
    if (this_player()->query_alignment() > 40)
	return "You are far too nice to do that";

    return "";
}

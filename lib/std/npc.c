/*
  /std/npc.c

  This is the base for all NPC creatures that need to use combat tools.

*/
#pragma save_binary
#pragma strict_types

inherit "/std/creature";

#include <files.h>
#include <money.h>
#include <stdproperties.h>

void
create_npc()
{
    ::create_creature();
}

nomask void
create_creature() 
{
    if (!random(5))
	add_leftover(LEFTOVER_OBJECT, "tooth", random(5) + 1, 0, 1, 0, 0);
    if (!random(5))
	add_leftover(LEFTOVER_OBJECT, "skull", 1, 0, 1, 1, 10);
    if (!random(5))
	add_leftover(LEFTOVER_OBJECT, "thighbone", 2, 0, 1, 1, 10);
    if (!random(5))
        add_leftover(LEFTOVER_OBJECT, "kneecap", 2, 0, 0, 1, 2);
    if (!random(5))
	add_leftover(LEFTOVER_OBJECT, "jaw", 1, 0, 1, 1, 2);
    if (!random(5))
	add_leftover(LEFTOVER_OBJECT, "rib", 2, 0, 1, 1, 4);

    if (query_prop(LIVE_I_UNDEAD))
    {
	create_npc();
        MONEY_CONDENSE(this_object());
	return;
    }

    if (!random(6))
        add_leftover(LEFTOVER_OBJECT, "ear", 2, 0, 0, 0, 1);
    if (!random(6))
        add_leftover(LEFTOVER_OBJECT, "scalp", 1, 0, 0, 1, 3);
    if (!random(6))
        add_leftover(LEFTOVER_OBJECT, "finger", 2, 0, 0, 1, 2);
    if (!random(6))
        add_leftover(LEFTOVER_OBJECT, "toe", 2, 0, 0, 1, 2);
    if (!random(6))
        add_leftover(LEFTOVER_OBJECT, "nail", random(5) + 1, 0, 0, 0);
    if (!random(6))
	add_leftover(LEFTOVER_OBJECT, "heart", 1, 0, 0, 1, 4);
    if (!random(6))
	add_leftover(LEFTOVER_OBJECT, "nose", 1, 0, 0, 0, 0);
    if (!random(6))
	add_leftover(LEFTOVER_OBJECT, "eye", 2, 0, 0, 0, 0);
    if (!random(6))
	add_leftover(LEFTOVER_OBJECT, "kidney", 2, 0, 0, 1, 3);
    if (!random(6))
	add_leftover(LEFTOVER_OBJECT, "intestine", 2, 0, 0, 1, 15);
    if (!random(6))
	add_leftover(LEFTOVER_OBJECT, "meat", 2, 0, 0, 1, 15);

    create_npc(); 
    MONEY_CONDENSE(this_object());
}

void
reset_npc()
{
    ::reset_creature();
}

nomask void
reset_creature()
{
    reset_npc();
}

/*
 * Description:  Use the combat file for generic tools
 */
public string 
query_combat_file() 
{
    return "/std/combat/ctool"; 
}

/*
 * Function name:  default_config_npc
 * Description:    Sets all neccessary values for this npc to function
 */
varargs public void
default_config_npc(int lvl)
{
    default_config_creature(lvl);
}

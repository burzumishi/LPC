/* 
   /std/act/ranpick.c

   Random picking of objects: Standard action module for mobiles

   set_pick_up(int)            If set to positive value, the monster
                               will pick up stuff found on the ground with
			       a certain %chance. (arg is %chance)

*/
#pragma save_binary
#pragma strict_types

#include <macros.h>
#include <stdproperties.h>

static	int	monster_pick;	       /* Chance of pick */

#define SEQ_PICK  "_mon_pick"

/*
 * Function name: set_pick_up
 * Description:   set the ability to pick up stuff from the ground
 * Arguments:	  pick - Positive value: Pick up stuff.
 */
void
set_pick_up(int pick)
{
    monster_pick = pick;

    if (!this_object()->seq_query(SEQ_PICK))
    {
	this_object()->seq_new(SEQ_PICK);
    }
    this_object()->seq_clear(SEQ_PICK);
    this_object()->seq_addfirst(SEQ_PICK, "@@monster_do_pick");
}

/*
 * Sequence functions
 */

int
can_pick(object ob)
{
    if (ob->query_prop(OBJ_I_NO_GET))
	return 0;
    return 1;
}

/*
 *  Description: The actual function picking, called by VBFC in seq_heartbeat
 */
void
monster_do_pick()
{
    object ob, *inv;

    this_object()->seq_clear(SEQ_PICK);
    this_object()->seq_addfirst(SEQ_PICK, ({ 4, "@@monster_do_pick" }) );

    if (random(100) > monster_pick)
	return;

    ob = environment(this_object());
    if (!ob)
	return;
    
    inv = filter(all_inventory(ob), can_pick);

    if (sizeof(inv))
    {
	ob = inv[random(sizeof(inv))];
	this_object()->seq_addfirst(SEQ_PICK, "get " + OB_NAME(ob));
    }
}

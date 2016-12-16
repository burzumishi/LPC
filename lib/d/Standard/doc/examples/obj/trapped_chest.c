/* /doc/examples/obj/trapped_chest.c
 *
 * An example of a trapped item using /d/Standard/lib/trap.c
 */

inherit "/std/receptacle";
inherit "/d/Standard/lib/trap";

#include <stdproperties.h>

public void
create_receptacle()
{
    set_name("chest");
    set_adj("small");
    add_adj("wooden");
    set_pshort("small wooden chests");

    /* Set the long description, including a VBFC to trap_desc() to
     * allow people to notice the trap by examining the chest.
     */
    set_long("A small wooden chest made of battered planks nailed "+
      "together and then bound by rusty iron bands for added "+
      "strength.  A small brass lock keeps unwanted hands out.\n" +
      "@@trap_desc@@");

    add_prop(CONT_I_WEIGHT, 2500);
    add_prop(CONT_I_MAX_WEIGHT, 10000);
    add_prop(CONT_I_VOLUME, 1000);
    add_prop(CONT_I_MAX_VOLUME, 3000);
    add_prop(CONT_I_RIGID, 1);
    add_prop(CONT_I_CLOSED, 1);

    add_prop(OBJ_I_VALUE, 120);

    /* We want to trigger the trap when it is opened or picked, so
     * we need to use set_cf() so that we can use the open() and
     * pick() functions.
     */
    set_cf(this_object());

    /* Set the id of the key for the chest */
    set_key("EXAMPLE_TRAPPED_CHEST_KEY_ID");
}

/*
 * Function name: spring trap
 * Description:   This function is called when the trap is sprung.
 * Arguments:     object who - the person who sprung the trap
 */
public void
spring_trap(object who)
{
    if (!query_trapped())
    {
        return;
    }

    who->catch_tell("A small razor sharp blade slashes "+
        "your hand as you " + query_verb() + " the chest!\n");
    who->heal_hp(-(90 + random(100)));

    if (who->query_hp() < 1)
    {
	who->do_die(this_object());
    }

    /* Disable the trap now that it has been sprung */
    set_trapped(0);
}

/*
 * Function name: open
 * Description:   This is called when the chest is opened.  We use it to
 *                spring the trap when that happens.
 */
public mixed
open(object ob)
{
    /* spring the trap */
    spring_trap(this_player());

    /* Returning 0 indicates that the chest can be opened */  
    return 0;
}

/*
 * Function name: pick_fail
 * Description:   This is called when someone tries to pick the 
 *                lock, but fails.  We want to spring the trap
 *                if that happens.
 * Arguments:     object who - the person that attempted to pick the lock
 *                int skill  - how well the well (badly) the lock was picked 
 */
public void
pick_fail(object who, int skill)
{
    /* spring the trap */
    spring_trap(who);
}

public void
init()
{
    ::init();
    /* Add the trap-related commands */
    init_trap();
}

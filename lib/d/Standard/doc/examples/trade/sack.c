/*
 * /d/Standard/doc/examples/trade/sack.c
 *
 * This sack is the "store-room" of the wandering merchant in the same
 * directory. This sack contains all functionality for a store-room and
 * it also has some extra's. In order to be able to carry more, this
 * sack reduces the weight of all objects inserted in it.
 *
 * If you want to see some more nifty stuff with respect to store-rooms,
 * check out the store.c in the same directory as this sack. Obviously,
 * that stuff can be applied to this sack as well, but one thing at a
 * time is enough.
 *
 * /Mercade, 10 March 1994
 *
 * If you are going to use this code for your objects (what examples are
 * meant for :-) ), please add a line in your comment header saying that
 * you did so.
 *
 * Revision history:
 */

inherit "/std/receptacle";
inherit "/lib/store_support";

#include <stdproperties.h>

/*
 * A receptacle is in fact a container with some commands added to it.
 * If you wish, you may also use create_receptacle().
 */
void
create_container()
{
    set_name("sack");
    set_adj("old");
    set_adj("large");

    set_short("large old sack");
    set_long(break_string("It is a large and old sack. The merchant whom " +
        "this sack once belonged to, must have travelled a lot with it " +
        "and it must have been in a lot of places.", 75) + "\n");

    /*
     * The CONT_I_REDUCE_WEIGHT and CONT_I_REDUCE_VOLUME reduce the
     * weight and volume of all objects inserted into this sack to 20
     * percent of their original weight/volume. When they leave the
     * sack, they will get their old stats back. This way the merchant
     * can carry more than he normally could. It allows him to put
     * 500 Kg into the sack and it will only burden him for 100 Kg.
     */
    add_prop(CONT_I_WEIGHT,          2000); /* container weights 2 Kg*/
    add_prop(CONT_I_MAX_WEIGHT,    102000); /* can carry 100 Kg */
    add_prop(CONT_I_REDUCE_WEIGHT,    500);
    add_prop(CONT_I_VOLUME    ,      3000); /* container is 3 liters */
    add_prop(CONT_I_MAX_VOLUME,    103000); /* can carry 100 liters  */
    add_prop(CONT_I_REDUCE_VOLUME,    500);

    add_prop(OBJ_M_NO_SELL, "@@check_contents");
    add_prop(OBJ_I_VALUE, 140);

    add_prop(OBJ_S_WIZINFO, break_string("This sack is the store-room of " +
        "the wandering merchant of the examples. It reduces the weight and " +
        "volume of the stuff that is put in it to 20 percent of its " +
        "original values. However, if this sack leaves the merchant, those " +
        "special properties are removed.", 75) + "\n");
}

/*
 * The function store_update should always be called with the object that
 * enters the store.
 */
void
enter_inv(object ob, object from)
{
    ::enter_inv(ob, from);

    store_update(ob);
}

/*
 * When this object leaves the merchant something must have happened to
 * the merchant. He is probably killed. Now we _must_ remove the
 * special properties. Otherwise mortal players could carry an almost
 * unlimited weight and we do not want that! When we remove it, the
 * weight of the stuff in it may be over 100Kg. That is not a problem,
 * but once removed, the same objects can never be put back if the
 * weight/volume would exceed 100Kg.
 *
 * The CONT_I_CLOSED property is added to annoy the player. The merchant
 * should obviously keep his sack closed though if we close the sack on
 * initialization, nothing can be inserted into it. With the open command
 * added to every receptacle, the mortal can open the sack though.
 */
void
leave_env(object from, object to)
{
    ::leave_env(from, to);

    /* did we leave the merchant? */
    if (from && from->id("_wandering_merchant"))
    {
        remove_prop(CONT_I_REDUCE_VOLUME);
        remove_prop(CONT_I_REDUCE_WEIGHT);
        add_prop(CONT_I_CLOSED, 1);
    }
}

/*
 * Lets be friendly. If someone tries to sell the sack, we check for him
 * or her whether it is empty and if it is not, we disallow him/her to
 * sell it.
 */
mixed
check_contents()
{
    if (sizeof(all_inventory()))
    {
        return "You cannot sell the sack for it is not empty.\n";
    }

    return 0;
}

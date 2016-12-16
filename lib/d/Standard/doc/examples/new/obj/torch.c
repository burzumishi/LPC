/*
 * torch.c
 */

/*
 * The torch lying in cave1.c
 */

inherit "/std/torch";

#include "ex.h"			/* Always good to have.... */
#include <stdproperties.h>	/* Properties */

/*
 * The create routine, called create_torch() in this case since we inherit
 * /std/torch.c
 */
void
create_torch()
{
    /*
     * Give the object a name.
     */
    set_name("torch");

    /*
     * Add an adjektive so players can 'look at small torch', not just
     * 'look at torch'
     */
    set_adj("small");

    /*
     * Long description shown when player examins this object.
     */
    set_long("A small torch but yet a light source better than a stick.\n");

    /*
     * Default short description would be 'small torch' with this name and
     * adjective. So you don't have to set the short description this time
     * unless you want to.
     */
    set_short("small torch");

    /*
     * Set the light strength on the torch. Good values are 1 for sticks, 2
     * on torches and 3 on lanterns.
     */
    set_strength(2);

    /*
     * Set how long time the torch can burn in seconds.
     */
    set_time(600);

    /*
     * Set the base value of the torch. This value will decayd when the torch
     * is burning. This is why we don't use a porperty to give the torch some
     * values.
     */
    set_value(30);

    /*
     * Properties! This time we'll use them to give this object weight and
     * volume. Weight is in grams, and volume in millilitres. Quite a small
     * torch in other words. Note that we have to include /sys/stdproperties.h
     * to be able to use the OBJ_I_WEIGHT and OBJ_I_VOLUME macros.
     */
    add_prop(OBJ_I_WEIGHT, 500);
    add_prop(OBJ_I_VOLUME, 300);
}

/*
 * We want to be able to recover the torch if the game crashes or goes down.
 * To make that possible we have to add two functions, query_recover() and
 * init_recover(). To handle normal torch query_torch_recover() and
 * init_torch_recover() has been made so unless we have done something special
 * with the torch we can use them as follows.
 * 
 * query_recover() returns a string that is used to recover this object later.
 * Therefor this function is of 'string' type. The first part of the recover
 * string is the file name of this object, the last part any argument that 
 * will be sent to init_recover() when recovering. This way you can let
 * variables be 'remembered'.
 */
string
query_recover()
{
    return EX_OBJ + "torch:" + query_torch_recover();
}

/*
 * init_recover() is called when the object is recovered.
 */
void
init_arg(string arg)
{
    init_torch_recover(arg);
}

/*
    /std/living/carry.c

    This is a subpart of living.c
    All routines related to carrying things are coded here.

    This file is inherited into living.c

    The weight unit is 1 gram.
    The maximum weight carried is (strength + 10) kg
    The volume unit is 1 ml.
    The maximum volume carried is (strength + 10) litre

*/

#include <macros.h>
#include <stdproperties.h>
#include <ss_types.h>

/*
 * Function name:   max_weight
 * Description:	    Calculate the maximum weight this living can budge.
 * Returns:         The weight.
 */
int
max_weight()
{
    return query_prop(CONT_I_WEIGHT) + (query_stat(SS_STR) + 10) * 1000;
}

/*
 * Function name:   max_volume
 * Description:	    Calculate the maximum volume this living can budge.
 * Returns:         The volume.
 */
int
max_volume()
{
    return query_prop(CONT_I_VOLUME) + (query_stat(SS_STR) + 10) * 1000;
}

/*
 * Function name: carry_reset
 * Description  : This function is called to reset the properties that are
 *                related to carrying or being carried.
 */
static nomask void
carry_reset()
{
    add_prop(OBJ_I_NO_GET, 1);	/* Lifeforms can't be picked up */
    add_prop(CONT_I_MAX_WEIGHT, max_weight);
    add_prop(CONT_I_MAX_VOLUME, max_volume);
}

/*
 * Function name:   query_encumberance_weight
 * Description:     Calculate how encumbered we are in % in weight
 * Returns:         The number in %
 */
int
query_encumberance_weight()
{
    int cont;

    return (query_prop(OBJ_I_WEIGHT) - (cont = query_prop(CONT_I_WEIGHT))) *
		100 / (query_prop(CONT_I_MAX_WEIGHT) - cont);
}

/*
 * Function name:   query_encumberance_volume
 * Description:     Calculate how encumbered we are in % in volume
 * Returns:         The number in %
 */
int
query_encumberance_volume()
{
    int cont;

    return (query_prop(OBJ_I_VOLUME) - (cont = query_prop(CONT_I_VOLUME))) *
                100 / (query_prop(CONT_I_MAX_VOLUME) - cont);
}

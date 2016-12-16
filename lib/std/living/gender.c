/*
 * /std/living/gender.c
 *
 * This is a subpart of living.c
 * All gender processing routines are defined here.
 *
 * This file is included into living.c
 */

#include <const.h>
#include <macros.h>

static nomask void
gender_reset()
{
    /* Illegal default value to force a change. */
    this_object()->set_gender(-1); 
}

/*
 * Function name:   query_gender_string
 * Description:     Gives back a string that contains the gender of a living
 * Returns:         The string
 */
public string
query_gender_string()
{
    return LD_GENDER_MAP[query_gender()];
}

/*
 * Function name:   query_pronoun
 * Description:     Returns the pronoun that goes with the gender of this
 *                  living.
 * Returns:         "he", "she" or "it", depending on gender.
 */
public string
query_pronoun()
{
    return LD_PRONOUN_MAP[query_gender()];
}

/*
 * Function name:   query_possessive
 * Description:     Returns the possessive that goes with the gender of this
 *                  living.
 * Returns:         "his", "her" or "its", depending on gender.
 */
public string
query_possessive()
{
    return LD_POSSESSIVE_MAP[query_gender()];
}

/*
 * Function name:   query_objective
 * Description:     Returns the objective that goes with the gender of this
 *                  living.
 * Returns:         "him", "her" or "it", depending on gender.
 */
public string
query_objective()
{
    return LD_OBJECTIVE_MAP[query_gender()];
}

/*
 * Function name: parse_command_adjectiv_id_list
 * Description  : This function is called from the gamedriver to obtain the
 *                list of adjectives applicable for this living. In this
 *                function we add the gender string as default adjective.
 * Returns      : string * - the adjectives of this living.
 */
public string *
parse_command_adjectiv_id_list()
{
    string *adjs = ::parse_command_adjectiv_id_list();

    if (pointerp(adjs))
    {
        return adjs + ({ query_gender_string() });
    }

    return 0;
}

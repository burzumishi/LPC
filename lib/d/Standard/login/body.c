/*
 * body.c
 *
 * This is a simply body that inhabits the bodies room in the character
 * creation sequence.  While it is a living, it isn't a 'real' living, 
 * in that it never interacts with a player. Rather, it just serves as a
 * physical template for a player to select a new body from. Players don't
 * actually enter these bodies, but by 'entering' a body, they really just 
 * copy the bodies variables over into their current ghost body, which will
 * then in turn be changed into a real one.
 *
 * Revision: Khail, July 26/96.
 *     Modified slightly from original format to incorporate the race
 *     descriptions into the body's description, changed the vars around
 *     a bit, and fully documented it.
 */

#pragma save_binary              // Save a binary copy of this file.

inherit "/std/living";

#include "login.h"
#include <macros.h>
#include <ss_types.h>
#include <stdproperties.h>
#include <const.h>

/*
 * Function name: create_living
 * Description  : Adds the 'body' name when first created, in addition to
 *                the normal create_living routine defined in /std/living.c
 */
create_living()
{
    ::create_living();
    add_name("body");
}

/*
 * Function name: create_body
 * Description  : Turns this object into one of several possible bodies 
 *                based on race and gender (or lack of). Race name, gender,
 *                basic stats and appearance are put toward making a new
 *                body for a player when they arrive in the bodies room.
 * Argument     : race - A string naming the desired race to create. Must 
 *                       be a valid race, such as 'elf', otherwise random
 *                       race is generated.
 *              : gender - optional string naming the gender. Either 'male'
 *                         or 'female'. If not set, a random gender is 
 *                         selected.
 */
public varargs void
create_body(string race, int gender)
{
    int *stats, i;
    
  /* Only want to make a 'real' object into a body, not the master */
  /* saved in memory. */
    if (!IS_CLONE)
        return;

  /* Give it a top-level name based on it's instance number. */
    add_name("#" + OB_NUM(this_object()));

    set_gender(gender);

    if (race && member_array(lower_case(race), RACES) >= 0)
    {
        race = lower_case(race);
	set_race_name(race);
    }
    else
	set_race_name(RACES[random(sizeof(RACES))]);

    set_adj(({ query_gender_string(), query_race()}));
//  set_short(query_gender_string() + " " + query_race() + " " + query_name());
    set_short(query_gender_string() + " " + query_race() + " body");

    set_long("The " + query_gender_string() + " " +
        query_race() + " bearing a tag labeled " +
        query_real_name() + " has an empty and blank stare. " +
        read_file(HELP + query_race() + "_race"));
    set_appearance(random(98) + 1);
	     
    add_prop(LIVE_I_NEVERKNOWN, 1);

    stats = RACESTAT[query_race()];

    for (i = 0 ; i < SS_NO_EXP_STATS; i++)
	set_base_stat(i, stats[i]);

  /* So all the bodies seem of normal length and width */
    add_prop(CONT_I_HEIGHT, RACEATTR[query_race()][0]);
    add_prop(CONT_I_WEIGHT, RACEATTR[query_race()][1]*1000);
    add_prop(CONT_I_VOLUME, RACEATTR[query_race()][4]*1000);
}

/*
 * MASKED
 * Function name: set_hp
 * Description  : Simply masks the normal set_hp so that the bodies won't
 *                heal, they'll always appear to be at death's door.
 */
void
set_hp(int hp)
{
}

/*
 * MASKED
 * Function name: start_heart
 * Description  : Masked to prevent the heart beat from starting when someone
 *                arrives in this object's environment.
 */
static void
start_heart()
{
}

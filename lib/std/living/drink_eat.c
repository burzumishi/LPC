/*
 * /std/living/drink_eat.c
 *
 * This is a subpart of living.c
 *
 * All food and drink routines are handled here.
 */

#include <formulas.h>
#include <drink_eat.h>

/*
 * Function name: 	intoxicated_max
 * Description:   	Gives the max intoxication level, used as a call
 *			function from the property: LIVE_I_MAX_INTOX
 * Returns:		Max intoxication level.
 */
int
intoxicated_max()
{
    return query_stat(SS_CON) * 2;
}

/*
 * Function name: 	intoxicated_min
 * Description:   	Gives the min intoxication level, used as a call
 *			function from the property: LIVE_I_MIN_INTOX
 * Returns:		Min intoxication level.
 */
int
intoxicated_min()
{
    return -query_stat(SS_CON);
}

/*
 * Function name: 	drink_max
 * Description:   	Gives the max amount of liquid that we can hold
 *			function from the property: LIVE_I_MAX_DRINK
 * Returns:		Max liquid level.
 */
int
drink_max()
{
      return 500 + query_stat(SS_CON) * 100;    /* 0.5 - Max 10.5 litres */
}

/*
 * Function name: 	eat_max
 * Description:   	Gives the max amount of food that we can hold
 *			function from the property: LIVE_I_MAX_EAT
 * Returns:		Max food level.
 */
int
eat_max()
{
      /* A newbie with CON 10 will be fully stuffed from eating 400g of food */
      return 100 + query_stat(SS_CON) * 30;  /* 0.1 - 3.1kg */
}

/*
 * Function name: 	drink_eat_reset
 * Description:   	Initializes the drink / food routines
 */
static void
drink_eat_reset()
{
    add_prop(LIVE_I_MAX_INTOX, intoxicated_max);
    add_prop(LIVE_I_MIN_INTOX, intoxicated_min);
    add_prop(LIVE_I_MAX_DRINK, drink_max);
    add_prop(LIVE_I_MAX_EAT,   eat_max);
}

/*
 * Function name: 	drink_alco
 * Description:   	Drinks alcohol of a certain potency
 * Arguments:		strength: The strength of the drink
 *			ask: True if we only want to know IF we can drink this
 * Returns:		True if successfully drunk.
 */
public int
drink_alco(int strength, int ask)
{
    int mintox, maxtox, curtox;;

    maxtox = query_prop(LIVE_I_MAX_INTOX);
    mintox = query_prop(LIVE_I_MIN_INTOX);
    curtox = query_intoxicated();

    /* Not too much, and not too much at once */
    if (!CAN_DRINK_ALCO(curtox, mintox, maxtox, strength)) return 0;

    if (ask)
	return 1;

    curtox += strength;
    
    set_intoxicated(curtox);

    return 1;
}

/*
 * Function name: 	drink_soft
 * Description:   	Drinks a certain amount of liquid
 * Arguments:		amount: The amount of the drink
 *			ask: True if we only want to know IF we can drink this
 * Returns:		True if successfully drunk.
 */
public int
drink_soft(int amount, int ask)
{
    int maxam, curam;

    maxam = query_prop(LIVE_I_MAX_DRINK);

    curam = query_soaked();

    if (amount >= 0)
    {

	/* Not too much, and not too much at once
	*/
	if (((curam + amount) > maxam) || (amount > maxam / 15))
	    return 0;
    }
    else
	if ((curam + amount) < 0)
	    return 0;

    if (ask)
	return 1;

    curam += amount;
    set_soaked(curam);

    return 1;
}

/*
 * Function name: 	eat_food
 * Description:   	Eats a certain amount of food.
 * Arguments:		amount: The amount of food.
 *			ask: True if we only want to know IF we can eat this
 * Returns:		True if successfully eaten.
 */
public int
eat_food(int amount, int ask)
{
    int curam;

    curam = query_stuffed();

    /* Not too much, and not too much at once */
    if (!CAN_EAT_FOOD(curam, query_prop(LIVE_I_MAX_EAT), amount))
	return 0;

    if (ask)
	return 1;

    curam += amount;
    set_stuffed(curam);

    return 1;
}

/*
 * /secure/admin/faircombat.c
 *
 * This module checks weapons and armours
 *
 * Rather unneccesary, time-consuming code that we could do without if people
 * only could stop cheating. 
 *
 */

#include "/sys/wa_types.h"
#include "/sys/log.h"
#include "/sys/math.h"

static void bad_object(object obj, string logfile, string logmess);

/*
 * Function name: check_wep
 * Description:   Check that a weapon is all right.
 * Arguments:	  wep - The weapon in question and takes neccesary actions to
			modify what is wrong.
 * Returns:       Correct weapon class.
 */
nomask int
check_wep(object wep)
{
    if (function_exists("check_weapon", wep) != "/std/weapon")
	return 0;

    return 1;
}

/*
 * Function name: check_arm
 * Description:   Check that a armour is all right.
 * Arguments:	  arm - The armour in question and takes neccesary actions to
			modify what is wrong.
 * Returns:       Correct armour class.
 */
nomask int
check_arm(object arm)
{
    if (function_exists("check_armour", arm) != "/std/armour")
	return 0;

    return 1;
}

/*
 * Function name: bad_object
 * Description:   Dispose of bad objects
 * Arguments:     obj - The object.
 *                logfile - Where to log the event.
 *                logmess - What to log.
 */
static void
bad_object(object obj, string logfile, string logmess)
{
    write("The " + obj->query_name() + " dissolves in a puff of black smoke!\n");
    log_file(logfile, logmess);
    obj->remove_object();
}

/*
 * Function name:
 * Description:
 * Arguments:
 * Returns:
 */

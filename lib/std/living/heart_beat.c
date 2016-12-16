/*
    /std/living/heart_beat.c

    This is a subpart of living.c
    All heart_beat relevant routines are defined here.

    This file is included into living.c
*/

/* You can disable heart_beat completely with this define. */
#undef HEART_NEEDED 

#include <config.h>		/* for STATUE_WHEN_LINKDEAD */
#include <macros.h>

static int stop_time;		/* Time when heart_beat was stopped */

static void heart_beat();

/*
 * Function name:   start_heart
 * Description:     Starts the heart beat for the living object and sees to
 *                  it that it heals as much as it shall due to lost time.
 */
static void
start_heart()
{
#ifdef HEART_NEEDED
    if (stop_time)
	heart_beat((time() - stop_time)/2);
    stop_time = 0;
    remove_call_out("heart_beat");
    set_alarm(2.0, 2.0, heart_beat);
#endif
}

/*
 * Function name: stop_heart
 * Description:   Stop the heart beat for the living object and mark
 *                the time for later reference.
 */
static void
stop_heart()
{
#ifdef HEART_NEEDED
    if (!stop_time)
	stop_time = time();
    remove_call_out("heart_beat");
#endif
}


/*
 * Function name:   heart_beat
 * Description:     Perform all heart_beat routines.
 */
static int
heart_beat()
{
#ifdef HEART_NEEDED

    /*
     * Call any heart_beat functions here.
     */
    return 1;
#else
    return 0;
#endif
}

void
restart_heart() 
{ 
#ifdef HEART_NEEDED
    set_alarm(2.0, 2.0, heart_beat);
#endif
}

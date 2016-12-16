/*
 * /d/Standard/lib/multi_alarm.c
 *
 * This object allows multiple alarms to be handled by a single alarm. The
 * following two functions can be called externally:
 *
 *
 * public int
 * set_multi_alarm(float delay, function fun)
 *
 * Function name: set_multi_alarm
 * Description  : With this function an alarm can be set. It works the same
 *                as a normal set_alarm() call with a few simplifications. It
 *                does not allow for repeating alarms, nor does it allow
 *                stringpointer functions to be called.
 * Arguments    : float delay  - the delay in seconds for the alarm. This
 *                               delay must be >= 0.5 seconds. If not, then
 *                               it will be set to 0.5 seconds.
 *                function fun - the function to be called when the alarm
 *                               is rung. All function-types are accepted.
 * Returns      : int - a unique alarm-identifier within this object. It can
 *                      be used to remove the alarm with remove_multi_alarm().
 *
 *
 * public void
 * remove_multi_alarm(int number)
 *
 * Function name: remove_multi_alarm
 * Description  : With this function, a call can be removed from the list
 *                of alarms to be called with multi_alarm().
 * Arguments    : int number - the identification number of the multi-alarm
 *                             to remove.
 */

#pragma save_binary
#pragma strict_types

private static mapping multi_alarms = ([ ]);
private static int multi_alarm_id = 0;
private static float multi_alarm_delay = 0.0;

/*
 * Function name: execute_multi_alarm
 * Description  : This function will be called whenever an alarm needs to
 *                be executed. It will execute those alarms that are within
 *                a 0.1 second interval from the designated time. Then it
 *                creates a new alarm if more alarms still need to ring.
 */
private void
execute_multi_alarm()
{
    float next_alarm;
    function fun;
    int   index;
    int   size;
    int  *indices;

    indices = m_indices(multi_alarms);
    size = sizeof(indices);
    index = -1;

    /* Loop over all alarms and see which should be executed within 0.1
     * second from now. Those 
     */
    while(++index < size)
    {
	if ((multi_alarms[indices[index]][1] - multi_alarm_delay) < 0.1)
	{
	    /* This temporary variable is necessary. */
	    fun = multi_alarms[indices[index]][0];

	    /* Don't allow buggy functions to screw up for other alarms.
	     * This does not trigger on eval-cost overflow. So if your
	     * function does overflow, the execution stops.
	     */
	    catch(fun());

	    /* Remove the alarm from the list. */
	    multi_alarms = m_delete(multi_alarms, indices[index]);
	}
    }

    /* If there are alarms left to be called, find out which is the next
     * and set an alarm to that time.
     */
    if (m_sizeof(multi_alarms))
    {
	indices = m_indices(multi_alarms);
	index = -1;
	size = sizeof(indices);
	next_alarm = multi_alarms[indices[0]][1] - multi_alarm_delay;

	while(++index < size)
	{
	    multi_alarms[indices[index]][1] -= multi_alarm_delay;

	    if (multi_alarms[indices[index]][1] < next_alarm)
	    {
		next_alarm = multi_alarms[indices[index]][1];
	    }
	}

	multi_alarm_id = set_alarm(next_alarm, 0.0, execute_multi_alarm);
	multi_alarm_delay = next_alarm;
    }
    /* Else, reset the internal variables. */
    else
    {
	multi_alarms = ([ ]);
	multi_alarm_id = 0;
	multi_alarm_delay = 0.0;
    }
}

/*
 * Function name: set_multi_alarm
 * Description  : With this function an alarm can be set. It works the same
 *                as a normal set_alarm() call with a few simplifications. It
 *                does not allow for repeating alarms, nor does it allow
 *                stringpointer functions to be called.
 * Arguments    : float delay  - the delay in seconds for the alarm. This
 *                               delay must be >= 0.5 seconds. If not, then
 *                               it will be set to 0.5 seconds.
 *                function fun - the function to be called when the alarm
 *                               is rung. All function-types are accepted.
 * Returns      : int - a unique alarm-identifier within this object. It can
 *                      be used to remove the alarm with remove_multi_alarm().
 */
public int
set_multi_alarm(float delay, function fun)
{
    mixed alarm_data;
    float remainder;
    float next_alarm;
    int   index;
    int   size;
    int  *indices;

    if (delay < 0.5)
    {
	delay = 0.5;
    }

    if (!functionp(fun))
    {
	return 0;
    }

    next_alarm = delay;
    if (multi_alarm_id)
    {
	alarm_data = get_alarm(multi_alarm_id);
        remainder = alarm_data[2];
	indices = sort_array(m_indices(multi_alarms));
	size = sizeof(indices);
	index = -1;

	while(++index < size)
	{
	    multi_alarms[indices[index]][1] -= (multi_alarm_delay - remainder);

	    if (multi_alarms[indices[index]][1] < next_alarm)
	    {
		next_alarm = multi_alarms[indices[index]][1];
	    }
	}

	index = indices[size - 1] + 1;
    }
    else
    {
	remainder = 0.0;
	index = 1;
    }

    remove_alarm(multi_alarm_id);
    multi_alarm_id = set_alarm(next_alarm, 0.0, execute_multi_alarm);

    multi_alarms[index] = ({ fun, delay });
    multi_alarm_delay = next_alarm;

    return index;
}

/*
 * Function name: remove_multi_alarm
 * Description  : With this function, a call can be removed from the list
 *                of alarms to be called with multi_alarm().
 * Arguments    : int number - the identification number of the multi-alarm
 *                             to remove.
 */
public void
remove_multi_alarm(int number)
{
    multi_alarms = m_delete(multi_alarms, number);
}
public mapping
get_all_multi_alarms()
{
    return secure_var(multi_alarms);
}

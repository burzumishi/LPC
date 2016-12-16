/*
 * Currently, the only way to cancel an input_to() is to remove the function
 * it calls.  This is often not practical, since the object where the function
 * is defined is needed.  In such a case, this object can be used.
 *
 * Simply clone this object and call get_input() with the appropriate
 * arguments.  A timeout can also specified.
 */

public void timeout_input(function func);
public void process_input(string input, function func);
public void remove_object();

/* 
 * Function name: get_input
 * Description:   Redirect player input to the specified function.  This can
 *                be canceled after a given duration by providing a timeout
 *                value.
 * Arguments:     1. (function) the function which should receive the player
 *                              input
 *                2. (float)    optional timeout duration
 */
public int
get_input(function func, float timeout = 0.0)
{
    if (!input_to(&process_input(, func)))
    {
        return 0;
    }

    if (timeout > 0.0)
    {
        set_alarm(timeout, 0.0, &timeout_input(func));
    }

    return 1;
}

/*
 * Function name: timeout_input
 * Description:   Called when the timeout duration is up to cancel the the
 *                input_to().  The input function will be called with a 0
 *                argument to signal the timeout.
 * Arguments:     (function) the function designated to receive the text input
 */
public void
timeout_input(function func)
{
    func(0);
    remove_object();
}

/*
 * Function name: process_input
 * Description:   This function captures the player's input and passes it to
 *                the function designated by the coder.
 * Arguments:     1. (string)   the player's input
 *                2. (function) the function designated to receive the input
 */
public void
process_input(string input, function func)
{
    func(input);
    remove_object();
}

public void
remove_object()
{
    destruct();
}

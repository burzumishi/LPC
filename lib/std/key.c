/* 
 * /std/key.c
 *
 * This is the standard key object.
 */

#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <stdproperties.h>

/*
 * Global variable. It contains the key-number.
 */
static mixed key;

/*
 * Function name: create_key
 * Description  : Call this function to create the key. Since you may not
 *                mask the function create_object, you must mask this one.
 */
public void
create_key()
{
}

/*
 * Function name: create_object
 * Description  : Initialize this object. It will set a few default
 *                variables. You may not mask this function. Define the
 *                function create_key() instead.
 */
nomask void
create_object()
{
    set_name("key");
    set_pname("keys");

    add_prop(OBJ_I_VALUE,  10);
    add_prop(OBJ_I_VOLUME,  5);
    add_prop(OBJ_I_WEIGHT, 30);

    create_key();
}

/*
 * Function name: reset_key
 * Description  : This function should hold the code that is to be executed
 *                when this key resets. In order for reset to work, you
 *                must call enable_reset() from your create_key.
 */
public void
reset_key()
{
}

/*
 * Function name: reset_object
 * Description  : You may not mask this function to make the key reset.
 *                Rather define the function reset_key.
 */
nomask void
reset_object()
{
    reset_key();
}

/*
 * Function name: set_key
 * Description  : Sets the key for the lock to use.
 * Arguments    : mixed keyval - the key value.
 */
public void
set_key(mixed keyval)
{
    if (query_lock())
    {
	return;
    }

    key = keyval;
}

/*
 * Function name: query_key
 * Description  : Returns the id number of the key.
 * Returns      : mixed - the key value.
 */
public mixed
query_key()
{
    return key;
}

/*
 * Function name: stat_object
 * Description  : If a wizard stats this key, we add the key number.
 * Returns      : string - the stat information.
 */
public string
stat_object()
{
    return ::stat_object() + sprintf("Key number: %O\n", key);
}


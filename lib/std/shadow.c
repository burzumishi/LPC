/*
 * /std/shadow.c
 *
 * This is a generic shadow object.
 */

#pragma save_binary
#pragma strict_types

#include "/sys/macros.h"

static object	shadow_who;	/* Which object are we shadowing */

/*
 * Function name: autoload_shadow
 * Description  : Called by the autoloading routine in /std/player_sec
 *                to ensure autoloading of the shadow.
 * Arguments    : mixed arg - possible arguments.
 */
public void
autoload_shadow(mixed arg)
{
    if (!objectp(shadow_who))
    {
	shadow_who = previous_object();
	shadow(shadow_who, 1);
    }
}

/*
 * Function name: shadow_double
 * Description  : Checks whether a certain shadow(file) is already
 *                shadowing a certain object.
 * Arguments    : string fname - the filename of the shadow to check.
 *                object ob    - the object to check whether it is already
 *                               shadowed by fname.
 * Returns      : int 1/0 - true if the ob is already shadowed by fname.
 */
static int
shadow_double(string fname, object ob)
{
    while (objectp(ob = shadow(ob, 0)))
    {
	if (fname == MASTER_OB(ob))
	{
	    return 1;
	}
    }

    return 0;
}    

/*
 * Fucntion   : remove_object
 * Description: Ensure that we remove the shadow object too
 */
public void 
remove_object() 
{ 
    if (objectp(shadow_who))
    {
        shadow_who->remove_object();
    }

    destruct();
}

/*
 * Function   : remove_shadow
 * Description: Removes the shadow without removing the shadowed object.
 */
public void 
remove_shadow()
{
    destruct();
}

/*
 * Function name: shadow_me
 * Description  : Called from object to make us shadow it.
 * Arguments    : mixed to_shadow - either a string or object. If it is
 *                a string, use find_player to find the player to shadow.
 * Returns      : int - True if shadowed.
 */
public varargs int
shadow_me(mixed to_shadow)
{
    if (stringp(to_shadow))
    {
	to_shadow = find_player(to_shadow);
    }
    
    if (!objectp(to_shadow))
    {
	to_shadow = previous_object();
    }

    if ((!objectp(shadow_who)) &&
	(!shadow_double(MASTER, to_shadow)))
    {
	if (shadow(to_shadow, 1))
	{
	    shadow_who = to_shadow;
	    return 1;
	}
    }
    return 0;
}

/*
 * Function name: query_shadow_who
 * Description  : Returns the object that we are shadowing.
 * Returns      : object - the object we are shadowing. 
 */
public object
query_shadow_who()
{
    return shadow_who;
}

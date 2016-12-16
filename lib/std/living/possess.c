/* 
 * /std/living/possess.c
 */

#include <files.h>

static string possessed;	/* Possessing demon */

/*
 * Function name: remove_object
 * Description:   Destruct this object, but check for possessed first
 */
private void
possessed_remove()
{
    object ob;

    while (ob = present("possob", this_object()))
    {
	if (function_exists("create_object", ob) == POSSESSION_OBJECT)
	{
	    command("$quit");
	}
	else
	{
	    ob->remove_object();
	    if (ob)
            {
		SECURITY->do_debug("destroy", ob);
            }
	}
    }
}

/*
 * Function name: set_possessed
 * Description:	  Sets possessd status.
 * Arguments:	  demon - The one who possesses
 */
public nomask void
set_possessed(string demon)
{
    if (function_exists("possess", previous_object()) != POSSESSION_OBJECT)
	return;

    possessed = demon;
}

/*
 * Function name: query_possessed
 * Description:	  Returns possessed status
 */
public nomask string
query_possessed()
{
    return possessed;
}

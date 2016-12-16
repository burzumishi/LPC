/*
   subloc.c

   Some routines for default descriptions of sublocations
*/

#pragma save_binary

#include <subloc.h>
#include <composite.h>
#include <stdproperties.h>

public int object_visible(object ob, object fob);

/*
 * Show a default sublocation
 * 
 * Returns: 	string - Description of inventory of sublocation
 *		0 -	 Unsupported sublocation
 *		1 -	 Empty sublocation.
 */
public mixed
show_subloc(mixed sloc, object cont, object for_obj)
{
    string p, pron, defprep, str;
    object *invobs;

#ifndef Genesis_jnatest
    return 0;
#endif

    if (function_exists("create_container", cont) != "/std/living")
    {
	pron = " it: ";
	defprep = "in";
    }
    else if (cont != for_obj)
    {
	pron = " " + cont->query_objective() + ": ";
	defprep = "carried by";
    }
    else
    {
	pron = " you: ";
	defprep = "carried by";
    }	

    if (sloc == 0)
	p = defprep;
    else if (sscanf(sloc, SUBL_PREPOS + "%s", p) != 1)
	return 0;

    invobs = cont->subinventory(sloc);
    if (sizeof(invobs))
	invobs = filter(invobs, &object_visible(, for_obj));

    if (sizeof(invobs))
    {
	if (cont != for_obj || sloc != 0)
	    str = capitalize(p) + pron + FO_COMPOSITE_DEAD(invobs, for_obj);
	else /* It must be a living, right? */
	    str = "You are carrying " + FO_COMPOSITE_DEAD(invobs, for_obj);
	return break_string(str + ".", 76, 2) + "\n";
    }
    else if (cont == for_obj && sloc == 0)
	return "  You do not carry anything.\n";

    return 1;
}

/*
 * Finds out if an object is visible or not.
 * Does not check containers etc, only the object itself
 */
public int
object_visible(object ob, object fob)
{
    if (ob->query_no_show())
	return 0;

    if (fob->query_wiz_level())
	return 1;

    if (ob->query_prop(OBJ_I_INVIS) <= 
	fob->query_prop(LIVE_I_SEE_INVIS))
	return 1;
    else
	return 0;
}

static string
obshorts(object ob, object fobj)
{
    if (object_visible(ob, fobj))
	return ob->short(fobj);
    else
	return 0;
}

/* 
 * Function name: subloc_access
 * Description:   Check if a sublocation can be accessed or not.
 *		  This is called from /std/container to manage sublocations
 *		  without manager objects. Normally prepositions.
 * Arguments:     sloc: Name of the sublocation
 *		  ob:   The object containing the subloc
 *		  acs:	Access type as defined in /sys/subloc.h
 *		  for_obj: Living for which the access is to be checked
 * Returns:	  1 if accessible
 */
public int
subloc_access(string sloc, object ob, string acs, object for_obj)
{
    string p;

    /* Only handle specific preposition sublocation
     */
    if (sscanf(sloc, SUBL_PREPOS + "%s", p) != 1)
	return 0;

    if (acs == SUBL_ACS_SEE)
    {
	switch (sloc)
	{
	case "in":
	case "inside":
	case "within":
	    if (ob->query_prop(CONT_I_HIDDEN) ||
		(ob->query_prop(CONT_I_CLOSED) &&
		 !ob->query_prop(CONT_I_TRANSP) &&
		 !ob->query_prop(CONT_I_ATTACH)))
		return 0;
	    else
		return 1;
	    break;
	default:
	    return 1;
	}
    }
    else if (acs == SUBL_ACS_MANIP)
    {
	switch (sloc)
	{
	case "in":
	case "inside":
	case "within":
	    if (ob->query_prop(CONT_I_HIDDEN) ||
		(ob->query_prop(CONT_I_CLOSED) &&
		 !ob->query_prop(CONT_I_ATTACH)))
		return 0;
	    else
		return 1;
	    break;
	default:
	    return 1;
	}
    }
}


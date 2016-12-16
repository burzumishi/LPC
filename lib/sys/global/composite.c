/*
 *  Description routines, composite sentences
 */
#pragma save_binary
#pragma strict_types

#include <language.h>
#include <options.h>
#include <stdproperties.h>

object *extra = ({});
mixed *OldArr = ({});
/*
 *  Prototypes
 */
string desc_same(object *oblist, object for_obj);
varargs string composite(mixed arr, string sepfunc, function descfunc, 
    object for_obj, int include_no_show);
string lpc_describe(mixed *uarr, function dfun, object for_obj);
varargs string composite_words(string *wlist, string word);


static varargs string
desc_live_dead(mixed arr, object for_obj, int include_no_show)
{
    return composite(arr, "short", desc_same, for_obj, include_no_show);
}

varargs string
desc_live(mixed arr, int no_someone, object for_obj, int include_no_show)
{
    string str;

    if (!objectp(arr) && !sizeof(arr) && !no_someone)
    {
        return "nobody";
    }

    if (!for_obj)
    {
        for_obj = this_player();
    }

    str = desc_live_dead(arr, for_obj, include_no_show);

    if (!str && !no_someone)
    {
	return "someone";
    }

    return str;
}

varargs string
fo_desc_live(mixed arr, object for_obj, int no_someone, int include_no_show)
{
    if (!for_obj)
    {
        for_obj = previous_object(-1);
    }
   
    if (!arr)
    {
        arr = OldArr;
    }

    return desc_live(arr, no_someone, for_obj, include_no_show);
}

/* Necessary for QCOMPLIVE to work with COMPOSITE_ALL_LIVE */
varargs string
fo_desc_last_live()
{
    object for_obj = previous_object(-1);
    return fo_desc_live(0, for_obj, 0, 1);
}

varargs string
desc_dead(mixed arr, int no_something, object for_obj, int include_no_show)
{
    string str;

    if (!objectp(arr) && !sizeof(arr) && !no_something)
    {
        return "nothing";
    }

    if (!for_obj)
    {
        for_obj = this_player();
    }

    str = desc_live_dead(arr, for_obj, include_no_show);

    if (!str && !no_something)
    {
	return "something";
    }

    return str;
}

varargs string
fo_desc_dead(mixed arr, object for_obj, int no_something, int include_no_show)
{
    if (!for_obj)
    {
        for_obj = previous_object(-1);
    }

    if (!arr)
    {
        arr = OldArr;
    }

    return desc_dead(arr, no_something, for_obj, include_no_show);
}

/* Necessary for QCOMPDEAD to work with COMPOSITE_ALL_DEAD */
varargs string
fo_desc_last_dead()
{
    object for_obj = previous_object(-1);
    return fo_desc_dead(0, for_obj, 0, 1);
}

/*
 * Function:    desc_same
 * Description: Gives a textdescription of a set of nonunique objects.
 *              Normal case: "two swords", "a box" etc.
 *              The basic function to get the description of an objects is
 *              its 'short' function. Pluralforms are taken from:
 *              1) The 'plural_short' function of the object.
 *              2) If 1) is not defined then the singular form is transformed
 *                 into the pluralform, not always correctly.
 *              Articles are added to singular objects that does NOT have a
 *              capitalized short description and that are not heap objects.
 * Arguments:   oblist: Array of objectpointers to the nonunique objects.
 *              for_obj: the object for which the description is being generated
 * Return:      Description string or 0 if no 'short' was not defined.
 */
string
desc_same(object *oblist, object for_obj)
{
    string pshort, sshort, pre, aft;
    object ob;

    if (!pointerp(oblist))
    {
        oblist = ({ oblist });
    }

    if (!sizeof(oblist))
    {
	return "";
    }

    pre = aft = "";
    ob = oblist[0];

    if (ob->query_prop(OBJ_I_HIDE) > 0)
    {
	pre = "[";
	aft = "]";
    }

    if (ob->query_prop(OBJ_I_INVIS) > 0)
    {
	pre = "(";
	aft = ")";
    }

    sshort = ob->short(for_obj);

    if (sizeof(oblist) > 1)
    {
	if (ob->query_prop(HEAP_I_IS))
	{
	    extra += oblist;
	    return 0;
	}

	pshort = (string)ob->plural_short(for_obj);

	if (!stringp(pshort) && !stringp(sshort))
	{
	    return 0;
	}

	if (!pshort)
	{
	    pshort = LANG_PSENT(sshort);
	}

  	if (sizeof(oblist) < 9)
	{
	    return pre + LANG_WNUM(sizeof(oblist)) + " " + pshort + aft;
	}

        return pre + "many " + pshort + aft;
    }

    if (!sshort)
    {
	return 0;
    }

    else if (ob->query_prop(HEAP_I_IS))
    {
	return pre + sshort + aft;
    }

    if (sshort[0] > 'Z')
    {
	return pre + LANG_ADDART(sshort) + aft;
    }

    return pre + sshort + aft;
}

varargs string
fo_composite(mixed arr, string sepfunc, function descfunc, object for_obj)
{
    if (!for_obj)
    {
	for_obj = previous_object();
    }

    return composite(arr, sepfunc, descfunc, for_obj);
}

/*
 * Function:    composite
 * Description: Creates a composite description of objects
 * Arguments:   arr:        Array of the objects to describe
 *              sepfunc:    Function to call in objects to get its <name>
 *                          Objects with the same <names> are described
 *                          together.
 *              descfunc:   Function to call to get the actual description of
 *                          a group of 'same objects' i.e objects whose
 *                          sepfunc returned the same value.
 *              for_obj:    object for which the description is being generated
 * 
 * Returns:     A description string on the format:
 *              <desc>, <desc> and <desc> 
 *              Where <desc> is what obj->descfunc() returns
 *
 */
varargs string
composite(mixed arr, string sepfunc, function descfunc, object for_obj,
    int include_no_show)
{
    mixed *a;
    
    if (!for_obj)
    {
	for_obj = this_player();
    }
    
    if (objectp(arr))
    {
	arr = ({ arr });
    }

    if (!include_no_show)
    {
        arr = filter(arr, not @ &->query_no_show_composite());
    }

    OldArr = arr;

    arr = filter(arr, &->check_seen(for_obj));

    if (!sizeof(arr))
    {
	return 0;
    }

    /* Make an array of unique lists of objects 
     */    
    a = unique_array(arr, sepfunc); 

    return lpc_describe(a, descfunc, for_obj);
}

/*
 * Function:    sort_similar
 * Description: sort an array in order shown to player
 * Arguments:   arr:        Array of the objects to sort
 *              sepfunc:    Function to call in objects to get its <name>
 *                          Objects with the same <names> are sorted
 *                          together.
 * 
 * Returns:     0 or sorted array
 *
 */
mixed
sort_similar(mixed arr, string sepfunc)
{
    int b, i;
    mixed *a;

    if ((!pointerp(arr)) || !sizeof(arr))
    {
        return 0;
    }
    
    a = unique_array(arr, sepfunc);

    if (!sizeof(a))
    {
	return 0;
    }

    b = a[0];
    for (i = 1; i < sizeof(a); i++)
    {
        b += a[i];
    }
    
    return b;
}

int
no_invis(mixed str)
{
    return (stringp(str));
}

string
lpc_describe(mixed *uarr, function dfun, object for_obj)
{
    mixed *a, *b;

    extra = ({});
    a = map(uarr, &dfun(, for_obj));

    if (sizeof(extra))
    {
    	b = map(extra, dfun);
	a += b;
    }

    a = filter(a, no_invis);

    return (sizeof(a) ? composite_words(a) : 0);
}

/*
 * Function name: composite_words
 * Description  : Concatenates a range of words with commas and a closer.
 * Arguments    : string *wlist - the words to concatenate.
 *                string word - the closer. Defaults to "and".
 * Returns      : string - "a, b, c and d"
 */
varargs string
composite_words(string *wlist, string word = "and")
{
    if (!sizeof(wlist))
    {
	return "";
    }

    if (sizeof(wlist) == 1)
    {
	return wlist[0];
    }

    if (sizeof(wlist) == 2)
    {
	return wlist[0] + " " + word + " " + wlist[1];
    }

    return implode(wlist[0..sizeof(wlist) - 2], ", ") +	" " + word + " " +
        wlist[sizeof(wlist) - 1];
}

/*
 * Function name: hanging_indent
 * Description  : This can be used to have some text tabulated. The first
 *                line of text begins left aligned and the rest of the
 *                lines will be indented.
 * Arguments    : string to_print - the text to parse. This should not contain
 *                    a trailing newline. That will be added automatically.
 *                int length - the length of the tabulated space.
 *                int width - 0 - use OPT_SCREEN_WIDTH on this_player().
 *                            1 - use the default (80 character screen).
 *                            i - use 'i' as width.
 * Returns      : string - the parsed text.
 */
public string
hanging_indent(string to_print, int length, int width)
{
    int    scrw; 
    string *tmp;

    switch(width)
    {
    case 0:
        scrw = this_player()->query_option(OPT_SCREEN_WIDTH);
	/* No screen width set. The person won't get any indentation. */
	if (scrw <= 0)
	{
	    return to_print + "\n";
	}
        scrw -= 3;
        break;

    case 1:
        scrw = 77;
        break;

    default:
        scrw = width;
    }

    if (strlen(to_print) <= scrw)
    {
        return (to_print + "\n");
    }

    length = ((length > 20) ? 20 : length);

    /* I'm sure you'll appreciate this return-statement.
     * Doesn't it look cute? *smile* /Mercade
     */
    tmp = explode(break_string(to_print, scrw), "\n");
    return (implode( ({ tmp[0] }) +
        (explode(break_string(implode(tmp[1..], " "),
        (scrw - length)), "\n")),
        ("\n" + "                    "[1..length])) + "\n");
}


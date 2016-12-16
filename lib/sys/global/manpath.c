/*
 * manpath.c
 *
 * Version 2.0
 *
 * Handle manaul search requests in the /doc/man section of the 
 * documentation.
 */

#pragma save_binary

#include <macros.h>
#include <std.h>

#define MANDIR "/doc/man"

static string	*chapters;		/* The chapters under MANDIR */
static mapping	chapt_index;		/* Array of "chapterized" permutated
					   indexes on the form:

					   ([ "chaptname" : "subj%%subj%%" ])

					   */

int
is_dir(string fname)
{
    return ((file_size(MANDIR + "/" + fname) == -2) &&
	    (member_array(fname, ({".",".."}) ) < 0));
}

int
is_file(string fname, string chapt)
{
    return (file_size(MANDIR + "/" + chapt + "/" + fname) > 0);
}

/*
 * Function name:   init_man
 * Description:     Initialize the index arrays.
 */
static void
init_man()
{
    int i;
    string *files, subjects;

    chapt_index = ([]);

    chapters = filter(get_dir(MANDIR + "/*"), is_dir);

    for (i = 0; i < sizeof(chapters); i++)
    {
	files = filter(get_dir(MANDIR + "/" + chapters[i] + "/*"),
		       &is_file(, chapters[i]));
	chapt_index[chapters[i]] = "%%" + implode(files, "%%") + "%%";
    }
}

/*
 * Function name:   get_index
 * Description:     Return all subjects in a given chapter
 * Arguments:	    chapter - The chapter to search in. (Optional)
 * Returns:         An array containing the list of found subjects in each
 *		    chapter. Each entry is on the form: 
 *			({ "chapter", ({ "subject", "subject" ... }) })
 */
public mixed *
get_index(string chapt)
{
    mixed *found_arr, *tmp;
    int i;

    if (stringp(chapt) && member_array(chapt, chapters) >= 0)
    {
	return ({ chapt, explode(chapt_index[chapt], "%%") });
    }
    
    found_arr = ({});
    for (i = 0; i < sizeof(chapters); i++)
    {
	tmp =  explode(chapt_index[chapters[i]], "%%");
	
	if (sizeof(tmp))
	{
	    found_arr += ({ ({ chapters[i], tmp }) });
	}
    }
    return found_arr;
}



static string * fix_subjlist(string *split, string keyw,int okbef, int okaft);

/*
 * Function name:   get_keywords
 * Description:     Return all possible subject name array matches of a 
 *		    given keyword in one or all chapters.
 * Arguments:	    chapter - The chapter to search in. (Optional)
 *		    keyword - The keyword to seach for.
 * Returns:         An array containing the list of found subjects in each
 *		    chapter. Each entry is on the form: 
 *			({ "chapter", ({ "subject", "subject" ... }) })
 */
public mixed *
get_keywords(string chapt, string keyword)
{
    mixed *found_arr, *tmp;
    int i, okbef, okaft;
    string st;
    
    okbef = 0; 
    okaft = 0;

    if (sscanf(keyword, "*%s", st) == 1)
    {
	keyword = st;
	okbef = 1;
    }

    if (sscanf(keyword, "%s*", st) == 1)
    {
	keyword = st;
	okaft = 1;
    }

    if (stringp(chapt) && member_array(chapt, chapters) >= 0)
    {
	return ({ chapt, fix_subjlist(explode(chapt_index[chapt], keyword),
			   keyword, okbef, okaft) });

    }
    else if (stringp(chapt)) /* No such chapter */
    {
	return ({});
    }
    
    found_arr = ({});
    for (i = 0; i < sizeof(chapters); i++)
    {
	tmp = fix_subjlist(explode(chapt_index[chapters[i]], keyword),
			   keyword, okbef, okaft);
	if (sizeof(tmp))
	{
	    found_arr += ({ ({ chapters[i], tmp }) });
	}
    }
    
    return found_arr;
}

/*
 * Function name:   fix_subjlist
 * Description:     Extract the subject names from a subjectlist split
 *		    on a keyword. Typical example: "%%hubba%%hubbi%%bibi%%"
 *		    split on 'bb' will give:
 *			({ "%%hu", "a%%hu", "i%%bibi%%" })
 *		    The result should be:
 *			({ "hubba", "hubbi" })   
 * Arguments:	    split: The subject list split on a keyword
 *		    keyw:  The keyword used to split the list.
 *		    okbef: True if letters before keyword is acceptable
 *		    okaft: True if letters after keyword is acceptable
 * Returns:         An array containing the matching subjects.
 *			
 */
string *
fix_subjlist(string *split, string keyw, int okbef, int okaft)
{
    int il, fb, fa;
    string t, *bef, bstr, fstr;
    string *res;

    res = ({});

    for (il = 1; il < sizeof(split);)
    {
	fa = 0; fb = 0;
	bef = explode(split[il-1] + "&&", "%%");
	t = bef[sizeof(bef)-1];
	if (t != "&&")
	{
	    sscanf(t,"%s&&", bstr);
	    fb = 1;
	}
	else
	    bstr = "";

	t = "&&";
	while (t == "&&" && il < sizeof(split))
	{
	    fstr = "";
	    bstr += keyw;
	    sscanf(split[il] + "%%&&", "%s%%%s", fstr, t);
	    il++;
	    bstr += fstr;
	    if (fstr != "")
		fa = 1;
	}

	if (fb && !okbef)
	    continue;

	if (fa && !okaft)
	    continue;

	res += ({ bstr });
    }
    return res;
}

public string *
get_chapters()
{
    if (!chapters)
	init_man();

    return chapters;
}

void
remove_object()
{
    destruct();
}

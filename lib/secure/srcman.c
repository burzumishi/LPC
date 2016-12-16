/*
 * /secure/srcman.c
 *
 * Handle search requests in source documentation.
 */
#pragma strict_types
#pragma save_binary

#include <macros.h>
#include <std.h>

#define SMANDIR "/doc/sman"
#define MAXLOCAL_DIRS 10
/*
 * DOCMARK
 *
 * If defined, this file must exist within a directory for the directory
 * to be accepted as a documentation directory
 *
 */
#undef DOCMARK /* "/DOCDIR" */

static 	mapping	docdirs;	/* Info on all docdirs currently known. 
				   Each entry on the form:
				     (["/dir/name/": 
				       ({ sdir_sortedlist, ([ subdir:info ]) })
				     ]) 
    				   There can be many subdir:info for one main
				   docdir. 'info' is on the form:
				     "%%function1%%function2%%function3"

				*/



/*
 * Function name:   valid_docdir
 * Description:     Check if this is a valid documentation directory
 */
int 
valid_docdir(string ddir)
{
#ifdef DOCMARK
    if (file_size(ddir + DOCMARK) < 0)
	return 0;
    else
#endif
	return 1;
}


/*
 * Function name:   read_index
 * Description:     Read the contents of a docdir and return a mapping
 *		    containing all the ([ subdir:info ]) information
 *
 */
static mapping 
read_index(string mdir)
{
    string *files, *sfiles, sdname, info;
    mapping res, sdir;
    int i, j;

    seteuid(geteuid(this_player()));

    files = get_dir(mdir + "/*");

    if (!sizeof(files))
	return ([]);

    files -= ({ ".", ".." });

    info = "%%";
    res = ([]);

    for (i = 0; i < sizeof(files); i++)
    {
	sdname = mdir + "/" + files[i];
	if (file_size(sdname) == -2 && (files[i] != ".obsolete"))
	{
	    if (mappingp(docdirs[sdname]))
		sdir = docdirs[sdname];
	    else
		sdir = read_index(sdname);
	    sfiles = m_indexes(sdir);
	    for (j = 0; j < sizeof(sfiles); j++)
	    {
		res["/" + files[i] + 
		    (sfiles[j] == "/" ? "" : sfiles[j])] = sdir[sfiles[j]];
	    }
	}
	else 
	    info += files[i] + "%%";
    }
    if (strlen(info) > 2)
	res["/"] = info;

    return res;
}

/*
 * Function name:   init_man
 * Description:     Initialize the docdir info for a specific docdir
 */
static void
init_docdir(string mdir)
{
    string stmp, sdnames;
    mapping sdirs;

    if (!mappingp(docdirs))
    {
	docdirs = ([]);
	if (mdir != SMANDIR)
	    init_docdir(SMANDIR);
    }

    if (!valid_docdir(mdir))
	return;
    
    if (m_sizeof(docdirs) >= MAXLOCAL_DIRS)
    {
	stmp = m_indexes(docdirs)[random(m_sizeof(docdirs) - 1) + 1];
	m_delkey(docdirs, stmp);
    }

    sdirs = read_index(mdir);
    sdnames = sort_array(m_indexes(sdirs));

    if (m_sizeof(sdirs))
	docdirs[mdir] = ({ sdnames, sdirs });
}

/*
 * Function name:   update_index
 * Description:     Update the function index
 */
public void
update_index(string mdir)
{
    init_docdir(mdir);
}


/*
 * Function name:   get_subdirs
 * Description:     Return a list of subdirectories for a given doc-dir
 */
string *
get_subdirs(string ddir)
{
    if (!mappingp(docdirs) || !pointerp(docdirs[ddir]))
	init_docdir(ddir);

    if (!sizeof(docdirs[ddir]))
	return ({});

    return docdirs[ddir][0];
}

string *
fix_subjlist(string *split, string keyw, int okbef, int okaft)
{
    return MANCTRL->fix_subjlist(split, keyw, okbef, okaft);
}

/*
 * Function name:   get_keywords
 * Description:     Return all possible function names that match a 
 *		    given keyword in one or all subdirs.
 * Arguments:	    mdir    - The main documentation directory
 *		    subdir  - The subdir to search in. (Optional)
 *		    keyword - The keyword to search for.
 * Returns:         An array containing the list of found names in each
 *		    subdir. Each entry is on the form: 
 *			({ "subdir", ({ "funcname", "funcname" ... }) })
 */
public mixed *
get_keywords(string mdir, string sdir, string keyword)
{
    mixed *found_arr, *tmp;
    string *sdlist, st;
    int i, okbef, okaft;

    if (!sizeof(sdlist = get_subdirs(mdir)))
	return ({ });

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

    if (keyword == "")
	return this_object()->get_index(mdir, sdir);

    if (stringp(sdir) && member_array(sdir, sdlist) >= 0)
	return ({ sdir, fix_subjlist(explode(docdirs[mdir][1][sdir], keyword),
				     keyword, okbef, okaft) });

    else if (stringp(sdir)) /* No such subdir */
	return ({});

    found_arr = ({});
    for (i = 0; i < sizeof(sdlist); i++)
    {
	tmp = fix_subjlist(explode(docdirs[mdir][1][sdlist[i]], keyword),
			   keyword, okbef, okaft);
	if (sizeof(tmp))
	    found_arr += ({ ({ sdlist[i], tmp }) });
    }
    
    return found_arr;
}


/*
 * Function name:   get_index
 * Description:     Return all function names in a given subdir (object)
 * Arguments:	    mdir - The main documentation directory
 *		    sdir - The subdir to search in. (Optional)
 * Returns:         An array containing the list of found names in each
 *		    subdir. Each entry is on the form: 
 *			({ "subdir", ({ "funcname", "funcname" ... }) })
 */
public mixed *
get_index(string mdir, string sdir)
{
    mixed *found_arr, *tmp;
    string *sdlist;
    int i;
    
    if (!sizeof(sdlist = get_subdirs(mdir)))
	return ({ });

    if (stringp(sdir) && member_array(sdir, sdlist) >= 0)
	return ({ sdir, explode(docdirs[mdir][1][sdir], "%%") });

    else if (stringp(sdir)) /* No such subdir */
	return ({});
    
    found_arr = ({});
    for (i = 0; i < sizeof(sdlist); i++)
    {
	tmp = explode(docdirs[mdir][1][sdlist[i]], "%%");

	if (sizeof(tmp))
	    found_arr += ({ ({ sdlist[i], tmp }) });
    }
    
    return found_arr;
}

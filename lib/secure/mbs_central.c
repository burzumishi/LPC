/*
 * mbs_central.c
 *
 * Mrpr's Board System - Board reading soul central object.
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/lib/cache";

#include <files.h>
#include <macros.h>
#include <mail.h>
#include <mbs.h>
#include <std.h>
#include <stdproperties.h>
#include <time.h>

/* Some nifty defines I tend to use */
#define NF(message)     notify_fail(message)
#define TP              this_player()
#define TI              this_interactive()
#define TO              this_object()
#define PO              previous_object()
#define LC(str)         lower_case((str))
#define UC(str)         capitalize(lower_case((str)))

/*
 * Globals, saved
 */
string		*AdminList;	// The list of trusted admins
mapping		CategoryMap,	// The mapping of categories
		BbpMap,		// Board by path map
    		BrokenMap,	// The mapping of broken boards
		UnusedMap;	// The mapping of unused boards
int		ReportTime,	// Last time a report was made
	        GcTime;		// Last time a global change was made
mapping		HelpMap;	// Contains indices in the help map file.

/*
 * Globals, static
 */
static mapping	BbcMap;		// Board by category map
static mapping	BbdMap;		// Board by domain map
static int	SaveCount;	// Save count for saving read board info
static int	SaveAlarm;	// Save alarm id
static int	IndexLine,	// The current index line.
		CountLine,	// The number of lines in a help text.
		HelpAlarmId;	// The id-number of the alarm used.
static string	HelpCmdName;	// The current command name.
static mapping  BobMap;		// Board object mapping

/*
 * BbpMap : ([ "save path" :
 *	({ "board", "category", "domain", "description",
 *		"save path", "room path", "last note", "# read on board" }) ])
 *
 * BbcMap : ([ "category" :
 *	= list of BbpMap value lists = ])
 *
 * BbdMap : ([ "domain" :
 *	= list of BbpMap value lists = ])
 *
 * BrokenMap, UnusedMap : ([ "save path" : time stamp ]);
 */

/*
 * Quick filter function to find a board, input = board and category
 * or board and domain
 */
#define get_board_list_cath(board, cath) filter(BbcMap[(cath)], \
						&operator(==)(board) \
						@ &operator([])(, BBP_BOARD))
#define get_board_cath(board, cath) (get_board_list_cath((board), \
							 (cath)) + ({ 0 }))[0]
#define get_board_list_dom(board, dom) filter(BbdMap[(dom)], \
					      &operator(==)(board) \
					      @ &operator([])(, BBP_BOARD))
#define get_board_dom(board, dom) (get_board_list_dom((board), \
						      (dom)) + ({ 0 }))[0]

/*
 * Prototypes
 */
static nomask object	find_board(string bspath);
static nomask void	update_bbmaps();
public nomask int	sort_dom_boards(string *item1, string *item2);
public nomask int	sort_cath_boards(string *item1, string *item2);
public nomask int	sort_usage_read(mixed item1, mixed item2);
public nomask int	sort_usage_posted(mixed item1, mixed item2);
public nomask int	sort_tusage_read(mixed item1, mixed item2);
public nomask int	sort_tusage_posted(mixed item1, mixed item2);
static nomask mixed	filt_bbp_data(string what, mapping mapp, int index);
static nomask void	print_board_info(string *data);
static nomask void	print_usage_info(mixed data);
static nomask void	reset_usage_info(mixed data);
static nomask void	print_tusage_info(mixed data);
static nomask int	tmfunc(string tm);
static nomask int	try_load_board(string board);
static nomask void	mail_notify(int what, mixed list);
static nomask string	*mk_discard_list(string spath);
static nomask void	load_all_boards(string *list);
static nomask void	logit(string mess);
static nomask void	autosave_mbs();
static nomask void	dosave();
static nomask void	index_help(int cmd);
static nomask void	check_integrity();
nomask static void	debug_out(string str);

/*
 * Function name: create
 * Description:   Initialize the object
 */
public nomask void
create()
{
    string *avail;
    mapping load_map;

    if (IS_CLONE)
    {
	destruct();
	return;
    }

    seteuid(ROOT_UID);

    /*
     * Set some initial values
     */
    AdminList = BASE_ADMIN;
    CategoryMap = BASE_CAT;
    BbpMap = BbcMap = BbdMap = BrokenMap = UnusedMap = ([]);
    HelpMap = ([]);

    restore_object(SAVE_MC);
    check_integrity();

    if (ReportTime == 0)
	ReportTime = time();	/* First update */

    update_bbmaps();

    SaveCount = 1;
    SaveAlarm = set_alarm(300.0, 300.0, autosave_mbs);

    /* Load all boards, but slowly */
    load_map = filter(BbpMap, strlen @ &operator([])(, BBP_BOARD));
    BobMap = ([]);
    load_all_boards(m_indexes(load_map));

    /*
     * Update the broken/unused maps
     * This shouldn't be necessary, strictly speaking, but.. 
     */
    avail = m_indexes(BrokenMap);
    avail = filter(avail, sizeof @ &operator([])(BbpMap, ));
    BrokenMap = mkmapping(avail, map(avail, &operator([])(BrokenMap, )));
    avail = m_indexes(UnusedMap);
    avail = filter(avail, sizeof @ &operator([])(BbpMap, ));
    UnusedMap = mkmapping(avail, map(avail, &operator([])(UnusedMap, )));

    /*
     * Initialize the cache
     */
    set_cache_size(USER_CACHE);
}

/* ************************************************************************
 * MBM functions
 */

/*
 * Function name: query_admin
 * Description:	  Return admin status of individual
 * Arguments:	  who - who to check for
 * Returns:	  1 = admin, 0 = not
 */
public nomask int
query_admin(string who)
{
    if (CALL_CHECK)
	return MBM_BAD_CALL;

    who = UC(who);

    return (member_array(who, AdminList) >= 0);
}

/*
 * Function name: add_admin
 * Description:	  Add a wizard to the list of trusted admins
 * Arguments:	  who - who to add
 * Returns:	  error code, if any
 */
public nomask int
add_admin(string who)
{
    if (CALL_CHECK)
	return MBM_BAD_CALL;

    who = UC(who);

    if (!SECURITY->query_wiz_rank(who))
    {
	MBS->err_args(who);
	return MBM_NO_WIZ;
    }
    else if (query_admin(who))
    {
	MBS->err_args(who);
	return MBM_IS_ADMIN;
    }
    
    AdminList += ({ who });

    write("Added " + who + ".\n");
    logit("Admin add [" + UC(TI->query_real_name()) + "] " + who);

    dosave();
    return MBM_NO_ERR;
}

/*
 * Function name: delete_admin
 * Description:	  Remove a wizard from the list of trusted admins
 * Arguments:	  who - who to remove
 * Returns:	  error code, if any
 */
public nomask int
delete_admin(string who)
{
    if (CALL_CHECK)
	return MBM_BAD_CALL;

    who = UC(who);
    
    if (!query_admin(who))
    {
	MBS->err_args(who);
	return MBM_NO_ADMIN;
    }
    else if (member_array(who, BASE_ADMIN) >= 0)
    {
	MBS->err_args(who);
	return MBM_BASE_ADMIN;
    }
    
    AdminList -= ({ who });
    
    write("Removed " + who + ".\n");
    logit("Admin delete [" + UC(TI->query_real_name()) + "] " + who);
    dosave();
    return MBM_NO_ERR;
}

/*
 * Function name: list_admin
 * Description:   List the admins
 * Arguments:	  admin - admin level access
 * Returns:	  Error code, if any
 */
public nomask int
list_admin(int admin)
{
    string *list;
    int i, sz;
    
    if (CALL_CHECK)
	return MBM_BAD_CALL;

    list = sort_array(map(AdminList, capitalize));
    write("The mbs administrator");
    if ((sz = sizeof(list)) > 1)
	write("s: ");
    else
	write(": ");

    for (i = 0 ; i < sz ; i ++)
    {
	if (!admin || member_array(list[i], BASE_ADMIN) < 0)
	    write(list[i]);
	else
	    write("[" + list[i] + "]");
	if (i < sz - 1)
	    write(", ");
    }

    write("\n");
    
    return MBM_NO_ERR;
}

/*
 * Function name: add_board
 * Description:	  Add a board to the list of boards
 * Arguments:	  data - data about the board: 
 *			 ({ 'name', 'category', 'dirpath', 'description' })
 * Returns:	  Error code, if any
 */
public nomask int
add_board(string *data)
{
    object	board;

    if (CALL_CHECK)
	return MBM_BAD_CALL;

    data[0] = UC(data[0]);
    data[0] = implode(explode(data[0], "."), "");
    data[1] = UC(data[1]);
//    data[3] = UC(data[3]);
    
    /* String lengths */
    if (strlen(data[3]) > DESC_LEN)
    {
	MBS->err_args(data[3]);
	return MBM_STR_LONG;
    }
    if (strlen(data[0]) > NAME_LEN)
    {
	MBS->err_args(data[0]);
	return MBM_STR_LONG;
    }
    /* Is the board registered? */
    if (!sizeof(BbpMap[data[2]]))
    {
	MBS->err_args(data[2]);
	return MBM_BAD_BPATH;
    }
    /* Is the board already in use? */
    if (strlen(BbpMap[data[2]][BBP_BOARD]))
    {
	MBS->err_args(BbpMap[data[2]][BBP_BOARD], BbpMap[data[2]][BBP_CAT]);
	return MBM_BOARD_IN_USE;
    }
    /* Category check */
    if (!strlen(CategoryMap[data[1]]))
    {
	MBS->err_args(data[1]);
	return MBM_NO_CAT;
    }
    /* Does the board exist in that category? */
    if (sizeof(get_board_list_cath(data[0], data[1])))
    {
	MBS->err_args(data[0], data[1]);
	return MBM_BOARD_EXISTS;
    }
    /* Does the board physically exist? */
    board = find_board(data[2]);
    if (!objectp(board))
    {
	MBS->err_args(data[2]);
	return MBM_BAD_BPATH;
    }

    /* Are the priviliges right? */
    if (!query_admin(TI->query_real_name()))
    {
	if (explode(data[2], "/")[2] !=
	    SECURITY->query_wiz_dom(TI->query_real_name()) ||
	    strlen(BASE_CAT[data[1]]))
	    return MBM_NO_AUTH;

	if (sizeof(BbdMap[SECURITY->query_wiz_dom(TI->query_real_name())])
	    >= MAX_NUM_BOARDS)
	    return MBM_NUM_BOARDS;
    }

    /*
     * All demands are met, add it.
     */
    BbpMap[data[2]][BBP_BOARD] = data[0];
    BbpMap[data[2]][BBP_CAT] = data[1];
    BbpMap[data[2]][BBP_DESC] = data[3];

    write("Added the board '" + data[0] + "' to the category '" + data[1] + "'.\n");
    logit("Board add [" + UC(TI->query_real_name()) + "] " + data[0] + ":" + data[1] + ":" + data[3]);
    dosave();
    update_bbmaps();
    return MBM_NO_ERR;
}

/*
 * Function name: remove_board
 * Description:	  Remove a board from the list
 * Arguments:	  board - the board
 *		  cath - the category
 *		  all - the central entry as well?
 * Returns:	  Error code, if any
 */
public nomask int
remove_board(string board, string cath, int all)
{
    string	*bdata, name;
    
    if (CALL_CHECK)
	return MBM_BAD_CALL;

    board = UC(board);
    cath = UC(cath);

    /* Category check */
    if (!strlen(CategoryMap[cath]))
    {
	MBS->err_args(cath);
	return MBM_NO_CAT;
    }
    /* Does the board exist? */
    if (!sizeof((bdata = get_board_cath(board, cath))))
    {
	MBS->err_args(board, cath);
	return MBM_NO_BOARD;
    }

    /* Are the priviliges right? */
    if (!query_admin((name = TI->query_real_name())))
    {
	if (bdata[BBP_DOMAIN] != SECURITY->query_wiz_dom(name))
	    return MBM_NO_AUTH;
    }

    /* Everything checks out, remove the board */

    if (all)
    {
	m_delkey(BbpMap, bdata[BBP_SPATH]);
	write("Removed the central entry '" + bdata[BBP_SPATH] + "'.\n");
    }
    else
    {
	BbpMap[bdata[BBP_SPATH]][BBP_BOARD] = "";
	BbpMap[bdata[BBP_SPATH]][BBP_CAT] = "";
	BbpMap[bdata[BBP_SPATH]][BBP_DESC] = "";
    }

    /*
     * Get rid of possible broken/unused entries, they are only valid for
     * registered boards.
     */
    if (BrokenMap[bdata[BBP_SPATH]])
	m_delkey(BrokenMap, bdata[BBP_SPATH]);
    if (UnusedMap[bdata[BBP_SPATH]])
	m_delkey(UnusedMap, bdata[BBP_SPATH]);

    write("Removed the board '" + board + "' in the category '" + cath + "'.\n"); 
    logit("Board delete [" + UC(TI->query_real_name()) + "] " + board + ":" + cath);
    if (all)
	logit("Central entry delete [" + UC(TI->query_real_name()) + "] " + bdata[BBP_SPATH]);
    GcTime = time();
    update_bbmaps();
    dosave();
    return MBM_NO_ERR;
}

/*
 * Function name: remove_central_entry
 * Description:	  Remove an entry from the central board
 * Arguments:	  entry - the entry to remove
 * Returns:	  Error code, if any
 */
public nomask int
remove_central_entry(string entry)
{
    string name, *bdata;

    if (CALL_CHECK)
	return MBM_BAD_CALL;

    /* Does the board exist? */
    if (!sizeof((bdata = BbpMap[entry])))
    {
	MBS->err_args(entry);
	return MBM_BAD_BPATH;
    }

    /* Is there a registered board */
    if (strlen(bdata[BBP_BOARD]))
    {
	MBS->err_args(bdata[BBP_BOARD], bdata[BBP_CAT]);
	return MBM_ENTRY_USED;
    }

    /* Are the priviliges right? */
    if (!query_admin((name = TI->query_real_name())))
    {
	if (explode(entry, "/")[2] != SECURITY->query_wiz_dom(name))
	    return MBM_NO_AUTH;
    }

    /* All is ok, remove it */
    m_delkey(BbpMap, entry);
    if (BrokenMap[entry])
	m_delkey(BrokenMap, entry);
    if (UnusedMap[entry])
	m_delkey(UnusedMap, entry);
    dosave();
    write("Removed the central entry '" + entry + "'.\n");
    logit("Central entry delete [" +
	  UC(TI->query_real_name()) + "] " + entry);
    return MBM_NO_ERR;
}

/*
 * Function name: rename_board
 * Description:	  Rename a board 
 * Arguments:	  old - the old name
 *		  cath - the category
 *		  new - the new name
 *		  ndesc - the new description, if any
 * Returns:	  Error code, if any
 */
public nomask int
rename_board(string old, string cath, string new, string ndesc)
{
    string	*bdata, name;
    
    if (CALL_CHECK)
	return MBM_BAD_CALL;

    old = UC(old);
    new = UC(new);
    cath = UC(cath);

    /* Category check */
    if (!strlen(CategoryMap[cath]))
    {
	MBS->err_args(cath);
	return MBM_NO_CAT;
    }
    /* Does the new board already exist? */
    if (old != new &&
	sizeof(get_board_cath(new, cath)))
    {
	MBS->err_args(new, cath);
	return MBM_BOARD_EXISTS;
    }
    /* Does the old board exist? */
    if (!sizeof((bdata = get_board_cath(old, cath))))
    {
	MBS->err_args(old, cath);
	return MBM_NO_BOARD;
    }
    /* Check name length */
    if (strlen(new) > NAME_LEN)
    {
	MBS->err_args(new);
	return MBM_STR_LONG;
    }

    /* Are the priviliges right? */
    if (!query_admin((name = TI->query_real_name())))
    {
	if (bdata[BBP_DOMAIN] != SECURITY->query_wiz_dom(name))
	    return MBM_NO_AUTH;
    }

    /* Everything checks out, rename the board */

    if (new != old)
    {
	write("Renamed the board '" + old + "' in the category '" + cath + "' to '" + new + "'.\n");
	BbpMap[bdata[BBP_SPATH]][BBP_BOARD] = new;
	logit("Board rename [" + UC(TI->query_real_name()) + "] " + old + "(" + cath + ") -> " + new);
	GcTime = time();
    }
    if (strlen(ndesc))
    {
	if (strlen(ndesc) > DESC_LEN)
	{
	    MBS->err_args(ndesc);
	    return MBM_STR_LONG;
	}
	else
	{
	    write("Changed the description of the board '" + old + "' in the category '" + cath + "' to '" + ndesc + "'.\n");
	    BbpMap[bdata[BBP_SPATH]][BBP_DESC] = ndesc;
	}
    }

    update_bbmaps();
    dosave();
    return MBM_NO_ERR;
}

/*
 * Function name: list_new_boards
 * Description:	  List new boards recorded in the central board
 * Arguments:	  dom - possible domain spec
 * Returns:	  Error code, if any
 */
public nomask varargs int
list_new_boards(string dom = "")
{
    mapping	boards, disc_map;
    string	*remains, *discard;
    string	name, domain;
    mixed	discard_list;

    /*
     * Find all new boards
     */
    boards = filter(BbpMap, &operator(==)("") @ &operator([])(, BBP_BOARD));

    /*
     * Eliminate bogus and unused boards.
     */
    disc_map = filter(boards, &try_load_board() @ &operator([])(, BBP_RPATH)) +
	filter(boards, &operator(<)(DTS(SCRAP_DELAY)) @ &tmfunc()
	       @ &operator([])(, BBP_LNOTE));

    if (m_sizeof(disc_map))
    {
	discard = sort_array(m_indexes(disc_map));
	write("Removing bogus boards, old or non-loading (" + sizeof(discard) + ")\n");
	write("-----------------------------------------\n");
	map(sort_array(discard), &write() @ &sprintf("%s\n", ));
	discard_list = map(discard, &mk_discard_list());
	mail_notify(M_E_REMOVED, discard_list);
	remains = m_indexes(BbpMap) - discard;
	BbpMap = mkmapping(remains, map(remains, &operator([])(BbpMap, )));
	dosave();
	write("\n");
    }
    boards = filter(BbpMap, &operator(==)("") @ &operator([])(, BBP_BOARD));

    /*
     * Find out which boards are interesting.
     */
    if (!query_admin((name = TI->query_real_name())))
    {
	domain = SECURITY->query_wiz_dom(name);
	boards = filter(boards, &operator(==)(domain)
			@ &operator([])(, BBP_DOMAIN));
    }
    else if (strlen(dom))
    {
	dom = UC(dom);
	if (SECURITY->query_domain_number(dom) < 0)
	{
	    MBS->err_args(dom);
	    return MBM_NO_DOMAIN;
	}
	boards = filter(boards, &operator(==)(dom)
			@ &operator([])(, BBP_DOMAIN));
    }

    if (!m_sizeof(boards))
	write("No new boards registered.\n");
    else
    {
	write("Unattached boards (" + m_sizeof(boards) + ")\n");
	write("-----------------\n");
	map(sort_array(m_indices(boards)), &write() @ &sprintf("%s\n",));
    }

    return MBM_NO_ERR;
}

/*
 * Function name: list_boards
 * Description:	  Produce a complete list of all boards
 * Arguments:	  order - the order of listing
 *		  ov_order - override order info
 *		  admin - Is this an admin call or not
 *		  spec - specific category or domain
 * Returns:	  Error code, if any
 */
public nomask varargs int
list_boards(int order, int admin, string spec)
{
    string	*caths, *doms, st;
    mixed	boards, bds2;
    int		i, j, sz;

    if (CALL_CHECK)
	return MBM_BAD_CALL;

    if (strlen(spec))
    {
	spec = UC(spec);
	if (member_array(spec, SECURITY->query_domain_list()) >= 0)
	    order = ORDER_DOMAIN;
	else if (strlen(CategoryMap[spec]))
	    order = ORDER_CAT;
	else
	{
	    MBS->err_args(spec);
	    return MBM_BAD_ORDER;
	}
    }

    if (!admin)
    {
	caths = sort_array(m_indexes(BbcMap));
	boards = sort_array(BbdMap[SECURITY->query_wiz_dom(TI->query_real_name())], "sort_dom_boards");
	write(sprintf("%-11s%-11s%-11s%-31s%s\n", "Board", "Category", "Domain", "Description & Savepath", "Access"));
	write(sprintf("%-11s%-11s%-11s%-31s%s\n", "-----", "---------", "------", "----------------------", "------"));
	for (i = 0, sz = sizeof(caths) ; i < sz ; i++)
	{
	    bds2 = sort_array(filter(boards, &operator(==)(caths[i])
				     @ &operator([])(, BBP_CAT)), "sort_cat_boards");
	    if (sizeof(bds2))
	    {
		map(bds2, print_board_info);
		write("\n");
	    }
	}
    }
    else if (order == ORDER_CAT)
    {
	if (!strlen(spec))
	{
	    caths = sort_array(m_indexes(BbcMap));
	    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "Board", "Category", "Domain", "Description & Savepath", "Access"));
	    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "-----", "---------", "------", "----------------------", "------"));
	    for (i = 0, sz = sizeof(caths) ; i < sz ; i++)
	    {
		boards = sort_array(BbcMap[caths[i]], "sort_cath_boards");
		if (sizeof(boards))
		{
		    map(boards, print_board_info);
		    write("\n");
		}
	    }
	}
	else
	{
	    if (!strlen(CategoryMap[spec]))
	    {
		MBS->err_args(spec);
		return MBM_NO_CAT;
	    }

	    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "Board", "Category", "Domain", "Description & Savepath", "Access"));
	    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "-----", "---------", "------", "----------------------", "------"));
	    boards = sort_array(BbcMap[spec], "sort_cath_boards");
	    if (sizeof(boards))
	    {
		map(boards, print_board_info);
		write("\n");
	    }
	    else
		write("The category '" + spec + "' has no registered boards.\n");
	}
    }
    else
    {
	if (!strlen(spec))
	{
	    doms = sort_array(m_indexes(BbdMap));
	    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "Board", "Category", "Domain", "Description & Savepath", "Access"));
	    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "-----", "---------", "------", "----------------------", "------"));
	    for (i = 0, sz = sizeof(doms) ; i < sz ; i++)
	    {
		boards = sort_array(BbdMap[doms[i]], "sort_dom_boards");
		if (sizeof(boards))
		{
		    map(boards, print_board_info);
		    write("\n");
		}
	    }
	}
	else
	{
	    if (SECURITY->query_domain_number(spec) < 0)
	    {
		MBS->err_args(spec);
		return MBM_NO_DOMAIN;
	    }
	    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "Board", "Category", "Domain", "Description & Savepath", "Access"));
	    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "-----", "---------", "------", "----------------------", "------"));
	    boards = sort_array(BbdMap[spec], "sort_dom_boards");
	    if (sizeof(boards))
	    {
		map(boards, print_board_info);
		write("\n");
	    }
	    else
		write("The domain '" + spec + "' has no registered boards.\n");
	}
    }
}

/*
 * Function name: add_category
 * Description:	  Add a category
 * Arguments:	  cath - category
 *		  desc - description
 * Returns:	  Error code, if any
 */
public nomask int
add_category(string cath, string desc)
{
    if (CALL_CHECK)
	return MBM_BAD_CALL;

    cath = UC(cath);

    if (strlen(CategoryMap[cath]))
    {
	MBS->err_args(cath);
	return MBM_CAT_EXISTS;
    }
    else if (strlen(cath) > NAME_LEN)
    {
	MBS->err_args(cath);
	return MBM_STR_LONG;
    }
    else if (strlen(desc) > DESC_LEN)
    {
	MBS->err_args(desc);
	return MBM_STR_LONG;
    }

    desc = UC(desc);

    CategoryMap[cath] = desc;
    write("Added category '" + cath + "'.\n");
    logit("Category add [" + UC(TI->query_real_name()) + "] " + cath);
    dosave();
    return MBM_NO_ERR;
}

/*
 * Function name: delete_category
 * Description:	  Delete a category
 * Arguments:	  cath - category to remove
 * Returns:	  Error code, if any
 */
public nomask int
delete_category(string cath)
{
    if (CALL_CHECK)
	return MBM_BAD_CALL;

    cath = UC(cath);

    if (!strlen(CategoryMap[cath]))
    {
	MBS->err_args(cath);
	return MBM_NO_CAT;
    }

    if (strlen(BASE_CAT[cath]))
	return MBM_BASE_CAT;

    if (sizeof(BbcMap[cath]))
    {
	MBS->err_args(cath);
	return MBM_CAT_IN_USE;
    }

    m_delkey(CategoryMap, cath);
    write("Removed category '" + cath + "'.\n");
    logit("Category delete [" + UC(TI->query_real_name()) + "] " + cath);

    GcTime = time();
    dosave();
    return MBM_NO_ERR;
}

/*
 * Function name: rename_category
 * Description:	  rename a category
 * Arguments:	  old - the old name
 *		  new - the new name
 *		  desc - the new ndesc
 * Returns:	  Error code, if any
 */
public nomask int
rename_category(string old, string new, string ndesc)
{
    string	desc;
    mixed	bds;
    int		i, sz;
    
    if (CALL_CHECK)
	return MBM_BAD_CALL;

    old = UC(old);
    new = UC(new);
    
    if (!strlen(CategoryMap[old]))
    {
	MBS->err_args(old);
	return MBM_NO_CAT;
    }
    if (strlen(new) > NAME_LEN)
    {
	MBS->err_args(new);
	return MBM_STR_LONG;
    }
    if (strlen(BASE_CAT[old]))
	return MBM_BASE_CAT;

    desc = CategoryMap[old];
    CategoryMap[new] = strlen(ndesc) ? ndesc : desc;
    if (old != new)
    {
	m_delkey(CategoryMap, old);

	bds = BbcMap[old];
	for (i = 0, sz = sizeof(bds) ; i < sz ; i++)
	    bds[i][BBP_CAT] = new;
	update_bbmaps();
	write("Renamed category '" + old + "' to '" +
	      new + "'.\n");
	logit("Category rename [" + UC(TI->query_real_name()) + "] " + old + " -> " + new);
	GcTime = time();
    }
    else
	write("Changed the description of the category '" + new + "'.\n");

    dosave();
    return MBM_NO_ERR;
}

/*
 * Function name: list_categories
 * Description:	  List the available categories
 * Arguments:	  admin - admin call or not
 * Returns:	  Error code, if any
 */
public nomask int
list_categories(int admin)
{
    string *caths;
    int i, sz;

    if (CALL_CHECK)
	return MBM_BAD_CALL;

    caths = sort_array(m_indexes(CategoryMap));

    if (sizeof(caths))
    {
	write((admin ? " " : "") + sprintf("%-15s%s", "Category", "Description") + "\n");
	write((admin ? " " : "") + sprintf("%-15s%s", "---------", "-----------") + "\n");
	for (i = 0, sz = sizeof(caths) ; i < sz ; i++)
	{
	    if (admin && strlen(BASE_CAT[caths[i]]))
		write(sprintf("*%-15s%s", caths[i],
			      CategoryMap[caths[i]]) + "\n");
	    else
		write((admin ? " " : "") + sprintf("%-15s%s", caths[i],
						   CategoryMap[caths[i]]) +
		      "\n");
	}
    }
    else
	write("No categories available.\n");
    
    return MBM_NO_ERR;
}

/*
 * Function name: tele_to_board
 * Description:	  Teleport the user to a board room
 * Arguments:	  spath - the save path
 * Returns:	  Error code, if any.
 */
public nomask int
tele_to_board(string spath)
{
    if (CALL_CHECK)
	return MBM_BAD_CALL;
    
    if (!sizeof(BbpMap[spath]))
    {
	MBS->err_args(spath);
	return MBM_BAD_BPATH;
    }

    TI->move_living("X", BbpMap[spath][BBP_RPATH], 1);

    return MBM_NO_ERR;
}

/*
 * Function name: generate_report
 * Description:	  Generate a usage report
 * Arguments:	  what - what to list by.
 * Returns:	  Error code, if any
 */
public nomask int
generate_report(int what)
{
    mixed blist;
    
    if (CALL_CHECK)
	return MBM_BAD_CALL;
    
    blist = m_values(BbpMap);
    blist = filter(blist, &operator(!=)(0) @ strlen @
		   &operator([])(, BBP_BOARD));

    switch (what)
    {
    case 0:
    case 1:
        write("Report of usage activity since " + ctime(ReportTime) + ".\n");
        write(sprintf("%-11s%-11s%-11s%-31s%-5s%-5s\n", "Board", "Category", "Domain", "Description", "Post", "Read"));
        write(sprintf("%-11s%-11s%-11s%-31s%-5s%-5s\n", "-----", "---------", "------", "-----------", "----", "----"));
        break;

    case 3:
    case 4:
        write("Report of usage since last reboot.\n");
        write(sprintf("%-11s%-11s%-11s%-31s%-5s%-5s\n", "Board", "Category", "Domain", "Description", "Post", "Read"));
        write(sprintf("%-11s%-11s%-11s%-31s%-5s%-5s\n", "-----", "---------", "------", "-----------", "----", "----"));
        break;

    default:
        break;
    }

    switch (what)
    {
    case 0:
	blist = sort_array(blist, "sort_usage_read");
	break;
	
    case 1:
	blist = sort_array(blist, "sort_usage_posted");
	break;

    case 2:
        ReportTime = time();
	map(blist, reset_usage_info);
	write("All statistics erased.\n");
        return MBM_NO_ERR;
	break;

    case 3:
	blist = sort_array(blist, "sort_tusage_posted");
	break;

    case 4:
	blist = sort_array(blist, "sort_tusage_read");
        break;
	
    default:
        return MBM_NO_ERR;
	break;
    }

    switch (what)
    {
    case 0:
    case 1:
        map(blist, print_usage_info);
        break;

    case 3:
    case 4:
        map(blist, print_tusage_info);
        break;

    default:
        break;
    }

    return MBM_NO_ERR;
}

/* ************************************************************************
 * MBS functions
 */

/*
 * Function name: save_mbs
 * Description:	  Save the mbs data, since the mbs itself can't.
 * Arguments:	  mapp - the data to store
 */
public void
save_mbs(mapping mapp)
{
    string name;
    
    if (CALL_CHECK)
	return;

    name = TI->query_real_name();
    
    save_cache(mapp, SAVE_DIR + name[0..0] + "/" + name);
}

/*
 * Function name: restore_mbs
 * Description:	  Restore the mbs data since the mbs itself can't.
 *		  Implement internal caching, size defined above.
 * Returns:	  The mapping containing data
 */
public mapping
restore_mbs()
{
    string name;
    
    if (CALL_CHECK)
	return ([]);

    name = TI->query_real_name();

    return read_cache(SAVE_DIR + name[0..0] + "/" + name);
}

/*
 * Function name: default_mbs
 * Description  : Restore the default mbs data since mbs itself cannot do so.
 * Returns      : mapping - the mapping containing default data.
 */
public mapping
default_mbs()
{
    if (CALL_CHECK)
	return ([]);

#ifdef MBS_DEFAULT
    return restore_map(MBS_DEFAULT);
#else
    return ([ ]);
#endif MBS_DEFAULT
}

/*
 * Function name: query_board_exist
 * Description:	  Return the board entry if it exists
 * Arguments:	  board - the board.
 *		  item - the category or domain.
 * Returns:	  Board entry.
 */
public nomask mixed
query_board_exist(string board, string item)
{
    if (CALL_CHECK)
	return MBS_BAD_CALL;
    
    if (!strlen(CategoryMap[item]))
	return get_board_dom(board, item);
    else
	return get_board_cath(board, item);
}

/*
 * Function name: query_board_desc
 * Description:	  Return the board description if it exists
 * Arguments:	  bpath - the board save path.
 * Returns:	  Board desc entry.
 */
public nomask string
query_board_desc(string bpath)
{
    mixed entry;

    if (CALL_CHECK)
	return "";

    entry = BbpMap[bpath];

    if (sizeof(entry))
	return entry[BBP_DESC];
    else
	return "<Error, no such board>";
}

/*
 * Function name: query_unread_news
 * Description:	  Check if news is unread on a specific board
 * Arguments:	  bpath - the board save path
 * Returns:	  unread status (binary)
 */
public nomask int
query_unread_news(string bpath, string last)
{
    mixed	entry;
    int		il, ic;

    entry = BbpMap[bpath];

    if (!last || !entry[BBP_LNOTE])
        return 0;
    
    il = atoi(last[1..]);
    ic = atoi(entry[BBP_LNOTE][1..]);

    return (il < ic);
}

/*
 * Function name: query_board_status
 * Description:	  Return the board status
 * Arguments:	  bpath - the board save path.
 *		  last - date of last read note.
 * Returns:	  Board status entry.
 */
public nomask string
query_board_status(string bpath, string last)
{
    mixed	entry;
    int		il, ic;
    string      st;
    object	bd;

    entry = BbpMap[bpath];

    if (sizeof(entry))
    {
	/* Broken? */
	if (BrokenMap[bpath])
	    return "Broken";

	/* Read status? */
	bd = find_board(bpath);
	if (objectp(bd))
	{
	    st = " [";
	    st += bd->check_reader() ? "-" : "r";
	    st += bd->check_writer() ? "-" : "w";
	    st += bd->check_remove() ? "-" : "d";
	    st += "]";

            /* News status? */
	    il = atoi(last[1..]);
            if (entry[BBP_LNOTE])
            {
                ic = atoi(entry[BBP_LNOTE][1..]);
                if (il < ic)
                    return "U" + st;
            }
            return "-" + st;
	}
	else
	    return "B [???]";
    }
    else
	return "<Error, no such board>";
}

/*
 * Function name: query_categories
 * Description:	  Return the list of categories
 * Returns:	  The list
 */
public nomask string *
query_categories()
{
    if (CALL_CHECK)
	return ({});
    
    return m_indexes(CategoryMap);
}

/*
 * Function name: query_boards
 * Description:	  Return a list of active boards
 * Arguments:	  order - order in conjunction with next item
 *		  spec - any special domain/category
 *		  excl - exclusion list
 * Returns:	  A list of boards
 */
public nomask mixed
query_boards(int order, string spec, string *excl)
{
    mapping bmap;
    string *bnames;

    if (CALL_CHECK)
	return MBS_BAD_CALL;

    bmap = filter(BbpMap, strlen @ &operator([])(, 0));
    bnames = m_indexes(bmap);
    bnames -= excl;
    bmap = mkmapping(bnames, map(bnames, &operator([])(BbpMap, )));
    if (strlen(spec))
    {
	switch (order)
	{
	case ORDER_DOMAIN:
	    bmap = filter(bmap, &operator(==)(spec) @ &operator([])(, BBP_DOMAIN));
	    break;

	default:
	    bmap = filter(bmap, &operator(==)(spec) @ &operator([])(, BBP_CAT));
	    break;
	}
    }
    return m_values(bmap);
}

/*
 * Function name: query_board
 * Description:	  Return the info of just one board
 * Arguments:	  board - the board
 *		  cath - the category
 * Returns:	  board, or error.
 */
public nomask mixed
query_board(string board, string cath)
{
    string *blist;
    
    if (CALL_CHECK)
	return MBS_BAD_CALL;

    if (!strlen(CategoryMap[cath]))
	return MBS_BAD_CAT;

    blist = filter(BbcMap, &operator(==)(board) @ &operator([])(, BBP_BOARD));
    if (sizeof(blist))
	return MBS_NO_BOARD;

    return blist;
}

/*
 * Function name: print_headers
 * Description:	  Print the headers of a board
 * Arguments:	  select - what to show
 *		  spath - board save path
 *		  lnote - last read note
 *		  order - order
 *		  oitem - order item
 * Returns:	  Error code, if any
 */
public nomask int
print_headers(int select, string spath, int lnote, string order, string oitem)
{
    mixed	blist, hds;
    int		i, sz, tme;
    string	year;
    object	bd;

    if (CALL_CHECK)
	return MBS_BAD_CALL;

    blist = BbpMap[spath];

    if (BrokenMap[spath])
    {
	write("The board is registered as broken. Trying to reload it.\n");
	load_all_boards(({ spath }));
	if (BrokenMap[spath])
	{
	    write("The board could not be reloaded.\n");
	    return MBS_NO_ERR;
	}
	write("The board was successfully reloaded.\n");
    }

    if (!select && (!blist[BBP_LNOTE] ||
            atoi(blist[BBP_LNOTE][1..]) <= lnote))
    {
	write("No news is good news.\n");
	return MBS_NO_ERR;
    }
    
    bd = find_board(spath);

    hds = bd->query_headers();
    if ((sz = sizeof(hds)))
    {
	write((select ? "Headers" : "Unread headers") + " of the board '" +
	    blist[BBP_BOARD] + "' in the "  + order + " '" + oitem + "'.\n");

	year = (bd->query_show_lvl() ? "yy" : "yyyy");
	for (i = 0 ; i < sz ; i++)
	{
	    tme = atoi(hds[i][1][1..]);
	    if (select || (tme > lnote))
	    {
		write(sprintf("%2d: %s %s\n", (i + 1), hds[i][0],
		    TIME2FORMAT(tme, year)));
	    }
	}
    }
    else
    {
	if (bd->check_reader())
	    return MBS_NO_RACC;
	
	write("No news is good news.\n");
    }

    return MBS_NO_ERR;
}

/*
 * Function name: read_item
 * Description:	  Read an item on a board
 * Arguments:	  board - the board to read
 *		  item - the item to read
 *		  mread - mread or not	
 * Returns:	  ({ Error code, if any, "timestamp" })
 */
nomask public mixed
read_item(string board, int item, int mread)
{
    object	bd;
    mixed	*hds;

    if (CALL_CHECK)
	return ({ MBS_BAD_CALL, "" });

    if (!objectp(bd = find_board(board)))
	return ({ MBS_BAD_BOARD, "" });

    if (bd->check_reader())
	return ({ MBS_NO_RACC, "" });

    hds = bd->query_headers();

    if (item > sizeof(hds))
	return ({ MBS_BAD_NNUM, "" });

    write("Reading note " + item + " on the board '" + BbpMap[board][BBP_BOARD] + "' in the category '" + BbpMap[board][BBP_CAT] + "'.\n");
    bd->read_msg("" + item, mread);

    return ({ MBS_NO_ERR, hds[item - 1][1] });
}

/*
 * Function name: find_next_unread
 * Description:	  Find the next unread article on a board, given time info
 * Arguments:	  board - the board to check
 *		  tme - time stamp to check from
 * Returns:	  ({ item, "time stamp" })
 */
nomask public mixed
find_next_unread(string board, int tme)
{
    mixed	*hds;
    object	bd;
    int		i, sz;

    if (CALL_CHECK)
	return ({ -1, MBS_BAD_CALL });

    if (!objectp(bd = find_board(board)))
	return ({ -1, MBS_BAD_BOARD });

    if (bd->check_reader())
	return ({ -1, MBS_NO_RACC });

    hds = bd->query_headers();
    
    for (i = 0, sz = sizeof(hds) ; i < sz ; i++)
    {
	if (tme < atoi(hds[i][1][1..]))
	    return ({ i + 1, hds[i][1] });
    }
    
    return ({ 0, "" });
}

/*
 * Function name: post_item
 * Description:	  Post an item on a board.
 * Arguments:	  board - the board
 *		  subj - the subject
 * Returns:	  Error code, if any
 */
nomask public int
post_item(string board, string subj)
{
    object bd;

    if (CALL_CHECK)
	return MBS_BAD_CALL;
    
    bd = find_board(board);
    if (!objectp(bd))
	return MBS_BAD_BOARD;

    if (bd->check_writer())
	return MBS_NO_WACC;
    else
	bd->new_msg(subj);

    return MBS_NO_ERR;
}


/*
 * Function name: remove_item
 * Description:	  Remove an item on a board
 * Arguments:	  board - the board
 *		  item - the number of the item to delete
 * Returns:	  Error code, if any
 */
nomask public int
remove_item(string board, string item)
{
    mixed hds;
    object bd;
    int i, cnt;

    if (CALL_CHECK)
	return MBS_BAD_CALL;

    bd = find_board(board);
    if (!objectp(bd))
	return MBS_BAD_BOARD;

    if (bd->check_remove())
	return MBS_NO_DACC;
    else
    {
	hds = bd->query_headers();
	if (sizeof(hds) < atoi(item))
	    return MBS_BAD_NNUM;
	bd->remove_msg("note " + item);
    }
    return MBS_NO_ERR;
}

/* ************************************************************************
 * MBH functions
 */

/*
 * Function name: do_help
 * Description:	  Perform help service for the mbs
 * Arguments:	  cmd - the command to get help for.
 * Returns:	  Error code, if any
 */
nomask public int
do_help(string cmd)
{
    int line, mlen;

    if (CALL_CHECK)
	return MBS_BAD_CALL;

    if (IndexLine != 0)
    {
	write("Automatic indexing of help file in progress... please try later.\n" + "Indexing at line " + IndexLine + " of the help file.\n");
	return MBS_NO_ERR;
    }

    if (file_time(HELP_FILE) > file_time(SAVE_MC + ".o"))
    {
	if (IndexLine == 0)
	{
	    index_help(1);
	    write("Automatic indexing of help file in progress... please try later.\n" + "Indexing at line " + IndexLine + " of the help file.\n");
	    return MBS_NO_ERR;
	}
    }

    switch (cmd)
    {
    	case 0:
    	case "":
            TI->more(read_file(HELP_FILE, HelpMap["mbh"], HelpMap["mbh_len"]));
	    break;
    
    	case "list":
	    write("Available MBS commands: mbs, mbm and mbh.\n");
	    break;
    
    	default:
	    if (!(line = HelpMap[cmd]))
		write("That command does not exist in the mbs helpfile.\n");
	    else
                TI->more(read_file(HELP_FILE, line, HelpMap[cmd + "_len"]));

	    break;
    }

    return MBS_NO_ERR;
}

/* ************************************************************************
 * Service functions
 */

/*
 * Function name: new_note
 * Description:	  This function is called from the board when a new note
 *		  is added to a board and the board is reporting 
 * Arguments:	  save_path - The place where the board stores notes
 *		  last_note - The name of the new note
 *		  room_path - The path to the room of the board
 */
public nomask void
new_note(string save_path, string last_note, string room_path)
{
    if (!sizeof(BbpMap[save_path]))
	BbpMap[save_path] = ({ "", "", explode(save_path, "/")[2], "",
				save_path, room_path, last_note, 1, 0 });
    else
    {
	BbpMap[save_path][BBP_LNOTE] = last_note;
	BbpMap[save_path][BBP_PNOTE]++;
    }

    if (room_path != BbpMap[save_path][BBP_RPATH])
	BbpMap[save_path][BBP_RPATH] = room_path;

    dosave();
}

/*
 * Function name: read_note
 * Description:	  This function is called from the board when a note
 *		  is read to update the internal accounting.
 * Arguments:	  save_path - the place where the board stores notes.
 */
public nomask void
read_note(string save_path)
{
    if (!sizeof(BbpMap[save_path]))
	return;

    BbpMap[save_path][BBP_RNOTE] += 1;

    if (!(SaveCount++ % 100))
	dosave();
}

/*
 * Function name: remove_note
 * Description:	  This function is called from the board when a note
 *		  is read to update the internal accounting.
 * Arguments:	  save_path - the place where the board stores notes.
 */
public nomask void
remove_note(string save_path)
{
    object board;

    if (!sizeof(BbpMap[save_path]))
	return;

    board = previous_object();
    if (function_exists("create_object", board) != BOARD_OBJECT)
        return;

    BbpMap[save_path][BBP_LNOTE] = board->query_latest_note();
    BbpMap[save_path][BBP_PNOTE] -= 1;

    if (!(SaveCount++ % 100))
	dosave();
}

/*
 * Function name: dosave
 * Description:	  Save the variables and reset the autosave counter
 */
static nomask void
dosave()
{
    autosave_mbs();
    remove_alarm(SaveAlarm);
    SaveAlarm = set_alarm(300.0, 300.0, autosave_mbs);
}

/*
 * Function name: autosave_mbs
 * Description:	  Autosave the mbs contents
 */
static nomask void
autosave_mbs()
{
    save_object(SAVE_MC);
    SaveCount = 1;
}

/*
 * Function name: index_help
 * Description:	  Perform automatic indexing of the help file.
 *		  Lines starting with the string "#ENTRY <index>"
 *		  are taken to start a paragraph, with that index.
 *		  All lines before the first index are discarded.
 * Arguments:	  cmd - The indexing command.
 */
static nomask void
index_help(int cmd)
{
    int i;
    string line, name;
    
    /*
     * Start the process.
     */
    if (cmd == 1)
    {
	HelpMap = ([]);
	IndexLine = 0;
	CountLine = 0;
	HelpCmdName = "";
    }

    HelpAlarmId = set_alarm(2.0, 0.0, &index_help(0));

    /*
     * Read 50 lines at a time.
     */
    for (i = 0 ; i < 50 ; i++)
    {
	line = read_file(HELP_FILE, IndexLine, 1);
	if (!strlen(line))
	{
	    HelpMap += ([ HelpCmdName + "_len" : CountLine ]);

	    save_object(SAVE_MC);

	    remove_alarm(HelpAlarmId);
	    HelpAlarmId = 0;
	    IndexLine = 0;
	    return;
	}
	
	if (sscanf(line, "#ENTRY %s\n", name) == 1)
	{
	    if (CountLine > 0)
	    {
		HelpMap += ([ HelpCmdName + "_len" : CountLine ]);
		CountLine = 0;
	    }

	    HelpCmdName = name;
	    HelpMap += ([ HelpCmdName : IndexLine + 1]);
	}
	else
	    if (HelpCmdName != "")
		CountLine++;

	IndexLine++;
    }
}

/*
 * Function name: logit
 * Description:	  Log an event in a central log.
 * Arguments:     mess - message to log
 */
static nomask void
logit(string mess)
{
    log_file(MBSLOG, ctime(time()) + ": " + mess + "\n");
}

/*
 * Function name: query_board_access
 * Description:	  Return the access string of a particular board.
 * Arguments:	  bspath - board storage path
 * Returns:	  The access string
 */
public nomask string
query_board_access(string bspath)
{
    string st;
    object bd;

    if (BrokenMap[bspath])
	return "Broken";

    if (objectp((bd = find_board(bspath))))
    {
	st = "[";
	st += bd->check_reader() ? "-" : "r";
	st += bd->check_writer() ? "-" : "w";
	st += bd->check_remove() ? "-" : "d";
	st += "]";
    }
    else
	st = "[???]";

    return st;
}

/*
 * Function name: find_board
 * Description:	  Find a board in the central board handler, return
 *		  its object pointer.
 * Arguments:	  bspath - board storage path
 * Returns:	  The object pointer to the board, if any
 */
static nomask object
find_board(string bspath)
{
    string broom;
    object *obs;
    int i;

    if (!mappingp(BobMap))
	BobMap = ([]);

    if (objectp(BobMap[bspath]))
	return BobMap[bspath];
    
    broom = BbpMap[bspath][BBP_RPATH];

    if (LOAD_ERR(broom))
	return 0;

    obs = all_inventory(find_object(broom));

    if ((i = member_array(bspath, obs->query_board_name())) >= 0)
    {
	BobMap[bspath] = obs[i];
	return obs[i];
    }

    return 0;
}

/*
 * Function name: get_board_by_path
 * Description:	  Get the board info by savepath
 * Arguments:	  savep - the path
 * Returns:	  The board array
 */
public nomask mixed
get_board_by_path(string savep)
{
    if (CALL_CHECK)
	return 0;
    
    return BbpMap[savep];
}

/*
 * Function name: check_integrity
 * Description:   Check the integrity of the saved values
 */
nomask static void
check_integrity()
{
    BbpMap = filter(BbpMap, pointerp);
}

/*
 * Function name: update_bbmaps
 * Description:	  Updates the board by domain and path map from the board by
 *		  category map.
 */
static nomask void
update_bbmaps()
{
    string *caths, *doms;
    mixed cmap, dmap, entry_map;

    caths = m_indexes(CategoryMap);
    cmap = map(caths, &filt_bbp_data(, BbpMap, BBP_CAT));
    BbcMap = mkmapping(caths, cmap);

    doms = SECURITY->query_domain_list();
    entry_map = filter(BbpMap, strlen @ &operator([])(, 0));
    dmap = map(doms, &filt_bbp_data(, entry_map, BBP_DOMAIN));
    BbdMap = mkmapping(doms, dmap);
}

/*
 * Function name: sort_cath_boards
 * Description:	  Sort function for category listings of boards
 * Arguments:	  As per quicksort.
 * Returns:	  As per quicksort. 
 */
public nomask int
sort_cath_boards(string *item1, string *item2)
{
    string it1, it2;

    it1 = item1[BBP_DOMAIN] + item1[BBP_BOARD];
    it2 = item2[BBP_DOMAIN] + item2[BBP_BOARD];
    if (it1 < it2)
	return -1;
    if (it1 > it2)
	return 1;
    return 0;
}

/*
 * Function name: sort_dom_boards
 * Description:	  Sort function for domain listings of boards
 * Arguments:	  As per quicksort.
 * Returns:	  As per quicksort. 
 */
public nomask int
sort_dom_boards(string *item1, string *item2)
{
    string it1, it2;

    it1 = item1[BBP_CAT] + item1[BBP_BOARD];
    it2 = item2[BBP_CAT] + item2[BBP_BOARD];
    if (it1 < it2)
	return -1;
    if (it1 > it2)
	return 1;
    return 0;
}

/*
 * Function name: sort_usage_read
 * Description:	  Sort function for usage listings of boards
 * Arguments:	  As per quicksort.
 * Returns:	  As per quicksort. 
 */
public nomask int
sort_usage_read(mixed item1, mixed item2)
{
    int it1, it2;

    it1 = item1[BBP_RNOTE];
    it2 = item2[BBP_RNOTE];
    if (it1 < it2)
	return 1;
    if (it1 > it2)
	return -1;
    return 0;
}

/*
 * Function name: sort_usage_posted
 * Description:	  Sort function for usage listings of boards
 * Arguments:	  As per quicksort.
 * Returns:	  As per quicksort. 
 */
public nomask int
sort_usage_posted(mixed item1, mixed item2)
{
    int it1, it2;

    it1 = item1[BBP_PNOTE];
    it2 = item2[BBP_PNOTE];
    if (it1 < it2)
	return 1;
    if (it1 > it2)
	return -1;
    return 0;
}

/*
 * Function name: sort_tusage_read
 * Description:	  Sort function for today's usage listings of boards
 * Arguments:	  As per quicksort.
 * Returns:	  As per quicksort. 
 */
public nomask int
sort_tusage_read(mixed item1, mixed item2)
{
    int it1, it2;
    object bd1, bd2;
    int *st;

    bd1 = find_board(item1[BBP_SPATH]);
    bd2 = find_board(item2[BBP_SPATH]);
    if (!objectp(bd1) || !objectp(bd2))
        return 0;

    it1 = (bd1->query_stats())[0];
    it2 = (bd2->query_stats())[0];

    if (it1 < it2)
	return 1;
    if (it1 > it2)
	return -1;
    return 0;
}

/*
 * Function name: sort_tusage_posted
 * Description:	  Sort function for today's usage listings of boards
 * Arguments:	  As per quicksort.
 * Returns:	  As per quicksort. 
 */
public nomask int
sort_tusage_posted(mixed item1, mixed item2)
{
    int it1, it2;
    object bd1, bd2;
    int *st;

    bd1 = find_board(item1[BBP_SPATH]);
    bd2 = find_board(item2[BBP_SPATH]);
    if (!objectp(bd1) || !objectp(bd2))
        return 0;

    it1 = (bd1->query_stats())[1];
    it2 = (bd2->query_stats())[1];

    if (it1 < it2)
	return 1;
    if (it1 > it2)
	return -1;
    return 0;
}

/*
 * Function name: mail_notify
 * Description:	  Notify people by mail about actions in the mbs
 * Arguments:	  what - what kind of notification
 *		  list - the list of notifications
 */
static nomask void
mail_notify(int what, mixed list)
{
    string dom, lord, recipient, message;

    dom = explode(list[0][0], "/")[2];
    if (member_array(dom, NO_LORD_DOMAIN) >= 0)
    {
	recipient = implode(AdminList, ",");
	message = "IN LOCO PARENTIS:\n\n" + message;
    }
    else
    {
	lord = SECURITY->query_domain_lord(dom);
	recipient = strlen(lord) ? lord : dom;
    }

    switch (what)
    {
    case M_E_REMOVED:
	message = break_string("The board in the room '" + list[0][1] + "' storing messages in the directory '" + list[0][0] + "' has been removed from the MBS central board register due to inactivity. There has been no postings on that board in over " + SCRAP_DELAY + " days. Either reactivate it by posting on it and attach it to the MBS, or simply remove the code and the existing messages.", 70) + "\n";
	break;
	
    case M_B_UNUSED:
	message = break_string("The board in the room '" + list[0][1] + "' storing messages in the directory '" + list[0][0] + "' has not recieved any postings in " + WARN_DELAY + " days. In 10 days the board will be removed entirely from the MBS unless a posting is made before then. Please consider its use and either post on it, or remove it entirely from the game and the MBS.", 70)  + "\n";
	break;
	
    case M_B_UN_REMOVED:
	message = break_string("The board '" + list[0][1] + "' in the category '" + list[0][2] + "' storing messages in the directory '" + list[0][0] + "' has been removed from the MBS central board register due to inactivity. There has been no postings on that board in " + SCRAP_DELAY + " days. Please either remove the board code and the stored messages, or reactivate it by posting on it and re-attach it to the MBS.", 70) + "\n";
	break;

    case M_B_BROKEN:
	message = break_string("The board '" + list[0][1] + "' in the category '" + list[0][2] + "' storing messages in the directory '" + list[0][0] + "' is broken, or possibly the room holding it. Please repair it immediately or remove the code and the stored messages along with the entry in the MBS. It will be removed automatically from the MBS in " + REMOVE_DELAY + " days unless it starts to function again.", 70) + "\n";
	break;

    case M_B_BR_REMOVED:
	message = break_string("The board '" + list[0][1] + "' in the category '" + list[0][2] + "' storing messages in the directory '" + list[0][0] + "' has been broken for " + REMOVE_DELAY + " days after the previous warning, and is now removed from the MBS central board register. Please either remove the code and the stored messages or repair it an re-attach it to the MBS.", 70) + "\n";
	break;
    }

    // CREATE_MAIL(subject, author, to, cc, body)
    CREATE_MAIL("MBS missive", "MBS central", lower_case(recipient), "", message);
    
    list = list[1..];
    if (sizeof(list))
	set_alarm(5.0, 0.0, &mail_notify(what, list));
}

/*
 * Function name: filt_bbp_data
 * Description:	  Filter chosen bbp data
 * Arguments:	  what - what to look for
 *		  mapp - which map to look in
 *		  index - the index for that item
 * Returns:	  The data for the found item
 */
static nomask mixed
filt_bbp_data(string what, mapping mapp, int ind)
{
    mixed data;
    data = filter(mapp, &operator(==)(what) @ &operator([])(, ind));

    return m_values(data);
}

/*
 * Function name: print_board_info
 * Description:   Print board info
 * Arguments:	  data - the data holder
 */
static nomask void
print_board_info(string *data)
{
    string st;
    object bd;

    if (BrokenMap[data[BBP_SPATH]])
	st = "Broken";
    else if (objectp((bd = find_board(data[BBP_SPATH]))))
    {
	st = "[";
	st += bd->check_reader() ? "-" : "r";
	st += bd->check_writer() ? "-" : "w";
	st += bd->check_remove() ? "-" : "d";
	st += "]";
    }
    else
	st = "[???]";

    write(sprintf("%-11s%-11s%-11s%-31s%s\n%24s%s\n", data[BBP_BOARD], data[BBP_CAT], data[BBP_DOMAIN], data[BBP_DESC], st, "", data[BBP_SPATH]));
}

/*
 * Function name: print_usage_info
 * Description:   Print usage info
 * Arguments:	  data - the data holder
 */
static nomask void
print_usage_info(mixed data)
{
    write(sprintf("%-11s%-11s%-11s%-31s%-5d%-5d\n", data[BBP_BOARD], data[BBP_CAT], data[BBP_DOMAIN], data[BBP_DESC], data[BBP_PNOTE], data[BBP_RNOTE]));
}

/*
 * Function name: reset_usage_info
 * Description:   Reset usage info
 * Arguments:	  data - the data holder
 */
static nomask void
reset_usage_info(mixed data)
{
    data[BBP_PNOTE] = 0;
    data[BBP_RNOTE] = 0;
}

/*
 * Function name: print_tusage_info
 * Description:   Print usage info of today's usage
 * Arguments:	  data - the data holder
 */
static nomask void
print_tusage_info(mixed data)
{
    object bd = find_board(data[BBP_SPATH]);
    int *st;

    if (!objectp(bd))
        return;

    st = bd->query_stats();

    write(sprintf("%-11s%-11s%-11s%-31s%-5d%-5d\n", data[BBP_BOARD], data[BBP_CAT], data[BBP_DOMAIN], data[BBP_DESC], st[1], st[0]));
}

/*
 * Function name: load_all_boards
 * Description:	  Try to load all the boards in the given list, however
 *		  do it slowly as not to break the game.
 * Arguments:	  list - the list of boards to load.
 */
static nomask void
load_all_boards(string *list)
{
    if (!sizeof(list))
	return;
    
    if (!objectp(find_board(list[0])))
    {
	/* If the board is broken, warn */
	if (!BrokenMap[list[0]])
	{
	    mail_notify(M_B_BROKEN, ({ ({ list[0],
					  BbpMap[list[0]][BBP_BOARD],
					  BbpMap[list[0]][BBP_CAT] }) }));
	    BrokenMap[list[0]] = time();
	    dosave();
	}
	/* If the board is broken for > REMOVE_DELAY, remove it */
	else if ((time() - BrokenMap[list[0]]) > DTS(REMOVE_DELAY))
	{
	    mail_notify(M_B_BR_REMOVED, ({ ({ list[0],
					      BbpMap[list[0]][BBP_BOARD],
					      BbpMap[list[0]][BBP_CAT] })
				   }));
	    m_delkey(BrokenMap, list[0]);
	    GcTime = time();
	    logit("Board delete broken [Auto] " + BbpMap[list[0]][BBP_BOARD] + ":" + BbpMap[list[0]][BBP_CAT]);
	    m_delkey(BbpMap, list[0]);
	    dosave();
	}
    }
    else
    {
	/* Make sure it's not on the broken list */
	if (BrokenMap[list[0]])
	    m_delkey(BrokenMap, list[0]);
	
	/* Check if the board is badly used */
/* There's no reason to remove slow boards anymore. Mercade */
#if 0
	if (tmfunc(BbpMap[list[0]][BBP_LNOTE]) > DTS(WARN_DELAY) &&
	    !strlen(BASE_CAT[BbpMap[list[0]][BBP_CAT]]))
	{
	    /* Warn if not already there */
	    if (!UnusedMap[list[0]])
	    {
		mail_notify(M_B_UNUSED, ({ ({ list[0],
					      BbpMap[list[0]][BBP_RPATH] })
				   }));
		UnusedMap[list[0]] = time();
		dosave();
	    }
	    else if ((time() - UnusedMap[list[0]]) > DTS(REMOVE_DELAY))
	    {
		mail_notify(M_B_UN_REMOVED, ({ ({ list[0],
					          BbpMap[list[0]][BBP_BOARD],
					          BbpMap[list[0]][BBP_CAT] })
				       }));
		m_delkey(UnusedMap, list[0]);
		GcTime = time();
		logit("Board delete idle [Auto] " + BbpMap[list[0]][BBP_BOARD] + ":" + BbpMap[list[0]][BBP_CAT]);
		m_delkey(BbpMap, list[0]);
		dosave();
	    }
	}
	else
#endif 0
/* End of removal of code related to deleting idle boards. Mercade */
	{
	    /* Make sure it's not on the unused list */
	    if (UnusedMap[list[0]])
		m_delkey(UnusedMap, list[0]);
	}
    }
    
    list = list[1..];
    if (sizeof(list))
	set_alarm(2.0, 0.0, &load_all_boards(list));
}

/*
 * Function name: try_load_board
 * Description:   Try to load a board, return 0 or the length of the error.
 * Arguments:	  board - the board to load
 * Returns:	  0 = no error, anything else is a failure
 */
static nomask int
try_load_board(string board)
{
    return strlen(LOAD_ERR(board));
}

/*
 * Function name: mk_discard_list
 * Description:	  Help function to create a list of discarded boards
 * Arguments:	  spath - the board to discard
 * Returns:	  ({ spath, rpath })
 */
static nomask string *
mk_discard_list(string spath)
{
    logit("Central entry delete [Auto] " + spath);
    return ({ spath, BbpMap[spath][BBP_RPATH] });
}

/*
 * Function name: tmfunc
 * Description:	  This help function calculates the age in seconds of a
 *		  posting on a board
 * Arguments:	  tm - the posting name
 * Returns:	  The time number
 */
static nomask int
tmfunc(string tm)
{
    if (strlen(tm))
	return time() - atoi(tm[1..]);
    else
	return 0;
}

/*
 * Function name: query_gc_time
 * Description:	  Get time of last global change
 * Returns:	  GcTime
 */
public nomask int
query_gc_time()
{
    if (CALL_CHECK)
	return 0;

    return GcTime;
}

/*
 * Function name: remove_object
 * Description  : Just before we die ... save.
 */
public nomask void
remove_object()
{
    dosave();
    destruct();
}

/*
 * Function name: query_prevent_shadow
 * Description:	  Disallows shadowing of this object.
 * Returns:	  int 1 - always.
 */
public nomask int
query_prevent_shadow()
{
    return 1;
}

nomask static void
debug_out(string str)
{
    object ob;

    if (objectp((ob = find_player("mrpr"))) && ob == TI)
	tell_object(ob, str);
}

/*
 * Function name: 
 * Description:
 * Arguments:
 * Returns:
 */

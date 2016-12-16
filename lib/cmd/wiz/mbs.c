/*
 * mbs.c
 *
 * Board reading soul.
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/cmd/std/command_driver";

#include <mbs.h>

/* Some nifty defines I tend to use */
#define NF(message)     notify_fail(message)
#define TP              this_player()
#define TI              this_interactive()
#define TO              this_object()
#define PO              previous_object()
#define LC(str)         lower_case((str))
#define UC(str)         capitalize(lower_case((str)))

/*
 * Variables to be saved.
 */
int		Selection;		// The sort/search selection
mapping		SbMap,			// Subscribed board map by selection
		BdMap;			// Subscribed board map by spath
string		*Groups,		// Privately defined groups
    		*CurrItem,		// The selected cath/dom/group
		CurrBoard,		// The currently selected board
	        *ScrapList;		// The list of scrapped boards
int		GcCheck;		// Last time GC was checked.

/*
 * BdMap: ([ "save path" : 
 *		({ "cath", "domain", "group", "board",
 *			"save path", "last read note"}) ])
 * 
 * SbMap : ([ "selection item" :
 *		({ List of above lists }) ])
 */

/*
 * Non-stored globals
 */
static string	*ErrArgs;		// Error arguments
static string	*NoNews;		// No news messages 
static int	NoNewsNum;		// The number of messages
/*
 * Some prototypes.
 */
static nomask int	mbm_error(int what);
static nomask int	mbs_error(int what);
static nomask void	restore_mbs();
static nomask void	save_mbs();
static nomask mixed	filt_bdata(string sort_item, mixed list, int ind);
static nomask void	shuffle_boards();
static nomask int	set_selection(string selection);
static nomask int	list_boards(int which, string rest);
static nomask void	print_newboard_info(string *data);
static nomask void	print_shortsub_info(mixed data);
static nomask void	print_subboard_info(string *data);
static nomask int	subscribe(string board, string cath, string group);
static nomask int	unsubscribe(string board, string cath);
static nomask int	add_newsgroup(string grp);
static nomask int	delete_newsgroup(string grp);
static nomask int	list_newsgroups();
static nomask int	rename_newsgroup(string old, string new);
static nomask int	select_selection_item(string item);
static nomask void	list_subscribed(int unread, int all);
static nomask int	filt_unread_news(mixed list);
static nomask int	read_nur(string arg1, string arg2, int mread);
static nomask int	catch_up(int what, int uncatch);
static nomask void	set_time(string tm, string spath);
static nomask int	select_board(int what, int headers,
				     string board, string item);
public nomask int	sort_dom_boards(string *item1, string *item2);
public nomask int	sort_cath_boards(string *item1, string *item2);
static nomask int	subscribe_sel(string sel, string group);
static nomask int	tele_to_board(string board, string sel);
static nomask int	unsubscribe_sel(string sel);
static nomask int	post_article(string header);
static nomask int	delete_article(string num);
static nomask int	list_scrap();
static nomask int	scrap_board(string board, string item);
static nomask void	check_gc();
nomask static void	debug_out(string str);
static nomask int	select_nur(int sw);
static nomask mixed	find_nur_insel(int sw);
public nomask varargs void
			err_args(string arg1, string arg2, string arg3,
				 string arg4, string arg5);

// 50 % ordinary messages
#define ONM "No news is good news!\n"
#define NO_NEWS (random(2) > 0 ? ONM : NoNews[random(NoNewsNum)])
/*
 * Initalize some stuff
News/
 */
void
create()
{
    NoNews = ({
		"Luchshaya novost' - otsutstvie novostey!\n",
		"Hadashot tovot she-ein hadashot!\n",
		"Inga nyheter �r bra nyheter!\n",
		"No notizie, buone notizie!\n",
		"Nuuzu nai wa ii nuuzu desu yo!\n",
		"Good news: NO news!\n",
		"This space for hire. Mail /dev/null.\n",
		"Oh wow! No new articles!\n",
		"Oops! We seem to have run out of articles for you.\n",
		"This message was examined by Inspector No " + (random(5) + 1) + "-" + (random(30) + 1) + ".\n",
		"Do something unusual today. Write some code!\n",
		"Sorry, but your lucky message has been removed.\n",
		"This is a beta version of this article.\n",
		"This is your article.\n",
		"And thus it was written: 'Thou shalt not read more news.'\n",
		"## MBS ERROR: Out of pixel ink, refill container.\n",
		"## MBS ERROR: News quota exceeded. Try again next " + ({ "Mon", "Tues", "Wednes", "Thurs", "Fri", "Satur", "Sun" })[random(7)] + "day.\n",
		"## MUSE ERROR: Out of inspiration, think up something for yourself.\n",
		"## Arch alert, you are being snooped! Try to look productive for a while!\n",
		"Newsoholics anonymous! Meeting today behind the church in Sparkle at " + (random(12) + 1) + " pm.\n",
		"Relax, take a beer, put your feet up: No news today.\n",
		"Sorry, your favourite pastime is exhausted; No news.\n",
		"To read, or not to read news? I think not.\n",
		"Confucius, he say: When news is finished, so is reading.\n",
		"It's possible to have fun without news, you know?\n",
		"WHAT HAVE YOU DONE? All the news is DELETED! Go redeem yourself!\n",
		"Argh! Dropped all the news into the sewer. Sorry.\n",
		"No news is ... well, no news.\n",
		"Temporary news availability error.\n",
		"Use \"mbs p\". Only then you can read some more news.\n",
		"We apologize for the inconvenience: no news.\n",
		});
    NoNewsNum = sizeof(NoNews);
}

/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
public nomask string
get_soul_id()
{
    return "Mrpr's board soul";
}

/* **************************************************************************
 * This is a command soul.
 */
public nomask int
query_cmd_soul() 
{ 
    return 1; 
}

/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
public nomask mapping
query_cmdlist()
{
    return ([
	     "mbs" : "mbs_cmd",
	     "mbh" : "mbh_cmd",
	     "mbm" : "mbm_cmd"
	     ]);
}

/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the 
 * same order as in the function name list.
 */

/* **************************************************************************
 * mbm functions
 */

/* **************************************************************************
 * mbm - The global mbm command, management
 */
public nomask int
mbm_cmd(string arg)
{
    string	*args, first, rest;
    int		admin, lvl;

    setuid();
    seteuid(getuid());

    admin = MC->query_admin(TI->query_real_name());
    lvl = WIZ_CHECK;
    
    if (!strlen(arg))
    {
	arg = "";
	args = ({});
    }
    else
    {
	args = explode(arg, " ");
	arg = args[0];
	if (sizeof(args) > 1)
	{
	    args = args[1..];
	    first = args[0];
	    rest = implode(args[1..], " ");
	}
	else
	{
	    args = ({});
	    first = rest = "";
	}
    }

    switch (arg)
    {

    /*
     * Board admin
     */
    case "a": /* Add board */
	LORD_CHECK(admin, lvl)
        MBM_ARG_CHECK(args, 4);
	args[3] = implode(args[3..], " ");
	mbm_error(MC->add_board(args));
	break;

    case "d": /* Delete board */
	LORD_CHECK(admin, lvl)
	MBM_ARG_CHECK(args, 2)
	mbm_error(MC->remove_board(first, args[1], 0));
	break;

    case "dD": /* Delete board and savepath entry */
	LORD_CHECK(admin, lvl)
	MBM_ARG_CHECK(args, 2)
	mbm_error(MC->remove_board(first, args[1], 1));
	break;

    case "D": /* Delete savepath entry */
	LORD_CHECK(admin, lvl)
	MBM_ARG_CHECK(args, 1)
	mbm_error(MC->remove_central_entry(first));
	break;

    case "":
    case "l": /* List new */
	LORD_CHECK(admin, lvl)
	mbm_error(MC->list_new_boards(first));
	break;

    case "L": /* List old */
	LORD_CHECK(admin, lvl)
	if (admin)
	    mbm_error(MC->list_boards(Selection, 1, first));
	else
	    mbm_error(MC->list_boards(Selection, 0, ""));
	break;
	
    case "r": /* Rename board */
	LORD_CHECK(admin, lvl)
	MBM_ARG_CHECK(args, 3)
	mbm_error(MC->rename_board(first, args[1], args[2],
				   implode(args[3..], " ")));
	break;

    case "t": /* Teleport to board by savepath */
	LORD_CHECK(admin, lvl)
	MBM_ARG_CHECK(args, 1)
	mbm_error(MC->tele_to_board(first));
	break;

    /*
     * Category admin
     */
    case "ac": /* Add a catergory */
	ADM_CHECK(admin)
	MBM_ARG_CHECK(args, 2)
	mbm_error(MC->add_category(first, rest));
	break;

    case "dc": /* Delete a catergory */
	ADM_CHECK(admin)
	MBM_ARG_CHECK(args, 1)
	mbm_error(MC->delete_category(first));
	break;

    case "lc": /* List all categories */
	LORD_CHECK(admin, lvl)
	mbm_error(MC->list_categories(1));
	break;
	
    case "rc": /* Rename a category */
	ADM_CHECK(admin)
	MBM_ARG_CHECK(args, 2)
	args += ({ "" });
	mbm_error(MC->rename_category(first, args[1],
				      implode(args[2..], " ")));
	break;

    /*
     * Admin list admin
     */
    case "aa": /* Add a board admin */
	ADM_CHECK(admin)
	MBM_ARG_CHECK(args, 1)
	mbm_error(MC->add_admin(first));
	break;

    case "da": /* Delete a board admin */
	ADM_CHECK(admin)
	MBM_ARG_CHECK(args, 1)
	mbm_error(MC->delete_admin(first));
	break;

    case "la": /* List all board admin */
	LORD_CHECK(admin, lvl)
	if (admin)
	    mbm_error(MC->list_admin(1));
	else
	    mbm_error(MC->list_admin(0));
	break;

    default:
	err_args(arg);
	mbm_error(MBM_NO_CMD);
	break;
    }

    return 1;
}

/* **************************************************************************
 * mbh functions
 */

/* **************************************************************************
 * mbh - The global mbh command, help
 */
public nomask int
mbh_cmd(string arg)
{
    setuid();
    seteuid(getuid());

    mbs_error(MC->do_help(arg));
    return 1;
}

/* **************************************************************************
 * mbs functions
 */

/* **************************************************************************
 * mbs - The global mbs command, reading
 */
public nomask int
mbs_cmd(string arg)
{
    string	*args, first, rest;
    int admin = MC->query_admin(TI->query_real_name());

    setuid();
    seteuid(getuid());

    if (!WIZ_CHECK)
    {
	mbs_error(MBS_NO_AUTH);
	return 1;
    }

    restore_mbs();
    check_gc();

    if (!strlen(arg))
    {
	arg = "";
	args = ({});
    }
    else
    {
	args = explode(arg, " ");
	arg = args[0];
	if (sizeof(args) > 1)
	{
	    args = args[1..];
	    first = args[0];
	    rest = implode(args[1..], " ");
	}
	else
	{
	    args = ({});
	    first = rest = "";
	}
    }

    switch (arg)
    {
    /*
     * News admin
     */
    case "b": /* Select a board */
	args += ({ "", "" });
	mbs_error(select_board(0, 0, first, args[1]));
	break;
	
    case "c": /* Catchup this board */
	mbs_error(catch_up(0, 0));
	break;

    case "C": /* Catchup the current selection */
	mbs_error(catch_up(1, 0));
	break;

    case "CC": /* Catch up on everything */
	mbs_error(catch_up(2, 0));
	break;

    case "uc": /* Uncatchup this board */
	mbs_error(catch_up(0, 1));
	break;

    case "UC": /* Uncatchup the current selection */
	mbs_error(catch_up(1, 1));
	break;

    case "UCC": /* Uncatch up on everything */
	mbs_error(catch_up(2, 1));
	break;

    case "d": /* Delete an article */
	MBS_ARG_CHECK(args, 1);
	mbs_error(delete_article(first));
	break;

    case "h": /* List unread headers */
	args += ({ "", "" });
	mbs_error(select_board(0, 1, first, args[1]));
	break;

    case "H": /* List all headers */
	args += ({ "", "" });
	mbs_error(select_board(1, 1, first, args[1]));
	break;
	
    case "l": /* List subscribed boards */
	list_subscribed(0, 0);
	break;

    case "": /* List unread boards with news */
    case "lu":
	list_subscribed(1, 0);
	break;

    case "lua": /* List ALL unread boards with news */
	list_subscribed(1, 1);
	break;
	
    case "L": /* List unsubscribed boards */
	mbs_error(list_boards(0, first));
	break;

    case "La": /* List all boards */
	mbs_error(list_boards(1, first));
	break;

    case "r": /* Read an item on a board */
        args += ({ "", "" });
	mbs_error(read_nur(first, args[1], 0));
	break;

    case "mr": /* Mread an item on a board */
        args += ({ "", "" });
	mbs_error(read_nur(first, args[1], 1));
	break;

    case "p": /* Post on a board */
	mbs_error(post_article(implode(args[0..], " ")));
	break;

    case "s": /* Subscribe to a board */
	MBS_ARG_CHECK(args, 2);
	args += ({ "" });
	mbs_error(subscribe(first, args[1], args[2]));
	break;

    case "S": /* Subscribe to a selection */
	MBS_ARG_CHECK(args, 1);
	args += ({ "" });
	mbs_error(subscribe_sel(first, args[1]));
	break;

    case "t": /* Teleport to a board */
	args += ({ "", "" });
	mbs_error(tele_to_board(first, args[1]));
	break;

    case "u": /* Unsubscribe a board */
	args += ({ "", "" });
	mbs_error(unsubscribe(first, args[1]));
	break;

    case "U": /* Unsubscribe a selection */
	MBS_ARG_CHECK(args, 1);
	mbs_error(unsubscribe_sel(first));
	break;

    case "UU": /* Unsubscribe all boards */
	SbMap = BdMap = ([]);
	CurrBoard = "";
	save_mbs();
	write("Unsubscribed all boards.\n");
	break;

    case "scrap": /* Scrap a board */
	MBS_ARG_CHECK(args, 2);
	mbs_error(scrap_board(first, args[1]));
	break;

    case "ls": /* List the scrap boards */
	mbs_error(list_scrap());	
	break;
	
    /*
     * Group admin
     */
    case "cn": /* Create a newsgroup */
	MBS_ARG_CHECK(args, 1);
	err_args(first);
	mbs_error(add_newsgroup(first));
	break;
	
    case "dn": /* Delete a newsgroup */
	MBS_ARG_CHECK(args, 1);
	err_args(first);
	mbs_error(delete_newsgroup(first));
	break;
	
    case "ln": /* List all newsgroups */
	mbs_error(list_newsgroups());
	break;

    case "rn": /* Rename a newsgroup */
	MBS_ARG_CHECK(args, 2);
	mbs_error(rename_newsgroup(first, args[1]));
	break;

    /*
     * Selection admin
     */
    case "+": /* Select next unread board */
	select_nur(1);
	mbs_error(select_board(0, 1, "", ""));
	break;
	   
    case "ss": /* Set the selection */
	mbs_error(set_selection(first));
	break;

    case "si": /* Select the selection item */
	mbs_error(select_selection_item(first));
	break;

    /*
     * Misc commands
     */
    case "la": /* List all board admin */
	mbm_error(MC->list_admin(0));
	break;

    case "lc": /* List all categories */
	mbm_error(MC->list_categories(0));
	break;

    case "gr": /* Generate a usage report */
	MBS_ARG_CHECK(args, 1);

	if (first == "reset")
	{
	    ADM_CHECK(admin)
	    mbm_error(MC->generate_report(2));
	}
	else if (first[0..0] == "p")
	    mbm_error(MC->generate_report(1));
	else if (first[0..0] == "r")
	    mbm_error(MC->generate_report(0));
	else if (first[0..1] == "tp")
	    mbm_error(MC->generate_report(3));
	else if (first[0..1] == "tr")
	    mbm_error(MC->generate_report(4));

	break;
	
    /*
     * End of story
     */
    default:
	args += ({ "", "" });
	mbs_error(select_board(0, 1, arg, first));
	break;
    }

    return 1;
}

/*
 * Function name: catch_up
 * Description:   Catch up on a board
 * Arguments:	  what - what to catch up on
 *		  uncatch - uncatchup or not
 */
static nomask int
catch_up(int what, int uncatch)
{
    mixed	ind;
    string	tm;

    if (uncatch)
	tm = "b0";
    else
	tm = "b" + time();
    
    switch (what)
    {
    case 1: /* Current selection */
	ind = m_values(BdMap);
	switch (Selection)
	{
	case ORDER_CAT:
	    ind = filt_bdata(CurrItem[Selection], ind, SB_CAT);
	    break;
	    
	case ORDER_DOMAIN:
	    ind = filt_bdata(CurrItem[Selection], ind, SB_DOMAIN);
	    break;
	    
	case ORDER_GROUP:
	    /* FALLTHROUGH */
	default:
	    ind = filt_bdata(CurrItem[Selection], ind, SB_GROUP);
	    break;
	}
	map(ind, &set_time(tm, ) @ &operator([])(, SB_SPATH));
	break;
	
    case 2: /* All boards */
	ind = m_indexes(BdMap);
	map(ind, &set_time(tm, ));
	break;
	
    case 0: /* Current board */
	/* FALLTHROUGH */
    default:
	if (strlen(CurrBoard) && sizeof(BdMap[CurrBoard]))
	    BdMap[CurrBoard][SB_LNOTE] = tm;
	else
	    return MBS_NO_CURR;
	break;
    }

    if (uncatch)
	write("Reset news, read it all again!\n");
    else
	write("Caught up on news, saved you a lot of time I bet!\n");

    select_nur(0);
    shuffle_boards();
    select_board(0, 1, "", "");
    save_mbs();
    return MBS_NO_ERR;
}

/*
 * Function name: list_subscribed
 * Description:	  List the subscribed boards
 * Arguments:	  unread - only unread, if set
 *		  all - See all undread
 */
static nomask void
list_subscribed(int unread, int all)
{
    string header, h2;
    mixed  blist, olist, plist;
    int	   i, sz, first = 1, news = 0;
    
    if (!m_sizeof(BdMap))
    {
	write("No subscribed boards.\n");
	return;
    }

    /* Print the header */
    header = ({ "Category", "Domain", "Newsgroup" })[Selection] + ": ";
    if (strlen(CurrItem[Selection]))
	header += CurrItem[Selection];
    else
    {
	header += "None";
	return;
    }
    
    header += sprintf("\n\n%-12s", " Board");
    if (Selection != ORDER_CAT)
	header += sprintf("%-11s", "Category");
    if (Selection != ORDER_DOMAIN)
	header += sprintf("%-11s", "Domain");
    if (Selection != ORDER_GROUP)
	header += sprintf("%-11s", "Newsgroup");
    header += sprintf("%-31s%s\n", "Description", "Status");

    header += sprintf("%-12s", " -----");
    if (Selection != ORDER_CAT)
	header += sprintf("%-11s", "---------");
    if (Selection != ORDER_DOMAIN)
	header += sprintf("%-11s", "------");
    if (Selection != ORDER_GROUP)
	header += sprintf("%-11s", "---------");
    header += sprintf("%-31s%s\n", "-----------", "------");

    /*
     * First check if unread listing, then report other unread
     * selections as well.
     */
    if (unread)
    {
	if (Selection == ORDER_CAT)
	    olist = sort_array(MC->query_categories());
	else if (Selection == ORDER_DOMAIN)
	    olist = sort_array(SECURITY->query_domain_list());
	else
	    olist = Groups;
	sz = sizeof(olist);

	for (i = 0 ; i < sz ; i++)
	{
	    if (CurrItem[Selection] != olist[i])
	    {
		blist = SbMap[olist[i]];
		if (sizeof((blist = filter(blist, filt_unread_news))))
		{
		    write("Unread news in the " + ({ "category", "domain", "newsgroup" })[Selection] + " '" + olist[i] + "'\n");
		    if (all)
		    {
			write("[");
			map(blist, print_shortsub_info);
			write("]\n");
		    }
		    news = 1;
		}
	    }
	}
    }
    
    /* Now print the actual boards */
    blist = SbMap[CurrItem[Selection]];

    if (Selection == ORDER_CAT)
    {
	olist = sort_array(SECURITY->query_domain_list());
	for (i = 0, sz = sizeof(olist) ; i < sz ; i++)
	{
	    plist = filter(blist, &operator(==)(olist[i]) @
			   &operator([])(, SB_DOMAIN));

	    if (!sizeof(plist))
		continue;

	    if (unread)
		plist = filter(plist, filt_unread_news);
	    
	    if (sizeof(plist))
	    {
		if (first)
		{
		    write(header);
		    first = 0;
		}
		plist = sort_array(plist, "sort_dom_boards", TO);
		map(plist, print_subboard_info);
		write("\n");
	    }
	}
	if (first && !news)
	    write(NO_NEWS);
    }
    else
    {
	olist = sort_array(MC->query_categories());
	for (i = 0, sz = sizeof(olist) ; i < sz ; i++)
	{
	    plist = filter(blist, &operator(==)(olist[i]) @
			   &operator([])(, SB_CAT));

	    if (!sizeof(plist))
		continue;

	    if (unread)
		plist = filter(plist, filt_unread_news);
	    
	    if (sizeof(plist))
	    {
		if (first)
		{
		    write(header);
		    first = 0;
		}
		plist = sort_array(plist, "sort_cath_boards", TO);
		map(plist, print_subboard_info);
		write("\n");
	    }
	}
	if (first && !news)
	    write(NO_NEWS);
    }
}

/*
 * Function name: list_boards
 * Description:	  List boards from mbs_central
 * Arguments:	  which - which boards
 *		  rest - restriction, of some kind.
 * Returns:	  Error code, if any.
 */
static nomask int
list_boards(int which, string rest)
{
    mixed	blist, olist, plist;
    int		order, i, sz;

    rest = UC(rest);

    if (strlen(rest))
    {
	if (member_array(rest, SECURITY->query_domain_list()) >= 0)
	    order = ORDER_DOMAIN;
	else if (member_array(rest, MC->query_categories()) >= 0)
	    order = ORDER_CAT;
	else
	{
	    err_args(rest);
	    return MBS_BAD_SEL;
	}
    }

    if (which)
	blist = MC->query_boards(order, rest, ({}));
    else
	blist = MC->query_boards(order, rest, m_indexes(BdMap));

    if (!sizeof(blist))
    {
	if (!strlen(rest))
	    write("No new boards registered.\n");
	else
	    write("No new boards registered in '" + rest + "'.\n");
	return MBS_NO_ERR;
    }
    
    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "Board", "Category", "Domain", "Description", "Access"));
    write(sprintf("%-11s%-11s%-11s%-31s%s\n", "-----", "--------", "------", "-----------", "------"));
    if (Selection == ORDER_DOMAIN)
    {
	olist = sort_array(SECURITY->query_domain_list());
	for (i = 0, sz = sizeof(olist) ; i < sz ; i++)
	{
	    plist = filter(blist, &operator(==)(olist[i]) @
			   &operator([])(, BBP_DOMAIN));
	    plist = filter(plist, &operator(>)(0) @
			   &member_array(, ScrapList) @
			   &operator([])(, BBP_SPATH));
	    if (sizeof(plist))
	    {
		plist = sort_array(plist, "sort_dom_boards", find_object(MC));
		map(plist, print_newboard_info);
		write("\n");
	    }
	}
    }
    else
    {
	olist = sort_array(MC->query_categories());
	for (i = 0, sz = sizeof(olist) ; i < sz ; i++)
	{
	    plist = filter(blist, &operator(==)(olist[i]) @
			   &operator([])(, BBP_CAT));
	    plist = filter(plist, &operator(>)(0) @
			   &member_array(, ScrapList) @
			   &operator([])(, BBP_SPATH));
	    if (sizeof(plist))
	    {
		plist = sort_array(plist, "sort_cath_boards", find_object(MC));
		map(plist, print_newboard_info);
		write("\n");
	    }
	}
    }

    return MBS_NO_ERR;
}

/*
 * Function name: read_nur
 * Description:	  Read next unread note, or specified note
 * Arguments:	  arg1 - the specified board or item, if any
 *                arg2 - the specified board, if any
 *		  mread - mread or not.
 * Returns:	  Error code, if any
 */
static nomask int
read_nur(string arg1, string arg2, int mread)
{
    mixed note_info;
    string board_name = "";
    int note_number = 0, err;

    if (strlen(arg2))
    {
        board_name = arg2;

        if ((note_number = atoi(arg1)) < 1)
        {
            return MBS_NUM_ZERO;
        }
    }
    else
    {
        if (sscanf(arg1, "%d", note_number))
        {
	    if (note_number < 1)
	    {
		return MBS_NUM_ZERO;
	    }
        }
        else
        {
            board_name = arg1;
        }
    }

    if (strlen(board_name) &&
	((err = select_board(0, 0, board_name, "")) != MBS_NO_ERR))
    {
        return err;
    }

    if (note_number)
    {
	if (!strlen(CurrBoard) || !sizeof(BdMap[CurrBoard]))
	    return MBS_NO_CURR;

	err_args(CurrBoard, BdMap[CurrBoard][SB_CAT]);
	note_info = MC->read_item(CurrBoard, note_number, mread);

	if (note_info[0] == MBS_NO_ERR &&
	    atoi(BdMap[CurrBoard][SB_LNOTE][1..]) <
		atoi(note_info[1][1..]))
	{
	    BdMap[CurrBoard][SB_LNOTE] = note_info[1];
	    shuffle_boards();
	    save_mbs();
	}

	return note_info[0];
    }

    if ((!strlen(board_name) && !select_nur(0)) ||
        ((note_info = MC->find_next_unread(CurrBoard,
        atoi(BdMap[CurrBoard][SB_LNOTE][1..])))[0] == 0))
    {
        write(NO_NEWS);
        return MBS_NO_ERR;
    }

    BdMap[CurrBoard][SB_LNOTE] = note_info[1];
    shuffle_boards();
    save_mbs();
    err_args(BdMap[CurrBoard][SB_BOARD],
    BdMap[CurrBoard][SB_CAT]);
    return MC->read_item(CurrBoard, note_info[0], mread)[0];
}

/*
 * Function name: post_article
 * Description:	  Post an article on the current board.
 * Arguments:	  header - The header to the note.
 * Returns:	  Error code, if any.
 */
static nomask int
post_article(string header)
{
    if (!strlen(CurrBoard))
	return MBS_NO_CURR;
    if (!strlen(header))
	return MBS_NO_HEADER;
 
    err_args(CurrBoard, BdMap[CurrBoard][SB_CAT]);

    write("Posting on board '" + BdMap[CurrBoard][SB_BOARD] + "' in the " +
	  ({ "category", "domain", "newsgroup" })[Selection] +
	  " '" + BdMap[CurrBoard][Selection] + "'.\n");
    return MC->post_item(CurrBoard, header);
}

/*
 * Function name: delete_article
 * Description:	  Delete an article on a board
 * Arguments:	  num - the article to delete
 * Returns:	  Error code, if any.
 */
static nomask int
delete_article(string num)
{
    if (!strlen(CurrBoard))
	return MBS_NO_CURR;

    err_args(CurrBoard, BdMap[CurrBoard][SB_CAT]);
    return MC->remove_item(CurrBoard, num);
}

/*
 * Function name: select_board
 * Description:   Select a board
 * Arguments:	  what - what to see.
 *		  headers - see headers or not, on empty argument
 *		  board - the board to select
 *		  item - item of interest, if any.
 * Returns:	  Error code, if any
 */
static nomask int
select_board(int what, int headers, string board, string item)
{
    mixed	blist;
    int		oldsel;

    board = UC(board);
    if (!strlen(board) && (what == 0 && headers == 0))
    {
	if (strlen(CurrBoard) && sizeof(BdMap[CurrBoard]))
	    write("Your currently selected board is '" +
		  BdMap[CurrBoard][SB_BOARD] + "' in the " +
		  ({ "category", "domain", "newsgroup" })[Selection] + " '" +
		  BdMap[CurrBoard][Selection] + "'.\n");
	else
	    return MBS_NO_CURR;
	return MBS_NO_ERR;
    }

    oldsel = ORDER_NONE;
    if (strlen(item))
    {
	item = UC(item);
	switch (Selection)
	{
	case ORDER_CAT:
	    err_args(item);
	    if (member_array(item, MC->query_categories()) >= 0)
		break;
	    else if (member_array(item, Groups) >= 0)
	    {
		Selection = ORDER_GROUP;
		oldsel = ORDER_CAT;
		shuffle_boards();
	    }
	    else if (member_array(item, SECURITY->query_domain_list()) >= 0)
	    {
		Selection = ORDER_DOMAIN;
		oldsel = ORDER_CAT;
		shuffle_boards();
	    }
	    else
		return MBS_BAD_CAT;
	    break;
	    
	case ORDER_DOMAIN:
	    err_args(item);
	    if (member_array(item, SECURITY->query_domain_list()) >= 0)
		break;
	    else if (member_array(item, Groups) >= 0)
	    {
		Selection = ORDER_GROUP;
		oldsel = ORDER_DOMAIN;
		shuffle_boards();
	    }
	    else if (member_array(item, MC->query_categories()) >= 0)
	    {
		Selection = ORDER_DOMAIN;
		oldsel = ORDER_DOMAIN;
		shuffle_boards();
	    }
	    else
		return MBS_BAD_DOMAIN;
	    break;
	    
	case ORDER_GROUP:
	    /* FALLTHROUGH */
	default:
	    err_args(item);
	    if (member_array(item, Groups) >= 0)
		break;
	    else if (member_array(item, MC->query_categories()) >= 0)
	    {
		Selection = ORDER_CAT;
		oldsel = ORDER_GROUP;
		shuffle_boards();
	    }
	    else if (member_array(item, SECURITY->query_domain_list()) >= 0)
	    {
		Selection = ORDER_DOMAIN;
		oldsel = ORDER_GROUP;
		shuffle_boards();
	    }
	    else
		return MBS_GRP_NONE;
	    break;
	}
    }
    else
	item = CurrItem[Selection];

    if (strlen(board))
    {
	blist = filter(SbMap[item], &operator(==)(board) @
		       &operator([])(, SB_BOARD));
    
	if (strlen(item))
	    CurrItem[Selection] = item;

	if (!sizeof(blist))
	{
	    err_args(board,
		     ({ "category", "domain", "newsgroup" })[Selection], item);
	    return MBS_NO_SUB;
	}

	blist = blist[0];
	CurrBoard = blist[SB_SPATH];
	save_mbs();
    
	write("Selected board '" + board + "' in the " + ({ "category", "domain", "newsgroup" })[Selection] + " '" + CurrItem[Selection] + "'.\n");
    }
    else
	blist = BdMap[CurrBoard];

    if (!sizeof(blist))
	return MBS_NO_CURR;

    /* Restore the old confusion */
    if (oldsel != ORDER_NONE)
    {
	Selection = oldsel;
	CurrItem[Selection] = blist[Selection];
	shuffle_boards();
	save_mbs();
    }

    if (headers)
    {
        return MC->print_headers(what, blist[SB_SPATH],
	           atoi(blist[SB_LNOTE][1..]),
	           ({ "category", "domain", "newsgroup" })[Selection],
		   CurrItem[Selection]);
    }

    return MBS_NO_ERR;
}

/*
 * Function name: subscribe
 * Description:	  Subscribe to a board
 * Arguments:	  board - the board
 *		  item - selection item
 * Returns:	  Error code, if any
 */
static nomask int
subscribe(string board, string item, string group)
{
    mixed info;

    if (!strlen(CurrItem[ORDER_GROUP]))
	return MBS_NO_GROUP;

    board = UC(board);
    item = UC(item);
    group = UC(group);

    if (member_array(item, SECURITY->query_domain_list()) < 0 &&
	member_array(item, MC->query_categories()) < 0)
    {
	err_args(item);
	return MBS_BAD_SEL;
    }
    
    info = MC->query_board_exist(board, item);

    if (strlen(group))
    {
	if (member_array(group, Groups) < 0)
	{
	    err_args(group);
	    return MBS_GRP_NONE;
	}
    }	    
    else
	group = CurrItem[ORDER_GROUP];

    if (sizeof(info))
    {
	if (sizeof(BdMap[info[BBP_SPATH]]))
	    return MBS_SUBSCRIBE;

	BdMap[info[BBP_SPATH]] = ({ info[BBP_CAT], info[BBP_DOMAIN],
		group, board, info[BBP_SPATH], "b0" });
	shuffle_boards();

	if (member_array(item, MC->query_categories()) < 0)
	    CurrItem[ORDER_DOMAIN] = item;
	else
	    CurrItem[ORDER_CAT] = item;
	CurrBoard = info[BBP_SPATH];
	ScrapList -= ({ info[BBP_SPATH] });
	save_mbs();

	write("You have subscribed to the board '" + board + "' in the category '" + info[BBP_CAT] + "' and assigned it to the newsgroup '" + group + "'.\n");
    }
    else
    {
	if (member_array(item, MC->query_categories()) < 0)
	    err_args(board, "domain", item);
	else
	    err_args(board, "category", item);
	return MBS_NO_BOARD;
    }

    return MBS_NO_ERR;
}

/*
 * Function name: unsubscribe
 * Description:	  Unsubscribe a board
 * Arguments:	  board - the board to unsubscribe
 *		  item - the selection item of the board
 * Returns:	  Error message, if any
 */
static nomask int
unsubscribe(string board, string item)
{
    string	*bdata, what;
    mixed	itemdata;

    board = UC(board);
    item = UC(item);
    
    if (!strlen(item))
    {
	if (!strlen(board))
	{
	    if (!strlen(CurrBoard))
		return MBS_NO_SPEC;

	    bdata = BdMap[CurrBoard];
	}
	else
	{
	    itemdata = SbMap[CurrItem[Selection]];
	    itemdata = filter(itemdata, &operator(==)(board) @ &operator([])(, SB_BOARD));
	    if (!sizeof(itemdata))
	    {
		err_args(board, "", "");
		return MBS_NO_SUB;
	    }
	
	    if (sizeof(itemdata) > 1)
	    {
		err_args(board, "", "");
		return MBS_SPEC_AMBIG;
	    }

	    bdata = itemdata[0];
	}
    }
    else
    {
	itemdata = m_values(BdMap);
	if (member_array(item, MC->query_categories()) < 0)
	{
	    itemdata = filter(itemdata, &operator(==)(item) @ &operator([])(, SB_DOMAIN));
	    what = "domain";
	}
	else
	{
	    itemdata = filter(itemdata, &operator(==)(item) @ &operator([])(, SB_CAT));
	    what = "category";
	}
	itemdata = filter(itemdata, &operator(==)(board) @ &operator([])(, SB_BOARD));

	if (!sizeof(itemdata))
	{
	    err_args(board, what, item);
	    return MBS_NO_SUB;
	}

	bdata = itemdata[0];
    }

    BdMap = m_delete(BdMap, bdata[SB_SPATH]);
    shuffle_boards();

    if (CurrBoard == bdata[SB_SPATH])
	CurrBoard = "";
    
    save_mbs();

    write("You have unsubscribed the board '" + bdata[SB_BOARD] +
	  "' in the category '" + bdata[SB_CAT] +
	  "' belonging to the newsgroup '" + bdata[SB_GROUP] + "'.\n");
	    
    return MBS_NO_ERR;
}

/*
 * Function name: subscribe_sel
 * Description:	  Subscribe an entire selection
 * Arguments:	  sel - the selection
 *		  group - the group, if any
 * Returns:	  Error code, if any.
 */
static nomask int
subscribe_sel(string sel, string group)
{
    mixed	blist;
    int		order, i, sz;

    sel = UC(sel);
    group = UC(group);

    if (!strlen(CurrItem[ORDER_GROUP]))
	return MBS_NO_GROUP;

    if (strlen(group) &&
	member_array(group, Groups) < 0)
    {
	err_args(group);
	return MBS_GRP_NONE;
    }
    else
	group = CurrItem[ORDER_GROUP];
    
    if (member_array(sel, SECURITY->query_domain_list()) >= 0)
	order = ORDER_DOMAIN;
    else if (member_array(sel, MC->query_categories()) >= 0)
	order = ORDER_CAT;
    else
    {
	err_args(sel);
	return MBS_BAD_SEL;
    }

    blist = MC->query_boards(order, sel, ({}));

    for (i = 0, sz = sizeof(blist) ; i < sz ; i++)
    {
	if (!sizeof(BdMap[blist[i][BBP_SPATH]]))
	{
	    ScrapList -= ({ blist[i][BBP_SPATH] });
	    write("Subscribing to '" + blist[i][BBP_BOARD] + "'.\n");
	    BdMap[blist[i][BBP_SPATH]] = ({ blist[i][BBP_CAT],
						blist[i][BBP_DOMAIN],
						group, blist[i][BBP_BOARD],
						blist[i][BBP_SPATH], "b0" });
	}
    }
    shuffle_boards();
    save_mbs();
    
    return MBS_NO_ERR;
}

/*
 * Function name: unsubscribe_sel
 * Description:	  Unsubscribe an entire selection
 * Arguments:	  sel - the selection
 * Returns:	  Error code, if any.
 */
static nomask int
unsubscribe_sel(string sel)
{
    mixed	blist;
    int		order, i, sz;

    sel = UC(sel);

    if (member_array(sel, SECURITY->query_domain_list()) >= 0)
	order = ORDER_DOMAIN;
    else if (member_array(sel, MC->query_categories()) >= 0)
	order = ORDER_CAT;
    else if (member_array(sel, Groups) >= 0)
	order = ORDER_GROUP;
    else
    {
	err_args(sel);
	return MBS_BAD_SEL;
    }

    blist = m_values(BdMap);
    switch (order)
    {
    case ORDER_DOMAIN:
	blist = filter(blist, &operator(==)(sel) @ &operator([])(, SB_DOMAIN));
	break;

    case ORDER_CAT:
	blist = filter(blist, &operator(==)(sel) @ &operator([])(, SB_CAT));
	break;

    default:
	blist = filter(blist, &operator(==)(sel) @ &operator([])(, SB_GROUP));
	break;
    }
    
    for (i = 0, sz = sizeof(blist) ; i < sz ; i++)
    {
	if (CurrBoard == blist[i][SB_SPATH])
	    CurrBoard = "";
	    
	write("Unsubscribing '" + blist[i][SB_BOARD] + "'.\n");
	BdMap = m_delete(BdMap, blist[i][SB_SPATH]);
    }
    shuffle_boards();
    save_mbs();
    
    return MBS_NO_ERR;
}

/*
 * Function name: tele_to_board
 * Description:	  Teleport to a board
 * Arguments:	  board - the board
 *		  sel - selection
 * Returns:	  Error code, if any.
 */
static nomask int
tele_to_board(string board, string sel)
{
    string	bpath;
    mixed	*blist;
    int		order;

    board = UC(board);
    sel = UC(sel);
    
    blist = m_values(BdMap);

    if (!strlen(board))
    {
	if (strlen(CurrBoard) && sizeof(BdMap[CurrBoard]))
	    return (MC->tele_to_board(BdMap[CurrBoard][SB_SPATH]));
	else
	    return MBS_NO_CURR;
    }
    else if (strlen(sel))
    {
	if (member_array(sel, SECURITY->query_domain_list()) >= 0)
	{
	    blist = filter(blist, &operator(==)(sel) @
			   &operator([])(, SB_DOMAIN));
	    blist = filter(blist, &operator(==)(board) @
			   &operator([])(, SB_BOARD));
	    order = ORDER_DOMAIN;
	}
	else if (member_array(sel, MC->query_categories()) >= 0)
	{
	    blist = filter(blist, &operator(==)(sel) @
			   &operator([])(, SB_CAT));
	    blist = filter(blist, &operator(==)(board) @
			   &operator([])(, SB_BOARD));
	    order = ORDER_CAT;
	}
	else if (member_array(sel, Groups) >= 0)
	{
	    blist = filter(blist, &operator(==)(sel) @
			   &operator([])(, SB_GROUP));
	    blist = filter(blist, &operator(==)(board) @
			   &operator([])(, SB_BOARD));
	    order = ORDER_GROUP;
	}
	else
	    return MBS_BAD_SEL;
    }
    else
    {
	blist = filter(SbMap[CurrItem[Selection]], &operator(==)(board) @
		       &operator([])(, SB_BOARD));
	order = Selection;
	sel = CurrItem[Selection];
    }

    if (!sizeof(blist))
    {
	err_args(board,
		 ({ "category", "domain", "newsgroup" })[order],
		 sel);
	return MBS_NO_SUB;
    }
    
    return (MC->tele_to_board(blist[0][SB_SPATH]));
}

/*
 * Function name: add_newsgroup
 * Description:	  Add a group to the list of groups
 * Arguments:	  grp - the new group.
 * Returns:	  Error code, if any
 */
static nomask int
add_newsgroup(string grp)
{
    grp = UC(grp);
    if (member_array(grp, Groups) >= 0)
	return MBS_GRP_EXIST;

    if (strlen(grp) > NAME_LEN)
    {
	err_args(grp);
	return MBS_STR_LONG;
    }
    
    Groups += ({ grp });
    Groups = sort_array(Groups);
    CurrItem[ORDER_GROUP] = grp;
    save_mbs();
    write("Newsgroup '" + grp + "' created.\n");
    return MBS_NO_ERR;
}

/*
 * Function name: list_newsgroups
 * Description:	  List the groups
 * Returns:	  Error code, if any
 */
static nomask int
list_newsgroups()
{
    int i, sz;
    
    if (!(sz = sizeof(Groups)))
	write("You have no groups currently defined.\n");
    else
    {
	write("Your newsgroups: ");
	for (i = 0 ; i < sz ; i++)
	{
	    if (Groups[i] == CurrItem[ORDER_GROUP])
		write("[" + Groups[i] + "]");
	    else
		write(Groups[i]);

	    if (i < sz - 1)
		write(", ");
	}
    }
    write("\n");
    return MBS_NO_ERR;
}

/*
 * Function name: delete_newsgroup
 * Description:	  Remove a group from the list of groups
 * Arguments:	  grp - the group to remove.
 * Returns:	  Error code, if any
 */
static nomask int
delete_newsgroup(string grp)
{
    mixed	blist;
    int		i, sz;

    grp = UC(grp);

    if (member_array(grp, Groups) < 0)
	return MBS_GRP_NONE;

    if (CurrItem[ORDER_GROUP] == grp)
    {
	CurrItem[ORDER_GROUP] = sizeof(Groups) ? Groups[0] : "";
	write("Current newsgroup: " + CurrItem[ORDER_GROUP] + "\n");
    }

    /* Remove the group */
    Groups -= ({ grp });
    write("Newsgroup '" + grp + "' deleted.\n");

    /* Remove all boards in that group */
    blist = m_values(BdMap);
    blist = filt_bdata(grp, blist, SB_GROUP);
    for (sz = sizeof(blist), i = 0 ; i < sz ; i++)
    {
	write("Unsubscribing board '" + blist[i][SB_BOARD] + "' in the category '" + blist[i][SB_CAT] + "'.\n");
	BdMap = m_delete(BdMap, blist[i][SB_SPATH]);
    }

    if (CurrItem[ORDER_GROUP] == grp)
	CurrItem[ORDER_GROUP] = sizeof(Groups) ? Groups[0] : "";

    shuffle_boards();
    save_mbs();

    return MBS_NO_ERR;
}

/*
 * Function name: rename_newsgroup
 * Description:	  Rename a group
 * Arguments:	  old - the old name
 *		  new - the new name
 * Returns:	  Error code, if any.
 */
static nomask int
rename_newsgroup(string old, string new)
{
    mixed	blist;
    int		i, sz;

    old = UC(old);
    new = UC(new);

    if (member_array(old, Groups) < 0)
    {
	err_args(old);
	return MBS_GRP_NONE;
    }
    
    if (member_array(new, Groups) >= 0)
    {
	err_args(new);
	return MBS_GRP_EXIST;
    }

    Groups -= ({ old });
    Groups += ({ new });

    CurrItem[ORDER_GROUP] = new;

    /* Rename all boards in that group */
    blist = m_values(BdMap);
    blist = filt_bdata(old, blist, SB_GROUP);
    for (sz = sizeof(blist), i = 0 ; i < sz ; i++)
    {
	write("Moving board '" + blist[i][SB_BOARD] + "' in the category '" + blist[i][SB_CAT] + "'.\n");
	blist[i][SB_GROUP] = new;
	BdMap[blist[i][SB_SPATH]] = blist[i];
    }
    shuffle_boards();
    save_mbs();
    write("Newsgroup '" + old + "' renamed to '" + new + "'.\n");
    return MBS_NO_ERR;
}

/*
 * Function name: set_selection
 * Description:	  Set search/list selection
 * Arguments:	  selection - the selection to set
 * Returns:	  Error code, if any.
 */
static nomask int
set_selection(string selection)
{
    selection = UC(selection);
    
    if (!strlen(selection))
    {
	write("Current selection is '" + ({ "Category", "Domain", "Newsgroup" })[Selection] + "' (" + CurrItem[Selection] + ").\n");
	return MBS_NO_ERR;
    }
    
    switch (selection[0])
    {
    case 'C':
	Selection = ORDER_CAT;
	break;

    case 'D':
	Selection = ORDER_DOMAIN;
	break;

    case 'N':
	Selection = ORDER_GROUP;
	break;

    default:
	err_args(selection);
	return MBS_BAD_ORDER;
	break;
    }

    write("Current selection is '" + ({ "Category", "Domain", "Newsgroup" })[Selection] + "' (" + CurrItem[Selection] + ").\n");
    shuffle_boards();
    save_mbs();
    return MBS_NO_ERR;
}

/*
 * Function name: select_selection_item
 * Description:	  Select the item to selection by
 * Arguments:	  item - the selection item
 * Returns:	  Error code, if any
 */
static nomask int
select_selection_item(string item)
{
    string *list;
    mixed info;
    
    item = UC(item);

    switch (Selection)
    {
    case ORDER_CAT:
	if (strlen(item))
	{
	    list = MC->query_categories();
	    err_args(item);
	    if (member_array(item, list) < 0)
		return MBS_BAD_CAT;
	    CurrItem[ORDER_CAT] = item;
	}
	break;
	
    case ORDER_DOMAIN:
	if (strlen(item))
	{
	    list = SECURITY->query_domain_list();
	    err_args(item);
	    if (member_array(item, list) < 0)
		return MBS_BAD_DOMAIN;
	    CurrItem[ORDER_DOMAIN] = item;
	}
	break;

    case ORDER_GROUP:
	/* FALLTHROUGH */
    default:
	if (strlen(item))
	{
	    err_args(item);
	    if (member_array(item, Groups) < 0)
		return MBS_GRP_NONE;
	    CurrItem[ORDER_GROUP] = item;
	}
	break;
    }

    write("Currently selected " + ({ "category: ", "domain: ", "newsgroup: " })[Selection] + CurrItem[Selection] + "\n");

    if ((info = find_nur_insel(0))[0] && strlen(info[1]))
	write(info[1]);

    save_mbs();
    return MBS_NO_ERR;
}

/*
 * Function name: list_scrap
 * Description:	  List the scrapped boards.
 * Returns:	  Error code, if any
 */
static nomask int
list_scrap()
{
    int		i, sz, first = 1;
    mixed	blist;
    string	header;
    
    if (!(sz = sizeof(ScrapList)))
	write("You haven't scrapped any boards.\n");
    else
    {
	header = sprintf("%-11s%-11s%-11s%s\n", "Board", "Category", "Domain", "Description");
	header += sprintf("%-11s%-11s%-11s%s\n", "-----", "--------", "------", "-----------");
	for (i = 0 ; i < sz ; i++)
	{
	    blist = MC->get_board_by_path(ScrapList[i]);
	    if (sizeof(blist))
	    {
		if (first)
		{
		    write(header);
		    first = 0;
		}
		write(sprintf("%-11s%-11s%-11s%s\n", blist[BBP_BOARD], blist[BBP_CAT], blist[BBP_DOMAIN], blist[BBP_DESC]));
	    }
	}
    }

    return MBS_NO_ERR;
}

/*
 * Function name: scrap_board
 * Description:	  Add a board to the list of scrapped boards.
 * Arguments:	  board - the board
 *		  item - selection item
 * Returns:	  Error code, if any
 */
static nomask int
scrap_board(string board, string item)
{
    mixed info;

    if (!strlen(CurrItem[ORDER_GROUP]))
	return MBS_NO_GROUP;

    board = UC(board);
    item = UC(item);

    info = MC->query_board_exist(board, item);

    if (sizeof(info))
    {
	if (sizeof(BdMap[info[BBP_SPATH]]))
	    return MBS_SUBSCRIBE;

	if (member_array(info[BBP_SPATH], ScrapList) >=0)
	    return MBS_SCRAPPED;

	ScrapList += ({ info[BBP_SPATH] });
	save_mbs();

	write("You have scrapped the board '" + board + "' in the category '" + info[BBP_CAT] + "'.\n");
    }
    else
    {
	if (member_array(item, MC->query_categories()) < 0)
	    err_args(board, "domain", item);
	else
	    err_args(board, "category", item);
	return MBS_NO_BOARD;
    }

    return MBS_NO_ERR;
}    

/* **************************************************************************
 * Service functions
 */

/*
 * Function name: select_nur
 * Description:	  Select next unread board, any selection
 * Arguments:	  sw - True, always switch to next
 *		       False, remain at same if it contains unread news
 * Returns:	  1 - if an unread note was found, 0 otherwise
 */
static nomask int
select_nur(int sw)
{
    int		s_ind, ssval, ssz;
    mixed	note_info;
    string	*blist, *slist, swinfo;
    mixed	binfo;

    switch (Selection)
    {
    case ORDER_DOMAIN:
	slist = sort_array(SECURITY->query_domain_list());
	break;

    case ORDER_CAT:
	slist = sort_array(MC->query_categories());
	break;

    default:
	slist = sort_array(Groups);
    }

    ssz = sizeof(slist);
    ssval = s_ind = member_array(CurrItem[Selection], slist);

    if (ssval < 0 || ssval > sizeof(slist))
	ssval = 0;
    do
    {
	if (CurrItem[Selection] != slist[s_ind])
	{
	    CurrItem[Selection] = slist[s_ind];
	    swinfo = "Switching to " +
		({ "category", "domain", "newsgroup" })[Selection] +
		" '" + CurrItem[Selection] + "'.\n";
	}
	else
	    swinfo = "";

	if ((binfo = find_nur_insel(sw))[0])
	{
	    // Write changed selection item info
	    if (strlen(swinfo))
		write(swinfo);

	    // Write changed board info
	    if (strlen(binfo[1]))
		write(binfo[1]);
	    return 1;
	}
	s_ind = (s_ind + 1) < ssz ? (s_ind + 1) : 0;
    } while (ssval != s_ind);
    CurrItem[Selection] = slist[s_ind];

    return 0;
}

/*
 * Function name: find_nur_insel
 * Description:   Find next board with unread board within the selection
 * Arguments:	  sw - Always switch to next board.
 * Returns:	  ({ 1, "board mess }) if new news found, ({ 0 }) otherwise.
 */
static nomask mixed
find_nur_insel(int sw)
{
    int		b_ind, bsval, bsz;
    mixed	note_info;
    string	*blist, info;

    if (sizeof(SbMap[CurrItem[Selection]]))
    {
	blist = map(SbMap[CurrItem[Selection]], &operator([])(, SB_SPATH));
	bsz = sizeof(blist);
	bsval = b_ind = member_array(CurrBoard, blist);
	if (sw)
	    b_ind = (b_ind + 1) < bsz ? (b_ind + 1) : 0;
	if (bsval < 0)
	    bsval = b_ind = 0;
	do
	{
	    note_info = MC->find_next_unread(blist[b_ind], atoi(BdMap[blist[b_ind]][SB_LNOTE][1..]));
	    if (note_info[0] > 0)
	    {
		if (CurrBoard != blist[b_ind])
		{
		    CurrBoard = blist[b_ind];
		    info = "Switching to board '" +
			BdMap[blist[b_ind]][SB_BOARD] + "'\n";
		}
		else
		    info = "";
		shuffle_boards();
		save_mbs();
		return ({ 1, info });
	    }
	    else if (note_info[0] == -1 &&
		     note_info[1] != MBS_NO_RACC)
	    {
		err_args(BdMap[blist[b_ind]][SB_BOARD],
			 BdMap[blist[b_ind]][SB_CAT]);
		mbs_error(note_info[1]);
		return ({ 0 });
	    }
	    
	    b_ind = (b_ind + 1) < bsz ? (b_ind + 1) : 0;
	} while (bsval != b_ind);
    }

    CurrBoard = "";
    return ({ 0 });
}

/*
 * Function name: print_newboard_info
 * Description:   Print new board info
 * Arguments:	  data - the data holder
 */
static nomask void
print_newboard_info(string *data)
{
    string st;
    
    st = MC->query_board_access(data[BBP_SPATH]);
    write(sprintf("%-11s%-11s%-11s%-31s%s\n", data[BBP_BOARD], data[BBP_CAT], data[BBP_DOMAIN], data[BBP_DESC], st));
}

/*
 * Function name: print_subboard_info
 * Description:   Print subscribed board info
 * Arguments:	  data - the data holder
 */
static nomask void
print_subboard_info(string *data)
{
    if (CurrBoard != data[SB_SPATH])
	write(sprintf(" %-11s", data[SB_BOARD]));
    else
	write(sprintf("*%-11s", data[SB_BOARD]));
    if (Selection != ORDER_CAT)
	write(sprintf("%-11s", data[SB_CAT]));
    if (Selection != ORDER_DOMAIN)
	write(sprintf("%-11s", data[SB_DOMAIN]));
    if (Selection != ORDER_GROUP)
	write(sprintf("%-11s", data[SB_GROUP]));
    write(sprintf("%-31s%s\n", MC->query_board_desc(data[SB_SPATH]),
		  MC->query_board_status(data[SB_SPATH], data[SB_LNOTE])));
}

/*
 * Function name: print_shortsub_info
 * Description:   Print short subscribed board info
 * Arguments:	  data - the data holder
 */
static nomask void
print_shortsub_info(mixed data)
{
    write(data[SB_BOARD] + "/" + data[SB_CAT] + ",");
}

/*
 * Function name: err_args()
 * Description:   Set the error arguments
 * Arguments:	  args - list of error arguments
 */
public nomask varargs void
err_args(string arg1, string arg2, string arg3, string arg4, string arg5)
{
    ErrArgs = ({ arg1, arg2, arg3, arg4, arg5 });
}

/*
 * Function name: mbm_error
 * Description:   Error output from mbm commands
 * Arguments:	  what - the error
 * Returns:       1 - an error occurred
 *                0 - no error occurred
 */
static nomask int
mbm_error(int what)
{
    if (what != MBM_NO_ERR)
    {
	write("## MBM ERROR: ");

	switch (what)
	{
	case MBM_BAD_CALL:
	    write("This function was called from an illegal source.\n");
	    break;

	case MBM_NO_AUTH:
	    write("You are not authorized to perform this command.\n");
	    break;
	    
	case MBM_NO_CMD:
	    write("No such command: '" + ErrArgs[0] + "'.\n");
	    break;

	case MBM_WRONG_ARGS:
	    write("Wrong number of arguments.\n");
	    break;

	case MBM_BASE_ADMIN:
	    write("A default mbs admin can't be deleted.\n");
	    break;

	case MBM_NO_WIZ:
	    write("You can only add wizards.\n");
	    break;

	case MBM_IS_ADMIN:
	    write(ErrArgs[0] + " already is an mbs administrator.\n");
	    break;

	case MBM_NO_ADMIN:
	    write(ErrArgs[0] + " is not an admin.\n");
	    break;

	case MBM_BASE_CAT:
	    write("A base category can't be removed or changed.\n");
	    break;

	case MBM_CAT_EXISTS:
	    write("The category '" + ErrArgs[0] + "' already exists.\n");
	    break;

	case MBM_STR_LONG:
	    write("The string '" + ErrArgs[0] + "' is too long.\n");
	    break;

	case MBM_NO_CAT:
	    write("There is no category named '" + ErrArgs[0] + "'.\n");
	    break;

	case MBM_CAT_IN_USE:
	    write("The category '" + ErrArgs[0] + "' is in use, it can't be removed.\n");
	    break;

	case MBM_BAD_BPATH:
	    write("There doesn't exist any board corresponding to the path '" + ErrArgs[0] + "'.\n");
	    break;

	case MBM_BOARD_EXISTS:
	    write("There already is a board named '" + ErrArgs[0] + "' in the category '" + ErrArgs[1] + "'.\n");
	    break;

	case MBM_BOARD_IN_USE:
	    write("The board is already in use as '" + ErrArgs[0] + "' in the category '" + ErrArgs[1] + "'.\n");
	    break;

	case MBM_NUM_BOARDS:
	    write("There are already at least " + MAX_NUM_BOARDS + " boards in your domain.\n");
	    break;
	    
	case MBM_BAD_ORDER:
	    write("You have specified an unknown search/list selection: '" + ErrArgs[0] + "'.\n");
	    break;

	case MBM_NO_DOMAIN:
	    write("There doesn't exist any domain called '" + ErrArgs[0] + "'.\n");
	    break;

	case MBM_NO_BOARD:
	    write("There doesn't exist any board called '" + ErrArgs[0] + "' in the category '" + ErrArgs[1] + "'.\n");
	    break;

	case MBM_ENTRY_USED:
	    write("A board called '" + ErrArgs[0] + "' in the category '" + ErrArgs[1] + "' is registered with that path.\n");
	    break;
	    
	default:
	    write("Strange error '" + what + "'. I have NO idea of what you've done to cause this. Please report to Mrpr ASAP.\n");
	    break;
	}

        return 1;
    }

    return 0;
}

/*
 * Function name: mbs_error
 * Description:   Error output from mbs commands
 * Arguments:	  what - the error
 * Returns:       1 - an error occurred
 *                0 - no error occurred
 */
static nomask int
mbs_error(int what)
{
    if (what != MBS_NO_ERR)
    {
	write("## MBS ERROR: ");

	switch (what)
	{
	case MBS_NO_AUTH:
	    write("You are not authorized to perform this command.\n");
	    break;
	    
	case MBS_NO_CMD:
	    write("No such command: '" + ErrArgs[0] + "'.\n");
	    break;

	case MBS_BAD_CALL:
	    write("This function was called from an illegal source.\n");
	    break;

	case MBS_BAD_ORDER:
	    write("There is no selection '" + ErrArgs[0] + "'.\n");
	    break;

	case MBS_BAD_DOMAIN:
	    write("There is no domain called '" + ErrArgs[0] + "'.\n");
	    break;

	case MBS_BAD_CAT:
	    write("There is no category called '" + ErrArgs[0] + "'.\n");
	    break;

	case MBS_BAD_SEL:
	    write("There is no domain or category called '" + ErrArgs[0] + "'.\n");
	    break;

	case MBS_NO_GROUP:
	    write("You haven't defined any newsgroup to assign boards to.\n");
	    break;

	case MBS_NO_BOARD:
	    write("There is no board by the name '" + ErrArgs[0] + "' in the " + ErrArgs[1] + " '" + ErrArgs[2] + "'\n");
	    break;

	case MBS_SUBSCRIBE:
	    write("You already subscribe to that board.\n");
	    break;

	case MBS_NO_SPEC:
	    write("There is no default board specified. Please provide specific instructions.\n");
	    break;

	case MBS_NO_SUB:
	    if (strlen(ErrArgs[2]))
		write("You are not subscribing to any board named '" + ErrArgs[0] + "' in the " + ErrArgs[1] + " '" + ErrArgs[2] + "'.\n");
	    else
		write("You are not subscribing to any board named '" + ErrArgs[0] + "'.\n");
	    break;

	case MBS_SPEC_AMBIG:
	    if (strlen(ErrArgs[2]))
		write("The specification of the board '" + ErrArgs[0] + "' in the " + ErrArgs[1] + " '" + ErrArgs[2] + "' is ambigous, please be more specific.\n");
	    else
		write("The specification of the board '" + ErrArgs[0] + "' is ambigous, please be more specific.\n");
	    break;

	case MBS_GRP_EXIST:
	    write("The newsgroup '" + ErrArgs[0] + "' already exists.\n");
	    break;

	case MBS_GRP_NONE:
	    write("The newsgroup '" + ErrArgs[0] + "' does not exist.\n");
	    break;
	    
	case MBS_NO_CURR:
	    write("You haven't selected any board.\n");
	    break;

	case MBS_NO_RACC:
	    write("You don't have read access on this board.\n");
	    break;

	case MBS_NO_DACC:
	    write("You don't have delete access on this board.\n");
	    break;

	case MBS_NO_WACC:
	    write("You don't have write access on this board.\n");
	    break;

	case MBS_STR_LONG:
	    write("The string '" + ErrArgs[0] + "' is too long.\n");
	    break;

	case MBS_BAD_BOARD:
	    write("The board '" + ErrArgs[0] + "' in the category '" + ErrArgs[1] + "' is out of order. Please try later.\n");
	    break;

	case MBS_BAD_NNUM:
	    write("There's not that many notes on this board.\n");
	    break;

	case MBS_WRONG_ARGS:
	    write("Wrong number of arguments.\n");
	    break;

	case MBS_SCRAPPED:
	    write("You already have scrapped that board.\n");
	    break;

	case MBS_NUM_ZERO:
	    write("You have to specify a number >= 1.\n");
	    break;

	case MBS_NO_HEADER:
	    write("You have to specify a header for your note.\n");
	    break;

	default:
	    write("Strange error '" + what + "'. I have NO idea of what you've done to cause this. Please report to Mrpr ASAP.\n");
	    break;
	}

        return 1;
    }

    return 0;
}

/*
 * Function name: save_mbs
 * Description:	  Save the data for the user
 */
static nomask void
save_mbs()
{
    MC->save_mbs(([
		   "sel" : Selection,
		   "sbm" : SbMap,
		   "bdm" : BdMap,
		   "grps" : Groups,
		   "curri" : CurrItem,
		   "currb" : CurrBoard,
		   "scrapl" : ScrapList,
		   "gcc": GcCheck,
	       ]));
}

/*
 * Function name: restore_mbs
 * Description:	  Restore the data for the user
 */
static nomask void
restore_mbs()
{
    mapping data;

    data = MC->restore_mbs();

    if (!m_sizeof(data))
    {
	write("You have no personal mbs save file, loading defaults ...\n");
        data = MC->default_mbs();
    }

    if (!m_sizeof(data))
    {
	write("Default mbs save file not available, generating new settings ...\n");
	Selection = ORDER_GROUP;
	BdMap = ([]);
	SbMap = ([]);
	Groups = ({ "Boards" });
	CurrItem = ({ "", "", "Boards" });
	CurrBoard = "";
	ScrapList = ({ });
	GcCheck = time();
    }
    else
    {
	Selection = data["sel"];
	SbMap = data["sbm"];
	BdMap = data["bdm"];
	Groups = data["grps"];
	CurrItem = data["curri"];
	CurrBoard = data["currb"];
	ScrapList = data["scrapl"];
	GcCheck = data["gcc"];
    }
}

/*
 * Function name: shuffle_boards
 * Description:	  Shuffle the board mapping so that it reflects on the
 *		  current selection.
 */
static nomask void
shuffle_boards()
{
    mixed	blist;
    string	*doms, *caths;

    if (!m_sizeof(BdMap))
	return;
    
    /* Start by concatting the list of boards */
    blist = m_values(BdMap);

   /* Now sort the list */

    switch (Selection)
    {
    case ORDER_CAT:
	caths = MC->query_categories();
	blist = map(caths, &filt_bdata(, blist, SB_CAT));
	SbMap = mkmapping(caths, blist);
	SbMap = filter(SbMap, sizeof);
	if (!sizeof(SbMap[CurrItem[ORDER_CAT]]))
	    CurrItem[ORDER_CAT] = m_indexes(SbMap)[0];
	break;

    case ORDER_DOMAIN:
	doms = SECURITY->query_domain_list();
	blist = map(doms, &filt_bdata(, blist, SB_DOMAIN));
	SbMap = mkmapping(doms, blist);
	SbMap = filter(SbMap, sizeof);
	if (!sizeof(SbMap[CurrItem[ORDER_DOMAIN]]))
	    CurrItem[ORDER_DOMAIN] = m_indexes(SbMap)[0];
	break;

    case ORDER_GROUP:
	/* FALLTHROUGH */
    default:
	blist = map(Groups, &filt_bdata(, blist, SB_GROUP));
	SbMap = mkmapping(Groups, blist);
	SbMap = filter(SbMap, sizeof);
	if (!sizeof(SbMap[CurrItem[ORDER_GROUP]]))
	    CurrItem[ORDER_GROUP] = m_indexes(SbMap)[0];
	break;
    }
}

/*
 * Function name: filt_unread_news
 * Description:	  Filter to distinguish boards with unread news
 * Arguments:	  list - the board list
 * Returns:	  1/0
 */
static nomask int
filt_unread_news(mixed list)
{
    return (MC->query_unread_news(list[SB_SPATH], list[SB_LNOTE]));
}

/*
 * Function name: filt_bdata
 * Description:	  Filter board data for shuffle_boards
 * Arguments:	  sort_item - the item to look for
 *		  list - the list to look in
 *		  ind - the index to look at.
 * Returns:	  The matching array
 */
static nomask mixed
filt_bdata(string sort_item, mixed list, int ind)
{
    return filter(list, &operator(==)(sort_item) @ &operator([])(, ind));
}

/*
 * Function name: set_time
 * Description:	  Set the read time of a particular note
 * Arguments:	  tm - the time to set
 *		  spath - the save path to the board
 */
static nomask void
set_time(string tm, string spath)
{
    BdMap[spath][SB_LNOTE] = tm;
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

    it1 = item1[SB_DOMAIN] + item1[SB_BOARD];
    it2 = item2[SB_DOMAIN] + item2[SB_BOARD];
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

    it1 = item1[SB_CAT] + item1[SB_BOARD];
    it2 = item2[SB_CAT] + item2[SB_BOARD];
    if (it1 < it2)
	return -1;
    if (it1 > it2)
	return 1;
    return 0;
}

/*
 * Function name: check_gc
 * Description:	  Check for gc changes
 */
static nomask void
check_gc()
{
    int		i, sz;
    mixed	list, blist;
    
    if (MC->query_gc_time() > GcCheck)
    {
	write("A global change has taken place, syncing mbs.\n");
	GcCheck = time();

	list = m_indices(BdMap);
	for (i = 0, sz = sizeof(list) ; i < sz ; i++)
	{
	    blist = MC->get_board_by_path(list[i]);
	    if (sizeof(blist) && strlen(blist[BBP_BOARD]))
	    {
		if (BdMap[list[i]][SB_BOARD] != blist[BBP_BOARD] ||
		    BdMap[list[i]][SB_CAT] != blist[BBP_CAT] ||
		    BdMap[list[i]][SB_DOMAIN] != blist[BBP_DOMAIN])
		{
		    write("Board change: '" +
			  BdMap[list[i]][SB_BOARD] + "/" +
			  BdMap[list[i]][SB_CAT] + "/" +
			  BdMap[list[i]][SB_DOMAIN] + "' -> '" +
			  blist[BBP_BOARD] + "/" +
			  blist[BBP_CAT] + "/" +
			  blist[BBP_DOMAIN] + "'\n");
		    BdMap[list[i]][SB_BOARD] = blist[BBP_BOARD];
		    BdMap[list[i]][SB_CAT] = blist[BBP_CAT];
		    BdMap[list[i]][SB_DOMAIN] = blist[BBP_DOMAIN];
		}
	    }
	    else
	    {
		write("The board '" + BdMap[list[i]][SB_BOARD] +
		      "' in the category '" + BdMap[list[i]][SB_CAT] +
		      "' has been deleted from the central mbs service.\n");
		BdMap = m_delete(BdMap, list[i]);
	    }
	}
	
	shuffle_boards();
	save_mbs();
    }
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

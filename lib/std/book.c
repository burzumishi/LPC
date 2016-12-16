/*
 * /std/book.c
 *
 * A general book with many pages. You have to open the book and turn it to
 * the right page in order to read it.
 *
 */
#pragma strict_types

inherit "/std/scroll";

#include <stdproperties.h>
#include <macros.h>
#include <language.h>
#include <cmdparse.h>

/*
 * Prototype
 */
varargs void	read_book_at_page(int page, string verb);
int read_scroll(string str);

int 	book_is_closed;
int 	what_page;
int 	maxm_page;
string 	gPage;

/*
 * Function name: create_book
 * Description:   creates a default book
 *                change it to make your own book
 */
public void
create_book()
{
    set_long("A empty looking book.\n");
}

/*
 * Function name: create_scroll
 * Description:   creates the general object
 * Arguments:	  
 */
nomask void
create_scroll() 
{
    add_name("book");
    book_is_closed = 1;
    what_page = 1;
    add_prop(OBJ_I_WEIGHT, 700);
    add_prop(OBJ_I_VOLUME, 400);
    add_prop(OBJ_I_VALUE, 200);
    create_book();
}

/*
 * Function name: init
 * Description:   initialise the commands 
 * Arguments:	  
 */
void
init()
{
    ::init();

    add_action(read_scroll, "close");
    add_action(read_scroll, "open");
    add_action(read_scroll, "turn");
}

/*
 * Function name: open_me
 * Description:   opens the book (at page one!)
 */
void
open_me()
{
    if (!book_is_closed)
    {
        write("The " + short(this_player()) + " is already open.\n");
	return;
    }

    what_page = 1;

    write("You open the " + short(this_player()) + " at page " +
	LANG_WNUM(what_page) + ".\n");
    say(QCTNAME(this_player()) + " opens the " + QSHORT(this_object()) + ".\n");

    book_is_closed = 0;
}

/*
 * Function name: close_me
 * Description:   closes the book again
 */
void
close_me()
{
    if (book_is_closed)
    {
        write("The " + short(this_player()) + " is already closed.\n");
        return;
    }

    write("You close the " + short(this_player()) + ".\n");
    say(QCTNAME(this_player()) + " closes the " + QSHORT(this_object()) + ".\n");

    what_page = 1;
    book_is_closed = 1;
}

/*
 * Function name: turn_me
 * Description:   turn the book to the next page
 */
void
turn_me()
{
    int appr_num;

    gPage = previous_object()->query_gPage();

    if (book_is_closed) 
    {
	write("But the " + short(this_player()) + " is closed.\n");
	return;
    }

    appr_num = LANG_NUMW(gPage);
    if (appr_num > 0 && appr_num < maxm_page + 1)
    {
        what_page = appr_num;

        write("You turn the " + short(this_player()) + " to page " +
		what_page + ".\n");
        return;
    }

    if (gPage == "forward" || gPage == "") 
    {
        if (maxm_page < what_page + 1) 
        {
            write("You have reached the last page of the " +
			short(this_player()) + ".\n");    
            return;
	}
        what_page += 1;
        if (maxm_page == what_page)
            write("You turn the " + short(this_player()) +
			" to the last page.\n");
        else
            write("You turn the " + short() +
                  " to page " + LANG_WNUM(what_page) + ".\n");
        return;
    }
    else if (gPage == "backward" || gPage == "back")
    {
        if (what_page == 1)
    	{
            write("You cannot turn the " + short(this_player()) +
                  " below the first page.\n");
            return;
        }
        what_page -= 1;
        write("You turn the " + short(this_player()) +
             " to page " + LANG_WNUM(what_page) + ".\n");
	return;
    }
    else
    {
	write("Do you want to turn the page 'forward' or 'backward'?\n");
	return;
    }
}

/*
 * Function name: set_max_page
 * Description:   sets the number of pages of the book
 * Arguments:	  how_many - how many pages ?
 */
void
set_max_pages(int how_many) 
{ 
    maxm_page = how_many; 
}

int query_max_pages() { return maxm_page; }

/*
 * Function name: read_scroll
 * Description:   We need some special stuff for the turn page command
 *		  The turn page has the following syntax
 *		    turn page - turn forward one page
 *		    turn page forward/[backward, back] - turn one page
 *			in the appropriate direction
 *		    turn book to/at page <num> - turn to page <num>
 *			where <num> is a string like one, two, eight,
 *			not an integer like 7.
 */
static int
read_scroll(string str)
{
    string where, what;

    if (!str)
	return ::read_scroll(what);

    if (str == "page")
    {
	gPage = "forward";
	what = "book";
    }
    else if (parse_command(str, ({}), "'page' [to] %w", where)) 
    {
	gPage = where;
	what = "book";
    }
    else if (!parse_command(str, ({}), "%s 'at' / 'to' 'page' %w", what, where))
    {
	gPage = "";
	what = str;
    }
    else
	gPage = where;

    return ::read_scroll(what);
}

/*
 * Function name: read_it
 * Description:   If player wanted to do anything to this book we end up here.
 * Arguments:	  verb - The verb the player had used
 */
void
read_it(string verb)
{
    switch (verb)
    {
    case "read":
    case "mread":
	if (book_is_closed)
	{
	    write("The " + short(this_player()) + " is closed.\n");
	    return;
	}
	say(QCTNAME(this_player()) + " reads the " + QSHORT(this_object()) +
	    ".\n");
	read_book_at_page(what_page, verb); break;
    case "turn":
	turn_me(); break;
    case "open":
	open_me(); break;
    case "close":
	close_me(); break;
    }
}

/*
 * Function name: read_book_at_page
 * Description:   should be redefined in your book. is called from read_me
 * Arguments:	  which - read the book at which page
 *		  verb  - If the player wanted to read it, or mread it.
 *			  To mread something, you can look how the scoll.c does
 *			  it in read_it() if needed.
 */
varargs void
read_book_at_page(int which, string verb)
{ 
    write("This is only a blafasel book with the same on every page.\n");
}

void
leave_env(object from, object to)
{
    book_is_closed = 1;
    ::leave_env(from, to);
}

/*
 * Function name: query_gPage
 * Description:   Ask what page info the player gave.
 * Returns:	  The same string the player gave
 */
string
query_gPage()
{
    return gPage;
}


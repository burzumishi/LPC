/* 
 * /lib/library.c
 *
 * Support for libraries.
 *
 * Example usage:
 * 
 * inherit "/std/room";
 * inherit "/lib/library";
 *
 * void
 * create_room()
 * {
 *     set_short("Library");
 *     set_long("This is a library.\n");
 *    
 *     // Set the directory where book files will be stored
 *     set_book_directory("/d/Domain/subdir/");
 *
 *     // Make two shelves, one for maps and one for other books
 *     add_book_shelf(({ "general", "maps" }));
 *
 *     // Initialize the library
 *     create_library();
 * 
 *     // Add a sign with help information
 *     add_item("sign", library_help());
 *     add_cmd_item("sign", "read", library_help());
 * }
 *
 * void
 * init()
 * {
 *     ::init();
 *
 *     // Add the library commands
 *     init_library();
 * }
 *
 * N.B.  Directories given in configuration must exist for the library to
 *       function properly.  If a shelf is added, a directory with the
 *       same name as the shelf must exist under the the directory given
 *       to set_book_directory().
 */

#include <cmdparse.h>
#include <files.h>
#include <language.h>
#include <macros.h>
#include <mail.h>
#include <stdproperties.h>

#define BOOK_TYPE  book_types[0]
#define PLURAL_BOOK_TYPE \
    (sizeof(plural_book_types) ? \
    plural_book_types[0] : LANG_PWORD(book_types[0]))
  
#define SHELF_TYPE shelf_types[0]
#define PLURAL_SHELF_TYPE \
    (sizeof(plural_shelf_types) ? \
    plural_shelf_types[0] : LANG_PWORD(shelf_types[0]))

#define BOOK_ID    (MASTER + "_library_book")
#define DEFAULT_TEXT_FILE_NAME "book"

#define TITLE_LINE   1
#define SUMMARY_LINE 2
#define AUTHOR_LINE  3
#define TEXT_LINE    4

#define MAX_TITLE_SIZE 20

static int borrow_required;

static string  book_dir  = "",
               appr_dir  = "",
               removal_dir = "",
               book_list = "",
               book_list_short = "",
               appr_list = "",
               appr_list_short = "",
               text_file_name = DEFAULT_TEXT_FILE_NAME,
              *book_types = ({ "book" }),
              *plural_book_types = ({ "books" }),
              *shelf_types = ({ "shelf" }),
              *plural_shelf_types = ({ "shelves" }),
               default_shelf = "";

static mapping book_map;
static mapping appr_map;
static mapping book_shelves = ([]);

public void done_writing(string title, string summary, string input);
public int library_approve_access();
public string get_book_name(string dir);

/*
 * Configuration functions
 */

/*
 * Function name: set_book_directory
 * Description:   Set the directory where books will be stored
 * Arguments:     string dir - the directory name with trailing slash
 */
public void
set_book_directory(string dir)
{
    book_dir = dir;
}

/*
 * Function name: set_book_approval_directory
 * Description:   Set the directory where unapproved books will be stored.
 *                If this is not set, new books will not need approval.
 * Arguments:     string dir - the directory name with trailing slash.
 */
public void
set_book_approval_directory(string dir)
{
    appr_dir = dir;
}

/*
 * Function name: set_book_removal_directory
 * Description:   Set the directory where removed (denied or discarded) books
 *                are placed.  If this is not set, the files will simply be
 *                deleted.
 * Arguments:     string dir - the directory name with trailing slash.
 */
public void
set_book_removal_directory(string dir)
{
    removal_dir = dir;
}

/*
 * Function name: set_text_file_name
 * Description:   Set the prefix for the filenames of the books.
 * Arguments:     string name - the filename prefix
 */
public void
set_text_file_name(string name)
{
    text_file_name = name;
}

/*
 * Function name: set_book_type
 * Description:   Set the names to be used to identify books in the library.
 *                The argument to this function is passed directly to
 *                the set_name() function on each book instance.
 * Arguments:     mixed types - a string name or array of string names
 */
public void
set_book_type(mixed types)
{
    if (stringp(types))
    {
        book_types = ({ types });
    }
    else if (pointerp(types))
    {
        book_types = types;
    }
}

/*
 * Function name: set_plural_book_type
 * Description:   Set the names to be used to identify multiple books.
 *                The argument to this function is passed directory to
 *                the set_pname function on each book instance.
 * Arguments:     mixed types - a string name or array of string names
 */
public void
set_plural_book_type(mixed types)
{
    if (stringp(types))
    {
        plural_book_types = ({ types });
    }
    else if (pointerp(types))
    {
        plural_book_types = types;
    }
}

/*
 * Function name: set_borrow_required
 * Description:   Indicate that that books must be borrowed from
 *                the library in order to be read.
 * Arguments:     int i - if true, books must be borrowed
 */
public void
set_borrow_required(int i)
{
        borrow_required = i;
}

/*
 * Function name: add_book_shelf
 * Description:   Add book shelves to the library.  This allows books to
 *                be listed according to categories.
 * Arguments:     mixed shelf - a shelf name or array of shelf names.  These
 *                              are used both as names for the shelves when
 *                              players list shelves and also as subdirectories
 *                              (under the directory specified by
 *                              set_book_directory()) in which the book files
 *                              for each shelf will be stored.
 */
public void
add_book_shelf(mixed shelf)
{
    int i;

    if (!pointerp(shelf))
    {
        shelf = ({ shelf });
    }

    if (!strlen(default_shelf))
    {
        default_shelf = shelf[0];
    }

    for (i = 0; i < sizeof(shelf); i++)
    {
        book_shelves[shelf[i]] = 0;
    }
}

/*
 * Function name: set_default_shelf
 * Description:   Set the default shelf to be used when none is specified
 *                by the code or player.
 * Arguments:     string shelf - the name of the shelf to use (one of the
 *                               shelf names given in add_book_shelf())
 */
public void
set_default_shelf(string shelf)
{
    default_shelf = shelf;
}

/*
 * Function name: set_shelf_type
 * Description:   Set the names by which a shelf can be identified in case
 *                "shelf" is not appropriate.
 * Arguments:     mixed types - a string name or array of string names
 */
public void
set_shelf_type(mixed types)
{
    if (stringp(types))
    {
        shelf_types = ({ types });
    }
    else if (pointerp(types))
    {
        shelf_types = types;
    }
}

/*
 * Function name: set_plural_shelf_type
 * Description:   Set the names by which multiple shelves can be identified
 *                in case "shelves" is not appropriate.
 * Arguments:     mixed types - a string name or array of string names
 */
public void
set_plural_shelf_type(mixed types)
{
    if (stringp(types))
    {
        plural_shelf_types = ({ types });
    }
    else if (pointerp(types))
    {
        plural_shelf_types = types;
    }
}

/*
 * Hooks for altering default messages
 */

/*
 * Function name: library_no_books_hook
 * Description:   Redefine this to alter the message given when commands
 *                fail because the library is empty.
 * Returns:       1/0
 */
public int
library_no_books_hook()
{
    write("There are no books available.\n");
    return 1;
}

/*
 * Function name: library_no_approval_books_hook
 * Description:   Redefine this to alter the message given when commands
 *                fail because there are no books needing approval
 * Returns:       1/0
 */
public int
library_no_approval_books_hook()
{
    write("There are no books needing approval.\n");
    return 1;
}

/*
 * Function name: library_list_short_hook
 * Description:   Redefine this to alter how the short listing of books
 *                is presented
 */
public void
library_list_short_hook()
{
    write(book_list_short);
}

/*
 * Function name: library_list_long_hook
 * Description:   Redefine this to alter how the long listing of books is
 *                presented
 */
public void
library_list_long_hook()
{
    write(book_list);
}

/*
 * Function name: library_list_approval_long_hook
 * Description:   Redefine this to alter how the long listing of books
 *                needing approval is presented
 */
public void
library_list_approval_long_hook()
{
    write(appr_list);
}

/* 
 * Function name: library_list_approval_short_hook
 * Description:   Redefine this to alter how the short listing of books
 *                needing approval is presented
 */
public void
library_list_approval_short_hook()
{
    write(appr_list_short);
}

/*
 * Function name: library_no_shelves_hook
 * Description:   Redefine this to alter the message given when a command
 *                fails because no shelves exist
 * Returns:       1/0
 */
public int
library_no_shelves_hook()
{
    notify_fail("There are no different " + SHELF_TYPE + ".\n");
    return 0;
}
   
/*
 * Function name: library_list_shelves_hook
 * Description:   Redefine this to alter how the listing of shelves is
 *                presented
 */
public void
library_list_shelves_hook()
{
    write(implode(map(m_indices(book_shelves), capitalize), "\n") + "\n");
}

/*
 * Function name: library_list_shelf_long_hook
 * Description:   Redefine this to alter how the long listing of books
 *                on a given shelf is presented
 * Arguments:     string shelf - the name of the shelf
 */
public void
library_list_shelf_long_hook(string shelf)
{
    write(book_shelves[shelf][0]);
}

/*
 * Function name: library_list_shelf_short_hook
 * Description:   Redefine this to alter how the short listing of books
 *                on a given shelf is presented
 * Arguments:     string shelf - the name of the shelf
 */
public void
library_list_shelf_short_hook(string shelf)
{
    write(book_shelves[shelf][1]);
}

/*
 * Function name: library_list_syntax_failure_hook
 * Description:   Redefine this to alter the message given when
 *                incorrect syntax is used for the "list" command
 * Arguments:     string str - the arguments to the "list" command
 * Returns:       1/0
 */
public int
library_list_syntax_failure_hook(string str)
{
    notify_fail("Usage: " + query_verb() + 
        (strlen(appr_dir) ? " [approval]" : "") + " [titles]\n");
    return 0;
}

/*
 * Function name: library_borrow_syntax_failure_hook
 * Description:   Redefine this to alter the message given when
 *                incorrect syntax is used for the "borrow" or
 *                "read" command
 * Arguments:     string str - the arguments to the "borrow" command
 * Returns:       1/0
 */
public int
library_borrow_syntax_failure_hook()
{
    notify_fail("Usage: " + query_verb() + " <title>\n");
    return 0;
}

/* 
 * Function name: library_borrow_unavailable_approval_book_hook
 * Description:   Redefine this to alter the message given when a
 *                user attempts to borrow or read a nonexistant book from
 *                books needing approval
 * Arguments:     string title - the title of the book
 * Returns:       1/0
 */
public int
library_borrow_unavailable_approval_book_hook(string title)
{
    write("There is no such " + BOOK_TYPE + " available for approval.\n");
    return 1;
}

/*
 * Function name: library_borrow_unavailable_book_hook
 * Description:   Redefine this to alter the message given when a
 *                user attempts to borrow or read a nonexistant book
 * Arguments:     string title - the title of the book
 * Returns:       1/0
 */
public int
library_borrow_unavailable_book_hook(string title)
{
    write("There is no such " + BOOK_TYPE + " available.\n");
    return 1;
}

/*
 * Function name: library_borrow_hook
 * Description:   Redefine this to alter the message given when a
 *                user borrows or reads a book
 * Arguments:     object book  - the book borrowed
 *                string title - the title of the book
 */
public void
library_borrow_hook(object book, string title)
{
    write("You borrow " + LANG_ADDART(book->short()) + ".\n");
    say(QCTNAME(this_player()) + " borrows " + LANG_ADDART(book->short()) +
        ".\n");
}

/*
 * Function name: library_return_syntax_failure_hook
 * Description:   Redefine this to alter the message given when incorrect
 *                syntax is used for the "return" command
 * Arguments:     string str - the arguments to the "return" command
 * Returns:       1/0
 */
public int
library_return_syntax_failure_hook(string str)
{
    notify_fail(capitalize(query_verb()) + " what?\n");
    return 0;
}

/*
 * Function name: library_return_hook
 * Description:   Redefine this to alter the message given when a
 *                user returns a book
 * Arguments:     object book - the book being returned
 */
public void
library_return_hook(object book)
{
    write("You return the " + book->short() + ".\n");
    say(QCTNAME(this_player()) + " returns " + LANG_ADDART(book->short()) + 
        ".\n");
}

/*
 * Function name: library_write_abort_hook
 * Description:   Redefine this to alter the message given when the
 *                user aborts writing a book
 */
public void
library_write_abort_hook()
{
    write("You stop writing.\n");
}

/*
 * Function name: library_write_prompt_title_input_hook
 * Description:   Redefine this to alter the message given to prompt
 *                the user to input a book title
 */
public void
library_write_prompt_title_input_hook()
{
    write("What is the name of the book? (fewer than " + MAX_TITLE_SIZE +
        " characters).  (~q to quit)\n> ");
}
       
/*
 * Function name: library_write_prompt_summary_input_hook
 * Description:   Redefine this to alter the message given to prompt
 *                the user to input a book summary
 */
public void
library_write_prompt_summary_input_hook()
{
    write("\nEnter a short summary of the book. (~q to quit)\n> ");
}

/*
 * Function name: library_write_failed_hook
 * Description:   Redefine this to alter the message given when a book
 *                cannot be saved for some reason.
 */
public void
library_write_failed_hook()
{
    write("Failed to write book.\n");
}

/*
 * Function name: library_approve_syntax_failure_hook
 * Description:   Redefine this to alter the message given when incorrect
 *                syntax is used for the "approve" command
 * Arguments:     string str - the arguments to the "approve" command
 * Returns:       1/0
 */
public int
library_approve_syntax_failure_hook(string str)
{
    notify_fail("Usage: " + query_verb() + " <title>\n");
    return 0;
}

/*
 * Function name: library_approve_unavailable_book_hook
 * Description:   Redefine this to alter the message given when a
 *                user attempts to approve a nonexistant book
 * Arguments:     string title - the book's title
 * Returns:       1/0
 */
public int
library_approve_unavailable_book_hook(string title)
{
    write("There is no book by that title that needs approval.\n");
    return 1;
}

/*
 * Function name: library_approve_hook
 * Description:   Redefine this to alter the message given when a
 *                user approves a book
 * Arguments:     string title - the book's title
 */
public void
library_approve_hook(string title)
{
    write("Ok.\n");
}

/*
 * Function name: library_classify_syntax_failure_hook
 * Description:   Redefine this to alter the message given when incorrect
 *                syntax is used for the "classify" command
 * Arguments:     string str - the arguments to the "classify" command
 * Returns:       1/0
 */
public int
library_classify_syntax_failure_hook(string str)
{
    notify_fail("Usage: " + query_verb() + " <title>.\n");
    return 0;
}

/*
 * Function name: library_classify_unavailable_book_hook
 * Description:   Redefine this to alter the message given when a
 *                user attempts to classify a nonexistant book
 * Arguments:     string title - the book's title
 * Returns:       1/0
 */
public int
library_classify_unavailable_book_hook(string title)
{
    notify_fail("There is no such " + BOOK_TYPE + " to classify.\n");
    return 0;
}

/*
 * Function name: library_classify_unavailable_shelf_hook
 * Description:   Redefine this to alter the message given when a
 *                user attempts to classify a book under a nonexistant
 *                shelf
 * Arguments:     string shelf - the shelf name
 */ 
public void
library_classify_unavailable_shelf_hook(string shelf)
{
    write("There is no such " + SHELF_TYPE + ".\n");
}

/* 
 * Function name: library_classify_hook
 * Description:   Redefine this to alter the message given when a
 *                user classifies a book
 * Arguments:     string title - the book's title
 *                string shelf - the shelf
 */
public void
library_classify_hook(string title, string shelf)
{
    write("Ok.\n");
}

/*
 * Function name: library_classify_prompt_shelf_input_hook
 * Description:   Redefine this to alter the message given to prompt
 *                the user to input a shelf name
 * Arguments:     string title - the title of the book being classified
 */
public void
library_classify_prompt_shelf_input_hook(string title)
{
    write("Which " + SHELF_TYPE + " do you wish to place " + title +
        " in?\n> ");
}

/*
 * Function name: library_deny_syntax_failure_hook
 * Description:   Redefine this to alter the message given when incorrect
 *                syntax is used for the "deny" command
 * Arguments:     string str - the arguments to the "deny" command
 * Returns:       1/0
 */
public int
library_deny_syntax_failure_hook(string str)
{
    notify_fail("Usage: " + query_verb() + " <title>\n");
    return 0;
}

/*
 * Function name: library_deny_unavailable_book_hook
 * Description:   Redefine this to alter the message given when a
 *                user attempts to deny a nonexistant book
 * Arguments:     string title - the book's title
 * Returns:       1/0
 */
public int
library_deny_unavailable_book_hook(string title)
{
    write("There is no book by that title that needs approval.\n");
    return 1;
}

/*
 * Function name: library_deny_hook
 * Description:   Redefine this to alter the message given when a
 *                user denies a book 
 * Arguments:     string title - the book's file
 */
public void
library_deny_hook(string title)
{
    write("Ok.\n");
}

/*
 * Function name: library_discard_syntax_failure_hook
 * Description:   Redefine this to alter the message given when incorrect
 *                syntax is used for the "discard" command
 * Arguments:     string str - the arguments to the "discard" command
 * Returns:       1/0
 */
public int
library_discard_syntax_failure_hook(string str)
{
    notify_fail("Usage: " + query_verb() + " <title>\n");
    return 0;
}

/*
 * Function name: library_discard_unavailable_book_hook
 * Description:   Redefine this to alter the message given when a
 *                user attempts to discard a nonexistant book
 * Arguments:     string title - the book's title
 * Returns:       1/0
 */
public int
library_discard_unavailable_book_hook(string title)
{
    write("There is no " + BOOK_TYPE + " by that title.\n");
    return 1;
}

/*
 * Function name: library_discard_hook
 * Description:   Redefine this to alter the message given when a
 *                user discards a book
 * Arguments:     string title - the book discarded
 */
public void
library_discard_hook(string title)
{
    write("Ok.\n");
}

/*
 * Library routines
 */

/*
 * Function name: library_remove_book
 * Description:   Remove a book from the library.  If a removal directory
 *                is specified, the file will be moved there; otherewise,
 *                it is deleted.
 * Arguments:     string filename - the filename of the book file
 */
public void
library_remove_book(string filename)
{
    setuid();
    seteuid(getuid());

    if (strlen(removal_dir))
    {
        rename(filename, get_book_name(removal_dir));
    }
    else
    {
        rm(filename);
    }
}

/*
 * Function name: library_move_book
 * Description:   Move a book file to another directory.
 * Arguments:     string book_file - the filename of the book file
 *                string new_dir   - the destination directory
 * Returns:       1/0 - move successful/unsuccessful
 */
public int
library_move_book(string book_file, string new_dir)
{
    setuid();
    seteuid(getuid());

    return rename(book_file, get_book_name(new_dir));
}

/*
 * Function name: query_book_title
 * Description:   Given a book file, return the book's title
 * Arguments:     string file - The filename for the desired book
 * Returns:       The book's title
 */
public string
query_book_title(string file)
{
    string str = read_file(file, TITLE_LINE, 1);

    /* remove trailing "\n" */
    if (strlen(str) && (str[-1..] == "\n"))
    {
        str = str[..-2];
    }

    /* remove extra whitespace and return */
    return implode(explode(str, " ") - ({ "" }), " ");
}

/*
 * Function name: query_book_author
 * Description:   Given a book file, return the book's author
 * Arguments:     string file - the filename for the desired book
 * Returns:       The book's author
 */
public string
query_book_author(string file)
{
    string str = read_file(file, AUTHOR_LINE, 1);

    /* remove trailing "\n" */
    if (strlen(str) && (str[-1..] == "\n"))
    {
        str = str[..-2];
    }

    /* remove extra whitespace and return */
    return implode(explode(str, " ") - ({ "" }), " ");
}

/*
 * Function name: query_book_summary
 * Description:   Given a book file, return the book's summary
 * Arguments:     string file - the filename for the desired book
 * Returns:       The book's author
 */
public string
query_book_summary(string file)
{
    string str = read_file(file, SUMMARY_LINE, 1);

    /* remove trailing "\n" */
    if (strlen(str) && (str[-1..] == "\n"))
    {
        str = str[..-2];
    }

    /* remove extra whitespace and return */
    return implode(explode(str, " ") - ({ "" }), " ");
}

/*
 * Function name: query_book_text
 * Description:   Given a book file, return the book's text
 * Arguments:     string file - the filename for the desired book
 * Returns:       the book's text
 */
public string
query_book_text(string file)
{
    return read_file(file, TEXT_LINE);
}

/*
 * Function name: get_books
 * Description:   Get the full pathnames of all book files in a given directory
 * Arguments:     string dir - the directory to search
 * Returns:       An array of pathname strings
 */
public string *
get_books(string dir)
{
    string *books;

    setuid();
    seteuid(getuid());

    books = map(get_dir(dir), &operator(+)(dir));

    /* Any non-empty file is counted as a book */
    books = filter(books, &operator(>)(,0) @ file_size);

    return books;
}
   
/*
 * Function name: get_book_info
 * Description:   For each of a given array of book files, add an element
 *                to a given mapping.  The elements map book title names
 *                to book filenames.
 * Arguments:     string *books - An array of book files.
 *                mapping m     - A mapping in which to store the above
 *                                mentioned information.
 */ 
public void
get_book_info(string *books, mapping m)
{
    int i;

    for (i = 0; i < sizeof(books); i++)
    {
        m[lower_case(query_book_title(books[i]))] = books[i];
    }
}

/*
 * Function name: format_book_list
 * Description:   Given an array of book files, return a nicely formatted
 *                string which lists the given books.
 * Arguments:     string *books - an array of book filename strings
 * Returns:       A string description of the given books
 */
public string
format_book_list(string *books)
{
    int i;
    string str = "";

    for (i = 0; i < sizeof(books); i++)
    {
        str += sprintf("%-" + MAX_TITLE_SIZE + "s %-13s %-=40s\n",
            query_book_title(books[i]), query_book_author(books[i]),
            query_book_summary(books[i]));
    }

    return str;
}

/*
 * Function name: format_book_list_short
 * Description:   Return a short version of the book listing (See documentation
 *                for format_book_list())
 * Arguments:     string *books - an array of book filename strings
 * Returns:       A string description of the given books
 */
public string
format_book_list_short(string *books)
{
    string *titles = map(books, query_book_title);
    return sprintf("%-#70.2s\n", implode(titles, "\n"));
}

/*
 * Function name: update_books
 * Description:   Update the in-memory book information when something
 *                has changed.
 */
public void
update_books()
{
    string *books, *shelves, shelf_list, shelf_list_short;
    int i;

    setuid();
    seteuid(getuid());

    book_map = ([]);
    appr_map = ([]);
    book_list = "";
    book_list_short = "";

    if (strlen(book_dir))
    {
        if (m_sizeof(book_shelves))
        {
            shelves = m_indices(book_shelves);

            for (i = 0; i < sizeof(shelves); i++)
            {
                books = get_books(book_dir + shelves[i] + "/");
                shelf_list = format_book_list(books);
                book_list += shelf_list;
                shelf_list_short = format_book_list_short(books);
                book_list_short += shelf_list_short;
                book_shelves[shelves[i]] = ({ shelf_list, shelf_list_short });
                get_book_info(books, book_map);
            }
        }
        else
        {
            books = get_books(book_dir);
            book_list = format_book_list(books);
            book_list_short = format_book_list_short(books);
            get_book_info(books, book_map);
        }
    }

    if (strlen(appr_dir))
    {
        books = get_books(appr_dir);
        appr_list = format_book_list(books);
        appr_list_short = format_book_list_short(books);
        get_book_info(books, appr_map);
    }
}

/*
 * Function name: create_library
 * Description:   Initialize the library.  This should be called when the
 *                library object is created, AFTER it has been configured.
 */
public void
create_library()
{
    update_books();
}

/*
 * Function name: library_configure_book
 * Description:   Configure a library book.  Redefine this to customize
 *                your library books.
 * Arguments:     object book           - the book object
 *                string text_filename - the file name of the book file to use
 *                string book_name     - the name of the book
 */
public void
library_configure_book(object book, string text_filename, string book_name)
{
    book->set_long("A " + BOOK_TYPE + " entitled \"" + book_name + "\".\n");
    book->set_name(book_types);
    
    if (sizeof(plural_book_types))
    {
        book->set_pname(plural_book_types);
    }

    book->add_name(book_name);
    book->add_prop(OBJ_I_NO_DROP, 1);
    book->set_file(text_filename);
}

/*
 * Function name: library_make_book
 * Description:   Create a book object for use in the library.  Override this
 *                if you do not wish to use the standard scroll object for
 *                books.
 * Arguments:     string book_name - the name of the book we are creating
 * Returns:       A book object
*/
public object
library_make_book(string book_name)
{
    setuid();
    seteuid(getuid());

    return clone_object(SCROLL_OBJECT);
}

/*
 * Function name: library_give_book
 * Description:   Clone and configure a book
 * Arguments:     string text_filename - The filename of the book file to use
 *                string book_name     - The title of the book
 * Returns:       A configured book
 */
public object
library_give_book(string text_filename, string book_name)
{
    object book;

    book = library_make_book(book_name);
    library_configure_book(book, text_filename, book_name);

    /* Add a name unique to this library */
    book->add_name(BOOK_ID);

    book->move(this_player(), 1);
    return book;
}

/*
 * Function name: library_admin_access
 * Description:   Redefine this function to limit permission to admin commands
 * Returns:       1 - this_player() is permitted to perform admin commands
 *                0 - this_player() is not permitted to perform admin commands
 */
public int
library_admin_access()
{
    if (!this_player()->query_wiz_level())
    {
        write("You don't have permission to do that.\n");
        return 0;
    }

    return 1;
}

/*
 * Function name: library_borrow_access
 * Description:   Redefine this function to limit permission to borrow books
 * Returns:       1 - this_player() is permitted to borrow a book
 *                0 - this_player() is not permitted to borrow a book
 */
public int
library_borrow_access()
{
    return 1;
}

/*
 * Function name: library_borrow
 * Description:   Try to borrow a book from the library.
 * Arguments:     string str - any arguments to the command
 * Returns:       1/0 - Command success/failure
 */
public int
library_borrow(string str)
{
    string book_file;
    object book;

    /* Check for borrow permission */
    if (!library_borrow_access())
    {
        return 1;
    }

    if (!strlen(str))
    { 
        return library_borrow_syntax_failure_hook();
    }

    str = lower_case(str);

    if (!strlen((book_file = book_map[str])))
    {
        if (sscanf(str, "approval %s", str))
        {
            if (!library_approve_access() || !library_admin_access())
            {
                return 1;
            }
 
            if (!strlen(book_file = appr_map[str]))
            {
                return library_borrow_unavailable_approval_book_hook(str);
            }
        }
        else
        {                    
            return library_borrow_unavailable_book_hook(str);         
        }
    }

    book = library_give_book(book_file, str);

    library_borrow_hook(book, str);
    return 1;
}

/*
 * Function name: library_read_book
 * Description:   Display the text of a book to the reader
 * Arguments:     string text_filename - The filename of the book file to use
 *                string book_name     - The title of the book
 *                int mread            - true if more should be used
 * Returns:       A configured book
 */
public void
library_read_book(string text_filename, string book_name, int mread)
{
    setuid();
    seteuid(getuid());

    if (mread || (file_size(text_filename) > 4000))
    {
        this_player()->more(read_file(text_filename));
    }
    else
    {
        write(read_file(text_filename));
    }
}

/*
 * Function name: library_read
 * Description:   Try to read a book from the library.
 * Arguments:     string str - any arguments to the command
 * Returns:       1/0 - Command success/failure
 */
public int
library_read(string str)
{
    string book_file;

    /* Check for borrow permission */
    if (!library_borrow_access())
    {
        return 1;
    }

    if (!strlen(str))
    { 
        return library_borrow_syntax_failure_hook();
    }

    str = lower_case(str);

    if (!strlen((book_file = book_map[str])))
    {
        if (sscanf(str, "approval %s", str))
        {
            if (!library_approve_access() || !library_admin_access())
            {
                return 1;
            }

            if (!strlen(book_file = appr_map[str]))
            {
                return library_borrow_unavailable_approval_book_hook(str);
            }
        }
        else
        {                    
            return library_borrow_unavailable_book_hook(str);         
        }
    }

    library_read_book(book_file, str, query_verb() == "mread");

    return 1;
}

/*
 * Function name: library_return_access
 * Description:   Redefine this to limit permission to return books
 * Returns:       1 - this_player() is permitted to return a book
 *                0 - this_player() is not permitted to return a book
 */
public int
library_return_access()
{
    return 1;
}

/*
 * Function name: library_return
 * Description:   Return a borrowed book
 * Arguments:     string str - any arguments to the command
 * Returns:       1/0 - Command success/failure
 */
public int
library_return(string str)
{
    if (!library_return_access())
    {
        return 1;
    }

    if (!sizeof(CMDPARSE_ONE_ITEM(str, "return_it", "return_access")))
    {
        return library_return_syntax_failure_hook(str);
    }

    return 1;
}

/*
 * Function name: return_access
 * Description:   Check to see if the given object is a book that can be
 *                returned to this library.
 * Arguments:     object ob - The object to check
 * Returns:       1 - the object can be returned to this library
 *                0 - the object cannot be returned to this library
 */
public int
return_access(object ob)
{
    return ((environment(ob) == this_player()) && ob->id(BOOK_ID));
}

/*
 * Function name: return_it
 * Description:   Return a library book to the library
 * Arguments:     object ob - the book to return
 * Returns:       1 - the book was returned
 *                0 - the book was not returned
 */
public int
return_it(object ob)
{
    library_return_hook(ob);
    ob->remove_object();
    return 1;
}

/*
 * Function name: library_list_access
 * Description:   Redefine this to limit permission to list books
 * Returns:       1 - this_player() is permitted to list books
 *                0 - this_player() is not permitted to list books
 */
public int
library_list_access()
{
    return 1;
}

/* 
 * Function name: library_list
 * Description:   The "list" command
 * Arguments:     string str - arguments given to the "list" command
 * Returns:       1/0 - syntax success/failure
 */
public int
library_list(string str)
{
    string shelf;

    if (!library_list_access())
    {
        return 1;
    }

    /* short listing of all books */
    if (str == "titles")
    {
        if (!m_sizeof(book_map))
        {
            return library_no_books_hook();
        }

        library_list_short_hook();
        return 1;
    }

    /* listing of shelf names */
    if (member_array(str, plural_shelf_types) >= 0)
    {
        if (!m_sizeof(book_shelves))
        {
            return library_no_shelves_hook();
        }

        library_list_shelves_hook();
        return 1;
    }

    /* long listing of a particular shelf */
    if (book_shelves[str])
    {
        library_list_shelf_long_hook(str);
        return 1;
    }

    if (strlen(str) && sscanf(str, "%s titles", shelf) && book_shelves[shelf])
    {
        library_list_shelf_short_hook(shelf);
        return 1;
    }

    /* long listing of approval books */
    if (strlen(appr_dir) && (str == "approval"))
    {
        if (!library_approve_access() || !library_admin_access())
        {
            return 1;
        }

        if (!m_sizeof(appr_map))
        {
            return library_no_approval_books_hook();
        }

        library_list_approval_long_hook();
        return 1;
    }

    /* short listing of approval books */
    if (strlen(appr_dir) && (str == "approval titles"))
    {
        if (!library_approve_access() || !library_admin_access())
        {
            return 1;
        }

        if (!m_sizeof(appr_map))
        {
            return library_no_approval_books_hook();
        }

        library_list_approval_short_hook();
        return 1;
    }

    /* at this point, if there's an argument, it's a syntax error */
    if (strlen(str))
    {
        return library_list_syntax_failure_hook(str);
    }

    /* long listing of all books */

    if (!m_sizeof(book_map))
    {
        return library_no_books_hook();
    }

    library_list_long_hook();
    return 1;
}

/*
 * Function name: library_validate_summary
 * Description:   Determine if a string is a valid book summary
 * Arguments:     string summary - the summary string
 * Returns:       1/0 - valid/invalid
 */
public int
library_validate_summary(string summary)
{
    if (!wildmatch("*[a-zA-Z0-9]*", summary))
    {
        write("\nThe summary must contain an alphanumeric character.\n" +
            "Please enter a valid summary. (~q to quit)\n> ");
        return 0;
    }

    return 1;
}

/*
 * Function name: library_write_get_summary
 * Description:   Catch input from the user to be used as the new
 *                book's summary
 * Arguments:     object who   - the user
 *                string title - the new book's title
 *                string input - the user's input
 * Returns:       1/0 - summary set/not set
 */
public int
library_write_get_summary(object who, string title, string input)
{
    if (input == "~q")
    {
        return 0;
    }

    set_this_player(who);

    if (!library_validate_summary(input))
    {
        input_to(&library_write_get_summary(this_player(), title));
        return 0;
    }

    setuid();
    seteuid(getuid());

    write("\n");
    clone_object(EDITOR_OBJECT)->edit(&done_writing(title, input));
    return 1;
}

/*
 * Function name: get_book_name
 * Description:   Get a filename for a new book in a specified directory
 * Arguments:     string dir - a directory
 * Returns:       the filename
 */
public string
get_book_name(string dir)
{
    string *files;
    int i, tmp, last = -1;

    setuid();
    seteuid(getuid());

    files = get_dir(dir);
    for (i = sizeof(files) - 1; i >= 0; i--)
    {
        if (!sscanf(files[i], text_file_name + "%d", tmp))
        {
           continue;
        }

        if (tmp > last)
        {
            last = tmp;
        }
    }

    return dir + text_file_name + (last + 1);
}

/*
 * Function name: library_hook_get_approval_names
 * Description  : This routine can be redefined in the library code to provide
 *                the names of the players who are authorized to approve books
 *                before they are added to the library. If this is the case,
 *                those players will receive a mail informing them of the fact
 *                that a new book has been submitted.
 * Returns      : string * - the list of names (in lower case).
 */
public mixed
library_hook_get_approval_names()
{
    return ({ });
}

/* 
 * Function name: add_book
 * Description:   Add a new book to the library
 * Arguments:     string title   - the book's title
 *                string summary - the book's summary
 *                string author  - the book's author
 *                string text    - the book's text
 *                int approval   - true if the book needs approval
 * Returns:       1/0 - book added/not added
 */
public int
add_book(string title, string summary, string author,
         string text, int approval)
{
    string dir = (approval ? appr_dir : book_dir);
    string *names;

    if (!approval && strlen(default_shelf))
    {
        dir += default_shelf + "/";
    }

    setuid();
    seteuid(getuid());

    /* Guild approval agents may be informed of the new book. */
    if (approval && sizeof(names = library_hook_get_approval_names()))
    {
        CREATE_MAIL("New book " + author, "Librarian",
            implode(names, ","), "",
            "Title: " + title + "\nSummary: " + summary + "\n");
    }

    if (!write_file(get_book_name(dir),
        sprintf("%|75s\n%|75s\n%|75s\n", title, summary, author) + text))
    {
         return 0;
    }

    update_books();
    return 1;
}

/*
 * Function name: done_writing
 * Description:   Catch input from the user to be used as the new
 *                book's text and add the new book to the library
 * Arguments:     string title   - the book's title
 *                string summary - the book's summary
 *                string input   - the book's text
 * Returns:       1/0 - text set/not set
 */
public int
done_writing(string title, string summary, string input)
{
    if (!strlen(input))
    {
        library_write_abort_hook();
        return 0;
    }

    if (!add_book(title, summary, "by " + this_player()->query_name(), 
        input, !!strlen(appr_dir)))
    {
        library_write_failed_hook();
    }
    else
    {
        write("Ok.\n");
    }

    return 1;
}

/*
 * Function name: library_validate_title
 * Description:   Determine if a string is a valid book title
 * Arguments:     string title - the title string
 * Returns:       1/0 - valid/invalid
 */
public int
library_validate_title(string title)
{
    if (strlen(title) > MAX_TITLE_SIZE)
    {
        write("\nThe title must be fewer than " + MAX_TITLE_SIZE +
            " characters long.\nPlease enter a valid title. (~q to quit)\n> ");
        return 0;
    }

    if (!wildmatch("*[a-zA-Z0-9]*", title))
    {
        write("\nThe title must contain an alphanumeric character.\n" +
              "Please enter a valid title. (~q to quit)\n> ");
        return 0;
    }

    update_books();
  
    if (book_map[title])
    {
        write("\nThere is already a " + BOOK_TYPE + " with that title.\n" +
              "Please enter a valid title. (~q to quit)\n> ");
        return 0;
    }

    return 1;
}

/*
 * Function name: library_write_get_title
 * Description:   Catch user input to be used as the new book's title
 * Arguments:     object who   - the user
 *                string input - the title
 * Returns:       1/0 - title set/not set
 */
public int
library_write_get_title(object who, string input)
{
    if (input == "~q")
    {
        return 0;
    }

    set_this_player(who);

    if (!library_validate_title(input))
    {
        input_to(&library_write_get_title(this_player()));
        return 0;
    }

    library_write_prompt_summary_input_hook();
    input_to(&library_write_get_summary(this_player(), input));
    return 1;
}

/*
 * Function name: library_write_access
 * Description:   Redefine this function to limit permission to write books
 * Returns:       1 - this_player() is permitted to write a book
 *                0 - this_player() is not permitted to write a book
 */
public int
library_write_access()
{
    write("You don't have permission to do that.\n");
    return 0;
}

/*
 * Function name: library_write
 * Description:   The "write" command
 * Arguments:     string str - arguments to the "write" command
 * Returns:       1/0 - syntax success/failure
 */
public int
library_write(string str)
{
    if (!library_write_access())
    {
        return 1;
    }

    library_write_prompt_title_input_hook();

    input_to(&library_write_get_title(this_player()));
    return 1;
}

/*
 * Function name: library_approve_access
 * Description:   Redefine this function to limit permission to approve books
 * Returns:       1 - this_player() is permitted to approve a book
 *                0 - this_player() is not permitted to approve a book
 */
public int
library_approve_access()
{
    return 1;
}

/*
 * Function name: library_approve
 * Description:   The "approve" command
 * Arguments:     string str - arguments to the "approve" command
 * Returns:       1/0 - syntax success/failure
 */
public int
library_approve(string str)
{
    string book_file;

    if (!library_approve_access() || !library_admin_access())
    {
        return 1;
    }

    if (!strlen(str))
    {
        return library_approve_syntax_failure_hook(str);
    }

    str = lower_case(str);

    if (!strlen(book_file = appr_map[str]))
    {
        return library_approve_unavailable_book_hook(str);
    }

    add_book(str, query_book_summary(book_file),
        query_book_author(book_file), query_book_text(book_file), 0);

    setuid();
    seteuid(getuid());

    rm(book_file);
    
    update_books();

    library_approve_hook(str);
    return 1;
}

/*
 * Function name: library_classify_access
 * Description:   Redefine this function to limit permission to classify books
 * Returns:       1 - this_player() is permitted to classify a book
 *                0 - this_player() is not permitted to classify a book
 */
public int
library_classify_access()
{
    return 1;
}

/*
 * Function name: library_classify_get_shelf
 * Description:   Catch input from the user and classify a book under
 *                the indicated shelf
 * Arguments:     object who   - the user
 *                string title - the book's title
 *                string input - the shelf name
 * Returns:       1/0 - book classified/not classified
 */
public int
library_classify_get_shelf(object who, string title, string input)
{
    string book_file; 

    set_this_player(who);

    if (!book_shelves[input])
    {
        library_classify_unavailable_shelf_hook(input);
        return 0;
    }

    if (!strlen(book_file = book_map[title])) 
    {
        library_classify_unavailable_book_hook(title);
        return 0;
    }

    library_move_book(book_file, book_dir + input + "/");

    update_books();

    library_classify_hook(title, input);
    return 1;
}
    
/*
 * Function name: library_classify
 * Description:   The "classify" command
 * Arguments:     string str - arguments to the "classify" command
 * Returns:       1/0 - syntax success/failure
 */
public int
library_classify(string str)
{
    if (!library_classify_access() || !library_admin_access())
    {
        return 1;
    }

    if (!strlen(str))
    {
        return library_classify_syntax_failure_hook(str);
    }

    if (!strlen(book_map[str])) 
    {
        return library_classify_unavailable_book_hook(str);
    }

    library_classify_prompt_shelf_input_hook(str);
    input_to(&library_classify_get_shelf(this_player(), str));
    return 1;
}

/*
 * Function name: library_deny_access
 * Description:   Redefine this function to limit permission to deny books
 * Returns:       1 - this_player() is permitted to deny a book
 *                0 - this_player() is not permitted to deny a book
 */
public int
library_deny_access()
{
    return 1;
}

/*
 * Function name: library_deny
 * Description:   The "deny" command
 * Arguments:     string str - arguments to the "deny" command
 * Returns:       1/0 - syntax success/failure
 */
public int
library_deny(string str)
{
    string book_file;

    if (!library_deny_access() || !library_admin_access())
    {
        return 1;
    }

    if (!strlen(str))
    {
        return library_deny_syntax_failure_hook(str);
    }

    str = lower_case(str);

    if (!strlen(book_file = appr_map[str]))
    {
        return library_deny_unavailable_book_hook(str);
    }

    library_remove_book(book_file);  

    update_books();

    library_deny_hook(str);
    return 1;
}
 
/*
 * Function name: library_discard_access
 * Description:   Redefine this function to limit permission to discard books
 * Returns:       1 - this_player() is permitted to discard a book
 *                0 - this_player() is not permitted to discard a book
 */
public int
library_discard_access()
{
    return 1;
}

/*
 * Function name: library_discard
 * Description:   The "discard" command
 * Arguments:     string str - arguments to the "discard" command
 * Returns:       1/0 - syntax success/failure
 */
public int
library_discard(string str)
{
    string book_file;

    if (!library_discard_access() || !library_admin_access())
    {
        return 1;
    }

    if (!strlen(str))
    {
        return library_discard_syntax_failure_hook(str);
    }

    str = lower_case(str);

    if (!strlen(book_file = book_map[str]))
    {
        return library_discard_unavailable_book_hook(str);
    }

    library_remove_book(book_file);

    update_books();

    library_discard_hook(str);
    return 1;
}

/*
 * Function name: init_library
 * Description:   Add library commands
 */
public void
init_library()
{
    if (borrow_required)
    {
        add_action(library_borrow, "borrow");
        add_action(library_return, "return");
    }
    else
    {
        add_action(library_read, "read");
        add_action(library_read, "mread");
    }

    add_action(library_list,   "list");
    
    add_action(library_write,  "write");

    add_action(library_approve,  "approve");
    add_action(library_classify, "classify");
    add_action(library_deny,     "deny");
    add_action(library_discard,  "discard");
}

/*
 * Function name: library_exit_block
 * Description:   Returns true if this_player() is in possession of
 *                a book from this library.  It can be used as a block
 *                function for library exits.
 * Returns:       1/0 - library book possessed/not possessed
 */
public int
library_exit_block()
{
    return !!sizeof(filter(deep_inventory(this_player()), &->id(BOOK_ID)));
}

/*
 * Function name: library_help
 * Returns:       A string containing library help information
 */
public string
library_help()
{
    function help_cmd = &sprintf("%-33s - %-=40s\n", , );

    string str =
        "Library commands available:\n" +
        help_cmd("List " + 
            (m_sizeof(book_shelves) ? "[<" + SHELF_TYPE + ">] " : "") +
            "[titles]", "List books in the library.");

    if (m_sizeof(book_shelves))
    {
        str += help_cmd("List " + PLURAL_SHELF_TYPE, 
                        "List " + PLURAL_SHELF_TYPE + " in the library.");
    }

    if (borrow_required)
    {
        str += 
            help_cmd("Borrow <title>",
                     "Borrow a book for reading.") +
            help_cmd("Return <book>",
                     "Return a book to the library.");
    }
    else
    {
        str += help_cmd("Read <title>", "Read a book in the library.") +
               help_cmd("Mread <title>", "Mread a book in the library.");
    }

    str += help_cmd("Write", "Write a book and submit it to the library.");

    str += "\nAdministrator commands available:\n";

    if (strlen(appr_dir))
    {
        str += 
            help_cmd("Approve <title>",
                     "Approve a book for inclusion in the library.");

        if (borrow_required)
        {
            str += help_cmd("Borrow approval <title>",
                "Administrators may borrow books which have not been " +
                 "approved by using the \"approval\" option.");
        }
        else
        {
            str += help_cmd("(m)read approval <title>",
                "Administrators may read books which have not been " +
                "approved by using the \"approval\" option.");
        }

        str +=
            help_cmd("Deny <title>",
                 "Deny a book from inclusion in the library.") +
            help_cmd("List approval [titles]",
                 "Administrators may list books that need approval with " +
                 "the \"approval\" option.");
    }

    str += help_cmd("Discard <title>", "Remove a book from the library.");

    if (m_sizeof(book_shelves))
    {
        str += help_cmd("Classify <title>",
                        "Assign a book to a " + SHELF_TYPE + ".");
    }

    return str + "\n";
}

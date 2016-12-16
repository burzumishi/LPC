/*
 *  /std/guild_library_book.c
 * 
 *  Book that's being used in the /lib/guild_library.c, can be
 *  inherited to create a custom book. Use set_library_book_object
 *  in the library to use it.
 *
 *  Created by Linor, 24-11-2003
 *  Ported from Angalon to Genesis by Eowul, 30-8-2004
 *  Updated for inclusion in the mudlib by Eowul on May 10th, 2009
 */

#include <macros.h>
#include <composite.h>
#include <cmdparse.h>
#include <stdproperties.h>

#define EDITOR  "/obj/edit"

inherit "/std/scroll";

object  library;
string  filename;
string  title;
string  text;
string  book_id;
string  book_type;

/*
 * Function name: create_library_book
 * Description  : Construct the library book
 */
void create_library_book()
{
}

/*
 * Function name: create_scroll
 * Description  : Constructor
 */
nomask void create_scroll()
{
    set_name("book");
    add_name("_guild_library_book");
    
    text = "";

    add_prop(OBJ_M_NO_DROP, "Return it to the library instead of dropping " +
        "it.\n");

    create_library_book();
}

/*
 * Function name: set_book_id
 * Description  : Set the ID of the book as known in the library
 * Arguments    : id - the book id
 */
void set_book_id(string id)
{
    book_id = id;
}

/*
 * Function name: set_library
 * Description  : Set the object pointer to the library this book belongs to
 * Arguments    : ob - the library object
 */
void set_library(object ob)
{
    library = ob;
    
    // See if we use books, or scrolls or parchments or whatever
    book_type = library->query_library_book_type();
    if(!strlen(book_type)) book_type = "book";
}

/*
 * Function name: set_filename
 * Description  : Set the filename of the book context so that it can be
 *                read and edited.
 * Arguments    : name - the name of the file
 */
void set_filename(string name)
{
    filename = name;
    set_file(filename);
}

/*
 * Function name: set_title
 * Description  : Set the title of this book
 * Arguments    : str - the title to set
 */
void set_title(string str)
{
    title = str;
}

/*
 * Function name: done_editing
 * Description  : Function called when the player is done editing the
 *              : text of the book.
 * Arguments    : str - the text of the book
 */
void done_editing(string str)
{
    // Aborted
    if(!strlen(str))
    {
        write("Aborted.\n");
        return;
    }

    // Update the text
    text = str;
    write("Ok.\n");
}

/*
 * Function name: title_input
 * Description  : Called from input_to with the new title of the book
 * Arguments    : str - the new title
 */
void title_input(string str)
{
    // Check if we want to abort
    if(!strlen(str) || str == "~q")
    {
        write("Aborted.\n");
        return;
    }

    // Update the title
    title = str;

    write("Ok.\n");
}

/*
 * Function name: edit_cmd
 * Description  : Modify the text of this book
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int edit_cmd(string args)
{
    object  *books;

    // Syntax check
    if(!args || args == "")
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    // Perform a parse command
    if(!parse_command(args, this_player(), "[the] %i", books))
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    // Filter
    books = NORMAL_ACCESS(books, 0, 0);

    // Do some checks
    if(sizeof(books) > 1)
    {
        notify_fail("You can only " + query_verb() + " one thing at a " +
            "time.\n");
        return 0;
    }

    // Check if we found something
    if(!sizeof(books))
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    // See if we mean this book
    if(books[0] != this_object())
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    // Do an access check
    if(!objectp(library))
    {
        write("Please leave a bug report, no library is set in this " +
            book_type + ".\n");
        return 1;
    }

    if(strlen(filename) && !library->query_librarian(this_player()))
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    // See if this book is locked from edditing
    if(strlen(book_id) && library->library_is_book_locked(book_id))
    {
        write("You are not allowed to alter the contents of this " +
            book_type + ".\n");
        return 1;
    }

    if(query_verb() == "title")
    {
        if(strlen(title))
            write("The current title is: " + title + "\n");

        write("What is the new title of the " + book_type + "?\n");
        write("~q to abort: ");
        input_to(title_input);

        return 1;
    }

    // Load the text if there is none yet
    if(!strlen(text) && strlen(filename))
    {
        seteuid(getuid());
        text = read_file(filename, 1);
    }

    // Start editing
    clone_object(EDITOR)->edit(done_editing, text);

    return 1;
}

/*
 * Function name: return_library_book
 * Description  : Function called when we want to return this book
 * Returns      : 0 on failure, 1 on success
 */
int return_library_book()
{
    // Do an access check
    if(!objectp(library))
    {
        write("Please leave a bug report, no library is set in this " + 
            book_type + ".\n");
        return 0;
    }

    // See if we have a book ID assigned
    if(strlen(book_id))
        library->library_remove_borrowed(book_id);

    // See if we have to add this book
    if(!strlen(book_id))
    {
        write("You must submit your " + book_type + " to add it to the " +
            "library.\n");
        return 0;
    }

    // Update the book if the text was altered
    if(strlen(text))
        library->update_book(book_id, title, text);

    return 1;
}

/*
 * Function name: discard_library_book
 * Description  : Function called when we want to submit this book
 * Returns      : 0 on failure, 1 on success
 */
int discard_library_book()
{
    // Do an access check
    if(!objectp(library))
    {
        write("Please leave a bug report, no library is set in this " + 
            book_type + ".\n");
        return 0;
    }

    // See if we have to add this book
    if(strlen(book_id))
    {
        write("You must return your " + book_type + ".\n");
        return 0;
    }
    
    return 1;
}

/*
 * Function name: submit_library_book
 * Description  : Function called when we want to submit this book
 * Returns      : 0 on failure, 1 on success
 */
int submit_library_book()
{
    // Do an access check
    if(!objectp(library))
    {
        write("Please leave a bug report, no library is set in this " + 
            book_type + ".\n");
        return 0;
    }

    // See if we have to add this book
    if(strlen(book_id))
    {
        write("You must return your " + book_type + ".\n");
        return 0;
    }

    if(!strlen(title))
    {
        write("The " + book_type + " has no title.\n");
        return 0;
    }

    if(!strlen(text))
    {
        write("The " + book_type + " titled '" + title + "' has no contents.\n");
        return 0;
    }   

    library->add_new_book(title, text, this_player()->query_real_name());
    return 1;
}

/*
 * Function name: return_cmd
 * Description  : Return the book or submit it
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int return_cmd(string args)
{
    object  *books;

    // Syntax check
    if(!args || args == "")
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    // Perform a parse command
    if(!parse_command(args, this_player(), "[the] %i", books))
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    // Filter
    books = NORMAL_ACCESS(books, 0, 0);

    // Check if we found something
    if(!sizeof(books))
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    switch(query_verb())     
    {
        case "discard":
        {
            // Call a return function in every book
            books = filter(books, &->discard_library_book());
            break;
        }
        case "submit":
        {
            // Call a submit function in every book
            books = filter(books, &->submit_library_book());
            break;
        }
        default:
        {
            // Call a return function in every book
            books = filter(books, &->return_library_book());
            break;
        }
    }

    if(sizeof(books) == 0)
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    // Write messages
    write("You " + query_verb() + " " + COMPOSITE_DEAD(books) + ".\n");
    say(QCTNAME(this_player()) + " " + query_verb() + "s " + 
        COMPOSITE_DEAD(books) + ".\n");

    // Destroy the books
    books->remove_object();

    return 1;
}

/*
 * Function name: init
 * Description  : Add commands to the player
 */
void init()
{
    ::init();

    // See if we are linked to a library
    if(!objectp(library))
        return;

    add_action(return_cmd,  "return");

    // See if the player is a librarian, or that this book is a new book
    if(!library->query_librarian(this_player()) && strlen(filename))
        return;

    // Add commands
    add_action(edit_cmd,    "edit");
    add_action(edit_cmd,    "title");
    add_action(return_cmd,  "submit");
    add_action(return_cmd,  "discard");
}

/*
 * Function:    long
 * Description: Description of the mount.
 * Arguments:   str: thing looked at; for_obj: who is looking.
 * Returns:     That description, as a string.
 */
string long(string str, object for_obj)
{
    string  result;

    // For add_items, use the original function
    if(strlen(str))
        return ::long(str, for_obj);

    result = ::long(str, for_obj);

    // See if the player is a librarian, or that this book is a new book
    if(!library->query_librarian(this_player()) && strlen(filename))
        return result + "You can 'return' the " + book_type + 
        " if you are done reading.\n";
    
    // Add extra commands
    result += "The following commands are available: 'title " + book_type + 
        "', 'edit " + book_type + "', 'submit " + book_type + 
        "' and 'discard " + book_type + "'.\n";

    return result;
}

/*
 * Function name: read_it
 * Description:   Perform the actual read
 */
void
read_it(string answer)
{
    // Read from file if no text is set, otherwise, display the text
    if(strlen(filename))   
    {
        ::read_it(answer);
        return;
    }

    if(!strlen(text))
    {
        write("The scroll is empty.\n");
    }
    else
    {
        // No more for now
        this_player()->more(text, 0);
    }
}

/*
 *  /lib/guild_library.c
 *  A set of functions to create a mortal maintained library. By default
 *  this library has two access levels, normal and librarian, where wizards
 *  are the default librarians. By adding more access levels and overriding
 *  the query_player_access and query_librarian functions, mortal guild
 *  leaders could be allowed to control the library. 
 *
 *  Example usage:
 *
 *  inherit "/std/room";
 *  inherit "/lib/guild_library";
 *
 *  void create_room()
 *  {
 *      set_short("Library");
 *      set_long("This is a guild library.\n");
 *
 *      set_library_save_path("/d/Domain/somedir/");
 *      set_library_allow_new_books(1);
 *
 *      create_library();
 *
 *      add_item(({"plaque", "instructions", }), library_help);
 *      add_cmd_item(({"plaque", "instructions", }), "read", library_help);
 *  }
 *
 *  void init()
 *  {
 *      ::init();
 *      init_library();
 *  }
 *
 *  void leave_inv(object ob, object to)
 *  {
 *      library_leave_inv(ob, to);
 *      ::leave_inv(ob, to);
 *  }
 *
 *  For a more advanced example, check /doc/examples/guild_library
 *
 *  Created by Linor, 23-11-2003
 *  Ported from Angalon to Genesis by Eowul, 30-8-2004
 */

#include <composite.h>
#include <macros.h>

#define EDITOR  "/obj/edit"

/*
 *  Variables:
 *
 *  library_data - mapping
 *      "shelves"   - mapping
 *          "shelf_name" : array( description, access_level )
 *      "books"     - mapping
 *          "book_name/number" : array ( filename, title, shelf, author, locked )
 */
mapping library_data;
string  library_save_file;
mapping library_access_levels;
string  library_new_book_shelf;
string  library_librarian_level;
string  library_save_path;
int     library_new_book_id_start;
string  *library_borrowed_books;
int     library_allow_new_books;
string  library_book_object;
string  library_log_file;
string  library_book_type;

// Prototypes
void    save_library_data();    // Write data to disk
void    load_library_data();    // Get data from disk
string  query_access_desc(string level);

// Hooks

/*
 * Function name: library_hook_rename_book
 * Description  : Called after a book is renamed
 * Arguments    : string book_name - the id of the book
 *              : new_name - the new title of the book
 * Returns      : a string
 */
string library_hook_rename_book(string book_name, string new_name)
{
    return "The " + library_book_type + " '" + book_name + "' is now " +
        "titled '" + new_name + "'.\n";
}

/*
 * Function name: library_hook_change_access_level
 * Description  : Called when the access level of a shelf is changed
 * Arguments    : shelf_name - the name of the shelf
 *              : level - the new access level
 */
string library_hook_change_access_level(string shelf_name, string level)
{
    // Messages
    return "Changed the access level of '" + shelf_name + "' to '" +
        level + "'.\n";
}

/*
 * Function name: library_hook_renumber_book
 * Description  : Called after a book has been renumbered
 * Arguments    : old_id - the original id of the book
 *              : new_id - the new id of the book
 */
void library_hook_renumber_book(string old_id, string new_id)
{
    // Write messages
    write("You renumber " + library_book_type + " '" + old_id + "' to '" + 
        new_id + "'.\n");
    say(QCTNAME(this_player()) + " moves some " + library_book_type + 
        "s around on the shelves.\n");
}

/*
 * Function name: library_hook_rename_shelf
 * Description  : Called when a shelf is renamed
 * Arguments    : shelf_name - the original name
 *              : new_name - the new name of the shelf
 */
string library_hook_rename_shelf(string shelf_name, string new_name)
{
    return "The shelf '" + capitalize(shelf_name) + "' has been renamed to " +
        "'" + capitalize(new_name) + "'.\n";
}

/*
 * Function name: library_hook_change_shelf_title
 * Description  : Called when a shelf is renamed
 * Arguments    : shelf_name - the name of the shelf
 *              : description - the description of the shelf
 */
string library_hook_change_shelf_title(string shelf_name, string description)
{
    return "The shelf '" + capitalize(shelf_name) + "' is now titled '" +
        description + "'.\n";
}

/*
 * Function name: library_hook_read_direct
 * Description  : Called when a player directly reads a book
 * Arguments    : content - the text of the book
 */
void library_hook_read_direct(string bookid, string content)
{
    this_player()->more(content);
}

/*
 * Function name: library_hook_receive_book
 * Description  : Write the messages for a player receiving a new book
 * Arguments    : player - the player receiving the book
 *              : book - the book the player receives
 */
void library_hook_receive_book(object player, object book)
{
    // Write messages
    player->catch_msg("You find the " + library_book_type + 
        " that you were looking " +
        "for and take it from the shelf.\n");
    tell_room(environment(player), QCTNAME(player) + " seems to " +
        "have found the " + library_book_type + " " + 
        player->query_pronoun() + " was " +
        "looking for and lifts it off the shelf.\n", player);
}

/*
 * Function name: library_hook_stop_leave
 * Description  : Called when a player is stopped while carrying books,
 *              : redefine it to give your own message.
 * Arguments    : player - the player that's trying to leave
 *              : books - an array with the books that have not been returned
 */
void library_hook_stop_leave(object player, object *books)
{
    player->catch_msg("The librarian wont let you leave with " +
        COMPOSITE_DEAD(books) + ".\n");
}

/*
 * Function name: library_hook_list_books
 * Description  : Called when a player lists the contents of a shelf
 * Arguments    : name - the name of the shelf being listed
 */
void library_hook_list_books(string shelf_name, mixed *shelf_data, mixed *books)
{
    int i;

    // Check for empty shelves
    if(sizeof(books) == 0)
    {
        write("The shelf '" + shelf_name + "' is empty.\n");
        return;
    }

    // Display the list
    write(shelf_data[0] + ":\n\n");
    write(sprintf("%-10s %s\n", "Number", "Title"));
    write(sprintf("%-10s %s\n", "----------", 
        "-----------------------------------------------------------------"));
    for(i=0;i<sizeof(books);i++)
    {
        write(sprintf("%-10s %s\n", 
            books[i],
            library_data["books"][books[i]][1]));
    }
}

/*
 * Function name: library_hook_list_shelves
 * Description  : Called when a player lists the available shelves
 * Arguments    : shelves - the array of shelves the player has access to
 */
void library_hook_list_shelves(string *shelves)
{           
    int i;

    write("You find the following shelves:\n\n");
    write(sprintf("%-25s %-35s %s\n",
        "Shelf", "Description", "Access level"));
    write(sprintf("%-25s %-35s %s\n",
        "-------------------------", 
        "-----------------------------------", 
        "---------------"));

    for(i=0;i<sizeof(shelves);i++)
    {
        write(sprintf("%-25s %-35s %s\n",
            shelves[i],
            library_data["shelves"][shelves[i]][0],
            query_access_desc(library_data["shelves"][shelves[i]][1])));
    }
}

/*
 * Function name: library_hook_remove_shelf
 * Description  : Called when a shelf is removed, should give a nice message
 * Returns      : a message
 */
string library_hook_remove_shelf(string shelf_name, int book_count)
{
    // Status
    if(book_count == 0)
    {
        return "The shelf '" + capitalize(shelf_name) + "' has been " +
            "removed, no " + library_book_type + "s needed moving.\n";
    }

    return "The shelf '" + capitalize(shelf_name) + "' has been " +
            "removed, " + book_count + " " + library_book_type + 
            (book_count > 1 ? "s have" : 
            " has") + " been moved to the '" + library_new_book_shelf + 
            "' shelf.\n";
}

/*
 * Function name: library_hook_create_shelf
 * Description  : Construct a message
 * Returns      : a string
 */
string library_hook_create_shelf(string shelf_name, string description)
{
    // Status
    return "The shelf '" + capitalize(shelf_name) + "' has been added with " +
        "description '" + description + "'.\n";
}

/*
 * Function name: library_remove_book
 * Description  : Called when a book is removed
 * Arguments    : book_data - the information about the removed book
 */
void library_hook_remove_book(string name, mixed *book_data)
{
    // Write a message
    write("You remove " + library_book_type + " '" + name + "'.\n");
    say(QCTNAME(this_player()) + " moves some " + library_book_type + 
        "s around on the shelves.\n");
}

/*
 * Function name: libraru_hook_move_book
 * Description  : Called after a book is moved from one shelf to the other
 * Arguments    : old_id - the previous name
 *              : new_id - the new name
 */
void library_hook_move_book(string book_id, string old_shelf, string new_shelf)
{
    // Write messages
    write("You assigned " + library_book_type + " '" + book_id + 
        "' to the '" + new_shelf + "' shelf.\n");
    say(QCTNAME(this_player()) + " moves some " + library_book_type + 
        "s around on the shelves.\n");

}




/*
 * Function name: create_library
 * Description  : Initialise the library, let it read data and such
 */
void create_library()
{
    // Make sure we always have a save file set
    if(!stringp(library_save_file) || !strlen(library_save_file))
    {
        library_save_file = MASTER;
    }

    // Load the data
    load_library_data();

    // Set our new book shelf
    if(!stringp(library_new_book_shelf) || !strlen(library_new_book_shelf))
    {
        library_new_book_shelf = "unpublished";
    }

    // Set the librarian level
    if(!stringp(library_librarian_level) || !strlen(library_librarian_level))
    {
        library_librarian_level = "librarian";
    }

    // Make sure our new book shelf exists
    if(member_array(library_new_book_shelf, 
        m_indexes(library_data["shelves"])) == -1)
    {
        // Always add the unpublished book shelf, and set access level
        // to librarian
        library_data["shelves"][library_new_book_shelf] = 
            ({ "Unpublished books", library_librarian_level });
    }

    if(!mappingp(library_access_levels))
    {
        // Always at the librarians level
        library_access_levels = ([
            library_librarian_level : "Librarians only",
            "normal"                : "Everyone" ]);
    }

    // Make a list of borrowed books
    if(!pointerp(library_borrowed_books))
    {
        library_borrowed_books = ({});
    }

    // Set our starting ID to a default value
    if(library_new_book_id_start == 0)
    {
        library_new_book_id_start = 8000;
    }

    // See if we have a book object
    if(!strlen(library_book_object))
    {
        library_book_object = "/std/guild_library_book";
    }

    // See if we have a book type
    if(!strlen(library_book_type))
    {
        library_book_type = "book";
    }
}

/*
 * Function name: set_library_book_type
 * Description  : Set the name of the books that we use (ie. book, scroll)
 * Arguments    : type - the book type
 */
void set_library_book_type(string type)
{
    library_book_type = type;
}

/*
 * Function name: query_library_book_type
 * Description  : See what type of books we use
 * Returns      : the value set with set_library_book_type
 */
string query_library_book_type()
{
    return library_book_type;
}

/*
 * Function name: set_library_log_file
 * Description  : Set the filename we log too, this uses log_file, so should
 *              : use the naming convention for that
 * Arguments    : file - the filename
 */
void set_library_log_file(string file)
{
    library_log_file = file;
}

/*
 * Function name: library_add_log
 * Description  : Add a log entry if the logfile is set
 * Arguments    : msg - the message to log
 *              : extra - extra text to add to the log
 */
varargs void library_add_log(string msg, string extra)
{
    // Do nothing if no log is set
    if(!strlen(library_log_file))
        return;

    // Log the message
    log_file(library_log_file, sprintf("%s [%12s] %s\n",
        ctime(time()), this_player()->query_real_name(), msg));

    // See if we have an extra message to append
    if(strlen(extra))
    {
        log_file(library_log_file, break_string(extra, 75, 3) + "\n");
    }
}

/*
 * Function name: library_remove_borrowed
 * Description  : Remove a book from the list of borrowed books
 * Arguments    : book_id - the id of the book being returned
 */
void library_remove_borrowed(string book_id)
{
    library_borrowed_books -= ({ book_id });
}

/*
 * Function name: query_librarian
 * Description  : See if a person is a librarian
 * Arguments    : player - the player we want to check
 * Returns      : 0 on failure, 1 on success
 */
int query_librarian(object player)
{
    return player->query_wiz_level();
}

/*
 * Function name: set_library_save_file
 * Description  : Set the filename in which we store our library data
 * Arguments    : filename - the name of the file
 */
void set_library_save_file(string filename)
{
    library_save_file = filename;
}

/*
 * Function name: query_library_save_file
 * Description  : Get the filename in which we want to store our data
 * Returns      : a string with the filename
 */
string query_library_save_file()
{
    return library_save_file;
}

/*
 * Function name: query_player_access
 * Description  : Return a list of the levels that the player have access to
 * Arguments    : player - the player we want to check
 * Returns      : an array with access levels
 */
string *query_player_access(object player)
{
    // Add librarian access to librarians
    if(query_librarian(player))
        return ({ "librarian", "normal" });

    // Non wizzes get access to the normal shelf only
    return ({ "normal" });
}

/*
 * Function name: load_library_data
 * Description  : Read our datafile from disk
 */
void load_library_data()
{
    // Load the file if it's not empty
    if(file_size(library_save_file + ".o") > 0)
    {
        seteuid(getuid());
        library_data = restore_map(library_save_file);
    }

    // Make sure the mapping exists
    if(!mappingp(library_data))
    {
        library_data = ([ "shelves" : ([]),
                          "books" : ([ ]) ]);
        save_library_data();
    }
}

/*
 * Function name: query_access_desc
 * Description  : Get the description of a certain access level
 * Arguments    : level - the index of the level
 * Returns      : a string with the name or 'Unknown'
 */
string query_access_desc(string level)
{
    // We work on lowercase levels only
    level = lower_case(level);

    // See if we have the level
    if(!mappingp(library_access_levels))
        return "Unknown";

    if(member_array(level, m_indexes(library_access_levels)) == -1)
        return "Unknown";

    // Return the name
    return library_access_levels[level];
}

/*
 * Function name: save_library_data
 * Description  : Save our datafile to disk
 */
void save_library_data()
{
    // Save our data to disc
    setuid();
    seteuid(getuid());
    save_map(library_data, library_save_file);
}

/*
 * Function name: sort_by_number
 * Description  : Make sure the books are sorted by number
 * Arguments    : bookid1 - the first book id
 *              : bookid2 - the second book id
 * Returns      : 0 if equal, -1 when 1 < 2, 1 when 2 > 1
 */
int sort_by_number(string bookid1, string bookid2)
{
    return atoi(bookid1) - atoi(bookid2);
}

/*
 * Function name: library_list_cmd
 * Description  : The code for the list command
 * Arguments    : args - the command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_list_cmd(string args)
{
    int     i;
    string  *shelves, *access, *books;

    // Find the shelves we have access to
    access = query_player_access(this_player());

    // List all shelves that people have access too
    if(!args || args == "" || args == "shelves")
    {
        // Find all shelves that the player can read
        shelves = m_indexes(library_data["shelves"]);
        shelves = filter(shelves, 
            &operator(!=)(-1) @
            &member_array(, access) @
            lower_case @
            &operator([])(, 1) @ 
            &operator([])(library_data["shelves"],));

        // Sort them based on name
        shelves = sort_array(shelves);

        // See if they have read access
        if(sizeof(shelves) == 0)
        {
            write("You look at the shelves but don't find anything " +
                "interesting.\n");
            return 1;
        }

        // Loop through the shelves and display them
        library_hook_list_shelves(shelves);
        return 1;
    }

    // Work with lowercase shelf names
    args = lower_case(args);

    // See if we have this shelf
    if(member_array(args, m_indexes(library_data["shelves"])) == -1)
    {
        notify_fail("There is no shelf '" + args + "'.\n");
        return 0;
    }

    // See if we have access to the shelf
    if(member_array(library_data["shelves"][args][1], access) == -1)
    {
        // No access, pretend like the shelf isnt there
        notify_fail("There is no shelf '" + args + "'.\n");
        return 0;
    }

    // Find the books on the shelf
    books = filter(m_indexes(library_data["books"]),
        &operator(==)(args) @ 
        lower_case @
        &operator([])(, 2) @
        &operator([])(library_data["books"], ));

    // Sort the books
    books = sort_array(books, sort_by_number);

    // Call the hook to have the books listed
    library_hook_list_books(args, library_data["shelves"][args], books);

    return 1;
}

/*
 * Function name: add_shelf
 * Description  : Add a new shelf to the list of available shelves, default
 *                access level will be library_librarian_level
 * Arguments    : shelf_name - the short name of the shelf
 *              : description - the description of the shelf
 * Returns      : a string with the result message
 */
string add_shelf(string shelf_name, string description)
{
    // Make sure everything exists
    if(!mappingp(library_data))
        library_data = ([]);
    if(!mappingp(library_data["shelves"]))
        library_data["shelves"] = ([]);

    // We only use lowercase shelf names
    shelf_name = lower_case(shelf_name);

    // See if the shelf already exists
    if(member_array(shelf_name, m_indexes(library_data["shelves"])) != -1)
    {
        return "There is already a shelf named '" + capitalize(shelf_name) +
            "'.\n";
    }

    // Add the shelf
    library_data["shelves"][shelf_name] = ({ description, 
        library_librarian_level });

    // Log
    library_add_log("Shelf '" + capitalize(shelf_name) + "' added.",
        "Description '" + description + "'");
        
    // Store
    save_library_data();
    
    return library_hook_create_shelf(shelf_name, description);
}

/*
 * Function name: remove_shelf
 * Description  : Remove a shelf from the available shelf list, all books
 *                will be moved to the unpublished shelf.
 * Arguments    : shelf_name - the short name of the shelf
 * Returns      : a string with the result
 */
string remove_shelf(string shelf_name)
{
    int     i, book_count;
    string  *book_names;

    // Make sure everything exists
    if(!mappingp(library_data))
        library_data = ([]);
    if(!mappingp(library_data["shelves"]))
        library_data["shelves"] = ([]);
    if(!mappingp(library_data["books"]))
        library_data["books"] = ([]);

    // We only use lowercase shelf names
    shelf_name = lower_case(shelf_name);

    // See if the shelf already exists
    if(member_array(shelf_name, m_indexes(library_data["shelves"])) == -1)
    {
        return "There is no shelf named '" + capitalize(shelf_name) + "'.\n";
    }
    
    // See if this is the new book shelf
    if(shelf_name == library_new_book_shelf)
    {
        return "You cannot remove that shelf.\n";
    }

    // Find all books on the shelf
    book_names = m_indexes(library_data["books"]);
    for(i=0;i<sizeof(book_names);i++)
    {
        // See if the book is on the deleted shelf
        if(library_data["books"][book_names[i]][2] == shelf_name)
        {
            // Move the book to the unpublished shelf
            library_data["books"][book_names[i]][2] = library_new_book_shelf;
            book_count++;
        }
    }

    // Remove the shelf
    library_data["shelves"] = m_delete(library_data["shelves"], shelf_name);

    // Log
    library_add_log("Removed shelf '" + capitalize(shelf_name) + "'");

    // Save
    save_library_data();

    // Return the message
    return library_hook_remove_shelf(shelf_name, book_count);
}

/*
 * Function name: change_shelf_description
 * Description  : Give a shelf a new description
 * Arguments    : shelf_name - the name of the shelf
 *              : description - the new description of the shelf
 * Returns      : a string with the result
 */
string 
change_shelf_description(string shelf_name, string description)
{
    // Make sure everything exists
    if(!mappingp(library_data))
        library_data = ([]);
    if(!mappingp(library_data["shelves"]))
        library_data["shelves"] = ([]);

    // We only use lowercase shelf names
    shelf_name = lower_case(shelf_name);

    // See if the shelf already exists
    if(member_array(shelf_name, m_indexes(library_data["shelves"])) == -1)
    {
        return "There is no shelf named '" + capitalize(shelf_name) + "'.\n";
    }

    // Rename the shelf
    library_data["shelves"][shelf_name][0] = description;

    // Log
    library_add_log("Description of '" + capitalize(shelf_name) + 
        "' changed.", "New title '" + description + "'");

    // Save
    save_library_data();

    // Status
    return library_hook_change_shelf_title(shelf_name, description);
}

/*
 * Function name: rename_book
 * Description  : Give a book a new title
 * Arguments    : book_name - the name of the book
 *              : new_name - the new name of the shelf
 * Returns      : a string with the result
 */
string 
rename_book(string book_name, string new_name)
{
    // Make sure everything exists
    if(!mappingp(library_data))
        library_data = ([]);
    if(!mappingp(library_data["books"]))
        library_data["books"] = ([]);

    // Look for this book
    if(member_array(book_name, m_indexes(library_data["books"])) == -1)
    {
        return "There is no " + library_book_type + " in the library with " +
            "number '" + book_name + "'\n";
    }

    library_add_log("New title '" + new_name + "' for '" + book_name + "'");

    // Rename the book otherwise
    library_data["books"][book_name][1] = new_name;

    // Save
    save_library_data();

    // Status
    return library_hook_rename_book(book_name, new_name);
}

/*
 * Function name: rename_shelf
 * Description  : Give a shelf a new description
 * Arguments    : shelf_name - the name of the shelf
 *              : new_name - the new name of the shelf
 * Returns      : a string with the result
 */
string 
rename_shelf(string shelf_name, string new_name)
{
    int     i;
    string  *book_names;

    // Make sure everything exists
    if(!mappingp(library_data))
        library_data = ([]);
    if(!mappingp(library_data["shelves"]))
        library_data["shelves"] = ([]);

    // We only use lowercase shelf names
    shelf_name = lower_case(shelf_name);
    new_name = lower_case(new_name);

    // See if the shelf already exists
    if(member_array(shelf_name, m_indexes(library_data["shelves"])) == -1)
    {
        return "There is no shelf named '" + capitalize(shelf_name) + "'.\n";
    }

    // See if this is the same
    if(new_name == shelf_name)
    {
        return "The names are identical, no need to rename them.\n";
    }

    // See if the shelf already exists
    if(member_array(new_name, m_indexes(library_data["shelves"])) != -1)
    {
        return "There is already a shelf named '" + capitalize(new_name) +
            "'.\n";
    }

    // Rename the shelf
    library_data["shelves"][new_name] = library_data["shelves"][shelf_name];
    library_data["shelves"] = m_delete(library_data["shelves"], shelf_name);

    // Find all books on the shelf and move them to the new one
    book_names = m_indexes(library_data["books"]);
    for(i=0;i<sizeof(book_names);i++)
    {
        // See if the book is on the deleted shelf
        if(library_data["books"][book_names[i]][2] == shelf_name)
        {
            // Move the book to the unpublished shelf
            library_data["books"][book_names[i]][2] = new_name;
        }
    }

    // Log
    library_add_log("Renamed '" + shelf_name + "' to '" + new_name + "'");

    // Save
    save_library_data();

    // Status
    return library_hook_rename_shelf(shelf_name, new_name);
}

/*
 * Function name: change_shelf_access
 * Description  : Allow us to alter the access level of a shelf
 * Arguments    : shelf - the shelf we want to alter
 *              : level - the new access level
 * Returns      : a string with the result
 */
string change_shelf_access(string shelf_name, string level)
{
    int     i;
    string  *levels, ret;

    // Make sure everything exists
    if(!mappingp(library_data))
        library_data = ([]);
    if(!mappingp(library_data["shelves"]))
        library_data["shelves"] = ([]);

    // We only use lowercase shelf names
    shelf_name = lower_case(shelf_name);
    level = lower_case(level);

    // See if the shelf already exists
    if(member_array(shelf_name, m_indexes(library_data["shelves"])) == -1)
    {
        return "There is no shelf named '" + capitalize(shelf_name) + "'.\n";
    }

    // See if this is the same
    if(library_data["shelves"][shelf_name][1] == level)
    {
        return "The access levels are identical, no need to change them.\n";
    }

    // See if we have this level
    if(member_array(level, m_indexes(library_access_levels)) == -1)
    {
        // Return the available access levels
        ret = "No such access level '" + level + "', known levels are:\n\n";
        levels = m_indexes(library_access_levels);
        for(i=0;i<sizeof(levels);i++)
        {
            ret += sprintf("%20s - %s\n", levels[i], 
                library_access_levels[levels[i]]);
        }
        
        return ret;
    }

    // Alter the access level
    library_data["shelves"][shelf_name][1] = level;

    // Log
    library_add_log("Access of '" + shelf_name + "' for '" + level + "'");

    // Save
    save_library_data();

    return library_hook_change_access_level(shelf_name, level);
}

/*
 * Function name: query_new_book_id
 * Description  : Find a new available book id
 * Returns      : a string with a number
 */
string query_new_book_id()
{
    string  new_id, *book_names;

    // Get the list of existing books
    book_names = m_indexes(library_data["books"]);

    // Loop through the list
    new_id = sprintf("%04i", library_new_book_id_start);
    while(member_array(new_id, book_names) != -1)
    {
        // Increase the start id
        library_new_book_id_start++;
        new_id = sprintf("%04i", library_new_book_id_start);
    }

    // Return the id
    return new_id;
}

/*
 * Function name: set_library_save_path
 * Description  : Set the directory in which the books are stored. This
 *                is used both for adding new books, as well as reading
 *                existing books.
 * Arguments    : path - the new path
 */
void set_library_save_path(string path)
{
    library_save_path = path;
}

/*
 * Function name: query_library_save_path
 * Description  : Get the path in which the books are stored
 * Returns      : the path with a trailing /
 */
string query_library_save_path()
{
    // If no trailing slash is given, add it
    if(library_save_path[strlen(library_save_path) - 1] != '/')
        return library_save_path + "/";
    
    return library_save_path;
}

/*
 * Function name: add_new_book
 * Description  : Add a new book to the shelves
 * Arguments    : title - the title of the book
 *              : contents - the contents of the book
 *              : author - the author of the book
 *              : shelf (optional) - the shelf on which to place the book
 *              : filename (optional) - which filename to use for storing the
 *              :       content
 *              : book_id (optional) - the id by which the book is known
 * Returns      : a filename which contains the contents or 0 on failure
 */
varargs mixed add_new_book(string title, string contents, 
    string author, string shelf = "", string filename = "", 
    string book_id = "")
{
    // Check for empty values
    if(!strlen(shelf))
    {
        // Set the value to the new book shelf
        shelf = library_new_book_shelf;
    }

    // Find ourselves a filename
    if(!strlen(filename))
    {
        // Generate a filename
        filename = sprintf("%s%i", author, time());
    }

    // Generate a new book id
    if(!strlen(book_id))
    {
        // Find a new book id
        book_id = query_new_book_id();
    }

    // We cannot add books if we dont have a path to save them too
    if(!strlen(library_save_path))
    {
        return 0;
    }

    // Get the right priviledges for file operations
    seteuid(getuid());

    // Make sure the path exists
    if(file_size(library_save_path) != -2)
    {
        mkdir(library_save_path);
    }

    // See if the file exists
    if(file_size(query_library_save_path() + filename) > 0)
    {
        // Erase the file first
        rm(query_library_save_path() + filename);
    }

    // Write the contents to the file
    write_file(query_library_save_path() + filename, contents);

    // Add the book to the list
    library_data["books"][book_id] =
        ({ filename, title, shelf, author, 0 });

    // Log
    library_add_log("Added book '" + book_id + "'",
        "Filename '" + filename + "'\n" +
        "Title    '" + title + "'\n" +
        "Shelf    '" + shelf + "'");

    // Store everything
    save_library_data();
    
    // Return the filename
    return filename;
}

/*
 * Function name: update_book
 * Description  : Update the contents of a book
 * Arguments    : book_id - the id of the book
 *              : title - the new title of the book
 *              : contents - the text of the book
 * Returns      : 0 on failure, 1 on success
 */
int update_book(string book_id, string title, string contents)
{
    string filename;

    // See if we find this book
    if(member_array(book_id, m_indexes(library_data["books"])) == -1)
    {
        return 0;
    }

    // Fetch our filename
    filename = library_data["books"][book_id][0];

    // We cannot add books if we dont have a path to save them too
    if(!strlen(library_save_path))
    {
        return 0;
    }

    // Get the right priviledges for file operations
    seteuid(getuid());

    // Make sure the path exists
    if(file_size(library_save_path) != -2)
    {
        mkdir(library_save_path);
    }

    // See if the file exists
    if(file_size(query_library_save_path() + filename) > 0)
    {
        // Erase the file first
        rm(query_library_save_path() + filename);
    }

    // Write the contents to the file
    write_file(query_library_save_path() + filename, contents);

    // Update the book on the list
    library_data["books"][book_id][1] = title;

    // Log
    library_add_log("Updated contents of '" + book_id + "'",
        "Title '" + title + "'");

    // Store everything
    save_library_data();
    
    // Return success
    return 1;
}

/*
 * Function name: give_book_to_player
 * Description  : Called after a delay when a player wants to get a book
 * Arguments    : player - the name of the player
 *                book_id - the id of the book
 */
void give_book_to_player(object player, string book_id)
{
    object  book;
    string  *access, shelf;

    // Dont do anything is the player isnt here anymore
    if(environment(player) != this_object())
        return;

    // Find the shelves we have access to
    access = query_player_access(player);

    // See if we have the book
    if(member_array(book_id, m_indexes(library_data["books"])) != -1)
    {
        // See if the player has access to the book
        shelf = library_data["books"][book_id][2];
        if((member_array(shelf, m_indexes(library_data["shelves"])) != -1) && 
           (member_array(library_data["shelves"][shelf][1], access) != -1))
        {
            // We have access to the book, make sure someone else didnt borrow
            // it already
            if(member_array(book_id, library_borrowed_books) == -1)
            {
                // We have the book, the player has access, make the book 
                seteuid(getuid());
                book = clone_object(library_book_object);
                book->set_library(this_object());
                book->set_filename(query_library_save_path() + 
                    library_data["books"][book_id][0]);
                book->set_title(library_data["books"][book_id][1]);
                book->set_book_id(book_id);
            }
        }
    }

    // See if we return a failure, or the book
    if(objectp(book))
    {
        if(book->move(player))
        {
            // Write messages
            player->catch_msg("You find the " + library_book_type + 
                "that you were looking " +
                "for, but it is too heavy to lift it off the shelf.\n");
            tell_room(environment(player), QCTNAME(player) + " seems to " +
                "have found the " + library_book_type + " " + 
                player->query_pronoun() + " was " +
                "looking for but seems unable to lift it off the shelf.\n",
                player);

            // Destroy the book
            book->remove_object();
        }
        else
        {
            // Set the book as borrowed
            library_borrowed_books += ({ book_id });

            library_hook_receive_book(player, book);
        }
    }
    else
    {
        player->catch_msg("You do not seem able to find the " + library_book_type
            + " you were looking for.\n");
        tell_room(environment(player), QCTNAME(player) + " seems unable " +
            "to find the " + library_book_type + " " + player->query_pronoun() + 
            " was looking for.\n", player);
    }
}

/*
 * Function name: library_is_book_locked
 * Description  : See if the book is locked for editting
 * Arguments    : book_id - the id of the book
 * Returns      : 0 if not locked, 1 if locked
 */
int library_is_book_locked(string book_id)
{
    // See if we have the book
    if(member_array(book_id, m_indexes(library_data["books"])) == -1)
        return 0;

    // Return the locked value
    return library_data["books"][book_id][4];
}

/*
 * Function name: library_borrow_cmd
 * Description  : The code for the 'borrow' command
 * Arguments    : args - the command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_borrow_cmd(string args)
{
    // Syntax check
    if(!args || args == "")
    {
        notify_fail("Borrow what " + library_book_type + "?\n");
        return 0;
    }

    // Write a message and set an alarm
    write("You start searching the shelves for the " + library_book_type + ".\n");
    say(QCTNAME(this_player()) + " starts to search the shelves.\n");

    // Set the alarm
    set_alarm(4.0, 0.0, &give_book_to_player(this_player(), args));

    return 1;
}

/*
 * Function name: library_allow_new_books
 * Description  : Allow new books to be added to the library
 * Arguments    : allow - allow it or not
 */
void set_library_allow_new_books(int allow)
{
    library_allow_new_books = allow;
}

/*
 * Function name: library_blank_cmd
 * Description  : Allow the person executing the command to get a new book
 *              : so that they can submit new things to the library
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_blank_cmd(string args)
{
    object  book;

    if(!args || args != library_book_type)
    {
        notify_fail("Blank what? A blank " + library_book_type + "?\n");
        return 0;
    }

    seteuid(getuid());
    book = clone_object(library_book_object);
    book->set_library(this_object());

    if(book->move(this_player()))
    {
        // We cannot carry the book
        write("You attempt to take a blank " + library_book_type + 
            " from the shelf but it is too heavy.\n");
        say(QCTNAME(this_player()) + " attempts to take a blank " +
            library_book_type + " from the shelf, but it is too heavy.\n");

        book->remove_object();
    }
    else
    {    
        write("You take a blank " + library_book_type + " from the shelf.\n");
        say(QCTNAME(this_player()) + " takes a blank " + library_book_type + 
            " from the shelf.\n");
    }

    // Log
    library_add_log("Received blank book.");

    return 1;
}

/*
 * Function name: library_renumber_cmd
 * Description  : Assign a new number to a book
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_renumber_cmd(string args)
{
    string  old_id, new_id;

    // Librarian check
    if(!query_librarian(this_player()))
        return 0;

    // Syntax check
    if(!args || !parse_command(args, ({}), "%s 'to' %s", old_id, new_id))
    {
        notify_fail("Renumber what to what?\n");
        return 0;
    }

    // See if the old id exists
    if(member_array(old_id, m_indexes(library_data["books"])) == -1)
    {
        notify_fail("There is no " + library_book_type + " '" + old_id + "'.\n");
        return 0;
    }

    // Check if someone borrowed the book
    if(member_array(old_id, library_borrowed_books) != -1)
    {
        write("You cannot seem to find the " + library_book_type + 
            " on the shelf, perhaps someone borrowed it?\n");
        return 1;
    }

    // Do not allow locked books to be altered
    if(library_is_book_locked(old_id))
    {
        write("That " + library_book_type + " cannot be altered.\n");
        return 1;
    }

    // See if they are equal (add 0's to make it at least 4 chars)
    new_id = sprintf("%04s", new_id);
    if(new_id == old_id)
    {
        notify_fail("The names are both the same, no need to renumber.\n");
        return 0;
    }

    // See if the new number exists
    if(member_array(new_id, m_indexes(library_data["books"])) != -1)
    {
        write("There is already a " + library_book_type + " '" + new_id + "'.\n");
        return 1;
    }

    // Change everything
    library_data["books"][new_id] = library_data["books"][old_id] + ({});
    library_data["books"] = m_delete(library_data["books"], old_id);

    // Store everything
    save_library_data();
    
    // Log
    library_add_log("Renumbered '" + old_id + "' to '" + new_id + "'");

    library_hook_renumber_book(old_id, new_id);

    return 1;
}

/*
 * Function name: library_assign_cmd
 * Description  : Assign a book to a shelf
 * Arguments    : args - command line arguments
 * Return       : 0 on failure, 1 on success
 */
int library_assign_cmd(string args)
{
    string  book_id, shelf, old_shelf;

    // Librarian check
    if(!query_librarian(this_player()))
        return 0;

    // Syntax check
    if(!args || !parse_command(args, ({}), "%s 'to' %s", book_id, shelf))
    {
        notify_fail("Assign which " + library_book_type + " to which shelf?\n");
        return 0;
    }

    // See if the old id exists
    if(member_array(book_id, m_indexes(library_data["books"])) == -1)
    {
        notify_fail("There is no " + library_book_type + " '" + book_id + "'.\n");
        return 0;
    }

    // Do not allow locked books to be altered
    if(library_is_book_locked(book_id))
    {
        write("That " + library_book_type + " cannot be altered.\n");
        return 1;
    }

    // See if the shelf exists
    if(member_array(shelf, m_indexes(library_data["shelves"])) == -1)
    {
        write("There is no shelf '" + shelf + "'.\n");
        return 1;
    }

    // See if the book is already on that shelf
    if(library_data["books"][book_id][2] == shelf)
    {
        write("The " + library_book_type + " is already on that shelf.\n");
        return 1;
    }

    // Log
    library_add_log("Moved '" + book_id + "' to '" + shelf + "'");

    old_shelf = library_data["books"][book_id][2];

    // Assign the book to the shelf
    library_data["books"][book_id][2] = shelf;
    save_library_data();

    library_hook_move_book(book_id, old_shelf, shelf);
    return 1;
}

/*
 * Function name: library_create_cmd
 * Description  : Allow us to create a shelf
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_create_cmd(string args)
{
    string  name, title, result;

    // Librarian check
    if(!query_librarian(this_player()))
        return 0;

    // Syntax check
    if(!args || !parse_command(args, ({}), 
        "[shelf] %s 'as' / 'with' [title] %s", name, title))
    {
        notify_fail("Create what shelf with what title?\n");
        return 0;
    }

    // Do the renaming
    result = add_shelf(name, title);
    write(result);

    return 1;
}

/*
 * Function name: library_rename_cmd
 * Description  : Allow us to rename a shelf
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_rename_cmd(string args)
{
    string  from, to;

    // Librarian check
    if(!query_librarian(this_player()))
        return 0;

    if(!args || sscanf(args, "%s to %s", from, to) != 2)
    {
        notify_fail("Rename what shelf or book?\n");
        return 0;
    }

    if(member_array(from, m_indexes(library_data["books"])) != -1)
    {
        write(rename_book(from, to));
        return 1;
    }
    
    write(rename_shelf(from, to));
    return 1;    
}

/*
 * Function name: library_change_cmd
 * Description  : Allow us to change the description of a shelf
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_change_cmd(string args)
{
    string  name, description, result;

    // Librarian check
    if(!query_librarian(this_player()))
        return 0;

    // Syntax check
    if(!args || !parse_command(args, ({}), 
        "[the] [description] [of] [shelf] %s 'to' %s", name, description))
    {
        notify_fail("Change the description of what shelf to what?\n");
        return 0;
    }

    // Do the renaming
    result = change_shelf_description(name, description);
    write(result);

    return 1;
}

/*
 * Function name: library_access_cmd
 * Description  : Allow us to change the access level of a shelf
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_access_cmd(string args)
{
    string  name, level, result;

    // Librarian check
    if(!query_librarian(this_player()))
        return 0;

    // Syntax check
    if(!args || !parse_command(args, ({}), 
        "[the] [shelf] %s 'to' / 'for' [level] %s", name, level))
    {
        notify_fail("Access what shelf for who?\n");
        return 0;
    }

    // Do the renaming
    result = change_shelf_access(name, level);
    write(result);

    return 1;
}

/*
 * Function name: library_remove_cmd
 * Description  : Allow us to remove a shelf
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_remove_cmd(string args)
{
    string  name, result, filename;
    mixed   book_data;

    // Librarian check
    if(!query_librarian(this_player()))
        return 0;

    // Syntax check
    if(args && parse_command(args, ({}), 
        "[the] 'shelf' %s", name))
    {
        // Remove the shelf
        result = remove_shelf(name);
        write(result);

        return 1;
    }

    if(!args || !parse_command(args, ({}),
        "[the] [" + library_book_type + "] %s", name))
    {
        notify_fail("Remove what " + library_book_type + "?\n");
        return 0;
    }

    // See if the book exists
    if(member_array(name, m_indexes(library_data["books"])) == -1)
    {
        notify_fail("There is no " + library_book_type + " '" + name + "'.\n");
        return 0;
    }

    // Log
    library_add_log("Removed book '" + name + "'");

    seteuid(getuid());
    filename = library_data["books"][name][0];
    book_data = library_data["books"][name];

    // Make sure our obsolete path exists
    if(file_size(library_save_path + "obsolete") != -2)
    {
        mkdir(library_save_path + "obsolete");
    }

    // See if the file exists
    if(file_size(query_library_save_path() + "obsolete/" + filename) > 0)
    {
        // Erase the file first
        rm(query_library_save_path() + "obsolete/" + filename);
    }

    // Move the book to the obsolete dir
    write_file(query_library_save_path() + "obsolete/" + filename, 
        read_file(query_library_save_path() + filename));
    rm(query_library_save_path() + filename);

    // Remove the book (just the index)
    library_data["books"] = m_delete(library_data["books"], name);

    // Store everything
    save_library_data();

    library_hook_remove_book(name, book_data);

    return 1;
}

/*
 * Function name: library_help
 * Description  : Get the help based on this_player()'s permissions
 * Returns      : a string
 */
string library_help()
{
    string  result;

    // Header
    result = "The following commands are available in this library:\n\n";
    
    // Regular commands
    result += "list [shelf]     - list the available shelves, or " + 
                library_book_type + "s on that shelf.\n" +
              "catalog [shelf]  - same as the list command\n" +
              "borrow <" + library_book_type + ">    - borrow the " + 
              library_book_type + " with the id <book>\n";

    // Add text when we allow new books
    if(library_allow_new_books)
    {
        result += "\nYou can write your own " + library_book_type + 
            ", just get a 'blank " + library_book_type + 
            "' and 'submit' it when done.\n";
    }

    // Give the librarian extra options
    if(query_librarian(this_player()) || this_player()->query_wiz_level())
    {
        result += "\nThe following commands are available to you as " +
            "librarian:\n\n" +
            "renumber <old id> to <new id>\n" +
			"edit <" + library_book_type + " id>\n" +
            "assign  <" + library_book_type + " id> to <shelf name>\n" +
            "create shelf <shelf name> with title <shelf title>\n" +
            "rename <old shelf name> to <new shelf name>\n" +
            "rename <" + library_book_type + " id> to <new title>\n" +
            "change <shelf name> to <new title>\n" +
            "access <shelf name> for <level>\n" +
            "remove shelf <shelf name>\n" +
            "remove " + library_book_type + " <" + library_book_type + " id>\n";
    }

    // Gives more help for wizzes
    if(this_player()->query_wiz_level())
    {
        result += "\nImmortals can also 'lock' or 'unlock' specific " + 
            library_book_type + "s for editing.\n";
    }

    return result;
}

/*
 * Function name: library_lock_cmd
 * Description  : Allows an immortal to lock a book from editing
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_lock_cmd(string args)
{
    string  name;

    // Do a wizard check
    if(!this_player()->query_wiz_level())
        return 0;

    // Syntax check
    if(!args || !parse_command(args, ({}),
        "[the] [" + library_book_type + "] %s", name))
    {
        notify_fail(capitalize(query_verb()) + " what " + 
            library_book_type + "?\n");
        return 0;
    }

    // See if the book exists
    if(member_array(name, m_indexes(library_data["books"])) == -1)
    {
        notify_fail("There is no " + library_book_type + " '" + name + "'.\n");
        return 0;
    }

    // Check if we want to lock or unlock
    if(query_verb() == "lock")
    {
        if(library_data["books"][name][4])
        {
            write(capitalize(library_book_type) + " '" + name + 
                "' was already locked.\n");
            return 1;
        }
        else
        {
            write("Locked " + library_book_type + " '" + name + 
                "' for editing.\n");

            // Log
            library_add_log("Locked '" + name + "' for editing.");

            // Modify and save
            library_data["books"][name][4] = 1;
            save_library_data();

            return 1;
        }
    }

    // Check for unlocked books
    if(!library_data["books"][name][4])
    {
        write(capitalize(library_book_type) + " '" + name + "' was not locked.\n");
        return 1;
    }
    else
    {
        write("Allowed editing for " + library_book_type + " '" + name + "'\n");

        // Log
        library_add_log("Unlocked '" + name + "' for editing.");

        // Modify and save
        library_data["books"][name][4] = 0;
        save_library_data();

        return 1;
    }

}

/*
 * Function name: library_read_cmd
 * Description  : Allows players to read a scroll directly from the shelf
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_read_cmd(string args)
{
    string content, *access, shelf;

    if(!args || args == "")
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
        return 0;
    }

    // See if the book exists
    if(member_array(args, m_indexes(library_data["books"])) == -1)
    {
        notify_fail("There is no " + library_book_type + " '" + args + "'.\n");
        return 0;
    }

    // Find the shelves we have access to
    access = query_player_access(this_player());

    // See if we have the book
    if(member_array(args, m_indexes(library_data["books"])) != -1)
    {
        // See if the player has access to the book
        shelf = library_data["books"][args][2];
        if((member_array(shelf, m_indexes(library_data["shelves"])) != -1) && 
           (member_array(library_data["shelves"][shelf][1], access) != -1))
        {
            // We have access to the book, make sure someone else didnt borrow
            // it already
            if(member_array(args, library_borrowed_books) == -1)
            {
                seteuid(getuid());
                content = read_file(query_library_save_path() + 
                    library_data["books"][args][0]);

                library_hook_read_direct(args, content);

                return 1;
            }
        }
    }

    this_player()->catch_msg("You do not seem able to find the " + library_book_type
        + " you were looking for, perhaps someone borrowed it?\n");
    tell_room(environment(this_player()), QCTNAME(this_player()) + " seems unable " +
        "to find the " + library_book_type + " " + this_player()->query_pronoun() + 
        " was looking for.\n", this_player());

    return 1;
}

/*
 * Function name: done_editing
 * Description  : Called after editing is complete
 * Arguments    : book_id - the book being edited
 *              : text - the text of the book
 */
void done_editing(string book_id, string text)
{
	// Remove the book from the borrowed list
	library_borrowed_books -= ({ book_id });

    // Aborted
    if(!strlen(text))
    {
        write("Aborted.\n");
        return;
    }

    // Update the text
	update_book(book_id, library_data["books"][book_id][1], text);
    write("Ok.\n");
}

/*
 * Function name: library_edit_cmd
 * Description  : Allows direct editing of a scroll
 * Arguments    : args - command line arguments
 * Returns      : 0 on failure, 1 on success
 */
int library_edit_cmd(string args)
{
    string content;

    // Librarian check
    if(!query_librarian(this_player()))
        return 0;

    if(!args || args == "")
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
        return 0;
    }

    // See if the book exists
    if(member_array(args, m_indexes(library_data["books"])) == -1)
    {
        notify_fail("There is no " + library_book_type + " '" + args + "'.\n");
        return 0;
    }

	// Make sure someone else didnt borrow it already, or that it's being edited
	if(member_array(args, library_borrowed_books) != -1)
	{
		this_player()->catch_msg("You do not seem able to find the " + library_book_type
			+ " you were looking for, perhaps someone borrowed it?\n");
		tell_room(environment(this_player()), QCTNAME(this_player()) + " seems unable " +
			"to find the " + library_book_type + " " + this_player()->query_pronoun() + 
			" was looking for.\n", this_player());

		return 1;
	}

    // See if this book is locked from edditing
    if(library_is_book_locked(args))
    {
        write("You are not allowed to alter the contents of this " +
            library_book_type + ".\n");
        return 1;
    }

	// Read the contents of the book
	seteuid(getuid());
	content = read_file(query_library_save_path() + 
		library_data["books"][args][0]);

	// Set the book as borrowed to prevent people from editing it as well
	library_borrowed_books += ({ args });

    // Start editing
    clone_object(EDITOR)->edit(&done_editing(args), content);

	return 1;
}


/*
 * Function name: init_library
 * Description  : Add the library commands to the player
 */
void init_library()
{
    add_action(library_read_cmd,        "read");
    add_action(library_read_cmd,        "mread");
    add_action(library_list_cmd,        "list");
    add_action(library_list_cmd,        "catalog");
    add_action(library_borrow_cmd,      "borrow");

    // Allow the addition of books
    if(library_allow_new_books)
        add_action(library_blank_cmd,   "blank");

    // Give the librarian extra options
    if(!query_librarian(this_player()) && !this_player()->query_wiz_level())
        return;

    add_action(library_renumber_cmd,    "renumber");
    add_action(library_assign_cmd,      "assign");
    add_action(library_create_cmd,      "create");
    add_action(library_rename_cmd,      "rename");
    add_action(library_change_cmd,      "change");
    add_action(library_access_cmd,      "access");
    add_action(library_remove_cmd,      "remove");
	add_action(library_edit_cmd,		"edit");

    // Only allow wiz chars beyond this point
    if(!this_player()->query_wiz_level())
        return;

    // Allow immortals to lock books
    add_action(library_lock_cmd,        "lock");
    add_action(library_lock_cmd,        "unlock");
}

/*
 * Function name: add_library_access_level
 * Description  : Add a new access level
 * Arguments    : level - the short name of the level
 *              : desc - the description of the level
 */
void add_library_access_level(string level, string desc)
{
    // Make sure the mapping exists
    if(!mappingp(library_access_levels))
        library_access_levels = ([]);

    // Add the level
    library_access_levels[lower_case(level)] =
        desc;
}

/*
 * Function name: set_library_librarian_level
 * Description  : Set the level of the librarian
 * Arguments    : str - the short name of the level
 */
void set_library_librarian_level(string str)
{
    library_librarian_level = str;
}

/*
 * Function name: query_library_librarian_level
 * Description  : Returns the level of the librarian
 * Returns      : a string
 */
string query_library_librarian_level()
{
    return library_librarian_level;
}

/*
 * Function name: library_leave_inv
 * Description  : Called when someone leaves our inventory (the room)
 * Arguments    : ob - the object leaving
 *              : to - where are we leaving to
 */
void library_leave_inv(object ob, object to)
{
    object  *obs;

    // Check for living
    if(living(ob))
    {
        // Find all library books
        obs = deep_inventory(ob);
        obs = filter(obs, &->id("_linor_library_book"));

        // Make a librarian run after the unreturned books
        if(sizeof(obs))
        {
            ob->catch_msg("A librarian comes running after you and " +
                "takes " + COMPOSITE_DEAD(obs) + ".\n");
            tell_room(environment(ob), "A librarian runs in after " +
                QTNAME(ob) + " and takes " + COMPOSITE_DEAD(obs) + 
                " from " + ob->query_objective() + ".\n", ob);

            obs->return_library_book();
            obs->remove_object();
        }
    }
}

/*
 * Function name: prevent_leave_with_books
 * Description  : A function that can be used in add_exit, it will prevent
 *              : a player from leaving while carrying books.
 * Returns      : 0 when allowed to leave, 1 when blocked
 */
int prevent_leave_with_books()
{
    object *obs;

    // Find all library books
    obs = deep_inventory(this_player());
    obs = filter(obs, &->id("_guild_library_book"));

    // Stop with books
    if(sizeof(obs))
    {
        library_hook_stop_leave(this_player(), obs);
        return 1;
    }

    return 0;
}

/*
 * Function name: set_library_book_object
 * Description  : Set the book object to clone when a new book is needed
 * Arguments    : file - the filename of the object
 */
void set_library_book_object(string file)
{
    library_book_object = file;
}

/*
 * Function name: query_book_data
 * Description  : Get the information about a book
 * Arguments    : book_id - the id of the book
 * Returns      : the book data
 */
mixed query_book_data(string book_id)
{
    return library_data["books"][book_id];
}

/*
 * Function name: query_library_new_book_shelf
 * Description  : Return the value set by set_library_new_book_shelf
 * Returns      : a string
 */
string query_library_new_book_shelf()
{
    return library_new_book_shelf;
}

/* 
 * Function name: set_library_new_book_shelf
 * Description  : Set the shelf on which to publish new books
 * Arguments    : shelf - the name of the shelf
 */
void set_library_new_book_shelf(string shelf)
{
    library_new_book_shelf = shelf;
}
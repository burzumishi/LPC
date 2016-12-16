/*
 * /doc/examples/library/library.c
 * 
 * An example of a library that mortals can write books for and
 * borrow from.  This example demonstrates how to add shelves,
 * restrict library access to certain people, force new books to
 * be approved, enable borrowing of books, and prevent players
 * from leaving with borrowed books.  For a simpler example of
 * a library, see /doc/examples/library/simple_library.c.
 */

#pragma strict_types

inherit "/std/room";
inherit "/lib/library";

/*
 * Function name: library_exit_block
 * Description:   Check to see if the player is carrying a library
 *                book.  This is meant to be used as a block function
 *                for exits from the library so that books cannot be
 *                removed.
 * Returns:       1/0 - book carried/not carried
 */
public int
library_exit_block()
{
    /* See if the player is carrying a book */
    if (::library_exit_block())
    {
        /* A book was found.  Give a warning and return 1 to block the exit */
        write("You cannot leave with a library book!\n");
        return 1;
    }

    /* No book present.  Return 0 to allow the player to exit */
    return 0;
}

void
create_room()
{
    set_short("Library");
    set_long("A library.  A sign is hanging on the wall.  Note that because " +
        "of user id issues, this particular library will always fail to save " +
        "any books you try to write; this would not the case for libraries " +
        "created in domains.\n");

    /* Add an exit using library_exit_block() to be sure that no-one
     * leaves the library with a book
     */
    add_exit("balcony", "north", library_exit_block);

    /* Indicate that we want players to have to "borrow" a book in
     * order to read it.
     */
    set_borrow_required(1);

    /* This is the directory where approved books are saved */
    set_book_directory("/doc/examples/library/book_dir2/");
    
    /* This is the directory where books that need approval are saved */
    set_book_approval_directory("/doc/examples/library/book_dir2/approval/");

    /* This is the directory where books that have been denied or discarded
     * are saved.
     */
    set_book_removal_directory("/doc/examples/library/book_dir2/removed/");

    /* Add shelves to the library so that books can be classified under
     * different categories.  Note that for each shelf you must add a
     * subdirectory with the same name under the base directory specified
     * with set_book_directory().  For instance, we would have to add
     * two directories, /doc/examples/library/book_dir2/general/ and
     * /doc/examples/library/book_dir2/maps/ for the two shelves,
     * "general" and "maps", in this library.
     */
    add_book_shelf(({ "general", "maps" }));
    create_library();

    /* Add a sign that gives instructions on how to use the library */
    add_item("sign", library_help());
    add_cmd_item("sign","read", library_help());
}

void
init()
{
    ::init();

    /* Add the library commands */
    init_library();
}

/*
 * Function name: library_admin_access
 * Description:   Limit library administration abilities to certain
 *                people.
 * Returns:       1 - this_player() has administration powers
 *                0 - this_player() does not have administration powers
 */
public int
library_admin_access()
{
    /* We'll give admin access to council members of the occupational
     * guild "Some example guild"
     */
    if (this_player()->query_guild_member_occ("Some example guild") &&
        this_player()->query_guild_leader_occ())
    {
        return 1;
    }

    /* We'll give admin access to wizards */
    if (this_player()->query_wiz_level())
    {
        return 1;
    }

    /* No-one else has admin access */
    write("You do not have permission to do that.\n");
    return 0;
}

/*
 * Function name: library_write_access
 * Description:   Limit permission to write books to certain people
 * Returns:       1 - this_player() is allowed to write
 *                0 - this_player() is not allowed to write
 */
public int
library_write_access()
{
    /* We'll allow any member of the occupational guild "Some example guild"
     * to write books
     */
    if (this_player()->query_guild_member_occ("Some example guild"))
    {
        return 1;
    }

    /* We'll allow wizards to write books */
    if (this_player()->query_wiz_level())
    {
        return 1;
    }

    /* No-one else has permission to write books */
    write("You do not have permission to do that.\n");
    return 0;
}

/*
 * /doc/examples/library/simple_library.c
 * 
 * A simple example of a library that mortals can write books
 * for and borrow from.
 */

#pragma strict_types

inherit "/std/room";
inherit "/lib/library";

void
create_room()
{
    set_short("Library");
    set_long("A simple library.  A sign is hanging on the wall.  " +
        "Note that because of user id issues, this particular library " +
        "will always fail to save any books you try to write; this would " +
        "not the case for libraries created in domains.\n");

    /* This is the directory where approved books are saved */
    set_book_directory("/doc/examples/library/book_dir1/");
    
    /* Initialize the library.  This must be called *after* the library
     * is fully configured.
     */
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

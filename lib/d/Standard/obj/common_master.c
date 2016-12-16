/*
 * /d/Standard/obj/common_master.c
 *
 * This object is the common board that should contain notes that are
 * important to all players in Standard. If you want to have a common
 * board in your room, you should clone /d/Standard/obj/board.c
 *
 * Never clone this board. It is meant to be the central common board
 * that takes care of everything. Boards are very sensitive and we would
 * not like to have problems with it.
 *
 * If you are going to make a board reader or something like that,
 * the notes are kept in /d/Standard/room/com_board_data
 *
 * The original common board was coded by /Nick
 *
 * Complete revision to get rid of all problems with the common board:
 * /Mercade, 23 March 1994
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/std/board";

#include <macros.h>
#include <std.h>

#define COMMON_ROOM "/d/Standard/wiz/com"

/*
 * Global variable that will note be saved
 */
static object *clones; /* Cloned copies of this board */

/*
 * Function name: create_board
 * Description  : This function is called to create the board.
 */
nomask void
create_board()
{
    setuid();
    seteuid(getuid());

    clones = ({ this_object() });

    set_board_name("/d/Standard/room/com_board_data");
    set_num_notes(50);
    set_silent(0);
    set_show_lvl(1);
    set_remove_rank(WIZ_LORD);
    set_remove_str("Sorry, only a Lord may remove the notes.\n");
    set_no_report(0);
    set_keep_discarded(1);

    COMMON_ROOM->teleledningsanka();
    set_alarm(0.1, 0.0, &move(COMMON_ROOM));
}

/*
 * Function name: init
 * Desciprtion  : This function is called to link the board-commands
 *                to people who encounter the board. The commands related
 *                to the black list are only available for archwizards
 *                and keepers.
 */
nomask void
init()
{
    ::init();

    /*
     * Since remove_msg() in /std/board.c is declared nomask for security
     * reasons, this board should use another function that calls the
     * appropriate function in /std/board.c
     */
    add_action("remove_note", "remove");
}

/*
 * Function name: add_cloned_board
 * Description  : This function will be called each time a common board
 *                is cloned with the objectpointer to the board.
 * Arguments    : object - the objectpointer to the cloned board.
 */
nomask void
add_cloned_board(object board)
{
    if (!clones)
    {
        clones = ({ });
    }

    clones += ({ board });
}

/*
 * Function name: query_all_cloned_boards
 * Description  : Returns an array of all cloned boards.
 * Returns      : object * - the array of all cloned common boards.
 */
nomask object *
query_all_cloned_boards()
{
    /* remove all destructed boards. */
    clones = filter(clones, objectp);

    return secure_var(clones);
}

/*
 * Function name: notify_all_cloned_boards
 * Description  : This function gives a message to all rooms that have
 *                a copy of the common board about the fact that the
 *                notes on the board have changed.
 */
nomask void
notify_all_cloned_boards()
{
    int i;

    /* remove all destructed boards. */
    clones = filter(clones, objectp);

    for (i = 0; i < sizeof(clones); i++)
    {
        if ((objectp(environment(clones[i]))) &&
	    (environment(this_player()) != environment(clones[i])))
        {
            tell_room(environment(clones[i]),
                "The notes on the board have changed.\n");
        }
    }
}

/*
 * Function name: post_note_hook
 * Description  : This function is called when a note on this board is
 *                completed. It then gives a message to all people that
 *                are in a room with a common board. Only if the masked
 *                function returns true, a message was added. Otherwise
 *                it is possible that adding the message fails.
 * Arguments    : string head - the header of the note.
 */
nomask void
post_note_hook(string head)
{
    ::post_note_hook(head);

    notify_all_cloned_boards();
}

/*
 * Function name: remove_note
 * Description  : Since remove_msg() is declared nomask in /std/board
 *                this function is created.
 * Arguments    : int - the note to remove
 * Returns      : int - whatever was returned from remove_msg()
 */
nomask int
remove_note(int what_msg)
{
    int result = remove_msg(what_msg);

    if (result)
    {
        notify_all_cloned_boards();
    }

    return result;
}

/*
 * Function name: block_writer
 * Description  : This function checks whether someone is allowed to
 *                write a note on the common board. Only wizards are
 *                allowed to do so.
 * Returns      : 1/0 - disallowed/allowed
 */
nomask varargs int
block_writer()
{
    return (!(SECURITY->query_wiz_rank(geteuid(this_interactive()))));
}

/*
 * Function name: query_prevent_shadow
 * Description  : This function is to make sure that this object is never
 *                shadowed.
 * Returns      : 1 - always.
 */
nomask int
query_prevent_shadow()
{
    return 1;
}

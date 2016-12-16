/*
 * /d/Standard/obj/no_notes.c
 *
 * This object simply prevents a person carrying it from posting notes on a
 * bulletin board. It should be cloned into those who abused the privilege.
 *
 * /Mercade - November 4 2001
 */

inherit "/std/object";

#include <macros.h>
#include <std.h>
#include <stdproperties.h>

/*
 * Function name: create_object
 * Description  : Constructor.
 */
void
create_object()
{
    setuid();
    seteuid(getuid());

    set_name("_no_notes");
    set_short("_no_notes");
    set_long("This item prevents people from posting notes on a bulletin " +
        "board.\n");

    set_no_show();

    remove_prop(OBJ_I_VOLUME);
    remove_prop(OBJ_I_WEIGHT);
    add_prop(OBJ_I_NO_DROP,     1);
    add_prop(OBJ_I_NO_GIVE,     1);
    add_prop(OBJ_M_NO_SELL,     1);
    add_prop(OBJ_I_NO_STEAL,    1);
    add_prop(OBJ_I_NO_TELEPORT, 1);
    add_prop(OBJ_S_WIZINFO, "This item prevents people from posting notes " +
        "on a bulletin board.\n");
}

/*
 * Function name: enter_env
 * Description  : When we enter an interactive environment, add the property.
 * Arguments    : object to - the object we are entering.
 *                object from - the object we come from.
 */
void
enter_env(object to, object from)
{
    ::enter_env(to, from);

    to->add_prop(PLAYER_I_NO_NOTES, 1);
}

/*
 * Function name: leave_env
 * Description  : When we leave an interactive environment, remove the
 *                property.
 * Arguments    : object from - the object we come from.
 *                object to - the object we are entering.
 */
void
leave_env(object from, object to)
{
    ::leave_env(from, to);

    from->remove_prop(PLAYER_I_NO_NOTES);
}

/*
 * Function name: query_auto_load
 * Description  : This function returns the path to this object ot make it
 *                auto-load.
 * Returns      : string - the path.
 */
string
query_auto_load()
{
    return MASTER;
}

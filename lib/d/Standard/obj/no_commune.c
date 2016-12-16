/*
 * /d/Standard/obj/no_commune.c
 *
 * This object simply prevents a person carrying it from communing. It should
 * be cloned into those who have abused the privilege.
 *
 * /Mercade - February 20 1999
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

    set_name("_no_commune");
    set_short("_no_commune");
    set_long("This item prevents people from communing.\n");

    set_no_show();

    remove_prop(OBJ_I_VOLUME);
    remove_prop(OBJ_I_WEIGHT);
    add_prop(OBJ_I_NO_DROP,     1);
    add_prop(OBJ_I_NO_GIVE,     1);
    add_prop(OBJ_M_NO_SELL,     1);
    add_prop(OBJ_I_NO_STEAL,    1);
    add_prop(OBJ_I_NO_TELEPORT, 1);
    add_prop(OBJ_S_WIZINFO, "This item prevents people from communing.\n");
}

/*
 * Function name: no_commune
 * Description  : Blocking function to lecture the player and intercept the
 *                commune.
 * Arguments    : string str - the message that is intercepted.
 * Returns      : int 1 - always.
 */
int
no_commune(string str)
{
    /* Log the interception, so wizards may be amused. */
    SECURITY->commune_log(("Intercepted: " + str + "\n"), 1);

    write("\nYou have abused the privilege of communing beyond the limits " +
        "of what the administration of Standard is prepared to tolerate. " +
        "Therefore, the gift has been taken away from you.\n" +
        "You can contact the Archwizard of Players if you think this is " +
        "not correct (anymore).\n" +
        "Your words are not heard.\n\n");

    return 1;
}

/*
 * Function name: init
 * Description  : Used to link the bogus "commune" command to the player.
 */
void
init()
{
    ::init();

    add_action(no_commune, "commune");
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

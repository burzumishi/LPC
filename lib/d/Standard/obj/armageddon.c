/*
 * /d/Standard/obj/armageddon.c
 *
 * Revision history:
 * /Mercade, August 5th 1994, general revision of the Armageddon system.
 * /Mercade, January 17, 2001, moved Armageddon tell into mudlib.
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/secure/armageddon";

#include <std.h>

/* The Armageddon tell functionality is now in the mudlib. */
#undef ARMA_TELL_NOT_IN_MUDLIB
#ifdef ARMA_TELL_NOT_IN_MUDLIB
#define ARMA_TELL    "/d/Standard/obj/arma_tell"
#define ARMA_TELL_ID "_arma_tell_"
#endif ARMA_TELL_NOT_IN_MUDLIB

/*
 * Function name: shutdown_started
 * Description  : Whenever a shutdown is announced, all mortal players
 *                should get an object allowing them to use tell.
 */
void
shutdown_started()
{
#ifdef ARMA_TELL_NOT_IN_MUDLIB
    object *u = users() - ({ 0 });
    int     i;

    setuid();
    seteuid(getuid());

    for (i = 0; i < sizeof(u); i++)
    {
	if (u[i]->query_wiz_level())
	{
	    continue;
	}

	clone_object(ARMA_TELL)->move(u[i], 1);
    }
#endif ARMA_TELL_NOT_IN_MUDLIB
}

/*
 * Function name: shutdown_stopped
 * Description  : If a previously announced shutdown is canceled by
 *                the authorizing wizard, the players should loose their
 *                tell-object again.
 * Arguments    : string shutter - the one who decided not to shut down.
 */
void
shutdown_stopped(string shutter)
{
#ifdef ARMA_TELL_NOT_IN_MUDLIB
    object *u = users() - ({ 0 });
    object  o;
    int     i;

    for (i = 0; i < sizeof(u); i++)
    {
	while(objectp(o = present(ARMA_TELL_ID, u[i])))
	{
	    o->remove_object();
	}
    }
#endif ARMA_TELL_NOT_IN_MUDLIB
}

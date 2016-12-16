/*
 * /std/domain_link.c
 *
 * This module should be inherited by all domain masters and master objects
 * of the independant wizards. It takes care of the preloading and it
 * provides a link for some other general things.
 */

#pragma no_clone
#pragma save_binary
#pragma strict_types

#include <files.h>
#include <std.h>

/*
 * Global variables.
 */
private static string *gPreload  = ({ });
private static mapping commodity = ([ ]);

/*
 * Function name: create
 * Description  : Constructor.
 */
public nomask void
create()
{
    setuid();
    seteuid(getuid());
}

/*
 * Function name: preload
 * Description  : This function should be called from the preload_link()
 *                function of the domain. It enables the domain to load some
 *                modules at boot time.
 * Arguments    : string file - the filename of the file to preload.
 */
nomask void
preload(string file)
{
    gPreload += ({ file });
}

/*
 * Function name: query_preload
 * Description  : Get the list of preload files.
 * Returns      : string * - the list of files to preload.
 */
public nomask string *
query_preload()
{
    return secure_var(gPreload);
}

/*
 * Function name : add_commodity
 * Description   : function would add a commodity to the domain mapping
 *                 of commodities, it should be called from preload_link
 *                 for every new commodity
 * Arguments     : string  - the name of commodity
 *                 string  - the file of commodity
 * Returns       : void    - nothing is returned
 */
public void
add_commodity(string name, string file)
{
    if (strlen(name) && strlen(file))
    {
        commodity += ([ name : (file[-2..] == ".c" ? file[..-3] : file) ]);
    }
}

/*
 * Function name : query_commodity
 * Description   : function returns file representing a specific commodity
 * Arguments     : string  - the name of commodity
 * Returns       : string  - the file of commodity
 */
public string
query_commodity(string name)
{
    return commodity[name];
}

/*
 * Function name : query_commodities
 * Description   : function returns the names of commodities found within
 *                 the domain, it isnt used by commerce module itself but
 *                 has a heavy use in the commodities listing command
 * Arguments     : void    - no arguments
 * Returns       : string* - the names of domain commodities
 */
public string*
query_commodities()
{
    return m_indices(commodity || ([ ]));
}

/*
 * Function name: preload_link
 * Description  : This function should be masked by domains that want to
 *                preload some files at boot time. It should contain only
 *                calls to the function preload().
 */
public void
preload_link()
{
}

/*
 * Function name: armageddon
 * Description  : This function is called from SECURITY when it is time to
 *                close down the game. Note that this function should only
 *                be used for some basic domain administration as all domains
 *                should be processed in one run.
 * Arguments    : int level - the status of Armageddon, see the definitions
 *                     in <const.h>.
 *                ARMAGEDDON_ANNOUNCE - Armageddon wakes up and announces.
 *                ARMAGEDDON_CANCEL   - No shut down after all.
 *                ARMAGEDDON_SHUTDOWN - Game shuts down NOW.
 */
public void
armageddon(int level)
{
}

/*
 * Function name: delete_player
 * Description  : This function is called every time a player is deleted from
 *                the game. It can be used for domain specific code that is
 *                called to clean up information about the player.
 * Notice       : Do NOT redefine this function if it is not used. For extra
 *                stability this function is called using an alarm, so if it
 *                is not used, let us not waste alarms on functions that do not
 *                do anything anyways.
 * Arguments    : string name - the name of the deleted player.
 */
public void
delete_player(string name)
{
}

/*
 * Function name: domain_delete_player
 * Description  : Called from SECURITY to inform the domain that a particular
 *                player has been deleted. It will call the domain specific
 *                function delete_player(). See there for more details.
 * Arguments    : string name - the name of the deleted player.
 */
nomask public void
domain_delete_player(string name)
{
    /* Don't accept calls other than from security. */
    if (!CALLED_BY_SECURITY)
    {
        return;
    }

    /* If the function delete_player() has been redefined, use an alarm to
     * call it. Otherwise, don't bother.
     */
    if (function_exists("delete_player", this_object()) != DOMAIN_LINK_OBJECT)
    {
        set_alarm(1.0, 0.0, &delete_player(name));
    }
}

/*
 * Function name: query_restore_items
 * Descritpion  : Called to find out which items might be restored into the
 *                player. This could be guild items and quest items alike.
 *                Note that this routine is a test routine only. No action
 *                should be taken. It should return only those items that
 *                the player is entitled to, but that are missing in the
 *                player. The returned mapping looks as follows:
 * 
 *                    ([ (string) code : (string) description ])
 *
 *                The code is an internal string used to call restore_item().
 *                It should be unique for all items, and preferably contains
 *                the domain name in it. The description is a text displayed
 *                to the player that explains the item to be restored. It
 *                should be no more than 70 characters.
 *                This is a silent function. No text printed to the player!
 * Arguments    : object player - the player to check for restoration items.
 * Returns      : mapping - the items possible for restoration on this player.
 *
 * Example: ([ "Domain_rabbit_quest": "Lucky charm from Jack Rabbit quest" ])
 */
public mapping
query_restore_items(object player)
{
    return 0;
}

/*
 * Function name: restore_item
 * Descritpion  : Called to restore a certain item to a player. This means the
 *                item has to be cloned into the player and customised when
 *                so required. In principle, this routine is only called when
 *                the items have been checked for restoration, but you are
 *                suggested to perform the check to see whether the player is
 *                entitled to the object. In principle, no text needs to be
 *                printed to the player inside this routine.
 * Arguments    : object player - the player who needs an item restored.
 *                string item - the code name of the item. This is the string
 *                     returned as index in the mapping that was returned from
 *                     query_restore_items().
 * Returns      : int 1/0 - if true, the restoration is successful, else 0.
 */
public int
restore_item(object player, string item)
{
    return 0;
}

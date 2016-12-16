/*
 * /d/Standard/obj/accounts_lib.c
 *
 * This object stores the accounts the players have at the gnomes of
 * Standard.
 *
 * Coded by Tintin, 1992
 *
 * Version 2.0 by Mercade, 26 july 1995
 */

#pragma no_clone
#pragma save_binary
#pragma strict_types

inherit "/lib/cache.c";

#include <macros.h>
#include <std.h>

#define DEPOSIT_SIZE    (6)
#define DEPOSIT_LOG     ("PATCH_ACCOUNT")
#define DEPOSIT_FILE(n) (deposit_dir + extract((n), 0, 0) + "/" + (n))

/*
 * Global variable.
 */
static private string deposit_dir;

/*
 * Each account is saved in a separate file. The account is a mapping
 * with the following elements:
 *
 * ([ "cc" : (int) copper coins,
 *    "sc" : (int) silver coins,
 *    "gc" : (int) gold coins,
 *    "pc" : (int) platinum coins,
 *    "tm" : (int) last time modified,
 *    "fd" : (int) fees due in copper coins
 * ])
 */

/*
 * Prototype.
 */
static nomask void update_accounts(int letter);

/*
 * Function name: create
 * Description  : This function is called when the object is created.
 *                It makes sure that all accounts are up to date and
 *                every month it cleans up old accounts.
 */
nomask void
create()
{
    setuid();
    seteuid(getuid());

    set_alarm(10.0, 0.0, &update_accounts(0));

    this_object()->create_accounts();
}

/*
 * Function name: set_deposit_dir
 * Description  : With this function you should set the directory in which
 *                the accounts are placed. The directory should contain
 *                26 subdirectories of the letters of the alphabet in which
 *                the actual files will be stored.
 * Arguments    : string dir - the master directory for the save-files. This
 *                             name _must_ have a trailing slash "/".
 */
void
set_deposit_dir(string dir)
{
    deposit_dir = dir;
}

/*
 * Function name: query_deposit_dir
 * Description  : Get the name of the master directory for the save files
 *                for the Gnomes of Standard.
 * Returns      : string - the path.
 */
string
query_deposit_dir()
{
    return deposit_dir;
}

/*
 * Function name: query_account
 * Description  : See whether the player has an account and if so, return
 *                the account.
 * Arguments    : string name - the name of the player
 * Returns      : mapping - the account; for a description of the format,
 *                          see the header of this program or 0 in case of
 *                          an error.
 */
nomask public mapping 
query_account(string name)
{
    name = lower_case(name);
    if (!strlen(name))
    {
    	return 0;
    }

    return read_cache(DEPOSIT_FILE(name));
}

/*
 * Function name: set_account
 * Description  : With this function the account of a player can be set.
 * Arguments    : string name  - the name of the person.
 *                mapping info - the account information.
 * Returns      : int 1/0 - success/failure.
 */
public nomask int
set_account(string name, mapping info)
{
    name = lower_case(name);
    if (!strlen(name))
    {
    	return 0;
    }

    if (!mappingp(info) ||
    	(m_sizeof(info) != DEPOSIT_SIZE))
    {
    	return 0;
    }

    if (objectp(this_interactive()) &&
	(this_interactive()->query_real_name() != name))
    {
	log_file(DEPOSIT_LOG, ctime(time()) + " " +
	    FORMAT_NAME(this_interactive()->query_real_name()) + 
	    " patched the account of " + FORMAT_NAME(capitalize(name)) +
	    ".\n");
    }

    save_cache(info, DEPOSIT_FILE(name));
    return 1;
}

/*
 * Function name: remove_account
 * Description  : With this function the account of a player can be removed.
 * Arguments    : string name - the name of the person.
 * Returns      : int 1/0 - success/failure.
 */
public nomask int
remove_account(string name)
{
    name = lower_case(name);
    if (!strlen(name))
    {
    	return 0;
    }

    if (objectp(this_interactive()) &&
	(this_interactive()->query_real_name() != name))
    {
	log_file(DEPOSIT_LOG, ctime(time()) + " " +
	    FORMAT_NAME(this_interactive()->query_real_name()) + 
	    " removed the account of " + FORMAT_NAME(capitalize(name)) +
	    ".\n");
    }

    rm_cache(DEPOSIT_FILE(name));
    return 1;
}

/*
 * Function name: update_accounts
 * Description  : This function will loop over all accounts and checks
 *                whether they are still owned by a real player. If not,
 *                the account is removed.
 * Arguments    : int letter - the index to the next letter to be removed.
 */
static nomask void
update_accounts(int letter)
{
    string *files = get_dir(deposit_dir + ALPHABET[letter..letter] + "/" +
    			ALPHABET[letter..letter] + "*.o");
    int     index = -1;
    int     size  = sizeof(files);

    while(++index < size)
    {
    	/* Remove the account if it belongs to a wizard or to a mortal that
    	 * does not exist any more.
    	 */
    	if ((SECURITY->query_wiz_level(extract(files[index], 0, -3))) ||
    	    (!(SECURITY->exist_player(extract(files[index], 0, -3)))))
    	{
    	    rm_cache(DEPOSIT_FILE(files[index]));
    	}
    }

    if (++letter < strlen(ALPHABET))
    {
    	set_alarm(10.0, 0.0, &update_accounts(letter));
    }
}

/*
 * Function name: remove_object
 * Description  : Call this function to remove the object from the memory.
 * Returns      : int 1 - always.
 */
nomask public int
remove_object()
{
    destruct();
    return 1;
}

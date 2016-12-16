/* A simple room where you can join this guild. */

#pragma strict_types

inherit "/std/room";

#include "guild.h"

void
create_room()
{
    set_short("Guild joinroom");

    set_long(
         "You can join the " + GUILD_NAME + " here; just type \"occjoin\".\n" +
         "If you wish to leave the guild, type \"occleave\".\n");

    add_exit("start", "east");
}

int
occjoin(string str)
{
    object sh;
    mixed why;

    if (strlen(str))
    {
        return 0;
    }

    /* Clone the shadow */

    setuid();
    seteuid(getuid());

    sh = clone_object(GUILD_SHADOW);

    /* See if this member is acceptable */
    if (stringp(why = sh->acceptable_member(this_player())))
    {
        write(why);
    }
    else
    {
        /* Try to add the shadow to the player */
	switch (sh->add_occ_shadow(this_player()))
	{
	    case 1:
	        /* We have successfully shadowed the player, so we're set */
		write("Ok.\n");
		return 1;
    
	    case -4:
	        /* The player has an occ guild already, or one of his existing
                 * guilds has blocked this one.
                 */ 
		write("Your other guilds don't want you in this one!\n");
		break;
    
	    default:
	        /* Some error in shadowing */
		write("An error has prevented you from joining.\n");
		break;
	}
    }

    /* We couldn't shadow the player, so remove the shadow */
    sh->remove_shadow();
    return 1;
}

int
occleave(string str)
{
    if (strlen(str))
    {
        return 0;
    }

    if (!this_player()->query_guild_member(GUILD_NAME))
    {
        write("But you aren't a member of this guild!\n");
        return 1;
    }

    this_player()->remove_occ_member();
    write("Ok.\n");

    return 1;
}

void
init()
{
    ::init();
    add_action(occjoin, "occjoin");
    add_action(occleave, "occleave");
}

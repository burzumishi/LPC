/*
 * An example of a chapel to meditate in.
 *
 * Created by Nick.
 * Adapted by Mercade.
 */

inherit "/std/room";
inherit "/lib/guild_support"; 

#include <macros.h>
#include <stdproperties.h>

void
create_room()
{ 
    set_short("The chapel");
    set_long(break_string(
	"You are in the Chapel devoted to Paladine. There is a small altar " +
	"in one end where you can kneel and meditate to ask Paladine about " +
	"your stats, and also change your learning preferences.", 75) + "\n");

    add_prop(ROOM_I_INSIDE, 1);
    add_prop(ROOM_M_NO_ATTACK, "The feeling of peace is too great here.\n");
}

void
init()
{
    ::init();

    init_guild_support();
}

void
gs_hook_start_meditating()
{
    write(break_string(
	"You kneel before the altar of Paladine and close your eyes. A " +
	"feeling of great ease and self control falls upon you. You block " +
	"of your senses and concentrate solely upon your own mind. You " +
	"feel Paladine with you and he gives you the power to <estimate> " +
	"your different stats and <set> the learning preferences at your " +
	"own desire. Just <rise> when you are done meditating.", 75) + "\n");
    say(QCTNAME(this_player()) + " kneels down before the altar.\n");
}

int
gs_hook_rise()
{
    write(break_string(
	"As if ascending from a great depth, you rise to the surface of " +
	"your consciousness. You exhale and feel very relaxed as you get " +
	"up and leave the altar.", 75) + "\n");
    say(QCTNAME(this_player()) + " rises from the altar.\n");
}


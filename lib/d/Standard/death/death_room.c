/* death_room.c */

#pragma strict_types
#pragma no_clone

inherit "std/room";

#include <stdproperties.h>

#define DEATH_ROOM "/d/Standard/death/death_room"
#define WRITE(x) player->catch_tell(x)

mixed players;

/*
 * Prototypes
 */
void add_player(object plobj, int alarm);
void do_remove();
void remove_death_obj(object player);
void remove_player(object plobj);
int filter(string str);
void heart_beat(object player);

void
init()
{
    ::init();
    
    add_action(filter, "", 1);
    
    if (!this_player()->query_ghost())
    {
	write("The doctor says: Take two asprin and call me on Monday.\n");
	this_player()->move(this_player()->query_default_start_location());
	return;
    }
	
    add_player(this_player(), set_alarm(2.0, 2.0, &heart_beat(this_player()), this_player()));
}


/*
 * Function name: create_room
 * Description:   Reset the room
 */
void
create_room() 
{
    players = ({ });
    set_short("Hospital");
    set_long("A typical room in a hospital. There are arms and legs all " +
	     "over the place. This looks like the place for unrepareable " +
	     "bodies.\n");
}

/*
 * Function name: do_remove
 * Description:   Removes players that's finished
 */
void
do_remove()
{
    int j, nr;
    
    if (!sizeof(players))
	return;
    
    nr = sizeof(players);
    
    for (j = 0 ; j < nr ; j += 3)
    {
	if (players[j + 1] >= 70)
	{
	    if (players[j])
	    {
		remove_death_obj(players[j]);
		players[j]->reincarnate();
	    }
	    remove_alarm(players[j+2]);
	    players = exclude_array(players, j, j + 2);
	    do_remove();
	    return;
	}
    }
}

/*
 * Function name: remove_death_obj
 * Description:   Remove the "death_mark"-object, add badge.
 */
void
remove_death_obj(object player)
{
    object plobj, badge;
    
    plobj = present("death_mark", player);
    while (plobj = present("death_mark", player))
	plobj->remove_object();
}

/*
 * Function name: add_player
 * Description:   Adds a player to the list
 */
void
add_player(object plobj, int alarm)
{
    players = players + ({plobj, 0, alarm});
}

/*
 * Function name: remove_player
 * Description:   Removes a player from the list
 */
void
remove_player(object plobj)
{
    
    int i;
    
    if(!(i = sizeof(players)))
	return;
    
    i = member_array(plobj, players);
    if (i >= 0) {
	remove_alarm(players[i+2]);
	players = exclude_array(players, i, i + 2);
    }
}

/*
 * Function name: leave_inv
 * Description:   Remove players if they leave the room prematurly
 */
void
leave_inv(object ob, object to)
{
    remove_player(ob);
    ::leave_inv(ob, to);
}

/* 
 * Function name: filter
 * Description:   Filter out relevant commands.
 */
int
filter(string str)
{
    switch(query_verb())
    {
    case "quit":
    	write("The doctor says: Star Burgers will get your body!\n");
	return 0;
    case "look":
    case "take":
    case "get":
	return 0;
    }    
    write("That is impossible in your state.\n");
    return 1;
}

/*
 * Function name: heart_beat
 * Description:   Let the actions be governed by time.
 */
void
heart_beat(object player)
{
    int align, j, nr;
    
    if (!(nr = sizeof(players)))
	return 0;
    
    for (j = 0 ; j < nr ; j += 3)
	if (players[j] == player)
	    break;
    if (j >= nr)
	return;

    players[j + 1]++;

    if (players[j + 1] == 5)
    {
	WRITE("\nThe doctor says: Sorry pal. Your body is beyond repair.\n" +
	      "\nThe doctor shrugs and leaves the room.\n\n");
    }

    if (players[j + 1] == 10)
    {
	WRITE("\nYou feel lonely despite all the body parts around you.\n" +
	      "This body was good, and you are sure you will miss it.\n\n");
    }

    if (players[j + 1] == 15)
    {
	WRITE("\nTwo men enters the room. Without a word they put you on a\n" +
	      "cart and wheel you out of the room and down a corridor.\n\n");
    }

    if (players[j + 1] == 20)
    {
	WRITE("\nThey take down endless corridors, up with elevators, through " +
	      "more corridors and finally stop at a desc in a small room. " +
	      "They sign a paper handed to them by a man at a desk. You only " +
	      "manage to see that the top of a paper has a head saying:\n" +
	      "  'STAR BURGER - The best hamburger in universe!'.\n" +
	      "When the paper is handed back to the man behind the desk they " +
	      "take you through a hatch and into another room. There they place " +
	      "your body in front of a terminal.\n\n");
    }

    do_remove();
}

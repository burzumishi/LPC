/*    

   /std/room/link.c

   Cloned transport rooms in between
   ----------------------------------------------------------------------
   Defined functions and variables:


   ----------------------------------------------------------------------
*/

#include "/sys/macros.h"

static  object    *link_ends,     /* Links to endpoints, filenames of rooms */
                  *link_starts;   /* Link to cloned rooms, obj pointers */
static	string	  room_mlink;     /* For clones: master room */

#define LINK_ROOM "/std/room"

/*
 * Prototypes
 */
int transport_to(string ex, mixed room, int delay);
object link_room(string lfile, mixed dest, int pos);

/*
 * Function name: transport_to
 * Description:   Transport player to first linked room in clone chain.  
 *                Also creates the corridor and links it to the other
 *                endpoint if the corridor does not exist.
 * Arguments:	  ex:    The direction command (north, south ...)
 *                room:  The destination objectp or stringp
 *                delay: The length of the cloned corridor.
 * Returns:       True if player moved to first pos in corridor.
 *                False if corridor could not be created.
 */
public int
transport_to(string ex, mixed room, int delay)
{
    int ne, c;
    string backstr;
    object ls, or, lastr;

    /* Find or == objectp, room = file_name
	*/
    if (stringp(room))
    {
	or = find_object(room);
	if (!or && (LOAD_ERR(room)))
	    return 0;
	or = find_object(room);
    }
    else
    {
	or = room;
	if (!sscanf(file_name(or), "%s#", room))
	    room = file_name(or);
    }
    
    lastr = 0;

    /*
       Add one more if not there in the number of linked room chains from
       this room.
    */
    if (sizeof(link_ends))
    {
	ne = member_array(room, link_ends);
	if (ne >= 0)
	    lastr = link_starts[ne];
	else
	    link_ends = link_ends + ({ room });
    }
    else
	link_ends = ({ room });

    /* 
        Make the cloned corridor if it is not already made. Build by adding
        one room at a time from the destination position.
     */
    if (!lastr)
    {  
	/* Next to dest */
	if (!(lastr = this_object()->link_room(LINK_ROOM, or, delay))) 
	    return 0;
	backstr = (string) or->make_link(this_object(), lastr);
	lastr->add_exit(or, ex, 0);   
	lastr->link_master(room);    /* Destination is master of corridor */

	for (c = 1; c < delay; c++)
	{
	    ls = link_room(LINK_ROOM, or, delay - c);
	    ls->add_exit(lastr, ex, 0);
	    lastr->add_exit(ls, backstr, 0);                   
	    lastr = ls;
	    lastr->link_master(room);
	}
	lastr -> add_exit(this_object(), backstr, 0);
	link_starts = (sizeof(link_starts) 
		       ? link_starts + ({ lastr }) : ({ lastr }));
    }

    if (this_player()->move_living(ex, lastr))
	return 0;
    return 1;
}

/*
 * Function name: make_link
 * Description:   Find or make the return link. Called from other endpoint
 *                when linking a corridor between itself and this room.
 * Arguments:	  to_room:  File name of other endpoint
 *                via_link: Objectp to nearest cloned room in corridor.
 * Returns:       command used to move to other endpoint from this room.
 */
public string
make_link(mixed to_room, object via_link)
{
    int ne;
    mixed ex;

    if (!(sscanf(file_name(to_room), "%s#", to_room)))
	to_room = file_name(to_room);

    if (sizeof(link_ends))
    {
	ne = member_array(to_room, link_ends);
	if (ne >= 0)
	    link_starts[ne] = via_link;
	else
	{
	    link_ends = link_ends + ({ to_room });
	    ne = -1;
	}
    }
    else
    {
	link_ends = ({ to_room });
	ne = -1;
    }
  
    if (ne < 0)
    	link_starts = (link_starts ? 
		       link_starts + ({ via_link }) : ({ via_link }));
    
    ex = query_exit();
    ne = member_array(to_room, ex);

    return (ne >= 0 ? ex[ne + 1] : "back");
    /* We can't go back when we are at the end point, ie one way corridor */
}
  
/*
 * Function name: link_room
 * Description:   Create a corridor room. Set the descriptions of it.
 *                Does not add exits. This is supposed to be replaced by 
 *                the actual room inheriting this standard room. It is supposed
 *                to set proper descriptions (long and short) instead of
 *                copying the endpoint description. Corridor rooms are
 *		  created when the delay value in the third parameter to
 *		  add_exit() is a negative integer. Length of corridor is
 *		  the absolute value of the delay parameter.
 * Arguments:	  lfile: Suggested filename to clone to get a corridor room.
 *                dest:  The other endpoint of the corridor.
 *		  pos:   The position away from this startpoint
 * Returns:       Objectpointer of the cloned room.
 */
varargs public object
link_room(string lfile, mixed dest, int pos)
{
    object ob;
  
    ob = clone_object(lfile);

    ob->set_short(query_short()); /* Default same short as starting point */
    ob->set_long(query_long()); /* Default same long as starting point */
    ob->set_add_item(query_item()); /*Default add_items same as starting pt */
    ob->add_prop(ROOM_I_LIGHT, query_prop_setting(ROOM_I_LIGHT));
    ob->add_prop(ROOM_I_INSIDE, query_prop_setting(ROOM_I_INSIDE));
    ob->add_prop(ROOM_I_TYPE, query_prop_setting(ROOM_I_TYPE));
  
    return ob;
}

/*
 * Function name: link_master
 * Description:   Set the master room for this cloned corridor
 * Arguments:	  mfile: Filename of master room
 */
void
link_master(string mfile)
{
    room_mlink = mfile;
}

/*
 * Function name: query_link_master
 * Description:   Set the master room for this cloned corridor
 * Arguments:	  mfile: Filename of master room
 */
string
query_link_master()
{
    return room_mlink;
}



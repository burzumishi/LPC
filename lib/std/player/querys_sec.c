/*
 * /player/queriess_sec.c
 *
 * This is a subpart of /std/player_sec.c
 *
 * NOTE
 * There are some calls of the type: this_object()->function() in a number
 * of places. The reason for this is to allow those functions to be shadowed
 * as internal function calls cannot be shadowed.
 */

#include <ss_types.h>
#include <std.h>

static string *queue_list; /* People who are queueing [for an audience] */

/*
 * Function name:   query_npc
 * Description:     Tells if we are an npc or not
 * Returns:         0 (we aren't 8-)
 */
nomask int
query_npc()
{
    return 0;
}

/*
 * Function name:   query_average
 * Description:     Gives back the average value of all stats of a player
 * Returns:         The average
 */
nomask int
query_average()
{
    return this_object()->query_average_stat();
}

/*
 * Function name:   query_presentation
 * Description:     Gives a presentation of the living in one line. Including
 *                  name, race, guild titles, alignment and experience level.
 *                  This should only be displayed to met players.
 * Returns:         The description string
 */
public string
query_presentation()
{
    string str;

    if (!query_wiz_level())
    {
	return ::query_presentation();
    }

    return SECURITY->query_wiz_pretitle(this_object()) + " " +
	this_object()->query_name() +
	(strlen(str = query_title()) ? (" " + str) : "") +
	", " + this_object()->query_gender_string() + " " +
	this_object()->query_race_name()
#ifndef NO_ALIGN_TITLE
	+ " (" + (string)this_object()->query_al_title() + ")"
#endif
	; /* Yeah, yeah, a semi-colon on a separate line isn't nice. */
}

/*
 * Function name:   add_queue
 * Description:     Add a player to a queue.
 * Arguments:       who: A string with a player-name
 * Returns:         Number of people in the queue after addition.
 */
public int
add_queue(string who)
{
    /* Kludge to fix non-initalized variables */
    if (!pointerp(queue_list))
	queue_list = ({});

    if (member_array(who, queue_list) >= 0)
	return 0;

    queue_list += ({ who });
    return sizeof(queue_list);
}

/*
 * Function name:   pop_queue
 * Description:     Gives the first element of the queue with playernames and
 *                  removes it from the queue. If an argument is passed it
 *                  removes that element if it exists.
 * Arguments:       who: The name to remove from the queue, or nothing to
 *                  get the first element.
 * Returns:         The name that was the first element of the queue,
 *                  "" if there was no such name.
 */
public string
pop_queue(string who)
{
    string name;
    int index;

    if (!sizeof(queue_list))
	return "";

    if (!who)
    {
	index = 0;
    }
    else
    {
	if ((index = member_array(who, queue_list)) < 0)
	    return "";
    }

    name = queue_list[index];
    queue_list = exclude_array(queue_list, index, index);
    return name;
}

/*
 * Function name:   query_queue_list
 * Description:     Return the queue list with player-names.
 * Returns:         The queue-list
 */
public string *
query_queue_list()
{
    /* Kludge to fix non-initalized variables */
    if (!pointerp(queue_list))
	queue_list = ({});

    return secure_var(queue_list);
}

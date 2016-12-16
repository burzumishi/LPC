/*
 * /std/leftover.c 
 *
 * Base leftover code.
 *
 * This code basically works just like /std/food.c . The main difference
 * really is the set_decay_time() call which determines how long the object
 * can remain in a room before it decays, like a corpse. If the object is
 * put inside any container, not based on a room (like a player) it will
 * not decay. It will resume decaying as soon as it is put back in any room.
 * The decay time is measured in minutes.
 */

#pragma save_binary
#pragma strict_types

inherit "/std/food";

#include <cmdparse.h>
#include <composite.h>
#include <files.h>
#include <language.h>
#include <living_desc.h>
#include <macros.h>
#include <stdproperties.h>

/* Global variables. */
int decay_time;		/* The time it takes for the food to decay in min */
int simple_names;	/* Setup of simple names or not */
string l_organ, l_race; /* Race and organ name */
static private int decay_alarm;

public void decay_fun();

public void
create_leftover()
{
    simple_names = 1;
    add_name("leftover");
    add_pname("leftovers");
    set_amount(10);
}

string
long_description()
{
    if ((num_heap() < 2) ||
	(num_heap() >= 1000))
    {
	return "It is the torn and bloody remains of " + short() + ".\n";
    }
    else
    {
	return "They are the torn and bloody remains of " + short() + ".\n";
    }
}

/*
 * Function name: leftover_init
 * Description  : Call this function to initialise the leftover. Specifically,
 *                this sets the property and adds the names and descriptions.
 * Arguments    : string organ - the organ.
 *                string race - the race of the corpse.
 */
public void
leftover_init(string organ, string race)
{
    if (simple_names)
    {
	set_adj(race);
	set_name(organ);
	add_name("_leftover_" + organ);
	set_pname(LANG_PWORD(organ));
        set_short(race + " " + organ);
	set_long(long_description);
	
	if (IN_ARRAY(organ, LD_BONES))
	{
	    add_name("bone");
	    add_pname("bones");
	}
	if (IN_ARRAY(organ, LD_ORGANS))
	{
	    add_name("organ");
	    add_pname("organs");
	}
    }

    add_prop(HEAP_S_UNIQUE_ID, "_leftover_" + race + "_" + organ);
    l_organ = organ;
    l_race = race;
}

/*
 * Function name: config_split
 * Description  : When the heap is split, copy the race and organ variables.
 * Arguments    : int new_num - the size of the new heap.
 *                object orig - the original heap object.
 */
void
config_split(int new_num, object orig)
{
    ::config_split(new_num, orig);

    /* Copy the race and organ variables, and initialise the leftover. */
    leftover_init(orig->query_organ(), orig->query_race());
}

/*
 * Function name: create_food
 * Description  : Constructor. You must mask create_leftover() to create your
 *                own leftover.
 */
public nomask void
create_food() 
{ 
    simple_names = 0;
    decay_time = 10;

    create_leftover();
}

/*
 * Function name: set_decay_time
 * Description  : Set the decay time in minutes that it will take to decay
 *                the leftover when it is not property contained.
 * Arguments    : int time - the time in minutes. Default: 10 minutes
 */
public void
set_decay_time(int time)
{
    decay_time = time;
}

/*
 * Function name: query_decay_time
 * Description  : Returns the time in minutes it takes for the leftover to
 *                decay when it is not properly contained.
 * Returns      : int - the decay time in minutes.
 */
public int
query_decay_time()
{
    return decay_time;
}

/*
 * Function name: query_race
 * Description  : Returns the race as set with leftover_init().
 * Returns      : string - the race.
 */
public string
query_race()
{
    return l_race;
}

/*
 * Function name: query_organ
 * Description  : Returns the organ as set with leftover_init().
 * Returns      : string - the organ.
 */
public string
query_organ()
{
    return l_organ;
}

/*
 * Function name: enter_env
 * Description  : When the leftover is dropped in a room, start the count
 *                down to the decay routine.
 */
public void 
enter_env(object dest, object old) 
{
    ::enter_env(dest, old);

    remove_alarm(decay_alarm);
    if (IS_ROOM_OBJECT(dest))
    {
	decay_alarm = set_alarm(1.0, 0.0, decay_fun);
    }
}

/*
 * Function name: decay_fun
 * Description  : Decays the leftover after a predefined period of time when
 *                the leftover is dropped in a room.
 */
public void
decay_fun()
{
    if (--decay_time)
    {
	decay_alarm = set_alarm(60.0, 0.0, decay_fun);
    }
    else
    {
        tell_room(environment(this_object()),
	    capitalize(LANG_THESHORT(this_object())) +
            " rapidly " + ((num_heap() == 1) ? "decays" : "decay") +
	    ".\n");
	remove_object();
    }
}

/*
 * Function name:	consume_them 
 * Description:		Consumes a number of food item. This function
 *			shadows the ordinary /std/food funcion.
 *			All this to detect cannibals...
 * Arguments:		arr: Array of food objects.
 */
public void
consume_them(object *arr)
{
    foreach(object obj: arr)
    {
        if (obj->query_race() == this_player()->query_race_name())
	{
	    write("You feel a bit uneasy doing this, but...\n");
	    this_player()->add_prop("cannibal", 1);
	}
	obj->delay_destruct();
    }
}

/*
 * Function name: query_recover
 * Description  : Get the recover string, so that the race and organ are
 *                preserved.
 * Returns      : string - the recover string.
 */
public string
query_recover()
{
    return MASTER + ":" + num_heap() + "|" + query_amount() +
        query_prop(HEAP_S_UNIQUE_ID);
}

/*
 * Function name: init_recover
 * Description  : Re-initialise the race and organ.
 * Arguments    : string - the recover arguments.
 */
public void
init_recover(string arg)
{
    string *args;

    args = explode(arg, "_");
    if (sizeof(args) == 4)
    {
        leftover_init(args[3], args[2]);

        /* Backwards compatibility, only set weight if it exists. */
        args = explode(args[0], "|");
        set_heap_size(atoi(args[0]));
        if (sizeof(args) > 1)
        {
            set_amount(atoi(args[1]));
        }
    }
    else
    {
	remove_object();
    }
}

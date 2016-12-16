/*  
 * /std/room.c
 *
 * This is the room object. It should be inherited by all rooms.
 */

#pragma save_binary
#pragma strict_types

inherit "/std/container";

#include <files.h>
#include <filter_funs.h>
#include <macros.h>
#include <ss_types.h>
#include <std.h>
#include <stdproperties.h>
#include <cmdparse.h>
#include <subloc.h>

#include "/std/room/exits.c"
#include "/std/room/description.c"
#include "/std/room/link.c"
#include "/std/room/move.c"
#include "/std/room/terrain.c"
#include "/std/room/objects.c"

static object   room_link_cont;	/* Linked container */
static object   *accept_here = ({ }); /* Items created here on roomcreation */

/*
 * Function name: create_room
 * Description  : Constructor. You should redefine this function to create
 *                your own room.
 */
public void
create_room()
{
    set_terrain(TERRAIN_OUTSIDE | TERRAIN_NATURAL);
    remove_prop(ROOM_I_INSIDE);          /* Default room has heaven above */
    add_prop(ROOM_I_TYPE, ROOM_NORMAL);  /* Default is a room */
}

/*
 * Function name: create_container
 * Description  : Constructor. Since you may not redefine this function,
 *                you must define the function create_room() to create your
 *                room.
 */
nomask void
create_container()
{
    add_prop(ROOM_I_IS,    1);
    add_prop(ROOM_I_LIGHT, 1);
    add_prop(ROOM_I_HIDE, 10);
    
    room_link_cont = 0;

    seteuid(creator(this_object()));

    /* As service to the folks, we automatically call the function
     * enable_reset() to start resetting if the function reset_room() has
     * been redefined.
     */
    if (function_exists("reset_room", this_object()) != ROOM_OBJECT)
    {
	enable_reset();
    }

    create_room();
    reset_auto_objects();

    accept_here = all_inventory(this_object());
    if (!sizeof(accept_here))
	accept_here = ({ });
}

/*
 * Function name: reset_room
 * Description  : This function should be redefined to make the room reset
 *                every half hour or so. If you redefine it, you do not have
 *                to call enable_reset() since we call it as part of our
 *                service ;-) Note that this service is only valid for rooms.
 */
void
reset_room()
{
}

/*
 * Function name: cleanup_loot
 * Description  : This function will clean all loot in the room that was
 *                dropped here. It will only function if no interactive
 *                players are in the room.
 */
void
cleanup_loot()
{
    object *inv;

    /* Only do it if there are no interactives in the room. */
    inv = all_inventory(this_object());
    if (sizeof(FILTER_PLAYERS(inv)))
    {
        return;
    }

    /* Find all items that were dropped here by dead livings. */
    inv = filter(inv, &operator(==)(this_object()) @
        &->query_prop(OBJ_O_LOOTED_IN_ROOM));

    /* Destruct those items. */
    inv->remove_object();
}

/*
 * Function name: reset_container
 * Description  : This function will reset the container. Since you may not
 *                redefine it, you must define the function reset_room() to
 *                make the room reset.
 */
nomask void
reset_container()
{
    cleanup_loot();

    reset_auto_objects();
    reset_room();

    if (!sizeof(accept_here))
	accept_here = ({ });
    else
	accept_here = filter(accept_here, objectp);
}

/*
 * Function name: clone_here
 * Description  : The behaviour of this function is exactly the same as the
 *                efun clone_object(). It clones the 'file' and returns the
 *                objectpointer to that object. However, it will also add
 *                the object to a list of items that 'belongs' in this room.
 *                This means that the presence of this object in the room will
 *                not prevent the room from being cleaned with clean_up().
 * Arguments    : string file - the path to the item to clone.
 * Returns      : object - the objectpointer to the clone.
 */
public object
clone_here(string file)
{
    object ob;
    
    ob = clone_object(file);
    accept_here += ({ ob });
    return ob;
}

/*
 * Function name: query_cloned_here
 * Description  : Returns all objects that have been cloned in this room with
 *                clone_here() or registered with add_accepted_here().
 * Returns      : object * - the list of objects.
 */
public object *
query_cloned_here()
{
    return secure_var(accept_here);
}

/*
 * Function name: add_accepted_here
 * Description  : With this function, you can register an object as being
 *                accepted in this room. This means that the object will not
 *                prevent the room from being cleaned up. It will give an
 *                item the same status as when it was cloned with clone_here()
 *                in this room.
 * Arguments    : object ob - the object to register.
 */
void
add_accepted_here(object ob)
{
    accept_here += ({ ob });
}
 
/*
 * Function name: light
 * Description:   Returns the light status in this room
 *                This function is called from query_prop() only.
 * Returns:	  Light value
 */
nomask int
light()
{
    int li;
    
    li = query_prop(ROOM_I_LIGHT);
    if (objectp(room_link_cont))
    {
	if ((environment(room_link_cont)) &&
	    (room_link_cont->query_prop(CONT_I_TRANSP) ||
	     room_link_cont->query_prop(CONT_I_ATTACH) ||
	    !room_link_cont->query_prop(CONT_I_CLOSED)))
	{
	    li += (environment(room_link_cont))->query_prop(OBJ_I_LIGHT);
	}
    }
    return query_internal_light() + li;
}

/*
 * Function name: set_container
 * Description:   Sets the container for which the room represents the inside
 * Arguments:	  ob: The container object
 */
public void
set_container(object ob)
{
    room_link_cont = ob;
}

/*
 * Function name: set_room
 * Description  : This function is a mask for the function set_room() in
 *                /std/container.c. That function is not valid for rooms,
 *                so we block it here.
 * Arguments    : the arguments described in /std/container.c
 */
public nomask void
set_room(mixed room)
{
}

/* 
 * Function name: update_internal
 * Description:   Updates the light, weight and volume of things inside
 *                also updates a possible connected container.
 * Arguments:     l: Light diff.
 *		  w: Weight diff. (Ignored)
 *		  v: Volume diff. (Ignored)
 */
public void
update_internal(int l, int w, int v)
{
    ::update_internal(l, w, v);

    if (room_link_cont)
	room_link_cont->update_internal(l, w, v);
}

/*
 * Function name: clean_up
 * Description  : This function destruct the room if there is nothing in it.
 *                If you have special variables stored in a room you should
 *		  define your own clean_up(). Also if you on startup of the
 *		  room clone some objects and put inside it, please define
 *		  your own clean_up() to destruct the room. This saves a
 *		  lot of memory in the game.
 * Returns      : int 1/0 - call me again/ don't bother me again.
 */
public int
clean_up()
{
    /* Do not destroy the room object. */
    if (MASTER == ROOM_OBJECT)
    {
	return 0;
    }

    if (!query_prop(ROOM_I_NO_CLEANUP) &&
	!sizeof(all_inventory(this_object()) - accept_here))
    {
	remove_object();
    }

    return 1;
}

/*
 * Function name: stat_object
 * Description:   Called when someone tries to stat the room
 * Returns:	  A string to write
 */
string
stat_object()
{
    string str;
    int type;

    str = ::stat_object();

    if (query_prop(ROOM_I_INSIDE))
	str += "inside\t";
    else
	str += "outside\t";

    type = query_prop(ROOM_I_TYPE);
    str += " ";
    switch (type)
    {
    case 0: str += "normal"; break;
    case 1: str += "in water"; break;
    case 2: str += "under water"; break;
    case 3: str += "in air"; break;
    case 4: str += "beach"; break;
    default: str += "unknown type"; break;
    }
    str += "\t";

    return str + "\n";
}

/*
 * Function name: query_domain
 * Description  : This function will return the name of the domain this
 *                room is in.
 * Returns      : string - the domain name.
 */
nomask string
query_domain()
{
    /* Normal room. */
    if (wildmatch("/d/*", file_name(this_object())))
    {
	return explode(file_name(this_object()), "/")[2];
    }

    /* Link-room. */
    if (query_link_master() &&
	wildmatch("/d/*", query_link_master()))
    {
	return explode(query_link_master(), "/")[2];
    }

    /* This shouldn't happen. */
    return BACKBONE_UID;
}

/*
 * Function name: block_action
 * Description:   Rooms can prevent targeted commands from being executed
 *                using this function.  By default, we just check to see
 *                if the subloc the target occupies can be accessed, but
 *                this can be redefined to block commands based on other
 *                criteria.
 *
 * Arguments:     string cmd    - the name of the executed command
 *                object actor  - the command performer
 *                object target - the target of the command
 *                int cmd_type  - the command attributes (from cmdparse.h) 
 * Returns:       0 - command allowed
 *                1 - command blocked, no error message provided
 *                string - command blocked, use string as error message.
 */
public mixed
block_action(string cmd, object actor, object target, int cmd_type)
{
    string subl, acs_type;

    /* Check for subloc restrictions */

    /* No problem if both actor and target are in the same subloc */
    if ((subl = actor->query_subloc()) == target->query_subloc())
    {
        return 0;
    }

    /* Determine what sort of subloc access we need */
    acs_type = ((cmd_type & (ACTION_CONTACT | ACTION_PROXIMATE)) ?
        SUBLOC_ACCESS_MANIP : SUBLOC_ACCESS_SEE);

    return subloc_cont_access(subl, acs_type, target);
}

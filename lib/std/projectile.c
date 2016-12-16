/*
 * /std/projectile.c
 *
 * This class is the base for all launched projectiles like
 * arrows, bolts and slingshots. The only known subclass at
 * this point is /std/arrow.c. You should probably not inherit
 * this class directly, but use one of its known subclasses
 * instead.
 */

#pragma strict_types

inherit "/std/heap";

#include <macros.h>
#include <language.h>
#include <ss_types.h>
#include <stdproperties.h>
#include <wa_types.h>

/* Global variables */
static string projectile_id;
static int hit,
           pen,
           broken,
           loaded,
           defered_join;

/* Prototype */
public void update_heap_id();

/*
 * Function name: create_projectile
 * Description  : Constructor. Mask this function if you want to create a
 *                projectile.
 */
public void
create_projectile()
{
    return;
}

/*
 * Function name: create_heap
 * Description  : Constructor. This sets some internal state of the projectile
 *                Mask create_projectile instead if you want to create a 
 *                projectile.
 */
public nomask void
create_heap()
{
    broken = 0;
    will_not_recover = (random(100) < PERCENTAGE_OF_RECOVERY_LOST);
    create_projectile();
    add_prop(HEAP_S_UNIQUE_ID, projectile_id);
}

/*
 * Function name: enter_env
 * Description  : Update the heap_id when we enter a new environment, so
 *                the projectiles stack correctly.
 *
 * Arguments    : object to - the object we are entering.
 *                object from - the object we come from.
 */
public void
enter_env(object to, object from)
{
    /* That this is necessary means something else is wrong! /Mercade. */
    if (!defered_join && to == this_player())
    {
	/*
	 * We have to defer the join a little, or else we get
	 * a very ugly message when we pick up the arrows.
	 */
	set_no_merge(1);
	defered_join = 1;	
	set_alarm(0.2, 0.0, &enter_env(to, from));
	::enter_env(to, from);
    }
    else if (objectp(to))
    {
	defered_join = 0;
	this_object()->update_heap_id();
	::enter_env(to, from);
    }
}

/*
 * Function name: load
 * Description  : This function marks this projectile as loaded and hides it
 *                from the player shooting it.
 */
public nomask void
load()
{
    set_no_show();
    loaded = 1;
    this_object()->update_heap_id();
}

/*
 * Function name: unload
 * Description  : This function marks this projectile as unloaded and makes
 *                it visible to the player again.
 */
public nomask void
unload()
{
    unset_no_show();
    unset_no_show_composite();
    loaded = 0;
    this_object()->update_heap_id();
}

/*
 * Function name: query_broken
 * Description  : Gives information of the broken status of this projectile.
 * Returns      : 1 - Projectile broken. 0 - Projectile whole.
 */
public int
query_broken()
{
    return broken;
}

/*
 * Function name: set_broken
 * Description  : This functions sets the broken status of this projectile.
 * Arguments    : int b - The new broken status. 1 - Broken. 0 - Whole.
 */
public void
set_broken(int b)
{
    broken = b;

    if (broken)
    {
        add_adj("broken");
	this_object()->update_heap_id();
    }
    else
    {
        remove_adj("broken");
	this_object()->update_heap_id();
    }
}

/*
 * Function name: set_hidden
 * Description  : This functions updates the heap_id of the arrow so it
 *                stacks properly whith other hidden/broken arrows as well.
 */
public void
update_heap_id()
{
    int level;

    level = query_prop(OBJ_I_HIDE);
    
    add_prop(HEAP_S_UNIQUE_ID,
	     (loaded ? "loaded_" : "") +
	     (broken ? "broken_" : "") +
	     (level ? "hidden_" + level + "_" : "") +
	     projectile_id);
}

/*
 * Function name: config_split
 * Description  : Copies the internal state of this projectile when it is
 *                split from a heap.
 * Arguments    : int new_num - the number of coins in this new heap.
 *                object orig - the heap we are split from.
 */
void
config_split(int new_num, object orig)
{
    if (orig->query_broken())
        set_broken(1);

    hit = orig->query_hit();
    pen = orig->query_pen();

    ::config_split(new_num, orig);
}

/*
 * Function name: query_hit
 * Description  : Returns the to hit value of this projectile.
 * Returns      : (int) Hit value.
 */
public int
query_hit()
{
    return hit;
}

/*
 * Function name: query_pen
 * Description  : Returns the penetration value of this projectile.
 * Returns      : (int) Penetration value.
 */
public int
query_pen()
{
    return pen;
}

/*
 * Function name: set_hit
 * Description  : Sets the to hit value of this projectile.
 * Arguments    : (int) To hit value.
 */
public void
set_hit(int h)
{
    hit = h;
}

/*
 * Function name: set_pen
 * Description  : Sets the penetration value of this projectiles.
 * Arguments    : (int) Penetration value.
 */
public void
set_pen(int p)
{
    pen = p;
}

/*
 * Function name: stat_object
 * Description  : This function is called when a wizard wants to get more
 *                information about an object.
 * Returns      : strint - The string to write.
 */
public string
stat_object()
{
    return ::stat_object() +
      "Hit: " + hit + "\t\t\tPen: " + pen + "\n" +
      "Heap id: " + query_prop(HEAP_S_UNIQUE_ID) + "\n";
}

/*
 * Function name: set_projectile_id
 * Description  : Sets the id of this projectile. Projectiles with the same
 *                id will stack together.
 * Arguments    : string - Projectile id.
 */
public void
set_projectile_id(string id)
{
    projectile_id = id;
}

/*
 * Function name: set_long
 * Description  : This function has been declared nomask in projectiles.
 *                Redefine the get_projectile_long to return the proper
 *                long description instead.
 */
private nomask void
set_long(string long)
{
    return;
}

/*
 * Function name: get_projectile_long
 * Description  : Use this function to return the proper long description
 *                of this projectile.
 * Arguments    : string str     - the pseudo-item to describe. Not used in
 *                                 this routine. It's intercepted in long().
 *                object for_obj - the object trying to get the long.
 *                int num        - The number of projectiles in this stack.
 * Returns      : string         - the description of the object or
 *                                 pseudo-item.
 */
public string
get_projectile_long(string str, object for_obj, int num)
{
    if (num == 1)
    {
	return "A standard projectile.\n";
    }
    else
    {
	return "A bunch of standard projectiles.\n";
    }
}

/*
 * Function name: long
 * Description  : Describe the object or one pseudo item in it. This
 *                resolves possible VBFC. This long function appends
 *                the broken status of a projectile to the long 
 *                description that get_projectile_long returns. You 
 *                should probably redefine get_projectile_long instead 
 *                of this function.
 * Arguments    : string str - the pseudo-item to describe. This is an
 *                             item added with add_item. If this is 0, it
 *                             will return the description of the whole
 *                             object.
 *                object for_obj - the object trying to get the long.
 * Returns      : string - the description of the object or pseudo-item.
 */

varargs public mixed
long(string str, object for_obj)
{
    /* Refer to the basic long for add_items. */
    if (str)
    {
        return ::long(str, for_obj);
    }

    return get_projectile_long(str, for_obj, num_heap()) +
	  (broken ? ((num_heap() > 1) ? "They are broken.\n" : "It is broken.\n") : "");
}

/*
 * Function name: singular_short
 * Description  : This function will return the singular short descritpion
 *                for this projectile prepended with the broken status of
 *                the projectile.
 * Arguments    : object for_obj - who wants to know.
 * Returns      : string - the singular short description.
 */
public string
singular_short(object for_obj)
{
    return (broken ? "broken " : "" ) + ::singular_short(for_obj);
}

/*
 * Function name: short
 * Description  : Get the short description for the projectile. Prepends
 *                the proper description of the broken status.
 * Arguments    : object for_obj - who wants to know.
 * Returns      : string - the short description.
 */
public varargs string
short(object for_obj)
{
    string str;

    if (!strlen(query_prop(HEAP_S_UNIQUE_ID)))
    {
        set_alarm(0.0, 0.0, remove_object);
        return "ghost " + singular_short(for_obj);
    }

    if (num_heap() < 1)
    {
        return 0;
    }
 
    if (num_heap() < 2)
    {
        str = singular_short(for_obj);
        return LANG_ADDART(str);
    }

    str = (broken ? "broken " : "") + plural_short(for_obj);

    if (num_heap() < 12) 
    {
	return LANG_WNUM(num_heap()) + " " + str;
    }
    
    if (this_player()->query_stat(SS_INT) / 2 > num_heap())
    {
	return num_heap() + " " + str;
    }

    return (num_heap() < 1000 ? "many " : "a huge heap of ") + str;
}


/*
 * Function name: projectile_hit_target
 * Description  : A hook that is called in projectiles when it hits a target.
 *                The hook is meant to be redefined to enable special effects
 *                like poisoned projectiles.
 *
 * Arguments:     attacker: The attacker shooting this missile.
 *                aid:      The attack id
 *                hdesc:    The hitlocation description.
 *                phurt:    The %hurt made on the enemy
 *                enemy:    The enemy who got hit
 *                dt:       The current damagetype
 *                phit:     The %success that we made with our weapon
 *                dam:      The actual damage caused by this weapon in hit
 *                          points.
 *                hid:      The hit location id.
 */
public varargs void 
projectile_hit_target(object archer, int aid, string hdesc, int phurt, 
		      object enemy, int dt, int phit, int dam, int hid)
{
    return;
}

/*
 * Function name: query_recover
 * Description  : Called to make this projectile is recoverable, and
 *                to make sure the keep status is kept.
 */
string
query_recover()
{
    return MASTER + ":PROJECTILE#" + broken + "#" + num_heap() + "#";
}

/*
 * Function name: init_recover
 * Description  : Initiates the broken status of a projectile.
 *
 */
void
init_recover(string arg)
{
    string head, tail;
    int heap_size;
    
    sscanf(arg, "%sPROJECTILE#%d#%d#%s", head, broken, heap_size, tail);
    set_heap_size(heap_size);
}

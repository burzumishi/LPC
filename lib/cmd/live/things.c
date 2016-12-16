/*
 * /cmd/live/things.c
 *
 * General commands for manipulating things.
 *
 * - appraise
 * - count
 * - drop
 * - examine
 * - exa
 * - get
 * - give
 * - glance
 * - hide
 * - i
 * - inventory
 * - keep
 * - l
 * - look
 * - peek
 * - pick
 * - put
 * - reveal
 * - search
 * - sneak
 * - take
 * - track
 * - unkeep
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/cmd/std/command_driver";

#include <composite.h>
#include <cmdparse.h>
#include <files.h>
#include <filter_funs.h>
#include <formulas.h>
#include <language.h>
#include <macros.h>
#include <money.h>
#include <options.h>
#include <ss_types.h>
#include <std.h>
#include <stdproperties.h>
#include <subloc.h>

#define ENV (environment(this_player()))
#define PREV_LIGHT CAN_SEE_IN_ROOM(this_player()) 

/*
 * Prototypes
 */
int visibly_hold(object ob);
int manip_set_dest(string prep, object *carr);

/*
 * Global variables
 */
static int silent;           /* silent flag if person did 'get/drop all' */
static object gDest;         /* destination use for put and give */
static object *gContainers;  /* array of containers to try */
static object *gFrom;        /* array of objects where player did get things */
static object gHolder;
static string gItem;         /* string to hold pseudoitem from look command */

void
create()
{
    seteuid(getuid(this_object())); 
}

/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
string
get_soul_id()
{
    return "things";
}

/* **************************************************************************
 * This is a command soul.
 */
int
query_cmd_soul()
{
    return 1;
}

/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
mapping
query_cmdlist()
{
    return ([
             "appraise":"appraise",

             "count":"count",

             "drop":"put",

             "examine":"examine",
             "exa":"examine",

             "get":"get",
             "give":"give",
             "glance":"glance",

             "hide":"hide",

             "i":"inventory",
             "inventory":"inventory",

             "keep":"keep",

             "l":"look",
             "look":"look",

             "peek":"peek",
             "pick":"get",
             "put":"put",

             "reveal":"reveal",

             "search":"search",
             "sneak":"sneak",

             "take":"get",
             "track":"track",

             "unkeep":"keep",
           ]);
}

/*
 * Function name: using_soul
 * Description:   Called once by the living object using this soul. Adds
 *                sublocations responsible for extra descriptions of the
 *                living object.
 */
public void 
using_soul(object live)
{
}

/* **************************************************************************
 * Here follows some support functions. 
 * **************************************************************************/

/*
 * We fail to do something because it is dark
 */
varargs int
light_fail(string str)
{
    string s;

    if (!strlen(str))
        str = query_verb() + " things";
    if (!stringp(s = environment(this_player())->query_prop(ROOM_S_DARK_MSG)))
        notify_fail("It is too dark to " + str + ".\n");
    else
        notify_fail(s + " " + str + ".\n");
    return 0;
}

/*
 * Function name: move_err_short
 * Description:   Translate move error code to message and prints it.
 * Arguments:     ierr: move error code
                  ob:   object
 * Outputs:       Message string.
 *
 * Error message examples:
        "The bag is too heavy."
        "The bird can not be dropped."
        "The pearl can not be removed."
        "The ankheg refuses."
        "The chest is full."
        "The river can not be taken."
 *
 * Ref:           see /std/object.c for the move function 
 */
varargs void 
move_err_short(int ierr, object ob, object dest)
{
    mixed str, str2;
    string shortdesc;

    if (silent || ierr <= 0 || ierr >= 11 || !objectp(ob))
        return;

    str = capitalize(LANG_THESHORT(ob));

    switch (ierr)
    {
    case 1:
        if (ob->query_prop(HEAP_I_IS) && ob->num_heap() > 1)
            str += " are too heavy.\n";
        else
            str += " is too heavy.\n";
        break;
    case 2:
        str2 = ob->query_prop(OBJ_M_NO_DROP);
        if (!stringp(str2))
            str += " cannot be dropped.\n";
        else
            str = str2;
        break;
    case 3:
        if (query_verb() == "give")
        {
            str2 = ob->query_prop(OBJ_M_NO_GIVE);
            if (!stringp(str2))
                str += " cannot be given away.\n";
            else
                str = str2;
            break;
        }
        if (dest)
            str2 = environment(ob)->query_prop(CONT_M_NO_REM);
        if (!stringp(str2))
            str += " cannot be removed.\n";
        else
            str = str2;
        break;
    case 4:
        str2 = ob->query_prop(OBJ_M_NO_INS);
        if (!stringp(str2))
            str += " resists.\n";
        else
            str = str2;
        break;
    case 5:
        if (dest)
        {
            str2 = dest->query_prop(CONT_M_NO_INS);
            if (!dest->query_prop(CONT_I_IN) || !stringp(str2))
                str += " cannot be put there.\n";
            else
                str = str2;
        }
        else
            str += " cannot be put there.\n";
        break;
    case 6:
        str2 = ob->query_prop(OBJ_M_NO_GET);
        if (!stringp(str2))
            str += " can not be taken.\n";
        else
            str = str2;
        break;
    case 7:
        str += " resists.\n";
        break;
    case 8:
        str += " is too big.\n";
        break;
    case 9:
        str += " can't be taken from a closed container.\n";
        break;
    case 10:
        str += " can't be put in a closed container.\n";
        break;
    }

    write(str);
}
   
/*
 * Function name: manip_drop_access
 * Description:   Test if player carries an object
 * Arguments:     ob: object
 * Returns:       1: caried object
                  0: otherwise
 *
 */
int
manip_drop_access(object ob)
{
    if (!objectp(ob))
        return 0;

    if (ob->query_no_show())
        return 0;

    if (environment(ob) == this_player())
        return 1;
    return 0;
}

/*
 * Function name: manip_give_access
 * Description  : This function is called to see whether the player can
 *                actually give this object. To be able to give an object
 *                to another player, you must first have it yourself.
 * Arguments    : object ob - the object to give.
 * Returns      : int 1/0 - true if the player has the object in his
 *                          inventory.
 */
int
manip_give_access(object ob)
{
    if (!objectp(ob))
    {
        return 0;
    }

    return (environment(ob) == this_player());
}

/*
 * Function name: manip_relocate_to
 * Description:   Test if an object can be moved to another one and do it.
 * Arguments:     item_o: object to be moved
                  to: object to move to
 * Returns:       1: object moved
                  0: otherwise
 * Outputs:       move error messages
 * Notify_fail:   ""  (no notify fail because of output of error messages)
 *
*/
varargs int
manip_relocate_to(object item_o, object to)
{
    object dest;        /* receiver */
    int ierr;
    string destmsg;
    
    if (!objectp(item_o))
        return 0;
    if (!to)
    {
        dest = environment(this_player());
        destmsg = "";
    }
    else
    {
        dest = to;
        destmsg = (query_verb() == "give" ? " to " : 
            (dest->query_prop(CONT_I_ATTACH) ? " on " : " in ")) +
            QSHORT(dest);
    }
    
    if (item_o == dest)
        return 0;               /* not into itself */
    if (item_o == this_player())
        return 0;               /* not him self */

    if (!(ierr = item_o->move(dest)))
        return 1;

    if (!silent)
        say(QCTNAME(this_player()) + " tries to " + query_verb() +
                " " + (stringp(item_o->short()) ? QSHORT(item_o) : 
        item_o->query_name()) + destmsg + " but fails.\n");

    move_err_short(ierr, item_o, dest);
    notify_fail("");
    return 0;
}

int
manip_put_dest(object item) 
{ 
    return manip_relocate_to(item, gDest); 
}

int
manip_set_dest(string prep, object *carr)
{ 
    string vb;

    vb = query_verb();

    if (!carr || sizeof(carr) == 0)
    {
        notify_fail(capitalize(vb) + " " + prep + " what?\n");
        return 0;
    }

    if (sizeof(carr) > 1)
    {
        notify_fail("Please be specific, where do you want to " + 
                    query_verb() + " them?\n");
        return 0;
    }

    gDest = carr[0];

    if (living(gDest))
    {
        notify_fail(gDest->query_the_name(this_player()) +
                " will not stand for that.\n");
        return 0;
    }

    if ((member_array(prep, ({ "in", "into", "inside" })) >= 0) ||
        (gDest->query_prop(CONT_I_ATTACH) &&
        (member_array(prep, ({ "on", "onto" })) >= 0)))
    {
        notify_fail(capitalize(query_verb()) + " what " + prep + 
            " the " + gDest->short() + "?\n");
        return 1;
    }
    else
    {
        notify_fail("I do not understand what you mean by put " + prep +".\n");
        return 0;
    }
}

int
manip_ch_prep(string ptext)
{
    return parse_command(ptext, ({0}), "'in' / 'inside' / 'from'");
}
   
/*
 * Function name: manip_relocate_from
 * Description:   tries to transfer object to player
 * Arguments:     item_o: object
 * Returns:       1: transferred the object
                  0: some obstruction
 * Outputs:       Messages on failed transfer.
 * Notify_fail    Is set to 0 to hide access failure.
 * Ex:               get item(s), get item(s) from container(s)
 *
 */
int
manip_relocate_from(object item_o)
{
    int ierr;
    object env;
    string tmp;
    
    if (!objectp(item_o))
    {
        return 0;
    }

    if (item_o == this_player())
    {
        return 0;
    }

    env = environment(item_o);
    if (env == this_player())
    {
        return 0;
    }

    if (env->query_prop(CONT_I_HIDDEN) || env->query_prop(CONT_I_CLOSED))
    {
        tmp = " to " + query_verb() + " " + LANG_THESHORT(item_o) + " from ";
        if (living(env))
        {
            tell_object(env, this_player()->query_The_name(env) +
                " tries" + tmp + "you.\n");
        }
        say(QCNAME(this_player()) + " tries" + tmp + "the " + QSHORT(env) +
            ".\n", ({ env, this_player() }) );
        write("You fail" + tmp + LANG_THESHORT(env) + ".\n");

        if (env->query_prop(CONT_I_CLOSED))
	{
            write(capitalize(LANG_THESHORT(env)) + " is closed.\n");
	}

        notify_fail("");
        return 0;
    }

    if ((ierr = item_o->move(this_player())) == 0)
    {
        gFrom = gFrom + ({env});
        return 1;
    }

    if (!silent)
    {
        say(QCNAME(this_player()) + " tries to " + query_verb() +
            " the " + (stringp(item_o->short()) ? QSHORT(item_o) : 
            item_o->query_name()) + " but fails.\n");
    }

    move_err_short(ierr, item_o, this_player());
    notify_fail("");    /* to hide the failed access one  */
    return 0;
}

/*
 * Function name: manip_put_whom
 * Description  : We only allow the object to be moved if this_player()
 *                has it in his inventory. For additional information on
 *                the move process and the return values, see the documents
 *                on manip_relocate_to(item, gDest).
 * Arguments    : object item - the object to move.
 * Returns      : int 1/0 - it will fail if 'item' is not in this_player(),
 *                          else see manip_relocate_to().
 */
int
manip_put_whom(object item) 
{
    if (environment(item) != this_player())
    {
        return 0;
    }

    return manip_relocate_to(item, gDest);
}

int
manip_set_whom(string prep, object *carr)
{ 
    mixed tmp;
    int i;

    if (!carr || sizeof(carr) == 0)
        return 0;

    if (sizeof(carr) > 1)
    {
        notify_fail("Please be specific, to whom do you want to give them?\n");
        return 0;
    }

    gDest = carr[0];
    if (parse_command(prep, ({ 0 }), "'to'"))
    {
        if (tmp = gDest->query_prop(LIVE_M_NO_ACCEPT_GIVE))
        {
            if (stringp(tmp))
                notify_fail(gDest->query_The_name(this_player()) + tmp);
            return 0;
        }

        notify_fail(capitalize(query_verb()) + " what " + prep + " " +
            gDest->query_the_name(this_player()) + "?\n");
        return 1;
    }
    else
    {
        notify_fail("I do not understand what you mean by give " +
                    prep + ".\n");
        return 0;
    }
}

/*
 * Function name: in_gContainers
 * Description:   test if object is in one of a set of containers
 * Arguments:     ob: object
 * Returns:       1: is in the conatiner
                  0: not in the container
 * globals        gContainers: the array of containers
 *
 */
int
in_gContainers(object ob) 
{
    mixed res;

    if (!objectp(ob))
        return 0;
    if (environment(ob) != gContainers[0] &&
                environment(ob) != gContainers[0]->query_room())
        return 0;

    if ((gContainers[0]->query_prop(CONT_I_CLOSED) &&
            !gContainers[0]->query_prop(CONT_I_TRANSP)) ||
            gContainers[0]->query_prop(CONT_I_HIDDEN))
        return 0;

    return 1;
}

/*
 * Here are some functions with the looks command.
 */

void
look_living_exec(mixed plr)
{
    write(plr->long());
}

/*
 * Function name: show_exec
 * Description:   Shows an item depending on its position, normally the long
                  description, but short description for items carried or
                  inside other items.
 * Arguments:     object ob
 *
*/
void 
show_exec(object ob)
{
    object env = environment(ob);
    string str;
    
    /* objects inside transparent objects */
    while ((env != this_player()) && (env != environment(this_player())))
    {
        if (!strlen(str))
        {
            str = "You see " + LANG_ASHORT(ob);
        }
        if (living(env))
        {
            str += " carried by " + env->short(this_player());
        }
        else
        {
            str += (env->query_prop(CONT_I_ATTACH) ? " on" : " inside") +
                " the " + env->short();
        }

        env = environment(env);
    }

    if (strlen(str))
    {
        write(str + ".\n");
    }

    write(ob->long());

}

/*
 * Function name: item_access
 * Description:   test if an object contains (pseudo) item description gItem
 * Arguments:     object ob
 * Returns:       1: found gItem
                  0: failed to find gItem
 * Globals:       string gItem
 *
*/
int 
item_access(object ob)
{
    if (!objectp(ob))
        return 0;
    return (int) ob->item_id(gItem);
}

int
inside_visible(object cobj)
{
    object env;

    if (!objectp(cobj) || cobj->query_no_show())
        return 0;

    /* Properties stop us from seing the inside
     */
    if (!cobj->query_prop(CONT_I_IN) || cobj->query_prop(CONT_I_HIDDEN) ||
                (cobj->query_prop(CONT_I_CLOSED) &&
                !cobj->query_prop(CONT_I_TRANSP) &&
                !cobj->query_prop(CONT_I_ATTACH)))
        return 0;        

    env = environment(cobj);
    if (env == this_player() || env == environment(this_player()) ||
                visibly_hold(cobj))
        return 1;

    while (env && (!env->query_prop(CONT_I_CLOSED) ||
            env->query_prop(CONT_I_TRANSP)) && !env->query_no_show())
    {
        if (visibly_hold(env))
            return 1;
        env = environment(env);
        if (env == this_player() || env == environment(this_player()))
            return 1;
    }
    return 0;
}

varargs int
visible(object ob, object cobj)
{
    object env;

    if (!objectp(ob))
        return 0;

    if (cobj && (env = (object)cobj->query_room()) &&
                (cobj->query_prop(CONT_I_TRANSP) ||
                !cobj->query_prop(CONT_I_CLOSED)))
    {
        return ((env->query_prop(OBJ_I_LIGHT) >
                -(this_player()->query_prop(LIVE_I_SEE_DARK))) && 
                CAN_SEE(this_player(), ob));
    }
        
    env = environment(ob);
    if (env == this_player() || env == environment(this_player()))
        return CAN_SEE(this_player(), ob);

    while (objectp(env) && !living(env) && (env->query_prop(CONT_I_TRANSP) ||
                !env->query_prop(CONT_I_CLOSED)))
    {
        env = environment(env);
        if (env == this_player() || env == environment(this_player()))
            return CAN_SEE(this_player(), ob);
    }
    return 0;
}

varargs int
accessible(object ob)
{
    object env;

    if (!ob)
    {
        return 0;
    }

    env = environment(ob);
    if ((env == this_player()) || (env == environment(this_player())))
    {
        return CAN_SEE(this_player(), ob);
    }

    while (env && !living(env) && !env->query_prop(CONT_I_CLOSED))
    {
        env = environment(env);
        if ((env == this_player()) || (env == environment(this_player())))
	{
            return CAN_SEE(this_player(), ob);
	}
    }

    return 0;
}

/* 
 * Is 
 */
int
visibly_hold(object ob)
{
    object env;
    if (!objectp(ob))
        return 0;
 
    env = environment(ob);
    while (objectp(env))
    {
        if (env == gHolder)
            return 1;

        if (env->query_prop(CONT_I_HIDDEN) ||
            (!env->query_prop(CONT_I_TRANSP) &&
             !env->query_prop(CONT_I_ATTACH) &&
             env->query_prop(CONT_I_CLOSED)))
            return 0;
        else
            env = environment(env);
    }
    return 0;
}

/* 
 * Look ended here.
 */

/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the 
 * same order as in the function name list.
 * **************************************************************************/

/*
 * appraise - Appraise something
 */

/*
 * Function name: appraise_light
 * Description  : This function handles the appraising of the light in the
 *                room by a player.
 * Returns      : int 1/0 - success/failure (always 1)
 */
int
appraise_light()
{
    object *objs;
    object *livings;
    object room = environment(this_player());
    int r_light = room->query_prop(ROOM_I_LIGHT);
    int o_light = room->query_prop(OBJ_I_LIGHT);
    int size;
    int index;

    /* Player cannot see anything. Nothing to appraise, either. */
    if (!CAN_SEE_IN_ROOM(this_player()))
    {
        write("It is dark, very dark. What else is there to observe? " +
            "Nothing, you see nothing.\n");
        return 1;
    }

    /* Tell whether the room is in- or outside, and if it is normally light. */
    if (room->query_prop(ROOM_I_INSIDE))
    {
        write(capitalize(room->short()) + " is indoors and " +
            ((r_light > 0) ?
            "lit, either by a torch, or by natural sources" :
            "naturally dark") + ".\n");
    }
    else
    {
        write(capitalize(room->short()) +
            " is in the open air and naturally " +
            ((r_light > 0) ? "light" : "dark") + ".\n");
    }

    /* Player can see with LIVE_I_SEE_DARK */
    if (o_light <= 0)
    {
        write("Your environment is cloaked in darkness, yet you manage " +
            "to see your surroundings clearly.\n");
    }

    /* Which items/livings emit light. */
    objs = filter((all_inventory(room) - ({ this_player() }) ),
        &operator( > )(, 0) @ &->query_prop(OBJ_I_LIGHT));
    objs = FILTER_CAN_SEE(objs, this_player());
    livings = FILTER_LIVE(objs);
    objs -= livings;
    if (size = sizeof(livings))
    {
        write("One or more items carried by " +
            ((size == 1) ? "" : "each of ") + COMPOSITE_LIVE(livings) +
            " emit light.\n");
    }
    if (size = sizeof(objs))
    {
        write(capitalize(COMPOSITE_DEAD(objs)) +
            ((size == 1) ? " emits" : " emit") + " light.\n");
    }

    /* Which items/livings emit darkness. */
    objs = filter((all_inventory(room) - ({ this_player() }) ),
        &operator( < )(, 0) @ &->query_prop(OBJ_I_LIGHT));
    objs = FILTER_CAN_SEE(objs, this_player());
    livings = FILTER_LIVE(objs);
    objs -= livings;
    if (size = sizeof(livings))
    {
        write("One or more items carried by " +
            ((size == 1) ? "" : "each of ") + COMPOSITE_LIVE(livings) +
            " emit darkness.\n");
    }
    if (size = sizeof(objs))
    {
        write(capitalize(COMPOSITE_DEAD(objs)) +
            ((size == 1) ? " emits" : " emit") + " darkness.\n");
    }

    return 1;
}

int
appraise(string str)
{
    object *obs;

    if (str == "light")
    {
        return appraise_light();
    }

    if (PREV_LIGHT <= 0)
        return light_fail("appraise");
 
    notify_fail("Appraise what?\n", 0);
    if (!stringp(str))
        return 0;

    obs = FIND_STR_IN_OBJECT(str, this_player());
    if (sizeof(obs) == 0)
        obs = FIND_STR_IN_OBJECT(str, environment(this_player()));

    if (sizeof(obs) == 0)
        return 0;

    foreach(object ob: obs)
    {
        write("You study " + (living(ob) ? ob->query_the_name(this_player()) :
            LANG_THESHORT(ob)) + " carefully.\n");
        ob->appraise_object();
    }

    return 1;
}

/*
 * Function name: count
 * Description:   Let the players count stuff other than coins (heaps)
 * Arguments:     str - the string describing what players want to count
 */
int
count(string str)
{
    object *ob;

    if (!stringp(str))
    {
        notify_fail("Count what?\n", 0);
        return 0;
    }

    if (!CAN_SEE_IN_ROOM(this_player()))
    {
        return light_fail();
    }

    ob = FIND_STR_IN_OBJECT(str, this_player());
    if (!sizeof(ob))
        ob = FIND_STR_IN_OBJECT(str, environment(this_player()));

    if (sizeof(ob))
    {
        /* Heaps (coins) have their own routines for countind */
        if (ob[0]->query_prop(HEAP_I_IS))
            return 0;

        if (sizeof(ob) == 1)
        {
            write("You find only a single " + ob[0]->short(this_player()) +
                ".\n");
            return 1;
        }
 
        write("You count " + LANG_WNUM(sizeof(ob)) + " " +
            str + ".\n");
        return 1;
    }

    notify_fail("You don't find any " + str + " here.\n");
    return 0;
}

/*
 * get - get something
 */
/*
 * Function name: get, pick, take
 * Description:   get items from environment or containers
 * Arguments:     str: tail of command string
 * Returns:       1: did get something
                  0: 
 * Globals:       gFrom: sources of items
 *                gContainers: containers 
 * Notify_fail:   Get what?
 *                Get from what?
 *
 * Ex:               get item(s), get item(s) from container(s)
 *
 */
int
get(string str)
{
    object *itema, *cont, linked, *obarr;
    string vb, prep, items;
    int i, from_self;
    
    if (!PREV_LIGHT) return light_fail();

    vb = query_verb();
    
    notify_fail(capitalize(vb) + " what?\n", 0);

    if (!stringp(str))
        return 0;

    gFrom = ({});

    /* This is done to avoid all those stupid messages 
       when you try 'get all' 
    */
    silent = (str == "all");

    if (parse_command(str, all_inventory(environment(this_player())), "%i", itema))
    {
        itema = NORMAL_ACCESS(itema, 0, 0);
        itema = filter(itema, manip_relocate_from);
        if (sizeof(itema) == 0)
        {
            if (silent)
                notify_fail("Nothing taken.\n");
            return 0;
        } 
        if (sizeof(itema) > 0)
        {
            itema->remove_prop(OBJ_I_HIDE);
            write("You " + vb + " " + COMPOSITE_DEAD(itema) + ".\n");
            say(QCNAME(this_player()) + " " + vb + "s " + QCOMPDEAD + ".\n");

            itema->force_heap_merge();

            return 1;
        }
    }

    if (silent)
    {
        notify_fail("Nothing taken.\n");
        return 0;
    }

    gFrom = ({});

    if (parse_command(str, all_inventory(environment(this_player())) + all_inventory(this_player()),
                      "%s 'in' / 'inside' / 'from' %i", items, cont))
    {
        gContainers = NORMAL_ACCESS(cont, "accessible", this_object());
        gContainers = FILTER_DEAD(gContainers);
        if (sizeof(gContainers) == 0)
        {
            notify_fail(capitalize(vb) + " from what?\n");
            return 0;
        }

        if (linked = gContainers[0]->query_room())
            obarr = all_inventory(linked);
        else
            obarr = deep_inventory(gContainers[0]);

        if (!parse_command(items, obarr, "%i", itema))
            return 0;

        itema = NORMAL_ACCESS(itema, "in_gContainers", this_object());
        if (sizeof(itema) == 0) 
            return 0;

        itema = filter(itema, manip_relocate_from);
        if (sizeof(itema) == 0)
            return 0;

        itema->remove_prop(OBJ_I_HIDE);

        from_self = 
            (member_array(this_player(), all_environment(gContainers[0])) >= 0);

        write("You " + vb + " " + COMPOSITE_DEAD(itema) + " from " +
            (from_self ? "your " : "the ") + gContainers[0]->short() + ".\n");
        say(QCTNAME(this_player()) + " " + vb + "s " + QCOMPDEAD +
            " from " + (from_self ? this_player()->query_possessive() : "the") +
            " " + QSHORT(gContainers[0]) + ".\n");

        itema->force_heap_merge();

        return 1;
    }

    if (environment(this_player())->item_id(str))
    {
        notify_fail("The " + str + " cannot be taken.\n");
        return 0;
    }

    return 0;
}

/* 
 * give - Give something to someone
 */
/*
 * Function name: give
 * Description:   tries to move object to another living
 * Arguments:     str: tail of command string
 * Returns:       1: manage to give something
 *                0: failed
 * Ex:            give item(s) to player
 */
int
give(string str)
{
    object *a;

    if (!PREV_LIGHT)
        return light_fail();

    notify_fail(capitalize(query_verb()) + " what to whom?\n");

    if (!strlen(str))
    {
        return 0;
    }

    silent = 0;

    a = (object *)CMDPARSE_STD->do_verb_with(str, "manip_set_whom",
        "manip_put_whom", "manip_give_access", 0, this_object());

    if (sizeof(a) > 0)
    {
	/* Reveal only when giving something to someone who is hidden. */
	if (!gDest->query_prop(OBJ_I_HIDE) && !gDest->query_prop(OBJ_I_INVIS))
	{
	    this_player()->reveal_me(1);
        }

	if (living(gDest))
        {
            write("You give " + COMPOSITE_DEAD(a) + " to " +
                gDest->query_the_name(this_player()) + ".\n");
            gDest->catch_msg(QCTNAME(this_player()) +
                " gives you " + QCOMPDEAD + ".\n");
            say(QCTNAME(this_player()) + " gives " + QCOMPDEAD + " to " +
                QNAME(gDest) + ".\n", ({gDest, this_player()}) );
        }
        else
        {
            write("You give " + COMPOSITE_DEAD(a) + " to ");
            say(QCTNAME(this_player()) + " gives " + QCOMPDEAD + " to ");
            write(COMPOSITE_DEAD(gDest) + ".\n");
            say(QCOMPDEAD + ".\n");
        }

        return 1;
    }

    return 0;
}

/*
 * hide - Hide something.
 */
int
hide(string str)
{
    object *itema, *cont;
    string prep, msg, vb, *args;
    mixed err;
    int hiding, i, val, bval;
    int poorly = 0;

    vb = query_verb(); 
    notify_fail(capitalize(vb) + " what?\n", 0);

    if (strlen(str))
    {
        args = explode(str, " ");
        if (poorly = (args[sizeof(args) - 1] == "poorly"))
        {
            str = implode(args[..-2], " ");
        }
    }
    silent = (str == "all");

    if ((this_player()->query_prop(OBJ_I_LIGHT) > 0) &&  
        (this_player()->query_prop(OBJ_I_LIGHT) > 
        environment(this_player())->query_prop(OBJ_I_LIGHT)))
    {
        notify_fail("You cannot hide here, shining like that!\n");
        return 0;
    }

    hiding = environment(this_player())->query_prop(ROOM_I_HIDE);
    bval = this_player()->query_skill(SS_HIDE);
    if (hiding < 0 || hiding > bval)
    {
        notify_fail("It's far too hard to hide anything here.\n");
        return 0;
    }

    bval = (bval - hiding) / 2;
    val = bval + random(bval);
    if (poorly)
    {
        bval = this_player()->query_skill(SS_AWARENESS) / 2;
        val = (val > (bval * 2) ? bval + random(bval) : val);
    }

    if (!strlen(str) ||
        (str == "myself"))
    {
        cont = all_inventory(environment(this_player()));
        itema = FILTER_LIVE(cont);
        itema = FILTER_CAN_SEE(itema, this_player());
        if (sizeof(itema) > 1)
        {
            notify_fail("You can't hide yourself while others are watching!\n");
            return 0;
        }

        if (this_player()->query_attack())
        {
            notify_fail("You can't hide yourself while in combat!\n");
            return 0;
        }

        if (this_player()->query_prop(OBJ_I_HIDE))
        {
            notify_fail("You can't hide any better than this!\n");
            return 0;
        }
        else
        {
            say(QCTNAME(this_player()) +
                " suddenly disappears out of sight.\n");
            write("You hide yourself as best you can.\n");
            this_player()->add_prop(OBJ_I_HIDE, val);
        }
        return 1;
    }

    /* Try to hide something from your inventory. */
    if (parse_command(str, all_inventory(this_player()), "%i", itema))
    {
        if (!sizeof(itema = CMDPARSE_STD->normal_access(itema,
            "manip_drop_access", this_object(), 1)))
        {
            notify_fail("Nothing hidden.\n");
            return 0;
        }

        itema = filter(itema, manip_relocate_to);
    }
    /* Try to hide something from your environment if not from your inventory. */
    else if (parse_command(str, all_inventory(environment(this_player())), "%i", itema))
    {
        if (!sizeof(itema = NORMAL_ACCESS(itema, 0, 0)))
        {
            notify_fail("Nothing hidden.\n");
            return 0;
        }

        msg = "";
        foreach(object obj: itema)
        {
            if (living(obj))
            {
                msg += "You have to catch " +
                    obj->query_the_name(this_player()) + " before you can hide " +
                    obj->query_objective() + ".\n";
                itema -= ({ obj });
                continue;
            }
            if (err = obj->query_prop(OBJ_M_NO_GET))
            {
                if (intp(err) && (err > 0))
                    err = "You cannot pick up " + LANG_THESHORT(obj)+ ".\n";
                if (strlen(err))
                {
                    msg += err;
                    itema -= ({ obj });
                    continue;
                }
                
            }
            if ((this_player()->query_prop(OBJ_I_WEIGHT) + obj->query_prop(OBJ_I_WEIGHT) > this_player()->query_prop(CONT_I_MAX_WEIGHT)) ||
                (this_player()->query_prop(OBJ_I_VOLUME) + obj->query_prop(OBJ_I_VOLUME) > this_player()->query_prop(CONT_I_MAX_VOLUME)))
            {
                msg += "You are too burdoned to hide " + LANG_THESHORT(obj)+ ".\n";
                itema -= ({ obj });
                continue;
            }
        }

        if (!sizeof(itema))
        {
            notify_fail(msg);
            return 0;
        }
        /* Give error message, but continue hiding what can be hidden. */
        write(msg);
    }
    else
	itema = ({ });

    if (sizeof(itema))
    {
        write("You " + vb + " " + COMPOSITE_DEAD(itema) + ".\n");
        say(QCTNAME(this_player()) + " " + vb + "s something.\n");
        itema->add_prop(OBJ_I_HIDE, val);
        itema->force_heap_merge();
        return 1;
    }

    if (!PREV_LIGHT)
        return light_fail("see");

    if (silent)
    {
        notify_fail("Nothing hidden.\n");
        return 0;
    }
    
    if (!parse_command(str, all_inventory(environment(this_player())) + all_inventory(this_player()), "%i %w %i",
        itema, prep, cont))
    {
        return 0;
    }
    cont = NORMAL_ACCESS(cont, 0, 0);
    if (!manip_set_dest(prep, cont))
    {
        return 0;
    }

    itema = NORMAL_ACCESS(itema, "manip_drop_access", this_object());
    if (sizeof(itema) == 0)
    {
        notify_fail(capitalize(vb) + " what?\n", 0);
        return 0;
    }

    itema = filter(itema, manip_put_dest);
    if (sizeof(itema) > 0)
    {
        prep = (gDest->query_prop(CONT_I_ATTACH) ? " on" : " in");

        write("You " + vb + " " + COMPOSITE_DEAD(itema) + prep + " the " +
            gDest->short() + ".\n");
        say(QCTNAME(this_player()) + " " + vb + "s something" + prep +
            " the " + QSHORT(gDest) + ".\n");
        itema->add_prop(OBJ_I_HIDE, (this_player()->query_skill(SS_HIDE) / 2) +
            random(this_player()->query_skill(SS_HIDE)));
        itema->force_heap_merge();
        return 1;
    }
    
    return 0;
}

/*
 * inventory - List things in my inventory
 */

/*
 * Function name: display_category
 * Description  : Displays one group of items within the inventory of a player.
 * Arguments    : object *oblist - the items in the display group, if any.
 *                string category - the title of the category (max 8 letters).
 * Returns      : object * - the oblist passed through the header.
 */
static object *
display_category(object *oblist, string category)
{
    if (sizeof(oblist))
    {
        write(HANGING_INDENT(sprintf("%-8s: %s.", extract(category, 0, 7),
            COMPOSITE_DEAD(oblist)), 10, 0));
    }

    return oblist;
}

int
inventory(string str)
{
    object player;
    object *objs;
    object *selection;
    int alarm_id;
    int display, size, table;
    string comp;

    if (stringp(str))
    {
        if (!(this_player()->query_wiz_level()))
        {
            notify_fail("What inventory?\n");
            return 0;
        }

        str = lower_case(str);
        if (!objectp(player = find_player(str)) &&
            !objectp(player = find_living(str)))
        {
            notify_fail("No such player or living '" + str + "'.\n");
            return 0;
        }
    }
    else
    {
        player = this_player();
    }

    if (PREV_LIGHT <= 0)
    {
        return light_fail("see");
    }

    str = ((player == this_player()) ? "You are " :
        player->query_The_name(this_player()) + " is ");

    objs = (object*)player->subinventory(0);
    objs = FILTER_SHOWN(objs);

    table = this_player()->query_option(OPT_TABLE_INVENTORY);

    /* Filter all coins and gems. */
    size = sizeof(objs);
    selection = filter(objs, &->id("coin")) | filter(objs, &->id("gem"));

    if (!table && sizeof(selection))
    {
        write(HANGING_INDENT(str + "in possession of " +
            COMPOSITE_DEAD(selection) + ".", 2, 0));
        objs -= selection;
    }

    /* Show all sublocs that describe things. */
    player->add_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS, 1);
    alarm_id = set_alarm(0.5, 0.0,
        &player->remove_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS));
    write(player->show_sublocs(this_player()));
    player->remove_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS);
    remove_alarm(alarm_id);

    if (table)
    {
        objs -= display_category(selection, "Value");
//      objs -= display_category(FILTER_ARMOUR_OBJECTS(objs), "Armours");
        objs -= display_category(FILTER_WEARABLE_OBJECTS(objs), "Clothes");
        objs -= display_category(FILTER_WEAPON_OBJECTS(objs), "Weapons");
        objs -= display_category(FILTER_HERB_OBJECTS(objs), "Herbs");
        objs -= display_category(FILTER_POTION_OBJECTS(objs), "Potions");
        objs -= display_category(FILTER_FOOD_OBJECTS(objs), "Food");
        objs -= display_category(FILTER_DRINK_OBJECTS(objs), "Drinks");
        objs -= display_category(FILTER_KEY_OBJECTS(objs), "Keys");
        objs -= display_category(FILTER_TORCH_OBJECTS(objs), "Torches");
    }
    else
    {
        write(HANGING_INDENT(str + "carrying " + COMPOSITE_DEAD(objs) +
            ".", 2, 0));
    }

    /* Nothing else (visible). */
    display = (sizeof(objs) != size);
    if (!sizeof(objs) || !(comp = SILENT_COMPOSITE_DEAD(objs)))
    {
        if (!display)
        {
            write(str + "not carrying anything.\n");
        }
    }
    else if (table)
    {
        write(HANGING_INDENT("Other   : " + comp, 10, 0));
    }
    return 1;
}

/*
 * keep - set the OBJ_M_NO_SELL property in an object.
 * unkeep - remove the OBJ_M_NO_SELL property from an object.
 */
int
keep(string str)
{
    object *objs;
    object *keep_objs;
    int    keep = (query_verb() == "keep");
    int    list;

    if (!stringp(str))
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
        return 0;
    }

    /* Player wants to list; remove the flag. */
    if (list = wildmatch("-l*", str))
    {
        str = extract(str, 3);
    }

    /* Playes wants to list, but didn't give any argument. Get all items
     * in his/her inventory. */
    if (list &&
        !strlen(str))
    {
        if (!sizeof(objs = FILTER_CAN_SEE(all_inventory(this_player()),
            this_player())))
        {
            notify_fail("You carry no items.\n");
            return 0;
        }
    }
    /* Or parse the argument to see which items to process. */
    else if (!parse_command(str, this_player(), "[the] %i", objs) ||
        !sizeof(objs = NORMAL_ACCESS(objs, 0, 0)))
    {
        notify_fail(capitalize(query_verb()) + (list ? " -l" : "") +
            " what?\n");
        return 0;
    }

    /* Filter all non-keepable objects. */
    keep_objs = filter(objs, &not() @ &->query_keepable());
    objs->restore_heap();

    /* List the 'keep' status of the selected items. */
    if (list)
    {
        if (sizeof(keep_objs))
        {
            write("Not keepable --------------\n" +
                break_string(COMPOSITE_DEAD(keep_objs), 70, 5) + "\n");
            objs -= keep_objs;
        }

        /* Filter all unsellables. */
        keep_objs = filter(objs, &->query_unsellable());
        if (sizeof(keep_objs))
        {
            write("Unsellable ----------------\n" +
                break_string(COMPOSITE_DEAD(keep_objs), 70, 5) + "\n");
            objs -= keep_objs;
        }

        /* Filter all kept objects. */
        keep_objs = filter(objs, &->query_keep());
        if (sizeof(keep_objs))
        {
            write("Keep protected ------------\n" +
                break_string(COMPOSITE_DEAD(keep_objs), 70, 5) + "\n");
            objs -= keep_objs;
        }

        /* The remainder is keepable, but not kept. */
        if (sizeof(objs))
        {
            write("Not keep protected --------\n" +
                break_string(COMPOSITE_DEAD(objs), 70, 5) + "\n");
        }
        
        return 1;
    }

    /* None of the objects are keepable. */
    if (sizeof(keep_objs) == sizeof(objs))
    {
        notify_fail("Not keepable: " + COMPOSITE_DEAD(keep_objs) + ".\n");
        return 0;
    }

    objs -= keep_objs;
    /* Remove any unsellable items from the list. */
    keep_objs = filter(objs, &->query_unsellable());
    if (sizeof(keep_objs))
    {
        write("Unsellable: " + COMPOSITE_DEAD(keep_objs) + ".\n");
        objs -= keep_objs;
        if (!sizeof(objs))
        {
            return 1;
        }
    }
    
    /* Now select the objects to (un)keep. */
    if (keep)
    {
        keep_objs = filter(objs, &not() @ &->query_keep());
    }
    else
    {
        keep_objs = filter(objs, &->query_keep());
    }

    /* No objects to process. */
    if (!sizeof(keep_objs))
    {
        write((keep ? "Already kept: " : "Not kept: ") +
            COMPOSITE_DEAD(objs) + ".\n");
        return 1;
    }

    keep_objs->set_keep(keep);

    /* See if we failed to (un)keep any items. */
    if (keep)
    {
        objs = filter(keep_objs, &not() @ &->query_keep());
    }
    else
    {
        objs = filter(keep_objs, &->query_keep());
    }
    if (sizeof(objs))
    {
        write("Failed to " + (keep ? "" : "un") + "keep: " +
            COMPOSITE_DEAD(keep_objs) + ".\n");
        keep_objs -= objs;
	if (!sizeof(keep_objs))
	{
	    return 1;
	}
    }

    write((keep ? "Set keep protection on " :
        "Removed keep protection from ") + COMPOSITE_DEAD(keep_objs) + ".\n");
    return 1;
}

/*
 * Function name: glance_desc
 * Description:   Return string giving a brief description of an object
 * Arguments:     object ob - the object to describe
 * Returns:       a brief description
 */
string
glance_desc(object ob)
{
    string desc;

    if (living(ob))
    {
        ob->add_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS, 1);
        desc = ob->long(this_player());
        ob->remove_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS);
        desc += ((ob == this_player()) ? "You are " :
            (capitalize(ob->query_pronoun()) + " is ")) +
            CMD_LIVE_STATE->show_subloc_health(ob, this_player()) + ".\n";
        return desc;
    }

    return capitalize(LANG_ASHORT(ob)) + ".\n";
}

/*
 * look - Look at something
 */
/*
 * Function name: look
 * Description:   glances around, examines objects, or looks at pseudo items
 * Arguments:     str: tail of look command
 *                int: if true, brief descriptions will be given
 * Returns:       1: found something to look at
 *                0: failed to find object
 * Notify_fail:   several
 * 
 * Globals:       gItem:  
 *                gHolder:
 * Ex:            look, look at orcs, look inside chest
 *
   Documentation of look
   
   The look at (in, inside) command has the following features.

   look 
        Shows long description of the room and short description of
        all visible items in the environment (see do_glance() ).

   look at 'multiple objects'
        Show the long description of several objects, carried or in the
        environment and the short description of object inside transparent
        objects.
        
   look at 'multiple living'
        Show the long description of several living in the environment.

   look at 'single living'
        Show the long description of one living in the environment and
        the objects carried by this being.

   look in 'containers'
        Show the long description of the containers and short descriptions 
        their contents.

   look at 'single object'
        Shows the long description of the object and if the object is open
        and a container, shows short descriptions of it contents.
   
   look at 'object' carried by 'living'
        Shows the short description of the object and if the object is
        an open or transparent container shows short descriptions of
        it contents.
   
 */
varargs int 
look(string str, int brief)
{
    string          prp,
                    prplc,
                    *tmp,
                    name;
    object          *obarr,
                    *obd,
                    *obl,
                    enemy;
    mixed           long_desc;

    if (!strlen(str))
    {
        return this_player()->do_glance(brief);
    }
    
    if (PREV_LIGHT <= 0)
    {
        return light_fail("see");
    }

    str = lower_case(str);
    tmp = explode(str, " ");
    if (sizeof(tmp) > 1 && tmp[1][0] == '0')
    {
        return 0;
    }
        
    /* look at object carried by something non living */
    /* We shouldn't be able to look at objects inside players presently
    if (parse_command(str, environment(this_player()), 
                      "'at' / 'in' / 'inside' %i 'on' / 'carried' [by] %i", 
                      obarr, obl))
    {
        obl = NORMAL_ACCESS(obl, 0, 0);
        if (pointerp(obl))
        {
            if (sizeof(obl) != 1)
            {
                notify_fail("Be specific, who do you want to look at.\n");
                return 0;
            }
            gHolder = obl[0];
            obarr = NORMAL_ACCESS(obarr, "visibly_hold", this_object());
            if (sizeof(obarr) == 1 && inside_visible(obarr[0]))
            {
                show_exec(obarr[0]);
                obarr[0]->show_visible_contents(this_player());
            } else {
                map(obarr, show_exec);
            }
            return 1;
        }
    }
    */

    /* test for preposition */
    if (sscanf(str, "%s %s", prp, name) < 2)
    {
        str = capitalize((query_verb() == "l") ? "look" : query_verb());
        notify_fail(str + " needs a preposition with an object.\n");
        return 0;
    }

    prplc = lower_case(prp);
    if (prplc != "at" && prplc != "in" && prplc != "inside" &&
        prplc != "prp_examine")
    {
        notify_fail("Look at or in something.\n");
        return 0;
    }

    /* through, under, behind, .. ? */

    gItem = lower_case(name);

    if (!parse_command(str, ENV, "%w %i", prp, obarr) ||
        !sizeof(obarr = NORMAL_ACCESS(obarr, "visible", this_object())))
    {
        /* No objects found */
        /* Test for pseudo item in the environment */
        if (CAN_SEE(this_player(), ENV) && 
            stringp(long_desc = environment(this_player())->long(gItem)))
        {
            write(long_desc);
            return 1;
        }
        else
        {
            obarr = deep_inventory(environment(this_player()));
            obarr = filter(obarr, visible);
            obarr = filter(obarr, item_access); 
            if (sizeof(obarr) > 0) 
            {
                map(obarr, write @ &->long(gItem));
                return 1;
            } 
            else
            {
                if ((name == "me") || (name == "myself"))
                {
		    write(brief ? glance_desc(this_player()) :
                        this_player()->long(this_player()));
                    return 1;
                }

                if (name == "enemy" && (enemy = this_player()->query_attack()))
                {
                    write(brief ? glance_desc(enemy) :
                        enemy->long(this_player()));
                    return 1;
                }

                notify_fail("You find no " + name + ".\n");
                return 0;
            }
        }
    }

    if (sizeof(obarr) == 0)
    {
        notify_fail("You find no " + name + ".\n");
        return 0;
    }

    /* Tries to see the inside of one or sevral containers (non living) */
    if (prplc == "in" || prplc == "inside")
    {
        /* test for not locked */
        obarr = filter(obarr, inside_visible);
        if (sizeof(obarr) > 0)
        {
            foreach(object ob: obarr)
            {
                if (!brief)
		{
                    show_exec(ob);
		}
                ob->show_visible_contents(this_player());
            }
            return 1;
        }
        else if (prplc != "prp_examine")
        {
            notify_fail("You can't see the inside of " + capitalize(name) +
                ".\n");
            return 0;
        }
    }

    if (prplc == "at" || prplc == "prp_examine")
    {
        obd = FILTER_DEAD(obarr);
        obl = FILTER_LIVE(obarr);
        if (sizeof(obd) == 0 && sizeof(obl) == 0)
        {
            notify_fail("You find no " + name + ".\n");
            return 0;
        }

        if (brief)
	{
            write(implode(map(obarr, glance_desc), ""));
	}
        else
	{
            /* if single container we show the contents */
            if (sizeof(obd) == 1 && inside_visible(obd[0]) && !brief)
            {
        	show_exec(obd[0]);
        	obd[0]->show_visible_contents(this_player());
            }
            else
	    {
        	map(obd, show_exec);
	    }
    
            if (sizeof(obl) == 1)
	    {
        	/* if a single living being we show carried items */
        	look_living_exec(obl[0]);
	    }
            else
	    {
        	map(obl, show_exec);
	    }
	}

        /* if we are looking at many dead objects perhaps such an object 
         * exists in the room too? If gItem has another singular form then
         * we suspect player gave a plural word to look for. Should work
         * in most cases. */
        if (LANG_SWORD(gItem) != gItem &&
            stringp(long_desc = environment(this_player())->long(gItem)))
        {
            write(long_desc);
        }

        return 1;
    }
}


/*
 * examine - Examine something
 */
/*
 * Function name: examine
 * Description:   pseudonym for  look at, look in, etc
 * Arguments:     string str: tail of examine command or exa command
 * Returns:       1: found something to look at
                  0: failed to found object
 * Ex:            examine("knife")
 *
*/
int
examine(string str)
{
    if (!stringp(str))
    {
        notify_fail("Examine what?\n", 0);
        return 0;
    }

    return look("prp_examine " + str);
}

/*
 * glance - take a quick look at something
 */
int
glance(string str)
{
   return look(str, 1);
}
 
int 
peek_access(object ob)
{
    if (!living(ob) || ob->query_ghost() || ob == this_player())
        return 0;
    else
        return 1;
}

/*
 * peek - Peek into someone's inventory, part of someone's inventory.
 */
int
peek(string str)
{
    string vb;
    object *p, *inv;
    int id, i, pp_skill;

    if (!CAN_SEE_IN_ROOM(this_player()))
    {
        return light_fail("see");
    }

    vb = query_verb();
    notify_fail(capitalize(vb) + " at whom?\n");

    if (!stringp(str))
        return 0;

    p = CMDPARSE_ONE_ITEM(str, "peek_access", "peek_access");

    if (!sizeof(p))
    {
        return 0;
    }
    if (sizeof(p) > 1)
    {
        notify_fail(capitalize(vb) + " at one person at a time.\n");
        return 0;
    }

    MONEY_EXPAND(p[0]);

    pp_skill = this_player()->query_skill(SS_PICK_POCKET) / 2;
    if ((pp_skill + random(pp_skill) > p[0]->query_skill(SS_AWARENESS)) &&
        (!p[0]->query_wiz_level()))
    {
        inv = all_inventory(p[0]);

        p[0]->add_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS, 1);
        id = set_alarm(0.1, 0.0, &(p[0])->remove_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS));
        write(p[0]->show_sublocs(this_player()));
        p[0]->remove_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS);
        remove_alarm(id);

        inv = (object*)p[0]->subinventory(0);
        inv = FILTER_SHOWN(inv);
        if (sizeof(inv))
            write(p[0]->query_The_name(this_player()) +
                " is currently in possession of: " +
                COMPOSITE_DEAD(inv) + ".\n");
        else
            write(p[0]->query_The_name(this_player()) +
                " does not own anything.\n");
    }
    else
    {
        tell_object(p[0], "You catch " +
            this_player()->query_the_name(p[0]) +
            " rifling through your private belongings!\n");
        write("Oops! " + p[0]->query_The_name(this_player()) +
            " seems to have caught on to you!\n");
    }

    return 1;
}

/*
 * put - Put something
 */
/*
 * Function name: put, drop
 * Description:   put items in environment or cointainers
 * Arguments:     str: tail of command string
 * Returns:       1: did get something
                  0: failedd to get anything
 * Notify_fail:   * "Put what?"
                  * 
 * Ex:               drop item(s), put item(s) in container
 *
 */
int
put(string str)
{
    object *itema, *cont;
    string prep, vb;
    int to_self;
    
    vb = query_verb(); 
    notify_fail(capitalize(vb) + " what?\n", 0);
    
    if (!stringp(str))
    {
        return 0;
    }

    /* This is done to avoid all those stupid messages 
       when you try 'drop all' 
     */
    silent = 0;
    if (str == "all")
    {
        silent = 1;
    }
    
    if (parse_command(str, all_inventory(this_player()), "%i", itema)) 
    {
        itema = CMDPARSE_STD->normal_access(itema, "manip_drop_access",
                this_object(), 1);
        if (sizeof(itema) == 0) {
            if (silent)
                notify_fail("Nothing moved.\n");
            return 0;
        }
        itema = filter(itema, manip_relocate_to);

        if (sizeof(itema) > 0)
        {
            write("You " + vb + " " + COMPOSITE_DEAD(itema) + ".\n");
            say(QCTNAME(this_player()) + " " + vb + "s " + QCOMPDEAD + ".\n");
            return 1;
        }
    }

    if (!PREV_LIGHT)
    {
        return light_fail("see");
    }

    if (silent)
    {
        notify_fail("Nothing dropped.\n");
        return 0;
    }
    
    if (!parse_command(str, all_inventory(environment(this_player())) + all_inventory(this_player()),
		       "%i %w %i", itema, prep, cont)) 
    {
        return 0;
    }

    cont = NORMAL_ACCESS(cont, "accessible", this_object());
    if (!manip_set_dest(prep, cont))
    {
        return 0;
    }

    itema = NORMAL_ACCESS(itema, "manip_drop_access", this_object());
    if (sizeof(itema) == 0)
    {
        notify_fail(capitalize(vb) + " what?\n", 0);
        return 0;
    }

    itema = filter(itema, manip_put_dest);
    if (sizeof(itema) > 0)
    {
        prep = (gDest->query_prop(CONT_I_ATTACH) ? " onto" : " into");

	to_self = (member_array(this_player(), all_environment(gDest)) >= 0);

        write("You " + vb + " " + COMPOSITE_DEAD(itema) + prep +
            (to_self ? " your " : " the ") + gDest->short() + ".\n");
        say(QCTNAME(this_player()) + " " + vb + "s " + QCOMPDEAD + prep +
            (to_self ? " " + this_player()->query_possessive() + " " : " the ")+
            QSHORT(gDest) + ".\n");
        return 1;
    }

    return 0;
}

/*
 * reveal - Reveal something hidden
 */
int
reveal(string str)
{
    object *itema, *cont, linked, *obarr;
    string vb, prep, items;
    int i;
    
    gFrom = ({});
    if (!PREV_LIGHT) return light_fail();

    vb = query_verb();
    if (!stringp(str))
    {
        notify_fail(capitalize(vb) + " what?\n", 0);
        return 0;
    }

    if (str == "myself" || str == "me")
    {
        if (!this_player()->reveal_me(1))
        {
            notify_fail("You are already in plain sight.\n");
            return 0;
        }

        return 1;
    }

    if (parse_command(str, environment(this_player()), "%i", itema))
    {
        itema = NORMAL_ACCESS(itema, 0, 0);
        itema = filter(itema, &->query_prop(OBJ_I_HIDE));
        if (!sizeof(itema))
        {
            notify_fail("Nothing revealed.\n");
            return 0;
        }
 
        itema->remove_prop(OBJ_I_HIDE);
        i = sizeof(itema);
        while(--i >= 0)
        {
            if (living(itema[i]))
            {
                write("You " + vb + " " +
                    itema[i]->query_the_name(this_player()) +".\n");
                tell_object(itema[i], this_player()->query_The_name(itema[i]) +
                    " reveals you!\n");
                say(QCNAME(this_player()) + " " + vb + "s " + QTNAME(itema[i]) +
                    ".\n", ({ itema[i], this_player() }) );
            }
            else
            {
                write("You " + vb + " " + LANG_THESHORT(itema[i]) + ".\n");
                say(QCNAME(this_player()) + " " + vb + "s " +
                    LANG_ASHORT(itema[i]) + ".\n");
            }
        }

        itema->force_heap_merge();
        return 1;
    }

    if (str == "all")
    {
        notify_fail("Nothing revealed.\n");
        return 0;
    }

    if (parse_command(str, all_inventory(environment(this_player())),
                      "%s 'in' / 'inside' %i", items, cont))
    {
        gContainers = NORMAL_ACCESS(cont, 0, 0);
        gContainers = filter(gContainers, &->query_prop(OBJ_I_HIDE));
        gContainers = FILTER_DEAD(gContainers);
        if (sizeof(gContainers) == 0)
        {
            notify_fail(capitalize(vb) + " from what?\n");
            return 0;
        }

        if (linked = gContainers[0]->query_room())
            obarr = all_inventory(linked);
        else
            obarr = deep_inventory(gContainers[0]);

        if (!parse_command(items, obarr, "%i", itema))
            return 0;

        itema = NORMAL_ACCESS(itema, "in_gContainers", this_object());
        if (sizeof(itema) == 0) 
            return 0;

        itema->remove_prop(OBJ_I_HIDE);

        /* Here we assume you do not reveal livings in containers. */
        write("You " + vb + " " + COMPOSITE_DEAD(itema) + " from the " +
            gContainers[0]->short() + ".\n");
        say(QCTNAME(this_player()) + " " + vb + "s " + QCOMPDEAD +
            " in the " + QSHORT(gContainers[0]) + ".\n");
        return 1;
    }

    if (environment(this_player())->item_id(str))
    {
        notify_fail("The " + str + " cannot be revealed.\n");
        return 0;
    }

    return 0;
}

/*
 * search - Search something
 */
int
search(string str)
{
    object *objs, obj;
    int time;
    string item;

    if (!stringp(str))
        str = "here";

    if (this_player()->query_attack())
    {
        write("But you are in the middle of a fight!\n");
        return 1;
    }
 
    if (!CAN_SEE_IN_ROOM(this_player()))
    {
        notify_fail("It is too dark to see anything in here.\n");
        return 0;
    }

    /* Find out where we are searching. */
    if (sscanf(str, "%s for %s", item, str) != 2)
    {
        item = str;
    }

    /* Locate the item. */
    if (item == "here")
    {
        objs = ({ environment(this_player()) });
    }
    else
    {
        /* Could be an item in his inventory or environment. */
        objs = FIND_STR_IN_OBJECT(item, this_player());
        if (!sizeof(objs))
        {
            objs = FIND_STR_IN_OBJECT(item, environment(this_player()));
        }
        /* Otherwise look for add-items in the environment or inventory. */
        if (!sizeof(objs))
        {
            if (environment(this_player())->item_id(item))
            {
                objs = ({ environment(this_player()) });
            }
            else
            {
                objs = filter(all_inventory(environment(this_player())), &->item_id(item));
            }
        }
    }

    if (!sizeof(objs))
    {
        notify_fail("You don't find any " + item + " to search.\n");
        return 0;
    }

    obj = objs[0];
    if (obj == environment(this_player()))
    {
        write("You start to search" +
            (item == "here" ? "" : " " + item) + ".\n");
        say(QCTNAME(this_player()) + " starts to search" +
            (item == "here" ? "" : " " + item) + ".\n");
    }
    else if (living(obj))
    {
        write("You start to search " +
            obj->query_the_name(this_player()) + ".\n");
        tell_object(obj, this_player()->query_The_name(obj) +
            " starts to search YOU!\n");
        say(QCTNAME(this_player()) + " starts to search " + QTNAME(obj) + 
            ".\n", ({ obj, this_player() }));
    }
    else
    {
        write("You start to search " + COMPOSITE_ALL_DEAD(obj) + ".\n");
        say(QCTNAME(this_player()) + " starts to search " + QSHORT(obj) +
            ".\n");
    }

    /* As temporary measure we support "search x for herbs" as argument. */
    if (item != str)
    {
        str = item + " for " + str;
    }
    obj->search_object(str);
    return 1;
}

/*
 * sneak - sneak somewhere.
 */
int
sneak(string str)
{
    int hiding, *dirs, i, val, bval;
    string str2;

    if (!stringp(str))
    {
        notify_fail("Sneak where?\n");
        return 0;
    }

    str2 = SECURITY->modify_command(str, environment(this_player()));
    if (strlen(str2))
        str = str2;
        
    dirs = environment(this_player())->query_exit_cmds();
    if (member_array(str, dirs) < 0)
    {
        notify_fail("Sneak where?\n");
        return 0;
    }

    if (this_player()->query_prop(OBJ_I_LIGHT) &&  
        (this_player()->query_prop(OBJ_I_LIGHT) > 
        environment(this_player())->query_prop(OBJ_I_LIGHT)))
    {
        notify_fail("You cannot sneak shining like that!\n");
        return 0;
    }

    if (objectp(this_player()->query_attack()))
    {
        notify_fail("You cannot sneak off somewhere while fighting!\n");
        return 0;
    }

    hiding = environment(this_player())->query_prop(ROOM_I_HIDE);
    bval = (this_player()->query_skill(SS_SNEAK) * 2 + this_player()->query_skill(SS_HIDE)) / 3;
    bval = (bval - hiding) / 2;

    if (hiding < 0 || bval <= 0)
    {
        notify_fail("It's too difficult to sneak from here.\n");
        return 0;
    }   

    val = bval + random(bval);
    this_player()->add_prop(OBJ_I_HIDE, val);

    this_player()->add_prop(LIVE_I_SNEAK, 1);
    this_player()->command(str);

    hiding = environment(this_player())->query_prop(ROOM_I_HIDE);
    bval = this_player()->query_skill(SS_HIDE);
    bval = (bval - hiding) / 2;

    if (hiding < 0 || bval <= 0)
    {
        write("It's too difficult to hide in here, you're visible again.\n");
        this_player()->reveal_me(0);
        return 1;
    }   

    if (this_player()->query_prop(OBJ_I_LIGHT) &&  
        (this_player()->query_prop(OBJ_I_LIGHT) > 
        environment(this_player())->query_prop(OBJ_I_LIGHT)))
    {
        write("You cannot hide here, shining like that!\n");
        this_player()->reveal_me(1);
        return 1;
    }

    val = bval + random(bval);
    this_player()->add_prop(OBJ_I_HIDE, val);
    return 1;
}

/*
 * Function name: track
 * Description:   look for tracks
 * Argument:      str - the string given to the command
 * Returns:       0 - failure
 */
int
track(string str)
{
    object  room = ENV;

    if (this_player()->query_attack())
    {
        notify_fail("But you are in the middle of a fight!\n");
        return 0;
    }

    if (stringp(str) &&
        (str != "here"))
    {
        notify_fail("Track where?\n");
        return 0;
    }

    if (!room->query_prop(ROOM_I_IS))
    {
        notify_fail("You cannot look for tracks here!\n");
        return 0;
    }

    if (room->query_prop(ROOM_I_INSIDE))
    {
        notify_fail("You cannot look for tracks inside a room!\n");
        return 0;
    }

    if (this_player()->query_mana() < 2*F_TRACK_MANA_COST)
    {
        notify_fail("You are mentally too exhausted to look for tracks.\n");
        return 0;
    }

    write("You kneel down to examine the ground closely, looking for tracks.\n");
    say(QCTNAME(this_player()) + " kneels down to examine the ground closely.\n");

    this_player()->add_prop(LIVE_S_EXTRA_SHORT, ", kneeling on the ground");
    this_player()->add_mana(-F_TRACK_MANA_COST);

    room->track_room();
    return 1;
}

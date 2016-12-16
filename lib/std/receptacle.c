/*
 * /std/receptacle.c
 *
 * This is a default that has code to support "open" and "close" of containers
 * in general. Furthermore, there is also support for if you want to use a key
 * to "lock" or "unlock" the container. In addition it is possible to "pick"
 * the lock on the container if it has a lock.
 *
 * Note that this code only supports manipulation of objects that inherit
 * this code, and it cannot be used to handle other containers that do not
 * inherit this code.
 *
 * If you want to add your a special check before you manipulate a container,
 * or do something else, you can add extra code. This code should be in the
 * following functions:
 *
 *   varargs int open(object ob)
 *   varargs int close(object ob)
 *   varargs int lock(object ob)   Only if the container has a lock
 *   varargs int unlock(object ob) Only if the container has a lock
 *   varargs int pick(object ob)   Only is the lock is pickable
 *
 * These functions are called with the objectpointer to the currently handled
 * container and are optional to use. To specify in which files the functions
 * can be found you have to use:
 *
 *   void set_cf(object o) Set the object that defines open(), close(),
 *                         lock(), unlock(), pick() functions.
 *
 * The special functions should return 0 if there is no change to the
 * manipulation of the containers, 1 if they can be handled, but the message
 * is written inside the function and -1 if it is NOT possible to handle the
 * container and the default fail message will be displayed and finally -2 if
 * it is NOT possible to manipulate and a fail message will be printed inside
 * the function. Obviously, this_player() is known in those functions.
 *
 * To handle the lock on the container there are some functions to use:
 *
 *   void set_key(mixed keyvalue)  Set the value of the key
 *   void set_pick(int pick_level) Set the pick level
 *   void set_no_pick(1/0)         Set for unable to pick
 *
 * The default pick level is set to 40. You may set the pick level in the
 * range [1..99]. The pick level will be checked against a value that
 * is randomized from the players pick-skill. A value outside the interval
 * will make the lock unpickable. In addition you may set_no_pick to ensure
 * that the lock is not pickable.
 *
 * There are query_ functions for all set_ functions.
 *
 * NOTE
 *
 * You will get ugly plural short descriptions if you do not set the plural
 * short description yourself. If you do not set it, the plural short
 * description might look like "chest (open)s" for I add status information
 * on the container in the short, pshort and long description!
 *
 * ADVISE
 *
 * Do not redefine the functions short, pshort and long in your code. You
 * should better use set_short, set_pshort and set_long with VBFC in the call
 * to get a better result. Note that are able to do exactly the same things
 * with VBFC redefinitions of the functions. If you fail to do this, my
 * definitions of short, pshort and long will be lost, which means that
 * players will not get status information on their containers any more ;-(
 *
 * RECOVERY
 *
 * If you are going to make your container recoverable, and why should you
 * not do that, add the following functions and do not forget to
 * #include <macros.h> for the definition of MASTER.
 *
 *   string
 *   query_recover()
 *   {
 *       return MASTER + ":" + my_recover_args + query_container_recover();
 *   }
 *
 *   void
 *   init_recover(string arg)
 *   {
 *       my_init_recover_code();
 *       init_container_recover(arg);
 *   }
 */
#pragma save_binary
#pragma strict_types

inherit "/std/container";

#include <cmdparse.h>
#include <files.h>
#include <macros.h>
#include <ss_types.h>
#include <stdproperties.h>

#define MANA_TO_PICK_LOCK      10
#define PICK_SKILL_DIFFERENCE  30
#define HANDLE_KEY_FAIL_STRING " what with what?"

/*
 * A few prototypes. I hate them, but they are handy to keep the code clean.
 */
public nomask void   add_temp_libcontainer_checked(object player);
public nomask void   remove_temp_libcontainer_checked(object player);
public nomask int    filter_open_close_containers(object ob);
public nomask varargs object * normal_access(string str, string pattern,
    string fail_str);
public nomask mixed  normal_access_key(string str);
public nomask object get_key_to_fit(object *key_items, mixed key_value);
public int           do_default_open(string str);
public int           do_default_close(string str);
public int           do_default_lock(string str);
public int           do_default_unlock(string str);
public int           do_default_pick(string str);

/*
 * Global variables
 */
static object cont_func;   /* the object defining open, close etc.          */
static mixed  cont_key;    /* the identifier to the key to open this object */
static int    cont_pick = 40; /* how hard it is to pick the lock            */

/*
 * Function name: create_receptacle
 * Description  : Creator. You should define this function in order to set
 *                up the object. However, as this is just a container with
 *                some extra functionality, you can also use the function
 *                create_container().
 */
void
create_receptacle()
{
}

/*
 * Function name: create_container
 * Description  : Since this object is called /std/receptacle, you can
 *                use create_receptacle to create it. Also, as normal,
 *                create_container can be used, since this is just a
 *                container with some extra functions.
 */
void
create_container()
{
    create_receptacle();
}

/*
 * Function name: reset_receptacle
 * Description  : If you want to make this receptacle reset at a regular
 *                interval this is the call to make. As this is just a
 *                container with some extra functionality you can also use
 *                the function reset_container(). Do not forget that you
 *                must call enable_reset() from your create function to
 *                actually make this object reset.
 */
void
reset_receptacle()
{
}

/*
 * Function name: reset_container
 * Description  : If you create the container with create_receptacle,
 *                you shall want to reset it with reset_receptacle and
 *                this function allows you to do just that. Do not forget
 *                that you have to call enable_reset() from your create
 *                function in order to make this object actually reset.
 */
void
reset_container()
{
    reset_receptacle();
}

/*
 * Function name: init
 * Description  : Link the commands to manipulate the container to the
 *                player.
 */
public void
init()
{
    ::init(); /* always make the call to the previous definitions. */

    add_action(do_default_open,  "open");
    add_action(do_default_close, "close");

    if (cont_key)
    {
        add_action(do_default_lock,   "lock");
        add_action(do_default_unlock, "unlock");

        if (cont_pick != 0)
        {
            add_action(do_default_pick, "pick");
        }
    }
}

/*
 * Function name: real_short
 * Description  : Returns the short without the addition of open/close
 * Arguments    : for_obj - who wants to know the short
 * Returns      : The short description
 */
public varargs string
real_short(object for_obj)
{
    return ::short(for_obj);
}

/*
 * Function name: short
 * Description  : Add the status of the container to it.
 * Arguments    : for_obj - who wants to know the short
 * Returns      : The short description.
 */
public varargs string
short(object for_obj)
{
    return ::short(for_obj) +
        ((query_prop(CONT_I_CLOSED)) ? "" : " (open)");
}

/*
 * Function name: pshort
 * Description  : Add the status of the container to it.
 * Arguments    : for_obj - who wants to know the pshort
 * Returns      : The plural short description.
 */
public varargs string
plural_short(object for_obj)
{
    string plural_short_description = ::plural_short(for_obj);

    return ((strlen(plural_short_description)) ? 
        (plural_short_description +
            ((query_prop(CONT_I_CLOSED)) ? "" : " (open)")) : 0);
}

/*
 * Function name: long
 * Description  : A the status of the container to it.
 * Arguments    : string str - the psuedo-item the person wants to see.
 *                object for_obj - who wants to know the long
 * Returns      : string - The long description.
 */
public varargs string
long(string str, object for_obj)
{
    string desc;

    desc = ::long(str, for_obj);
    if (!strlen(str))
    {
        desc += ((query_prop(CONT_I_CLOSED)) ?
            "It is closed.\n" : "It is open.\n");
    }
    return desc;
}

/*
 * Function name: do_default_open
 * Description  : Try to open one or more chests. This command does not allow
 *                the use of specific tricks to prevent the command parser to
 *                try the open command on each container in his/her presence
 *                for the command should obviously also work on doors and
 *                such.
 *                However, with the use of the TEMP_LIBCONTAINER_CHECKED
 *                property, there will hardly be any loss of cpu-time.
 * Arguments    : str - the rest of the command line by the mortal
 * Returns      : 1 = success, 0 = failure.
 */
public int
do_default_open(string str)
{
    object tp = this_player();
    object *items;
    int    i;
    int    cres;
    int    succes = 0;
    string what;
    string gFail = "";
    string gSucces = "";

    if (!sizeof(items = normal_access(str, "%i")))
    {
        return 0;
    }

    for (i = 0; i < sizeof(items); i++)
    {
        what = (string)items[i]->real_short(tp);

        if (!(items[i]->query_prop(CONT_I_CLOSED)))
        {
            gFail += "The " + what + " is already open.\n";
        }
        else if (items[i]->query_prop(CONT_I_LOCK))
        {
            gFail += "The " + what + " is locked.\n";
        }
        else
        {
            if (!objectp(items[i]->query_cf()))
            {
                cres = 0;
            }
            else
            {
                cres = (int)((items[i]->query_cf())->open(items[i]));

                if (cres == 2)
                {
                    gFail += "The " + what + " cannot be opened.\n";
                }
            }

            if (cres == 0)
            {
                gSucces += "You open the " + what + ".\n";
                say(QCTNAME(tp) + " opens the " + items[i]->real_short() +
                    ".\n");
            }
            if (cres <= 1)
            {
                succes = 1;
                items[i]->remove_prop(CONT_I_CLOSED);
            }
        }
    }

    if (succes)
    {
        remove_temp_libcontainer_checked(tp);
        if (strlen(gFail))
        {
            write(gFail);
        }
        if (strlen(gSucces))
        {
            write(gSucces);
        }
        return 1;
    }

    notify_fail(gFail);
    return 0;
}

/*
 * Function name: do_default_close
 * Description  : Try to close one or more chests. This command does not allow
 *                the use of specific tricks to prevent the command parser to
 *                try the close command on each container in his/her presence
 *                for the command should obviously also work on doors and
 *                such.
 *                However, with the use of the TEMP_LIBCONTAINER_CHECKED
 *                property, there will hardly be any loss of cpu-time.
 * Arguments    : str - the rest of the command line by the mortal
 * Returns      : 1 = success, 0 = failure.
 */
public int
do_default_close(string str)
{
    object tp = this_player();
    object *items;
    int    i;
    int    cres;
    int    succes = 0;
    string what;
    string gFail = "";
    string gSucces = "";

    if (!sizeof(items = normal_access(str, "%i")))
    {
        return 0;
    }

    for (i = 0; i < sizeof(items); i++)
    {
        what = (string)items[i]->real_short(tp);

        if (items[i]->query_prop(CONT_I_CLOSED))
        {
            gFail += "The " + what + " is already closed.\n";
        }
        else
        {
            if (!objectp(items[i]->query_cf()))
            {
                cres = 0;
            }
            else
            {
                cres = (int)((items[i]->query_cf())->close(items[i]));

                if (cres == 2)
                {
                    gFail += "The " + what + " cannot be closed.\n";
                }
            }

            if (cres == 0)
            {
                gSucces += "You close the " + what + ".\n";
                say(QCTNAME(tp) + " closes the " + items[i]->real_short() +
                    ".\n");
            }
            if (cres <= 1)
            {
                succes = 1;
                items[i]->add_prop(CONT_I_CLOSED, 1);
            }
        }
    }

    if (succes)
    {
        remove_temp_libcontainer_checked(tp);
        if (strlen(gFail))
        {
            write(gFail);
        }
        if (strlen(gSucces))
        {
            write(gSucces);
        }
        return 1;
    }

    notify_fail(gFail);
    return 0;
}

/*
 * Function name: do_default_lock
 * Description  : Try to lock one container. This command does not allow
 *                the use of specific tricks to prevent the command parser to
 *                try the lock command on each container in his/her presence
 *                for the command should obviously also work on doors and
 *                such.
 *                However, with the use of the TEMP_LIBCONTAINER_CHECKED
 *                property, there will hardly be any loss of cpu-time.
 * Arguments    : str - the rest of the command line by the mortal
 * Returns      : 1 = success, 0 = failure.
 */
public int
do_default_lock(string str)
{
    object tp = this_player();
    object container_item;
    object key_item;
    mixed  items;
    mixed  keyval;
    string qvb = query_verb();

    if (normal_access(str, "foobar", HANDLE_KEY_FAIL_STRING) == 0)
    {
        return 0;
    }
    if (!pointerp(items = normal_access_key(str)))
    {
        return 0;
    }
    container_item = items[0][0];

    if (!(keyval = container_item->query_key()))
    {
        notify_fail("The " + container_item->real_short(tp) +
            " does not have a lock.\n");
        return 0;
    }

    if (container_item->query_prop(CONT_I_LOCK))
    {
        notify_fail("The " + container_item->real_short(tp) +
            " is already locked.\n");
        return 0;
    }

    if (!objectp(key_item = get_key_to_fit(items[1], keyval)))
    {
        notify_fail("You do not have the key to " + qvb + " the " +
            container_item->real_short(tp) + " on you.\n");
        return 0;
    }

    if ((objectp(container_item->query_cf())) &&
        ((container_item->query_cf())->lock(container_item)))
    {
        write("You fail to " + qvb + " the " +
            container_item->real_short(tp) + ".\n");
        return 1;
    }

    container_item->add_prop(CONT_I_LOCK, 1);
    write("You " + qvb + " the " + container_item->real_short(tp) + ".\n");
    say(QCTNAME(tp) + " " + qvb + "s the " + container_item->real_short() +
        ((environment(this_object()) == this_player()) ?
        (" " + tp->query_pronoun() + " carries") : "") + ".\n");

    return 1;
}

/*
 * Function name: do_default_unlock
 * Description  : Try to unlock one container. This command does not allow
 *                the use of specific tricks to prevent the command parser to
 *                try the lock command on each container in his/her presence
 *                for the command should obviously also work on doors and
 *                such.
 *                However, with the use of the TEMP_LIBCONTAINER_CHECKED
 *                property, there will hardly be any loss of cpu-time.
 * Arguments    : str - the rest of the command line by the mortal
 * Returns      : 1 = success, 0 = failure.
 */
public int
do_default_unlock(string str)
{
    object tp = this_player();
    object container_item;
    object key_item;
    mixed  items;
    mixed  keyval;
    string qvb = query_verb();

    if (normal_access(str, "foobar", HANDLE_KEY_FAIL_STRING) == 0)
    {
        return 0;
    }
    if (!pointerp(items = normal_access_key(str)))
    {
        return 0;
    }
    container_item = items[0][0];

    if (!(keyval = container_item->query_key()))
    {
        notify_fail("The " + container_item->real_short(tp) +
            " does not have a lock.\n");
        return 0;
    }

    if (!(container_item->query_prop(CONT_I_LOCK)))
    {
        notify_fail("The " + container_item->real_short(tp) +
            " is not locked.\n");
        return 0;
    }

    if (!objectp(key_item = get_key_to_fit(items[1], keyval)))
    {
        notify_fail("You do not have the key to " + qvb + " the " +
            container_item->real_short(tp) + " on you.\n");
        return 0;
    }

    if ((objectp(container_item->query_cf())) &&
        ((container_item->query_cf())->unlock(container_item)))
    {
        write("You fail to " + qvb + " the " +
            container_item->real_short(tp) + ".\n");
        return 1;
    }

    container_item->remove_prop(CONT_I_LOCK);
    write("You " + qvb + " the " + container_item->real_short(tp) + ".\n");
    say(QCTNAME(tp) + " " + qvb + "s the " + container_item->real_short() +
        ((environment(this_object()) == this_player()) ?
        (" " + tp->query_pronoun() + " carries") : "") + ".\n");

    return 1;
}

/*
 * Function name: do_default_pick
 * Description  : Allows someone to try to pick the lock. The players pick
 *                skill will be randomized a little and then checked against
 *                the pick level of this lock.
 * Arguments    : str - the argument to the command line verb
 * Reurns       : 1   - success on picking a lock
 *                0   - on failure
 */
public int
do_default_pick(string str)
{
    object tp = this_player();
    object *items;
    int    skill;
    int    res;

    if (!sizeof(items = normal_access(str, " 'lock' 'on' %i ",
        " lock on what?")))
    {
        return 0;
    }

    skill = (int)tp->query_skill(SS_OPEN_LOCK);
    if (skill == 0)
    {
        write("You do not know how to pick a lock.\n");
        return 1;
    }

    if (tp->query_mana() <= MANA_TO_PICK_LOCK)
    {
        write("You cannot concentrate enough to pick a lock.\n");
        return 1;
    }
    tp->add_mana(-(MANA_TO_PICK_LOCK));

    if (!(items[0]->query_prop(CONT_I_LOCK)))
    {
        write("Much to your surprise you find that the " +
            items[0]->real_short(tp) + " is not locked.\n");
        return 1;
    }

    if (items[0]->query_no_pick())
    {
        write("Close examination of the lock on the " +
            items[0]->real_short(tp) +
            " teaches you that it is unpickable.\n");
        return 1;
    }

    /*
     * /std/door.c uses total random on the skill and I do not like that. In
     * fact, I do not even like this function. :-(
     */
    skill = ((skill * 3 / 4) + random(skill / 2));

    if (skill >= items[0]->query_pick())
    {
        if (!objectp(items[0]->query_cf()) ||
            ((res = items[0]->query_cf()->pick(items[0])) >= 0))
	{
            items[0]->remove_prop(CONT_I_LOCK);
        
            remove_temp_libcontainer_checked(tp);

            if (res == 0)
	    {
        	say(QCTNAME(tp) + " fumbles with the lock on the " +
        	    items[0]->real_short() +
        	    ((environment(this_object()) == this_player()) ?
        	    (" " + tp->query_pronoun() + " carries") : "") +
        	    " and you hear soft 'click' coming from it.\n");
        	write("You feel very satisfied when you hear a soft 'click' from " +
        	    "the lock on the " + items[0]->real_short(tp) +
        	    " after you picked it.\n");
	    }

            return 1;
	}
    }
    else
    {
        items[0]->pick_fail(this_player(), skill);
    }

    remove_temp_libcontainer_checked(tp);

    if (res >= -1)
    {
    	say(QCTNAME(tp) + " fumbles with the lock on the " +
    	    items[0]->real_short() +
    	    ((environment(this_object()) == this_player()) ?
    	    (" " + tp->query_pronoun() + " carries") : "") +
    	    ". Nothing happens though.\n");
    	write("You fail to pick the lock on the " + items[0]->real_short(tp) +
    	    "." + ((skill + PICK_SKILL_DIFFERENCE < items[0]->query_pick()) ?
    	    " The lock seems unpickable to you.\n" : "\n"));
    }

    return 1;
}

/*
 * Function name: set_cf
 * Description  : Allows you to specify an object that holds code that is
 *                called before you try to open a specific object. This code
 *                will be called with:
 *                  open() and
 *                  close()
 *                These functions should return 1 if it is not possible to
 *                open/close the container and 0 if it is oke to manipulate
 *                it.
 * Arguments    : object obj - the object specifying the code.
 */
public void
set_cf(object obj)
{
    /* All changes to the object might have been ruled out. */
    if (query_lock())
    {
        return;
    }

    cont_func = obj;
}

/*
 * Function name: query_cf
 * Description  : Query the object that holds protection code.
 * Returns      : object - the object specifying the code.
 */
public object
query_cf()
{
    return cont_func;
}

/*
 * Function name: set_no_pick
 * Description  : Make sure the lock on the container is not pickable.
 */
public void
set_no_pick()
{
    /* all changes to the object might have been ruled out. */
    if (query_lock())
    {
        return;
    }

    cont_pick = 0;
}

/*
 * Function name: query_no_pick
 * Description  : Returns whether the lock on the container can be picked.
 * Returns      : 1 - lock on container is not pickable
 *                0 - lock on container is pickable
 */
public int
query_no_pick()
{
    return (cont_pick == 0);
}

/*
 * Function name: set_pick
 * Description  : Set the difficulty to pick the lock.
 * Arguments    : i - the pick level
 */
public void
set_pick(int i)
{
    /* all changes to the object might have been ruled out. */
    if (query_lock())
    {
        return;
    }

    if ((i > 0) && (i < 100)) /* a legal pick value */
    {
        cont_pick = i;
        return;
    }

    cont_pick = 0; /* not pickable */
}

/*
 * Function name: query_pick
 * Description  : Returns how easy it is to pick the lock on a door
 * Returns      : int - the pick level
 */
public int
query_pick()
{
    return cont_pick;
}

/*
 * Function name: set_key
 * Description  : Set the number of the key that fits this container.
 * Arguments    : keyval - the key to the container
 */
public void
set_key(mixed keyval)
{
    /* All changes might have been locked out. */
    if (query_lock())
    {
        return;
    }

    cont_key = keyval;
}

/*
 * Function name: query_key
 * Description  : Query the key that fits this container.
 * REturns      : mixed - the key to the container
 */
public mixed
query_key()
{
    return cont_key;
}

/*
 * Function name: get_key_to_fit
 * Description  : Get the key to fir a certain lock from an array of keys
 *                that fit the description the player wanted to test. This
 *                container allows you to specify multiple keys to test.
 * Arguments    : key_items - the keys to test
 *                key_value - the key-value to test
 * Returns      : object    - the first key that the player has in his
 *                            inventory and that matches all requirements.
 *                0         - failure
 */
public nomask object
get_key_to_fit(object *key_items, mixed key_value)
{
    int i;

    for(i = 0; i < sizeof(key_items); i++)
    {
        if ((function_exists("create_object", key_items[i]) == KEY_OBJECT) &&
            (key_items[i]->query_key() == key_value) &&
            (environment(key_items[i]) == this_player()))
        {
            return key_items[i];
        }
    }

    return 0;
}

/*
 * Function name: normal_access
 * Description  : This function is used by all commands attached to the
 *                container to get an array of objects that the player wanted
 *                to manupulate
 * Arguments    : str      - the argument to the command line verb
 *                pattern  - the pattern to match the argument
 *                fail_str - the string to add to notify_fail on failure
 * Returns      : object * - the objects the player wants to handle
 *                0        - on failure.
 */
public varargs nomask object *
normal_access(string str, string pattern, string fail_str)
{
    object tp = this_player();
    object *items;
    int    retry;

    /* No access on another container, so don't bother to check this one. */
    if (retry = tp->query_prop(TEMP_LIBCONTAINER_CHECKED))
    {
        /* Auto-remove property when it's been too long. */
        if (time() < retry)
        {
            return 0;
        }
        /* Intentional fallthrough. */
    }

    add_temp_libcontainer_checked(tp);
    if (!CAN_SEE_IN_ROOM(tp))
    {
        notify_fail("It is too dark to see.\n");
        return 0;
    }

    notify_fail(capitalize(query_verb()) +
        (strlen(fail_str) ? fail_str : " what?") + "\n");
    if (!strlen(str))
    {
        return 0;
    }

    if (!sizeof(items =
    	filter((all_inventory(environment(tp)) + all_inventory(tp)),
    	       filter_open_close_containers)))
    {
        return 0;
    }

    if (fail_str == HANDLE_KEY_FAIL_STRING)
    {
        /* this means that this function is just used for basic access */
        return ({ tp });
    }

    if (!(parse_command(str, items, pattern, items)))
    {
        return 0;
    }
    if (!sizeof(items = NORMAL_ACCESS(items, 0, 0)))
    {
        return 0;
    }

    return items;
}

/*
 * Function name: normal_access_key
 * Description  : This function is used by all commands attached to the
 *                container to get an array of objects that the player wanted
 *                to manupulate and the keys he wants to manipulate the
 *                containers with.
 * Arguments    : str   - the argument to the command line verb
 * Returns      : mixed - the objects the player wants to handle and what he
 *                        wants to handle them with in an array of arrays:
 *                        ({ ({ container_items }), ({ key_items }) })
 *                0     - on failure.
 */
public nomask mixed
normal_access_key(string str)
{
    object *container_items;
    object *key_items;

    notify_fail(capitalize(query_verb()) + HANDLE_KEY_FAIL_STRING + "\n");

    if (!parse_command(str, environment(this_player()), " %i 'with' %i ",
        container_items, key_items))
    {
        return 0;
    }
    if ((!(sizeof(container_items = NORMAL_ACCESS(container_items, 0, 0)))) ||
        (!(sizeof(key_items = NORMAL_ACCESS(key_items, 0, 0)))))
    {
        return 0;
    }
    if (!sizeof(container_items =
    	filter(container_items, filter_open_close_containers)))
    {
        return 0;
    }

    return ({ container_items, key_items });
}

/*
 * Function name: filter_open_close_containers
 * Description  : Filter to get only the containers that inherit this file as
 *                well for we don't want to close a container that does not
 *                have an open command.
 * Arguments    : ob - the object to process
 * Returns      : 1 - this container is oke.
 *                0 - apparently it is not a good container.
 */
public nomask int
filter_open_close_containers(object ob)
{
    return (function_exists("filter_open_close_containers", ob) ==
    	RECEPTACLE_OBJECT);
}

/*
 * Function name: add_temp_libcontainer_checked
 * Description  : If a player handled a container and failed, he will not
 *                try to handle other containers in the same heartbeat.
 * Arguments    : player - the player to add it to.
 */
public nomask void
add_temp_libcontainer_checked(object player)
{
    set_alarm(1.0, 0.0, &player->remove_prop(TEMP_LIBCONTAINER_CHECKED));
    /* Use two seconds grace in case the property isn't removed somehow. */
    player->add_prop(TEMP_LIBCONTAINER_CHECKED, (time() + 2));
}

/*
 * Function name: remove_temp_libcontainer_checked
 * Description  : If a player handled a container and failed, he will not
 *                try to handle another containers in the same heartbeat.
 * Arguments    : player - the player to remove it from
 */
public nomask void
remove_temp_libcontainer_checked(object player)
{
    player->remove_prop(TEMP_LIBCONTAINER_CHECKED);
}

/*
 * Function name: stat_object
 * Description  : This function is called when a wizard wants to know more
 *                about the container.
 * Returns      : string - the stats of the container.
 */
public string
stat_object()
{
    string str = ::stat_object();

    if (cont_key)
    {
        str += "Key number : " + cont_key + "\n";

        if (cont_pick == 0)
        {
            str += "Its lock is not pickable.\n";
        }
        else
        {
            str += "Pick level : " + cont_pick + "\n";
        }
    }
    else
    {
        str += "It has no lock.\n";
    }

    return str;
}

/*
 * Function name: get_pick_chance
 * Description  : Make a string from the difference in pick-skill of the
 *                player and pick-level of the container for the appraise
 *                by the player
 * Arguments    : pick_val - the mentioned difference
 * Returns      : string   - a nice string with the mentioned description.
 */
public nomask string
get_pick_chance(int pick_val)
{
    if (pick_val >= 40)
        return "pickable by only looking at it";
    else if (pick_val >= 30)
        return "very easy to pick";
    else if (pick_val >= 20)
        return "quite easy to pick";
    else if (pick_val >= 10)
        return "easy to pick";
    else if (pick_val >= 0)
        return "pickable";
    else if (pick_val >= -10)
        return "tricky, but pickable";
    else if (pick_val >= -20)
        return "difficult to pick";
    else if (pick_val >= -30)
        return "very hard to pick";
    else if (pick_val >= -40)
        return "almost impossible to pick";

    /* pick_val apparently < -40 */
    return "completely unpickable by you";
}

/*
 * Function name: appraise_object
 * Description  : This function is called when a player appraises the
 *                container to find out more about it.
 * Arguments    : num - use this num rather than the players appraise skill
 */
public varargs void
appraise_object(int num)
{
    int pick_level = cont_pick;
    int seed;
    int skill;

    ::appraise_object(num);

    if (!cont_key)
    {
        return;
    }

    if (!num)
    {
	skill = (int)this_player()->query_skill(SS_APPR_OBJ);
    }
    else
    {
	skill = num;
    }

    sscanf(OB_NUM(this_object()), "%d", seed);
    skill = random((1000 / (skill + 1)), seed);
    pick_level = (int)this_player()->query_skill(SS_OPEN_LOCK) -
        cut_sig_fig(pick_level + (skill % 2 ? -skill % 70 : skill) *
	pick_level / 100);

    write ("You appraise that its lock is " + get_pick_chance(pick_level) +
        ".\n");
}

/*
 * Function name: query_container_recover
 * Description  : Return the recover string with information on the status
 *                of the container.
 * Returns      : part of recover string
 */
public nomask string
query_container_recover()
{
    return ("#c_c#" + query_prop(CONT_I_CLOSED) + "#" +
            "#c_l#" + query_prop(CONT_I_LOCK)   + "#");
}

/*
 * Function name: init_container_recover
 * Description  : Initialize the container to the status it had before the
 *                reboot.
 * Arguments    : arg - the total recover string
 */
public nomask void
init_container_recover(string arg)
{
    string foobar;
    int    tmp;

    sscanf(arg, "%s#c_c#%d#%s", foobar, tmp, foobar);
    add_prop(CONT_I_CLOSED, tmp);
    sscanf(arg, "%s#c_l#%d#%s", foobar, tmp, foobar);
    add_prop(CONT_I_LOCK, tmp);
}

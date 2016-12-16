/*
 * /std/scroll.c      
 *
 * A standard object that can be both read, and mread :)
 *
 * set_file(string file) - Will connect a file to the read command
 * 
 * set_autoload()        - Will make this object autoload
 */

#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <config.h>
#include <cmdparse.h>
#include <files.h>
#include <macros.h>
#include <stdproperties.h>

/*
 * Global variables.
 */
string scroll_file;
int auto_load;

/*
 * Prototypes.
 */
static int read_scroll(string str);

/*
 * Function name: init
 * Description  : Initialize commandverbs.
 */
public void
init()
{
    ::init();

    add_action(read_scroll, "mread");
    add_action(read_scroll, "read");
}

/*
 * Function name: create_scroll
 * description  : Create the scroll. You should redefine this function if
 *                you want to create the scroll.
 */
void
create_scroll()
{
    set_name("scroll");
    set_long("An empty scroll, no use to either read or mread it.\n");
}

/*
 * Function name: create_object
 * Description  : Create the object. You must define the function
 *                create_scroll() to define the scroll.
 */
nomask void
create_object()
{
    add_prop(OBJ_I_WEIGHT, 10); /* 10 g */
    add_prop(OBJ_I_VOLUME, 10); /* 10 ml */
    remove_prop(OBJ_I_VALUE); /* 0 copper */

    create_scroll();
}

/*
 * Function name: reset_scroll
 * Description  : Define this function if you want the scroll to reset at
 *                a certain interval and do not forget to call the function
 *                enable_reset() to initialize the reset-process.
 */
void
reset_scroll()
{
}

/*
 * Function name: reset_object
 * Description  : Reset the scroll. You may not mask this function so you
 *                have to define the function reset_scroll in your scroll.
 */
nomask void
reset_object()
{
    reset_scroll();
}

/*
 * Function name: set_file
 * Description  : Set the file to list when player reads this.
 * Arguments    : string file - the filename.
 */
public void
set_file(string file)
{
    scroll_file = file;
}

/*
 * Function name: query_file
 * Description  : Query what file is connected to read command
 * Returns      : string - the filename.
 */
string
query_file()
{
    return scroll_file;
}

/*
 * Function name: set_autoload
 * Description  : If you call this function the scroll will become
 *                autoloadable.
 */
void
set_autoload()
{
    auto_load = 1;
}

/*
 * Function name: init_arg
 * Description  : Set the parameters of the scroll when auto-loading.
 * Arguments    : string arg - the argument to init.
 */
public void
init_arg(string arg)
{
    string tmp;

    if (!strlen(arg))
    {
	return;
    }

    sscanf(arg, "%s##%s", scroll_file, tmp);

    if (tmp == "0")
	add_prop(OBJ_M_NO_DROP, 0);
    else if (tmp == "1")
	add_prop(OBJ_M_NO_DROP, 1);
    else
	add_prop(OBJ_M_NO_DROP, tmp);
   
    auto_load = 1;
}

/*
 * Function name: query_auto_load
 * Description:   Autoload this object.
 */
public string
query_auto_load()
{
    if (auto_load)
        return MASTER_OB(this_object()) + ":" + scroll_file + "##" +
		query_prop(OBJ_M_NO_DROP);
}

/*
 * Function name: read_scroll
 * Description  : This function is called when the player wants to read
 *                the scroll.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
static int
read_scroll(string str)
{
    string vb;
    object *a;

    if (this_player()->query_prop(TEMP_STDREAD_CHECKED))
        return 0;

    vb = query_verb();
    notify_fail(capitalize(vb) + " what?\n", 0);
    if (!str)
	return 0;

    a = CMDPARSE_ONE_ITEM(str, "read_access", "read_access");
    if (sizeof(a) > 0)
    {
        a[0]->read_it(vb);
        return 1;
    }

    set_alarm(1.0, 0.0, &(this_player())->remove_prop(TEMP_STDREAD_CHECKED));
    this_player()->add_prop(TEMP_STDREAD_CHECKED, 1);
    return 0;
}

/*
 * Function name: read_access
 * Description  : This function checks whether the object wanted is a true
 *                scroll and whether the player has the scroll in either his
 *                inventory of in his environment.
 * Arguments    : object ob - the object to test.
 * Returns      : int 1/0 - true if the conditions are met.
 */
public int
read_access(object ob)
{
    return (((environment(ob) == this_player()) ||
	 (environment(ob) == environment(this_player()))) &&
	(function_exists("create_object", ob) == SCROLL_OBJECT));
}

/*
 * Function name: read_it
 * Description  : Perform the actual read. This function printes the text
 *                to the player and displays a message to the people in
 *                the room.
 * Arguments    : string - the command the player executed, either 'read'
 *                         or 'mread'.
 */
void
read_it(string str)
{
    seteuid(getuid(this_object()));
    say(QCTNAME(this_player()) + " reads the " +
	QSHORT(this_object()) + ".\n");

    if (!scroll_file)
    {
        write("There was nothing to read in the " + QSHORT(this_object()) +
        ".\n");
        return;
    }

    if ((str == "read") &&
    	(file_size(scroll_file) < 4000))
    {
        cat(scroll_file);
    }
    else
    {
	if (file_size(scroll_file) >= 50000)
	{
	    write("The scroll is too large to be read.\n");
	    return;
	}

        this_player()->more(read_file(scroll_file));
    }
}

/*
 *  The standard herb.
 *
 *  The original made by Elessar Telcontar of Gondor, 
 *		Genesis, April to July 1992.
 */

#pragma save_binary

inherit "/std/object";
inherit "/lib/herb_support";

#include <cmdparse.h>
#include <composite.h>
#include <files.h>
#include <formulas.h>
#include <language.h>
#include <macros.h>
#include <ss_types.h>
#include <stdproperties.h>

/*
 * Prototype.
 */
public int ingest_it(string str);

/*
 * Variables
 */
int 	find_diff, id_diff, herb_value;
int     food_amount, dried, dryable, ate_it;
string	id_long, unid_long, ingest_verb, herb_name, herb_pname;
object  *gFail;
static string *herb_obj_names, *herb_obj_pnames;

/*
 * Function name: do_id_check
 * Description:   This little function is called each time the herb is referred
 *                to by a player, to check if (s)he identifies it or not.
 * Arguments:	  object player - The player, defaults to this_player()
 * Returns:       int - 1 for identification, else 0.
 */
varargs int
do_id_check(object player = this_player())
{
    return (id_diff <= player->query_skill(SS_HERBALISM));
}

/*
 * Function name: init
 * Description:   adds the ingest-action to the player
 */
void
init()
{
    ::init(); /* If this isn't added cmd items won't work. */

    add_action(ingest_it, ingest_verb);
    if (ingest_verb != "eat")
        add_action(ingest_it, "eat");
}

/*
 * Function name: long_description
 * Description:   This is an VBFC function for the set_long in the herb, which
 *                tests if the player examining it can identify it, before
 *                returning the appropriate long-description. To make this
 *                work, you must be sure to do set_id_long(str) and
 *                set_unid_long(str) from the create_herb() function.
 */
nomask string
long_description()
{
    return do_id_check() ? id_long : unid_long;
}

/*
 * Function Name: parse_command_id_list
 * Description  : Used by parse_command to find the names of this
 *                item.
 * Returns      : An array of the names this herb can be references with.
 */
public string *
parse_command_id_list()
{
    if (!do_id_check())
        return ::parse_command_id_list();

    /* Array is cached for better performance */
    if (!herb_obj_names)
        herb_obj_names = ::parse_command_id_list() + ({ herb_name });

    return herb_obj_names;
}

/*
 * Function Name: parse_command_plural_id_list
 * Description  : Used by parse_command to find the names of this
 *                item.
 * Returns      : An array of the names this herb can be references with.
 */
public string *
parse_command_plural_id_list()
{
    if (!do_id_check())
        return ::parse_command_plural_id_list();

    if (!herb_obj_pnames)
        herb_obj_pnames = ::parse_command_plural_id_list() + ({ herb_pname });
    return herb_obj_pnames;
}

/*
 * Function name: set_ingest_verb
 * Description:   Set what verb the player should type to be able to ingest 
 * 		  the herb. Default is "eat"
 * Arguments:     str - The verb
 */
void
set_ingest_verb(string str) { ingest_verb = str; }

/*
 * Function name: query_ingest_verb
 * Description:   What verb is required to ingest this herb?
 * Returns:	  The verb;
 */
string
query_ingest_verb() { return ingest_verb; }

/*
 * Function name: set_decay_time
 * Description:	  Set how long time it takes for the herb to decay.
 *                OBSOLETE! Retained only for backward compatibility.
 * Argumetns:	  i - the time (in seconds)
 */
void
set_decay_time(int i) { }

/*
 * Function name: set_dryable
 * Description:   Calling this function makes the herb dryable
 */
void
set_dryable() { if (!dried) dryable = 1; }

/*
 * Function name: query_dryable
 * Description:   Is the function dryable?
 * Returns:       1 if it is dryable
 */
int
query_dryable() { return dryable; }

/*
 * Function name: dry
 * Description:   This function is called when a herb dries, to allow
 *                different effects for dried herbs.
 */
void
dry() {}

/*
 * Function name: force_dry
 * Description:   Call this function if you want to make the herb
 *                dry after creation. E.g. if you have some tool that
 *                makes a herb dry. If you want to set the herb 
 *                to dried at creation, use set_dried.
 */
void
force_dry()
{
    string pshort;

    if (dried || !dryable) return;

    dryable = 0;
    dried = 1;
    set_short("dried " + short());
    if (pshort = query_plural_short())
      set_pshort("dried " + pshort);
    add_adj("dried");
    dry();
}
 
/*
 * Function name: set_dried
 * Description:   Set the herb to dried. Use this function in
 *                create_herb. If you want to make the herb dry
 *                after creation, use force_dry.
 */
void
set_dried()
{
    if (!dried)
    {
	dried = 1;
	dryable = 0;
    }
}

/*
 * Function name: query_dried
 * Description:   Is the herb dried?
 * Returns:	  1 if dried
 */
int
query_dried() { return dried; }

/*
 * Function name: set_herb_name
 * Description:   Set the true name of the herb
 * Arguments:	  str - The name
 */
void
set_herb_name(string str)
{
    herb_name = str;
    herb_pname = LANG_PWORD(str);
}

/*
 * Function name: query_herb_name
 * Description:   What is the true name of the herb
 * Returns:	  The name of the herb
 */
string
query_herb_name() { return herb_name; }

/*
 * Function name: set_id_long
 * Description:   Set the long description you see if you know what herb it
 *		  is.
 * Arguments:     str - The description
 */
void
set_id_long(string str) { id_long = str; }

/*
 * Function name: query_id_long
 * Description:   The long description if you can id the herb
 * Returns:       The long description
 */
string query_id_long() { return id_long; }

/*
 * Function name: set_unid_long
 * Description:   Set the long description you see if you cannot identify the 
 *		  herb.
 * Arguments:     str - The long description
 */
void
set_unid_long(string str) { unid_long = str; }

/*
 * Function name: query_unid_long
 * Description:   Query the long description you get if you cannot identify
 *		  the herb.
 * Returns:   	  The unidentified long description
 */
string
query_unid_long() { return unid_long; }

/*
 * Function name: set_id_diff
 * Description:   Set how hard it is to identify a herb
 * Arguments:     i - The skill needed to know the herb
 */
void
set_id_diff(int i) { id_diff = i; }

/*
 * Function name: query_id_diff
 * Description:   How hard is it to identify this herb
 * Returns:  	  The difficulty
 */
int
query_id_diff() { return id_diff; }

/*
 * Function name: set_find_diff
 * Description:   Set how hard it is to find the herb
 * Arguments:     i - Difficulty (suggested range is 0 - 10)
 */
void
set_find_diff(int i) { find_diff = i; }

/*
 * Function name: query_find_diff
 * Description:   How hard is it to find this herb
 * Returns: 	  The difficulty to find the herb in the nature
 */
int
query_find_diff() { return find_diff; }

/*
 * Function name: set_herb_value
 * Description:   Set the value of the herb when dealing with a herb specialist
 * Arguments:     i - The value
 */
void
set_herb_value(int i) { herb_value = i; }

/*
 * Function name: query_herb_value
 * Description:   The value of the herb when dealing with a specialist
 * Returns:	  The value
 */
int
query_herb_value() { return herb_value; }

/*
 * Function name:       set_amount
 * Description:         sets the amount of food in this herb (in grams)
 * Arguments:           a: The amount of food
 */
public void
set_amount(int a) 
{ 
    food_amount = a; 
    add_prop(OBJ_I_VOLUME, max(1, (a / 5)));
    add_prop(OBJ_I_WEIGHT, a);
}

/*
 * Function name:       query_amount
 * Description:         Gives the amount of food in this herb
 * Returns:             Amount as int (in grams)
 */
public int
query_amount() { return food_amount; }

/*
 * Function name:       set_ate_it
 * Description:         This is called if the herb is eaten and the 
 *                      ingest_verb != "eat"
 */
void
set_ate_it() { ate_it = 1; }

/*
 * Function name: ate_non_eat_herb
 * Description:   This function is called in stead of do_herb_effect
 *                if you eat a herb that isn't supposed to be eaten
 */
void
ate_non_eat_herb()
{
    write("You don't feel any effect.\n");
}

/*
 * Function name: create_herb
 * Description:   This is the create-function of the herb, which you should
 *                redefine and setup the herb from.
 */
void
create_herb()
{
}

/*
 * Function name: create_object
 * Description:   Constructor. Do not redefine this routine to make your own
 *                herb. Instead define create_herb().
 */
nomask void
create_object()
{
    set_name("herb");
    set_short("unknown herb");
    set_id_long("This herb has not been described by the wizard.\n");
    set_unid_long("This is an undescribed, unknown herb.\n");
    set_herb_name("xortion");
    set_amount(2);
    set_id_diff(20);
    set_find_diff(5);
    set_herb_value(10);
    set_ingest_verb("eat");

    add_prop(OBJ_I_VALUE, 0);

    /* If this will be true, the herb will not recover on reboot and is
     * lost after the reboot.
    will_not_recover = (random(100) < PERCENTAGE_OF_RECOVERY_LOST);
     */

    create_herb();
    set_long(long_description);
}

/*
 * Function name: reset_object
 * Description  : Reset routine. To reset this herb, define reset_herb() and
 *                call enable_reset() to start the reset functionality.
 */
nomask void
reset_object()
{
    this_object()->reset_herb();
}

/*
 * Function name: consume_text
 * Description  : Called when the player eats one or more herbs.
 * Arguments    : object *arr - the herbs eaten.
 *                string verb - the verb used for eating.
 */
void
consume_text(object *arr, string verb)
{
    string str;

    write("You " + verb + " " + (str = COMPOSITE_DEAD(arr)) + ".\n");
    say(QCTNAME(this_player()) + " " + verb + "s " + str + ".\n");
}

/*
 * Function name:	ingest_it
 * Description:		Ingests the objects described as parameter to the
 *                      ingest_verb. It uses command parsing to find which
 *                      objects to ingest.
 * Arguments:		string str: The trailing command after 'ingest_verb ...'
 * Returns:		True if command successfull
 */
public int
ingest_it(string str)
{
    object *foods;
    string str2, verb;
    
    if (this_player()->query_prop(TEMP_STDHERB_CHECKED) ||
	query_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT))
    {
	return 0;
    }

    gFail = ({ });
    verb = query_verb(); 
    
/* If you type "eat all" and the command comes to a herb first
 * it would cause all eatable herbs to be eaten but no food.
 * This is to prevent this. (How often do you want to eat all herbs?)
 * This also stops "smoke all" "chew all" and so on. 
 */
    if (str == "all")
    {
	notify_fail("You must specify which herb to " + verb + ".\n");
	return 0;
    }

    if (!strlen(str))
    {
        notify_fail(capitalize(verb) + " what?\n", 0);
	return 0;
    }
    
    foods = CMDPARSE_ONE_ITEM(str, "ingest_one_thing", "ingest_access");
    if (sizeof(foods) > 0)
    {
	consume_text(foods, verb);
	foods->destruct_object();
	return 1;
    }
    else
    {
	set_alarm(1.0, 0.0,
	    &(this_player())->remove_prop(TEMP_STDHERB_CHECKED));
	this_player()->add_prop(TEMP_STDHERB_CHECKED, 1);
	if (sizeof(gFail))
	    notify_fail("@@ingest_fail:" + file_name(this_object()));
	return 0;
    }
}

string
ingest_fail()
{
    string str;

    str = "You try to " + ingest_verb + " " + COMPOSITE_DEAD(gFail) +
	" but fail.\n";
    say(QCTNAME(this_player()) + " tries to " + ingest_verb + " " +
        QCOMPDEAD + " but fails.\n");
    this_player()->remove_prop(TEMP_STDHERB_CHECKED);
    return str;
}

int
ingest_access(object ob)
{ 
    string verb;

    verb = query_verb();

    if ((environment(ob) == this_player()) &&
	(function_exists("create_object", ob) == HERB_OBJECT) &&
	(verb == ob->query_ingest_verb() || verb=="eat") &&
        !ob->query_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT))
	return 1;
    else
	return 0;
}

int
ingest_one_thing(object ob)
{
    int am;
    string verb;

    verb=query_verb();
    am = (int) ob->query_amount();
    
    if (this_player()->query_prop(LIVE_I_HERB_EFFECT) > time() - F_HERB_INTERVAL)
    {
        write(capitalize(LANG_ASHORT(ob)) + " is too much for you.\n");
        gFail += ({ ob });
        return 0;
    }
    
    if (verb=="eat")
    {
        if (!this_player()->eat_food(am))
	{
	    write(capitalize(LANG_ASHORT(ob)) + " is too much for you.\n");
	    gFail += ({ ob });
	    return 0;
	}
	/* Test if you ate a non_eat herb */
	if (verb != ob->query_ingest_verb())
	    ob->set_ate_it();

        this_player()->add_prop(LIVE_I_HERB_EFFECT, time());
	return 1;
    }
    
    this_player()->add_prop(LIVE_I_HERB_EFFECT, time());
    return 1;
}

void
destruct_object()
{
    if (ate_it && ingest_verb != "eat")
    {
	ate_non_eat_herb();
    } else {
	do_herb_effects();
    }

    add_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT, 1);
    set_alarm(1.0, 0.0, remove_object);
}

/*
 * Function name: query_herb_recover
 * Description:   Return the recover strings for changing herb variables.
 */
string
query_herb_recover()
{
    return "#h_dr#" + dried + "#";
}

/*
 * Function name: init_herb_recover
 * Description:   Initialize the herb variables at recover.
 */
void
init_herb_recover(string arg)
{
    string foobar;
    int dr;

    sscanf(arg, "%s#h_dr#%d#%s", foobar, dr, foobar);
    if (dr && !dried)
    {
	force_dry();
    }
}

/*
 * Function name: query_recover
 * Description:   A default query_recover() for herbs.
 * Returns:	  A default recovery string.
 */
string
query_recover()
{
    /* Don't recover if we're about to destruct. */
    if (query_prop(TEMP_OBJ_ABOUT_TO_DESTRUCT))
        return 0;

    return MASTER + ":" + query_herb_recover();
}

/*
 * Function name: init_recover
 * Description:   A default init_recover() for herbs.
 * Arguments:	  arg - String with variables to recover.
 */
void
init_recover(string arg)
{
    init_herb_recover(arg);
}

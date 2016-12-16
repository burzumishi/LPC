/*
 * /d/Standard/doc/examples/obj/mirror.c
 *
 * This is a little mirror, if you examine it, you see yourself. *ack*
 * It is an example of how you can use VBFC, in this case for the long
 * description of the mirror.
 *
 * It also shows you how you can use mudlib functions in the player
 * object to do some nice things. However, they  are just specific
 * examples of that use. If you want to do something else, you probably
 * have to do some mudlib search yourself to get the right functions or
 * ask someone with more experience in them.
 *
 * When you are looking for information on functionality, a quick way to
 * get to the right functions in the right objects is to do:
 *
 * sman -k *topic*
 *
 * where topic is a sub-part of the name of the function that you are
 * looking for. It will return a list of all objects in the mudlib that
 * define a function and of the functions that have the topic as part
 * their name.
 *
 * /Mercade, 1 April 1994
 *
 * If you are going to use this code for your objects (what examples are
 * meant for :-) ), please add a line in your comment header saying that
 * you did so.
 *
 * Revision history:
 */

inherit "/std/object";

#include <composite.h>
#include <formulas.h>
#include <language.h>
#include <stdproperties.h>
#include <wa_types.h>

/*
 * This function is called to initialize the mirror.
 */
void
create_object()
{
    set_name("mirror");
    set_adj("small");
    add_adj("ornamented");
/*
 * Here we use VBFC to set the long description. Every time someone
 * examines the mirror, (s)he will get a description of him/herself, just
 * as well as a description of the mirror.
 */
    set_long("@@mirror_description@@");

    add_prop(OBJ_I_VALUE,  200);
    add_prop(OBJ_I_VOLUME, 200);
    add_prop(OBJ_I_WEIGHT, 500);
}

#define SCAR_IN_FACE ({ "forehead", "left cheek", "right cheek", "nose" })

string
beauty_description()
{
    int    i;
    int    scar = this_player()->query_scar();
    object armour;
    string *scar_desc = ({ });
    string *armours = ({ });
    string beauty;
    string scars;
    string wearing = "";

/*
 * This gives the string of what we think of our own beauty...
 */
    beauty = call_other("/cmd/live/state", "beauty_text",
	(this_player()->my_opinion(this_player())),
	!this_player()->query_gender());

/*
 * Give a description of the armours worn on the head and on the neck.
 * Earrings are not in the set of default hitlocations :-)
 */
    if (objectp(armour = this_player()->query_armour(TS_NECK)))
	armours += ({ LANG_ADDART(armour->short(this_player())) +
	    " around your neck" });
    if (objectp(armour = this_player()->query_armour(TS_HEAD)))
	armours += ({ LANG_ADDART(armour->short(this_player())) +
	    " on your head" });
    if (sizeof(armours))
	wearing = ", wearing " + COMPOSITE_WORDS(armours);

/*
 * Give us an array of all scars the player has in his/her face and make
 * a nice string of them.
 */
    for (i = 1; i < F_MAX_SCAR; i++)
	if (scar && (i * 2))
	   scar_desc += ({ F_SCAR_DESCS[i] });
    scar_desc -= (scar_desc - SCAR_IN_FACE);

    if (sizeof(scar_desc))
	scars = "You have " + ((sizeof(scar_desc) == 1) ? "a scar" : "scars") +
	    " on your " + COMPOSITE_WORDS(scar_desc) +
	    " and you ";
    else
	scars = "You ";

/*
 * Here we do a call_other to the object that normall generates the
 * descriptions if you examine someone. It returns how healthy you are.
 */
    return scars + "think that you look " + beauty + wearing +
	". You seem to be " +
	call_other("/cmd/live/state", "show_subloc_health", this_player(),
	this_player()) + ".\n";
}

/*
 * When someone examines this mirror, he gets an impression of him or
 * herself.
 */
string
mirror_description()
{
    string description;

/*
 * The description of the mirror with a description of your looks.
 */
    description = break_string("You look into the mirror and see yourself, " +
	LANG_ADDART(this_player()->query_nonmet_name()) + ". " +
	beauty_description(), 75) + "\n";

    return description;
}

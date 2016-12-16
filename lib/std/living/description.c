/*
 * /std/living/description.c
 *
 * This is a subpart of /std/living.c
 *
 * All description relevant routines are defined here.
 *
 * NOTE
 * There is some calls of the type: this_object()->function()
 * in a number of places. The reason for this is to allow those
 * functions to be shadowed as internal function calls are not
 * possible to shadow.
 *
 * $Id:$
 */

#include <composite.h>
#include <filter_funs.h>
#include <formulas.h>
#include <language.h>
#include <macros.h>
#include <state_desc.h>
#include <stdproperties.h>
#include <wa_types.h>
#include <state_desc.h>

#ifdef DAY_AND_NIGHT
#include <mudtime.h>
#endif

private static int appearance_offset;
private static int is_linkdead;

/*
 * Function name: set_linkdead
 * Description  : Set the linkdeath-status of the player.
 * Arguments    : int i - true if the player is linkdead.
 */
public void
set_linkdead(int i)
{
    if (i)
	is_linkdead = time();
    else
	is_linkdead = 0;
}

/*
 * Function name: query_linkdead
 * Description  : Return the linkdeath-status of the player.
 * Returns      : int - if false, the player is not linkdead. Else it
 *                      returns the time the player linkdied.
 */
public int
query_linkdead()
{
    return is_linkdead;
}

/*
 * Function name: query_humanoid
 * Description  : Tells whether we are humanoid or not. By default, all
 *                livings are marked as humanoid and then in /std/creature
 *                we unmark them as such by masking this function.
 * Returns      : int - 1.
 */
public int
query_humanoid()
{
    return 1;
}

/*
 * Function name: query_met
 * Description  : Tells if we know a certain living's name. As NPC's always
 *                know everyone, this function is masked in the player object
 *                for true met/nonmet behaviour for players (if defined).
 * Arguments    : mixed name - the name or objectpointer to the living we
 *                             are supposed to have met.
 * Returns      : int 1 - always as NPC's always know everyone.
 */
public int
query_met(mixed name)
{
    return 1;
}

/*
 * Function name:   notmet_me
 * Description:     Finds out if obj is considered to have met me. Players
 *                  must have been introduced to me. All others don't have to
 *                  be introduced to know me.
 * Arguments:       obj: Object in question, has it met me?
 * Returns:         True if object has not met me.
 */
public int
notmet_me(object obj)
{
#ifdef MET_ACTIVE
    if (obj && query_interactive(obj))
	return !obj->query_met(this_object());
#else
    return !this_player()->query_met(this_object());
#endif MET_ACTIVE
}

/*
 * Function name:   query_real_name
 * Description:     Returns the lowercase name of this living.
 *                  E.g.: "fatty".
 * Returns:         The name
 */
public string
query_real_name()
{
    return lower_case(::query_name());
}

/*
 * Function name:   query_name
 * Description:     Returns the capitalized name of this living.
 *                  E.g.: "Fatty".
 * Returns:         The name
 */
public string
query_name()
{
    return capitalize(::query_name());
}

/*
 * Function name:   query_met_name
 * Description:     Returns the name of this living, or "ghost of name" if
 *                  this living is not quite so living. E.g.: "Fatty".
 * Returns:         The name
 */
public string
query_met_name()
{
    if (this_object()->query_ghost())
	return "ghost of " + query_name();
    else
	return query_name();
}

/*
 * Function name: query_nonmet_name
 * Description  : Gives the name of this living to players who have not met
 *                this living. E.g.: "big fat male gnome wizard" If no
 *                short description is set one will be generated from the
 *                first two adjectives, the gender and the face of the
 *                living. Non-humanoids will not have the gender desc and for
 *                humanoids it can be blocked with LIVE_I_NO_GENDER_DESC.
 * Returns      : string - the name of the living when unmet.
 */
public string
query_nonmet_name()
{
    string *adj, str;
    string gender;

    /* Espcially true for NPC's. If a short has been set, use it. */
    if (query_short())
	return ::short();

    /* Get the gender description only if needed. */
    if (query_humanoid() &&
	!query_prop(LIVE_I_NO_GENDER_DESC) &&
	(this_object()->query_gender() != G_NEUTER))
	gender = this_object()->query_gender_string() + " ";
    else
	gender = "";

    if (sizeof((adj = this_object()->query_adj(1))) > 0)
	str = implode(adj[..1], " ") + " " + gender +
	    this_object()->query_race_name() +
	    (query_wiz_level() ? (" " + LD_WIZARD) : "");
    else
	str = gender + this_object()->query_race_name() +
	    (query_wiz_level() ? (" " + LD_WIZARD) : "");

    if (query_ghost())
	str += " " + LD_GHOST;

    return str;
}

/*
 * Function name:   query_Met_name
 * Description:     Returns the capitalized name of this living, prepended
 *                  with "Ghost of" if the living is not that living at all.
 *                  E.g.: "Ghost of Fatty".
 * Returns:         The capitalized name of the living when met.
 */
public string
query_Met_name()
{
    return capitalize(query_met_name());
}

/*
 * Function name:   query_art_name
 * Description:	    Gives the name with a prefix article when the object
 *                  has not met this living.
 *                  E.g.:
 *                        when met:   "Fatty",
 *                        when unmet: "a big fat gnome wizard".
 * Arguments:       pobj: The object that wants to know the name.
 *                  If pobj is undefined, it is assumed that the call has
 *                  been done through a protected vbfc
 * Returns:         Name prefixed with article.
 */
public varargs string
query_art_name(object pobj)
{
    string pre, aft;

    pre = ""; aft = "";
    if (!objectp(pobj))
	pobj = previous_object(-1);
    if (!CAN_SEE(pobj, this_object()) || !CAN_SEE_IN_ROOM(pobj))
	return LD_SOMEONE;
    if (query_prop(OBJ_I_INVIS) > 0)
    {
	pre = "(";
	aft = ")";
    }
    else if (query_prop(OBJ_I_HIDE) > 0)
    {
	pre = "[";
	aft = "]";
    }

#ifdef MET_ACTIVE
    if (notmet_me(pobj))
	return pre +  LANG_ADDART(query_nonmet_name()) + aft;
    else
#endif
	return pre + query_met_name() + aft;
}

/*
 * Function name:   query_Art_name
 * Description:	    Gives the name with a capitalized prefix article when the
 *                  calling object has not met this living.
 *                  E.g.:
 *                        when met:   "Fatty",
 *                        when unmet: "A big fat gnome wizard".
 * Arguments:       pobj: The object that wants to know the name.
 *                  If pobj is undefined, it is assumed that the call has
 *                  been done through a protected vbfc
 * Returns:         Name prefixed with capitalized article.
 */
public varargs string
query_Art_name(object pobj)
{
    string desc = query_art_name(pobj);

    /* Capitalize the right character if the description starts with ( or [. */
    if (wildmatch("[\\[(]*", desc))
        return desc[..0] + capitalize(desc[1..]);

    return capitalize(desc);
}

/*
 * Function name:   query_art_possessive_name
 * Description:	    Gives the possessive form of the name with a prefix
 &                  article when the calling object has not met this living.
 *                  E.g.:
 *                        when met:   "Fatty's",
 *                        when unmet: "a big fat gnome wizard's".
 * Arguments:       pobj: The object that wants to know the name.
 *                  If pobj is undefined, it is assumed that the call has
 *                  been done through a protected vbfc
 * Returns:         Possessive name prefixed with article.
 */
public varargs string
query_art_possessive_name(object pobj)
{
    return LANG_POSS(query_art_name(pobj));
}

/*
 * Function name:   query_Art_possessive_name
 * Description:	    Gives the possessive form of the name with a capitalized prefix
 &                  article when the  calling object has not met this living.
 *                  E.g.:
 *                        when met:   "Fatty's",
 *                        when unmet: "A big fat gnome wizard's".
 * Arguments:       pobj: The object that wants to know the name.
 *                  If pobj is undefined, it is assumed that the call has
 *                  been done through a protected vbfc
 * Returns:         Possessive name prefixed with capitalized article.
 */
public varargs string
query_Art_possessive_name(object pobj)
{
    return LANG_POSS(query_Art_name(pobj));
}

/*
 * Function name:   query_the_name
 * Description:	    Gives the name preceded by "the" when the object
 *                  has not met this living.
 *                  E.g.:
 *                        when met:   "Fatty",
 *                        when unmet: "the big fat gnome wizard".
 * Arguments:       pobj: The object that wants to know the name.
 *                  If pobj is undefined, it is assumed that the call has
 *                  been done through a protected vbfc
 * Returns:         Name prefixed with "the".
 */
public varargs string
query_the_name(object pobj)
{
    string pre = "", aft = "";

    if (!objectp(pobj))
	pobj = previous_object(-1);

    if (!CAN_SEE(pobj, this_object()) || !CAN_SEE_IN_ROOM(pobj))
	return LD_SOMEONE;
    if (query_prop(OBJ_I_HIDE) > 0)
    {
	pre = "[";
	aft = "]";
    }
    if (query_prop(OBJ_I_INVIS) > 0)
    {
	pre = "(";
	aft = ")";
    }

#ifdef MET_ACTIVE
    if (notmet_me(pobj))
	return pre + LD_THE + " " + query_nonmet_name() + aft;
    else
#endif
	return pre + query_met_name() + aft;
}

/*
 * Function name:   query_The_name
 * Description:	    Gives the name preceded by "The" when the calling object
 *                  has not met this living.
 *                  E.g.:
 *                        when met:   "Fatty",
 *                        when unmet: "The big fat gnome wizard".
 * Argument:        pobj: The object that wants to know the name.
 *                  If pobj is undefined, it is assumed that the call has
 *                  been done through a protected vbfc
 * Returns:         Name prefixed with "The".
 */
public varargs string
query_The_name(object pobj)
{
    string desc = query_the_name(pobj);

    /* Capitalize the right character if the description starts with ( or [. */
    if (wildmatch("[\\[(]*", desc))
        return desc[..0] + capitalize(desc[1..]);

    return capitalize(desc);
}

/*
 * Function name:   query_the_possessive_name
 * Description:	    Gives the possessive form of the name preceded by "the"
 *                  when the calling object  has not met this living.
 *                  E.g.:
 *                        when met:   "Fatty's",
 *                        when unmet: "the big fat gnome wizard's".
 * Argument:        pobj: The object that wants to know the name.
 *                  If pobj is undefined, it is assumed that the call has
 *                  been done through a protected vbfc
 * Returns:         Possessive name prefixed with "the".
 */
public varargs string
query_the_possessive_name(object pobj)
{
    return LANG_POSS(query_the_name(pobj));
}

/*
 * Function name:   query_The_possessive_name
 * Description:	    Gives the possessive form of the name preceded by "The"
 *                  when the calling object  has not met this living.
 *                  E.g.:
 *                        when met:   "Fatty's",
 *                        when unmet: "The big fat gnome wizard's".
 * Argument:        pobj: The object that wants to know the name.
 *                  If pobj is undefined, it is assumed that the call has
 *                  been done through a protected vbfc
 * Returns:         Possessive name prefixed with "The".
 */
public varargs string
query_The_possessive_name(object pobj)
{
    return LANG_POSS(query_The_name(pobj));
}

/*
 * Function name: query_exp_title
 * Description  : Returns "wizard" if this living is a wizard, or else
 *                calculates a title from the stats.
 * Returns      : string - the title.
 */
public string
query_exp_title()
{
    if (query_wiz_level())
	return LD_WIZARD;
 
    return GET_EXP_LEVEL_DESC(this_object()->query_average_stat());
}

/*
 * Function:    query_presentation
 * Description: Gives a presentation of the living in one line. Including
 *              Name, Race, Guild titles, Alignment and Experience level
 *              This should only be displayed to met players.
 *              E.g.: "Fatty the donut-fan, wizard, male gnome (eating)"
 * Returns:     The presentation string
 */
public string
query_presentation()
{
    string a, b, c;
    
    a = query_title(); 
    b = this_object()->query_exp_title(); 
#ifndef NO_ALIGN_TITLE
    c = this_object()->query_al_title();
#endif

    /* If there is no (guild) title, use the experience title as title. */
    if (!strlen(a) && strlen(b))
    {
        b = "the " + implode(map(explode(b, " "), capitalize), " ");
    }

    return query_name() +
	(strlen(a) ? (" " + a + ",") : "") +
	(strlen(b) ? (" " + b + ", ") : " ") +
	this_object()->query_gender_string() + " " + 
        this_object()->query_race_name()
#ifndef NO_ALIGN_TITLE
	+ (strlen(c) ? (" (" + c + ")") : "")
#endif
	; /* Oke, it is ugly to have a semi-colon on a separate line. */
}

/*
 * Function name:   short
 * Description:     Returns the short-description of this living, for the
 *                  object given. Handles invisibility and met/nonmet.
 * Returns:         The short string.
 */
public varargs string
short(object for_obj)
{
    string desc;
    string extra;

    if (!objectp(for_obj))
	for_obj = this_player();

#ifdef MET_ACTIVE
    if (notmet_me(for_obj))
    {
	desc = this_object()->query_nonmet_name();
#ifdef STATUE_WHEN_LINKDEAD
	if (is_linkdead)
	    desc = STATUE_DESC + " " + LANG_ADDART(desc);
#endif STATUE_WHEN_LINKDEAD
    }
    else
#endif MET_ACTIVE
    {
	desc = this_object()->query_met_name();
#ifdef STATUE_WHEN_LINKDEAD
	if (is_linkdead)
	    desc = STATUE_DESC + " " + desc;
#endif STATUE_WHEN_LINKDEAD
    }

    if (strlen(extra = query_prop(LIVE_S_EXTRA_SHORT)))
	return (desc + extra);

    return desc;
}

/*
 * Function name: vbfc_short
 * Description:   Gives short as seen by previous object
 * Returns:	  string holding short()
 */
public string
vbfc_short()
{
    object for_obj;

    for_obj = previous_object(-1);
    if (!this_object()->check_seen(for_obj))
	return LD_SOMEONE;

    return this_object()->short(for_obj);
}

/*
 * Function name:   num_scar
 * Description:     Get the number of scars a player has
 * Returns:         int holding number of scars
 */
public int
num_scar()
{
    int i, j, n,scar;

    j = 1; n = 0; scar = this_object()->query_scar();

    while (i < F_MAX_SCAR)
    {
    	if (scar & j) n++;
    	j *= 2; i++;
    }
    return n;
}

/*
 * Function name:   desc_scar
 * Description:     Get the composite string holding the scar description
 * Returns:         string holding scar description.
 */
public string
desc_scar()
{
    int i, j, scar;
    string *scar_desc, *my_scars;

    scar_desc = F_SCAR_DESCS;
    scar = this_object()->query_scar();
    j = 1;
    my_scars = 0;
    while(i < F_MAX_SCAR)
    {
	if (scar & j)
	{
	    if (my_scars)
		my_scars = my_scars + ({ scar_desc[i] });
	    else
		my_scars = ({ scar_desc[i] });
	}
	j *= 2;
	i += 1;
    }

    if (!my_scars) return 0;

    return COMPOSITE_WORDS(my_scars);
}

/*
 * Function name:   show_scar
 * Description:     Finds out which scars this living has.
 * Returns:         A string with the scars.
 */
string
show_scar(mixed for_obj)
{
    int num;

    num = num_scar();

    if (!num)
	return "";

    if (!objectp(for_obj))
	for_obj = this_player();

    if (for_obj == this_object())
	return LD_YOUR_SCARS(num, desc_scar()) + ".\n";

    return capitalize(this_object()->query_pronoun()) + LD_HAS_SCARS(num) +
        " " + this_object()->query_possessive() + " " + desc_scar() + ".\n";
}

/*
 * Function name:   long
 * Description:     Returns the long-description of this living, and shows also
 *                  the inventory. Handles invisibility and met/nonmet. Note
 *                  that the function does not do any writes! It only returns
 *                  the correct string.
 * Arguments:       string for_obj: return an item-description
 *                  object for_obj: return the living-description
 * Returns:         The description string
 */
public varargs string
long(mixed for_obj)
{
    string          cap_pronoun, res;
    object 	    eob;

    if (stringp(for_obj))   /* Items */
	return ::long(for_obj);

    if (!objectp(for_obj))
	for_obj = this_player();

    if (for_obj == this_object())
	res = LD_PRESENT_YOU(this_object());
    else
    {
	res = query_long();
	if (!stringp(res) &&
	    !functionp(res))
	{
	    cap_pronoun = capitalize((string)this_object()->query_pronoun());
	    if (!notmet_me(for_obj))
		res = LD_PRESENT_TO(this_object());
	    else if (!(this_object()->short(for_obj)))
		return "";
	    else
		res = cap_pronoun + " is " +
			LANG_ADDART(this_object()->short(for_obj)) + ".\n";

	    if (this_object()->query_ghost())
	    {
#ifdef MET_ACTIVE
		if (notmet_me(for_obj))
		    return LD_NONMET_GHOST(this_object());
		else
#endif
		    return LD_MET_GHOST(this_object());
	    }
	}
	else
	    res = check_call(res);
    }

    res += this_object()->show_scar(for_obj);

    return res + this_object()->show_sublocs(for_obj);
}

/*
 * Function name: describe_combat
 * Description  : This function describes the combat that is going on in
 *                the room this_object() is in. It is an internal function
 *                and should only be called internally by do_glance.
 * Arguments    : object *livings - the livings in the room.
 */
static void
describe_combat(object *livings)
{
    int     index;
    string  text = "";
    string  subst = "";
    object  victim;
    mapping fights = ([ ]);

    livings = filter(livings, objectp);

    /* Sanity check. No need to print anything if there aren't enough
     * people to actually fight. Note that if there is only one living, it
     * is possible that we fight that living.
     */
    if (sizeof(livings) < 2)
	return;

    /* First compile a mapping of all combats going on in the room.
     * The format is the victim as index and an array of all those
     * beating on the poor soul as value. Add this_object() to the
     * list since it isn't in there yet.
     */
    livings += ({ this_object() });
    foreach(object obj: livings)
    {
	/* Only if the living is actually fighting. */
	if (objectp(victim = obj->query_attack()))
	{
	    if (pointerp(fights[victim]))
		fights[victim] += ({ obj });
	    else
		fights[victim] = ({ obj });
	}
    }

    /* No combat going on. */
    if (!m_sizeof(fights))
	return;

    /* First we describe the combat of the player him/herself. This will
     * be a nice compound message. Start with 'outgoing' combat.
     */
    if (objectp(victim = this_object()->query_attack()))
    {
	fights[victim] -= ({ this_object() });

	/* Victim is fighting back. */
	if (victim->query_attack() == this_object())
	{
	    text = "You are in combat with " +
		victim->query_the_name(this_object());
	    fights[this_object()] -= ({ victim });
	    subst = " also";
	}
	else
	    text = "You are fighting " +
		victim->query_the_name(this_object());

	/* Other people helping us attacking the same target. */
	if (sizeof(fights[victim]))
	    text += ", assisted by " + FO_COMPOSITE_ALL_LIVE(fights[victim],
							     this_object());
	m_delkey(fights, victim);

	/* Other people hitting on me. */
	if (index = sizeof(fights[this_object()]))
	    text += ", while " + FO_COMPOSITE_ALL_LIVE(fights[this_object()],
						       this_object()) + 
		((index == 1) ? " is" : " are") +
		subst + " fighting you";
	text += ".\n";
    }
    /* If we aren't fighting, someone or something may be fighting us. */
    else if (index = sizeof(fights[this_object()]))
    {
	text = capitalize(FO_COMPOSITE_ALL_LIVE(fights[this_object()],
	    this_object())) + ((index == 1) ? " is" : " are") +
	    " fighting you.\n";
    }

    /* Now generate messages about the other combat going on. This will
     * not be as sophisticated as the personal combat, but it will try to
     * to circumvent printing two lines of 'a fights b' and 'b fights a'
     * since I think that is a silly way of putting things.
     */
    m_delkey(fights, this_object());
    livings = m_indices(fights);
    foreach(object obj: livings)
    {
	/* Victim is fighting (one of his) attackers. */
	if (objectp(victim = obj->query_attack()) &&
	    IN_ARRAY(victim, fights[obj]))
	{
	    fights[obj] -= ({ victim });

            if (pointerp(fights[victim]))
                fights[victim] -= ({ obj });

	    /* Start with the the name of one of the fighters. */
	    text += obj->query_The_name(this_object());

	    /* Then the people helping the first combatant. */
	    if (sizeof(fights[victim]))
	    {
		text += ", with the assistance of " +
		    FO_COMPOSITE_ALL_LIVE(fights[victim], this_object()) + ",";
		m_delkey(fights, victim);
	    }

	    /* Then the second living in the fight. */
	    text += " and " + victim->query_the_name(this_object());

	    /* And the helpers on the other side. */
	    if (sizeof(fights[obj]))
		text += ", aided by " +
		    FO_COMPOSITE_ALL_LIVE(fights[obj], this_object());

           text += " are fighting each other.\n";
	}
	else if (sizeof(fights[obj]))
	{
	    text += capitalize(FO_COMPOSITE_ALL_LIVE(fights[obj], this_object())) +
		((sizeof(fights[obj]) == 1) ? " is" : " are") +
		" fighting " +
		obj->query_the_name(this_object()) + ".\n";
	}

	m_delkey(fights, obj);
    }

    write(text);
}
 
/*
 * Function name: do_glance
 * Description  : This is the routine describing rooms to livings. It will
 *                print the long or short description of the room, the
 *                possible exits, all visible non-living and living objects
 *                in the room (but not this_object() itself) and then it
 *                will print possible attacks going on. Note that this
 *                function performs a write(), so this_player() will get the
 *                descriptions printed to him/her.
 * Arguments    : int brief - if true, write the short-description,
 *                            else write the long-description.
 * Returns      : int 1 - always.
 */
public int
do_glance(int brief)
{
    object env;
    string item;

    /* Don't waste the long description on NPC's. */
    if (!interactive(this_object()))
	return 0;

    /* Wizard gets to see the filename of the room we enter and a flag if
     * there is WIZINFO in the room.
     */
    env = environment();
    if (query_wiz_level())
    {
	if (stringp(env->query_prop(OBJ_S_WIZINFO)))
	    write("Wizinfo ");

	write(file_name(env) + "\n");
    }

    /* It is dark. */
    if (!CAN_SEE_IN_ROOM(this_object()))
    {
 	if (!stringp(item = env->query_prop(ROOM_S_DARK_LONG)))
 	    write(LD_DARK_LONG);
 	else
 	    write(item);
	return 1;
    }

    /* Describe the room and its contents. */
#ifdef DAY_AND_NIGHT
    if (!env->query_prop(ROOM_I_INSIDE) && 
	((HOUR > 21) ||
	 (HOUR < 5)) &&
	((env->query_prop(OBJ_I_LIGHT) +
	 query_prop(LIVE_I_SEE_DARK)) < 2))
	write(LD_IS_NIGHT(env));
    else
#endif
    {
	if (brief)
  	{
	    write(capitalize(env->short()) + ".\n");
	    write(env->exits_description());
	}
	else
	    write(env->long());
    }

    env->show_visible_contents(this_object());

    /* Give a nice description of the combat that is going on. */
    describe_combat(FILTER_LIVE(all_inventory(env)));

    return 1;
}

/*
 * Function name: set_appearance_offset
 * Description:   The appearance offset affect how this living appears, 
 *                independent of who the watcher is. (The normal appearance
 *		  is relative to the watcher's opinion.)
 *                A negative offset makes the living more beautiful.
 *                A reasonable value of the offset is within +-50
 * Arguments:     off: the offset
 */
void
set_appearance_offset(int off)
{
    appearance_offset = off;
}

/*
 * Function name: query_appearance_offset
 * Description:   The appearance offset affect how this living appears, 
 *                independent of who the watcher is. (The normal appearance
 *		  is relative to the watcher's opinion.)
 *                A negative offset makes the living more beautiful.
 *                A reasonable value for the offset is within +-50
 * Returns:       The offset
 */
int
query_appearance_offset()
{
    return appearance_offset;
}

/*
 * Function name: my_opinion
 * Description:   Return my opinion about the appearance of an other
 *                living creature. -1 if not living, 0 - 100 if living.
 *                0 = perfectly in taste, 100 = the most ugly you can imagine
 * Arguments:     ob: the object to give an opinion about
 * Returns:       The opinion as int.
 */
public int
my_opinion(object ob)
{
    int op, op_offset;

    if (!living(ob))
	return -1;
    op = query_opinion() - ob->query_appearance();
    op = op > 0 ? op : - op;
    op = op < 50 ? op : 100 - op;

    op_offset = ob->query_appearance_offset();
    if (op_offset)
	op += op_offset;
    else
	if (query_race_name() != ob->query_race_name())
	    op += 10;

    if (query_race_name() == ob->query_race_name() &&
	query_gender() != ob->query_gender())
	op -= 5;
    if (query_intoxicated() > 0)
	op -= query_intoxicated() / 4;
    op *= 2;

    return (op < 0 ? 0 : (op > 99 ? 99 : op));
}

/*
 * Function name: appraise_object
 * Description:   This function is called when someone tries to appraise a
 *		  living object.
 * Arguments:     num - use this number instead of the skill.
 */
public void
appraise_object(int num)
{
    write("\n" + this_object()->long(this_player()) + "\n");
    write(break_string(LD_APPRAISE(appraise_weight(num), appraise_volume(num)),
	75) + "\n");
}

/*
 * Function name: query_align_text
 * Description:	  Returns the alignment text for this object.
 * Returns:	  The alignment text.
 */
string
query_align_text()
{
    int a, prc;
    string t, *names;

    a = this_object()->query_alignment();

    if (a < 0)
    {
	prc = (100 * (-a)) / (F_KILL_NEUTRAL_ALIGNMENT * 100);
	names = SD_EVIL_ALIGN;
    }
    else
    {
	prc = (100 * a) / (F_KILL_NEUTRAL_ALIGNMENT * 100);
	names = SD_GOOD_ALIGN;
    }

    a = (sizeof(names) * prc) / 100;

    if (a >= sizeof(names))
	a = sizeof(names) - 1;

    return names[a];
}
 
/*
 * Function name: show_hook
 * Description  : This hook is called whenever an item is 'snown' to this
 *                living. It is shown by this_player().
 * Arguments    : object ob - the object shown.
 */
public void
show_hook(object ob)
{
    return;
}

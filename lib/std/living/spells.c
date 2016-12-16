/* 
 * /std/living/spells.c
 */

#include <living_desc.h>

static object *spell_objs;      /* The list of spell objects */
static string current_spell;	/* The spell being cast. */
static int    aid;              /* Alarm id for spells */
static object current_spellob;  /* Spell object for current spell */

static void cast_spell(string spell, string sparg, object spellob);

static void
spells_reset()
{
    spell_objs = ({});
}

/*
 * Function name: add_spellobj
 * Description:   Add a spell object to the list of spell objects.
 * Arguments:     obj - The object to add.
 */
public void
add_spellobj(object obj)
{
    spell_objs += ({ obj });
}

/*
 * Function name: remove_spellobj
 * Description:   Remove a spell object from the list of spell objects
 * Arguments:     obj - the object to remove
 */
public void
remove_spellobj(object obj)
{
    spell_objs -= ({ obj });
}

/*
 * Function name: query_spellobjs
 * Description:   return the spellobj list.
 */
public object *
query_spellobjs()
{
    spell_objs = filter(spell_objs, objectp);

    return secure_var(spell_objs); 
}

public object
find_spell(string spell)
{
    int i;
    for (i = 0; i < sizeof(spell_objs); i++)
    {
        if (spell_objs[i]->exist_command(spell))
	{
            return spell_objs[i];
	}
    }

    return 0;
}

/*
 * Function name: start_spell
 * Description:   Initiate the casting of a spell
 * Arguments:     string spell - the spell being cast
 *                mixed  arg   - spell invokation arguments
 *                object spellob - the spell object
 * Returns:       1/0 - spell successful / unsuccessful
 */
public int
start_spell(string spell, mixed arg, object spellob)
{
    int t;
    string mess;
    if (aid && get_alarm(aid))
    {
        write("You are already concentrating on a spell.\n");
        return 0;
    }

    /* A call to 'start_spell_fail' indicates that we are
       preparing for casting the spell. This can be used to
       deduct mana. If the spell is broken mana will still be lost.
     */
    if (spellob->start_spell_fail(spell, arg))
    {
        return 0;
    }

    if (!spellob->query_spell_mess(spell, arg))
    {
        write("You start to concentrate upon the spell.\n");
        mess = " closes " + query_possessive() + " eyes and " +
                "concentrates.";
        say(({ METNAME + mess + "\n",
            TART_NONMETNAME + mess + "\n", "" }));
    }

    t = spellob->query_spell_time(spell, arg);
    aid = set_alarm(itof(t), 0.0, &cast_spell(spell, arg, spellob));

    current_spell = spell;
    current_spellob = spellob;

    return 1;
}

/*
 * Function name: cast_spell
 * Description:   Execute a spell
 * Arguments:     string spell - the name of the spell being cast
 *                mixed  sparg - arguments to spell invokation
 *                object spellob - the spell object  
 */
static void
cast_spell(string spell, mixed sparg, object spellob)
{
    mixed fail;

    current_spellob = current_spell = 0;

    set_this_player(this_object());

    if (spellob)
    {
	if (stringp(fail = spellob->do_command(spell, sparg)))
	{
            write(fail);
            return;
	}

	if (intp(fail) && (fail == 1))
	{
	    return;
	}
    }

    write(LD_SPELL_FAIL);
}

/*
 * Function name: break_spell
 * Description:   Break concentration on a spell if one is active
 * Arguments:     string msg - why concentration was broken
 *                object breaker - who broke the concentration
 * Returns:       1/0 - concentration broken/not broken
 */
varargs public int
break_spell(string msg, object breaker)
{
    if (aid && get_alarm(aid))
    {
	remove_alarm(aid);
	aid = 0;
	this_object()->remove_prop(LIVE_I_CONCENTRATE);

	if (!strlen(msg))
	{
	    tell_object(this_object(), LD_SPELL_CONC_BROKEN);
	}
	else
	{
	    this_object()->catch_msg(msg);
	}

        current_spellob->break_spell(current_spell, breaker);
        current_spellob = current_spell = 0;

        return 1;
    }

    return 0;
}

/*
 * Function Name: interrupt_spell
 * Description  : Attempt to interrupt a spell cast.
 *                This currently has a very small chance of breaking the
 *                the casters concentration.
 * Returns      : 1 / 0 - a spell was interrupted / not interrupted
 */
public int
interrupt_spell()
{
    if (!aid || !get_alarm(aid))
        return 0;
    
    if (random(100) < 1)
    {
        break_spell();
        return 1;
    }

    current_spellob->interrupt_spell(current_spell);
    return 1;
}

/*
 * Function name: abort_spell
 * Description:   Willingly abort  concentration on a spell if one is active
 * Arguments:     string msg - why the spell was aborted
 * Returns:       1/0 - a spell was aborted/not aborted
 */
varargs public int
abort_spell(string msg)
{
    if (aid && get_alarm(aid))
    {
        remove_alarm(aid);
        aid = 0;
 
        this_object()->remove_prop(LIVE_I_CONCENTRATE);
     
        if (!strlen(msg))
	{
            tell_object(this_object(), "Ok.\n");
        }
        else
	{
            tell_object(this_object(), msg);
	}

        current_spellob->abort_spell(current_spell);

        current_spellob = current_spell = 0;
 
        return 1;
    }

    return 0;
}

/*
 * Function name: query_spell
 * Description:   Get the name of the spell being cast (if any)
 * Returns:       The spell name
 */
public string
query_spell()
{
    return current_spell;
}

/*
 * Function name: query_spell_object
 * Description:   Get the current spell object (if any)
 * Returns:       The spell object responsible for the spell being cast
 */
public object
query_spell_object()
{
    return current_spellob;
}

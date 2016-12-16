/*
 * /d/Standard/lib/intro.c
 *
 * Inherit this in an npc to have it introduce itself.
 */

#pragma strict_types

#include <macros.h>
#include <stdproperties.h>

int intro_delay = 0;

/*
 * Function name: query_intro_delay
 * Description  : Returns the present intro-delay value.
 * Returns      : int - the delay in seconds.
 */
int
query_intro_delay()
{
    return intro_delay;
}

/*
 * Function name: set_intro_delay
 * Description  : Call this to set a delay in the reaction to an
 *                introduction. The delay must be at least 3 seconds.
 * Arguments    : int d - the delay in seconds.
 */
void
set_intro_delay(int d)
{
    if (d >= 3)
    {
        intro_delay = d;
    }
}

/*
 * Function name: greetings 
 * Description:   This function makes the npc do a greeting to people it
 *                already know and to fellow npc's. It is possible to
 *                redefine, not however that it should never contain an
 *                'introduce myself' command.
 * Arguments:     object who - the living that introduced to me
 *
 */
public void
greet(object who)
{
    command("bow " + OB_NAME(who));
    command("say Greetings " + who->query_name() + "!");
}

/*
 * Function name:       introduce
 * Description:         This function makes the npc do an introduction to a
 *                      player that has not been introed to it earlier. The
 *                      function may be redefined to create varity.
 * Arguments            object who - the livint that introduced tome
 *
 */
public void
introduce(object who)
{
    command("introduce myself to " + OB_NAME(who));
    command("say Well met " + who->query_name() + "!");
} 


/*
 * Function name: react_to_introduction
 * Description  : Called to actually react to an introduction.
 * Arguments    : string name - the name of the person being introduced.
 */
void
react_to_introduction(string name)
{
    object who = find_player(name);

    if (!who || !present(who, environment(this_object())))
    {
        return;
    }

    if (this_object()->query_prop(LIVE_I_NEVERKNOWN) ||
        who->query_met(this_object()))
    {
        greet(who);
    }
    else
    {
        introduce(who);
    }
}

/*
 * Function name: add_introduced
 * Description  : Called automatically from the mudlib whenever a person
 *                introduces himself to this NPC.
 * Arguments    : string name - the name of the person being introduced.
 */
void
add_introduced(string name)
{
    if (intro_delay)
    {
        set_alarm(itof(intro_delay), 0.0, &react_to_introduction(name));
    }
    else
    {
        react_to_introduction(name);
    }
}

/*
 * /d/Standard/lib/trap.c
 *
 * A module for creating trapped items.
 */

#pragma strict_types

#include <stdproperties.h>
#include <cmdparse.h>
#include <macros.h>
#include <ss_types.h>

static int trapped = 1;
static int trap_level = 50;
static string trap_desc;

/*
 * Function name: set_trapped
 * Description:   Enable the trap
 * Arguments:     int t - true: trap enabled
 *                        false: trap disabled
 */
public void
set_trapped(int t)
{
    trapped = t;
}

/*
 * Function name: query_trapped
 * Description:   Find out if the trap is enabled
 * Returns:       1/0 - trap enabled/disabled
 */
public int
query_trapped()
{
    return trapped;
}

/*
 * Function name: set_trap_level
 * Description:   Indicate how hard this trap is to notice and disarm
 * Arguments:     int l - the trap's level
 */
public void
set_trap_level(int l)
{
    trap_level = l;
}

/*
 * Function name: query_trap_level
 * Returns:       The trap's difficulty level
 */
public int
query_trap_level()
{
    return trap_level;
}

/*
 * Function name: set_trap_desc
 * Description:   Set the trap's description
 * Arguments:     string desc - a string describing the trap
 */
public void
set_trap_desc(string desc)
{
    trap_desc = desc;
}

/*
 * Function name: query_trap_desc
 * Returns:       a string description of the trap
 */
public string
query_trap_desc()
{
    return trap_desc;
}

/*
 * Function name: trap_desc
 * Description:   Checks to see if the onlooker notices the trap
 *                and returns a string describing the trap if so.
 * Returns:       The trap description or "" if it is not trapped
 *                or the trap isn't noticed.
 */
public string
trap_desc(object for_obj = this_player())
{
    int skill;
    string desc;

    if (!query_trapped())
    {
        return "";
    }
    
    skill = this_player()->query_skill(SS_FR_TRAP);
    skill = (4 * skill / 5) + random(skill);

    if ((skill - trap_level) > 0)
    {
        if (trap_desc)
        {
            return trap_desc;
        }

        desc = this_object()->real_short(for_obj);
	return "The " + (desc ? desc : this_object()->short(for_obj)) + 
            " seems to be trapped!\n";
    }

    return "";
}

/*
 * Function name: spring_trap
 * Description:   This function is called when a trapped container
 *                has been sprung.  You should redefine this to provide
 *                your own behaviour.
 * Arguments:     object player - the object that sprung the trap
 */
void
spring_trap(object player)
{
    set_trapped(0);
}

/* 
 * Function:    do_disarm
 * Description: Attempt to disarm a trapped container.
 * Arguements:  str: what we are trying to disarm.
 * Returns:     0/1 - syntax failure/success
 */
int
do_disarm(string str)
{
    mixed *items;
    string msg;
    int res;

    if (!CAN_SEE_IN_ROOM(this_player()))
    {
        msg = environment(this_player())->query_prop(ROOM_S_DARK_MSG);
        notify_fail((msg ? msg : "It is too dark to ") + "see.\n");
        return 0;
    }

    if (!strlen(str) || !parse_command(str, all_inventory(this_player()) +
        all_inventory(environment(this_player())), 
        "[the]  'trap' 'on' [the] %i", items) ||
        !sizeof(items = NORMAL_ACCESS(items, 0, 0)))
    {
        notify_fail(capitalize(query_verb()) + " the trap on what?\n");
        return 0;
    }

    if (sizeof(items) > 1)
    {
        notify_fail(capitalize(query_verb()) + " the trap on what?  Be " +
            "more specific!\n");
        return 0;
    }

    if (this_player()->query_skill(SS_FR_TRAP) == 0)
    {
	write("You do not know how to disarm a trap.\n");
	return 1;
    }

    if (!(items[0]->query_trapped()))
    {
        msg = items[0]->real_short(this_player());
	write("Hmm the " + (msg ? msg : items[0]->short()) + 
	  " doesn't seem to be trapped.\n");
	return 1;
    }

    say(QCTNAME(this_player()) + " tries to disarm a trap.\n");

    if ((res = items[0]->disarm_trap(this_player())) == 1)
    {
	write("You manage to disarm the trap.\n");
        items[0]->set_trapped(0);
    }
    else if (res == 0)
    {
        write("You fumble your attempt, and spring the trap!\n");
        items[0]->spring_trap(this_player());
    }
    else
    {
        msg = items[0]->real_short(this_player());
        write("You didn't disarm the " + (msg ? msg : items[0]->short()) + 
	    " but luckily you didn't set the trap off in your attempt.\n");
    }

    return 1;      
}

/* 
 * Function name: disarm_trap
 * Description:   Test to see if the trap can be diarmed
 * Returns:       1 - trap can be disarmed
 *                0 - trap cannot be disarmed and was sprung in the attempt
 *               -1 - trap cannot be disarmed, but was not sprung
 */
public int
disarm_trap(object who)
{
    int skill = who->query_skill(SS_FR_TRAP);

    skill = (3 * skill / 4) + random(skill);

    if (skill >= trap_level)
    {    
        return 1;
    }

    return ((skill < (trap_level - 10)) ? 0 : -1);
}

/*
 * Function name: init_trap
 * Description:   Add trap-related commands.  Call this from the
 *                init() function of your trapped object.
 */
void
init_trap()
{
    add_action(do_disarm, "disarm");
}


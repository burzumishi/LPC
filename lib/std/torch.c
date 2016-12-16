/*
 * /std/torch.c
 *
 * The standard torch code.
 */

#pragma save_binary
#pragma strict_types

inherit "/std/object";
 
#include <cmdparse.h>
#include <composite.h>
#include <files.h>
#include <macros.h>
#include <stdproperties.h>

#define DECAY_TIME (300.0)
 
/*
 * Prototypes:
 */
public void set_strength(int strength);
public void set_value(int value);
public void set_time(int time);
public int torch_value();
public void burned_out();

/*
 * Global variables:
 */
private int Torch_Value,	/* The max value of the torch. */
            Light_Strength,	/* How strongly the 'torch' will shine */
            Time_Left;		/* How much time is left? */
static  int Burn_Alarm,		/* Alarm used when the torch is lit */
            Decay_Alarm,        /* Alarm used when the torch decays */
            Max_Time;		/* How much is the max time (start time) */

/*
 * Function name: create_torch
 * Description:   The standard create. This has some standard settings on
 *		  long and short descriptions, but don't be afraid to change
 *		  them.
 *                The pshort has to be set to some value, otherwise the short
 *                description of several objects will look ugly.
 */
void
create_torch()
{
    set_name("torch");
    set_pname("torches");
    set_short("torch");
    set_pshort("torches");
}

/*
 * Function name: create_object
 * Description:   The standard create routine.
 */
nomask void
create_object()
{
    add_prop(OBJ_I_LIGHT,     0);
    add_prop(OBJ_I_WEIGHT,  700);
    add_prop(OBJ_I_VOLUME, 1000);
    add_prop(OBJ_I_VALUE,  torch_value);

    set_time(300);
    set_strength(1);
    set_value(40);

    create_torch();
}

/*
 * Function name: reset_torch
 * Description  : Since you may not mask reset_object() in torches, you have
 *                to redefine this function. Use enable_reset() to make the
 *                torch actually reset itself.
 */
public void
reset_torch()
{
}

/*
 * Function name: reset_object
 * Description:   Reset the object. Since this function is nomasked, you
 *                must redefine reset_torch() to make the torch reset.
 */
public nomask void
reset_object()
{
    reset_torch();
}

/*
 * Function name: torch_value
 * Description:   A VBFC gets here when someone wants to know the value of
 *		  the value of this object, default setting
 * Returns:	  The value
 */
public int
torch_value()
{
    int v;

    if (!Max_Time)
    	return 0;

    if (Burn_Alarm && sizeof(get_alarm(Burn_Alarm)))
	v = ftoi(get_alarm(Burn_Alarm)[2]);
    else
	v = Time_Left;

    return (v * (Torch_Value - 5)) / Max_Time + 5;
}
 
/*
 * Function name: short, pshort, long
 * Description:   We change the short, plural short and long description of
 *		  the torch if it's lit, default settings.
 * Returns:	  The description.
 */
public varargs string
short(object for_obj)
{
    string tmp = ::short(for_obj);
   
    if (Burn_Alarm)
    {
	return "burning " + tmp;
    }
    if (!Time_Left)
    {
	return "burnt out " + tmp;
    }

    return tmp;
}

/*
 * Function name: short, pshort, long
 * Description:   We change the short, plural short and long description of
 *		  the torch if it's lit, default settings.
 * Returns:	  The description.
 */
public varargs string
plural_short(object for_obj)
{
    string tmp = ::plural_short(for_obj);

    if (!stringp(tmp))
    {
	return 0;
    }
    if (Burn_Alarm)
    {
	return "burning " + tmp;
    }
    if (!Time_Left)
    {
	return "burnt out " + tmp;
    }

    return tmp;
}

/*
 * Function name: short, pshort, long
 * Description:   We change the short, plural short and long description of
 *		  the torch if it's lit, default settings.
 * Returns:	  The description.
 */
public varargs string
long(object for_obj)
{
    string tmp = ::long(for_obj);
    
    if (Burn_Alarm)
    {
	return tmp + "It is lit.\n";
    }
    if (!Time_Left)
    {
	return tmp + "It is burnt out.\n";
    }

    return tmp;
}

/*
 * Function name: set_time
 * Description:   Set how long time the torch can burn.
 * Arguments:     int t - the time in seconds.
 */
public void
set_time(int t)
{
    Max_Time = t;
    Time_Left = t;
}

/*
 * Function name: query_max_time
 * Description:	  Query the original burn time of the torch.
 * Returns:       the time
 */
public int
query_max_time()
{
    return Max_Time;
}

/*
 * Function name: query_time
 * Description:	  Query how long time the torch can burn
 * Argument:      flag: if true, then return the time until the
 *                torch burns out if the torch is lit
 * Returns:       int - the time left in seconds.
 */
public int
query_time(int flag = 0)
{
    mixed   alarm;

    if (flag && Burn_Alarm && sizeof(alarm = get_alarm(Burn_Alarm)))
	return ftoi(alarm[2]);
    return Time_Left;
}

/*
 * Function name: query_lit
 * Description:   Query of the torch is lit.
 * Argument:      flag - if set, return id of the alarm to the function
 *                that is called when the torch burns out.
 * Returns:       0        - if torch is not lit,
 *                -1       - if torch is lit,
 *                alarm id - if torch is lit and flag was set.
 */
public int
query_lit(int flag)
{
    if (flag)
	return Burn_Alarm;
    else
	return (!Burn_Alarm ? 0 : -1);
}

/*
 * Function name: set_time_left
 * Description:   Set how long time the torch can burn.
 *                Use this for 'torches' that can be refilled, like oil lamps.
 * Arguments    : int left - the time left.
 */
public void
set_time_left(int left)
{
    Time_Left = ((left < Max_Time) ? left : Max_Time);

    /* If lit, then also update the alarm. */
    if (Burn_Alarm)
    {
	remove_alarm(Burn_Alarm);
	Burn_Alarm = set_alarm(itof(Time_Left), 0.0, burned_out);
    }
}

/*
 * Function name: set_strength
 * Description:   Set the light strength of this 'torch'
 * Arguments:     int strength - the light value.
 */
public void
set_strength(int strength)
{
    Light_Strength = strength;
}

/*
 * Function name: query_strength
 * Description:   Query how strongly the torch will shine
 * Returns:       int - the light value.
 */
public int
query_strength()
{
    return Light_Strength;
}

/*
 * Function name: set_value
 * Description:   Set the max value of the torch (i.e. unused value).
 * Arguments:     int value - the max value.
 */
public void
set_value(int value)
{
    Torch_Value = value;
}

/*
 * Function name: query_torch_may_decay
 * Description  : This function will indicate whether the torch may decay or not
 *                after it burns out. By default it returns 1, but if you do not
 *                want to have it decay, you must redefine this function to make
 *                it return 0. This is especially true for oil lamps.
 * Returns      : int 1 - always.
 */
public int
query_torch_may_decay()
{
    return 1;
}

/*
 * Function name: light_me_after_delay
 * Description  : Routine that actually lights the torch after a delay. Why
 *                exactly this is used, I don't know. Maybe to let the
 *                previous command finish with the old light value?
 * Returns      : int 1 - no reason, really, since it's called from an alarm.
 */
public int
light_me_after_delay()
{
    remove_prop(TEMP_STDTORCH_CHECKED);
    add_prop(OBJ_I_LIGHT, Light_Strength);
    add_prop(OBJ_I_HAS_FIRE, 1);
    Burn_Alarm = set_alarm(itof(Time_Left), 0.0, burned_out);
    return 1;
}

/*
 * Function name: query_light_fail
 * Description  : Routine that returns why this torch cannot be lit.
 * Returns      : string - descriptive error message.
 *                int 0 - the torch can be lit.
 */
public string
query_light_fail()
{
    if (environment(this_player())->query_prop(ROOM_I_TYPE) ==
	ROOM_UNDER_WATER)
    {
	return "You are currently submerged.\n";
    }
 
    if (!Time_Left)
    {
        return "You try to light the " + short() + ", but fail... " +
	    "It's useless!\n";
    }

    if (Burn_Alarm || query_prop(TEMP_STDTORCH_CHECKED))
    {
        return "The " + short() + " is already lit.\n";
    }

    return 0;
}

/*
 * Function name: light_me
 * Description:   Actual routine to light this torch.
 * Returns:       1/0 - success/failure
 */
public int
light_me()
{
    if (!Time_Left || Burn_Alarm)
    {
	return 0;
    }

    add_prop(TEMP_STDTORCH_CHECKED, 1);
    set_alarm(0.0, 0.0, light_me_after_delay);
    return 1;
}

/*
 * Function name: command_light
 * Description:   light this torch
 * Returns:       string - an error message (failure)
 *                1 - torch successfully lit
 */
public mixed
command_light()
{
    mixed fail;

    if (stringp(fail = query_light_fail()))
    {
        return fail;
    }

    return light_me();
}

/*
 * Function name: extinguish_me
 * Description:   Actual routine to extinguish this torch.
 * Returns:       1/0 - success/failure
 */
int
extinguish_me()
{
    mixed *alarm;

    if (!Burn_Alarm)
    {
        return 0;
    }

    if (sizeof(alarm = get_alarm(Burn_Alarm)))
    {
	Time_Left = ftoi(get_alarm(Burn_Alarm)[2]);
	remove_alarm(Burn_Alarm);
    }

    Burn_Alarm = 0;
    remove_prop(OBJ_I_LIGHT);
    remove_prop(OBJ_I_HAS_FIRE);
    return 1;
}

/*
 * Function name: command_extinguish
 * Description:   Extinguish this torch.
 * Returns:       string - an error message (failure)
 *                1 - torch successfully extinguished
 */
public mixed
command_extinguish()
{
    if (!Burn_Alarm)
    {
	return "The " + short() + " isn't lit.\n";
    }

    return extinguish_me();
}

/*
 * Function name: decay_torch
 * Description  : This function is called with a delay to destruct the
 *                torch after burning out. Notice that the check for
 *                query_torch_may_decay() must done before calling this
 *                function.
 */
void
decay_torch()
{
    object env = environment();
    string tmp = short();

    if (living(env))
    {
	tell_object(env, "The remains of the " + tmp + " fall apart.\n");
	tell_room(environment(env), "The remains of the " + tmp + " that " +
	    QTNAME(env) + " is holding fall apart.\n", env);
    }
    else if (env->query_prop(ROOM_I_IS))
    {
	tell_room(env, "The remains of the " + tmp + " fall apart.\n");
    }

    remove_object();
}

#if 0
/*
 * Function name: hook_torch_burned_out
 * Description  : This function is called in the ENVIRONMENT of the torch when
 *                it burns out. This can be a living or a room. The purpose of
 *                this hook is that livings or rooms may trigger on the event.
 * Arguments    : object torch - the torch that burnt out.
 */
public void
hook_torch_burned_out(object torch)
{
}
#endif

/*
 * Function name: burned_out
 * Description:	  If this function is called when the torch has burned out.
 */
public void
burned_out()
{
    object env = environment();
    string tmp = ::short();

    Time_Left = 0;
    Burn_Alarm = 0;

    remove_prop(OBJ_I_LIGHT);
    remove_prop(OBJ_I_HAS_FIRE);

    set_adj("out");
    set_adj("burnt");

    if (!objectp(env))
    {
	return;
    }

    if (living(env))
    {
	tell_object(env, "The " + tmp + " goes out.\n");
	tell_room(environment(env), "The " + tmp + " that " +
	    QTNAME(env) + " is holding goes out.\n", env);
    }
    else if (env->query_prop(ROOM_I_IS))
    {
	tell_room(env, "The " + tmp + " goes out.\n");

        if (query_torch_may_decay())
        {
            Decay_Alarm = set_alarm(DECAY_TIME, 0.0, decay_torch);
        }
    }

    /* Call this in the environment to let it trigger on the event. */
    env->hook_torch_burned_out(this_object());
}

/*
 * Function name: enter_env
 * Description  : A torch that is burned out will decay after some time. The
 *                decaying starts when the object is put in a room, and it
 *                stops again when picked up. Check query_torch_may_decay()
 *                to see if it is allowed to decay.
 * Arguments    : object to - where the torch is going.
 *                object from - where the torch comes from.
 */
public void
enter_env(object to, object from)
{
    ::enter_env(to, from);

    if (Decay_Alarm)
    {
        if (!to->query_prop(ROOM_I_IS))
        {
            /* Don't bother to keep track of the decay time it has already
             * had. When it is dropped again, simply start counting anew.
             */
            remove_alarm(Decay_Alarm);
            Decay_Alarm = 0;
        }
    }
    else if (!Time_Left && query_torch_may_decay())
    {
        Decay_Alarm = set_alarm(DECAY_TIME, 0.0, decay_torch);
    }
}

/*
 * Function name: query_torch_recover
 * Description:   Return the recover string for changing tourch values
 * Returns:	  string - part of recover string
 */
public string
query_torch_recover()
{
    int tmp;

    if (Burn_Alarm && sizeof(get_alarm(Burn_Alarm)))
    {
	tmp = ftoi(get_alarm(Burn_Alarm)[2]);
    }
    else
    {
	tmp = Time_Left;
    }

    return "#t_t#" + tmp + "#t_l#" + query_prop(OBJ_I_LIGHT) + "#";
}

/*
 * Function name: init_torch_recover
 * Description:   Initialize the torch variables at recover.
 * Arguments:     string arg - The recover string as recieved from
 *			query_torch_recover()
 */
public void
init_torch_recover(string arg)
{
    string foobar;
    int tmp;

    sscanf(arg, "%s#t_t#%d#%s", foobar, Time_Left, foobar);
    sscanf(arg, "%s#t_l#%d#%s", foobar, tmp, foobar);
    if (tmp > 0)
    {
	add_prop(OBJ_I_LIGHT, tmp);
	add_prop(OBJ_I_HAS_FIRE, 1);
	Burn_Alarm = set_alarm(itof(Time_Left), 0.0, burned_out);
    }
}

/*
 * Function name: query_recover
 * Description:   A default query_recover() for torches.
 * Returns:	  A default recovery string.
 */
public string
query_recover()
{
    return MASTER + ":" + query_torch_recover();
}

/*
 * Function name: init_recover
 * Description:   A default init_recover() for torches.
 * Arguments:	  arg - String with variables to recover.
 */
public void
init_recover(string arg)
{
    init_torch_recover(arg);
}

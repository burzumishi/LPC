/*
 * /std/poison_effect.c
 *
 * This object is the standard poison effect. It will harm people who are
 * poisioned.
 */

/*
 * Example of simple poison effect:
 *
 * inherit "/std/poison_effect";
 * #include <poison_types.h>
 *
 * create_poison_effect()
 * {
 *     set_interval(20);
 *     set_time(500);
 *     set_damage( ({ POISON_FATIGUE, 100, POISON_HP, 30 }) );
 *     set_strength(20);
 *     set_poison_type("spider");
 * }
 *
 * You can make your own poison effects too. To make your own effect, define
 * POISON_USER_DEF as damage type, and then the value of the special damage.
 * You can have multiple special damage types. See the header of the file
 * special_damage().
 *
 * Usage: from the poisoning object, when the poison should be started,
 *        execute the following code fragment.
 *
 * poison = clone_object("our_poison");
 * poison->move(living_we_want_to_poison);
 * poison->start_poison(poisoner);
 */

/*
 * The poison is autoloading in nature. If you want to add you own variables
 * to the autoloadstring, please define the functions:
 *
 * string query_poison_recover() { return my_args; }
 *
 * void   init_poison_recover(string my_args) { my_recovery_code; }
 */
#pragma save_binary
#pragma strict_types

inherit "/std/object";

#include <files.h>
#include <log.h>
#include <macros.h>
#include <poison_types.h>
#include <ss_types.h>
#include <std.h>
#include <stdproperties.h>

/*
 * Global variables
 */
float  p_time;   /* The poison lasts for 'p_time' hearbeats             */
float  interval; /* The poison damages you every 'interval' seconds     */
int    strength; /* The strength of the poison, to match for cure       */
int    silent;   /* 0 - poisonee gets messages                          */
                 /* 1 - poisonee will not cough but get damage messages */
                 /* 2 - poisonee will not get any messages              */
int    recovery; /* if set to 1 this is a recovery after you quit       */
int    *damage;  /* The damage the poison can do                        */
int    a_dam;    /* The id of the damage_player alarm                   */
int    a_time;   /* The id of the time_out alarm                        */
int    no_cleanse; /* If true, then this poison cannot be cleansed.     */
string type;     /* The type of the poison, to match for cure           */
object poisonee; /* The victim that is being poisoned                   */
string responsible_object; /* The object that cloned the poison into us */
string responsible_living; /* The living responsible for the poisoning. */

/* Prototype. */
public void remove_object();

/*
 * A property we use when the player is linkdead.
 */

#define POISON_F_TIME_LEFT "_poison_f_time_left"

/*
 * Function name: create_poison_effect
 * Description  : The normal create for the poison_effect. Redefine this
 *                function if you want to create your own poison.
 */
public void
create_poison_effect()
{
}

/*
 * Function name: create_object
 * Description  : This function defines the object. There are several things
 *                we want to define so we define it for you.
 */
public nomask void
create_object()
{
    ::create_object();

    set_name("poison");
    set_long("This is poison.\n");
    set_short("poison");

    set_no_show();

    add_prop(OBJ_I_WEIGHT, 0);
    add_prop(OBJ_I_VOLUME, 0);
    add_prop(OBJ_I_VALUE, 0);

    /* Set up a default poison in case it isn't done by the creator. */
    p_time = 500.0;
    interval = 30.0;
    strength = 50;
    poisonee = 0;
    silent = 0;
    type = "standard";
    damage = ({ POISON_FATIGUE, 40, POISON_HP, 30, POISON_MANA, 20,
        POISON_STAT, SS_CON });

    create_poison_effect();
}

/*
 * Function name: set_no_cleanse
 * Description  : When called, this function makes it impossible to cleanse
 *                the potion. Note that it is still possible to destruct
 *                such a potion.
 */
nomask public void
set_no_cleanse()
{
    no_cleanse = 1;
}

/*
 * Function name: query_no_cleanse
 * Description  : Call this to find out whether the poison can be cleansed.
 * Returns      : int - if true, the poison cannot be cleansed.
 */
nomask public int
query_no_cleanse()
{
    return no_cleanse;
}

/*
 * Function name: set_poison_type
 * Description  : This sets the type of poison, for use with antidotes.
 * Artuments    : t - the type of the poison
 */
public void
set_poison_type(string t)
{
    type = t;
}

/*
 * Function name: query_poison_type
 * Description  : This returns the type of poison, for use with antidotes.
 * Returns      : string - the type of the poison
 */
public string
query_poison_type()
{
    return type;
}

/*
 * Function name: set_strength
 * Description  : This sets the strength of the poison to overcome player
 *                resistance. This strength must be in the range 1 - 100.
 * Arguments    : int s - The strength of the poison.
 */
public void 
set_strength(int s) 
{ 
    if (s < 1)
    {
        s = 1;
    }
    else if (s > 100)
    {
        s = 100;
    }
    strength = s; 
}

/*
 * Function name: query_strength
 * Description  : This returns the strength of the poison.
 * Returns      : int - the strength
 */
public int 
query_strength() 
{ 
    return strength; 
}

/*
 * Function name: set_silent
 * Description  : This sets whether the poison should be 'silent'. If it is
 *                silent, the poisonee will not cough or make other sounds.
 * Arguments    : 0 - the poisonee will get all messages
 *                1 - the poisonee will nog cough etc but get other messages
 *                2 - the poisonee will get no messages at all
 */
public void
set_silent(int t)
{
    silent = t;
}

/*
 * Function name: query_silent
 * Description  : This returns whether the poison is silent.
 * Returns      : 0 - the poisonee will get all messages
 *                1 - the poisonee will nog cough etc but get other messages
 *                2 - the poisonee will get no messages at all
 */
public int
query_silent()
{
    return silent;
}

/*
 * Function name: set_time
 * Description  : This sets the length of time the poison will last,
 *                in seconds.
 * Arguments    : t - the time the poison lasts
 */
public void 
set_time(int t) 
{ 
    p_time = itof(t);
}

/*
 * Function name: query_time
 * Description  : This returns the time the poison will last, in seconds.
 * Returns      : int - the time the poison lasts in total.
 */
public int 
query_time() 
{ 
    return ftoi(p_time);
}

/*
 * Function name: query_time_left
 * Description  : Returns the time left in the poison before it will expire.
 * Returns      : int - the time in seconds.
 */
public int
query_time_left()
{
    mixed arr;

    if (a_time &&
        sizeof(arr = get_alarm(a_time)))
    {
        return ftoi(arr[2]);
    }
    else if (query_prop(POISON_F_TIME_LEFT))
    {
        return query_prop(POISON_F_TIME_LEFT);
    }

    return 0;
}

/*
 * Function name: set_interval
 * Description  : This sets the average interval between damage being
 *                applied, in seconds.
 * Arguments    : i - the interval
 */
public void 
set_interval(int i) 
{
    interval = itof(i);
}

/*
 * Function name: query_interval
 * Description  : This returns the average interval between damage
 *                applications, in seconds.
 * Returns      : int - the interval
 */
public int 
query_interval() 
{
    return ftoi(interval);
}

/*
 * Function name: query_recovery
 * Description  : This returns whether the poison is being recovered or
 *                whether it is a first time poisoning.
 * Returns      : 1 - the poison is recovered after someone quit
 *                0 - this is a first time poisoning.
 */
public int
query_recovery()
{
    return recovery;
}

/*
 * Function name: query_responsible_living
 * Description  : Returns a description (name, filename) of the living thought
 *                to be responsible for this poison. The poisonee should be
 *                set in start_poision().
 * Returns      : string - the description of the living.
 */
public string
query_responsible_living()
{
    return responsible_living;
}

/*
 * Function name: query_responsible_object
 * Description  : Returns the filename of the object directly responsible for
 *                this poison.
 * Returns      : string - the filename of the object.
 */
public string
query_responsible_object()
{
    return responsible_object;
}

/*
 * Function name: set_damage
 * Description  : This function sets the maximum damage done by the poison
 *                Damage is an array of type and amount (or stat in the
 *                case of POISON_STAT)
 * Arguments    : *d - the damage types
 */
public void 
set_damage(int *d) 
{
    damage = d;
}

/*
 * Function name: add_damage
 * Description  : This adds an extra damage type.
 * Arguments    : *d - the damage types to add
 */
public void
add_damage(int *d)
{
    damage += d;
}

/*
 * Function name: query_damage
 * Description  : This function returns the damage array of the poison
 * Returns      : int * - the damage types
 */
public int *
query_damage() 
{
    return damage;
}

/*
 * Function name: timeout
 * Description  : This is called when the poison duration has expired.
 *                It simply removes itself.
 */
public void
timeout()
{
    if (silent < 2)
    {
        tell_object(poisonee, "You feel much better.\n");
    }

    remove_alarm(a_dam);
    remove_object();
}

/*
 * Function name: kill_player
 * Description  : The player has died, so we kill him! 8-)
 */
public void
kill_player()
{
    if (a_time)
    {
        remove_alarm(a_time);
    }
    a_time = 0;

    tell_object(poisonee, "You have died.\n");
    set_alarm(0.0, 0.0, remove_object);
    poisonee->do_die(this_object());
}

/*
 * Function name: log_player_death_extra_info
 * Description  : This function is called to query extra information about the
 *                object that was responsible for the player death. We use it
 *                to report more about the person/object responsible.
 * Returns      : string - the information.
 */
string
log_player_death_extra_info()
{
    string log_msg = "Responsible object: " + responsible_object + "\n";

    if (strlen(responsible_living))
    {
        log_msg += "Responsible living: " + responsible_living + "\n";
    }
    return log_msg;
}

/*
 * Function name: tell_damage_player
 * Description  : The player has been hurt; tell him how.  A string
 *                is passed, which must sound reasonable in the 
 *                sentences "You feel xxxx." and "You feel much xxxx."
 * Arguments    : phit - the damage level
 *                str  - the string to tell the player.
 */
public void
tell_damage_player(int phit, string str)
{
    if (silent > 1)
    {
        return;
    }

    if (phit > 90)
    {
        tell_object(poisonee, "You feel so much " + str + 
            ", you wish you were dead.\n");
        return;
    }

    if (phit > 75)
    {
        tell_object(poisonee, "You feel much " + str + ".\n");
        return;
    }

    tell_object(poisonee, "You feel " + str + ".\n");
    return;
}

/*
 * Function name: special_damage
 * Description  : This function is called for any non-standard values of the
 *                poison.  This function should be redefined when you want to
 *                get a different damage type.
 *
 *                To get special damages in simply use POISON_USER_DEF as the
 *                poison type, followed by the damage value. For example:
 *                    ({ POISON_USER_DEF, 15 })
 *
 * Arguments    : int damage - the damage value for the user defined damage.
 */
public void
special_damage(int damage)
{
}

/*
 * Function name: damage_player
 * Description:   This function actually carries out the damage on the
 *                player.  It then sets a new damage_player set_alarm.
 *                This function provides stat reduction, hp damage, fatiguing,
 *                and mana reduction. special_damage may be redefined to
 *                provide other types of damage.
 */
public nomask void
damage_player()
{
    int index;
    int size;
    int stat;
    int pdam;
    int res;
    int dam;

    /*
     * Get resistance as a percent, since we use it more than once.
     */
    res = 100 - poisonee->query_magic_res(MAGIC_I_RES_POISON);
    res = MAX(res, 0);

    index = 0;
    size = sizeof(damage);
    while(index < size)
    {
        if (random(poisonee->query_stat(SS_CON)) > strength)
        {
            index += 2;
            continue;
        }

        switch (damage[index]) 
        {
        case POISON_HP:
            dam = (res * random(damage[index + 1])) / 100;
            if (!(pdam = poisonee->query_hp()))
            {
                pdam = 1;
            }
            pdam = 100 * dam / pdam;
            poisonee->reduce_hit_point(dam);
            tell_damage_player(pdam, "less healthy");
            if (poisonee->query_hp() <= 0)
            {
                kill_player();
            }
            break;

        case POISON_MANA:
            dam  = (res * random(damage[index + 1])) / 100;
            if (!(pdam = poisonee->query_mana()))
            {
                pdam = 1;
            }
            pdam = 100 * dam / pdam;
            poisonee->add_mana(-dam);
            tell_damage_player(pdam, "mental fatigue");
            break;

        case POISON_FATIGUE:
            dam  = (res * random(damage[index + 1])) / 100;
            if (!(pdam = poisonee->query_fatigue()))
            {
                pdam = 1;
            }
            pdam = 100 * dam / pdam;
            poisonee->add_fatigue(-dam);
            tell_damage_player(pdam, "more tired");
            break;

        case POISON_STAT:
            if (random(100) < res)
            {
                poisonee->add_tmp_stat(damage[index + 1], - 1,
                    ftoi(p_time * rnd()));
            }
            break;

        case POISON_USER_DEF:
        default:
            special_damage(damage[index + 1]);
            break;
        }
        index += 2;
    }

    /* Let us make the player emit random pitiful sounds if the poison is
     * not silent. 8)
     */
    if ((random(3)) && (!silent))
    {
        poisonee->command( ({ "$choke", "$cough", "$puke", "$shiver",
            "$moan", "$groan" })[random(6)]);
    }
  
    a_dam = set_alarm((interval / 2.0) + (rnd() * interval), 0.0,
                      damage_player);
}

/*
 * Function name: start_poison
 * Description  : This function simply starts the poison working.  Until the
 *                poison is acivated, it is a simple object. Once activated,
 *                the poison will disappear after 'time' seconds.
 * Arguments    : object poisoner - the living that is responsible for the
 *                    poisoning of this player. This could be an NPC or real
 *                    player, if any.
 */
public varargs void
start_poison(object poisoner)
{
    object previous_ob = previous_object();

    poisonee = environment(this_object());

    /* Find out who owns us */
    if (!living(poisonee))
    {
         /* If they aren't living, we punt. */
         remove_object();
         return;
    }

    if (!recovery)
    {
        /* Register who poisoned us. */
        responsible_object = file_name(previous_ob);
        responsible_living = "";
        /* If not given, let's try to find out. Could be a weapon. */
        if (!objectp(poisoner) && IS_WEAPON_OBJECT(previous_ob))
        {
            poisoner = previous_ob->query_wielded();
        }
        if (!objectp(poisoner))
        {
            poisoner = this_player();
            responsible_living = "TP: ";
        }
        if (objectp(poisoner))
        {
            if (interactive(poisoner))
            {
                responsible_living += poisoner->query_name() + "; " +
                    file_name(poisoner);
            }
            else
            {
                responsible_living += file_name(poisoner);
            }
        }
        else
        {
            responsible_living = "unspecified";
        }
    }

    if (silent < 2)
    {
        tell_object(poisonee, "You have been poisoned!\n");
    }

    add_prop(OBJ_I_NO_DROP, 1);
    add_prop(OBJ_I_NO_GIVE, 1);

    if (interval)
    {
        a_dam = set_alarm((interval / 2.0) + (rnd() * interval), 0.0,
            damage_player);
    }

    a_time = set_alarm(p_time, 0.0, timeout);
}

/*
 * Function name: cure_damage_type
 * Description  : Called internally to try to cure a specific damage type.
 * Arguments    : int type - the type to cure.
 *                int success - the success of the cure.
 */
static void
cure_damage_type(int type, int success)
{
    int index = sizeof(damage) - 2;

    /* Must be strong enough to have success. */
    if (success <= strength)
    {
        return;
    }

    /* Check whether the damage type exists, and if so, remove it. */
    while(index >= 0)
    {
        if (damage[index] == type)
        {
            damage = slice_array(damage, 0, (index - 1)) +
                slice_array(damage, (index + 2), sizeof(damage));
        }
        index -= 2;
    }
}

/*
 * Function name: cure_poison
 * Description  : This function tries to cure a poison. If is it is succesful,
 *                then the poison will be removed. It will later be made
 *                possible to weaken poisons when removing completely does not
 *                succeed.
 * Arguments    : string *cure_type - the types that can be cured.
 *                int success - the 'strength' of the cure.
 * Returns      : 1/0 (success/fail)
 */
public int
cure_poison(string *cure_type, int success)
{
    int index = -1;
    int size = sizeof(cure_type);

    /* No cleanse -> no cleanse. */
    if (no_cleanse)
    {
        return 0;
    }

    while(++index < size)
    {
        switch(cure_type[index])
        {
        case POISON_CURE_FATIGUE:
            cure_damage_type(POISON_FATIGUE, success);
            break;

        case POISON_CURE_HP:
            cure_damage_type(POISON_HP, success);
            break;

        case POISON_CURE_MANA:
            cure_damage_type(POISON_MANA, success);
            break;

        case POISON_CURE_STAT:
            cure_damage_type(POISON_STAT, success);
            break;

        case "all":
            if ((success / 2) > strength)
            {
                damage = ({ });
            }
            break;

        default:
            if ((member_array(type, cure_type) != -1) &&
                (success > strength))
            {
                damage = ({ });
            }
        }
    }

    /* No damage left, we must be cured. */
    if (!sizeof(damage))
    {
        if (a_time)
        {
            remove_alarm(a_time);
        }
        a_time = 0;
        timeout();
        return 1;
    }

    return 0;
}

/*
 * Function name: remove_object
 * Description  : This function is called when the object is removed.
 */
void
remove_object()
{
    if (a_time)
    {
        remove_alarm(a_time);
    }

    ::remove_object();
}

/*
 * Function name: linkdeath_hook()
 * Description  : When the player who is poisoned loses his link, then we
 *                suspend the poison until the person revives.
 * Arguments    : object player - the player we poisoned.
 *                int linkdeath - 1/0 - if true, the player linkdies,
 *                    else he revives.
 */
public void
linkdeath_hook(object player, int linkdeath)
{
    mixed arr;
    float time_left;

    /* Player linkdies. */
    if (linkdeath)
    {
        /* Access failure. */
        if (!a_time)
        {
            return;
        }

        /* Find out how much time there is left. */
        if (sizeof(arr = get_alarm(a_time)))
        {
            add_prop(POISON_F_TIME_LEFT, arr[2]);
        }
        remove_alarm(a_time);
        a_time = 0;
        remove_alarm(a_dam);
        a_dam = 0;
    }
    /* Player revives from linkdeath. */
    else
    {
        time_left = query_prop(POISON_F_TIME_LEFT);
        if (!time_left)
        {
            return;
        }

        if (interval)
        {
            a_dam = set_alarm((interval / 2.0) + (rnd() * interval), 0.0,
                damage_player);
        }

        a_time = set_alarm(time_left, 0.0, timeout);
    }
}

/*
 * Function name: query_poison_recover
 * Description  : To add more information to the recover string, you should
 *                mask this function to return that information. Do not
 *                make a call to ::query_poison_recover!
 * Returns      : string - the extra recover string.
 */
public string
query_poison_recover()
{
    return "-";
}

/*
 * Function name: query_auto_load
 * Description:   Used to reload the poison into the player if it hasn't
 *                expired when he quits. When you want to add more information
 *                to the recover string, mask query_poison_recover().
 */
public nomask string
query_auto_load()
{
    float time_left = 0.0;
    string dam_string = "";
    int index;
    int prevent_cleanse = no_cleanse;
    mixed arr;

    for (index = 0; index < sizeof(damage); index++)
    {
        dam_string += "," + damage[index];
    }

    if (!a_time)
    {
        time_left = query_prop(POISON_F_TIME_LEFT);
        if (!floatp(time_left))
        {
            time_left = 0.0;
        }
    }
    else if (sizeof(arr = get_alarm(a_time)))
    {
        time_left = arr[2];
    }

    /* When a posion is kept while quitting, half of the time that already
     * passed will be re-added to the remaining time.
     */
    time_left = (time_left + p_time) / 2.0;

    /* If Armageddon is not present, force the no-cleanse on. */
    if (!SECURITY->shutdown_active())
    {
        prevent_cleanse = 1;
    }

    return MASTER + ":" +
        ftoi(time_left) + "," +
        ftoi(interval) + "," +
        type + "," +
        strength + "," +
        silent + "," +
        prevent_cleanse +
        dam_string + "#USER#" +
        query_poison_recover();
}

/*
 * Function name: init_posion_recover
 * Description  : To add more information to the recover string, you should
 *                mask this function to process that information after you
 *                have added it with query_poison_recover().
 * Arguments    : string arg - the extra recover string.
 */
public void
init_poison_recover(string arg)
{
}

/*
 * Function name: init_arg
 * Description  : Parses the data from the saved object.
 * Arguments    : arg - the arguments to init
 */
public nomask void
init_arg(string arg)
{
    int index;
    string *arglist;
    int dam;
    string s1, s2;

    /* Set that this is a recovery after someone quit */
    recovery = 1;
    responsible_object = "unknown after quit";
    responsible_living = "";

    sscanf(arg, "%s#USER#%s", s1, s2);

    arglist = explode(s1, ",");

    sscanf(arglist[0], "%f", p_time);
    sscanf(arglist[1], "%f", interval);
    type = arglist[2];
    sscanf(arglist[3], "%d", strength);
    sscanf(arglist[4], "%d", silent);

    /* If you quit, the poison will be more vicious when you return.  */
    strength = (110 * strength) / 100;

    /* If the no-cleanse is already set, make sure we don't reset it. */
    if (!no_cleanse)
    {
        sscanf(arglist[5], "%d", no_cleanse);
    }

    damage = ({ });

    if (silent < 2)
    {
        write("You still feel deadly ill.\n");
    }

    for (index = 6; index < sizeof(arglist); index++)
    {
        sscanf(arglist[index], "%d", dam);
        damage += ({ dam });
    }

    init_poison_recover(s2);

    set_alarm(1.0, 0.0, start_poison);
}

/*
 * Function name: stat_object
 * Description  : Called when wizard stats the object
 * Returns      : A string describing the object.
 */
public string
stat_object()
{
    return ::stat_object() +
        "Time      : " + ftoi(p_time) + "\n" +
        "Time left : " + query_time_left() + "\n" +
        "Interval  : " + ftoi(interval) + "\n" +
        "Strength  : " + strength + "\n" +
        "Type      : " + type + "\n" +
        "Silent    : " + silent + "\n" +
        "No cleanse: " + no_cleanse + "\n";
}

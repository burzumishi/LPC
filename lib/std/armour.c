/*
 * /std/armour.c
 *
 * Contains all routines relating to armours of any kind.
 *
 * set_ac(int)          Sets the armour class.
 *
 * set_at(type)         Set the armour type.
 *                      If not defined, set to default from /sys/formulas.h
 *
 * set_am(list)         Set armour modifier vs weapon damage type.
 *                      If not defined, set to default from /sys/formulas.h
 *
 * set_af(obj)          Sets the name of the object that contains the function
 *                      to call for extra defined wear() and remove()
 *                      functions.
 *
 * By default, armours recover. The function query_recover is included
 * in this code. If you want to recover some variables yourself, you
 * have to redefine the function and add a call to query_wep_recover.
 * Subsequently, init_recover is automatically called and you only have
 * to redefine it if you have added your own variables. The original
 * functionality in query_recover and init_recover has to be kept if
 * you redefine the recovery functions!
 * The easiest way to accomplish this is to do:
 *
 * string
 * query_recover()
 * {
 *     ::query_recover() + my_args;
 * }
 *
 * void
 * init_recover(string arg)
 * {
 *     my_recover_code;
 *     ::init_recover(arg);
 * }
 */

#pragma save_binary
#pragma strict_types

inherit "/std/object";
inherit "/lib/wearable_item";

#include <cmdparse.h>
#include <formulas.h>
#include <macros.h>
#include <stdproperties.h>
#include <wa_types.h>

/*
 * Prototypes.
 */
string arm_condition_desc();
void update_prop_settings();
varargs void remove_broken(int silent = 0);
int query_value();

/*
 * Variables. They are all static which means that they will not be saved.
 */
static int      arm_ac,         /* Armour class */
                *arm_mods,      /* Armour modifiers */
                arm_shield,     /* Bodypart(s) protected by shield */
                max_value,      /* The value of armour when not worn down */
                hits,           /* No of hits on armour without worse cond */
                condition,      /* How correded/worn down the armour has been */
                likely_break,   /* How likely the armour is to break */
                likely_cond,    /* Likely cond of armour will worsen */
                repair;         /* How much of this has been repaired */

/*
 * Function name: create_armour
 * Description  : In order to create an armour, you should redefine this
 *                function. Typically you use this function to set the
 *                name and description, the armour type, the armour class
 *                etcetera.
 */
void
create_armour() 
{
}

/*
 * Function name: create_object
 * Description  : This function is called to create the armour. It sets
 *                some default (sanity) settings and then the function
 *                create_armour is called. You must redefine that function
 *                as this function is nomasked, ie you cannot redefine it.
 */
public nomask void
create_object()
{
    set_slots(F_ARMOUR_DEFAULT_AT);
    arm_mods = allocate(W_NO_DT);
    likely_break = 2;
    likely_cond = 3;
    set_name("armour");
    set_pname("armours");
    add_adj("unworn");
    set_looseness(2);
    set_layers(4);
    worn = 0;
    add_prop(OBJ_I_VALUE, &query_value());
    add_prop(OBJ_I_VOLUME, 1000);
    add_prop(OBJ_I_WEIGHT, 500);

    /* If this will be true, the weapon will not recover on reboot and is
     * lost after the reboot.
     */
    will_not_recover = (random(100) < PERCENTAGE_OF_RECOVERY_LOST);

    create_armour();

    update_prop_settings();
}

/*
 * Function name: reset_armour
 * Description  : At a regular interval this function will be called to
 *                reset the armour. If you define this function, you must
 *                also call enable_reset from your create_armour. If you
 *                fail to do this, the function reset_armour will _not_
 *                be called.
 */
void
reset_armour()
{
}

/*
 * Function name: reset_object
 * Description  : At a regular interval this function will be called. All
 *                it does is call reset_armour. For further details see
 *                that function. As this function is declared nomask, you
 *                may not redefine it.
 */
nomask void
reset_object()
{
    reset_armour();
}

/*
 * Function name: short
 * Description  : If the armour is broken, we add the adjective broken to
 *                the short description. There is a little caveat it the
 *                short description has not been explicitly set. In that
 *                case, the adjective broken may appear twice.
 * Arguments    : object for_obj - the object that wants to know.
 * Returns      : string - the short description.
 */
public varargs string
short(object for_obj)
{
    if (query_prop(OBJ_I_BROKEN))
    {
        return "broken " + ::short(for_obj);
    }

    return ::short(for_obj);
}

/*
 * Function name: plural_short
 * Description  : If the armour is broken, we add the adjective broken
 *                to the plural short description.
 * Arguments    : object for_obj - the object that wants to know.
 * Returns      : string - the plural short description.
 */
public varargs string
plural_short(object for_obj)
{
    string str = ::plural_short(for_obj);

    /* We make this additional check for stringp(str) because if no plural
     * short has been set, we shoudln't alter it. The plural short will
     * be generated if no plural short has been set.
     */
    if (query_prop(OBJ_I_BROKEN) && stringp(str))
    {
        return "broken " + str;
    }

    return str;
}

/*
 * Function names: long
 * Description   : The long description. We add the condition information
 *                 to it. This function only returns the string, it does
 *                 not print it to the looker.
 * Arguments     : string str - a possible add-item to look for.
 *                 object for_obj - the object that wants to know.
 * Returns       : string - the long description.
 */
public varargs string
long(string str, object for_obj)
{
    return ::long(str, for_obj) + (str ? "" : arm_condition_desc());
}

static mixed
do_wear_item()
{
    return wearer->wear_arm(this_object());
}

static void
do_remove_item()
{
    wearer->remove_arm(this_object());
}

static mixed
check_slot(int slot)
{
    object tool;

    if (tool = this_player()->query_tool(slot))
    {
        return "The " + tool->short() + " is in the way.\n";
    }

    return ::check_slot(slot);
}

/*
 * Function name: leave_env
 * Description:   The armour is moved from the inventory
 * Arguments:     from - Where from
 */
void
leave_env(object from, object to)
{
    wearable_item_leave_env(from, to);
    ::leave_env(from, to);
}

/*
 * Function name: set_ac
 * Description:   Set the armour class of the object
 */
void
set_ac(int class)
{
    /* All changes may have been locked out. */
    if (query_lock())
    {
        return;
    }

    arm_ac = class;
}

/*
 * Description: Give the ac for a specific hitlocation that is protected
 * Arguments:   hid: Hitlocation id
 */
varargs int
query_ac(int hid)
{
    return arm_ac - condition + repair;
}

/*
 * Function name: set_at
 * Description:   Set armour type
 */
void
set_at(int type)
{
    set_slots(type);
}

/*
 * Description: Give the armour type for this armour.
 *              Armour type is a combination of the tool slots that this
 *              armour takes up and possibly the magic flag.
 *              (see /sys/wa_types.h)
 * Arguments:   hid: Hitlocation id
 */
int
query_at()
{
    return query_slots_setting();
}

/*
 * Function name: set_am
 * Description:   Set the modifyers for different attacks
 */
void
set_am(int *list)
{
    if (query_lock())
        return;                 /* All changes have been locked out */

    if (F_LEGAL_AM(list))
        arm_mods = list;
}

/*
 * Description: Give the ac modifier for a specific hitlocation
 * Arguments:   hid: Hitlocation id
 */
int *
query_am(int hid)
{
    return arm_mods + ({});
}

/*
 * Function name: set_condition
 * Description:   Use this to increases the corroded status on armour. If the
 *                armour gets dented or anything that turns it into worse
 *                condition, call this function.
 * Arguments:     cond - The new condition we want (can only be raised)
 * Returns:       1 if new condition accepted
 */
int
set_condition(int cond)
{
    if (cond > condition)
    {
        condition = cond;
        if (F_ARMOUR_BREAK(condition - repair, likely_break))
            set_alarm(0.1, 0.0, remove_broken);
        if (worn && wearer)
            wearer->update_armour(this_object());
        return 1;
    }
    return 0;
}

/*
 * Function name: query_condition
 * Description:   Return the general condition modifier. It indicates how
 *                many times the condition of this armour has been worsened.
 *                The tru condition of the armour is:
 *                      base_ac - condition + repairs
 * Returns:       The general condition modifier
 */
int
query_condition()
{
    return condition;
}

/*
 * Function name: set_likely_cond
 * Description:   Set how likely the armour will get worn down if hit
 * Arguments:     i - how likely [0, 30] recommended
 */
void
set_likely_cond(int i)
{
    likely_cond = i;
}

/*
 * Function name: query_likely_cond
 * Description:   How likely is this armour to wear down with use (a relative
 *                number)
 * Returns:       The likliness
 */
int
query_likely_cond()
{
    return likely_cond;
}

/*
 * Function name: set_likely_break
 * Description:   Set how likely the armour is to break if you use it.
 * Argument:      i - How likely, [0, 20] recommended
 */
void
set_likely_break(int i)
{
    likely_break = i;
}

/*
 * Function name: query_likely_break
 * Description:   How likely this piece of armour is to break (a relative number)
 * Returns:       How liely it is
 */
int
query_likely_break()
{
    return likely_break;
}

/*
 * Function name: remove_broken
 * Description  : The armour got broken so we remove it from the
 *                player.
 * Arguments    : int silent - true if no message should be generated
 *                             about the fact that the armour breaks.
 */
varargs void
remove_broken(int silent = 0)
{
    /* If the armour is not worn, we only adjust the broken-information
     * by adding the adjective and the property. We do this inside the
     * if-statement since we do not want to screw the message that may
     * be displayed later. Note that the property automatically adds the
     * adjective broken.
     */
    if (!worn || !wearer)
    {
        add_prop(OBJ_I_BROKEN, 1);
        return;
    }

    /* A broken armour will always be removed, so we do not have to
     * dereference the result.
     */
    if (objectp(wear_func))
    {
        wear_func->remove(this_object());
    }

    /* If the wizard so chooses, this message may be omitted. */
    if (!silent)
    {
        tell_object(wearer, "The " + short(wearer) + " breaks!!!\n");
        tell_room(environment(wearer), "The " + QSHORT(this_object()) +
            " worn by " + QTNAME(wearer) + " breaks!!!\n", wearer);
    }

    /* Force the player to remove the armour and adjust the broken
     * information by adding the property and the adjective.
     */
    wearer->remove_arm(this_object());
    add_prop(OBJ_I_BROKEN, 1);
    remove_adj("worn");
    add_adj("unworn");
    worn = 0;
}

/*
 * Function name: set_repair
 * Description:   When trying to repair the armour, call this function. Repairs
 *                can only increase the repair factor.
 * Arguments:     rep - The new repair number
 * Returns:       1 if new repair status accepted
 */
int
set_repair(int rep)
{
    if (rep > repair && F_LEGAL_ARMOUR_REPAIR(rep, condition))
    {
        repair = rep;
        if (worn && wearer)
            wearer->update_armour(this_object());
        return 1;
    }
    return 0;
}

/*
 * Function name: query_repair
 * Description:   How many times this weapon has been repaired. Actual ac is:
 *                      base_ac - condition + repairs
 * Returns:       How many times...
 */
int
query_repair()
{
    return repair;
}

/*
 * Function Name: query_repair_cost
 * Description  : Returns the cost to repair this armour one level.
 * Returns      : int - the cost in cc
 */ 
int
query_repair_cost()
{
    return max(max_value, F_VALUE_ARMOUR(query_ac())) *
        F_ARMOUR_REPAIR_COST_FACTOR / 100;
}

/*
 * Function name: set_armour_hits
 * Description:   By setting the hits counter you will have influence over how
 *                likely the armour is to get in a worse condition. The hits 
 *                variable keeps track of how many times this piece of armour
 *                has been hit.
 * Argument:      new_hits - integer
 */
public void
set_armour_hits(int new_hits)
{
    hits = new_hits;
}

/*
 * Function name: query_armour_hits
 * Description:   This function returns how many times this armour has been hit
 *                since last time it got degenerated. The lower this number is
 *                the less likely the armour will degenerate (strange isn't it?)
 * Returns:       The hits variable
 */
public int
query_armour_hits()
{
    return hits;
}

/*
 * Function name: add_prop_obj_i_value
 * Description:   Someone is adding the value prop to this object.
 * Arguments:     val - The new value (mixed)
 * Returns:       1 if not to let the val variable through to the prop
 */
int
add_prop_obj_i_value(mixed val)
{
    if (!max_value)
    {
        max_value = 10;
        return 0;
    }

    if (intp(val) && val)
        max_value = val;

    return 1;
}

/*
 * Function name: query_value
 * Description:   Qhat's the value of this armour
 */
int
query_value()
{
    if (query_prop(OBJ_I_BROKEN))
        return 0;

    return max_value * F_ARMOUR_VALUE_REDUCE(condition - repair) / 100;
}

/*
 * Sets the object to call wear/remove in when this occurs.
 * Those functions can return:
 *              0 - No affect the armour can be worn / removed
 *              1 - It can be worn / removed but no text should be printed
 *                  (it was done in the function)
 *              -1  It can not be worn / removed default failmsg will be 
 *                  written
 *             string  It can not be worn / removed 'string' is the 
 *                     fail message to print
 */
void
set_af(object obj)
{
    set_wf(obj);
}

object
query_af()
{
    return query_wf();
}

/*
 * Function name: got_hit
 * Description:   Notes that the defender has been hit. It can be used
 *                to reduce the ac for this hitlocation for each hit.
 * Arguments:     hid:   The hitloc id, ie the bodypart hit.
 *                ph:    The %hurt
 *                att:   Attacker
 *                aid:   The attack id
 *                dt:    The damagetype
 *                dam:   The damage done to us in hit points
 */
varargs int
got_hit(int hid, int ph, object att, int dt, int dam)
{
    if (dam <= 0)
        return 0;

    hits++;
    if (F_ARMOUR_CONDITION_WORSE(hits, arm_ac, likely_cond))
    {
        hits = 0;
        set_condition(query_condition() + 1);
    }

    return 0;
}

/*
 * Function name: check_armour
 * Description:   Check file for security.
 */
nomask int
check_armour()
{
    return 1;
}

/*
 * Function name: set_default_armour
 * Description  : This routine is a shortcut function for the following:
 *                  set_ac(ac);
 *                  set_at(at);
 *                  set_am(am);
 *                  set_af(af);
 * Arguments    : See the referred routines.
 */
public varargs void
set_default_armour(int ac, int at, int *am, object af)
{
    /* Sets the armour class. */
    if (ac) set_ac(ac); 
    else set_ac(1);

    /* Set the armour type. */
    if (at) set_at(at);
    else set_at(A_BODY);
    
    /* Set armour modifier vs weapon damage type. */
    if (am) set_am(am);
    else set_am(A_NAKED_MOD);

    
    /* Sets the name of the object that contains the function
       to call for extra defined wear_arm() and remove_arm()
       functions. */
    if (af) set_af(af);
}
    

/*
 * Function name: set_shield_slot
 * Description:   Set the hitlocation(s) protected by the shield or magic
 *                armour
 * Arguments:     hids: Hitlocation(s)
 */
public void
set_shield_slot(mixed hids)
{
    arm_shield = hids;
}

public nomask int *
query_protects();

/*
 * Function name: query_shield_slots
 * Description:   Give a bodypart protected by a shield or magic armour
 */
public int *
query_shield_slots()
{
    int *ss;

    ss = check_call(arm_shield);
    if (ss) return ss;
    if (worn_on_part & W_LEFT) return ({ A_L_ARM });
    if (worn_on_part & W_RIGHT) return ({ A_R_ARM });
    if (query_prop(OBJ_I_IS_MAGIC_ARMOUR)) return query_protects();
    return ({ });
}

/*
 * Function name: query_protects
 * Description:   Give an array of the bodyparts protected by this armour.
 */
public nomask int *
query_protects()
{
    int abit, *hids, *chid;
    /*
       Table of armour slots <-> Hitlocations

             TORSO HEAD LEGS R_ARM L_ARM ROBE SHIELD

    TORSO      X      
    HEAD            X
    LEGS                 X
    R_ARM                      X
    L_ARM                            X
    ROBE       X         X                
    SHIELD                     X     X                          
    MAGIC      X    X    X     X     X          

        Max ac is 100 for all hitlocations. If the sum of the ac for a
        given hitlocation is > 100 then it is set to 100.
    */
    for (hids = ({}), abit = 1; abit <= worn_on_part; abit *= 2)
    {
        if (worn_on_part & abit)
        {
            switch (abit)
            {
                case A_CHEST:
                    chid = ({ A_TORSO });    
                    break;
                case A_HEAD:    
                case A_LEGS:
                case A_R_ARM:
                case A_L_ARM:
                    chid = ({ abit });
                    break;
                case A_ROBE:  
                    chid = ({ A_TORSO, A_LEGS });
                    break;
                case W_LEFT:
                case W_RIGHT:
                    chid = query_shield_slots();
                    break;
                default:
                    chid = ({});
            }

            hids = (hids - chid) + chid;
        }
    }

    return hids;
}
 
/*
 * Function name: update_prop_settings
 * Description:   Will uppdate weight and value of this object to be legal
 */
nomask void
update_prop_settings()
{
    if (query_prop(OBJ_I_VALUE) < F_VALUE_ARMOUR(arm_ac) &&
                        !query_prop(OBJ_I_IS_MAGIC_ARMOUR))
        add_prop(OBJ_I_VALUE, F_VALUE_ARMOUR(arm_ac));
 
    if (F_WEIGHT_FAULT_ARMOUR(query_prop(OBJ_I_WEIGHT), arm_ac, arm_at) &&
                        !query_prop(OBJ_I_IS_MAGIC_ARMOUR))
        add_prop(OBJ_I_WEIGHT, F_WEIGHT_DEFAULT_ARMOUR(arm_ac, arm_at));
}

/*
 * Function name: stat_object
 * Description:   This function is called when a wizard wants to get more
 *                information about an object.
 * Returns:       str - The string to write..
 */
string
stat_object()
{
    string str;

    str = ::stat_object();

    str += "AC: " + arm_ac + "\t";
    if (sizeof(arm_mods))
        str += "Mods: " + arm_mods[0] + ", " + arm_mods[1] + ", " +
                arm_mods[2] + "\t";

    str += "Type: " + arm_at + "\n";
    if (!query_worn())
        wearer = this_player();
    if (arm_at == A_SHIELD)
        str += "It's a shield.\n";
    else if (arm_at == A_ANY_FINGER)
        str += "It's a ring.\n";
    else
        str += "It can be worn" + wear_how(arm_at) + ".\n";

    str += "Hits: " + hits + "  Condition: " + condition + "  Repairs: " + repair;

    return str + "\n";
}

/*
 * Function name: arm_condition_desc
 * Description:   Returns the description of this armour
 */
string
arm_condition_desc()
{
    string str;

    if (query_prop(OBJ_I_BROKEN))
        return "It is broken.\n";

    switch(condition - repair)
    {
        case 0:
            str = "in prime condition";
            break;
        case 1:
        case 2:
            str = "a little worn down";
            break;
        case 3:
        case 4:
            str = "in a very bad shape";
            break;
        case 5:
        case 6:
        case 7:
            str = "in urgent need of repair";
            break;
        default:
            str = "likely to break any second";
            break;
    }

    return "It looks like it is " + str + ".\n";
}

/*
 * Function name: appraise_object
 * Description  : Someone tries to appraise the object. We add information
 *                about the way you should use this armour.
 */
void
appraise_object(int num)
{
    ::appraise_object(num);

    appraise_wearable_item();
}

/*
 * Function name: query_arm_recover
 * Description:   Return the recover strings for changing armour variables.
 * Returns:       Part of the recoder string
 */
string
query_arm_recover()
{
    return "#ARM#" + hits + "#" + condition + "#" + repair + "#" +
        query_prop(OBJ_I_BROKEN) + "#";
}

/*
 * Function name: init_arm_recover
 * Description:   Initialize the armour variables at recover.
 * Arguments:     arg - The recover string as recieved from query_arm_recover()
 */
void
init_arm_recover(string arg)
{
    string foobar;
    int    broken;

    sscanf(arg, "%s#ARM#%d#%d#%d#%d#%s", foobar,
        hits, condition, repair, broken, foobar);

    if (broken != 0)
    {
        add_prop(OBJ_I_BROKEN, 1);
    }
}

/*
 * Function name: query_recover
 * Description  : Called to check whether this armour is recoverable.
 *                If you have variables you want to recover yourself,
 *                you have to redefine this function, keeping at least
 *                the filename and the armour recovery variables, like
 *                they are queried below.
 *                If, for some reason, you do not want your armour to
 *                recover, you should define the function and return 0.
 * Returns      : string - the default recovery string.
 */
public string
query_recover()
{
    return MASTER + ":" + query_arm_recover();
}

/*
 * Function name: init_recover
 * Description  : When the object recovers, this function is called to set
 *                the necessary variables. If you redefine the function,
 *                you must add a call to init_arm_recover or a call to
 *                ::init_recover with the string that you got after querying
 *                query_arm_recover.
 * Arguments    : string argument - the arguments to parse
 */
public void
init_recover(string arg)
{
    init_arm_recover(arg);
}

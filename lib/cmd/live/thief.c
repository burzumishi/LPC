/*
 * thief.c
 *
 * This soul holds the thief commands for the mud.
 *
 * The soul contains the following commands:
 *
 * - backstab
 * - steal
 */
#pragma no_clone
#pragma no_inherit
#pragma strict_types

inherit "/cmd/std/command_driver";

#include <cmdparse.h>
#include <composite.h>
#include <files.h>
#include <filter_funs.h>
#include <formulas.h>
#include <macros.h>
#include <money.h>
#include <ss_types.h>
#include <stdproperties.h>
#include <tasks.h>
#include <wa_types.h>

// Define where to log steals, or undef to not log.
#undef LOG_STEALS "/some_dir/open/STEAL"

// Prototypes
private int query_the_value(object ob);

/*
 * Function name: create
 * Description  : This function is called the moment this object is created
 *                and loaded into memory.
 */
public void
create()
{
    seteuid(getuid(this_object()));
}

/*************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
public string
get_soul_id()
{
    return "thief";
}

/*************************************************************************
 * This is a command soul.
 */
public int
query_cmd_soul()
{
    return 1;
}

/*************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
public mapping
query_cmdlist()
{
    return ( ([
                "backstab": "backstab",
                "steal"   : "steal",
             ]) );
}

/*
 * Function name: using_soul
 * Description:   Called once by the living object using this soul. Adds
 *                sublocations responsible for extra descriptions of the
 *                living object.
 */
public void
using_soul(object live)
{
}

/************************************************************************
 * Here follows the support functions for the commands below            *
 ************************************************************************/

/*
 * Function name: remove_extra_skill
 * Description:   This will remove the awareness bonus given to a
 *                player whom a thief has visited.
 * Arguments:     object who - Player to remove the bonus from
 * Returns:       void
 */
public void
remove_extra_skill(object who)
{
    int tmp;

    if (!objectp(who))
    {
        return;
    }

    if ((tmp = who->query_prop(LIVE_I_VICTIM_ADDED_AWARENESS)) < 2)
    {
        who->set_skill_extra(SS_AWARENESS,
            who->query_skill_extra(SS_AWARENESS) - F_AWARENESS_BONUS);
        who->remove_prop(LIVE_I_VICTIM_ADDED_AWARENESS);
    }
    else
    {
        who->add_prop(LIVE_I_VICTIM_ADDED_AWARENESS, --tmp);
    }
}

/*
 * Function name: query_internal_value
 * Description:   This is used to determine the combined value of
 *                all the objects inside a container.
 * Arguments:     object ob - The container whos inventory we are checking
 * Returns:       int - The combined value of all objects inside ob
 */
private int
query_internal_value(object ob)
{
    object *inv = deep_inventory(ob);
    int sum = 0;
    int index = 0;
   
    if (!pointerp(inv) || !(index = sizeof(inv)))
    {
        return 0;
    }

    while( index-- )
    {
        if (!objectp(inv[index]))
            continue;

        sum += query_the_value(inv[index]);
    }
    
    return sum;
}

/*
 * Function name: query_the_value
 * Description:   This is used to determine the value of
 *                an object.
 * Arguments:     object ob - the object whos value we are checking
 * Returns:       int - ob's value
 */
private int
query_the_value(object ob)
{
    if (!objectp(ob))
    {
        return 0;
    }

    if (IS_HEAP_OBJECT(ob))
    {
        return ob->heap_value();
    }

    if (IS_HERB_OBJECT(ob))
    {
        return ob->query_herb_value();
    }

    if (IS_POTION_OBJECT(ob))
    {
        return ob->query_potion_value();
    }

    if (IS_CONTAINER_OBJECT(ob))
    {
        return (ob->query_value() + ob->query_internal_value(ob));
    }

    return ob->query_value();
}

/*
 * Function name: check_watchers_see_steal
 * Description:   Determines whether or not onlookers can see a thief
 *                (attemp to) steal from his victim.  Also handles the
 *                printing of messages if this is the case.
 * Arguments:     object place - the place we are stealing from
 *                object victim - the person we are stealing from
 *                object thief - the person committing the crime
 *                int chance - the thief's success value
 *                object item - the item being stolen
 * Returns:       void
 * Note:          This _only_ gets called for successful thefts, don't
 *                  need to waste the processing otherwise.
 *                  Also: Wizards see all steals automatically in the room.
 */
private void
check_watchers_see_steal(object place, object victim, object thief,
                         int chance, object item)
{
    mixed  tmp;
    object *whom;
    object *wiz;
    object *watchers;
    int index;
    int notice;

    /* Sanity check! */
    if (!objectp(place) || !objectp(thief))
    {
        return;
    }

    whom  = FILTER_LIVE(all_inventory(environment(thief)));
    whom -= ({ 0, thief });

    if (victim)
    {
        whom -= ({ victim });
    }

    /* Filter any present wizards out */
    wiz   = FILTER_IS_WIZARD(whom);
    whom -= wiz;
    index = sizeof(whom);

    while(index--)
    {
        /* If we can't see, we can't notice */
        if (!CAN_SEE_IN_ROOM(whom[index]) ||
            !CAN_SEE(whom[index], thief)  ||
            !CAN_SEE(whom[index], victim) )
        {
            continue;
        }

        /* Some quick checks to see if we spotted it. */
        notice  = (12 * (whom[index]->query_skill(SS_AWARENESS) / 2));
        tmp     = ((600 * whom[index]->query_intoxicated()) /
                   whom[index]->intoxicated_max());
        notice -= MAX(tmp, 0);
        tmp     = ((200 * whom[index]->query_fatigue()) /
                   whom[index]->query_max_fatigue());
        notice -= MAX(tmp, 0);
        notice -= (objectp(whom[index]->query_attack()) ? 600 : 0);

        /* We didn't spot the theft. */
        if (notice < chance)
        {
            continue;
        }

        /* Generate the watcher message */
        tmp = "You notice " + thief->query_the_name(whom[index]);
        if (place == environment(thief))
        {
            tmp += " quickly pocketing something with a " +
                "furtive look.\n";
        }
        else
        {
            tmp += " gently removing something from " +
                (victim ? victim->query_the_name(whom[index]) :
                place->short()) + ".\n";
        }
        tell_object(whom[index], tmp);

        /* Can we tell if we got spotted? */
        if (CAN_SEE(thief, whom[index]) && thief->resolve_task(
                TASK_DIFFICULT, ({ SS_AWARENESS }), whom[index],
                ({ TS_DIS })))
        {
            if (!pointerp(watchers))
            {
                watchers = ({ whom[index] });
            }
            else
            {
                watchers += ({ whom[index] });
            }
        }

        /* An extra hook to call, for a guild perhaps? */
        whom[index]->hook_onlooker_stolen_object(thief, victim);

        /* Does the onlooker attack thieves? */
        if (whom[index]->query_prop(LIVE_I_ATTACK_THIEF))
        {
            if (CAN_SEE_IN_ROOM(whom[index]) &&
                CAN_SEE(whom[index], thief))
            {
                whom[index]->command("$say to " +
                    OB_NAME(thief) + " Stop it, you thief!");

                /* Do not switch targets in combat */
                if (!objectp(whom[index]->query_attack()))
                {
                    whom[index]->command("$kill " + OB_NAME(thief));
                }
            }
            else
            {
                whom[index]->command("$say Stop it, you thief!");
            }
        }
    }
    
    if (tmp = sizeof(watchers))
    {
        /* Uh oh.. we got spotted, print the bad news.. */
        tell_object(thief, capitalize(FO_COMPOSITE_LIVE(watchers, thief)) +
            ((tmp > 1) ? " seem " : " seems ") +
            "to have noticed the theft!\n");
    }
   
    if (sizeof(wiz))
    {
        /* We have a wizard audience, let them know what transpired. */
        wiz->catch_tell("You see " + thief->query_name() + " " +
            query_verb() + "ing " + LANG_ASHORT(item) + " from " +
            (victim ? victim->query_name() : 
            place->short()) + ".\n");
    }
}


/*************************************************************************
 * Backstab  -  Nail 'em in the back I say
 */
private void
perform_backstab(string str, object whom)
{
    mixed tmp;
    object ob;
    object weapon;
    string one;
    string two;
    int stat;
    int skill;
    int aware;

    if (!objectp(whom))
    {
        return;
    }

    set_this_player(whom);

    this_player()->remove_prop(LIVE_I_BACKSTABBING);

    if (!strlen(str) || sscanf(str, "%s with %s", one, two) != 2)
    {
        write("Backstab whom with what?\n");
        return;
    }

    if (interactive(this_player()) &&
        (this_player()->query_skill(SS_BACKSTAB) < F_BACKSTAB_MIN_SKILL))
    {
        write("You lack the training for such an " +
            "endeavour, you would get caught for sure.\n");
        return;
    }

    if (!this_player()->query_prop(OBJ_I_HIDE))
    {
        write("You must be hidden in order to backstab someone.\n");
        return;
    }

    if (!CAN_SEE_IN_ROOM(this_player()))
    {
        if (tmp = environment(this_player())->query_prop(ROOM_S_DARK_MSG))
        {
            write(tmp + " backstab anyone here.\n");
            return;
        }
        write("You cannot see enough to backstab anyone here.\n");
        return;
    }

    if (this_player()->query_ghost())
    {
        write("Umm yes, killed. That's what you are.\n");
        return;
    }

    one = lower_case(one);
    if (!sizeof(tmp = parse_this(one, "[the] %l")))
    {
        write("You find no such living creature.\n");
        return;
    }

    if (sizeof(tmp) > 1)
    {
        write("Be specific, you cannot backstab " +
            COMPOSITE_ALL_LIVE(tmp) + " at the same time.\n");
        return;
    }

    ob = tmp[0];
    if (!living(ob))
    {
        write(capitalize(LANG_THESHORT(ob)) + " isn't alive!\n");
        return;
    }

    if (ob->query_ghost())
    {
        write(ob->query_The_name(this_player()) + " is already dead!\n");
        return;
    }

    if (objectp(this_player()->query_attack()))
    {
        write("You cannot backstab while already fighting!");
        return;
    }

    if (tmp = environment(this_player())->query_prop(ROOM_M_NO_ATTACK))
    {
        write(stringp(tmp) ? tmp :
            "You sense a divine force preventing your attack.\n");
        return;
    }

    if (tmp = ob->query_prop(OBJ_M_NO_ATTACK))
    {
        if (stringp(tmp))
        {
            write(tmp);
        }
        else
        {
            write("You feel a divine force protecting this being, your " +
                "attack fails.\n");
        }

        return;
    }

    if (!sizeof(tmp = this_player()->query_weapon(-1)) ||
        !sizeof(tmp = FIND_STR_IN_ARR(two, tmp)))
    {
        write("You need to be wielding a knife or dagger to " +
            "backstab with.\n");
        return;
    }

    if (sizeof(tmp) > 1)
    {
        write("Be specific, you cannot backstab with " +
            COMPOSITE_ALL_DEAD(tmp) + " at the same time.\n");
        return;
    }

    weapon = tmp[0];
    if (weapon->query_wt() != W_KNIFE)
    {
        write("You need a knife or dagger to backstab with.\n");
        return;
    }

    if (weapon->query_wielded() != this_player())
    {
        write("You must be wielding the " + weapon->short() +
            "in order to backstab with it.\n");
        return;
    }

    if (this_player()->query_fatigue() < F_BACKSTAB_FATIGUE)
    {
        write("You are too tired to attempt a backstab " +
            "at this time.\n");
        return;
    }


    if ((this_player()->query_npc() == 0) &&
         this_player()->query_met(ob) &&
        (this_player()->query_prop(LIVE_O_LAST_KILL) != ob))
    {
        this_player()->add_prop(LIVE_O_LAST_KILL, ob);
        /* Only ask if the player did not use the real name of the target. */
        if (one != ob->query_real_name())
        {
            write("Backstab " + ob->query_the_name(this_player()) +
                "?!? Please confirm by typing again.\n");
            return;
        }
    }

    if (!F_DARE_ATTACK(this_player(), ob))
    {
        write("Umm... no. You do not have enough self-discipline to dare!\n");
        return;
    }

    skill = this_player()->query_skill(SS_BACKSTAB);
    aware = ob->query_skill(SS_AWARENESS) / 2;

    if (ob->query_npc() && !aware)
    {
        tmp   = (ob->query_stat(SS_INT) + ob->query_stat(SS_WIS)) / 3;
        aware = (tmp > 100 ? 100 : tmp);
    }

    if (tmp = ob->query_prop(LIVE_I_GOT_BACKSTABBED))
    {
        aware += (tmp * tmp * 2);
    }

    stat = MAX(120, this_player()->query_stat(SS_DEX));
    stat = ((stat + this_player()->query_skill(SS_WEP_KNIFE)) / 2);
    tmp  = F_BACKSTAB_HIT(skill, stat, aware, ob->query_skill(SS_DEFENCE));

    if (tmp < 1)
    {
        this_player()->add_fatigue(-(F_BACKSTAB_FATIGUE / 2));
        this_player()->reveal_me(1);
        this_player()->attack_object(ob);

        write("You lunge at " + ob->query_the_name(this_player()) +
            " with your " + weapon->short() + ", but miss and " +
            "reveal your intent!\n");
        ob->tell_watcher(QCTNAME(this_player()) + " lunges at " + QTNAME(ob) +
            " from behind, but misses.\n", this_player());
        tell_object(ob, this_player()->query_Art_name(ob) +
            " lunges at you from behind, but misses.\n");
    }
    else
    {
        stat = this_player()->query_stat(SS_STR);
        stat = ((stat > 120) ? 120 : stat);
        tmp = F_BACKSTAB_PEN( skill, this_player()->query_skill(SS_WEP_KNIFE),
            weapon->query_pen(), stat );
        tmp = ob->hit_me(tmp, W_IMPALE, this_player(), -1, A_BACK);
        this_player()->add_fatigue(-F_BACKSTAB_FATIGUE);
        this_player()->attack_object(ob);
        
        switch(tmp[0])
        {
        case 70..98:
            one = "massacring you";
            two = "massacring " + ob->query_objective();
            break;
        case 45..69:
            one = "hurting you very badly";
            two = "hurting " + ob->query_objective() + " very badly";
            break;
        case 25..44:
            one = "hurting you rather badly";
            two = "hurting " + ob->query_objective() + " rather badly";
            break;
        case 10..24:
            one = "hurting you";
            two = "hurting " + ob->query_objective();
            break;
        case 5..9:
            one = "slighty hurting you";
            two = "slightly hurting " + ob->query_objective();
            break;
        case 1..4:
            one = "grazing you";
            two = "grazing " + ob->query_objective();
            break;
        case -1..0:
            one = "without harming you";
            two = "without harming " + ob->query_objective();
            break;
        default:
            one = "assassinating you";
            two = "assassinating " + ob->query_objective();
            break;
        }

        write("You sneak up on " + ob->query_the_name(this_player()) +
            " and " + (tmp[0] < 1 ? "try to " : "") +
            "drive your " + weapon->short() + " into " +
            ob->query_possessive() + " back, " + two + ".\n");
        ob->tell_watcher( QCTNAME(ob) + " cries out " +
            (tmp[0] > 20 ? "in pain " : "") + "as " +
            QTNAME(this_player()) + " " + (tmp[0] < 1 ?
              "tries to stab " : "stabs ") + ob->query_objective() +
            " in the " + tmp[1] + " from behind.\n", this_player() );
        tell_object(ob, this_player()->query_The_name(ob) + " " +
            (tmp[0] < 1 ? "tries to stab " : "stabs ") +
            "your " + tmp[1] + " from behind, " + one + ".\n");

        weapon->did_hit(1, tmp[1], tmp[0], ob, W_IMPALE, tmp[2], tmp[3]);
        
        this_player()->reveal_me(1);
    }

    if (ob->query_hp() < 1)
    {
        ob->do_die(this_player());
    }
    else
    {
        if ((tmp = ob->query_prop(LIVE_I_GOT_BACKSTABBED)) < 5)
        {
            ob->add_prop(LIVE_I_GOT_BACKSTABBED, tmp + 1);
        }
    }
    return;
}

public int
backstab(string str)
{
    if (this_player()->query_prop(LIVE_I_BACKSTABBING))
    {
        write("You are already preparing for a backstab, be patient.\n");
        return 1;
    }

    write("You prepare to perform a backstab...\n");

    set_alarm( (2.0 + (rnd() * 2.0)), 0.0,
        &perform_backstab(str, this_player()));

    return 1;
}

/*************************************************************************
 * Steal  -  Rob 'em till they are blind
 */
public int
steal(string str)
{
    mixed tmp;
    object item;
    object victim;
    object place;
    int chance;
    int notice;
    int result;
    int success;
    int caught;
    int moveresult;
    int xp;
    string str1;
    string str2;
    string str3;
#ifdef LOG_STEALS
    int *log = ({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
#endif LOG_STEALS

    if (!str)
    {
        /* Umm.. we require input... */
        notify_fail(capitalize(query_verb()) + " what from " +
            "where [of what]?\n");
        return 0;
    }

    if (sscanf(str, "%s from %s of %s", str1, str3, str2) != 3)
    {
        if (sscanf(str, "%s from %s", str1, str2) != 2)
        {
            /* Oh my, we typed something unacceptable, what are the odds? */
            notify_fail(capitalize(query_verb()) + " what from " +
                "where [of what]?\n");
            return 0;
        }
    }

    /* You need at least a basic understanding... */
    if (interactive(this_player()) &&
        (this_player()->query_skill(SS_PICK_POCKET) < F_STEAL_MIN_SKILL))
    {
        write("You lack the training for such an " +
            "endeavour, you would get caught for sure.\n");
        return 1;
    }

    /* It's a bit hard to go stealing things in the midst of combat. */
    if (objectp(this_player()->query_attack()))
    {
        write("You are a bit preoccupied to be " + query_verb() +
                "ing things.\n");
        return 1;
    }

    /* If we cannot see, however are we going to steal? */
    if (!CAN_SEE_IN_ROOM(this_player()))
    {
        tmp = environment(this_player());
        if (str = tmp->query_prop(ROOM_S_DARK_LONG))
        {
            write(str);
            return 1;
        }

        if (str = tmp->query_prop(ROOM_S_DARK_MSG))
        {
            write(str + " " + query_verb() + " anything here.\n");
            return 1;
        }

        write("You cannot see enough to " + query_verb() +
            " anything here.\n");
        return 1;
    }

    if (tmp = environment(this_player())->query_prop(ROOM_M_NO_STEAL))
    {
        write(strlen(tmp) ? tmp : "You may not " + query_verb() +
            " in this place.\n");
        return 1;
    }

    if (member_array(str2, ({ "here", "ground", "floor" })) != -1)
    {
        /* Stealing from the room */
        place = environment(this_player());
    }
    else if (str2 == "someone")
    {
        tmp = filter(all_inventory(environment(this_player())), living);
        tmp = filter(tmp, &not() @ &->query_wiz_level());
        tmp = filter(tmp, &->check_seen(this_player()));
        tmp -= ({ 0, this_player() });

        if (!sizeof(tmp))
        {
            write("There doesn't appear to be anyone to "+ query_verb() +
                " from.\n");
            return 1;
        }

        /* Just another random sucker, unscrupulous thieves... */
        victim = tmp[random(sizeof(tmp))];
        place  = victim;
    }
    else
    {
        tmp = FIND_STR_IN_ARR(str2, 
            all_inventory(environment(this_player())) - ({ this_player() }));

        if (!sizeof(tmp))
        {
            write("You don't appear to be able to " + query_verb() +
                " from " + str2 + ".\n");
            return 1;
        }

        /* This should not happen anymore, but just in case.. */
        if (tmp[0] == this_player())
        {
            write("Now why would you choose to " + query_verb() +
                "from yourself?\n");
            return 1;
        }

        /* Hey.. it's there.. lets try to steal from it! */
        place = tmp[0];
        if (living(place))
        {
            /* It just happens to be alive too, all the better.. */
            victim = place;
        }
    }

    /* We picked a 'live' one, parse a few extra checks... */
    if (objectp(victim))
    {
        if (victim->query_wiz_level())
        {
            write("You may not " + query_verb() + " from deities.\n");
            return 1;
        }

        if (victim->query_linkdead())
        {
            write("You may not " + query_verb() + " from the linkdead.\n");
            return 1;
        }

        /* This is a potential waste, but there is no easy workaround. */
        if (victim->query_npc() && victim->query_prop(OBJ_M_HAS_MONEY))
        {
            MONEY_EXPAND(victim);
        }
    }

    /* We wanna try to steal from a container inside someone/something */
    if (str3)
    {
        tmp = FIND_STR_IN_OBJECT(str3, place);
        if (!sizeof(tmp))
        {
            write("You don't see any " + str3 + " " + (victim ?
                "being carried by "+ victim->query_the_name(this_player()) :
                "inside " + place->short()) + " to " + query_verb() +
                " from.\n");
            return 1;
        }

        /* Suspect found matching the description... */
        place = tmp[0];
    }

    /* Not sure how this could happen, probably a bug someplace. */
    if (!objectp(place))
    {
        write("Where are you trying to " + query_verb() + " from?\n");
        return 1;
    }

    /* Umm.. hello?  Try picking a container sometime... */
    if (!IS_CONTAINER_OBJECT(place))
    {
        write("You cannot " + query_verb() + " from " +
            place->short() + ".\n");
        return 1;
    }

    /* Okay.. better.. try one that is open next time. */
    if (place->query_prop(CONT_I_CLOSED))
    {
        write("The " + place->short() + " appears to be closed, " +
            "better look for something else to " + query_verb() +
            " from.\n");
        return 1;
    }

    /* Now we parse for the item to steal.. start with the random ones. */
    if (str1 == "something")
    {
        /* We limit what we might randomly steal... */
        tmp = filter(all_inventory(place), &->check_seen(this_player()));
        tmp = filter(tmp, &not() @ &->query_prop(OBJ_M_NO_STEAL));

        if (victim)
        {
            tmp = filter(tmp, &not() @ &->query_wielded());
            tmp = filter(tmp, &not() @ &->query_worn());
            tmp = filter(tmp, &not() @ &->query_held());
        }
        else if (place = environment(this_player()))
        {
            tmp = filter(tmp, &not() @ living);
        }

        /* Well.. we found something.. let's run with it. */
        if (sizeof(tmp))
        {
            item  = tmp[random(sizeof(tmp))];
        }
        else
        {
            write("You don't see anything to " + query_verb() + 
                (victim ? " from " + victim->query_the_name(this_player()) :
                place == environment(this_player()) ? "" :
                " from " + place->short()) + ".\n");

            /* Give the victim a chance to notice the attempt. */
            if (victim)
            {
                tmp = victim->resolve_task(TASK_DIFFICULT, 
                    ({ SS_AWARENESS, TS_DEX }), this_player(),
                    ({ SS_PICK_POCKET, TS_DEX }));

                if (tmp > 0)
                {
                    tell_object(victim, "You catch " +
                        this_player()->query_the_name(victim) +
                        " riffling through your belongings!\n");
                }
            }
            return 1;
        }
    }
    else
    {
        /* Issue the APB alert... */
        tmp = FIND_STR_IN_OBJECT(str1, place);

        /* Found it.  Cuff it and book it.. */
        if (sizeof(tmp))
        {
            item = tmp[0];
        }
        else
        {
            write("You don't see any " + str1 + " to " + query_verb() + 
                (victim ? " from " + victim->query_the_name(this_player())
                : place == environment(this_player()) ? "" :
                " from " + place->short()) + ".\n");

            /* Give the victim a chance to notice the attempt. */
            if (victim)
            {
                tmp = victim->resolve_task(TASK_DIFFICULT, 
                    ({ SS_AWARENESS, TS_DEX }), this_player(),
                    ({ SS_PICK_POCKET, TS_DEX }));

                if (tmp > 0)
                {
                    tell_object(victim, "You catch " +
                        this_player()->query_the_name(victim) +
                        " riffling through your belongings!\n");
                }
            }
            return 1;
        }
    }

    /* If we got here, something is wrong.. */
    if (!objectp(item))
    {
        write("What exactly are you trying to " + query_verb() + "?\n");
        return 1;
    }

    /* Stealing held/worn items has been outlawed, issue the bad news. */
    if (victim &&
        (item->query_wielded() || item->query_worn() || item->query_held()))
    {
        write("You are unable to " + query_verb() + " worn, wielded or " +
            "held items, your attempt has been noticed.\n");

        tell_object(victim, this_player()->query_The_name(victim) +
            " attempted to " + query_verb() + " your " + 
            item->short() + "!\n");

        return 1;
    }

    /* Setup the thieves base chances.. */
    tmp  = this_player()->query_skill(SS_SNEAK);
    tmp += this_player()->query_skill(SS_AWARENESS);
    chance += ((tmp / 2) + random(tmp /  2));
    chance += (6 * this_player()->query_skill(SS_PICK_POCKET));
    chance += (2 * MIN(this_player()->query_stat(SS_DEX), 120));

#ifdef LOG_STEALS
    log[0] = chance;
#endif LOG_STEALS

    /* If we have stolen within the last 10 minutes, lower our chances */
    tmp = this_player()->query_prop(LIVE_I_LAST_STEAL);
    if ((tmp = (300 + (tmp - time()))) > 0)
    {
        chance -= tmp;
    }
    
    /* Reset the last attempt value. */
    tmp = (time() + ((tmp > 0) ? ((tmp > 1200) ? 600 : (tmp / 2)) : 0));
    this_player()->add_prop(LIVE_I_LAST_STEAL, tmp);

    /* We may get a bonus for some reason. (guild?) */
    chance += this_player()->hook_thief_steal_bonus(victim, place, item);

    /* We get nervous if there is a large audience */
    tmp = filter(all_inventory(environment(this_player())), living);
    tmp = filter(tmp, &not() @ &->query_wiz_level());
    tmp -= ({ 0, this_player() });
    if ((tmp = sizeof(tmp) > 5))
    {
        tmp *= 50;
        chance -= MIN(tmp, 200);
    }

    /* We aren't stealing from a live victim, so we are more confident. */
    chance += (objectp(victim) ? 0 : 300);

    /* Drunkeness leads to sloppiness. */
    tmp = ((200 * this_player()->query_intoxicated()) /
           this_player()->intoxicated_max());
    chance -= MAX(tmp, 0);

    /* The more tired.. the less nimble the fingers. */
    tmp = ((100 * this_player()->query_fatigue()) /
           this_player()->query_max_fatigue());
    chance -= MAX(tmp, 0);

    /* Victim is underattack and blind, flailing, hard to steal from. */
    if (victim && victim->query_attack() && !CAN_SEE_IN_ROOM(victim))
    {
        chance /= 2;
    }

    /* Modify our chances based on the object itself. */
    if (IS_HEAP_OBJECT(item))
    {   
        chance -= (item->heap_light() * 300);
        chance -= (item->heap_weight() / 60);
        chance -= (item->heap_volume() / 60);
    }
    else
    {   
        chance -= (item->query_prop(OBJ_I_LIGHT) * 300);
        chance -= (item->query_weight() / 60);
        chance -= (item->query_volume() / 60);
    }

    /* Smaller players value their items more and are thus more wary */
    tmp = (victim ? (victim->query_npc() ? 100 : 
        victim->query_average_stat()) : 100);
    tmp = (query_the_value(item) / tmp);
    chance -= MAX(tmp, 0);

#ifdef LOG_STEALS
    log[1] = chance;
    log[2] = tmp;
#endif LOG_STEALS

    /* We're stealing from someone, not an easy task. */
    if (objectp(victim))
    {
        /* Int and Wis gives us a basic perception value. */
        tmp = (victim->query_stat(SS_INT) + victim->query_stat(SS_WIS));
        tmp = MIN(tmp, 240);
        notice += (2 * ((tmp / 2) + random(tmp)));

        /* And this gives us our basic chance to notice. */
        notice += (5 * victim->query_skill(SS_AWARENESS));

#ifdef LOG_STEALS
        log[3] = notice;
#endif LOG_STEALS

        /* We give newbies a better chance to notice artificially. */
        if (!victim->query_npc() && (victim->query_average_stat() < 30))
        {
            tmp = (5 * (35 - victim->query_average_stat()));
            notice += MAX(tmp, 0);
        }

        /* We might have a bonus to notice for some reason. (guild?) */
        notice += victim->hook_victim_no_steal_bonus(item);

        /* We are stealing from a carried object, even easier to spot. */
        tmp = (victim != place ? 10 : 0);
        notice += tmp;
        chance -= tmp;

        /* Drunk victims make less aware victims. */
        tmp = ((200 * victim->query_intoxicated()) /
                victim->intoxicated_max());
        notice -= MAX(tmp, 0);

        /* If your tired.. less chance you'll notice the small things */
        tmp = ((100 * victim->query_fatigue()) /
                victim->query_max_fatigue());
        notice -= MAX(tmp, 0);

        /* We can't see.. that drops our chances noticebly */
        if (!CAN_SEE_IN_ROOM(victim)) 
        {
            notice /= 2;
        }
    }
 
#ifdef LOG_STEALS
    log[4] = notice;
#endif LOG_STEALS

    /* Can't have a negative chance.. that would be bad. */
    if (chance < 1)
    {
        chance = 1;
    }
    else
    {
        /* Mix it up and randomize the chance a bit */
        chance = ((chance / 2) + random(chance / 4) + random(chance / 4));
    }

    /* Can't have a negative notice value.. that would also be bad */
    if (notice < 1)
    {
        notice = 1;
    }
    else
    {
        /* Randomize the chance to notice.. */
        notice = ((notice / 2) + random(notice / 4) + random(notice / 4));
    }

#ifdef LOG_STEALS
    log[5] = chance;
    log[6] = notice;
#endif LOG_STEALS

    if ((result = (chance - notice)) < 1)
    {
        /* We botched it, but did we get caught? */
        if (this_player()->resolve_task(TASK_DIFFICULT,
                 ({ TS_DEX, SS_PICK_POCKET }), victim, 
                ({ TS_DEX, SS_AWARENESS })) > 0)
        {
            success = 0;
            caught  = 0;
        }
        else
        { 
            success = 0;
            caught  = 1;
        }
    }
    else if ((result = ((random(result) + result) - 
                (random(1000) + random(1500)))) < 1)
    {
        /* We did it, but there is a chance we got spotted */
        if (this_player()->resolve_task(TASK_ROUTINE,
                 ({ TS_DEX, SS_PICK_POCKET }), victim, 
                ({ TS_DEX, SS_AWARENESS })) > 0)
        {
            success = 1;
            caught  = 0;
        }
        else
        {
            success = 1;
            caught  = 1;
        }
    }
    else 
    {
        /* We got off scott free */
        success = 1;
        caught  = 0;
    }

#ifdef LOG_STEALS
    log[7] = result;
#endif LOG_STEALS

    /* This should not happen.. But just in case.. */
    if (!objectp(place) || !objectp(item))
    {
        write("There doesn't seem to be anything to " + query_verb() +
            " by that description.\n");

        if (caught && victim)
        {
            tell_object(victim, "You catch " + 
                this_player()->query_the_name(victim) + " riffling " +
                "through your belongings!\n");
        }
        return 1;
    }

    /* Lets make sure the item isn't protected against thievery.. */
    if (!moveresult && (tmp = item->query_prop(OBJ_M_NO_STEAL)))
    {
        if (stringp(tmp))
        {
            write(tmp);
        }
        else
        {
            write("You cannot seem to " + query_verb() + " anything by " +
                "that description.\n");
        }

        if (caught && victim)
        {
            tell_object(victim, "You catch " + 
                this_player()->query_the_name(victim) + " attempting " +
                "to " + query_verb() + " your " + item->short() + 
                (place != victim ? " from your "+ place->short() : "" ) +
                "!\n");
        }
        return 1;
    }

    /* We succeeded, hurray! */
    if (success)
    {
        /* Lets try to move the item shall we? */
        moveresult = item->move(this_player());

#ifdef LOG_STEALS
        log[8] = moveresult;
#endif LOG_STEALS

        if (!moveresult)
        {
            /* A stolen item is no longer hidden. */
            item->remove_prop(OBJ_I_HIDE);

            /* A couple hook checks, for a guild perhaps.. */
            this_player()->hook_thief_stolen_object(item, victim, place);
            victim->hook_victim_stolen_object(item, this_player(), place);

            /* If you didn't get caught, you get some general exp. */
            if (!caught)
            {
                /* Only give experience when there are onlookers. The victim
                 * counts as an onlooker in this.
                 */
                if (sizeof(FILTER_OTHER_LIVE(all_inventory(environment(this_player())))))
                {
                    result = ABS(result);
                    xp = F_STEAL_EXP(result);
                }

                /* See if we stole from here before */
                tmp = victim->query_prop(LIVE_AO_THIEF);
                if (pointerp(tmp))
                {
                    /* We did, reduce the exp. */
                    if (member_array(this_player(), tmp) != -1)
                    {
                        xp /= 3;
                    }
                    else
                    {
                        tmp += ({ this_player() });
                    }
                }
                else
                {
                    tmp = ({ this_player() });
                }
                
                victim->add_prop(LIVE_AO_THIEF, tmp);

                /* Sanity check */
                if (xp > 0)
                {
#ifdef LOG_STEALS
                    log[9] = xp;
#endif LOG_STEALS
                    this_player()->add_exp_general(xp);
#ifdef STEAL_EXP_LOG
                    log_file(STEAL_EXP_LOG, sprintf(
                        "%12s %11s: %5d %s %s (%s)\n", 
                        ctime(time())[4..15], 
                        this_player()->query_name(), xp, 
                        file_name(item), victim->short(this_object()), 
                        file_name(victim)));
#endif STEAL_EXP_LOG
                }
            }

            tmp = (victim ? " from " + 
                victim->query_the_name(this_player()) :
                (place == environment(this_player()) ? "" :
                " from " + place->short()));

            write("You managed to " + query_verb() + " " +
                 COMPOSITE_DEAD(item) + tmp + ".\n");
        }
        else if (moveresult > 0)
        {
            success = 0;
            tmp = "You cannot " + query_verb() + " " + LANG_ASHORT(item) +
                " from " + 
                (objectp(victim) ? 
                 victim->query_the_name(this_player()) : 
                "the " + place->short()) + ", ";

            switch(moveresult)
            {
            case 9: /* This should not happen, but just in case. */
                tmp += "since the " + place->short() + " is closed!\n";
                break;
            case 1: case 4: case 5: case 8: case 10:
                tmp += "since you are unable to carry the item!\n"; 
                break;
            case 2: case 3: case 6:
                tmp += "since the item is not allowed to leave its place!\n";
                break;
            default: /* This should not happen either. */
                tmp += "since there is a wrongness in the fabric of space " +
                    "preventing it.\n";
                break;
            }
            write(tmp);
        }
        else  /* This should not happen either */
        {
            success = 0;
            write("You cannot " + query_verb() + " " + LANG_ASHORT(item) +
                " from " + (objectp(victim) ?
                victim->query_the_name(this_player()) :
                "the " + place->short()) + ", since there is a " +
                "wrongness in the fabric of space preventing it.\n");
        }
    }
    else /* We failed. */
    {
        tmp = (victim ? " from " + 
            victim->query_the_name(this_player()) :
            (place == environment(this_player()) ? "" :
            " from " + place->short()));

        write("You fail to " + query_verb() + " " + LANG_ASHORT(item) +
            tmp + ".\n");
    }

    /* We picked a live victim and were caught, print the messages */
    if (victim && caught)
    {
        if (success)
        {
            tell_object(victim, "You notice " +
                this_player()->query_the_name(victim) + " " +
                query_verb() + "ing " + LANG_ASHORT(item) + " from you!\n");
        }
        else
        {
            tell_object(victim, "You catch " + 
                this_player()->query_the_name(victim) + " trying " +
                "to " + query_verb() + " " + LANG_ASHORT(item) + 
                " from " + (place != victim ? "your " + place->short() : 
                "you") + "!\n");
        }

        /* We got busted, but do we know we got busted? */
        if (this_player()->resolve_task(TASK_SIMPLE, ({ SS_AWARENESS }),
                victim, ({ TS_DIS })) > 0)
        {
            write("Oops! " + victim->query_the_name(this_player()) +
                " seems to have caught on to you!\n");
        }

        /* Does the victim attack thieves instinctively? */
        if (victim->query_prop(LIVE_I_ATTACK_THIEF) &&
            CAN_SEE_IN_ROOM(victim) && 
            CAN_SEE(victim, this_player()))
        {
            victim->command("$say to " + OB_NAME(this_player()) +
                " Stop it you thief!");
            
            /* Don't switch targets in combat! */
            if (!objectp(victim->query_attack()))
            {
                victim->command("$kill " + OB_NAME(this_player()));
            }
        }

        /* Catching a thief makes us a bit more wary for awhile.. */
        tmp = victim->query_prop(LIVE_I_VICTIM_ADDED_AWARENESS);
        if (tmp == 0)
        {
            victim->set_skill_extra(SS_AWARENESS, 
                (victim->query_skill_extra(SS_AWARENESS) + 
                F_AWARENESS_BONUS));
        
            /* 5 minutes to be precise */
            set_alarm(300.0, 0.0, &remove_extra_skill(victim));
        }

        /* This ensures that we only get one bonus at a time */
        victim->add_prop(LIVE_I_VICTIM_ADDED_AWARENESS, ++tmp);
    }

    /* We are only going to bother with this if we got the item */
    if (success && moveresult == 0)
    {
        check_watchers_see_steal(place, victim, this_player(), 
            chance, item);
    }

#ifdef LOG_STEALS
    tmp = sprintf("Thieves name:  %-11s   Time:  %-s\nPlace/Victim:  %-s\n"+
        "Item Stolen:   %-s\nT_Pick:  %3d  T_Steal: %3d  "+
        "T_Aware: %3d  T_Dex: %3d\nV_Aware: %3d  "+
        "V_Int:   %3d  V_Wis:   %3d  V_Dex: %3d\n"+
        "B_Chance: %5d  E_Chance: %8d  "+
        "R_Chance: %8d\nB_Notice: %5d  E_Notice: %8d  "+
        "R_Notice: %8d\nItm_Val:  %5d  Result:   %8d  "+
        "Move_Res: %8d\nSuccess:  %5d  Caught:   %8d  "+
        "Gen_XP:   %8d\n%'-'75s\n\n", 
        this_player()->query_name(), ctime(time()),
        (victim ?  interactive(victim) ? victim->query_name() :
        file_name(victim) : file_name(place)), file_name(item),
        this_player()->query_skill(SS_PICK_POCKET), 
        this_player()->query_skill(SS_SNEAK),
        this_player()->query_skill(SS_AWARENESS), 
        this_player()->query_stat(SS_DEX), 
        victim->query_skill(SS_AWARENESS),
        victim->query_stat(SS_INT), 
        victim->query_stat(SS_WIS),
        victim->query_stat(SS_DEX),
        log[0], log[1], log[5], log[3], log[4], 
        log[6], log[2], log[7], log[8], 
        success, caught, log[9], ""); 

    setuid();
    seteuid(getuid(this_object()));
    write_file(LOG_STEALS, tmp);
#endif LOG_STEALS

    return 1;
}

/*
 *  /std/bow.c
 *
 * This class is the base of all bow weapons. Inherit this
 * class if you want to create a bow of your own.
 *
 * See /doc/examples/weapons/elven_bow.c for an example.
 */

#pragma strict_types
#pragma save_binary

inherit "/std/launch_weapon";
inherit "/lib/keep";

#include <filter_funs.h>
#include <formulas.h>
#include <language.h>
#include <macros.h>
#include <options.h>
#include <stdproperties.h>
#include <wa_types.h>
#define ENV(x) environment(x)


// Variables
string bowstring;    /* The path to the bowstring used to string this bow. */
int    stringed;     /* A flag that is set when bow is stringed.           */


// Prototypes
public string query_damage_desc(object archer, object target, object projectile, int phurt);
public nomask void snap_string();
public nomask void unstring_bow();
public nomask int parse_string(string args);
public nomask int parse_unstring(string args);


/*
 * Function name: create_bow
 * Description:   Constructor to be used by bow implementations.
 */
public void
create_bow()
{
    return;
}

/*
 * Function name: create_launch_weapon
 * Description:   Constructor to be used by launch_weapon implementations.
 */
public nomask void
create_launch_weapon()
{
    set_name("bow");
    set_long("A plain bow.\n");
    set_hit(30);
    set_pen(30);
    set_wf(this_object());
    set_shoot_command("shoot");
    set_drawn_weapon(1);
    stringed = 1;
    set_keep(0);
    create_bow();
    set_valid_projectile_function("is_arrow");
}

/*
 * Function name: init
 * Description  : Adds the string and unstring commands to the player.
 */
public void
init()
{
    ::init();
    add_action(parse_string, "string");
    add_action(parse_unstring, "unstring");
}

/*
 * Function name: did_parry
 * Description:   Called when this weapon was used to parry an attack. It can
 *                be used to wear down a weapon. Note that his method is called
 *                before the combat messages are printed out.
 * Arguments:     att:   Attacker
 *                aid:   The attack id
 *                dt:    The damagetype
 */
public varargs void
did_parry(object att, int aid, int dt)
{
    /* Higher skill means lower snap chance. With the +5 offset we ensure that
     * people who have no missile skill at all have a doubled chance of
     * breaking, while e.g. people with 15 skill have half the chance, etc. */
    int chance = ((F_BOWSTRING_SNAP_CHANCE * (query_wielded()->query_skill(SS_WEP_MISSILE) + 5)) / 10);

    if (stringed && !random(chance))
    {
	snap_string();
    }
}

/*
 * Function name: snap_string
 * Description:   Called when the bowstring of this bow snaps. It
 *                is responsible for giving the messages to all
 *                involved parties and calling unstring_bow()
 *                to get the internal state updated.
 */
public nomask void
snap_string()
{
    object archer = query_wielded();
    object *bystanders = FILTER_LIVE(all_inventory(environment(archer))) -
	({ archer });
	
    bystanders =
	filter(FILTER_IS_SEEN(archer, bystanders), &->can_see_in_room());

    tell_object(query_wielded(), "The bowstring of your " +
                                 short() + " snaps!\n");
    if (sizeof(bystanders))
    {
	bystanders->catch_msg("The bowstring of " +
			      LANG_POSS(QTNAME(archer)) + " " +
			      short() + " breaks!\n");
    }

    unstring_bow();
    bowstring = 0;
}

/*
 * Function name: string_string
 * Description:   Updates the internal state of the bow when it
 *                gets a bowstring.
 */
public nomask void
string_bow(string bowstr)
{
    bowstring = bowstr;
    stringed = 1;
    unwield_me();
    set_wt(W_MISSILE);
    set_dt(W_IMPALE);
    this_object()->string_bow_hook();
}

/*
 * Function name: query_stringed
 * Description:   Returns whether this bow is sttringed or not.
 *
 * Returns      : 1 stringed, 0 unstringed. 
 */
public nomask int
query_stringed()
{
    return stringed;
}

/*
 * Function name: unstring_bow
 * Description:   Updates the internal state of the bow when it
 *                looses its bowstring.
 */
public nomask void
unstring_bow()
{
    unwield_me();
    stringed = 0;
    set_wt(W_POLEARM);
    set_dt(W_BLUDGEON);
    this_object()->unstring_bow_hook();
}

/*
 * Function name: parse_string
 * Description  : Parses the player input when he wants to string
 *                a bow.
 *                
 * Arguments    : string - Bow to string.
 * Returns      : 1 command handled, 0 parse failed.
 */
public nomask int
parse_string(string args)
{
    object dummy;
    object *strings, *bystanders, archer;

    archer = this_player();
    
    if (environment(this_object()) != archer)
    {
	return 0;
    }

    if (!args || !parse_command(args, ({ this_object() }), " %o ", dummy))
    {
	notify_fail("String what? The " + short() + "?\n");
	return 0;
    }

    if (stringed)
    {
	write("The " + short() + " is already strung.\n");
	return 1;
    }

    strings = filter(all_inventory(archer), &->is_bowstring());

    if (!sizeof(strings))
    {
	write("You don't have any bowstrings.\n");
	return 1;
    }

    string_bow(MASTER_OB(strings[0]));
    
    write("You string your " + short() + " with " +
	  LANG_ADDART(strings[0]->short()) + ".\n");

    bystanders = FILTER_LIVE(all_inventory(environment(archer))) - ({archer});
    bystanders = filter(FILTER_IS_SEEN(archer, bystanders),
			&->can_see_in_room());

    if (sizeof(bystanders))
    {
	bystanders->catch_msg(QCTNAME(archer) + " strings " +
			      archer->query_possessive() + " " +
			      short() + ".\n");
    }
    
    strings[0]->remove_object();
    return 1;
}

/*
 * Function name: parse_unstring
 * Description  : Parses input from a player when he wants to unstring
 *                a bow.
 *                
 * Arguments    : string - Bow to unstring.
 * Returns      : 1 command handled, 0 parse failed.
 */
public nomask int
parse_unstring(string args)
{
    object dummy;
    object *bystanders, bstring, archer;

    archer = this_player();
    
    if (environment(this_object()) != archer)
    {
	return 0;
    }

    if (!args || !parse_command(args, ({ this_object() }), " %o ", dummy))
    {
	notify_fail("Unstring what? The " + short() + "?\n");
	return 0;
    }

    if (!stringed)
    {
	write("The " + short() + " has no string.\n");
	return 1;
    }

    /* Must have an EUID to clone. */
    setuid(); seteuid(getuid());
    if (bowstring)
    {
	clone_object(bowstring)->move(archer, 1);
    }
    else
    {
	clone_object("/std/bowstring")->move(archer, 1);
    }

    unstring_bow();
    
    write("You unstring your " + short() + ".\n");
    
    bystanders = FILTER_LIVE(all_inventory(environment(archer))) - ({archer});
    bystanders = filter(FILTER_IS_SEEN(archer, bystanders),
			&->can_see_in_room());

    if (sizeof(bystanders))
    {
	bystanders->catch_msg(QCTNAME(archer) + " unstrings " +
			      archer->query_possessive() + " " +
			      short() + ".\n");
    }
    
    return 1;
}

/*
 * Function name: query_hit
 * Description  : This function merely checks if the bow is
 *                stringed and delegates to launch_weapon.
 *                If the bow is not stringed it returns a hit
 *                of 10 and behaves like a normal weapon.
 */
public int
query_hit()
{
    if (stringed)
    {
	return ::query_hit();
    }
    else
    {
	return 10;
    }
}

/*
 * Function name: query_pen
 * Description  : This function merely checks if the bow is
 *                stringed and delegates to launch_weapon.
 *                If the bow is not stringed it returns a
 *                pen of 10 and behaves like a normal weapon.
 */
public int
query_pen()
{
    if (stringed)
    {
	return ::query_pen();
    }
    else
    {
	return 10;
    }
}

/*
 * Function name: try_hit
 * Description  : This function merely checks if the bow is
 *                stringed and delegates to launch_weapon.
 *                If the bow is not stringed it returns 1
 *                and behaves like a normal weapon.
 */
public int
try_hit(object target)
{
    if (stringed)
    {
	return ::try_hit(target);
    }
    else
    {
	return 1;
    }
}

/*
 * Function name: did_hit
 * Description  : This function merely checks if the bow is
 *                stringed and delegates to launch_weapon.
 *                If the bow is not stringed it returns 0
 *                and behaves like a normal weapon.
 */
public varargs int
did_hit(int aid, string hdesc, int phurt,
	object enemy,	int dt, int phit, int dam)
{
    if (stringed)
    {
	return ::did_hit(aid, hdesc, phurt, enemy, dt, phit, dam);
    }
    else
    {
	return 0;
    }
}

/*
 * Function name: extra_sanity_checks
 * Description  : Verifies that the archer and his environment are suited
 *                for aiming or shooting. This function is meant to be
 *                overloaded by subclasses.
 *                
 * Arguments    : action (string) - Action the archer performs.
 *              : args (string)   - Arguments the archer supplies.
 *
 * Returns      : 1 - Setup is sane. 0 - Setup fishy.
 */
public int 
extra_sanity_checks(string action, string args)
{
    if (stringed)
    {
	return 1;
    }
    else
    {
	write("Your bow is not strung.\n");
	return 0;
    }
}

/*
 * Function name: load_desc
 * Description  : Returns an extra description of the loaded weapon.
 *                This is meant to be overloaded in subclasses for
 *                a prettier description.
 *
 * Returns      : string - Extra description of the load status.
 */
public string
load_desc()
{
    return (query_projectile() ? "It has been drawn and loaded with " +
	    LANG_ADDART(query_projectile()->singular_short()) + "\n": "");
}

/*
 * Function name: tell_archer_no_missiles
 * Description  : Produces a message to the wielder that he is out of missiles.
 *                This function is meant to be overridden by launch_weapon
 *                implementations.
 */
public void 
tell_archer_no_missiles()
{
    tell_object(query_wielded(), "You are out of arrows!\n");
}

/*
 * Function name: tell_archer_fatigue_unload
 * Description:   Informs the archer that he is too tired to keep the
 *                bow drawn.
 * Arguments:     archer:     Archer wileding the bow.
 *                target:     The target archer was aiming at.
 *                projectile: The projectile he is about to unload.
 */
public void 
tell_archer_fatigue_unload(object archer, object target, object projectile)
{
    tell_object(archer, "You are too tired to keep the " + short() + 
		" drawn. You unload your " + short() + ".\n");
}

/*
 * Function name: tell_archer_unload
 * Description  : Produces messages to the wielder when he unloads his
 *                launch_weapon. This function is meant to be overridden
 *                by launch_weapon implementations.
 *                
 * Arguments    : archer:      The archer unloading a projectile.
 *                projectile:  The projectile he unloads.
 */
public void
tell_archer_unload(object archer, object target, object projectile)
{
    tell_object(archer, "You relax your grip on the " + short() +
		" and unload the " + projectile->singular_short() + ".\n");
}

/*
 * Function name: tell_others_unload
 * Description  : Produces messages to bystanders that the wielder unloads
 *                his launch_weapon. This function is meant to be
 *                overridden by launch_weapon implementaions.
 *                
 * Arguments    : archer:      The archer unloading a projectile.
 *                target:      The person the wilder aimed at.
 *                projectile:  The projectile he unloads.
 */
public void
tell_others_unload(object archer, object target, object projectile)
{
    object *bystanders;

    bystanders = all_inventory(ENV(archer)) - ({ archer });
    bystanders = FILTER_IS_SEEN(archer, FILTER_LIVE(bystanders));
    bystanders = filter(bystanders, &->can_see_in_room());
    
    bystanders->catch_msg(QCTNAME(archer) + " relaxes " +
			  archer->query_possessive() +
			  " grip on the " + short() + " and unloads the " +
			  projectile->singular_short() + ".\n");
}

/*
 * Function name: tell_archer_load
 * Description  : Produces messages to bystanders when the archer wields
 *                his launch_weapon. This function is meant to be
 *                overridden by launch_weapon implementations.
 *                
 * Arguments    : archer:      The archer loading a projectile.
 *                target:      The soon to be target!
 *                projectile:  Projectile beeing loaded.
 *                adj_desc:    Description of the adjecent room.
 */
public void 
tell_archer_load(object archer, object target,
		 object projectile, string adj_desc)
{
    // You nock a white-feathered arrow and draw your elven longbow, 
    // aiming carefully at the black orc.

    if (ENV(archer) == ENV(target))
    {
        tell_object(archer, "You nock " +
		    LANG_ADDART(projectile->singular_short()) +
		    " and draw your " + short() + ", aiming carefully at " + 
		    target->query_the_name(archer) + ".\n");
    }
    else
    {
        tell_object(archer, "You nock " +
		    LANG_ADDART(projectile->singular_short()) +
		    " and draw your " + short() + ", aiming carefully at " + 
		    target->query_the_name(archer) + " " + adj_desc + ".\n");
    }
}

/*
 * Function name: tell_others_load
 * Description  : Produces messages to spectators when the player loads his
 *                launch_weapon. This method is meant to be overridden by
 *                launch_weapon implementations.
 *                
 * Arguments    : archer:     The player loading his weapon.
 *                target:     The target player is aiming at.
 *                projectile: The projectile we are loading.
 *                adj_string: Description of the adjecent location.
 */
public void 
tell_others_load(object archer, object target,
		 object projectile, string adj_desc)
{
    if (ENV(archer) == ENV(target))
    {
        tell_bystanders_miss(QCTNAME(archer) + " nocks " +
			     LANG_ADDART(projectile->singular_short()) +
			     " and draws " + archer->query_possessive() +
			     " " + short() + ", aiming carefully at " +
			     QTNAME(target) + ".\n",

			     QCTNAME(archer) + " nocks " +
			     LANG_ADDART(projectile->singular_short()) +
			     " and draws " + archer->query_possessive() +
			     " " + short() +
			     ", aiming at something.\n",

			     0, 0, archer, target, ENV(archer));
    }
    else
    {
        tell_bystanders_miss(QCTNAME(archer) + " nocks " +
			     LANG_ADDART(projectile->singular_short()) +
			     " and draws " + archer->query_possessive() +
			     " " + short() +
			     ", aiming carefully at " + QTNAME(target) +
			     " " + adj_desc + ".\n",
			     
			     QCTNAME(archer) + " nocks " +
			     LANG_ADDART(projectile->singular_short()) +
			     " and draws " + archer->query_possessive() +
			     " " + short() +
			     ", aiming carefully at something " +
			     adj_desc + ".\n",
			     
			     0, 0, archer, target, ENV(archer));
    }
}		     


/*
 * Function name: tell_target_load
 * Description  : Produces a message to the player when he is loading his
 *                launch_weapon.
 *                
 * Arguments    : archer:     The player loading his weapon.
 *                target:     The target player is aiming at.
 *                projectile: The projectile we are loading.
 */
public void
tell_target_load(object archer, object target, object projectile)
{
    if (ENV(archer) == ENV(target) &&
	CAN_SEE(target, archer) && CAN_SEE_IN_ROOM(target))
    {
        tell_object(target, archer->query_The_name(target) + " nocks " +
		    LANG_ADDART(projectile->singular_short()) + 
		    " and draws " + archer->query_possessive() + " " +
		    short() + ", aiming carefully at you.\n");
    }
}

/*
 * Function name: tell_archer_miss
 * Description  : Produces a message to the player when he misses his target.
 *                This function take visual conditions in consideration as
 *                well as shoots across rooms.
 *
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stand in the same room.
 */
public void
tell_archer_miss(object archer, object target, object projectile,
		 string adj_room_desc)
{
    if (archer->query_npc() || archer->query_option(OPT_GAG_MISSES))
    {
        return;
    }
 
    if (ENV(archer) == ENV(target))
    {
        if (CAN_SEE(archer, target) && CAN_SEE_IN_ROOM(archer))
	{
	  /*
	   * You shoot an arrow at the black orc, but miss.
	   */
	    
	  tell_object(archer, "You shoot an arrow at " +
		      target->query_the_name(archer) + ", but miss.\n");
	}
	else
	{
            // You shoot blindly at the orc.
	    
	    if (archer->query_met(target))
	    {
		tell_object(archer, "You shoot blindly at " +
			    target->query_met_name() + ".\n");
	    }
	    else
	    {
		tell_object(archer, "You shoot blindly at the " +
			    target->query_race_name() + ".\n");
	    }
	}
    }
    else
    {
        if (check_remote_seen(archer, target))
	{
	    // You shoot an arrow at the black orc on the courtyard, but miss.
	    
	    tell_object(archer, "You shoot an arrow at " +
			target->query_the_name(archer) + " " +
			adj_room_desc + ", but miss.\n");
	} 
	else
	{
	    // You shoot blindly at the orc on the courtyard.

	    if (archer->query_met(target))
	    {
		tell_object(archer, "You shoot blindly at " +
			    target->query_met_name() + " " +
			    adj_room_desc + ".\n");
	    }
	    else
	    {
		tell_object(archer, "You shoot blindly at the " +
			    target->query_race_name() + " " +
			    adj_room_desc + ".\n");
	    }
	}
    }
}

/*
 * Function name: tell_target_miss
 * Description  : Produces a message to the target when the archer tries to
 *                shoot at him but miss. This function take visual
 *                conditions in consideration as well as shoots across rooms.
 *                
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stands in the same room at the archer.
 *                org_room_desc: Description of originating room. 0 if
 *                               target stands in the same room as the archer.
 */
public void
tell_target_miss(object archer, object target, object projectile,
		 string adj_room_desc, string org_room_desc)
{
    if (target->query_npc() || target->query_option(OPT_GAG_MISSES))
    {
        return;
    }
    
    if (ENV(archer) == ENV(target))
    {
        if (CAN_SEE(target, archer) && CAN_SEE_IN_ROOM(archer))
	{
	    // The tall green-clad elf shoots an arrow at you, but misses.

	    tell_object(target, archer->query_The_name(target) + 
			" shoots an arrow at you, but misses.\n");
	}
	else
	{
	    tell_object(target, "You hear the hiss of an arrow " +
			"flying past.\n");
	} 
    }
    else
    {
        if (check_remote_seen(target, archer))
	{
	    /*
	     * The tall green-clad elf shoots an arrow at you from
	     * the battlement, but misses.
	     */
	    
            tell_object(target, archer->query_The_name(target) +
			" shoots an arrow at you from " +
			org_room_desc + ", but misses.\n");
	}
	else if (CAN_SEE_IN_ROOM(target))
	{
	    tell_object(target, "Someone shoots an arrow at you," +
			" but misses.\n");
	}
	else
	{
	    tell_object(target, "You hear the hiss of an arrow " +
			"flying past.\n");
	}
    }
}

/*
 * Function name: tell_others_miss
 * Description  : Produces messages to all bystanders when the archer misses
 *                his target. This function take visual conditions in
 *                consideration as well as shoots across rooms.
 *                
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stands in the same room at the archer.
 *                org_room_desc: Description of originating room. 0 if
 *                               target stands in the same room as the archer.
 */
public void
tell_others_miss(object archer, object target, object projectile,
		 string adj_room_desc, string org_room_desc)
{
    if (ENV(archer) == ENV(target))
    {
	
	/*
	 * The tall green-clad elf shoots an arrow at the black orc,
	 * but misses.
	 */

	tell_bystanders_miss(QCTNAME(archer) + " shoots an arrow at " +
			     QTNAME(target) + ", but misses.\n",

			     QCTNAME(archer) +
			     " shoots an arrow at something.\n",

			     "An arrow flies past " + QTNAME(target) + ".\n",

			     "You hear the hiss of an arrow flying through " +
			     "the air.\n",

			     archer, target, ENV(archer));
    }
    
    else
    {
	
	/*
	 * Archer shooting to adjecent room. Archer room:
	 *
	 * The tall green-clad elf shoots an arrow at the black orc on
	 * the courtyard, but misses.	 
	 */

	tell_bystanders_miss(QCTNAME(archer) +
			     " shoots an arrow at " + QTNAME(target) +
			     " " + adj_room_desc + ", but misses.\n",

			     QCTNAME(archer) +
			     " shoots an arrow at something " +
			     adj_room_desc + ".\n",

			     "Someone shoots an arrow at " + QTNAME(target) +
			     " " + adj_room_desc + ", but misses.\n",
			     
			     "You hear the hiss of an arrow flying through " +
			     "the air.\n",

			     archer, target, ENV(archer));

	/*
	 * Archer shooting to adjecent room. Target room:
	 *
	 * The tall green-clad elf shoots an arrow at the black orc
	 * from the battlements, but misses.
	 *
	 */

	tell_bystanders_miss(QCTNAME(archer) + " shoots an arrow at " +
			     QTNAME(target) + " from " + org_room_desc +
			     ", but misses.\n",

			     QCTNAME(archer) + " shoots an arrow " +
			     " at something.\n",

			     "Someone shoots an arrow at " +
			     QTNAME(target) + ", but misses.\n",
			     
			     "You hear the hiss of an arrow flying through " +
			     "the air.\n",

			     archer, target, ENV(target));
    }

    return;
}

/*
 * Function name: tell_archer_bounce_armour
 * Description  : Produces a message to the archer when his arrow hits the
 *                target without causing any harm. This is described as
 *                the arrow bouncing off his armour.
 *                
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stands in the same room at the archer.
 *                org_room_desc: Description of originating room. 0 if
 *                               target stands in the same room as the archer.
 *                armour:        The armour that deflects the arrow. armour
 *                               may be 0 if no armour covers the hid.
 */
public void 
tell_archer_bounce_armour(object archer, object target, object projectile,
			  string adj_room_desc, object armour)
{
    string armour_desc;
    
    if (archer->query_npc() || archer->query_option(OPT_GAG_MISSES))
    {
        return;
    }

    if (armour)
    {
	armour_desc = ", but the arrow glances off " +
	    target->query_possessive() + " " + armour->short() + ".\n";
    }
    else
    {
	armour_desc = ", but the arrow glances off " +
	    target->query_objective() + " harmlessly.\n";
    }
    
	
    if (ENV(archer) == ENV(target))
    {
        if (CAN_SEE(archer, target) && CAN_SEE_IN_ROOM(archer))
	{
	    // You shoot an arrow at the black orc, but the arrow glances off
	    // his helm.

	  tell_object(archer, "You shoot an arrow at " +
		      target->query_the_name(archer) + armour_desc);
	}
	else
	{
            // You shoot blindly at the orc.
	    if (archer->query_met(target))
	    {
		tell_object(archer, "You shoot blindly at " +
			    target->query_met_name() + ".\n");
	    }
	    else
	    {
		tell_object(archer, "You shoot blindly at the " +
			    target->query_race_name() + ".\n");
	    }
	}
    }
    else
    {
        if (check_remote_seen(archer, target))
	{
	    /*
	     * You shoot an arrow at the black orc on the courtyard,
	     * but the arrow glances off his helm. 
	     */
	    tell_object(archer, "You shoot an arrow at " +
			target->query_the_name(archer) + " " +
			adj_room_desc + armour_desc);

	} 
	else
	{
	    // You shoot blindly at the orc on the courtyard.
	    if (archer->query_met(target))
	    {
		tell_object(archer, "You shoot blindly at " +
			    target->query_met_name() + " " +
			    adj_room_desc + ".\n");
	    }
	    else
	    {
		tell_object(archer, "You shoot blindly at the " +
			    target->query_race_name() + " " +
			    adj_room_desc + ".\n");
	    }
	}
    }
}

/*
 * Function name: tell_target_bounce_armour
 * Description  : Produces a message to the target when he is hit by
 *                an arrow that do no dammage. This is described as
 *                the arrow bouncing off his armour.
 *                
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stands in the same room at the archer.
 *                org_room_desc: Description of originating room. 0 if
 *                               target stands in the same room as the archer.
 *                armour:        The armour that deflects the arrow. armour
 *                               may be 0 if no armour covers the hid.
 */
public void 
tell_target_bounce_armour(object archer, object target, object projectile,
			  string adj_room_desc, string org_room_desc,
			  object armour)
{
    string armour_desc;
    
    if (target->query_npc() || target->query_option(OPT_GAG_MISSES))
    {
        return;
    }

    if (armour)
    {
	armour_desc = ", but the arrow glances off your "
	    + armour->short() + ".\n";
    }
    else
    {
	armour_desc = ", but the arrow glances off harmlessly.\n";
    }

    if (ENV(archer) == ENV(target))
    {
        if (CAN_SEE(target, archer) && CAN_SEE_IN_ROOM(archer))
	{
	    /*
	     * The tall green-clad elf shoots an arrow
	     * at you, but the arrow glances off your helm.
	     */

	    tell_object(target, archer->query_The_name(target) + 
			" shoots an arrow at you" + armour_desc);
	}

	else
	{
	    tell_object(target, "An arrow from out of nowhere hits you, " +
			"but harmlessly glances off" +
			(armour ? " your " + armour->short() + ".\n"
			 : " you.\n"));
	} 
    }
    else
    {
        if (check_remote_seen(target, archer))
	{
	    /*
	     * The tall green-clad elf shoots an arrow at you
	     * from the battlements, but the arrow glances off your helm.
	     */
	    
            tell_object(target, archer->query_The_name(target) + 
			" shoots an arrow at you" + armour_desc);
	}
	else if (CAN_SEE_IN_ROOM(target))
	{
	    tell_object(target, "Someone shoots an arrow at you from " +
			org_room_desc + armour_desc);
	}
	else
	{
	    tell_object(target, "An arrow from out of nowhere hits you, " +
			"but harmlessly glances off" +
			(armour ? " your " + armour->short() + ".\n"
			 : " you.\n"));
	}
    }
}

/*
 * Function name: tell_others_bounce_armour
 * Description  : Produces messages to bystanders when the archer's arrow
 *                harmlessly hits the target. This is described as
 *                the arrow bouncing off the target's armour.
 *                
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stands in the same room at the archer.
 *                org_room_desc: Description of originating room. 0 if
 *                               target stands in the same room as the archer.
 *                armour:        The armour that deflects the arrow. May
 *                               be 0 if no piece of armour protects the
 *                               location.
 */
public void 
tell_others_bounce_armour(object archer, object target, object projectile,
			  string adj_room_desc, string org_room_desc,
			  object armour)
{
    string armour_desc;

    if (armour)
    {
	armour_desc = ", but the arrow glances off " +
	    target->query_possessive() + " " + armour->short() + ".\n";
    }
    else
    {
	armour_desc = ", but the arrow glances off " +
	    target->query_objective() + " harmlessly.\n";
    }
    
    if (ENV(archer) == ENV(target))
    {
	
	/*
	 * The tall green-clad elf shoots an arrow at the
	 * black orc, but the arrow glances off his chainmail.
	 *
	 */

	tell_bystanders_miss(QCTNAME(archer) + " shoots an arrow at " +
			     QTNAME(target) + armour_desc,

			     QCTNAME(archer) + " shoots an arrow at " +
			     "something. You hear a thud as the arrow " +
			     "hits something.\n",

			     "An arrow hits " + QTNAME(target) + armour_desc,

			     "You hear the hiss of an arrow flying through " +
			     "the air and a thud as it hits something.\n",

			     archer, target, ENV(archer));
    }
    
    else
    {
	
	/*
	 * Archer shooting to adjecent room. Archer room:
	 *
	 * The tall green-clad elf shoots an arrow at the black orc on
	 * the courtyard, but the arrow glances off his chainmail.
	 */

	tell_bystanders_miss(QCTNAME(archer) + " shoots an arrow at " +
			     QTNAME(target) + " " + adj_room_desc +
			     armour_desc,
			     
			     QCTNAME(archer) +
			     " shoots an arrow at something" +
			     adj_room_desc + ".\n",

			     "Someone shoots an arrow at " + QTNAME(target) +
			     " " + adj_room_desc + armour_desc,
			     
			     "You hear the hiss of an arrow flying through " +
			     "the air.\n",

			     archer, target, ENV(archer));

	/*
	 * Archer shooting to adjecent room. Target room:
	 *
	 * The tall green-clad elf shoots an arrow at the black orc
	 * from the battlements, but the arrow glances off his chainmail.
	 */

	tell_bystanders_miss(QCTNAME(archer) + " shoots an arrow at " +
			     QTNAME(target) + " from " + org_room_desc +
			     armour_desc,
			     
			     QCTNAME(archer) + " shoots an arrow at " +
			     "something. " +
			     "You hear a thud as the arrow hits something.\n",

			     "Someone shoots an arrow at " +
			     QTNAME(target) + " from " + org_room_desc +
			     armour_desc,
			     
			     "You hear the hiss of an arrow flying through " +
			     "the air and a thud as it hits something.\n",

			     archer, target, ENV(target));
    }
    return;
}

/*
 * Function name: tell_archer_hit
 * Description  : Produces a message to the archer when he hits the target.
 *                This function takes visual conditions as well as shoots
 *                across rooms in consideration.
 *                
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stands in the same room at the archer.
 *                org_room_desc: Description of originating room. 0 if
 *                               target stands in the same room as the archer.
 *                hdesc:         The hitlocation description.
 *                phurt:         The %hurt made on the enemy
 *                dt:            The current damagetype
 *                dam:           The damage caused by this weapon in hit points
 *                hid:           The hitlocation we hit.
 */
public void 
tell_archer_hit(object archer, object target,
		object projectile, string adj_room_desc,
		string hdesc, int dt, int phurt, int dam, int hid)
{
    string damage_desc = query_damage_desc(archer, target, projectile, phurt);
    
    if (archer->query_npc())
    {
        return;
    }

    if (ENV(archer) == ENV(target))
    {
	if (CAN_SEE(archer, target) && CAN_SEE_IN_ROOM(archer))
	{
	    /*
	     * You shoot an arrow at the black orc.
	     * The arrows strikes into his legs.
	     */

	    tell_object(archer, "You shoot an arrow at " +
			target->query_the_name(archer) +
			". The arrow" + damage_desc +
			target->query_the_possessive_name(archer) +
			" " + hdesc + ".\n");
	}
	else
	{
            // You shoot blindly at the orc.
	    if (archer->query_met(target))
	    {
		tell_object(archer, "You shoot blindly at " +
			    target->query_met_name() +
			    ". You hear a thud as the arrow hits.\n");
	    }
	    else
	    {
		tell_object(archer, "You shoot blindly at the " +
			    target->query_race_name() +
			    ". You hear a thud as the arrow hits.\n");
	    }
	}
    }
    else
    {
        if (check_remote_seen(archer, target))
	{
	    /*
	     * You shoot an arrow at the black orc on the courtyard.
	     * The arrow hits him in the legs.
	     */
	    	
	    tell_object(archer, "You shoot an arrow at " +
			target->query_the_name(archer) + " " +
			adj_room_desc + ". The arrow" +	damage_desc +
			target->query_the_possessive_name(archer) +
			" " + hdesc + ".\n");
	} 
	else
	{
	    if (archer->query_met(target))
	    {
		tell_object(archer, "You shoot blindly at " +
			    target->query_met_name() + " " +
			    adj_room_desc + ".\n");
	    }
	    else
	    {
		tell_object(archer, "You shoot blindly at the " +
			    target->query_race_name() + " " +
			    adj_room_desc + ".\n");
	    }
	}
    }
}

/*
 * Function name: tell_target_hit
 * Description  : Produces a message to the target when he is hit by the
 *                archer. This function takes visual conditions as well as
 *                shoots across rooms in consideration.
 *                
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stands in the same room at the archer.
 *                org_room_desc: Description of originating room. 0 if
 *                               target stands in the same room as the archer.
 *                hdesc:         The hitlocation description.
 *                phurt:         The %hurt made on the enemy
 *                dt:            The current damagetype
 *                dam:           The damage caused by this weapon in hit points
 *                hid:           The hitlocation we hit.
 */
public void
tell_target_hit(object archer, object target, object projectile,
		string adj_room_desc, string org_room_desc, string hdesc,
		int dt, int phurt, int dam, int hid)
{
    string damage_desc = query_damage_desc(archer, target, projectile, phurt);

    if (target->query_npc())
    {
        return;
    }

    if (ENV(archer) == ENV(target))
    {
	if (CAN_SEE(target, archer) && CAN_SEE_IN_ROOM(archer))
	{
	    tell_object(target, archer->query_The_name(target) +
			" shoots an arrow at you. The arrow" +	damage_desc +
			"your " + hdesc + ".\n");
	}
	else
	{
	    tell_object(target, "An arrow from out of nowhere" + damage_desc +
			"your " + hdesc + ".\n");
	}
	
    }
    else
    {
	if (check_remote_seen(target, archer))
	{
	    tell_object(target, archer->query_The_name(target) +
			" shoots an arrow at you from " + org_room_desc +
			". The arrow" + damage_desc + "your " + hdesc + ".\n");
	}
	else if (CAN_SEE_IN_ROOM(target))
	{
	    tell_object(target, "Someone shoots an arrow at you from " +
			org_room_desc + ". The arrow" + damage_desc + "your " +
			hdesc + ".\n");
	}
	else
	{
	    tell_object(target, "An arrow from out of nowhere" + damage_desc +
			"your " + hdesc + ".\n");
	}
    }
}

/*
 * Function name: tell_others_hit
 * Description  : Produces messages to all bystanders when target is hit
 *                by the archer. This function tells targets in both the
 *                archer's and target's environment if they are not the
 *                same.
 *                
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stands in the same room at the archer.
 *                org_room_desc: Description of originating room. 0 if
 *                               target stands in the same room as the archer.
 *                hdesc:         The hitlocation description.
 *                phurt:         The %hurt made on the enemy
 *                dt:            The current damagetype
 *                dam:           The damage caused by this weapon in hit points
 *                hid:           The hitlocation we hit.
 */
public void
tell_others_hit(object archer, object target, object projectile,
		string adj_room_desc, string org_room_desc, string hdesc,
		int dt, int phurt, int dam, int hid)
{
    
    string damage_desc = query_damage_desc(archer, target, projectile, phurt);

        if (ENV(archer) == ENV(target))
    {
	
	/*
	 * Typical message:
	 *
	 * The tall green-clad elf shoots an arrow at the black orc.
	 * The arrows strikes deeply into the orc's legs.
	 */

	tell_bystanders_hit(QCTNAME(archer) + " shoots an arrow at " +
			    QTNAME(target) + ". The arrow" +
			    damage_desc + QTPNAME(target) + " " +
			    hdesc + ".\n",

			    QCTNAME(archer) +
			    " shoots an arrow at something. " +
			    "You hear a thud as the arrow hits something.\n",

			    "An arrow hits " + QTNAME(target) + ". The arrow" +
			    damage_desc + QTPNAME(target) +
			    " " + hdesc + ".\n",

			    "You hear the hiss of an arrow flying through " +
			    "the air and a thud as it hits something.\n",
			    
			    archer, target, ENV(archer));
    }
    
    else
    {
	
	/*
	 * Archer shooting to adjecent room. Archer room:
	 *
	 * The tall green-clad elf shoots an arrow at the black orc
	 * on the courtyard. The arrow strikes soundly into the orc's legs.
	 *
	 */

	tell_bystanders_hit(QCTNAME(archer) + " shoots an arrow at " +
			    QTNAME(target) + " " + adj_room_desc +
			    ". The arrow" + damage_desc + QTPNAME(target) +
			    " " + hdesc + ".\n",
			    
			    QCTNAME(archer) +
			    " shoots an arrow at something " +
			    adj_room_desc + ".\n",
			    
			    "Someone shoots an arrow at " +
			    QTNAME(target) + " " + adj_room_desc +
			    ". The arrow" + damage_desc + QTPNAME(target) +
			    " " + hdesc + ".\n",
			     
			    "You hear the hiss of an arrow flying through " +
			    "the air.\n",
			    
			    archer, target, ENV(archer));

	/*
	 * Archer shooting to adjecent room. Target room:
	 *
	 * The tall green-clad elf shoots an arrow at the black orc from the
	 * battlements. The arrow strikes soundly into the orc's legs.
	 *
	 */

	tell_bystanders_hit(QCTNAME(archer) + " shoots an arrow at " +
			    QTNAME(target) + " from " + org_room_desc +
			    ". The arrow" + damage_desc + QTPNAME(target) +
			    " " + hdesc + ".\n",			     

			    QCTNAME(archer) +
			    " shoots an arrow at something. " +
			    "You hear a thud as the arrow hits something.\n",

			    "Someone shoots an arrow at " + QTNAME(target) +
			    " from " + org_room_desc + ". The arrow" +
			    damage_desc + QTPNAME(target) +
			    " " + hdesc + ".\n",			     
			     
			    "You hear the hiss of an arrow flying through " +
			    "the air and a thud as it hits something.\n",
			    
			    archer, target, ENV(target));
    }
    return;
}

/*
 * Function name: query_damage_desc
 * Description  : Returns a string describing how much damage we have done.
 *                The string should have leading and tailing spaces.
 *
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                phurt:         The %hurt made on the enemy.
 * Returns      : (string) A string describing how much the arrow hurt.
 */
public string
query_damage_desc(object archer, object target, object projectile, int phurt)
{    
    switch (phurt)
    {
      case 0..2:
	return " merely glances off of ";
	break;          
      case 3..5:
	return " grazes ";
	break;
      case 6..9:
	return " hits ";
	break;
      case 10..19:
	return " strikes ";
	break;
      case 20..39:
	return " hits soundly into ";
	break;
      case 40..59:
	return " strikes solidly into ";
	break;
      case 60..90:
	return " drives deeply into ";
	break;
      default:
	return " strikes with devastating precision into ";
	break;
    }
}


/*
 * Function name: query_wep_recover
 * Description:   Return the recover strings for changing weapon variables.
 * Returns:       A recover string
 */
string
query_wep_recover()
{
    return ::query_wep_recover() + "BOW#" + bowstring + "#" + stringed +
	"#" + query_keep_recover() + "#";
}

/*
 * Function name: init_wep_recover
 * Description:   Initialize the weapon variables at recover.
 * Arguments:     arg - String with variables to recover
 */
void
init_wep_recover(string arg)
{
    string wep_str, recover_str, tail;
    
    sscanf(arg, "%sBOW#%s#%d#%s#%s",
	   wep_str, bowstring, stringed, recover_str, tail);

    init_keep_recover(recover_str);
    ::init_wep_recover(wep_str);
}

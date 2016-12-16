/*
 * /std/launch_weapon.c
 *
 * This class is the base for all weapons that launch
 * projectiles. The only known subclass are /std/bow.c,
 * but it can be inherited to create other weapon types
 * such as crossbows or slings. You should probably not
 * inherit this class directly unless you are creating
 * a new weapon type, but use one of its known subclasses
 * instead.
 *
 */

#pragma save_binary
#pragma strict_types

inherit "/std/weapon";

#include <composite.h>
#include <language.h>
#include <macros.h>
#include <options.h>
#include <stdproperties.h>
#include <wa_types.h>
#include <filter_funs.h>
#include <ss_types.h>
#include <formulas.h>
#include "/std/combat/combat.h"

#define ENV(x) environment(x)

string Valid_projectile,    /* The type of projectiles we use. */
       Shoot_command,       /* The command word used to shoot. */
       Aim_command,         /* The command word used to aim. */
       Fire_command,        /* The command word used to fire. */ 
       Unload_command,      /* The command word used to unload. */
       Adj_room_desc,       /* Description of room we shoot too. */
       Org_room_desc,       /* Description of room we shoot from. */
       Sec_wep_cmd;         /* The command to wield the secondary weapon. */

object Projectile_stack,    /* The currently used stack of projectiles. */
       Projectile,          /* The currently loaded projectile. */
       Archer,              /* A sneeky chap in green tights. */
       Target,              /* The current target. */
       Archer_env,          /* The archer's environment. */
       Target_env;          /* The target's environment. */

mixed  Hitloc_id;           /* The hitloc we aim on. */

int    Shoot_alarm,         /* Alarm used between aim and fire. */
       Fatigue_alarm,       /* Alarm used to unload a bow if not fired. */
       Ready,               /* Flag signifying we have aimed and can shoot. */
       Next_round,          /* The time we can shoot once more. */       
       Drawn_wep;           /* Flag signifying that is weapon must be held
			     * in drawn position by hand. A bow is a drawn
			     * weapon, a crossbow is not.            
			     */

// Prototypes

public int shoot(string args);
public int aim(string args);
public int fire(string args);
public int unload(string args);
public int secondary_wep_cmd(string args);
public int extra_sanity_checks(string action, string args);
private void ready_to_fire();
public void do_fire();
public nomask void set_secondary_wep_cmd(string command);
public nomask string query_secondary_wep_cmd();
private nomask void wield_secondary_weapon();
private nomask int setup_sane(string action, string args);
public nomask void find_projectiles();
public nomask void load_projectile();
public nomask void unload_projectile();
public nomask int try_load();
public nomask object find_protecting_armour(object target, int hid);
private nomask void reset_launch_weapon();
private nomask void fatigue_unload();
public void move_projectile_to_env(object location, object projectile);
public void move_projectile_to_target(object target, object projectile);
public void tell_archer_no_missiles();
public void tell_archer_fatigue_unload(object archer, object target, object projectile);
public void tell_archer_unload(object archer, object target, object projectile);
public void tell_others_unload(object archer, object target, object projectile);
public void tell_all_projectile_break(object projectile, object target);
public void tell_archer_miss(object archer, object target, object projectile, string adj_room_desc);
public void tell_target_miss(object archer, object target, object projectile, string adj_room_desc, string org_room_desc);
public void tell_others_miss(object archer, object target, object projectile, string adj_room_desc, string org_room_desc);
public void tell_archer_bounce_armour(object archer, object target, object projectile, string adj_room_desc, object armour);
public void tell_target_bounce_armour(object archer, object target, object projectile, string adj_room_desc, string org_room_desc, object armour);
public void tell_others_bounce_armour(object archer, object target, object projectile, string adj_room_desc, string org_room_desc, object armour);
public void tell_archer_hit(object archer, object target, object projectile, string adj_room_desc, string hdesc, int dt, int phurt, int dam, int hid);
public void tell_target_hit(object archer, object target, object projectile, string adj_room_desc, string org_room_desc, string hdesc, int dt, int phurt, int dam, int hid);
public void tell_others_hit(object archer, object target, object projectile, string adj_room_desc, string org_room_desc, string hdesc, int dt, int phurt, int dam, int hid);
public void tell_archer_load(object archer, object target, object projectile, string adj_desc);
public void tell_others_load(object archer, object target, object projectile, string adj_desc);
public void tell_target_load(object archer, object target, object projectile);
nomask void tell_bystanders_miss(string see_both, string see_archer, string see_target, string see_noone, object archer, object target, object env);
private int filter_see_blood(object person);
private int filter_gag_misses(object person);

/*
 * Function name: create_launch_weapon
 * Description  : Create the weapon. You must define this function to
 *                construct the launch weapon.
 */
public void
create_launch_weapon()
{
}

/*
 * Function name: create_weapon
 * Description  : Create the launch_weapon. As this function is declared
 *                nomask you must use the function launch_create_weapon 
 *                to actually construct it. This function does some basic
 *                initializing.
 */
public nomask void
create_weapon()
{
    Shoot_command = "shoot";
    Aim_command = "aim";
    Fire_command = "fire";
    Unload_command = "unload";

    create_launch_weapon();

    set_hands(W_BOTH);
    set_wt(W_MISSILE);
    set_wf(this_object());
}

/*
 * Function name: init
 * Description  : Adds the aim, fire, shoot and unload commands to the player.
 */
public void
init()
{
    ::init();
    add_action(shoot, Shoot_command);
    add_action(aim, Aim_command);
    add_action(fire, Fire_command);
    add_action(unload, Unload_command);
    add_action(secondary_wep_cmd, "secondary");
}

/*
 * Function name: leave_env
 * Description  : This function is called each time this object leaves an
 *                old environment. If you mask it, be sure that you
 *                _always_ call the ::leave_env(dest, old) function.
 * Arguments    : object old  - the location we are leaving.
 *                object dest - the destination we are going to. Can be 0.
 */
public void
leave_env(object from, object to)
{
    Sec_wep_cmd = 0;
    ::leave_env(from, to);
}

/*
 * Function name: set_shoot_command;
 * Description  : Sets the command used to aim and fire this launch weapon.
 *                
 * Arguments    : String command - Command that is used to aim and fire 
 *                this weapon.
 */
public nomask void
set_shoot_command(string command)
{
    Shoot_command = command;
}

/*
 * Function name: set_aim_command;
 * Description  : Sets the command used to aim this launch weapon.
 *                
 * Arguments    : String command - Command that is used to aim this weapon.
 */
public nomask void
set_aim_command(string command)
{
    Aim_command = command;
}

/*
 * Function name: set_fire_command;
 * Description  : Sets the command used to fire this launch weapon.
 *                
 * Arguments    : String command - Command that is used to fire this weapon.
 */
public nomask void
set_fire_command(string command)
{
    Fire_command = command;
}

/*
 * Function name: set_unload_command;
 * Description  : Sets the command used to unload this launch weapon.
 *                
 * Arguments    : String command - Command that is used to unload this weapon.
 */
public nomask void
set_unload_command(string command)
{
    Unload_command = command;
}

/*
 * Function name: set_valid_projectile_function
 * Description  : Sets the name of the function used to filter valid 
 *                projectiles that can be used with this weapon.
 *                
 * Arguments    : String function_name
 */

public nomask void
set_valid_projectile_function(string function_name)
{
    Valid_projectile = function_name;
}

/*
 * Function name: set_drawn
 * Description  : Sets that this weapon is held in a loaded position by
 *                the archer. Bows are typically drawn weapons, while
 *                crossbows are not.
 *                
 * Arguments    : int 1 - Drawn, 0 - Not drawn.
 */

public nomask void
set_drawn_weapon(int drawn)
{
    Drawn_wep = drawn;
}

/*
 * Function name: query_valid_projectiles
 * Description  : Uses the function name set with set_valid_projectile to
 *                find the valid projetiles in the specified container.
 *                This function does only search the specified container
 *                not any containers within the specified container.
 *                
 * Arguments    : object container - Container to search for projectiles.
 * Returns      : An array of the valid projectiles found in this container. 
 */
public nomask object *
query_valid_projectiles(object container)
{
    if (!objectp(container) || !stringp(Valid_projectile))
    {
	return 0;
    }

    if (container->query_prop(CONT_I_CLOSED))
    {
	return 0;
    }
    
    return filter(filter(all_inventory(container),
			 &call_other( , Valid_projectile)),
		  &not() @ &->query_broken());
}

/*
 * Function name: set_projectile_stack
 * Description  : Set which stack of projectile we draw projectiles from
 *                when we shoot.
 *                
 * Arguments    : object ps - The stack of projectiles to use.
 */
public nomask void
set_projectile_stack(object ps)
{
    Projectile_stack = ps;
    query_wielded()->update_weapon(this_object());
}

/*
 * Function name: query_projectile_stack
 * Description  : Returns the projectile stack we use to draw projectiles
 *                from when we shoot.
 *
 * Returns      : object - The currently used projectile stack.
 */
public nomask object
query_projectile_stack()
{
    return Projectile_stack;
}

/*
 * Function name: query_projectile
 * Description  : Returns the projectile we currently have loaded.
 *
 * Returns      : object - The currently loaded projectile or 0 if none.
 *                is loaded.
 */
public nomask object
query_projectile()
{
    return Projectile;
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
    return (Projectile ? "It has been loaded with " +
	    LANG_ADDART(Projectile->singular_short()) + "\n": "");
}

/*
 * Function name: secondary_wep_cmd
 * Description  : Parses the input from the player when he wants to set the
 *                command to perform to wield his secondary weapon.
 *                
 * Arguments    : string args - User input.
 * Returns      :  1  - Successful parse.
 *                 0  - No match.
 */
public int
secondary_wep_cmd(string args)
{
    if (!args)
    {
	if (Sec_wep_cmd)
	{
	    notify_fail("secondary <command to wield secondary weapon>\n" +
			"Currently set to: '" + Sec_wep_cmd + "'\n");
	}
	else
	{
	    notify_fail("secondary <command to wield secondary weapon>\n" +
			"Example: secondary draw sword from scabbard\n");
	}
	
	return 0;
    }
    
    set_secondary_wep_cmd(args);
    write("Ok. Set to '" + args + "'.\n");

    return 1;
}


/*
 * Function name: set_secondary_wep_cmd
 * Description  : Sets the command to wield the secondary weapon to use
 *                when we run out of projectiles or use the launch_weapon
 *                for parries.
 *                
 * Arguments    : string - command to wield secondary weapon.
 */
public nomask void
set_secondary_wep_cmd(string command)
{
    Sec_wep_cmd = command;
}

/*
 * Function name: query_secondary_wep_cmd
 * Description  : Gives the command to use when we want to wield a secondary
 *                weapon.
 *                
 * Returns      :  string - command to wield secondary weapon.
 */
public nomask string
query_secondary_wep_cmd()
{
    return Sec_wep_cmd;
}


/*
 * Function name: parse_aim
 * Description  : Parses the user input for the aim and shoot commands.
 *                
 * Arguments    : string args - User input.
 * Returns      :  1  - Successful parse.
 *                 0  - No match.
 *                -1  - Faulty input.
 */
private nomask int
parse_aim(string args)
{
    object room, *people;
    mixed livings, hitloc, *hitlocs, *other_rooms;
    string pattern, location;
    int i;

    if (!args)
    {
        notify_fail(capitalize(query_verb()) + " at whom [where]?\n");
        return 0;
    }

    if (!query_wielded())
    {
	notify_fail("Wield the " + short() + " first.\n");
	return 0;
    }
    
    Archer = query_wielded();
    people = FILTER_CAN_SEE(FILTER_LIVE(all_inventory(ENV(Archer))), Archer) - 
      ({ Archer });
    Archer_env = ENV(Archer);

    // Parse patterns matching: shoot black orc
    if (parse_command(args, people, " [at] [the] %l ", livings) &&
	pointerp(livings))
    {
        if (livings[0] == 0)
        {
	    tell_object(Archer, "You must select one target.\n");
	    return -1;
	}

	Target = livings[ABS(livings[0])];
	Target_env = ENV(Target);
	Adj_room_desc = 0;
	Org_room_desc = 0;
	hitloc = 0;
	return 1;
    }
    
    // Perhapps the target stands in an adjecent room?
    if ((other_rooms = Archer_env->query_range_targets(Archer)))
    {
        // Loop through the adjecent rooms.
        for (i = 0; i < sizeof(other_rooms); i += 4)
        {
	    room = other_rooms[i];
	    pattern = other_rooms[i + 1];

	    // Skip the room if we can't see in it.
	    if (!room || (room->query_prop(OBJ_I_LIGHT) +
			  (Archer->query_prop(LIVE_I_SEE_DARK)) <= 0))
	    {
	        continue;
	    }
	    
	    if (parse_command(args, 
			      FILTER_CAN_SEE(FILTER_LIVE(room->subinventory()),
					     Archer),
			      " [at] [the] %l " + pattern + " [in] [the] %s ",
			      livings, location) && pointerp(livings))
	    {
	        if (livings[0] == 0)
		{
		    tell_object(Archer, "You must select one target.\n");
		    return -1;
		}

		Target = livings[ABS(livings[0])];
		Adj_room_desc = other_rooms[i + 2];
		Org_room_desc = other_rooms[i + 3];
		Target_env = ENV(Target);

		if (strlen(location))
		{
		    hitlocs = Target->query_combat_object()->query_hitloc_id();

		    if (!sizeof(hitlocs))
		    {
		        tell_object(Archer, "You don't find any " + location +
				    " on " + Target->query_the_name(Archer) +
				    " to " + query_verb() + " at.\n");
			return -1;
		    }
	
		    for (i = 0; i < sizeof(hitlocs); i++)
		    {
		        hitloc = Target->query_combat_object()->query_hitloc(hitlocs[i]);
	    
			if (location == hitloc[2])
			{
                            Hitloc_id = hitlocs[i];
		            break;
			}
		    }	
	
		    if (i >= sizeof(hitlocs))
		    {
		        tell_object(Archer, "You don't find any " + location +
				    " on " + Target->query_the_name(Archer) +
				    " to " + query_verb() + " at.\n");
			return -1;
		    }
		}

		return 1;
	    }
	}
    }

    // Parse patterns matching: shoot black orc in the head
    if (parse_command(args, people, " [at] [the] %l 'in' [the] %s ",
		      livings, location) && pointerp(livings))
    {
        if (livings[0] == 0)
        {
	    tell_object(Archer, "You must select one target.\n");
	    return -1;
	}

	Target = livings[ABS(livings[0])];
	hitlocs = Target->query_combat_object()->query_hitloc_id();

	if (!sizeof(hitlocs))
	{
            tell_object(Archer, "You don't find any " + location + " on " +
			Target->query_the_name(Archer) +
			" to " + query_verb() + " at.\n");
	    return -1;
	}
	
	for (i = 0; i < sizeof(hitlocs); i++)
	{
            hitloc = Target->query_combat_object()->query_hitloc(hitlocs[i]);
	    
	    if (location == hitloc[2])
	    {
                Hitloc_id = hitlocs[i];
	        break;
	    }
	}	
	
	if (i >= sizeof(hitlocs))
	{
	    tell_object(Archer, "You don't find any " + location + " on " + 
			Target->query_the_name(Archer) +
			" to " + query_verb() + " at.\n");
	    return -1;
	}

	Target_env = ENV(Target);
	Adj_room_desc = 0;
	Org_room_desc = 0;
	return 1;
    }

    notify_fail(capitalize(query_verb()) + " at whom [where]?\n");
    return 0;
}

/*
 * Function name: shoot
 * Description  : Handles a single shot from a player.
 *                
 * Arguments    : string - Person we shoot at.
 * Returns      : 1 - command handled, 0 parse failed.
 */
public int
shoot(string args)
{

    object archer = query_wielded();

    if (!living(ENV(this_object())) || query_prop(OBJ_I_BROKEN))
    {
        return 0;
    }
    
    if (Ready)
    {
        return fire(args);
    }
    
    switch (parse_aim(args))
    {
    case 0:
      return 0;
    case -1:
      return 1;
    }
   
    // Run the usual sanity checks.
    if (!setup_sane(Shoot_command, args))
    {
        return 1;
    }

    // Run some extra sanity checks.
    if (!this_object()->extra_sanity_checks(Shoot_command, args))
    {
        return 1;
    }
    
    // Thou shallt possess missiles.
    if (!Projectile)
    {
	try_load();
	
	if (!Projectile)
	{
	    this_object()->tell_archer_no_missiles();
	    return 1;
	}
    }

    this_object()->tell_archer_load(archer, Target, Projectile, Adj_room_desc);

    this_object()->tell_others_load(archer, Target, Projectile, Adj_room_desc);

    this_object()->tell_target_load(archer, Target, Projectile);

    archer->query_combat_object()->cb_calc_speed();
    
    Shoot_alarm = set_alarm(archer->query_combat_object()->cb_query_speed(),
			    0.0, &do_fire());
    return 1;
}  

/*
 * Function name: aim
 * Description  : Handles when the archer aims this weapon.
 *                
 * Arguments    : string - Person we aim at.
 * Returns      : 1 command handled, 0 parse failed.
 */
public int
aim(string args)
{
    object archer = query_wielded();

    if (!living(ENV(this_object())) || query_prop(OBJ_I_BROKEN))
    {
        return 0;
    }

    switch (parse_aim(args))
    {
    case 0:
      return 0;
    case -1:
      return 1;
    }

    // Run the usual sanity checks.
    if (!setup_sane(Aim_command, args))
    {
        return 1;
    }

    // Run some extra sanity checks.
    if (!this_object()->extra_sanity_checks(Shoot_command, args))
    {
        return 1;
    }

    // Thou shallt possess missiles.
    if (!Projectile)
    {
	try_load();
	
	if (!Projectile)
	{
	    this_object()->tell_archer_no_missiles();
	    return 1;
	}
    }
         
    this_object()->tell_archer_load(archer, Target, Projectile, Adj_room_desc);
    
    this_object()->tell_others_load(archer, Target, Projectile, Adj_room_desc);

    this_object()->tell_target_load(archer, Target, Projectile);
    
    archer->query_combat_object()->cb_calc_speed();
    
    Shoot_alarm = set_alarm(archer->query_combat_object()->cb_query_speed(),
			    0.0, &ready_to_fire());

    return 1;
}

/*
 * Function name: fire
 * Description  : Fires an missile when archer has aimed.
 *                
 * Arguments    : string - Dummy string.
 * Returns      : 1 - fired, 0 command not handled.
 */
public int
fire(string args)
{
    if (this_player() != query_wielded())
    {
        return 0;
    }

    if (args)
    {
        notify_fail("Just " + query_verb() + " would suffice to " + 
		    query_verb() + " the " + short() + ".\n");
	return 0;
    }
  
    if (Ready)
    {
        do_fire();
    }
    else
    {
        tell_object(this_player(), "You are not ready to " + query_verb() +
		    " yet.\n");
    }
  
    return 1;
}

/*
 * Function name: unload
 * Description  : Lets the player unload a missile.
 *                
 * Arguments    : string - Player input.
 * Returns      : 1 command handled, 0 parse failed.
 */
public int
unload(string args)
{
    if (this_player() != query_wielded())
    {
        return 0;
    }

    if (args)
    {
        notify_fail("Just " + Unload_command + " would suffice to " + 
		    Unload_command + " the " + short() + ".\n");
	return 0;
    }

    if (Projectile)
    {
        unload_projectile();
    }
    else
    {
        // Replace with something prettier.
        tell_object(this_player(), "You don't have anything to " + 
		    Unload_command + ".\n");
    }
    return 1;
}

/*
 * Function name: ready_to_fire
 * Description  : Tells the player he is ready to fire and enables fire.
 *                
 */
private void
ready_to_fire()
{
    if (Projectile)
    {
        Ready = 1;
	tell_object(query_wielded(),
		    "You are ready to " + Fire_command + ".\n");
    }
    
    Shoot_alarm = 0;
}

/*
 * Function name: setup_sane
 * Description  : Verifies that the archer and his environment are suited
 *                for aiming or shooting.
 *
 * Arguments    : action (string) - Action the archer performs.
 *              : args (string)   - Arguments the archer supplies.
 *                
 * Returns      : 1 - Setup is sane. 0 - Setup fishy.
 */
private nomask int 
setup_sane(string action, string args)
{
    mixed tmp;

    // Thou shalt be alive.

    if (Archer->query_ghost())
    {
        tell_object(Archer, "You can not do that. You are... eh. Dead!\n");
        return 0;
    }

    // Thou shalt not shoot at the dead.

    if (Target->query_ghost())
    {
        if (Target)
	{
            tell_object(Archer, "No need to do that. " + 
			Target->query_The_name(Archer) +
			" is already dead.\n");
	}
	else
	{
	    tell_object(Archer, "Your target is already dead.\n");
	}

        return 0;
    }

    // Neither at the nonpresent link dead.

    if (interactive(Target) && Target->query_linkdead())
    {
        tell_object(Archer, "You will have to wait for " +
		    Target->query_the_name(Archer) + 
		    " to return from link death first.\n");
	return 0;
    }

    // Thou shalt not be stunned.

    if (Archer->query_prop(LIVE_I_STUNNED))
    {
        tell_object(Archer, "You can not " + action + 
		    " when you are stunned.\n");
        return 0;
    }

    // Thou shall wield your bow.

    if (Archer != query_wielded())
    {
        tell_object(Archer, "You must wield your " + short() + " first.\n");
        return 0;
    }


    // Thou shalt have the skill.

    if (interactive(Archer) && !Archer->query_skill(SS_WEP_MISSILE))
    {
        tell_object(Archer, "You lack the training for such an " +
		    "endeavour, you would not hit a barn from the inside.\n");
        return 0;
    }
    
    // Thou shalt not kill on sacred ground.

    if ((tmp = environment(Target)->query_prop(ROOM_M_NO_ATTACK)) ||
	(tmp = environment(Archer)->query_prop(ROOM_M_NO_ATTACK)))
    {
        if (stringp(tmp))
            tell_object(Archer, tmp);
        else
            tell_object(Archer, "You sense a divine force preventing " +
			"your attack.\n");
        return 0;
    }

    // Nor harass sacred ones.

    if (tmp = Target->query_prop(OBJ_M_NO_ATTACK))
    {
        if (stringp(tmp))
        {
            tell_object(Archer, tmp);
        }
        else
        {
            tell_object(Archer, "You feel a divine force protecting this " + 
			"being, your attack fails.\n");
        }

        return 0;
    }

    // Thou shalt be brave enough.

    if (!F_DARE_ATTACK(Archer, Target))
    {
        tell_object(Archer, "Umm... no. You do not have enough " + 
		    "self-discipline to dare!\n");
        return 0;
    }

    // Tho shalt not shoot whilst shooting.

    if (Shoot_alarm)
    {
        tell_object(Archer, "You are busy " + Aim_command + "ing at " + 
		    Target->query_the_name(Archer) + ".\n");
	return 0;
    }

    if ((tmp = Archer->query_attack()))
    {
        tell_object(Archer, "You are busy " + Shoot_command + "ing at " + 
		    tmp->query_the_name(Archer) + ".\n");
	return 0;
    }

    if (time() <= Next_round)
    {
        tell_object(Archer, "You have to relax for a bit longer before " + 
		    "you can " + action + " again.\n");
	return 0;
    }

    // Thou shall not shoot your friends unwillingly.
    if (!Archer->query_npc() && Archer->query_met(Target) &&
        (Archer->query_prop(LIVE_O_LAST_KILL) != Target))
    {
        Archer->add_prop(LIVE_O_LAST_KILL, Target);
        /* Only ask if the person did not use the real name of the target. */
        if (lower_case(args != Target->query_real_name()))
        {
            tell_object(Archer, "Attack " +
			Target->query_the_name(Archer) +
			"?!? Please confirm by trying again.\n");
            return 0;
        }
    }

    return 1;
}

/*
 * Function name: do_fire
 * Description  : Perfroms the actual shot in single shot mode. This function
 *                is more or less and modified version of heart_beat() in
 *                /std/combat/cbase.c
 */
public void 
do_fire()
{
    object combat_ob;
    mixed fail, hitresult;
    int hitsuc, ftg, crit, pen, dt, attack_location;
    mixed tmp;

    // Some sanity checks again.
    if (!objectp(Archer) || Archer != query_wielded())
    {
        if (Projectile)
	{
            Projectile->unload();
	    Projectile->move(ENV(this_object()), 1);
	}

	reset_launch_weapon();
	return;
    }

    /*
     * If we are in combat we silently return and let combat 
     * take care of the shooting.
     */
    if (Archer->query_attack())
    {
        return;
    }     

    // Archer should not move while aiming.
    if (ENV(Archer) != Archer_env)
    {
        tell_object(Archer, "The movment made you lose your target.\n");
	unload_projectile();
	return;
    }

    // The target should not move away from the archer while he is aiming.

    if (!(Target && (ENV(Target) == Target_env || ENV(Target) == Archer_env)))
    {
        tell_object(Archer, "Your target has dissapeared.\n");
	unload_projectile();
        return;
    }

    // The archer should be alive.

    if (Archer->query_ghost())
    {
        tell_object(Archer, "You can not do that. You are... eh. Dead!\n");
	unload_projectile();
        return;
    }

    // Thou shalt not shoot at the dead.

    if (Target->query_ghost())
    {
        if (Target)
	{
            tell_object(Archer, "You give up shooting at " + 
			Target->query_The_name(Archer) + " since " +
			Target->query_pronoun() + " is already dead.\n");
	}
	else
	{
	    tell_object(Archer, "Your target is already dead.\n");
	}

	unload_projectile();
        return;
    }

    // Neither at the nonpresent link dead.

    if (interactive(Target) && Target->query_linkdead())
    {
        tell_object(Archer, "You will have to wait for " +
		    Target->query_the_name(Archer) + 
		    " to return from link death first.\n");
	unload_projectile();
	return;
    }

    // Thou shalt not be stunned.

    if (Archer->query_prop(LIVE_I_STUNNED))
    {
        tell_object(Archer, "You can not " + Shoot_command +
		    " when you are stunned.\n");
        return;
    }

    /*
     * Ok, we are set to at least try to hit the target as it is done in
     * heart_beat()
     */

    combat_ob = Archer->query_combat_object();
    combat_ob->cb_calc_speed();

    // First do some check if we actually attack.

    if (pointerp(fail = Archer->query_prop(LIVE_AS_ATTACK_FUMBLE)) &&
        sizeof(fail))
    {
        if (!Archer->query_npc())
        {
            Archer->catch_msg(fail[0]);
        }

	set_alarm(combat_ob->cb_query_speed(), 0.0, &do_fire());
        return;
    }

    if ((tmp = Archer->query_prop(LIVE_I_ATTACK_DELAY)))
    {
        if ((tmp -= ftoi(combat_ob->cb_query_speed())) > 0)
        {
            Archer->add_prop(LIVE_I_ATTACK_DELAY, tmp);
	    set_alarm(combat_ob->cb_query_speed(), 0.0, &do_fire());
            return;
        }
        else
        {
            Archer->remove_prop(LIVE_I_ATTACK_DELAY);
	}
    }

    if (Archer->query_prop(LIVE_I_STUNNED))
    {
        set_alarm(combat_ob->cb_query_speed(), 0.0, &do_fire());
        return;
    }

    if (Archer->query_prop(LIVE_I_CONCENTRATE))
    {
        tell_object(Archer, 
		    "You too busy with other things to shoot right now.\n");
	unload_projectile();
        return;
    }

    // Mark this moment as being in combat.
    combat_ob->cb_update_combat_time();
    Next_round = time() + ftoi(combat_ob->cb_query_speed());

    if (ENV(Archer) == ENV(Target))
    {
        Archer->attack_object(Target);
        tell_object(Archer,
		    "You attack " + Target->query_the_name(Archer) +".\n");
	tell_object(Target, Archer->query_The_name(Target) +
		    " attacks you.\n");
	tell_room(environment(Archer), QCTNAME(Archer) + " attacks " +
		  QTNAME(Target) + ".\n", ({ Archer, Target }), Archer);
    }
    else
    {
        // Give a small message even if OPT_GAG_MISSES is on.	    
        if (Archer->query_option(OPT_GAG_MISSES))
	{
            tell_object(Archer, "You shoot at " +
			Target->query_the_name(Archer) +".\n");
	}
    }
	    
    
    // This fine piece of code finds the attack location of our launch_weapon.

    attack_location = filter(combat_ob->query_attack_id(),
			     &operator(==)(, this_object()) @
			     &operator([])(, 6) @
			     &combat_ob->query_attack())[0];


    /*
     * The attack has a chance of failing. If for example the attack
     * comes from a wielded weapon, the weapon can force a fail or
     * if the wchit is to low for this opponent.
     */

    set_this_player(Archer);
    hitsuc = combat_ob->cb_try_hit(attack_location);
    
    // Make sure there is a proper delay where you can not fire after you
    // have fired an arrow.
    // And make sure you are not attacked if the target can not see you.

    if (hitsuc <= 0)
    {
        return;
    }

    /*
     * The intended victim can also force a fail. like in the weapon
     *  case, if fail, the cause must produce explanatory text himself.
     */

    hitsuc = Target->query_not_attack_me(Archer, attack_location);

    if (hitsuc > 0)
    {
        return;
    }
    
    hitsuc = combat_ob->cb_tohit(attack_location, query_hit(), Target);

    if (hitsuc > 0)
    {
        // [0] here means we allways use the impale pen.
        pen = combat_ob->query_attack(attack_location)[ATT_M_PEN][0];
  
        // Choose one damage type //
        if (crit = (!random(10000)))
	{
	    // Critical hit!
	    pen *= 5;
	}

	dt = W_IMPALE;
	
	if (Hitloc_id)
	{
            hitresult = (mixed*)Target->hit_me(pen, dt, Archer,
					       attack_location, Hitloc_id);
	}
	else
	{
            hitresult = (mixed*)Target->hit_me(pen, dt, Archer,
					       attack_location);
	}

	if (crit)
	{
	    log_file("CRITICAL", sprintf("%s: %-11s on %-11s " +
					 "(dam = %4d(%4d))\n\t%s on %s\n",
					 ctime(time()),
					 Archer->query_real_name(),
					 Target->query_real_name(),
					 hitresult[3], pen,
					 file_name(Archer),
					 file_name(Target)), -1);
	}
    }
    else
    {
        hitresult = (mixed*)Target->hit_me(hitsuc, 0, Archer, attack_location);
    }

    /*
     * Generate combat message, arguments Attack id, hitloc description
     * proc_hurt, Defender
     */
    if (hitsuc > 0)
    {
	hitsuc = query_pen();
	if (hitsuc > 0)
	{
	    hitsuc = 100 * hitresult[2] / hitsuc;
	}
	else
	{
	    hitsuc = 0;
	}
    }

    if (hitresult[1])
    {
	combat_ob->cb_did_hit(attack_location, hitresult[1], hitresult[4],
			      hitresult[0], Target, dt, hitsuc, hitresult[3]);
    }
    
    if (!Target || Target->query_ghost())
    {
        tell_object(Archer, Target->query_The_name(Archer) + 
		    " is allready dead.\n");
	unload_projectile();
    }
    
    // Oops, Lifeform turned into a deadform. Reward the killer. //
    if ((int)Target->query_hp() <= 0)
    {
        Target->do_die(Archer);
    }
    
    /*
     * Fighting is quite tiresome you know
     */
    
    ftg = random(3) + 1;
    if (Archer->query_fatigue() >= ftg)
    {
        Archer->add_fatigue(-ftg);
    }
    else
    {
        tell_object(Archer,
		    "You are so tired that every move drains your health.\n");
        Archer->set_fatigue(0);
        Archer->reduce_hit_point(ftg);
    }
    
    /*
     * Fighting is frightening, we might panic!
     */
    combat_ob->cb_may_panic();
    
    return;
}

/*
 * Function name: try_load
 * Description  : Tries to load the launch_weapon with a projectile from
 *                the archer's inventory.
 * Returns      : int 1 - Success. 0 - Failed.
 */
public nomask int
try_load()
{
    object stack_env;
    
    if (!objectp(Projectile_stack) && !find_projectiles())
    {
        return 0;
    }

    set_this_player(query_wielded());
    stack_env = ENV(Projectile_stack);
    
    if (stack_env == query_wielded() ||
	(stack_env->query_prop(CONT_I_IS_QUIVER) &&
	 !stack_env->query_prop(CONT_I_CLOSED) &&
	 ENV(stack_env) == query_wielded()))
    {
	    // We still have the stack present.
	    load_projectile();
	    return 1;
    }
    else
    {
        // We do not have projectiles nearby anymore.
        if (!find_projectiles())
	{
            return 0;
	}
	else
	{
	    return 1;
	}
    }
}

/*
 * Function name: long
 * Description  : The long description. We add the information about the
 *                load status and the condition to it.
 * Arguments     : string str - a possible add-item to look for.
 *                 object for_obj - the object that wants to know.
 * Returns       : string - the long description.
 */
public varargs string
long(string str, object for_obj)
{
    if (str)
    {
        return ::long(str, for_obj);
    }
    if (obj_long)
    {
        return check_call(obj_long, for_obj) + 
            this_object()->load_desc() + wep_condition_desc();
    }
    return "You see a non-descript object.\n";
}

/*
 * Function name: try_hit
 * Description  : Try hit is called from the combat object when the archer
 *                fights. Since a bow only shoot every second round it
 *                tries to load a missile first round and fires the missile
 *                the second round.
 *                
 * Arguments    : object - target we shoot at.
 * Returns      : int - 1 Fire. 0 Load (or try to load).
 */
int 
try_hit(object target) 
{
    ::try_hit();

    Archer = query_wielded();
    
    if (!Archer)
    {
        return 0;
    }
 
    if (!Projectile)
    {
        if (try_load())
        {
	    this_object()->tell_archer_load(query_wielded(), target,
					    Projectile, "");
	    
	    this_object()->tell_others_load(query_wielded(), target,
					    Projectile, "");
	    
	    this_object()->tell_target_load(query_wielded(), target,
					    Projectile);
	    return 0;	    
	}
	else
	{
	    this_object()->tell_archer_no_missiles();
	    set_this_player(query_wielded());
	    unwield_me();
	    if (Sec_wep_cmd)
	    {
		wield_secondary_weapon();
	    }
	}
    }
    else
    {
        Next_round = time() + 
	  ftoi(query_wielded()->query_combat_object()->cb_query_speed());
        return 1;
    }
}


/*
 * Function name: wield_secondary_weapon
 * Description  : Commands the player to wield the secondary weapon he has
 *                specified.
 */
private nomask void
wield_secondary_weapon()
{
    this_player()->command("$" + Sec_wep_cmd);
}


/*
 * Function name: load_projectile
 * Description  : Loads the launch_weapon with a missile from the currently
 *                selected projectile stack.
 */
public nomask void
load_projectile()
{
    object env;

    env = environment(Projectile_stack);
    Projectile_stack->split_heap(1);
    Projectile = Projectile_stack;
    Projectile_stack = Projectile_stack->make_leftover_heap();

    /*
     * Move the projectile to the inventory of the player and make it
     * invisible to him.
     */
    Projectile->load();
    Projectile->move(query_wielded(), 1);
    Projectile_stack->move(env, 1);
    
    // Set an alarm that forces the player to unload a drawn weapon eventually.
    if (Drawn_wep)
    {
        Fatigue_alarm = set_alarm(F_LAUNCH_W_FATIGUE_TIME(Archer), 0.0, 
				  &fatigue_unload());
    }
}

/*
 * Function name: fatigue_unload
 * Description  : Forces an unload of the launch_weapon if it has been held
 *                in drawn position for too long.
 */
private nomask void 
fatigue_unload()
{
    if (Fatigue_alarm && Archer && Archer == query_wielded() && Projectile)
    {
        this_object()->tell_archer_fatigue_unload(Archer, Target, Projectile);
	this_object()->tell_others_unload(Archer, Target, Projectile);
	Projectile->unload();
	reset_launch_weapon();
        Archer->add_fatigue(-10);
    }
}

/*
 * Function name: unwield
 * Description  : This function might be called when someone tries to unwield
 *                this weapon. In a launch weapon unwield causes the weapon to
 *                be unloaded as well.
 *
 * Arguments    : object obj - the weapon to stop wielding.
 * Returns      : int  0 - the weapon can be unwielded normally.
 *                     1 - unwield the weapon, but print no messages.
 *                    -1 - do not unwield the weapon, print default messages.
 *                string - do not unwield the weapon, use this fail message.
 */
public mixed
unwield(object obj)
{
    if (Projectile)
    {
        unload_projectile();
    }
    return 0;
}

/*
 * Function name: unload_projectile
 * Description  : Unloads a loaded projectile and resets the launch_weapon.
 */   
public nomask void
unload_projectile()
{
    if (objectp(Projectile))
    {
        Projectile->unload();
	
        this_object()->tell_archer_unload(query_wielded(), Target, Projectile);
        this_object()->tell_others_unload(query_wielded(), Target, Projectile);
	
	if (Projectile_stack)
	{
	    Projectile_stack->set_heap_size(Projectile_stack->num_heap() + 1);
	    Projectile->remove_object();
	}
    }
    
    reset_launch_weapon();
}

/*
 * Function name: find_projectiles
 * Description  : Searches the inventory of the wielder for projectiles.
 *                If that fails it tries to find quivers containing
 *                suitable projectiles in the wielders inventory.
 *                If projectiles are found the wielder is notified.
 *
 * Returns      : int - 1 If projectiles are found. 0 Otherwise.
 */
public nomask int
find_projectiles()
{
    object *projectiles, *quivers;
    int i;
    
    projectiles = query_valid_projectiles(query_wielded());

    if (!sizeof(projectiles))
    {
        quivers = filter(all_inventory(query_wielded()), 
			 &->query_prop(CONT_I_IS_QUIVER));

	if (!sizeof(quivers))
	{
            return 0;
	}

	for (i = 0; i < sizeof(quivers); i++)
        {
	    projectiles = query_valid_projectiles(quivers[i]);

	    if (sizeof(projectiles))
	    {
	        break;
	    }
	}
	
	if (!sizeof(projectiles))
	    return 0;
    }

    set_projectile_stack(projectiles[0]);
    tell_object(query_wielded(), "Using " + Projectile_stack->short() + 
		(sizeof(quivers) ? " from the " + quivers[i]->short() : "") +
		".\n");
    
    return 1;      
}

/*
 * Function name: query_hit
 * Description  : Returns the average of launcher and projectile hit values.
 *                
 * Returns      : int - Returns the average of launcher and projectile
 *                hit values.
 */
int
query_hit()
{
    if (objectp(Projectile_stack))
        return (::query_hit() + Projectile_stack->query_hit()) / 2;
    else
        return ::query_hit();
}

/*
 * Function name: query_pen
 * Description  : Returns the average of launcher and projectile pen values.
 *                
 * Returns      : int - Returns the average of launcher and projectile
 *                pen values.
 */
int
query_pen()
{
    if (objectp(Projectile_stack))
        return (::query_pen() + Projectile_stack->query_pen()) / 2;
    else
        return ::query_pen();
}

/*
 * Function name: did_hit
 * Description:   Tells us that we hit something. Should produce combat
 *                messages to all relevant parties. If the weapon
 *                chooses not to handle combat messages then a default
 *                message is generated. This function is meant to be overridden
 *                by launch_weapon implementations. Remember to call
 *                ::did_hit() since this function is also responsible
 *                for moving the missiles and reseting the launch_weapon
 *                for the next round.
 *
 * Arguments:     aid:   The attack id
 *                hdesc: The hitlocation description.
 *                phurt: The %hurt made on the enemy
 *                enemy: The enemy who got hit
 *                dt:    The current damagetype
 *                phit:  The %success that we made with our weapon
 *                dam:   The actual damage caused by this weapon in hit points
 * Returns:       True if it handled combat messages, returning a 0 will let
 *                the normal routines take over
 */
public varargs int
did_hit(int aid, string hdesc, int phurt, object target, int dt,
                int phit, int dam, int hid)
{
    object armour;

    // Miss.
    if (phurt == -1)
    {
        this_object()->
	    tell_archer_miss(query_wielded(), target,
			     Projectile, Adj_room_desc);

        this_object()->
	    tell_target_miss(query_wielded(), target,
			     Projectile, Adj_room_desc, Org_room_desc);

        this_object()->
	    tell_others_miss(query_wielded(), target,
			     Projectile, Adj_room_desc, Org_room_desc);
    }
    // Hit, but target is protected by his armour.
    else if (dam == 0)
    {
        armour = find_protecting_armour(target, hid);

	this_object()->
	    tell_archer_bounce_armour(query_wielded(), target,
				      Projectile, Adj_room_desc, armour);

        this_object()->
	    tell_target_bounce_armour(query_wielded(), target, Projectile,
				      Adj_room_desc, Org_room_desc, armour);
	
        this_object()->
	    tell_others_bounce_armour(query_wielded(), target, Projectile,
				      Adj_room_desc, Org_room_desc, armour);  
    }
    // Hit.
    else
    {
	this_object()->
	    tell_archer_hit(query_wielded(), target, Projectile,
			    Adj_room_desc, hdesc, dt, phurt, dam, hid);
	
	this_object()->
	    tell_target_hit(query_wielded(), target, Projectile, Adj_room_desc,
			    Org_room_desc, hdesc, dt, phurt, dam, hid);

	this_object()->
	    tell_others_hit(query_wielded(), target, Projectile, Adj_room_desc,
			    Org_room_desc, hdesc, dt, phurt, dam, hid);
    }

    Projectile->unload();

    if (!random(F_PROJECTILE_BREAK_CHANCE))
    {
        this_object()->
	    tell_all_projectile_break(Projectile, target, ENV(target));
	Projectile->set_broken(1);
    }  
    
    if (phurt < 5)
    {
        move_projectile_to_env(ENV(target), Projectile);
    }
    else
    {
        move_projectile_to_target(target, Projectile);
    }

    if (dam > 0)
    {
        target->heal_hp((F_LAUNCH_W_DAM_FACTOR * -dam) / 100);
	Projectile->projectile_hit_target(query_wielded(), aid, hdesc, phurt,
					  target, dt, phit, dam, hid);
    }

    /* Reset launch weapon only after all other processing. */
    reset_launch_weapon();

    return 1;
}

/*
 * Function name: reset_launch_weapon
 * Description  : Resets the launch weapon so it can be aimed and fired again.
 */
private nomask void 
reset_launch_weapon()
{
    Projectile = 0;
    Ready = 0;

    if (Shoot_alarm)
    {
        remove_alarm(Shoot_alarm);
	Shoot_alarm = 0;
    }

    if (Fatigue_alarm)
    {
        remove_alarm(Fatigue_alarm);
	Fatigue_alarm = 0;
    }
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
    tell_object(query_wielded(), "You are out of projectiles!\n");
}

/*
 * Function name: tell_archer_fatigue_unload
 * Description  : Produces a message to the wielder that he is too tired to 
 *                keep his launch_weapon drawn. This function is meant to be
 *                overridden by launch_weapon implementations.
 *                
 * Arguments    : archer:      The tired archer.
 *                target:      The person the wilder aimed at.
 *                projectile:  The projectile we unload in the proccess.
 */
public void 
tell_archer_fatigue_unload(object archer, object target, object projectile)
{
    tell_object(archer, "You are too tired to keep the " + short() + 
		" loaded, you unload your " + short() + ".\n");
}

/*
 * Function name: tell_archer_unload
 * Description  : Produces messages to the wielder when he unloads his
 *                launch_weapon. This function is meant to be overridden
 *                by launch_weapon implementations.
 *                
 * Arguments    : archer:      The archer unloading a projectile.
 *                target:      The person the wilder aimed at.
 *                projectile:  The projectile he unloads.
 */
public void
tell_archer_unload(object archer, object target, object projectile)
{
    tell_object(archer, "You unload your " + short() + ".\n");
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
    tell_room(ENV(archer), QCTNAME(archer) + " unloads " +
	      archer->query_possessive() + " " + short() + ".\n",
	      ({ archer }), archer);
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
    if (ENV(archer) == ENV(target))
    {
        archer->catch_msg("You load your " + short() + " with " +
 			  LANG_ADDART(Projectile->singular_short()) + 
			  " and take careful aim at " + 
			  target->query_the_name(archer) + ".\n");
    }
    else
    {
        archer->catch_msg("You load your " + short() + " with " + 
			  LANG_ADDART(Projectile->singular_short()) + 
			  " and take careful aim at " + 
			  target->query_the_name(archer) + " " + 
			  adj_desc + ".\n");
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
tell_others_load(object archer,  object target,
		 object projectile, string adj_desc)
{
    if (ENV(archer) == ENV(target))
    {
        // X loads his weapon and aims at Y.
        tell_bystanders_miss(QCTNAME(archer) + " loads " + 
			     archer->query_possessive() + " " + short() + 
			     " and aims at " + QTNAME(target) + ".\n",
			     QCTNAME(archer) + " loads " + 
			     archer->query_possessive() + " " + short() + 
			     " and aims at something.\n",
			     0, 0, archer, target, ENV(archer));
    }
    else
    {
        tell_bystanders_miss(QCTNAME(archer) + " loads " + 
			     archer->query_possessive() + " " + short() + 
			     " and aims at something " + adj_desc + ".\n",
			     QCTNAME(archer) + " loads " + 
			     archer->query_possessive() + " " + short() + 
			     " and aims at something " + adj_desc + ".\n",
			     0, 0, archer, target, ENV(archer));
    }
}

/*
 * Function name: tell_target_load
 * Description  : Produces a message to the player when he is loading his
 *                launch_weapon. This method is intended to be overridden
 *                by implementations of launch_weapon.
 *                
 * Arguments    : archer:     The player loading his weapon.
 *                target:     The target player is aiming at.
 *                projectile: The projectile we are loading.
 */
public void
tell_target_load(object archer,  object target, object projectile)
{
    if (ENV(target) == ENV(archer))
    {
        tell_object(target, archer->query_The_name(target) + " loads " +
		    archer->query_possessive() + " " + short() + " with " + 
		    LANG_ADDART(Projectile->singular_short()) + 
		    "and takes careful aim at you.\n");
    }
}

/*
 * Function name: tell_all_projectile_break
 * Description  : Tells everyone when a projectile breaks.
 *                
 * Arguments    : projectile:  The projectile that breaks.
 *                enemy:       The enemy the projectile was shot at.
 *                enemy_env:   The environment of the enemy.
 */
public void 
tell_all_projectile_break(object projectile, object target)
{
    tell_room(ENV(target), "The " +
	      Projectile->singular_short() + " breaks!\n");
}

/*
 * Function name: tell_watcher
 * Description  : Tells those players that watch the fight what happens. This
 *                function uses catch_msg which take care of met/nomet.
 *                
 * Arguments    : see_both:   Message seen by those that see player and target.
 *                see_archer: Message seen by those that see the archer only.
 *                see_target: Message seen by those that see target only.
 *                see_noone:  Message seen by those that see noone.
 *                archer:     The archer shooting with a launch_weapon.
 *                target:     The target that is shot at.
 *                bystanders: Array of the bystanders to send the message.
 */
nomask void
tell_bystanders(string see_both, string see_archer,
		string see_target, string see_noone,
		object archer, object target, object *bystanders)
{
    object *current_bystanders;
    
    if (!sizeof(bystanders))
    {
	return;
    }
    
    // Bystanders that see both the archer and the target.
    
    current_bystanders =
	filter(FILTER_IS_SEEN(target, FILTER_IS_SEEN(archer, bystanders)),
	       &->can_see_in_room());

    if (sizeof(current_bystanders))
    {
        current_bystanders->catch_msg(see_both);
    }

    if (sizeof(current_bystanders) == sizeof(bystanders))
    {
        return;
    }

    // Bystanders that see the archer only.

    bystanders -= current_bystanders;
    current_bystanders = filter(FILTER_IS_SEEN(archer, bystanders),
				&->can_see_in_room());

    if (sizeof(current_bystanders) && stringp(see_archer))
    {
        current_bystanders->catch_msg(see_archer);
    }

    // Bystanders that see the target only.

    bystanders -= current_bystanders;
    current_bystanders = filter(FILTER_IS_SEEN(target, bystanders),
				&->can_see_in_room());
    if (sizeof(current_bystanders) && stringp(see_target))
    {
        current_bystanders->catch_msg(see_target);
    }

    // Bystanders that niether see in room nor any of the combatants.

    bystanders -= current_bystanders;
    
    if (sizeof(bystanders) && stringp(see_noone))
    {
        bystanders->catch_msg(see_noone);
    }
}

/*
 * Function name: tell_bystander_miss
 * Description:   Send the appropriate string to interactive bystanders
 *                that want to see misses and want to see others fight.
 *                
 * Arguments:     see_both:   The string to send to those that see both
 *                            target and archer.
 *                see_archer: The string to send to those that see the
 *                            archer only.
 *                see_target: The string to send to those that see the
 *                            target only.
 *                see_noone:  The string to send to those that see neither
 *                            of the combatants or don't see in the room.
 *                archer:     The archer.
 *                target:     The target.
 *                env:        The room containing the bystanders.
 */
nomask void
tell_bystanders_miss(string see_both, string see_archer,
		     string see_target, string see_noone,
		     object archer, object target, object env)
{
    object *bystanders;

    bystanders = all_inventory(env) - ({ archer, target });
    bystanders = filter(bystanders, &interactive());
    bystanders = filter(bystanders, &filter_gag_misses());   
    bystanders = filter(bystanders, &filter_see_blood());
    
    tell_bystanders(see_both, see_archer, see_target, see_noone,
		    archer, target, bystanders);
}

/*
 * Function name: tell_bystanders_hit
 * Description:   Send the appropriate string to interactive bystanders
 *                that want to see misses and want to see others fight.
 *                
 * Arguments:     see_both:   The string to send to those that see both
 *                            target and archer.
 *                see_archer: The string to send to those that see the
 *                            archer only.
 *                see_target: The string to send to those that see the
 *                            target only.
 *                see_noone:  The string to send to those that see neither
 *                            of the combatants or don't see in the room.
 *                archer:     The archer.
 *                target:     The target.
 *                env:        The room containing the bystanders.
 */
nomask void
tell_bystanders_hit(string see_both, string see_archer,
		    string see_target, string see_noone,
		    object archer, object target, object env)
{
    object *bystanders;

    bystanders = all_inventory(env) - ({ archer, target });
    bystanders = filter(bystanders, &interactive());
    bystanders = filter(bystanders, &filter_see_blood());
    
    tell_bystanders(see_both, see_archer, see_target, see_noone,
		    archer, target, bystanders);
}

/*
 * Function name: filter_see_blood
 * Description  : Returns true if the person don't want to see the message.
 *                
 * Arguments    : person: Player to check for OPT_NO_FIGHTS.
 * Returns      : int:    True if the person don't want to see blood.
 */
private int 
filter_see_blood(object person)
{
    return !(person->query_option(OPT_NO_FIGHTS));
}

/*
 * Function name: filter_gag_misses
 * Description  : Returns true if player wants to see misses..
 *                
 * Arguments    : person: Player to check for OPT_GAG_MISSES.
 * Returns      : int:    True if player wants to see misses.
 */
private int 
filter_gag_misses(object person)
{
    return !(person->query_option(OPT_GAG_MISSES));
}

/*
 * Function name: find_protecting_armour
 * Description  : Returns one of the armours protecting the given hid
 *                on target.
 *
 * Arguments    : target: Person to search for armours.
 *                hid:    Hit location to search for armours.
 */
public nomask object
find_protecting_armour(object target, int hid)
{
    object *armours;
  
    armours = target->query_combat_object()->cb_query_armour(hid);

    if (sizeof(armours = filter(armours,
				&operator(!=)(A_MAGIC) @ &->query_at())))
    {
        return armours[random(sizeof(armours))];
    }
    else
    {
        return 0;
    }
}

/*
 * Function name: move_projectile_to_env
 * Description  : Moves and hides a projectile at the specified location.
 *
 * Arguments    : location:   The new location of the projectile.
 *                projectile: Projectile to hide.
 */
public void
move_projectile_to_env(object location, object projectile)
{
    projectile->add_prop(OBJ_I_HIDE, 33 * random(3));
    projectile->move(location, 1);
}

/*
 * Function name: move_projectile_to_target
 * Description  : Moves the projectile to the specified target.
 *
 * Arguments    : target:    The target that was hit by the projectile.
 *                projectile: Projectile that was shot.
 */
public void
move_projectile_to_target(object target, object projectile)
{
    projectile->move(target, 1);
}

/*
 * Function name: check_remote_seen
 * Description  : Checks wether the specatator can see the target from
 *                another room.
 *
 * Arguments    : spectator:  The person that is trying to see.
 *                target:     The person that might be seen by the spectator.
 *
 * Returns      : 1 if spectator can see target.
 */
public int
check_remote_seen(object spectator, object target)
{
    if (!objectp(spectator) || !objectp(target) || target->query_no_show())
    {
	// Obj does not exist or is no_show.
	return 0;
    }
    
    if (spectator->query_prop(LIVE_I_SEE_INVIS) <
	target->query_prop(OBJ_I_INVIS) ||
	spectator->query_skill(SS_AWARENESS) <
	target->query_prop(OBJ_I_HIDE))
    {
	// Obj is too well hidden.
        return 0;
    }
    
    if (environment(spectator)->quey_prop(OBJ_I_LIGHT) >
	-(spectator->query_prop(LIVE_I_SEE_DARK)))
    {
	// You can not see in your own room.
        return 0;
    }

    if (environment(target)->quey_prop(OBJ_I_LIGHT) >
	-(spectator->query_prop(LIVE_I_SEE_DARK)))
    {
	// You can not see in target's room.
        return 0;
    }

    return 1;
}

/*
 * Function name: tell_archer_miss
 * Description  : Produces a message to the player when he misses his target.
 *                This function take visual conditions in consideration as
 *                well as shoots across rooms. This function is meant to be
 *                overridden in launch_weapon implementations.
 *
 * Arguments    : archer:        The player loading his weapon.
 *                target:        The target player is aiming at.
 *                projectile:    The projectile we are loading.
 *                adj_room_desc: Description of the room we shoot into. 0 if
 *                               target stand in the same room.
 */
public void
tell_archer_miss(object archer, object target,
		 object projectile, string adj_room_desc)
{
    return;
}

/*
 * Function name: tell_target_miss
 * Description  : Produces a message to the target when the archer tries to
 *                shoot at him but miss. This function take visual
 *                conditions in consideration as well as shoots across rooms.
 *                This function is meant to be overridden in launch_weapon
 *                implementations.
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
    return;
}

/*
 * Function name: tell_others_miss
 * Description  : Produces messages to all bystanders when the archer misses
 *                his target. This function take visual conditions in
 *                consideration as well as shoots across rooms. This function
 *                is meant to be overridden in launch_weapon implementations.
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
    return;
}

/*
 * Function name: tell_archer_bounce_armour
 * Description  : Produces a message to the archer when his arrow hits the
 *                target without causing any harm. This is described as
 *                the arrow bouncing off his armour. This function is meant
 *                to be overridden by launch_weapon implementations.
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
    return;
}

/*
 * Function name: tell_target_bounce_armour
 * Description  : Produces a message to the target when he is hit by
 *                an arrow that do no dammage. This is described as
 *                the arrow bouncing off his armour. This function is
 *                meant to be overridden by launch_weapon implementations.
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
    return;
}

/*
 * Function name: tell_others_bounce_armour
 * Description  : Produces messages to bystanders when the archer's arrow
 *                harmlessly hits the target. This is described as
 *                the arrow bouncing off the target's armour. This method
 *                is meant to be overridden by launch weapon implementations.
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
    return;
}

/*
 * Function name: tell_archer_hit
 * Description  : Produces a message to the archer when he hits the target.
 *                This function takes visual conditions as well as shoots
 *                across rooms in consideration. This method is meant to be
 *                overridden by launch_weapon implementations.
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
    return;
}

/*
 * Function name: tell_target_hit
 * Description  : Produces a message to the target when he is hit by the
 *                archer. This function takes visual conditions as well as
 *                shoots across rooms in consideration. This method is meant
 *                to be overriddent by launch_weapon implementations.
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
    return;
}

/*
 * Function name: tell_others_hit
 * Description  : Produces messages to all bystanders when target is hit
 *                by the archer. This function tells targets in both the
 *                archer's and target's environment if they are not the
 *                same. This method is meant to be overridden by
 *                launch_weapon implementations.
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
    return;
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
    return 1;
}

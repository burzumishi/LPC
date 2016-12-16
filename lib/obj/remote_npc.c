/*
 * /std/remote_npc.c
 *
 * A small gadget for listning in and controling NPC's.
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/std/object";

/*
 * Global variables.
 */
static 	object	link_player,
		link_monster;
static	int	link_toggle;

/*
 * Prototypes
 */
public int remote_cmd(string str);
public int posstoggle(string str);

#define SEQ_LINK "_remnpc_link"

/*
 * Function name: create_object
 * Description  : This function is called when the object is created.
 */
public void
create_object()
{
    set_name("wand");
    add_name("remote");
    add_name("controller"); 
    add_adj("silver");

    set_short("silver wand");
    set_long("@@long_func");
}

/*
 * Function name: long_func
 * Description  : Return the long description of this object.
 * Returns      : string - the long description.
 */
public string
long_func()
{
    string str;

    str = "It is quite definetely a wand of control. Linked commands:\n" +
"    controltoggle <name>  - toggle the control over <name> on/off.\n" +
"    #<command>            - make the controlled monster do <command>.\n" +
"    #<name>#<command>     - make monster <name> do <command>\n" +
"    #! ... [%%<command2>] - <command> ... <commandn> are to be executed\n" +
"                            as sequence action.\n";

    if (link_monster)
    {
        if (link_monster &&
            (environment(this_object()) == this_player()))
        {
	    str += "You feel the presence of " + 
		link_monster->query_name() + " the " +
		link_monster->query_race_name() +
		".\n";
	}
	else
	{
	    str += "It seems to be active.\n";
	}
    }
    return str;
}

/*
 * Function name: set_npc
 * Description  : This function is called to set the monster being linked
 *                to this object and therewith be controlled.
 * Arguments    : object monster - the monster to be controlled.
 */
public void
set_npc(object monster)
{
    if (link_monster)
    {
    	return;
    }

    link_monster = monster;
    link_toggle = 1;
    link_monster->set_tell_active(1);
}

/*
 * Function name: link_intext
 * Description  : Whenever a message is printed to the NPC, it is also
 *                printed to the player controlling the NPC. This function
 *                is called from the NPC to print the message.
 * Arguments    : string str - the message to print.
 */
public void
link_intext(string str)
{
    string name;

    if (!link_toggle)
    {
	return;
    }

    if (link_player)
    {
	name = "#" + link_monster->query_name() + "# ";
	str = implode(explode(str, "\n"), "\n" + name);

	tell_object(link_player, name + str + "\n");
    }
}

/*
 * Function name: init
 * Description  : This funciton is called to link the controlling commands
 *                of this wand to the wizard using it.
 */
void
init()
{
    ::init();

    if ((environment(this_object()) == this_player()) &&
	this_player()->query_wiz_level())
    {
	add_action(remote_cmd, "#", 1);
	add_action(posstoggle, "controltoggle");
    }
}

/*
 * Function name: remote_cmd
 * Description  : This function is called to command the linked monster
 *                to perform a certain command.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
static int
remote_cmd(string str)
{
    string cmd, name, exe, *many;

    if (!link_monster)
    {
    	notify_fail("No monster linked; the wand selfdestructs.\n");
	destruct();
	return 0;
    }

    cmd = query_verb() + (str ? (" " + str) : "");
    if (sscanf(cmd, "#%s#%s", name, exe) == 2)
    {
	if (name != lower_case(link_monster->query_name()))
	{
	    notify_fail("No living \"" + name + "\" linked.\n");
	    return 0;
	}
    }
    else
    {
	exe = extract(cmd, 1);
    }

    if (sscanf(exe, "!%s", cmd) == 1)
    {
	many = explode(cmd + "%%", "%%");
	if (!sizeof(link_monster->seq_query(SEQ_LINK)))
	    link_monster->seq_new(SEQ_LINK);
	link_monster->seq_clear(SEQ_LINK);
	link_monster->seq_addfirst(SEQ_LINK,many);
	write("Ordered sequence actions.\n");
	return 1;
    }

    notify_fail("#" + link_monster->query_name() + "# What?\n");
    return link_monster->command(exe);
}

/*
 * Function name: posstoggle
 * Description  : This function is called when the wizard wants to toggle
 *                the control on or off.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
static int
posstoggle(string str)
{
    if (!link_monster ||
	!link_monster->id(str))
    {
	notify_fail("Toggle control of what monster? <Give name of monster>\n");
	return 0;
    }

    if (link_toggle) 
    {
	link_toggle = 0;
	write("Toggled control of " +
	    link_monster->query_name() + " off.\n");
    }
    else
    {
	link_toggle = 1;
	write("Toggled control of " +
	    link_monster->query_name() + " on.\n");
    }
    return 1;
}

/*
 * Function name: enter_env
 * Descripiton  : Whenever this object enters a new environment, this function
 *                is called to set the objectpointer to the wizard
 *                controlling this wand.
 * Arguments    : object dest - the environment the object enters.
 *                object old  - the environment the object left. Can be 0.
 */
public void
enter_env(object dest, object old)
{
    ::enter_env(dest, old);

    if (living(dest))
    {
	link_player = environment(this_object());
    }
}

/*
 * Function name: leave_env
 * Description  : Whenever this object leaves its environment, this function
 *                is called to reset the objectpointer to the wizard
 *                controlling the NPC.
 * Arguments    : object old  - the environment the object left.
 *                object dest - the environment the object enters. Can be 0.
 */
public void
leave_env(object old, object dest)
{
    ::leave_env(old,dest);
    
    link_player = 0;
}

/*
 * Function name: query_prevent_shadow
 * Description  : This function is used to protect this object from being
 *                shadowed.
 * Returns      : int 1 - always.
 */
nomask public int
query_prevent_shadow()
{
    return 1;
}

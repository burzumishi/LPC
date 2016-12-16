/* 
 * door.c
 *
 * This is the standard door object.
 *
 * User function in this object:
 * -----------------------------
 *
 * Arguments:	        s = string, i = integer, o = object, m = mixed
 *			*x = list of given type.
 *			** mandatory function. 
 *			-* special circumstances apply
 *
 * All set functions has a corresponding query function defined.
 *
 *
 * create_door()	- The function that initates the object. You should
 *			  have calls to the following functions here.
 *
** set_other_room(s)	- Call this function with the name of the room where
 *			  the other side of the door is.
 *
** set_door_id(s)	- Call this function with a unique name for the door,
 *			  this should really be something strange that no
 *			  other door in the same room can be called. "door"
 *			  is *not* a good choise...
 *
 * set_door_desc(s)	- Call this function with the description of the
 *			  door as seen when looking at it.
 *
-* set_door_name(m)	- Call this function with the name or a list of all
 *			  the names that the door can be called.
 *			  Must be defined if a description is defined.
 *			
 * set_open(i)		- Call if 1 if open, 0 if closed.
 *
** set_open_desc(s)	- Call this function with the description of the
 *			  open door.
 *
** set_closed_desc(s)	- Call this function with the description of the
 *			  closed door.
 *
 * set_pass_command(m)	- Call this function with the command string or a 
 *			  list of all the command strings that can be 
 *			  given to pass through the door. Wizards can pass
 *                        any door by adding an exclamation point to the
 *                        command.
 *
-* set_fail_pass(s)	- Call this function with the message you are given
 *			  if the door is closed when you try to pass through.
 *			  Must be defined if a pass command is defined.
 *
 * set_open_command(m)	- Call this function with the command string or a 
 *			  list of all the command strings that can be
 *			  given to open the door.
 *
-* set_open_mess(*s)	- Call this function with a list of two strings. The
 *			  first string is the message given in the room when
 *			  someone opens the door on the (emote-like) format 
 *			  "opens the door" and the other is the message given
 *			  in the other room.
 *			  Must be defined if an open command is defined.
 *
-* set_fail_open(m)	- Call this function with the message given when you
 *			  fail to open an already open door. If you have a 
 *			  lock in the door, call it with a list of two 
 *			  strings with the first string being the message
 *			  given when the door already is open and the second
 *			  when the door is closed, but locked.
 *			  Must be defined if an open command is defined.
 *
 * set_close_command(m)	- Call this function with the command string or a 
 *			  list of all the command strings that can be
 *			  given to close the door.
 *
-* set_close_mess(*s)	- Call this function with a list of two strings. The
 *			  first string is the message given in the room when
 *			  someone closess the door on the (emote-like) format 
 *			  "closes the door" and the other is the message given
 *			  in the other room.
 *			  Must be defined if a close command is defined.
 *
-* set_fail_close(s)	- Call this function with the message you get if you
 *			  try to close an already closed door.
 *			  Must be defined if a close command is defined.
 *
 * set_locked(i)	- Call if 1 if locked, 0 if unlocked.
 * 
-* set_lock_name(m)	- Call this function with the name or a list of all
 *			  the names that the lock (if any) can be called.
 *			  Must be defined if a lock if wanted.
 *
 * set_lock_desc(s)	- Call this function with the description of the
 *			  lock (if any).
 *
 * set_lock_command(m)	- Call this function with the command string or a 
 *			  list of all the command strings that can be
 *			  given to open the door.
 *
-* set_lock_mess(*s)	- Call this function with a list of two strings. The
 *			  first string is the message given in the room when
 *			  someone opens the door on the (emote-like) format 
 *			  "locks the door" and the other is the message given
 *			  in the other room.
 *			  Must be defined if a lock command is defined.
 *
-* set_fail_lock(*s)	- Call this function with a list of two strings. The
 *			  first string is the message given in the room when
 *			  you try to lock an already locked door, and the
 *			  second you get when you try to lock an open door.
 *			  Must be defined if a lock command is defined.
 *
 * set_unlock_command(m)  Call this function with the command string or a 
 *			  list of all the command strings that can be
 *			  given to unlock the door.
 *
-* set_unlock_mess(*s)	- Call this function with a list of two strings. The
 *			  first string is the message given in the room when
 *			  someone unlocks the door on the (emote-like) format 
 *			  "unlocks the door" and the other is the message given
 *			  in the other room.
 *			  Must be defined if an unlock command is defined.
 *
 *
-* set_fail_unlock(s)	- The message given if the door already was unlocked.
 *			  Must be defined if an unlock command is defined.
 *
 * set_key(m)		- Call this function with the key for a key to
 *			  use.
 *
 * lock_function()	- Replace this function with your own if you have
 *			  other ideas of how to open locks.
 *
 * set_no_pick()	- Call this function if your door has a lock but you
 *			  don't want a 'pick' verb.
 * 
 * set_pick(i)		- Set how hard it is to pick the lock. >100 and it 
 *			  won't be possible to pick.
 *
 * set_str(i)           - Set strength needed to open/close the door.
 *
-* set_knock_command(m)	- Call this function with the command string or a 
 *			  list of all the command strings that can be
 *			  given to knock on the door. Default cmd is
 *			  ({ "knock", "pound" }). To disable knocking,
 *			  call it with an arg of 0.
 *
 * set_knock_resp(*s)   - Call this function with an array of 3 message
 *			  strings. The first is the message written to
 *			  this_player() when s/he knocks on a closed door,
 *			  the second string is the message given in the 
 *			  room when someone knocks on the closed door in the 
 *			  (emote-like) format "knocks on the door" and
 *			  the third is the message given in the room
 *			  on the other side of the closed door.
 *
 * set_knock_fail(*s)   - Call this function with an array of 3 message
 *			  strings. The first is the message written to
 *			  this_player() when s/he knocks on an open door,
 *			  the second string is the message given in the 
 *			  room when someone knocks on the open door in the 
 *			  (emote-like) format "foolishly knocks on the
 *			  open door" and the third is the message given
 *			  in the room on the other side of the open door.
 *
 * Don't forget to set the special door properties if the standard
 * settings aren't what you want. 
 *
 * The standard door has no locks, weighs 60 kg and has a volume of
 * 80 liters. The height of the standard door is 2 meters.
 *
 */
#pragma save_binary
#pragma strict_types

inherit "/std/object";
#include <stdproperties.h>
#include <macros.h>
#include <ss_types.h>
#include <cmdparse.h>
#include <language.h>

string	other_room,	/* The name of the other side of the door */
	door_id,	/* A unique (?) id of the door */
	lock_desc,	/* A description of the lock */
    	open_desc,	/* The description of the open door */
	closed_desc,	/* The description of the closed door */
	fail_pass,	/* The fail pass message */
	fail_close,	/* The fail close message */
	fail_unlock;	/* The fail unlock message */

string	*open_mess,	/* The open messages */
	*fail_open,	/* The fail open messages */
	*close_mess,	/* The close messages */
	*lock_mess,	/* The lock messages */
	*fail_lock,	/* The fail lock messages */
	*unlock_mess;	/* The unlock messages */

string	*knock_commands,/* The commands used to knock on the door */
	*knock_resp,	/* The knock ok responses */
	*knock_fail;	/* The knock fail responses */

mixed	door_name,	/* The name(s) of the door */
	lock_name,	/* The name(s) of the lock */
    	key;		/* The key for opening locks */

object	other_door;	/* The door at the other side */

string	*pass_commands,		/* The commands used to enter the door */
	*open_commands,		/* The commands used to open the door */
	*close_commands,	/* The commands used to close the door */
	*lock_commands,		/* The commands used to lock the door */
	*unlock_commands;	/* The commands used to unlock the door */

int	open_status,		/* If the door is open or not */
	lock_status,		/* If the door is locked or not */
	no_pick,		/* If the door is possible to pick */
	pick,			/* How hard is the lock to pick? */
	open_str;		/* Strength needed to open door */

/* 
 * Some prototypes 
 */
void create_door();
void set_lock_desc(string desc);
void set_lock_mess(string *mess);
void set_fail_lock(mixed mess);
void set_unlock_mess(string *mess);
void set_fail_unlock(string mess);
void set_other_room(string name);
void set_door_id(string id);
void set_door_name(mixed name);
void set_door_desc(string desc);
void set_open_desc(string desc);
void set_closed_desc(string desc);
void set_pass_command(mixed command);
void set_fail_pass(string mess);
void set_open_command(mixed command);
void set_open_mess(string *mess);
void set_fail_open(mixed mess);
void set_close_command(mixed command);
void set_close_mess(string *mess);
void set_fail_close(string mess);
void load_other_door();
static void remove_door_info(object dest);
void do_open_door(string mess);
void do_close_door(string mess);
void do_lock_door(string mess);
void do_unlock_door(string mess);
int  lock_procedure(string arg);
void set_open(int i);
void set_locked(int i);
void do_set_key(mixed keyval);
int  knock_door(string arg);
void set_knock_command(mixed cmd);
void set_knock_resp(string *msg);
void set_knock_fail(string *msg);
string *query_knock_command();
string *query_knock_resp();
string *query_knock_fail();
string standard_knock_resp0();
string standard_knock_resp1();
string standard_knock_resp2();
string standard_knock_fail0();
string standard_knock_fail1();
string standard_knock_fail2();

/*
 * Function name: create_object
 * Description:   Initialize object.
 */
void
create_object()
{
    pass_commands = ({});
    open_commands = ({});
    close_commands = ({});
    knock_commands = ({});
    pick = 40 + random(20); /* Pickable by default. */
    add_prop(OBJ_I_WEIGHT, 60000);
    add_prop(OBJ_I_VOLUME, 80000);
    add_prop(DOOR_I_HEIGHT, 300); /* Default hight 3 meters. */
    add_prop(OBJ_I_NO_GET, 1);
    set_no_show_composite(1);

    /*
     * Default messages.
     */
    set_open_desc("@@standard_open_desc");
    set_closed_desc("@@standard_closed_desc");
    set_fail_pass("@@standard_fail_pass");
    set_open_command("open");
    set_open_mess(({"@@standard_open_mess1", "@@standard_open_mess2"}));
    set_fail_open(({"@@standard_fail_open1", "@@standard_fail_open2"}));
    set_close_command("close");
    set_close_mess(({"@@standard_close_mess1", "@@standard_close_mess2"}));
    set_fail_close("@@standard_fail_close");
    set_door_desc("@@standard_door_desc");
    set_lock_desc("A common bolt lock.\n"); /* Any better suggestions ? */
    set_lock_mess(({"@@standard_lock_mess1", "@@standard_lock_mess2"}));
    set_fail_lock(({"@@standard_fail_lock1", "@@standard_fail_lock2"}));
    set_unlock_mess(({"@@standard_unlock_mess1", "@@standard_unlock_mess2"}));
    set_fail_unlock("@@standard_fail_unlock");
    set_open(1);
    set_locked(0);

    set_knock_command( ({ "knock", "pound" }) );
    set_knock_resp( ({ VBFC_ME("standard_knock_resp0"),
	VBFC_ME("standard_knock_resp1"),
	VBFC_ME("standard_knock_resp2") }) );
    set_knock_fail( ({ VBFC_ME("standard_knock_fail0"),
	VBFC_ME("standard_knock_fail1"),
	VBFC_ME("standard_knock_fail2") }) );

    /* If you want to have a lock on the door, and be able to both lock it
       and unlock it you have to add these functions yourself. Don't do it
       here since all doors would have locks then... */

    /* You have to define the following yourself:
       (see docs in the beginning of this file)

             set_other_room()
             set_door_id()
             set_pass_command()
             set_door_name()
    */

    create_door();
}

/*
 * Function name: create_door
 * Description:   Sets default names and id
 */
void
create_door() {}

/*
 * Function name: reset_door
 * Description:   Reset the door
 */
void
reset_door() {}

/*
 * Function name: reset_object
 * Description:   Reset the object
 */
nomask void
reset_object()
{
    reset_door();
}

int pass_door(string str);
int open_door(string str);
int close_door(string str);
int lock_door(string str);
int unlock_door(string str);
int pick_lock(string str);

/*
 * Function name: init
 * Description:   Initalize the door actions
 */
void
init()
{
    ::init();

    foreach(mixed cmd: pass_commands)
    	add_action(pass_door, check_call(cmd));

    if (this_player()->query_wiz_level())
    {
        foreach(mixed cmd: pass_commands)
	    add_action(pass_door, check_call(cmd) + "!");
    }

    foreach(mixed cmd: open_commands)
	add_action(open_door, check_call(cmd));

    foreach(mixed cmd: close_commands)
	add_action(close_door, check_call(cmd));

    foreach(mixed cmd: lock_commands)
	add_action(lock_door, check_call(cmd));

    foreach(mixed cmd: unlock_commands)
	add_action(unlock_door, check_call(cmd));

    if (sizeof(unlock_commands))
	add_action(pick_lock, "pick");

    foreach(mixed cmd: knock_commands)
	add_action(knock_door, check_call(cmd));
}

/*
 * Function name: pass_door
 * Description:   Pass the door. Wizards can pass the door by giving the
 *                command with an exclamation point.
 * Arguments:	  arg - arguments given
 */
int
pass_door(string arg)
{
    string sub;
    int dexh;

    if (!other_door)
    {
	load_other_door();
    }

    if (this_player()->query_wiz_level() &&
        (extract(query_verb(), -1) == "!"))
    {
        arg = extract(query_verb(), 0, -2);
        sub = SECURITY->query_command_substitute(arg);
        this_player()->move_living((sub ? sub : arg) + " though the " + short(), other_room);
        return 1;
    }

    if (open_status)
    {
        /* The times higher a player can be and still get through */
        dexh = 2 + (this_player()->query_stat(SS_DEX) / 25);

	/* Lets say we arbitrarily can bend as dexh indicates.
	   For something else, inherit and redefine.
	 */
	if ((int)this_player()->query_prop(CONT_I_HEIGHT) > 
			query_prop(DOOR_I_HEIGHT) * dexh) 
	{
	    write("The " + short() + " is more than " + 
		  LANG_WNUM(dexh) + " times lower than you.\n" +
		  "You're too tall and " +
		  "not enough dexterous to get through.\n");
	    return 1;
	}
	else if ((int)this_player()->query_prop(CONT_I_HEIGHT) > 
			query_prop(DOOR_I_HEIGHT))
	{
	    write("You bend down to pass through the " + short() + ".\n");
	    tell_room(environment(this_object()),
		      QCTNAME(this_player()) +
		      " bends down to pass through the " +
		      short() + ".\n", this_player());
	}

	this_player()->move_living(query_verb(), other_room);
    }
    else
    {
	write(check_call(fail_pass));
    }

    return 1;
}

/*
 * Function name: this_door
 * Description:   Decide if we refer to this door.
 * Arguments:     arg - the string to decide by.
 * Returns:       1 if we're the lucky victim.
 */
static int
this_door(string arg)
{
    object *objs;

    if (!strlen(arg)) return 0;

    objs = FIND_STR_IN_OBJECT(arg, environment());
    return (sizeof(objs) && (objs[0] == this_object()));
}

/*
 * Function name: open_door
 * Description:   Open the door.
 * Arguments:	  arg - arguments given
 */
int
open_door(string arg)
{
    if (!this_door(arg))
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
	return 0;
    }

    if (!other_door)
	load_other_door();

    if (!open_status)
    {
	if (lock_status)
	    write(check_call(fail_open[1]));
	else if (this_player()->query_stat(SS_STR) < open_str)
	    write("You lack the strength needed.\n");
	else
	{
	    write("Ok.\n");
	    say(QCTNAME(this_player()) + " " + check_call(open_mess[0]),
		this_player());
	    do_open_door("");
	    other_door->do_open_door(check_call(open_mess[1]));
	}
    }
    else
	write(check_call(fail_open[0]));

    return 1;
}

void
do_open_door(string mess)
{
    object env;

    env = environment(this_object());
    env->change_my_desc(check_call(open_desc), this_object());
    if (strlen(mess))
	tell_room(env, mess);
    open_status = 1;
}

/*
 * Function name: close_door
 * Description:   Close the door.
 * Arguments:	  arg - arguments given
 */
int
close_door(string arg)
{
    if (!this_door(arg))
    {
        notify_fail(capitalize(query_verb()) + " what?\n", 0);
	return 0;
    }

    if (!other_door)
	load_other_door();

    if (open_status)
    {
	if (this_player()->query_stat(SS_STR) < open_str)
	    write("You lack the strength needed.\n");
	else
	{
	    write("Ok.\n");
	    say(QCTNAME(this_player()) + " " +
		check_call(close_mess[0]), this_player());
	    do_close_door("");
	    other_door->do_close_door(check_call(close_mess[1]));
	}
    }
    else
	write(check_call(fail_close));

    return 1;
}

void
do_close_door(string mess)
{
    object env;

    env = environment(this_object());
    env->change_my_desc(check_call(closed_desc), this_object());
    if (strlen(mess))
	tell_room(env, mess);
    open_status = 0;
}

/*
 * Function name: lock_door
 * Description:   Lock the door.
 * Arguments:	  arg - arguments given
 */
int
lock_door(string arg)
{
    if (!lock_procedure(arg))
	return 0;

    if (!other_door)
	load_other_door();

    if (!lock_status)
    {
	if (open_status)
	    write(check_call(fail_lock[1]));
	else
	{
	    write("Ok.\n");
	    say(QCTNAME(this_player()) + " " +
		check_call(lock_mess[0]), this_player());
	    do_lock_door("");
	    other_door->do_lock_door(check_call(lock_mess[1]));
	}
    }
    else
	write(check_call(fail_lock[0]));

    return 1;
}

void
do_lock_door(string mess)
{
    if (strlen(mess))
	tell_room(environment(this_object()), mess);
    lock_status = 1;
}

/*
 * Function name: unlock_door
 * Description:   Unlock the door.
 * Arguments:	  arg - arguments given
 */
int
unlock_door(string arg)
{
    if (!lock_procedure(arg))
	return 0;

    if (!other_door)
	load_other_door();

    if (lock_status)
    {
	write("Ok.\n");
	say(QCTNAME(this_player()) + " " + check_call(unlock_mess[0]),
	    this_player());
	do_unlock_door("");
	other_door->do_unlock_door(check_call(unlock_mess[1]));
    }
    else
	write(check_call(fail_unlock));

    return 1;
}

void
do_unlock_door(string mess)
{
    if (strlen(mess))
	tell_room(environment(this_object()), mess);
    lock_status = 0;
}

/*
 * Function name: lock_procedure
 * Description:   This function is called to determine if
 *		  a lock can be locked/unlocked.
 *
 *  Two methods are used here:
 *
 *  1: No key. You unlock by typing "<command> <door_id>".
 *  2: Key. You unlock by typing "<command> <door_id> with <key_id>".
 *
 * Arguments:	  arg - the argument given to the command lock/unlock
 * Returns:       1 - Ok, 0 - Fail.
 * 
 */
int
lock_procedure(string arg)
{
    int use_key;
    string key_str, prep, door;
    object *key_ob;

    use_key = query_prop(DOOR_I_KEY);

    notify_fail(capitalize(query_verb()) + " what?\n", 0);
    if (!use_key)
    {
        if (!this_door(arg))
	{
	    return 0;
	}
        return 1;
    }

    if (!strlen(arg))
    {
        return 0;
    }

    if (sscanf(arg, "%s with %s", door, key_str) < 2)
    {
	door = arg;
    }
    if (!this_door(door))
    {
        return 0;
    }
    if (key_str && (sizeof(key_ob = FIND_STR_IN_OBJECT(key_str,
						       this_player()))))
    {
	if ((mixed)key_ob[0]->query_key() == key)
	    return 1;
        notify_fail("The " + key_ob[0]->short() + " doesn't fit.\n");
        return 0;
    }

    notify_fail(capitalize(query_verb()) + " " + door + " with what?\n");
    return 0;
}

/*
 * Function name: do_pick_lock
 * Description:   Here we pick the lock, redefine this function if you want
 *		  something to happen, like a trap or something.
 * Arguments:	  skill - randomized picking skill of player
 *		  pick  - how difficult to pick the lock
 */
void
do_pick_lock(int skill, int pick)
{
    if (skill > pick)
    {
	if (no_pick)
	{
	    write("You failed to pick the lock. It seems unpickable to you.\n");
	    return;
	}

	write("You get very satisfied when you hear a soft 'klick' from " +
	    "the lock.\n");
	say("You hear a soft 'klick' from the lock.\n");
	do_unlock_door("");
	other_door->do_unlock_door(check_call(unlock_mess[1]));
    } else if (skill < (pick - 50))
	write("You failed to pick the lock. It seems unpickable to you.\n");
    else
	write("You failed to pick the lock.\n");
}

/*
 * Function name: pick_lock
 * Description:   Pick the lock of the door.
 * Arguments:	  str - arguments given
 */
int
pick_lock(string str)
{
    int skill;
    string arg;

    notify_fail(capitalize(query_verb()) + " what?\n", 0);
    if (!str)
    {
	return 0;
    }
    if (sscanf(str, "lock on %s", arg) != 1)
    {
	arg = str;
    }
    if (!this_door(arg))
    {
	return 0;
    }

    if (!lock_status)
    {
	write("Much to your surprise, you find it unlocked already.\n");
	return 1;
    }

    if (this_player()->query_mana() < 10)
    {
        write("You can't concentrate enough to pick the lock.\n");
	return 1;
    }

    this_player()->add_mana(-10); /* Cost 10 mana to try to pick a lock.*/
    write("You try to pick the lock.\n");
    say(QCTNAME(this_player()) + " tries to pick the lock.\n");
    skill = random(this_player()->query_skill(SS_OPEN_LOCK));

    if (!other_door)
	load_other_door();

    do_pick_lock(skill, pick);

    return 1;      
}

/*
 * Function name:	knock_door
 * Description	:	knock on the door
 * Arguments	:	string arg -- whatever the player typed
 * Returns	:	1 if we understand the arg, 0 if not
 */
int
knock_door(string arg)
{
    string	tmpstr;

    notify_fail(capitalize(query_verb()) + " on what?\n", 0);
    if (!strlen(arg))
    {
	return 0;
    }

    if (sscanf(arg, "on %s", tmpstr))
    {
	arg = tmpstr;
    }
    if (!this_door(arg))
    {
	return 0;
    }

    if (!other_door)
    {
	load_other_door();
    }

    if (!open_status)
    {
	this_player()->catch_msg(knock_resp[0]);
	say(QCTNAME(this_player()) + " " + check_call(knock_resp[1]),
	    this_player());
	if ( objectp(other_door) )
	{
	     tell_room(environment(other_door), knock_resp[2]);
	}
    }
    else
    {
	this_player()->catch_msg(knock_fail[0]);
	say(QCTNAME(this_player()) + " " + check_call(knock_fail[1]),
	    this_player());
	if ( objectp(other_door) )
	{
	     tell_room(environment(other_door), knock_fail[2]);
	}
    }
    return 1;
}

/*
 * Function name: set_str
 * Description:   Set the strength needed to open/close the door
 * Arguments:     str - strength needed
 */
void
set_str(int str) { open_str = str; }

/*
 * Function name: set_no_pick
 * Description:   Make sure the lock of the door is not pickable
 */
void
set_no_pick() { no_pick = 1; }

/*
 * Function name: query_no_pick
 * Description:	  Return 1 if door not pickable
 */
int
query_no_pick()	{ return no_pick; }

/*
 * Function name: set_pick
 * Description:   Set the difficulty to pick the lock, 100 impossible, 10 easy
 */
void
set_pick(int i)	{ pick = i; }

/*
 * Function name: query_pick
 * Description:	  Returns how easy it is to pick the lock on a door
 */
int
query_pick() { return pick; }

/*
 * Function name: set_open
 * Description:   Set the open staus of the door
 */
void
set_open(int i)	{ open_status = i; }

/*
 * Function name: query_open
 * Description:   Query the open status of the door.
 */
int
query_open() { return open_status; }

/*
 * Function name: set_door_name
 * Description:	  Set the name of the door
 */
void
set_door_name(mixed name) 
{ 
    int i, s;
    if (!pointerp(name))
    {
        set_name(name);
        return;
    }
    set_name(name[0]);
    if ((s = sizeof(name)) > 1)
    {
        for (i = 1; i < s; i++)
        {
            add_name(name[i]);
        }
    }
}

/*
 * Function name: query_door_name
 * Description:   Return the name of the door
 */
mixed
query_door_name() { return query_names(); }

/*
 * Function name: set_door_desc
 * Description:   Set the long description of the door
 */
void
set_door_desc(string desc) { set_long(desc); }

/*
 * Function name: query_door_desc
 * Description:   Query the long description of the door
 */
string
query_door_desc() { return query_long(); }

/*
 * Function name: set_open_desc
 * Description:   Set the description of the door when open
 */
void
set_open_desc(string desc) { open_desc = desc; }

/*
 * Function name: query_open_desc
 * Description:   Query the open description of the door
 */
string
query_open_desc() { return open_desc; }

/*
 * Function name: set_closed_desc
 * Description:   Set the description of the door when closed
 */
void
set_closed_desc(string desc) { closed_desc = desc; }

/*
 * Function name: query_closed_desc
 * Description:   Query the description of the door when closed
 */
string
query_closed_desc() { return closed_desc; }

/*
 * Function name: set_pass_command
 * Description:   Set which command is needed to pass the door.
 *                Wizards can pass any door by adding an exclamation point
 *                to the command.
 */
void
set_pass_command(mixed command)
{
    if (!command)
	pass_commands = ({ });
    if (pointerp(command))
	pass_commands = command;
    else
	pass_commands = ({ command });
}

/*
 * Function name: query_pass_command
 * Description:   Query what command lets you pass the door
 */
string *
query_pass_command() { return pass_commands; }

/*
 * Function name: set_fail_pass
 * Description:   Set messaged when failing to pass the door.
 */
void
set_fail_pass(string mess) { fail_pass = mess; }

/*
 * Function name: query_fail_pass
 * Description:   Query message when failing to pass the door
 */
string
query_fail_pass() { return fail_pass; }

/*
 * Function name: set_open_command
 * Description:   Set command to open the door
 */
void
set_open_command(mixed command)
{
    if (!command)
	open_commands = ({ });
    if (pointerp(command))
	open_commands = command;
    else
	open_commands = ({ command });
}

/*
 * Function name: query_open_command
 * Description:   Query what command opens the door
 */
string *
query_open_command() { return open_commands; }

/*
 * Function name: set_open_mess
 * Description:   Set the message to appear when door opens
 */
void
set_open_mess(string *mess) { open_mess = mess; }

/*
 * Function name: query_open_mess
 * Description:   Query what messages we get when dor is opened
 */
string *query_open_mess() { return open_mess; }

/*
 * Function name: set_fail_open
 * Description:   Set the message when we fail to open door
 */
void	
set_fail_open(mixed mess)	
{ 
    if (!mess)
        fail_open = ({ });
    if (pointerp(mess))
	fail_open = mess;
    else
	fail_open = ({ mess });
}

/*
 * Function name: query_fail_open
 * Description:   Query message when open fails
 */
string	*
query_fail_open() { return fail_open; }

/*
 * Function name: set_close_command
 * Description:   Set what command closes the door
 */
void
set_close_command(mixed command)
{
    if (!command)
	close_commands = ({ });
    if (pointerp(command))
	close_commands = command;
    else
	close_commands = ({ command });
}

/*
 * Function name: query_close_command
 * Description:   Query what command closes the door
 */
string *
query_close_command() { return close_commands; }

/*
 * Function name: set_close_mess
 * Description:   Set the message to appear when we close the door
 */
void
set_close_mess(string *mess) { close_mess = mess; }

/*
 * Function name: query_close_mess
 * Description:   Query message when we close the door
 */
string *
query_close_mess() { return close_mess; }

/*
 * Function name: set_fail_close
 * Description:   Set message when we fail to close the door
 */
void
set_fail_close(string mess) { fail_close = mess; }

/*
 * Function name: query_fail_close
 * Description:   Query message when we fail to close the door
 */
string
query_fail_close() { return fail_close; }

/*
 * Function name: set_locked
 * Description:   Set lock status
 */
void
set_locked(int i) { lock_status = i; }

/*
 * Function name: query_locked
 * Description:   Query lock status
 */
int
query_locked() { return lock_status; }

/*
 * Function name: set_other_room
 * Description:   Set which rooms is on the other side
 */
void
set_other_room(string name) { other_room = name; }

/*
 * Function name: query_other_room
 * Description:   Query what room is on the other side
 */
string
query_other_room() { return other_room; }

/*
 * Function name: set_lock_name 
 * Description:   Set name of the lock
 */
void
set_lock_name(mixed name)
{
    lock_name = name;
    if (lock_desc)
        add_item(lock_name, lock_desc);
}

/*
 * Function name: query_lock_name
 * Description:   Query the name of the lock
 */
mixed
query_lock_name() { return lock_name; }

/*
 * Function name: set_lock_desc
 * Description:   Set the description of the lock
 */
void
set_lock_desc(string desc)
{
    lock_desc = desc;
    if (lock_name)
        add_item(lock_name, lock_desc);
}

/*
 * Function name: query_lockdesc
 * Description:   Query the description of the lock
 */
string
query_lock_desc() { return lock_desc; }

/*
 * Function name: set_locl_command
 * Description:   Set which command locks the door
 */
void
set_lock_command(mixed command)
{
    if (!command)
	lock_commands = ({ });
    if (pointerp(command))
	lock_commands = command;
    else
	lock_commands = ({ command });
}

/*
 * Function name: query_lock_command
 * Description:   Query what command locks the door
 */
string *
query_lock_command() { return lock_commands; }

/*
 * Function name: set_lock_mess
 * Description:   Set message when locking door
 */
void
set_lock_mess(string *mess) { lock_mess = mess; }

/*
 * Function name: query_lock_mess
 * Description:   Query the message when locking door
 */
string *
query_lock_mess() { return lock_mess; }

/*
 * Function name: set_fail_lock
 * Description:   Set message when fail to lock the door
 */
void	
set_fail_lock(mixed mess)	
{ 
    if (!mess)
	fail_lock = ({ });
    if (pointerp(mess))
	fail_lock = mess; 
    else
    	fail_lock = ({ mess });
}

/*
 * Function name: query_fail_lock
 * Description:   Query message when lock fails
 */
string*
query_fail_lock() { return fail_lock; }

/*
 * Function name: set_unlock_command
 * Description:   Set what command unlocks the door
 */
void
set_unlock_command(mixed command)
{
    if (!command)
	unlock_commands = ({ });
    if (pointerp(command))
	unlock_commands = command;
    else
	unlock_commands = ({ command });
}

/*
 * Function name: query_unlock_command
 * Description:   Query what command unlocks the door
 */
string *
query_unlock_command() { return unlock_commands; }

/*
 * Function name: set_unlock_mess
 * Description:   Set message when unlocking door
 */
void
set_unlock_mess(string *mess) { unlock_mess = mess; }

/*
 * Function name: query_unlock_mess
 * Description:   Query message when unlocking door
 */
string *
query_unlock_mess() { return unlock_mess; }

/*
 * Function name: set_fail_unlock
 * Description:   Set fail message when unlocking
 */
void
set_fail_unlock(string mess) { fail_unlock = mess; }

/*
 * Function name: query_fail_unlock
 * Description:   Query message when failing to unlock door
 */
string
query_fail_unlock() { return fail_unlock; }

/*
 * Function name: set_key
 * Description:   Set the number of the key that fits
 */
void	
set_key(mixed keyval)		
{ 
    do_set_key(keyval);
/*
 * These lines cause trouble when you do set_key() in the create_door(),
 * because by that time none of the doors has an environment yet...
 * You'll just have to set the same key value in the other door manually

    if (!other_door)
	load_other_door();
    other_door->do_set_key(keyval);
 */
}

/*
 * Function name: do_set_key
 * Description:   Called from the other side
 */
void
do_set_key(mixed keyval)
{
    key = keyval;
    add_prop(DOOR_I_KEY, 1);
}

/*
 * Function name: query_key
 * Description:   Query what key that fits.
 */
mixed
query_key() { return key; }

/*
 * Function name:	set_knock_command
 * Description	:	set the list of commands for knocking on the door
 * Arguments	:	mixed cmd -- a command string or an array of 
 *			command strings, or 0 to disable 'knock'.
 *			The default command is ({ "knock", "pound" }).
 */
void
set_knock_command(mixed cmd)
{
    if ( stringp(cmd) )
    {
	knock_commands = ({ cmd });
    }
    else if ( pointerp(cmd) )
    {
	knock_commands = cmd;
    }
    else
    {
	knock_commands = ({});
    }
}

/* 
 * Function name:	query_knock_command
 * Description	:	query the commands for knocking on a door
 * Returns	:	string * -- the commands
 */
string *
query_knock_command() { return knock_commands; }

/*
 * Function name:	set_knock_resp
 * Description	:	set the responses to knocking on a closed door
 * Arguments	:	string *msg -- array of msgs to display to
 *			this_player(), this room, and the other room
 */
void
set_knock_resp(string *msg) { knock_resp = msg;}

/* 
 * Function name:	query_knock_resp
 * Description	:	query the responses to knocking on a closed door
 * Returns	:	string * -- the responses
 */
string *
query_knock_resp() { return knock_resp; }

/*
 * Function name:	set_knock_fail
 * Description	:	set the responses to knocking on an open door
 * Arguments	:	string *msg -- array of msgs to display to
 *			this_player(), this room, and the other room
 */
void
set_knock_fail(string *msg) { knock_fail = msg; }

/* 
 * Function name:	query_knock_fail
 * Description	:	query the responses to knocking on an open door
 * Returns	:	string * -- the responses
 */
string *
query_knock_fail() { return knock_fail; }

/*
 * Function name: set_door_id
 * Description:   Set the id of the door
 */
void	
set_door_id(string id) 		
{ 
    door_id = id; 
}

/*
 * Function name: query_door_id
 * Description:   Query the id of the door
 */
string
query_door_id()	{ return door_id; }

/*
 * Function name: query_other_door
 * Description:   Get the other door object pointer. The other
 *		  door will be loaded if neccesary. If that proovs
 *		  impossible, this door will autodestruct.
 */
object
query_other_door()
{
    if (!other_door)
	load_other_door();

    return other_door;
}

/*
 * Function name: load_other_door
 * Description:   Try to load the door in the other room. If this
 *		  fails, autodestruct.
 */
void
load_other_door()
{
    string *door_ids;
    object *doors;
    int pos;

    seteuid(getuid());

    /*
     * No other side or already loaded.
     */
    if (!strlen(other_room) || other_door)
	return;

    /*
     * Try to load the other side.
     */
    if (!find_object(other_room))
    {
	other_room->teleledningsanka();
	if (!find_object(other_room))
	{
	    write("Error in loading other side of door: " + other_room + ".\n");
	    remove_door_info(environment(this_object()));
	    remove_object();
	    return;
	}
    }

    door_ids = (string *)other_room->query_prop(ROOM_AS_DOORID);
    doors = (object *)other_room->query_prop(ROOM_AO_DOOROB);
    pos = member_array(door_id, door_ids);
    if (pos < 0)
    {
	write("Other side of door is not in room after it has been loaded: " +
		other_room + ".\n");
	remove_door_info(environment(this_object()));
	remove_object();
	return;
    }

    other_door = doors[pos];
}

/*
 * Function name: add_door_info
 * Description:   Add information about this door to the room it
 *		  stands in. If this door already exists, autodestruct.
 * Arguments:	  dest - The room that contains the door.
 */
static void
add_door_info(object dest)
{
    string *door_ids;
    object *doors;

    door_ids = (string *)dest->query_prop(ROOM_AS_DOORID);
    doors = (object *)dest->query_prop(ROOM_AO_DOOROB);
    if (!pointerp(door_ids))
    {
	door_ids = ({});
	doors = ({});
    }
    
    /* One door of the same type is enough. */
    if (member_array(door_id, door_ids) >= 0)
    {
	write("One door is enough.\n");
	remove_object();
	return;
    }

    door_ids += ({ door_id });
    doors += ({ this_object() });

    dest->add_prop(ROOM_AS_DOORID, door_ids);
    dest->add_prop(ROOM_AO_DOOROB, doors);
}

/*
 * Function name: remove_door_info
 * Description:   Remove information about this door from the room it
 *		  stands in.
 * Arguments:	  dest - The room that contains the door.
 */
static void
remove_door_info(object dest)
{
    string *door_ids;
    object *doors;
    int pos;

    door_ids = (string *)dest->query_prop(ROOM_AS_DOORID);
    doors = (object *)dest->query_prop(ROOM_AO_DOOROB);
    if (!sizeof(door_ids))
	return;

    pos = member_array(door_id, door_ids);
    if (pos < 0)
	return;

    door_ids = exclude_array(door_ids, pos, pos);
    doors = exclude_array(doors, pos, pos);

    dest->add_prop(ROOM_AS_DOORID, door_ids);
    dest->add_prop(ROOM_AO_DOOROB, doors);
}

/*
 * Function name: enter_env
 * Description:   The door enters a room, activate it.
 * Arguments:	  dest - The destination room,
 * 		  old - Where it came from
 */
void
enter_env(object dest, object old)
{
    ::enter_env(dest, old);

    add_door_info(dest); 
    if (open_status)
    {
        dest->change_my_desc(check_call(open_desc), this_object());
    }
    else
    {
        dest->change_my_desc(check_call(closed_desc), this_object());
    }
}

/*
 * Function name: leave_env
 * Description:   The door leaves a room, remove it.
 * Arguments:     old - Where it came from,
 * 		  dest - The destination room
 */
void
leave_env(object old, object dest)
{
    ::leave_env(old, dest);

    if (!old)
    {
	return;
    }

    old->remove_my_desc(this_object());
    if (pointerp(lock_name))
    {
        if (sizeof(lock_name))
        {
            remove_item(lock_name[0]);
        }
    }
    else
    {
        remove_item(lock_name);
    }
    remove_door_info(old);
}

/*
 * Function name: get_pick_chance
 * Description  : Make a string from the difference in pick-skill of the
 *                player and pick-level of the container for the appraise
 *                by the player
 * Arguments    : pick_val - the mentioned difference
 * Returns      : string   - a nice string with the mentioned description.
 */
public nomask string
get_pick_chance(int pick_val)
{
    if (pick_val >= 40)
        return "pickable by only looking at it";
    else if (pick_val >= 30)
        return "very easy to pick";
    else if (pick_val >= 20)
        return "quite easy to pick";
    else if (pick_val >= 10)
        return "easy to pick";
    else if (pick_val >= 0)
        return "pickable";
    else if (pick_val >= -10)
        return "tricky, but pickable";
    else if (pick_val >= -20)
        return "difficult to pick";
    else if (pick_val >= -30)
        return "very hard to pick";
    else if (pick_val >= -40)
        return "almost impossible to pick";

    /* pick_val apparently < -40 */
    return "completely unpickable by you";
}

/*
 * Function name: appraise_object
 * Description  : This function is called when a player appraises the
 *                container to find out more about it.
 * Arguments    : num - use this num rather than the players appraise skill
 */
public varargs void
appraise_object(int num)
{
    int pick_level = pick;
    int seed;
    int skill;

    ::appraise_object(num);

    if (!key)
    {
        return;
    }

    skill = num ? num : (int)this_player()->query_skill(SS_APPR_OBJ);

    sscanf(OB_NUM(this_object()), "%d", seed);
    skill = random((1000 / (skill + 1)), seed);
    pick_level = (int)this_player()->query_skill(SS_OPEN_LOCK) -
        cut_sig_fig(pick_level + (skill % 2 ? -skill % 70 : skill) *
	pick_level / 100);

    write ("You appraise that its lock is " + get_pick_chance(pick_level) + ".\n");
}

/*
 * Function name: standard_open_desc
 */
string
standard_open_desc() 
{
  string temp_desc;
  temp_desc = "There is an open " + short();
  if (strlen(pass_commands[0]) <= 2)
    temp_desc = temp_desc + " leading " + pass_commands[1] + ".\n";
  else
    temp_desc = temp_desc + " here.\n";
  return temp_desc;
}


/*
 * Function name: standard_closed_desc
 */
string
standard_closed_desc()
{
  string temp_desc;
  temp_desc = "There is a closed " + short();
  if (strlen(pass_commands[0]) <= 2)
   temp_desc = temp_desc + " leading " + pass_commands[1] + ".\n";
  else
    temp_desc = temp_desc + " here.\n";
  return temp_desc;
}

/*
 * Function name: standard_open_mess1
 */
string
standard_open_mess1()
{
    return "opens the " + short() + ".\n";
}

/*
 * Function name: standard_open_mess2
 */
string
standard_open_mess2()
{
    return "The " + short() + " opens.\n";
}

/*
 * Function name: standard_fail_open1
 */
string
standard_fail_open1()
{
    return "The " + short() + " is already open.\n";
}

/*
 * Function name: standard_fail_open2
 */
string
standard_fail_open2()
{
    return "The " + short() + " is locked.\n";
}

/*
 * Function name: standard_close_mess1
 */
string
standard_close_mess1()
{
    return "closes the " + short() + ".\n";
}

/*
 * Function name: standard_close_mess2
 */
string
standard_close_mess2()
{ 
    return "The " + short() + " closes.\n";
}

/*
 * Function name: standard_lock_mess1
 */
string
standard_lock_mess1()
{
    return "locks the " + short() + ".\n";
}

/*
 * Function name: standard_lock_mess2
 */
string
standard_lock_mess2()
{
    return "You hear a soft 'click' from the " + short() + ".\n";
}

/*
 * Function name: standard_fail_lock1
 */
string
standard_fail_lock1()
{
    return "The " + short() + " is already locked.\n";
}

/*
 * Function name: standard_fail_lock2
 */
string
standard_fail_lock2()
{
     return "You must close the " + short() + " first.\n";
}

/*
 * Function name: standard_unlock_mess1
 */
string
standard_unlock_mess1()
{
     return "unlocks the " + short() + ".\n";
}

/*
 * Function name: standard_unlock_mess2
 */
string
standard_unlock_mess2()
{
    return "You hear a soft 'click' from the " + short() + ".\n";
}

/*
 * Function name: standard_door_desc
 */
string
standard_door_desc()
{
    return "A sturdy looking " + short() + ".\n";
}

/*
 * Function name: standard_fail_pass
 */
string
standard_fail_pass()
{
    return "The " + short() + " is closed.\n";
}

/*
 * Function name: standard_fail_close
 */
string
standard_fail_close()
{
     return "The " + short() + " is already closed.\n";
}

/*
 * Function name: standard_fail_unlock
 */
string
standard_fail_unlock()
{
    return "The " + short() + " is already unlocked.\n";
}

/*
 * Function name:	standard_knock_resp0
 * Description	:	give the default you-knock msg for a closed door
 * Returns	:	string -- the msg
 */
string
standard_knock_resp0()
{
    return "You " + query_verb() + " on the " + short() + ".\n";
}

/*
 * Function name:	standard_knock_resp1
 * Description	:	give the default foo-knocks msg for a closed door
 * Returns	:	string -- the msg
 */
string
standard_knock_resp1()
{
    return query_verb() + "s on the " + short() + ".\n";
}

/*
 * Function name:	standard_knock_resp2
 * Description	:	give the default someone-knocks msg for the
 *			other room of a closed door
 * Returns	:	string -- the msg
 */
string
standard_knock_resp2()
{
    /* not "Someone knocks on the door." -- that could be mistaken
     * for an invisible someone knocking on this side of the door.
     */
    return "You hear someone " + query_verb() + " on the " + short() + ".\n";
}

/*
 * Function name:	standard_knock_fail0
 * Description	:	give the default you-knock msg for an open door
 * Returns	:	string -- the msg
 */
string
standard_knock_fail0()
{
    return "You " + query_verb() + " on the open " + short() + ".\n";
}

/*
 * Function name:	standard_knock_fail1
 * Description	:	give the default foo-knocks msg for an open door
 * Returns	:	string -- the msg
 */
string
standard_knock_fail1()
{
    return "foolishly "+query_verb()+
	   "s on the open " + short() + ".\n";
}

/*
 * Function name:	standard_knock_fail2
 * Description	:	give the default someone-knocks msg for the
 *			other room of an open door
 * Returns	:	string -- the msg
 */
string
standard_knock_fail2()
{
    /* not "Someone knocks on the open door." -- that could be mistaken
     * for an invisible someone knocking on this side of the door.
     */
    return "You hear someone foolishly "+query_verb()+
	   " on the open " + short() + ".\n";
}

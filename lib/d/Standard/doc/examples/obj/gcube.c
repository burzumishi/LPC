/*
   gcube.c

   A breakable object

   This is a late night hack and most of the details can be refined,
   the purpose is only to show that the concept works.

*/
inherit "/std/object";

#include <macros.h>

int wholeness = 100;

create_object()
{
    set_short("glass cube");
    set_pshort("glass cubes");
    set_name("cube"); add_name("glass cube");
    set_pname("cubes"); add_pname("glass cubes");
    set_long("A not so solid locking cube of cheap glass.\n");
}

init()
{
  add_action("smash_me", "smash");
}

/*
   These were unexpectedly needed too
*/
mixed* query_weapon() { return ({}); }

string query_the_name() { return "the cube"; }

/*
	- hit_me(), should return ({  %hurt, "hitloc description", phit, dam })
*/
mixed* hit_me(int wcpen, int dt, object attacker, int aid)
{
  int phurt;

  if (wholeness < 1)
    return ({ 0, "glass", 0, 0 });

  phurt = 100*wcpen / wholeness;

  /*
  */
  wholeness -= wcpen;
  return ({ phurt, "glass", phurt, wcpen });
}

/*
	- query_hp(), should return 0 if the object is 'dead'
*/
int query_hp()
{
  if (wholeness < 1)
    return 0;
  else
    return wholeness;
}


/*
	- do_die() Do whatever when 'killed'. No return value.
*/
void do_die()
{
  tell_room(environment(this_object()), "The glass cube shatters!\n");
  wholeness = 0;
}

/*
	- query_ghost() should return 1 if object has been 'killed', but still
	  remains for some reason.
*/
int query_ghost()
{
  if (wholeness < 1)
    return 1;
  else
    return 0;
}

/*
Note also that the normal 'kill name' will not start combat with a nonliving
object. To initialize combat the object must call:
	
	- player->attack_object(this_object())
*/

int smash_me(string cmd)
{
  if (!id(cmd))
    return 0;

  if (wholeness < 1)
  { 
    write("The cube is already broken!\n");
    return 1;
  }

  /* The combat system currently demands opponents to be
     in the same room.
  */
  if (environment(this_object()) != environment(this_player()))
  {
    notify_fail("It must be in the room with you!\n");
    return 0;
  }

  write("You attack the glass cube!\n");
  say(QCTNAME(this_player()) + " attacks the glass cube!\n");
  this_player()->attack_object(this_object());
  return 1;
}

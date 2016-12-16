#pragma strict_types
#pragma no_clone

inherit "/std/room";

#include "/sys/stdproperties.h"
#include "/sys/macros.h"

#define	long_line  (strlen(ART_NONMETNAME) > 78 - strlen(arr_line[0] + \
		    arr_line[1]))

string arr_line;

/*
 * Prototypes
 */
int pray(string str);
int rest(string str);

void
create_room()
{
    add_prop(OBJ_S_WIZINFO, "@@wizinfo");
    set_short("LPmud standard church");
    
    set_long("You are in the local village church. " +
	     "There is a deep pit in the center.\n");

    add_exit("/d/Standard/start/mailroom","south","@@sblock");

    add_item(({"pit" }),
	     "It is very deep and smells badly. You don't dare go near " +
	     "the edge. They guy who made this room must be nuts.\n");
}

int
query_no_snoop()
{
    return 1;
}

void
init()
{
    add_action(pray, "pray");
    add_action(rest, "rest");
    ::init();
}

int
pray(string str)
{
    write("Since God doesn't exist, nothing happens.\n");
    if (!this_player()->query_invis())
	say( ({METNAME +" kneels down and mumbles.\n",
	       ART_NONMETNAME +" kneels down and mumbles.\n"}) );

    return 1;
}

int
rest(string str)
{
    write("Ah, wasn't that nice.\n");
    return 1;
}

string
wizinfo()
{
   return "This is the starting location for all players in the standard "+
	  "distribution of this mudlib. It should be replaced as soon "+
	  "as possible by the local administrator.\n";
}

int
sblock()
{
    return 0;
}

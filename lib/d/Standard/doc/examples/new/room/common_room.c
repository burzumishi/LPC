
/*
 * This is a Common Room, complete with tables,
 * chairs and local table conversations and a sample
 * local table emote.
 * Coding by Asmodean, started 4/29/1995
 * Modified by Xarguul (formerly Asmodean) on 5/22/1995
 *
 * 
 * NOTE : To inherit this file is quite easy, this is what a sample
 * inherit would look like:
 * (includes)
 * inherit "/doc/examples/new/room/common_room";
 *
 * create_room()
 * {
 *   ::create_room();
 *   set_short("The Xarguul is the niftiest Inn Common Room");
 *   set_long("This is the Common Room of the 'Xarguul is the "+
 *            "niftiest Inn'\n\n");
 *   add_exit("/d/Genesis/wiz/post","south");
 * }
 *
 * THIS IS ONLY AN EXAMPLE  (Standard Disclaimer)
 *
 * Also, if the drink prices are too high, remove them and
 * add your own drinks, and re-define the read() function.
 */

#include <composite.h>
#include "/d/Immortal/std/domain.h"
#include "/d/Immortal/pale/paladin/hold.h"

inherit "/std/room";
inherit "/lib/pub";

#include <stdproperties.h>
#include <macros.h>
#include <language.h>
#include <money.h>

int get_my_table_num();
string dump_tables();
string dump_chairs();
/* I have to declare do_stand because I call it from another
 * function --above-- it in the code.
 */
int do_stand(string str);

/*
 * Global variables to the room related to people in chairs.
 */

/* Table #s range from 0-6 */
#define MAXTABLES 7

static int *totalseats = ({ 2, 2, 2, 3, 4, 5, 5});
/* TableContents begins as empty, eventually will have the
 * player objects stored in them, for use in tablewhos and
 * table emotes and conversations.
 */
static mixed *tablecontents = ({
  ({}),({}),({}),({}),({}),({}),({}) });

void
create_room()
{
    set_short("Common Room");
    set_long(	"This is the common room of The Inn. It is "+
		"a cozy, warm environment. "+
		"There are several cushioned chairs and tables arranged "+
		"throughout the common room. There is a warm fireplace "+
		"on the far wall. The smell of hot meals and liquor from "+
		"the kegs behind the bar drifts through the smoky room. "+
		"There is a menu behind the bar.\n\n");
    
    /* My Personal policy - add_items make rooms so much more
     * realistic, and they add feel to places like common rooms.
     * You should note that if you inherit this files, you can
     * remove these items with remove_item()
     */
    add_item("bar", "The bar is made of expensive, rare red oak, which "+
	     "is incredibly hard to come by in an underground city. "+
	     "There is a menu behind it.\n");
    add_item("menu", "The menu is scribed into a golden plaque attached "+
	     "to a piece of fine mahogony hanging on the wall behind the "+
	     "bar.\n");
    /* The Following add_item contains a VBFC call to dump_chairs, 
     * which is a function that uses write() to print all the
     * chairs and which ones are empty. For more info on VBFC,
     * check out the manual.
     */
    add_item("chairs",
	     "They are made out of wood as is the rest of the "+
	     "place.\n@@dump_chairs@@"); 
    /* 
     * The VBFC here uses write() to print out who's sitting at
     * which tables.
     */
    add_item("tables", "The tables are made of a fine mahogony, the "+
	     "edges and legs are decorated with exquisite hand-carved "+
              "scrollwork. Try 'tablehelp' for more info."+
		"\n@@dump_tables@@");
    add_item(({"fireplace","hearth"}),
	     "The fireplace is a magnificent piece of work. Golden "+
	     "scrollwork surrounds the entire hearth. In front of the "+
	     "hearth is a magnificent spark guard, finely detailed in "+
	     "gold and silver, causing the warm amber flicker of the "+
	     "fire to reflect yellow and orange beams across the far "+
	     "wall of the common room. Regularly, almost on the hour, "+
	     "a finely dressed maid appears to stoke the fire.\n");

    add_cmd_item("menu", "read", "@@read");

    INSIDE;

    /*
     * The prices below --should-- be accurate, but they were originally
     * coded for the Immortal domain, so check them before using.
     */

    add_drink( ({ "ale", "ales", "large", "large ale", "large fine ale",
	"fine ale" }),
         "ale","large",250,58,415,0,0,
        "A mug of some of the finest ale you've ever tasted.\n",0);
    add_drink( ({"wine", "wines","spiced wine","fine spiced wine",
	"vintage wine","spiced wine","fine spiced vintage wine" }),
         "wine","vintage",200,55,375,0,0,
        "A vintage red wine, from the finest vineyards. A truly "+
	"wonderful glass of wine.\n",0);
    add_food( ({ "watermelon","imported watermelon" }),
        "watermelon","imported",350,250,0,0,
       "It looks quite juicy, despite being a smaller, imported melon.\n",0);
    add_food( ({"venison","stew","venison stew"}),
        "stew", "venison", 400,326,"huge bowl of venison stew",
        "huge bowls of venison stew",
        "An absolutely enormous bowl filled with steaming, rich "+
	"broth containing large chunks of venison and other hot, "+
	"fresh vegetables.\n",0);
    add_food( ({ "steak","large","large prime rib",
                 "large steak"}),
        "steak","large",500,506,"large prime rib steak",
        "large prime rib steaks",
        "This is an exquisite (and enormous) cut of fine prime rib, "+
	"weighing in at a steamy, delicious 500 grams (20 oz.)\n",0);

    /*
     * There are no add_exits, please define them in your own
     * version of create room. See top of document for info on
     * how to inherit this file.
     */
}

/*
 * Function name: init
 * Description:   Initalize the pub actions
 */
void
init()
{
    /* If you do not call the previous instance of init if you've
     * re-defined init() (which we've done here), then NO useful
     * commands will work in a room at all! 
     */

    ::init();

    init_pub();
    add_action("do_sit","sit");
    add_action("do_stand","stand");
    add_action("table_say","tablesay");
    add_action("table_who","tablewho");
    add_action("table_help","tablehelp");
    add_action("table_smile","tablesmile");
}

leave_inv(object ob, object to)
{
   if (ob->query_prop("_is_sitting_down_AsMo_"))
    {
      tablecontents[get_my_table_num()]-= ({ ob });
      ob->remove_prop("_is_sitting_down_AsMo_");
      remove_my_desc(TO);
      change_my_desc(figure_out_desc(),TO);
    }
   ::leave_inv(ob,to);
}

/* pub_hook_player_buys is a function in /lib/pub, called
 * each time a player buys something. I have re-defined it
 * so that if the player is standing, he/she gets a different
 * message than sitting at a table (a waiter brings food/drink
 * to the tables)
 */

void
pub_hook_player_buys(object ob, int price)
{
  if (TP->query_prop("_is_sitting_down_AsMo_"))
    {
      write("A waiter picks up "+ob->short()+" and brings it to "+
	    "you, taking "+price+" coppers from you.\n");
      tell_room(TO,"A waiter picks up "+ob->short()+" and brings it to "+
                QCTNAME(TP)+"'s table, taking "+price+" coppers from "+
		TP->query_objective()+".\n",TP);
    }
  else
    {
      write("You drop "+price+" coppers onto the bar and pick up "+
	    ob->short()+".\n");
      tell_room(TO,QCTNAME(TP)+" drops "+price+" coppers onto the bar "+
               "and picks up "+ob->short()+".\n");
     }
}

/*
 * The ONLY reason I used a VBFC instead of typing it, is
 * because it seems clearer to me to have this big, bulky
 * menu in its own function.
 *
 * NOTE : These prices were originally for the Immortal domain,
 * so if you are inheriting this, not using a modified version,
 * please double-check these. If you want a new menu, simply
 * re-define the 'read' function in your common room.
 */

int
read(string str)
{
    write(
	"                  Common Room Menu\n"+
        ".--------------------------------------------------.\n" +
        "| o                                              o |\n" +
        "|                                                  |\n" +
        "|   Large, Fine ale           2 gc  10 sc 7 cc     |\n" +
        "|                                                  |\n" +
        "|   Vintage Spiced Wine       2 gc  7 sc  3 cc     |\n" +
        "|                                                  |\n" +
        "|   Imported Watermelon       1 gc  8 sc 10 cc     |\n" +
        "|                                                  |\n" +
        "|   Venison Stew              2 gc  3 sc  2 cc     |\n" +
        "|                                                  |\n" +
        "|   Large 20 oz. Prime Rib    3 gc  6 sc  2 cc     |\n" +
        "|                                                  |\n" +
        "| o                                              o |\n" +
        "`--------------------------------------------------'\n\n");
    return 1;
}

/*
 * CAUTION : This is where the code gets a bit nasty.
 * This function decides what the extra desc looks like
 * for people sitting down in the room or not.
 */

public string
figure_out_desc()
{
    object *p;
   p = filter(all_inventory(TO),"prop_filt",TO);
    if (!sizeof(p))
        return "No one is sitting at any of the tables.\n";
    if (sizeof(p) == 1)
         return COMPOSITE_LIVE(p)+" is sitting at a table.\n";
   else
   return COMPOSITE_LIVE(p)+" are sitting at tables.\n";
}

/*
 * This is a filter function which filters out people
 * who are not sitting down (or other way around depending
 * on how you use it. All people sitting down have the
 * property '_is_sitting_down_AsMo_'
 */

public int
prop_filt(object player)
{
    if (!player->query_prop("_is_sitting_down_AsMo_"))
        return 0;
    return 1;
}

/*
 * Called when a player types 'sit at table x'
 * This performs all modifications to the tablecontents
 * array and properties of players
 */

int
do_sit(string str)
{
  string *argarr;
  int tablenumber;

  if (TP->query_prop("_is_sitting_down_AsMo_")==1)
    {
      NF("You're already sitting down!\n");
      return 0;
    }
 
  if (!str)
   {
    NF("Try 'sit at table #'. For more info on the tables, "+
       "Use 'tablehelp'\n");
    return 0;
   }
  argarr=explode(str," ");
  if (sizeof(argarr)<2)
   {
    NF("What are you trying to do? Try 'tablehelp' if you're "+
       "confused.\n");
    return 0;
   }
  if ( (argarr[0]!="at") && (argarr[1]!="table") )
    {
      NF("Sit Where?\n");
      return 0;
    }
  if (sizeof(argarr)!=3) 
    {
     NF("Sit at which table?\n");
     return 0;
    }
  tablenumber=atoi(argarr[2]);
  if ( (tablenumber < 0) || (tablenumber > MAXTABLES) )
    {
      NF("You can't seem to find that particular table here.\n");
      return 0;
    }
  if (sizeof(tablecontents[tablenumber-1])==totalseats[tablenumber-1])
    {
      NF("There are no more chairs at that table.\n");
      return 0;
    }
  tell_room(TO,QCTNAME(TP)+" sits down in a chair at a table.\n",TP);
  TP->catch_msg("You sit down at table " + tablenumber + ".\n");
  TP->add_prop("_is_sitting_down_AsMo_",1);
  remove_my_desc(TO);
  change_my_desc(figure_out_desc(),TO);
  tablecontents[tablenumber-1]+=({ TP });
  return 1;
}

/*
 * Called when people stand up
 */

int
do_stand(string str)
{
  int i;

  if (TP->query_prop("_is_sitting_down_AsMo_")!=1)
    {
      NF("You're not sitting down!\n");
      return 0;
    }
  tell_room(TO,QCTNAME(TP)+" pushes "+TP->query_possessive()+" chair "+
            "away from the table and stands up.\n",TP);
  TP->catch_msg("You push your chair away from the table and stand up.\n");
  TP->remove_prop("_is_sitting_down_AsMo_");
  for (i=0;i<MAXTABLES;i++)
    {
      if ( (member_array(TP,tablecontents[i]))!=-1)
	tablecontents[i]-=({TP});
    }
  remove_my_desc(TO);
  change_my_desc(figure_out_desc(),TO);
  return 1;
  
}

/*
 * This was mentioned before, this is done inside a VBFC
 * to print out who is sitting at which table
 */

string
dump_tables()
{
   int i;

   for (i=0; i<MAXTABLES; i++)
     {
      write("Seated at table "+LANG_WNUM(i+1)+": ");
      if (sizeof(tablecontents[i]))
	{
         write(COMPOSITE_LIVE(tablecontents[i])+"\n");
       }
      else
	write("No one.\n");
    }
   return "";
 }

/*
 * Function to carry on local conversations by typing
 * 'tablesay <msg>'
 */

int
table_say(string str)
{
  int i;
   int mytable;
  int j;

  if (!str)
    {
      NF("Say What?\n");
      return 0;
    }
  if (!TP->query_prop("_is_sitting_down_AsMo_"))
    {
      NF("You're not sitting at a table.\n");
      return 0;
    }
  for (i=0; i<MAXTABLES; i++)
    {
      if ( (member_array(TP,tablecontents[i]))!=-1)
	{
          mytable=i;
	  for (j=0; j<sizeof(tablecontents[i]); j++)
	    {
	       if (tablecontents[i][j]->query_real_name()!=
		   TP->query_real_name())
		 {
		   tablecontents[i][j]->catch_msg(QCTNAME(TP)+" leans "+
			"across the table and says: "+str+"\n");
		 }
	     }
	}
    }
  TP->catch_msg("You lean across the table and say: "+str+"\n");
  tell_room(TO,QCTNAME(TP)+" murmurs something to those at "+
            TP->query_possessive()+" table.\n",tablecontents[mytable]);
  return 1;
}

/*
 * Lists people sitting at your table.
 */

int
table_who(string str)
{
  object *tmparr;
  int i;

  if (!TP->query_prop("_is_sitting_down_AsMo_"))
    {
      NF("You're not sitting at a table.\n");
      return 0;
    }
  for (i=0; i<MAXTABLES; i++)
    {
      if ( (member_array(TP,tablecontents[i]))!=-1)
	{
          tmparr=tablecontents[i];
          tmparr-= ({ TP });
          if (!sizeof(tmparr)) {
		write("There is no one else at the table.\n"); 
		return 1;
	}
	  write("You glance across the table and see:\n"+
           COMPOSITE_LIVE(tmparr)+"\n");
	}
    }
  return 1;
}

/*
 * This is outputted as the
 * 'tablehelp' command
 */
int
table_help(string str)
{
  write("\n"+
	".-------------------------------------.\n"+
	"| o                                 o |\n"+
	"|                                     |\n"+
	"| sit at table x                      |\n"+
	"| stand                               |\n"+
	"| tablesay <msg> - Talk to your Table |\n"+
        "| tablewho - look at your table       |\n"+
	"| tablehelp - this message            |\n"+
	"|                                     |\n"+
	"| o                                 o |\n"+
	"`-------------------------------------'\n\n");
  return 1;
}

/*
 * This is a utility function which returns the
 * table index number in the array of tables which
 * you are sitting at.
 */

int
get_my_table_num()
{
  int i;
  
  for (i=0;i<MAXTABLES;i++)
    {
      if ( (member_array(TP,tablecontents[i]))!=-1)
	return i;
    }
  return -1;
}

/*
 * table-local smile emote
 */
int
table_smile(string str)
{
  
  int n;
  int i;
  int sz;
  object *tmparr;

  if (!TP->query_prop("_is_sitting_down_AsMo_"))
    {
      NF("You're not sitting at a table!\n");
      return 0;
    }
  n = get_my_table_num();
  if (n==-1)
    {
      NF("Hmm..Can't seem to figure out what the hell's going on here.\n"+
	 "Notify the spiffy Asmo ASAP.\n");
      return 0;
    }
  tmparr=tablecontents[n];
  tmparr-= ({ TP });
  sz = sizeof(tmparr);
  if (!sz)
    {
      NF("There is no one else at the table.\n");
      return 0;
    }
  for (i=0; i<sz; i++)
    {
      tmparr[i]->catch_msg(QCTNAME(TP)+" smiles across the table.\n");
    }
  TP->catch_msg("You smile across the table.\n");
  return 1;
}

/*
 * Lists which chairs are empty and which are not
 */
string
dump_chairs()
{
  int i;
  int e;
  
  for (i=0;i<MAXTABLES;i++)
    {
      e = totalseats[i]-sizeof(tablecontents[i]);
      write("Table "+LANG_WNUM(i+1)+" has "+totalseats[i]+" chairs. "+
	    "( "+e+" are empty.)\n");
    }
  return "";
}




















/*
 *  The Hobbiton alchemist
 *
 *  The alchemist can change certain items into other, useful things
 *  for payment. It is possible to add new items with the function
 *
 *	add_transform_item(name, filename, price, filename2);
 *
 *  		name		The name of the object that is to be
 *				recognised, e.g. "carrot".
 *		filename	Filename of the object that is to be
 *				recognised, e.g. "/d/Shire/common/obj/carrot"
 *		price		The price that is asked for the changing of
 *				the object into the other object.
 *		filename2	Filename of the object after the trans-
 *				formation, e.g. "/d/Shire/common/obj/potion"
 *				Note that filename2 may also be an array, in
 *				which case a random choice of the objects
 *				is made.
 *
 *  If the same item (i.e. with the same name and filename) is added, only
 *  the latest definition will be remembered. An itemdefinition can be
 *  removed from the alchemists memory with the function
 *
 *	remove_transform_item(name, filename);
 *
 *  The alchemist keeps track of all transformable items by saving them in
 *  a file, so he will also remember them next reboot.
 *
 *  How does the alchemist work?
 *	- Give him an object, he will say if he can change it into something,
 *	  and for what cost.
 *	- If you don't like it, say "give it back".
 *	- If you do like it, pay him, and he will transform the given object.
 *
 *							Tricky, June 1992
 */

inherit "/std/monster";
inherit "/lib/trade";

#include "defs.h"
#include "/sys/money.h"
#include "/sys/macros.h"
#include "/sys/ss_types.h"
#include "/sys/stdproperties.h"

#define NUM sizeof(MONEY_TYPES) /* How many types of money do we use? */

/* This variable will be saved: */
mixed *transform_arr;

/* These variables will not be saved: */
static string pay_type, get_type, coin_pay_text, coin_get_text, *sold_arr,
              *tr_name_arr, *tr_fname_arr, *from_arr;
static object *given_obj_arr;
static mixed tr_fname2_arr;
static int *money_arr, *tr_price_arr;

/* Prototypes */
void restore_memory();
void save_memory();
int can_afford(int price);
void give_back(object what);


void
create_monster()
{
   if (!IS_CLONE)
      return;

   set_name("alchemist");
   set_race_name("alchemist"); 
   set_living_name("_alchemist_");
   set_adj("town");
   set_long("@@my_long");

   add_prop(CONT_I_WEIGHT,47000);   /* 47 Kg */
   add_prop(CONT_I_HEIGHT,87);      /* 87 cm */
   add_prop(LIVE_I_NEVERKNOWN,1);

            /* str dex con int wis dis */
   set_stats(({ 22, 27, 21, 70, 70, 34}));
   set_hp(10000); /* Heal fully */
   set_skill(SS_UNARM_COMBAT, 70);
   set_skill(SS_DEFENCE, 47);

   set_aggressive(0);
   set_attack_chance(0);

   set_chat_time(5 + random(6));
   add_chat("@@hint_chat");
   add_chat("Prococlum querido!");
   add_chat("Renegrato quiclocum nomenclatura!");
   add_chat("Desdegnifiat spegliak fronkium!");
   add_chat("If you give me something, I can examine it.");
   add_chat("Hmmm. I love it when a plan comes together...");
   add_chat("Be careful, the roads are not safe nowadays.");

   set_cchat_time(3);
   add_cchat("How dare you attack a weak old man like me?");
   add_cchat("I am defenseless!");
   add_cchat("If you continue, I will turn you into a newt.");
   add_cchat("Never mess with an alchemist...");

   /* Triggers */
   trig_new("%w 'smiles' %s", "react_smile");
   trig_new("%w 'smirks.\n' %s", "react_smirk");
   trig_new("%w 'sighs' %s", "react_sigh");
   trig_new("%w 'introduces' %s", "react_introduce");
   trig_new("%w 'nods' %s", "react_nod");
   trig_new("%w 'grins' %s", "react_grin");
   trig_new("%w 'giggles' %s", "react_giggle");
   trig_new("%w 'frowns.\n'", "react_frown");
   trig_new("%w 'bows' %s", "react_bow");
   trig_new("%w 'growls.\n' %s", "react_growl");
   trig_new("%w 'growls' %s", "react_growl");
   trig_new("%w 'waves' %s", "react_wave");
   trig_new("%w 'shrugs' %s", "react_shrug");
   trig_new("%w 'spits' %s", "react_spit");
   trig_new("%w 'throws' %w 'head' 'back' 'and' 'cackles' 'with' 'glee!\n' %s",
         "react_cackle");
   trig_new("%w 'falls' 'down' %s", "react_laugh");
   trig_new("%w 'shakes' %w 'head' %s", "react_shake");

   /* Set up the trading system */
   config_default_trade();
                         /* Copper Silver Gold Platinum  */
   set_money_give_out(   ({ 10000,   700,  10,   10 })); /* Max_pay   */
   set_money_give_reduce(({     0,     0,   0,    0 })); /* Threshold */
   add_prop(OBJ_S_WIZINFO, "@@wizinfo");

   restore_memory();
}

void
init_living()
{
   add_action("do_pay","pay",0);
}

string
my_long()
{
   string str;

   str = "You are looking at the local alchemist, who is known to "
       + "possess the power to transform certain objects into other objects. "
       + "Simply give him an object, so he can judge it. If you want him "
       + "to transform something, pay him, else say 'give it back'.";

   if (this_player()->query_wiz_level())
      str += " Do \"info alchemist\" to get more info.\n";
   else
      str += "\n";

   return break_string(str, 70);
}

string
wizinfo()
{
   string str;
   int i;

   str =
    "\nYou can add your own stuff to the alchemist by doing:\n"
  + "   Call alchemist add_transform_item <name>%%<fname>%%<price>%%<fname2>\n"
  + "You can remove your stuff again by doing:\n"
  + "   Call alchemist remove_transform_item <name>%%<fname>\n"
  + "Read the alchemist.c file for more specific info on adding stuff.\n\n"
  + "At the moment the alchemist can transform the following objects:\n";
   if (tr_name_arr)
      for (i=0; i < sizeof(tr_name_arr); i++)
         str += sprintf("%-16s %-60s\n", tr_name_arr[i], tr_fname_arr[i]);
   else
      str += "          Nothing.\n";

   return str;
}

/********************************************************************
 *
 * Player commands
 */

int
do_pay(string str) {
   int i, j;
   object ob, tp;
   string item;

   if (!str)
      return 0;

   /* Check if the player defines what she would like in return. */
   if (sscanf(str, "alchemist and get %s",get_type) != 3)
   {
      get_type = "";
      if (str != "alchemist")
      {
         notify_fail("You can simply type 'pay alchemist'.\n");
         return 0;
      }
   }

   tp = this_player();

   if ((i = member_array(tp->query_real_name(), from_arr)) < 0)
   {
      write("The alchemist does not have anything of you.\n");
      return 1;
   }

   if ((j = member_array(MASTER_OB(given_obj_arr[i]), tr_fname_arr)) < 0)
   {
      give_back(given_obj_arr[i]);
      return 1;
   }

   if (can_afford(tr_price_arr[j]))
   {
      if (pointerp(tr_fname2_arr[j])) /* Was it an array? */
         ob = clone_object(tr_fname2_arr[j][random(sizeof(tr_fname2_arr[j]))]);
      else
         ob = clone_object(tr_fname2_arr[j]);

      if (!ob)
      {
         command("say Ooops... something went wrong!");
         return 1;
      }

      ob->move(this_object());
      tell_room(environment(), "The alchemist mumbles some words.\n"
       + "There is a flash and a cloud of smoke covers the "
       + check_call(given_obj_arr[i]->short()) + ".\n"
       + "When the smoke clears you see "
       + LANG_ADDART(check_call(ob->short())) + ".\n");
      command("say Here you are, "
       + ((tp->query_gender() == G_MALE) ? "master" : "lady") + ".");
      command("give " + ob->query_name() + " to " + tp->query_real_name());

      /* Now remove her from the arrays */
      from_arr = exclude_array(from_arr, i, i);
      given_obj_arr[i]->remove_object();
      given_obj_arr = exclude_array(given_obj_arr, i, i);
   }
   else
   {
      command("laugh");
      command("say You cannot afford it, "
       + ((tp->query_gender() == G_MALE) ? "master" : "lady") + ".");
   }
   return 1;
}

/*
 *  Test if this_player can afford the price, and perform
 *  the money transactions.
 */
int
can_afford(int price)
{
   /* Try to take the money */
   if (sizeof(money_arr = pay(price, this_player(), 0, 0, 0, get_type)) == 1)
      return 0;

   coin_pay_text = text(exclude_array(money_arr, NUM, NUM*2-1));
   coin_get_text = text(exclude_array(money_arr, 0, NUM-1));

   write("You pay the alchemist.\n");
   if (coin_get_text)
      write("You get " + coin_get_text + " back.\n");
   return 1;
}

void
restore_memory()
{
   int i;
   seteuid(getuid());
   restore_object(STAND_DIR + "alchemist");

   tr_name_arr   = ({ });
   tr_fname_arr  = ({ });
   tr_price_arr  = ({ });
   tr_fname2_arr = ({ });

   for (i=0; i < sizeof(transform_arr); i++)
   {
      tr_name_arr   += ({ transform_arr[i][0] });
      tr_fname_arr  += ({ transform_arr[i][1] });
      tr_price_arr  += ({ transform_arr[i][2] });
      tr_fname2_arr += ({ transform_arr[i][3] });
   }
}

void
save_memory()
{
   int i;

   transform_arr = ({ });

   for (i=0; i < sizeof(tr_name_arr); i++)
      transform_arr += ({ ({ tr_name_arr[i], tr_fname_arr[i],
                             tr_price_arr[i], tr_fname2_arr[i] }) });
   seteuid(getuid());
   save_object(STAND_DIR + "alchemist");
}

void
remove_transform_item(string name, string filename)
{
   int i;

   for (i=0; i < sizeof(tr_name_arr); i++)
   {
      if (name == tr_name_arr[i] && filename == tr_fname_arr[i])
      {
         tr_name_arr   = exclude_array(tr_name_arr, i, i);
         tr_fname_arr  = exclude_array(tr_fname_arr, i, i);
         tr_price_arr  = exclude_array(tr_price_arr, i, i);
         tr_fname2_arr = exclude_array(tr_fname2_arr, i, i);
         break;
      }
   }
   save_memory();
}

void
add_transform_item(string name, string filename, int price, mixed filename2)
{
   int i;

   if (!name)
      return;

   if (!filename)
      return;

   if (price <= 0)
      price = 1;   /* One cannot pay 0 coins... */

   if (!filename2)
      return;

   /* Remove a possible old occurrence */
   for (i=0; i < sizeof(tr_name_arr); i++)
   {
      if (name == tr_name_arr[i] && filename == tr_fname_arr[i])
      {
         tr_name_arr   = exclude_array(tr_name_arr, i, i);
         tr_fname_arr  = exclude_array(tr_fname_arr, i, i);
         tr_price_arr  = exclude_array(tr_price_arr, i, i);
         tr_fname2_arr = exclude_array(tr_fname2_arr, i, i);
         break;
      }
   }

   /* Correct arrays that just happen to be 0 */
   if (!tr_name_arr)
      tr_name_arr   = ({ });
   if (!tr_fname_arr)
      tr_fname_arr  = ({ });
   if (!tr_price_arr)
      tr_price_arr  = ({ });
   if (!tr_fname2_arr)
      tr_fname2_arr = ({ });

   /* Add the new set */
   tr_name_arr   += ({ name });
   tr_fname_arr  += ({ filename });
   tr_price_arr  += ({ price });
   tr_fname2_arr += ({ filename2 });
   save_memory();
}


string
hint_chat()
{
   string r_item;
   int ran;

   if (!sizeof(tr_name_arr))
      return "And isn't it a lovely day?";

   r_item = tr_name_arr[random(sizeof(tr_name_arr))];

   ran = random(4);

   switch (ran)
   {
      case 0: return "I wish I could get my hands on " + LANG_ADDART(r_item)
                   + ".";
      case 1: return "Have you ever seen " + LANG_ADDART(r_item) + "?";
      case 2: return LANG_PWORD(r_item) + " are useful objects.";
      case 3: return "Bring me some " + LANG_PWORD(r_item) + ".";
   }
}

void
enter_inv(object what, object from)
{
   int ran;

   ::enter_inv(what, from);

   if (!from) /* Perhaps it was cloned and moved to me... */
      return;

   if (!given_obj_arr)
      given_obj_arr = ({ });

   if (!from_arr)
      from_arr = ({ });

   if (member_array(from->query_real_name(), from_arr) >= 0)
   {
      call_out("give_back_immediately", 1, ({what,from}));
      return;
   }

   given_obj_arr += ({ what });
   from_arr += ({ from->query_real_name() });

   if (member_array(MASTER_OB(what), tr_fname_arr) < 0)
      call_out("give_back", 3 + random(4), what);
   else
      call_out("name_price", 3 + random(4), what);

   call_out("thank_chat", 1, ({ from, what }));
}

void
thank_chat(object *arr)
{
   object from, what;
   int ran;

   ran = random(4);

   from = arr[0];
   what = arr[1];
   switch (ran)
   {
      case 0: command("say Ah, thank you. Now let me see...");
              break;
      case 1: command("say "
                    + capitalize(LANG_ADDART(check_call(what->short())))
                    + ", what a coincidence...");
              break;
      case 2: command("say "
                    + capitalize(LANG_ADDART(check_call(what->short())))
                    + ", how interesting...");
              break;
      case 3: command("say I will examine this, " + from->query_race() + ".");
   }
   return;
}

void
give_back_immediately(mixed return_arr)
{
   command("say I am already busy with one of your objects, "
         + return_arr[1]->query_race() + ".");
   command("give " + return_arr[0]->query_name() + " to "
         + return_arr[1]->query_real_name());
}

void
give_back(object what)
{
   int i, ran;
   string str;
   object who;

   if ((i = member_array(what, given_obj_arr)) < 0)
      return;

   if (who = present(from_arr[i], environment()))
   {
      ran = random(4);
      switch (ran)
      {
         case 0: str = "I don't know what to do with it, "
                     + who->query_race() + ".";
                 break;
         case 1: str = "Why do " + LANG_PWORD(who->query_race()) + " always "
                     + "come with useless objects?";
                 break;
         case 2: str = "This thing looks useless to me, "
                     + who->query_race() + ".";
                 break;
         case 3: str = "No, this thing is useless.";
      }
      command("say " + str);
      command("give " + what->query_name() + " to " + from_arr[i]);
      from_arr = exclude_array(from_arr, i, i);
      given_obj_arr = exclude_array(given_obj_arr, i, i);
   }
   else
   {
      command("say I never asked for this thing!");
      command("drop " + what->query_name());
      from_arr = exclude_array(from_arr, i, i);
      given_obj_arr = exclude_array(given_obj_arr, i, i);
   }
}

void
name_price(object what)
{
   int i, j, ran, price;
   string str;
   object who;

   if ((i = member_array(what, given_obj_arr)) < 0)
      return;

   if (who = present(from_arr[i], environment()))
   {
      if ((j = member_array(MASTER_OB(what), tr_fname_arr)) < 0)
      {
         give_back(what);
         return;
      }
      ran = random(4);
      switch (ran)
      {
         case 0: str = "This " + check_call(what->short()) + " looks useful, "
                     + who->query_race() + ".";
                 break;
         case 1: str = "I could turn this " + check_call(what->short())
                     + " into something else, " + who->query_race() + ".";
                 break;
         case 2: str = "Ah, a well-known " + check_call(what->short())
                     + ".";
                 break;
         case 3: str = "Give me a chance with this "
                     + check_call(what->short()) + ", " 
                     + ((who->query_gender() == G_MALE) ? "master" : "lady")
                     + ".";
      }
      command("say " + str);
      command("say You must pay me " + tr_price_arr[j] + " coppers to "
            + "transform it.");
   }
   else
   {
      command("say I never asked for this thing!");
      command("drop " + what->query_name());
      from_arr = exclude_array(from_arr, i, i);
      given_obj_arr = exclude_array(given_obj_arr, i, i);
   }
}

void
catch_tell(string str)
{
   int i, ran;
   string who, what;

   ::catch_tell(str);

   if (str && sscanf(str, "%s says: %s\n", who, what) == 2)
   {
      what = lower_case(what);

      if (what == "give it back" || what == "give it back." 
       || what == "give it back!" || what == "give it back, please.")
      {
         if ((i = member_array(lower_case(who), from_arr)) >= 0)
         {
            ran = random(3);
            switch (ran)
            {
               case 0: command("say Ah, okay.");
                       break;
               case 1: command("say Well, it is yours.");
                       break;
               case 2: command("say Sure, if you wish it back.");
            }
            command("give " + given_obj_arr[i]->query_name()
                  + " to " + lower_case(who));
            from_arr = exclude_array(from_arr, i, i);
            given_obj_arr = exclude_array(given_obj_arr, i, i);
         }
         else
         {
            ran = random(3);
            switch (ran)
            {
               case 0: command("say Uh, I have nothing of you...");
                       break;
               case 1: command("say Now if I had something that belonged to "
                             + "you...");
                       break;
               case 2: command("say Give what back?");
            }
         }
      }
   }
}

/********************************************************************
 *
 *  Some trigger feelings to make the alchemist more vivid.
 *  The feelings are split in two parts: the part that is called by
 *  a trigger function, and a part that is called after a random time.
 *  This is done to get a feeling of reality in the game.
 */
int
react_sigh(string who, string dummy) {
   if (who) {
      who = lower_case(who);
      call_out("return_sigh", 3, who);
      return 1;
   }
}

void
return_sigh(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say Why are you depressed, " + obj->query_nonmet_name()
               + "?");
      if (ran == 1)
         command("comfort " + who);
      if (ran == 2)
         command("say Is life tough for you, " + obj->query_nonmet_name()
               + "?");
   }
}

int
react_smile(string who, string dummy) {
   if (who) {
      who = lower_case(who);
      call_out("return_smile", 3, who);
      return 1;
   }
}

void
return_smile(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say Life is great, isn't it, " + obj->query_nonmet_name()
               + "?");
      if (ran == 1)
         command("smile at " + who);
      if (ran == 2)
         command("say It is great to see you smiling, " + obj->query_nonmet_name()
               + ".");
   }
}

int
react_smirk(string who) {
   if (who) {
      who = lower_case(who);
      call_out("return_smirk", 3, who);
      return 1;
   }
}

void
return_smirk(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say I sense irony, " + obj->query_nonmet_name()
               + "...");
      if (ran == 1)
         command("smirk");
      if (ran == 2)
         command("grin at " + who);
   }
}

int
react_grin(string who, string dummy) {
   if (who) {
      who = lower_case(who);
      call_out("return_grin", 3, who);
      return 1;
   }
}

void
return_grin(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say What cunning plan have you forged, "
                     + ((obj->query_gender() == G_MALE) ? "master" : "lady")
               + "?");
      if (ran == 1)
         command("grin");
      if (ran == 2)
         command("say Get that grin off your face, "+ obj->query_race() + ".");
   }
}

int
react_introduce(string who, string dummy) {
   if (who) {
      who = lower_case(who);
      call_out("return_introduce", 3, who);
      return 1;
   }
}

void
return_introduce(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
      {
         command("say Nice to meet you, " + obj->query_name() + ".");
         command("bow to " + who);
      }
      if (ran == 1)
      {
         command("bow to " + who);
      }
      if (ran == 2)
      {
         command("say Be welcome, " + obj->query_race() + ".");
         command("bow to " + who);
      }
   }
}

int
react_nod(string who, string dummy) {
   if (who) {
      who = lower_case(who);
      call_out("return_nod", 3, who);
      return 1;
   }
}

void
return_nod(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say I'm glad you agree, "
               + ((obj->query_gender() == G_MALE) ? "master" : "lady") + ".");
      if (ran == 1)
         command("pat " + who);
      if (ran == 2)
         command("say Indeed, " + obj->query_race() + ".");
   }
}

int
react_shake(string who, string dummy1, string dummy2) {
   if (who) {
      who = lower_case(who);
      call_out("return_shake", 3, who);
      return 1;
   }
}

void
return_shake(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say So you disagree, " + obj->query_race() + "?");
      if (ran == 1)
         command("say I agree with you, "
               + ((obj->query_gender() == G_MALE) ? "master" : "lady") + ".");
      if (ran == 2)
         command("say Why do " + LANG_PWORD(obj->query_race())
               + " always disagree?");
   }
}

int
react_laugh(string who, string dummy1) {
   if (who) {
      who = lower_case(who);
      call_out("return_laugh", 3, who);
      return 1;
   }
}

void
return_laugh(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say Very funny indeed...");
      if (ran == 1)
         command("laugh");
      if (ran == 2)
         command("giggle");
   }
}

int
react_growl(string who, string dummy1) {
   if (who) {
      who = lower_case(who);
      call_out("return_growl", 3, who);
      return 1;
   }
}

void
return_growl(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say You frighten me with your growling, " + obj->query_race()
               + "...");
      if (ran == 1)
         command("say Why so hostile, " + obj->query_race() + "?");
      if (ran == 2)
         command("frown");
   }
}

int
react_cackle(string who, string dummy1, string dummy2) {
   if (who) {
      who = lower_case(who);
      call_out("return_cackle", 3, who);
      return 1;
   }
}

void
return_cackle(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say You sound like a duck, " + obj->query_race() + ".");
      if (ran == 1)
         command("say " + capitalize(LANG_PWORD(obj->query_race()))
               + " cackle very often.");
      if (ran == 2)
         command("giggle");
   }
}

int
react_shrug(string who, string dummy1) {
   if (who) {
      who = lower_case(who);
      call_out("return_shrug", 3, who);
      return 1;
   }
}

void
return_shrug(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say Is there anything you " + LANG_PWORD(obj->query_race())
               + " do know?");
      if (ran == 1)
         command("say Don't look at me... I don't know either!");
      if (ran == 2)
         command("shrug");
   }
}

int
react_bow(string who, string dummy1) {
   if (who) {
      who = lower_case(who);
      call_out("return_bow", 3, who);
      return 1;
   }
}

void
return_bow(string who) {
   command("bow " + who);
}

int
react_wave(string who, string dummy1) {
   if (who) {
      who = lower_case(who);
      call_out("return_wave", 3, who);
      return 1;
   }
}

void
return_wave(string who) {
   command("wave");
}

int
react_frown(string who, string dummy1) {
   if (who) {
      who = lower_case(who);
      call_out("return_frown", 3, who);
      return 1;
   }
}

void
return_frown(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say Is something wrong, " + obj->query_race());
      if (ran == 1)
         command("say It wasn't my fault!");
      if (ran == 2)
         command("say I had nothing to do with it, I assure you!");
   }
}

int
react_spit(string who, string str) {
   string where;
   if (who) {
      who = lower_case(who);
      if (str[strlen(str)-2] == '.')
         str = str[0..strlen(str)-3];
      else
         str = str[0..strlen(str)-2];

      call_out("return_spit", 3, ({who,str}));
      return 1;
   }
}

void
return_spit(string *arr) {
   string who, where;
   object obj;
   int ran;

   who = arr[0];
   where = arr[1];
   if (obj = present(who, environment())) {
      ran = random(4);
      if (ran == 0)
         command("say Damn " + LANG_PWORD(obj->query_race()) + "!");
      if (ran == 1)
         command("say Hey! Don't do that! Don't spit in here!");
      if (ran == 2)
         command("say Hey! Don't spit " + where + "!");
      if (ran == 3)
         command("say " + capitalize(LANG_PWORD(obj->query_race()))
               + " are such rude people!");
   }
}

int
react_giggle(string who, string dummy1) {
   if (who) {
      who = lower_case(who);
      call_out("return_giggle", 3, who);
      return 1;
   }
}

return_giggle(string who) {
   object obj;
   int ran;

   if (obj = present(who, environment())) {
      ran = random(3);
      if (ran == 0)
         command("say Funny, eh " + obj->query_race() + "?");
      if (ran == 1)
         command("say Ah, " + LANG_PWORD(obj->query_race())
               + " are such merry people.");
      if (ran == 2)
         command("giggle");
   }
}


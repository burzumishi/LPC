/*
 * An excellent example on how to use triggers. Tricky made an alchmeist
 * and I copied the triggering part from it.
 *
 * Yes, I just stold it right there Tricky, /Nick
 *
 */

inherit "/std/monster";

#include "/sys/macros.h"
#include "/sys/ss_types.h"
#include "/sys/stdproperties.h"

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
}

string
my_long()
{
   string str;

   str = "You are looking at the local alchemist, who is known to "
       + "possess the power to transform certain objects into other objects. "
       + "Simply give him an object, so he can judge it. If you want him "
       + "to transform something, pay him, else say 'give it back'.";

   return break_string(str, 70);
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


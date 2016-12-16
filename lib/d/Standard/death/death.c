/* death.c */

#pragma strict_types

inherit "/std/monster";

#include <stdproperties.h>

/*
 * Prototypes
 */
int take_it(string str);

/*
 * Function name: init_living
 * Description:   Init Death
 */
void
init_living() 
{
    ::init_living();
    add_action(take_it, "take");
}

/*
 * Function name: reset
 * Description:   Reset Death
 */
void
create_monster() 
{
    set_name("figure");
    set_name("moot");
    set_name("death");
    set_race_name("immortal");
    set_adj("shadowlike");
    set_short("shadowlike figure, clad in black");
    add_prop(LIVE_I_ALWAYSKNOWN, 1);
}

/*
 * Function name: long
 * Description:   Long description
 */
varargs mixed
long(string str, object for_obj)
{
    
    if (str == "death" || str == "moot")
    {
	return "Death seems to have taken Jane Fonda's exercise and diet " +
	       "program much too seriously. A clear case of Anorexia " +
	       "Neurosa. Except for a wicked looking scythe he's dressed "+
	       "in a black hooded robe that suits him admireably well. " +
	       "There is something about his eyes as well, or maybe the " +
	       "lack of eyes, that you feel you had better not investigate " +
	       "too closely.\n";
    }
    
    if (str == "scythe")
    {
	return "An extremly sharpened scythe. It's so sharp that gusts of " +
	       "wind actually try to turn away from the edge rather than " +
	       "be sliced in two by the wicked looking blade. It does " +
	       "strange things with light as well as unlucky photons split " +
	       "into their sub-components when they hit the blade.\n";
    }
    
    if (str == "robe")
    {
	return "A black hooded robe with numerous pockets. It does not seem " +
	       "to fit you very well however. It seems to have been tailored " +
	       "for a very lean customer. VERY lean actually...\n";
    }
    return "";
}

/*
 * Function name: id
 * Description:   Identifies death and his belongings.
 */
int
id(string str)
{
    return str == "death" || str == "moot" || str == "scythe" || str == "robe";
}

/*
 * Function name: take_it
 * Description:   Try to take something from death.
 */
int
take_it(string str)
{
    
    string name;
    int extra;
    
    name = capitalize(this_player()->query_real_name());
    extra = random(900) + 10;
    
    if (str == "scythe" || str == "robe")
    {
	write("You take a firm grip on the " + str + " and try to pull it " +
	      "towards you.\n" +
	      "Death frowns, raps you smartly across your fingers with a " +
	      "bony hand and says:\n" +
	      "STUPID MORTAL. YOU JUST EARNED " + extra + " EXTRA YEARS IN " +
	      "PURGATORY!\n");
	return 1;
    }

}

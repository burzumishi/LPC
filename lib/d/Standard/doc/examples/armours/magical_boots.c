/* 
 * /doc/examples/armour/magical_boots.c
 *
 * A simple example of how to add magical resistance to an armour.
 *
 * This file is almost excessively commented, so as to answer questions
 * before they come up. Don't let the comments intimidate you :)
 *
 *    Lilith 01/29/97
 */

/* We inherit the basic armour file. */
inherit "/std/armour.c";

/* If we want owner to be able to toggle sell-ability. */
inherit "/lib/keep";

#include <stdproperties.h>     /* Where the properties are defined. */
#include <wa_types.h>          /* Where weapon and armour defs are. */
#include <ss_types.h>          /* Where the skill defines are.      */
#include <tasks.h>             /* For using resolve_task in wear()  */
#include <formulas.h>          /* Where the formulas for value, 
                                * weight, volume, etc are located.  */

/* Now, lets create the armour. */
void
create_armour()
{
    /* Describe the armour thoroughly. */
    set_name("boots");
    set_adj("fur-lined");
    add_adj(({"leather", "fur", "lined"}));
    set_short("pair of fur-lined boots");
    set_pshort("pairs of fur-lined boots");
    set_long("These boots are from the hide of a wheezabit, a "+
      "creature which is exceptionally resistant to burning. "+
      "The leather is rather thick and has been treated with "+
      "a hardening agent.\n");

    /* If it is mentioned in the desciption, better add it! */
    add_item(({"fur", "lining", "fur-lining"}), "The fur is mottled "+
      "brown and cream, short, and very thick. It makes the boots "+
      "very comfortable to wear.\n");

    /* The type of armour (where it is worn) */
    set_at(A_FEET);

    /* The class of the armour (see man armour_guide) */
    set_ac(14);

    /* Impale (+1), Slash (+1), Bludgeon (-2) armour modifiers.  */
    set_am(({ 1, 1, -2}));

    /* Default weight/volume for magical armours and weapons is too
     * low (100, I believe) so lets set it to what it would be if the
     * boots were a normal (non-magical) armour.
     */
    add_prop(OBJ_I_WEIGHT, F_WEIGHT_DEFAULT_ARMOUR(14, A_FEET));
    add_prop(OBJ_I_VOLUME, F_WEIGHT_DEFAULT_ARMOUR(14, A_FEET) / 5);

    /* We are adding extra code to the wear and remove functions 
     * to enable magical resistance, so we need to tell it where 
     * to find those functions. In this case, in this object.  
     * (See wear() and remove() below).                          
     */ 
    set_af(this_object());

    /* I'm making it keepable, for the hell of it :)  */
    set_keep(1);

    /* It is magical, so lets make it worth a bit more.  */
    add_prop(OBJ_I_VALUE, F_VALUE_ARMOUR(14) + 100); 

    /* Armours over the legal non-magical ac limit (40) must     
     * be unsellable. These boots are only ac 14, but lets       
     * do it anyway.                                             
     */
    add_prop(OBJ_M_NO_BUY, 
        "The boots are much too valuable to sell at any price!");

    /* This armour is magical, so we have to add this property.  */
    add_prop(OBJ_I_IS_MAGIC_ARMOUR, 1);

    /* The 'form' of the magic and the degree of its expression. */
    add_prop(MAGIC_AM_MAGIC, ({ 40, "enchantment" }));

    /* This property makes it possible for mortal identification *
     * spells to get more revealing information on the armour.   */
    add_prop(MAGIC_AM_ID_INFO, ({
        "These boots are enchanted. ", 10,          
        "They have the virtues of the creature from whose hide "+
        "they were made. ", 30,
        "They are resistant to damage by fire and acid.\n", 50}));

    /* This property gives detailed information to wizards */
    add_prop(OBJ_S_WIZINFO,
        "These boots are enchanted. They add MAGIC_I_RES_FIRE to "+
        "20, and MAGIC_I_RES_ACID to 15.\n");
}

/*
 * Function name:  query_magic_protection     (see /std/object.c)
 * Description:    This function should return the amount of 
 *                 protection versus an attack of 'prop' on 'obj'.
 * Arguments:      prop - The element property to defend.
 *                 protectee - Magic protection for who or what?
 */
varargs mixed
query_magic_protection(string prop, object protectee = previous_object())
{
    /* Check to see if the boots are still enchanted */
    if (query_prop(OBJ_I_IS_MAGIC_ARMOUR) && (protectee == query_worn()))
    {
        if (prop == MAGIC_I_RES_FIRE)
            return ({ 20, 1}); 
        if (prop == MAGIC_I_RES_ACID)
            return ({ 15, 1});
    } 
    else
    {
        return ::query_magic_protection(prop, protectee);
    }
}

/*
 * Function name: wear          (This is excerpted from /std/armour.c)
 * Description  : This function might be called when someone tries to wear
 *                this armour. To have it called, use set_af().
 * Arguments    : object obj - The armour we want to wear.
 * Returns      : int  0 - The armour can be worn normally.
 *                     1 - The armour can be worn, but print no messages.
 *                    -1 - The armour can't be worn, use default messages.
 *                string - The armour can't be worn, use this message.
 */
mixed
wear(object ob)
{
    /* Check to see if the boots are still enchanted */
    if (!query_prop(OBJ_I_IS_MAGIC_ARMOUR))
    {
        return 0;
    }

    /* Lets prevent drunkards from wearing these things :) */
    if (wearer->query_intoxicated() > 100)
    {
        return "No matter how hard you try, you cannot seem to put "+
               "your feet in the "+ short() +". Perhaps you've had "+
               "too much to drink?\n";
    }

    /* Lets use some skills to determine if the boots can be worn */
    if (wearer->resolve_task(TASK_DIFFICULT, ({TS_DIS, SS_AWARENESS})) < 0)
    {
        return "Something within the boots resists you, as though "+
               "you haven't the discipline to impose your will upon "+
               "them.\n";
    }

    /* Check to make sure the player has the required mana */
    if (wearer->query_mana() <= 25) 
    {
        return "You are too mentally exhausted to engage the "+ short() +
               "in a battle of wills\n";
    }

    /* The player is sober and has the needed skill/stat combo, so 
     * He gets to wear the boots. 
     */

    /* Lets take mana first */
    wearer->add_mana(-25);    

    /* add_magic_effect() is defined in /std/object.c  It adds    
     * to the wearer the resistance as described in the function  
     * query_magic_protection (above).                            
     */
    wearer->add_magic_effect(this_object());

    /* We'll give the player a message */
    wearer->catch_tell("The fire in your heart is fanned by the " + 
        short(wearer) + ", and you feel as though you could cross a " +
        "river of fire without any harm coming to you.\n");

    /* We return 0 so that the normal wear messages occur.        */ 
    return 0;
}

/*
 * Function name: remove        (This is excerpted from /std/armour.c)
 * Description  : This function might be called when someone tries to 
 *                    remove this armour. To have it called, use set_af().
 * Arguments    : object obj - The armour to remove.
 * Returns      : int  0 - Remove the armour normally.
 *                     1 - Remove the armour, but print no messages.
 *                    -1 - Do not remove the armour, print default message.
 *                string - Do not remove the armour, use this message.
 */
mixed
remove(object ob)
{
    /* The magical resistance should only work when the owner is   
     * wearing the armour, so we have to remove_magic_effect()     
     * when the boots are removed.                                 
     */
    wearer->remove_magic_effect(this_object());

    /* Just a little something to let them know they removed 
     * something special.
     */
    if (query_prop(OBJ_I_IS_MAGIC_ARMOUR))
    {
        wearer->catch_tell("The fire in your blood is quenched.\n");
    }

    /* Again, returning 0 so the normal remove message is given.   */ 
    return 0;
}


public int
disenchant_object(object disenchanter)
{
    /* Disenchant the boots */

    /* Reset magical properties */
    remove_prop(OBJ_I_IS_MAGIC_ARMOUR);
    remove_prop(MAGIC_AM_MAGIC);
    add_prop(MAGIC_AM_ID_INFO, ({ 
        "The boots once possessed some magical power, but it is " +
        "now gone.\n", 50 }));

    /* It isn't worth as much without the magic */
    add_prop(OBJ_I_VALUE, F_VALUE_ARMOUR(14)); 

    /* No reason to restrict purchase of the boots anymore */
    remove_prop(OBJ_M_NO_BUY);

    /* Remove the magic effect if the item is worn */
    if (query_worn())
    {
        wearer->remove_magic_effect(this_object());
    }

    /* Give a message indicating that the boots have been disenchanted */
    tell_room(environment(this_object()), "The fur lining of the " +
        "fur-lined leather boots suddenly greys, and large clumps " +
        "fall off!\n");

    /* Reset the description of the fur lining */
    remove_item("fur");
    add_item(({"fur", "lining", "fur-lining"}), "The fur is mottled "+
        "brown and cream with ugly splotches of grey;  large clumps " +
        "of it have fallen out.\n");

    /* Return 1, indicating that the boots were disenchanted */
    return 1;
}

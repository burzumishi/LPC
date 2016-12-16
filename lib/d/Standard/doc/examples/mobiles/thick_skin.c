/*
 * thick_skin.c
 *
 * This is a simple example of an npc that has "built-in" armour
 * and enhanced unarmed attacks.
 */

inherit "/std/monster";

create_monster()
{
    int i;

    set_name("goblin");
    set_long("It's very small but it looks back on you and shows its teeth.\n");
    set_race_name("goblin");
    set_adj("small");

    set_stats(({ 10, 11, 10, 5, 5, 6 }));

    /* Give the npc 5 points of built-in armour, which is added to
     * any other armour that it might wear.  Note that this can also
     * be set with a three-element array to give different values for
     * impale/slash/bludgeon.
     */
    set_all_hitloc_unarmed(5); /* also an array like ({ 5, -3, -2 }) */


    /* Give the npc enhanced unarmed attacks, having a hit and pen of 15 */
    set_all_attack_unarmed(15, 15);
}

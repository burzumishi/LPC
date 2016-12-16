/*
    /doc/examples/mobiles/tiger.c

    JnA 920111

    A sample creature 

       This creature uses no tools for fighting. 
       It inherits the routines for unarmed combat.

    This is a rather tough beast. You need on average 30 or more in your
    stats to handle it. You also need wc30 weapon with skills to match.

*/

inherit "/std/creature";
inherit "/std/combat/unarmed";   /* This gets us standard unarmed routines */

#include <wa_types.h>
#include <ss_types.h>
#include <macros.h>

/*
 * Define some attack and hitloc id's (only for our own benefit)
 */
#define A_BITE  1
#define A_LCLAW 2
#define A_RCLAW 4

#define H_HEAD 1
#define H_BODY 2

create_creature()
{
    set_name("tiger"); 
    set_race_name("tiger");
    set_short("white tiger");
    set_adj(({"white", "vicious" }));
    set_long("It looks rather vicious!\n");

    /* str, con, dex, int, wis, dis */
    set_stats(({ 90, 30, 80, 20, 5, 75}));

    set_skill(SS_DEFENCE, 30);
    set_skill(SS_SWIM, 80);

    /* Give the tiger some attacks */

    /* Arguments to set_attack_unarmed: 
     *   attack id, hit, pen, damage type, percent usage, description
     */
    set_attack_unarmed(A_BITE,  20, 30, W_IMPALE, 40, "jaws");
    set_attack_unarmed(A_LCLAW, 40, 20, W_SLASH,  30, "left paw");
    set_attack_unarmed(A_RCLAW, 40, 20, W_SLASH,  30, "right paw");
   
    /* Give the tiger some hit locations */

    /* Arguments to set_hitloc_unarmed:
     *   hit location id, ac, percent hit, description
     */
    set_hitloc_unarmed(H_HEAD, ({ 15, 25, 20,}), 20, "head");
    set_hitloc_unarmed(H_BODY, ({ 10, 15, 30,}), 80, "body");
}

/*
 * Here we redefine the special_attack function which is called from
 * within the combat system. If we return 1 then there will be no
 * additional ordinary attack.
 *
 */
int
special_attack(object enemy)
{
    object me;
    mixed *hitresult;
    string how;

    /* Only execute the special attack 1 in 10 rounds */
    if (random(10))
    {
	return 0;  /* Continue with the normal attacks */
    }

    hitresult = enemy->hit_me(20+random(30), W_IMPALE, me, -1);
    switch (hitresult[0])
    {
        case 0:
            how = "unwounded";
            break;
        case 1..10:
            how = "barely wounded";
            break;
        case 11..20:
            how = "wounded";
            break;
        default:
            how = "seriously wounded";
            break;
    }

    enemy->catch_tell(query_The_name(enemy) + " tears into your throat!\n"+
		     "You are " + how + ".\n");
    tell_watcher(QCTNAME(me) + " tears into " + QTNAME(enemy) + "!\n" +
        capitalize(enemy->query_pronoun()) + " is " + how + ".\n", enemy);

    if (enemy->query_hp() <= 0)
    {
	enemy->do_die(this_object());
    }
    
    return 1; /*  Important! Should not have two attacks in a round. */
}

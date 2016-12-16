/*
 * Copyright (c) 1991 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */

/*
  /obj/knife.c

  This is a simple weapon.
 */
#pragma save_binary

inherit "/std/weapon";

#include "/sys/wa_types.h"
#include "/sys/formulas.h"


/*
 * Function name: reset_weapon
 * Description:   Reset the weapon
 * Arguments:	  arg: The reset argument.
 */
nomask
create_weapon() {
{
	  set_name( ({"knife", "short knife"}) );
	  set_short("short knife");
	  set_long("This is a simple-looking short knife.\n");
	  set_hit(5);
	  set_pen(5);
	  set_wt(W_SWORD);
	  set_dt(W_IMPALE);
	  set_hands(W_NONE);
}

/*
 * Function name: 
 * Description:   
 * Arguments:	  
 * Returns:       
 */
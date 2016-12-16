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
create_weapon()
{
	  set_name("greatsword");
	  set_short("greatsword");
	  set_long("It is a huge great greatsword.\n");
	  set_hit(25);
	  set_pen(25);
	  set_wt(W_SWORD);
	  set_dt(W_SLASH);
	  set_hands(W_BOTH);
}

/*
 * Function name: 
 * Description:   
 * Arguments:	  
 * Returns:       
 */
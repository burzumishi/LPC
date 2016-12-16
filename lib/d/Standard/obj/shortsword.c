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
	  set_name("shortsword");
	  set_short("shortsword");
	  set_long("It is a simple looking shortsword.\n");
	  set_hit(15);
	  set_pen(15);
	  set_wt(W_SWORD);
	  set_dt(W_SLASH);
	  set_hands(W_NONE);
}

/*
 * Function name: 
 * Description:   
 * Arguments:	  
 * Returns:       
 */
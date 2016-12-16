/*
 * Copyright (c) 1991 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */

/*
  /obj/shield.c

  This is a simple armour.
 */
#pragma save_binary

inherit "/std/armour";

#include "/sys/wa_types.h"
#include "/sys/formulas.h"


/*
 * Function name: reset_armour
 * Description:   Reset the armour
 * Arguments:	  arg: The reset argument.
 */
create_armour()
{
	  set_name("shield");
	  set_short("small wooden shield");
	  set_long("It is a small but sturdy looking wooden shield.\n");
	  set_ac(F_ARMOUR_CLASS_PROC(4));  /* Armour class. 4% of max */
	  set_at(A_L_ARM); 		   /* Set the armour type. */
}

/*
 * Function name: 
 * Description:   
 * Arguments:	  
 * Returns:       
 */
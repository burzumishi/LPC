/*
 * Copyright (c) 1991 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */

/*
  /obj/plate.c

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
nomask
create_armour()
{
	  set_name("plate");
	  set_short("plate mail");
	  set_long("It is a simple looking plate mail.\n");
	  set_ac(15);
	  set_at(A_BODY);
	  set_am(({ 0, 1, 2, 3}));
}

/*
 * Function name: 
 * Description:   
 * Arguments:	  
 * Returns:       
 */
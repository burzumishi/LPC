/*
 * Copyright (c) 1991 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */

/*
  /obj/glass_case.c

  This is a trasnparent container.
 */
#pragma save_binary

inherit "/std/container";

#include "/sys/formulas.h"
#include "/sys/stdproperties.h"

/*
 * Function name: reset_container
 * Description:   inits the properties
 * Arguments:	  arg: The reset argument.
 */
nomask
create_container()
{
	set_name( ({"glass-case", "case"}) );
	set_short("transparent glass-case");
	set_long("This is a man sized transparent glass-case with silver doors.\n");

	add_prop(CONT_RIGID,1);      	/* A rigid body */
	add_prop(CONT_TRANSP,1);      	/* A transparent container */

	add_prop(CONT_WEIGHT, 1000000);	/* 1000 Kg (heavy thing) */
	add_prop(CONT_VOLUME, 1000000);	/* 1000 l */

	add_prop(CONT_MAX_WEIGHT, 2000000);   /* 2000 Kg (can hold 1000 kg) */
	add_prop(CONT_MAX_VOLUME, 1000000);   /* 1000 l (can hold 1000 l) */

	add_prop(OBJ_VALUE, 1000);	/* 1000 copper coins  */
	
	add_item(({"silver doors", "doors"}), 
	 "This is a pair of doors fitted with shining silver mirrors.\n");
}

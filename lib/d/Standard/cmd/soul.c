/*
 * Copyright (c) 1996 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */

/*
 * /d/Standard/cmd/soul.c
 *
 * This module contains the Standard specific emotions. Currently two sets
 * have been included, namely for hair and eyes. Glykron originally started
 * with a hair shadow for Gelan, Calia. Maniac extended most of the coding,
 * extended it a lot and also coded the eyes. Mercade now merged this into
 * the Standard character creation procedure. October 6 1996.
 */

#pragma no_clone
#pragma save_binary
#pragma strict_types

inherit "/cmd/std/soul_cmd.c";
 
#include <macros.h>

/* **************************************************************************
 * The list of souls to replace this one with.
 */
public string *
replace_soul()
{
    return ({
	MASTER,
	"/d/Standard/cmd/double",
	});
}
 
/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
mapping
query_cmdlist()
{
    return ::query_cmdlist();
}

/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the 
 * same order as in the function name list.
 */

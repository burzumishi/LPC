/*
 * Copyright (c) 1991 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */

/*
 * /d/Standard/cmd/soul_cmd_ghost.c
 *
 * This is the soul command object for ghosts. 
 */

#pragma no_clone
#pragma save_binary
#pragma strict_types

inherit "/d/Standard/cmd/soul";

/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 *
 * This function is outcommented while there are no special commands in it.
 *
mapping
query_cmdlist()
{
    return ::query_cmdlist();
}
 */

/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the 
 * same order as in the function name list.
 * **************************************************************************/

int
giggle(string str)
{
    write("You make a horrible gurgling sound.\n");
    all(" gurgles rather awful.");
    return 1;
}

int
sing(string str)
{
    write("You moan and complain in a very frightful way.\n");
    all(" moans and complain in a very frightful way.\n");
    return 1;
}

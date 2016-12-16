/*
 * money2.h
 *
 * This file defines the local currency units.
 * The only use of this file is that it should be included by /sys/money.h.
 */

/*
 * SIZEOF_MONEY_TYPES - the amount of coin types we have.
 */
#define SIZEOF_MONEY_TYPES (4)

/*
 * Indices to the arrays defined below.
 */
#define MONEY_CC_INDEX (0)
#define MONEY_SC_INDEX (1)
#define MONEY_GC_INDEX (2)
#define MONEY_PC_INDEX (3)

/*
 * MONEY_TYPES  - array with the different types of money.
 * MONEY_VALUES - the values of the coins relative to the lowest denomination.
 * MONEY_WEIGHT - the weight of the types of money per coin.
 * MONEY_VOLUME - the volume of the types of money per coin.
 */
#define MONEY_TYPES  ({ "copper", "silver", "gold", "platinum" })
#define MONEY_SHORT  ({ "cc"    , "sc"    , "gc"  , "pc" })
#define MONEY_VALUES ({ 1       ,  10     , 100   , 1000 })
#define MONEY_WEIGHT ({ 15      ,  20     , 25    , 30  })
#define MONEY_VOLUME ({ 2       ,  2      , 2     , 2   })

/*
 * These macros return the objectpointer to a heap of coins of the type of
 * choice. I am sure you'll know the types from the macros. The argument
 * 'num' is the number of coins that should be in the heap.
 */
#define MONEY_MAKE_CC(num)    MONEY_MAKE(num, "copper")
#define MONEY_MAKE_SC(num)    MONEY_MAKE(num, "silver")
#define MONEY_MAKE_GC(num)    MONEY_MAKE(num, "gold")
#define MONEY_MAKE_PC(num)    MONEY_MAKE(num, "platinum")

/*
 * These macros will try to move a certain amount of coins of the type
 * in the macro from the object 'from' to the object 'to'. The amount of
 * coins to be moved is stored in 'num'. I am sure that you will know the
 * types from the macro-names. If 'from' == 0, coins will be created.
 *
 * This macro has three return levels:
 *  -1 : not enough coins of the specified type in 'from'.
 *   0 : the move was succesful!
 * >=1 : an error from the function move().
 */
#define MONEY_MOVE_CC(num, from, to) MONEY_MOVE(num, "copper",   from, to)
#define MONEY_MOVE_SC(num, from, to) MONEY_MOVE(num, "silver",   from, to)
#define MONEY_MOVE_GC(num, from, to) MONEY_MOVE(num, "gold",     from, to)
#define MONEY_MOVE_PC(num, from, to) MONEY_MOVE(num, "platinum", from, to)

/*
 * MONEY_LOG_LIMIT - the minimal amount of coins of a type that needs to
 *                   be moved in order to be logged.
 */
#define MONEY_LOG_LIMIT ([ "copper" : 3000, "silver" : 1200, "gold" : 160, "platinum" : 35 ])

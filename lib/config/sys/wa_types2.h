/*
 * wa_types2.h
 *
 * This file defines the available weapons are locally configured. 
 * The only use of this file is that it should be included
 * by /sys/wa_types.h.
 */

/* Weapon types */

#define W_FIRST     0		/* The first weapon index */

#define W_SWORD     W_FIRST + 0 
#define W_POLEARM   W_FIRST + 1
#define W_AXE	    W_FIRST + 2
#define W_KNIFE     W_FIRST + 3
#define W_CLUB	    W_FIRST + 4
#define W_MISSILE   W_FIRST + 5
#define W_JAVELIN   W_FIRST + 6
#define W_NO_T      7		/* The number of weapons defined */

/*
 * Drawbacks are arrange for each weapon type ({ dull, corr, break })
 * and types are ({ sword, polearm, axe, knife, club, missile, javelin })
 */
#define W_DRAWBACKS ({ ({ 5, 10, 5 }), ({ 8, 11, 11 }), ({ 4, 9, 7 }), \
                        ({ 5, 5, 9 }), ({ 4, 1, 5 }), ({ 2, 2, 13 }), \
                        ({ 7, 3, 11 }) })

#define W_WEIGHTS_OFFSET ({ 1300, 3200, 1800,  180, 1200,  300, 1700 })
#define W_WEIGHTS_FACTOR ({   35,   55,   46,   16,   28,   18,   36 })

#define W_NAMES ({ "sword", "polearm", "axe", "knife", "club", "missile weapon", "javelin" })

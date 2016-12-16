/*
 * /d/Standard/login/login.h
 *    
 * Some constants relevant to the login procedure.
 */

/*
 * PATH
 * 
 * The path in which the login rooms reside
 */
#define PATH "/d/Standard/login/"

/*
 * HELP
 *
 * The path which contains all the login help files
 */
#define HELP (PATH + "help/")

/*
 * ATTRIBS
 *
 * The path to the directory containing all the player attributes
 *
 */
#define ATTRIBS (PATH + "attributes/")

/*
 * Path macros to various rooms, if wanted.
 */
#define LOGIN_BODIES         (PATH + "bodies")
#define LOGIN_FEATURES       (PATH + "features")
#define LOGIN_HEIGHT         (PATH + "mirrors_1")
#define LOGIN_WEIGHT         (PATH + "mirrors_2")
#define LOGIN_SKILLS         (PATH + "skills")

/* 
 * LOGIN_FILE_NEW_PLAYER_INFO
 *
 * This is the names of the files that are written to new users logging in.
 */
#define LOGIN_FILE_NEW_PLAYER_INFO "/d/Standard/doc/login/NEW_PLAYER_INFO"

/*
 * RACEMAP
 *
 * This mapping holds the names of the allowed player races in the game and
 * which player file to use for new characters of a given race.
 */
#define RACEMAP ([ \
		"human"  : "/d/Standard/race/human_std",   \
		"vogon" : "/d/Standard/race/vogon_std",  \
		])
/*
 * RACEATTR
 *
 * This mapping holds the standard attributes for each race. The attributes
 * are: 
 *      standard height         (cm)
 *      standard weight         (kg)
 *      standard opinion        (0-100)
 *      standard appereance     (0-100)
 *      standard volume         (dm^3)
 *      standard fatness        (g/cm)
 */
#define RACEATTR ([ \
		"human"  : ({180, 70, 50, 50, 70, 389 }), \
		"vogon" : ({100, 45, 90, 90, 45, 450 }), \
		])

/*
 * RACESTATMOD
 *
 * This mapping holds the standard modifiers of each stat, i.e. a dwarf
 * should have it easier to raise con than other races, but get a harder
 * time raising its int.
 *
 * Mapping is: race:  str, dex, con, int, wis, dis, race, occ, lay, craft
 */
#define RACESTATMOD ([ \
		"human"  : ({ 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 }), \
                "vogon"  : ({ 15, 5, 15, 8, 9, 13, 10, 10, 10, 10 }),    \
		])

/*
 * when m_indexex work on constants: m_indexes(RACEMAP)
 */
#define RACES ({ "human", "vogon"})

#define RACES_SHORT ([ \
                      "human" : "hum", \
		      "vogon" : "vog", \
		    ])

/*
 * RACESTART
 *
 * This mapping holds the files of the starting locations for each race.
 */
#define RACESTART ([ \
		    "human"  : "/d/Standard/start/church",   \
		    "vogon"  : "/d/Standard/start/church",   \
	          ])

/*
 * RACEPOST
 *
 * This mapping holds the files of the post office locations for each race.
 */
#define RACEPOST ([ \
		    "human"  : "/d/Standard/start/mailroom",    \
		    "vogon"  : "/d/Standard/start/mailroom",    \
	          ])

/*
 * RACESTAT
 *
 * This mapping holds the stats that each race has on start
 */
#define RACESTAT ([            /* str, dex, con, int, wis, dis */      \
		    "human"  : ({   9,  9,   9,   9,   9,   9 }),      \
	            "vogon"  : ({  10,   20,   30,  1,  1,  1 }),      \
	          ])


/*
 * RACEMISCCMD
 *
 * This mapping holds the files of the souls that should be used as
 * misc command soul for each race
 */
#define RACEMISCCMD ([ \
		    "human"  : "/d/Standard/cmd/misc_cmd_human",  \
		    "vogon"  : "/d/Standard/cmd/misc_cmd_vogon", \
	          ])

/*
 * RACESOULCMD
 *
 * This mapping holds the files of the souls that should be used as
 * misc command soul for each race
 */
#define RACESOULCMD ([ \
		    "human"  : "/d/Standard/cmd/soul_cmd_human",  \
		    "vogon"  : "/d/Standard/cmd/soul_cmd_vogon",  \
	          ])
/*
 * RACESOUND
 *
 * What sound do subindex-race hear when mainindex race speaks
 */
#define RACESOUND2 ([						\
		    "human" : ([ 				\
				"human" : "says",		\
				"vogon" : "mumbles",		\
				]),				\
		    "vogon" : ([				\
				"human" : "whispers",		\
				"vogon" : "says",		\
				]),				\
		    ])

#define RACESOUND ([						\
		    "human" : ([ 				\
				"human" : "says",		\
				"vogon"	: "whispers",		\
				]),				\
		    "vogon" : ([				\
				"human" : "mumbles",		\
				"vogon": "says",		\
				 ]),				\
		    ])
#define LINKDEATH_MESSAGE ([ \
       "human"  : " passes into limbo.\n",                  \
       "vogon"  : " is cast into the void.\n" ])

#define REVIVE_MESSAGE ([ \
       "human"  : " returns from limbo.",                   \
       "vogon"  : " returns from the void." ])
 
/*
 * HEIGHTDESC
 */
#define HEIGHTDESC ({"extremely short", "very short", "short", \
    "of normal length", "tall", "very tall", "extremely tall" })

/*
 * WIDTHDESC
 */
#define WIDTHDESC ({"very skinny", "skinny", "lean", "of normal width", \
    "plump", "fat", "very fat" })

/*
 * SPREAD_PROC
 */
#define SPREAD_PROC ({ 70, 80, 90, 100, 110, 120, 130 })

/*
 * LOGIN_NO_NEW - Log file for failed creation attempts.
 */
#define LOGIN_NO_NEW "/d/Standard/login/no_new_players"

/*
 * This is the current state of this ghost (uses set_ghost() / query_ghost())
 */
#define GP_BODY		1
#define GP_MANGLE	2
#define GP_FEATURES	4
#define GP_SKILLS       8
#define GP_NEW          (GP_BODY | GP_MANGLE | GP_FEATURES | GP_SKILLS)
#define GP_DEAD         32

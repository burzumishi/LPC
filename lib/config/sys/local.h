/*
  /local.h

  This file contains all world specific information. To configure your
  own world and set up your own mud, make changes to this file.

  This file is configured for Standard.

*/
#ifndef CONFIG_DEFINED
#define CONFIG_DEFINED

/*
 *
 * MUD_NAME
 *
 * The name of the mud. This will be defined in all files compiled.
 * It is either a single string or a mapping of one names for each
 * port it might be running on. '0' in the mapping means default name.
 *
 */
#define MUD_NAME ([ 0: "Standard_test", 4000: "Standard" ])

/*
 * DEFAULT_WIZARDS
 *
 * This array contains the default setup of wizards of this LPmud.
#define DEFAULT_WIZARDS ([ \
    "postmaster":({7,35,"root","Standard","root",}), \
    "wiz":({7,35,"root","Wiz","root",}), \
    "root":({8,50,"root","Wiz","root"}), ])
*/
#define DEFAULT_WIZARDS ([ \
    "postmaster":({7,35,"root","Standard","root",0,"",({}),}), \
    "root":({9,50,"root","Wiz","root",0,"",({}),}), ])

/*
 * DEFAULT_DOMAINS
 *
 * This array contains the default setup of domains of this LPmud.
#define DEFAULT_DOMAINS ([ \
    "Standard":({0,"gen","postmaster","",({"postmaster",}),"",1,0,0,0 }), \
    "Wiz":({1,"wiz","wiz","",({"root",}),"",10,0,0,0}),])
*/
#define DEFAULT_DOMAINS ([ \
    "Standard":({0,"std","postmaster","",({"postmaster",}),"",1,0,0,0 }), \
    "Wiz":({1,"wiz","wiz","",({"root",}),"",10,0,0,0}),])

#define BASE_DOMAIN "Standard"

/*
 * WIZARD_DOMAIN
 *
 * This is the name of the special domain where all 'lonely' wizards are
 * placed. This domain has no domainlord and no common, open dirs
 * 
 */
#define WIZARD_DOMAIN "Wiz"

/*
 * OPEN_LOG_DIR
 *
 * This is the location of the open logs.
 */
#define OPEN_LOG_DIR "/d/Standard/log"

/*
 * DISCARD_DOMAIN_DIR
 *
 * This is the name of the directory where all the removed domains are
 * put awaiting further developments.
 */
#define DISCARD_DOMAIN_DIR "/d/Discarded"

/*
 * DEFAULT_PLAYER
 *
 * This is the default player file if nothing else is defined
 */
#define DEFAULT_PLAYER "/d/Standard/race/human_std"

/*
 * DEFAULT_START
 *
 * This is the default starting location if nothing else is defined
 */
#define DEFAULT_START "/d/Standard/start/human/town/church"

/*
 * STARTING PLACES
 *
 * This array contains all the places that are acknowledged starting places
 * by default. 
 *
 */
#define STARTING_PLACES ({ "/d/Standard/start/human/town/church", \
			   "/d/Standard/start/goblin/caverns/quarters", \
   			   "/d/Standard/start/elf/room/begin", \
			   "/d/Standard/start/hobbit/v/church", \
			   "/d/Standard/start/dwarf/lvl1/temple"})
#define TEMP_STARTING_PLACES ({ })

/*
 * WIZ_ROOM
 *
 * This is the path to the cozy room to which we set new wizards starting
 * location.
 */
#define WIZ_ROOM "/d/Standard/wiz/green"

/*
 * ADMIN_HOME
 *
 * This is the administrators workroom. Has some special functions in the game.
 */
#define ADMIN_HOME "/d/Standard/wiz/admin"

/*
 * WIZ_APPRENTICE_SCROLL 
 *
 * This is the path to the scroll that gets loaded and given to new wizards.
 */
#define WIZ_APPRENTICE_SCROLL "/d/Standard/obj/init_scroll"
#define APPRENTICE_SCROLL_FILE "/d/Standard/obj/init_scroll"

/* 
 * LOGIN_NEW_PLAYER
 *
 * This is the player object that manages all entering of new players
 */
#define LOGIN_NEW_PLAYER "/d/Standard/login/ghost_player"
#define LOGIN_TEST_PLAYER "/d/Standard/login/test/ghost_player"

/*
 * WIZ_MAKER
 *
 * This is a list of the paths to objects that may create apprentices
 */
#define WIZ_MAKER ({ "/d/Standard/start/human/town/adv_guild" })

/* 
 * LOGIN_FILE_WELCOME  - general welcome before password.
 * LOGIN_FILE_NEWS     - special news message after password.
 * LOGIN_FILE_RUNLEVEL - message about using a runlevel.
 * LOGIN_FILE_NEWCHAR  - message to new characters before confirmation.
 *
 * This is the names of the files that are written to user logging in.
 */
#define LOGIN_FILE_WELCOME  "/d/Standard/doc/login/WELCOME"
#define LOGIN_FILE_NEWS     "/d/Standard/doc/login/NEWS"
#define LOGIN_FILE_RUNLEVEL "/d/Standard/doc/login/RUNLEVEL"
#define LOGIN_FILE_NEWCHAR  "/d/Standard/doc/login/NEWCHAR"

/* 
 * MAIL_INFO_OBJ
 * SPECIAL_INFO_OBJ
 * 
 * These are the objects that the finger_player object uses to find out
 * interesting stuff about the player
 */
#define MAIL_INFO_OBJ "/d/Standard/std/mail_stuff"
#define SPECIAL_INFO_OBJ "/d/Standard/std/special_stuff"

/* 
 * DEFAULT_DEATH tells what object should be loaded when a player dies
 */

#define DEFAULT_DEATH "/d/Standard/room/death/death_mark"

/*
 * DOMAIN_LINK
 * WIZARD_LINK
 *
 * Filenames trailing /d/Domain_name/ and /d/<WIZARD_DOMAIN>/wizard_name/
 * to preload on startup
 */
#define DOMAIN_LINK "domain_link"
#define WIZARD_LINK "wizard_link"

/*
 * GAMEINFO_LOGIN
 * GAMEINFO_INFO
 *
 * Information about the game in login procedure.
 */
#define GAMEINFO_LOGIN	"gameinfo"
#define GAMEINFO_INFO	"/d/Standard/doc/login/GAMEINFO_INFO"

/*
 * ALWAYS_APPLY - Define this if you always want players to apply before
 *                they can create a character. Undefine it if you want to
 *                have automatic character creation.
 * APPLICATION_LOGIN
 * APPLICATION_INFO
 * APPLICATION_BOARD_LOC
 *
 * Banned site login procedure
 */
#undef ALWAYS_APPLY
#define APPLICATION_LOGIN	"application"
#define APPLICATION_INFO	"/d/Standard/doc/login/APPLICATION_INFO"
#define APPLICATION_BOARD_LOC	"/d/Standard/wiz/admin"

/*
 * NO_GUEST_LOGIN
 *
 * Define this if you do not want a shared "guest" character to log in
 * without using a password. Undefine this and "guest" will be able to
 * log in.
 */
#define NO_GUEST_LOGIN

/*
 * COMBAT_FILE
 * 
 * This file is cloned to manage a fight for a living object.
 */
#define COMBAT_FILE "/std/combat/cbase"

/*
 * MAX_PLAY
 *
 * This is the maximum amount of players that may be logged on at
 * one time.
 */
#define MAX_PLAY 200

/*
 * SMALL_MEMORY_LIMIT
 * LARGE_MEMORY_LIMIT
 *
 * This is the maximum memory allowed by the master object before itself
 * starts Armageddon. The memory consumption is fetched from the
 * debug("malloc") efun as the number before (a). The use of SMALL/LARGE
 * is determined by external calls to the gamedriver.
 */

/*
   No need for us to monitor this any more, I'm setting this to a very
   high value, in effect disabling the monitoring functionality. /Mrpr
*/
/* 1.0 GB = 1073741824 */
#define SMALL_MEMORY_LIMIT (1073741824)
#define LARGE_MEMORY_LIMIT (1073741824)

/*
 * SWAP_MEM_MIN
 * SWAP_MEM_MAX
 * SWAP_TIME_MIN
 * SWAP_TIME_MAX
 *
 * These are swapping control parameters and control when we do swapping.
 * The swapping will start when we have used SWAP_MEM_MIN bytes of memory,
 * when we reach SWAP_MEM_MAX we will swap as much as possible. We will
 * not swap objects unless it is SWAP_TIME_MIN seconds old and we will
 * swap all objects older than SWAP_TIME_MAX seconds.
 */
#define SWAP_MEM_MIN    350000000
#define SWAP_MEM_MAX    450000000
#define SWAP_TIME_MIN   180
#define SWAP_TIME_MAX   900

/*
 * ARMAGEDDON
 *
 * This is the object to call "slow_shut" in when the GD tells master
 * that the memory is getting low.
 */
#define ARMAGEDDON "/d/Standard/obj/armageddon"

/*
 * NPC_SOULS
 *
 * The array of sould that all mobiles should have
 */
#define NPC_SOULS ({ "/cmd/std/soul_cmd", "/cmd/std/misc_cmd" })

/*
 * MAX_IDLE_TIME
 *
 * The maximum time you are guaranteed to be able to stay idle before
 * being logged out. The amount is counted in seconds.
 */
#define MAX_IDLE_TIME 3600

/*
 * NO_WIZARD_IDLE_CHECK
 *
 * If defined, wizards will not be checked for idleness. Undefine to
 * make wizards checked, too. The total idle time in seconds is the
 * rank * MAX_IDLE_TIME.
 */
#undef NO_WIZARD_IDLE_CHECK
#define NO_WIZARD_IDLE_CHECK

/*
 * FOB_KEEP_CHANGE_TIME
 *
 * Define this when you also want to keep track in the KEEPERSAVE of the time
 * the domain or rank/level of the wizard was changed. When undefined, this
 * infromation will not be kept. Naturally the LEVEL log will still contain
 * that information, should that be defined.
 */
#undef FOB_KEEP_CHANGE_TIME

/*
 * USE_WIZ_LEVELS
 *
 * Define this when you want to allow individual wizard levels within a rank.
 * Ranks are wizard/mage/arch, levels are 15, 16, 17.
 */
#undef USE_WIZ_LEVELS

/*
 * FORCE_PASSWORD_CHANGE <seconds>
 *
 * Define this if you want to force players to alter their password at least
 * every once in a while. This way, it will be less likely that a password is
 * found. When undefined, passwords have no expiration.
 *
 * Seconds is the number of seconds between two forced password changes.
 */
#undef FORCE_PASSWORD_CHANGE

/*
 * MET_ACTIVE
 *
 * If defined all livings will have an 'unmet' name and a normal name.
 */
#define MET_ACTIVE

/*
 * DAY_AND_NIGHT
 *
 * If DAY_AND_NIGHT is active there will be a day night cyclus from 10 pm
 * to 5 am.
 */
#undef DAY_AND_NIGHT

/*
 * STATUE_WHEN_LINKDEAD
 *
 * Show players as statues when they are linkdead. 
 * 
 */
#define STATUE_WHEN_LINKDEAD
#define OWN_STATUE "/d/Standard/obj/statue"
#define STATUE_DESC "statue of"
#define STATUE_TURNS_INTO "turns into stone"
#define STATUE_TURNS_ALIVE "turns alive again"

/*
 * EDITOR_SAVE_OBJECT
 *
 * Will be called from the editor if a player goes linkdead while editing
 * something, a note, mail, whatever. Allows them to restore the message
 * from the editor after they re-link.
 */
#define EDITOR_SAVE_OBJECT "/w/postmaster/editor_save"

/*
 * UDP_MANAGER
 *
 * The object that manages udp messages
 */
#undef UDP_MANAGER "/d/Standard/obj/udp"

/* 
 * DECAY_XP
 *
 * The part of xp decayed each 15 minutes. (xp -= xp / DECAY_XP)
 * If this is 0 then no decay occurs.
 */
#define DECAY_XP 250

/*
 * DOMAIN_RANKWEIGHT_FORMULA
 *
 * This is the formula for calculating the weight factor in the domain
 * ranking list. q and c are quest and combat xp given to mortals by the
 * domain. Normally it is good to give quest xp and bad to give combat xp.
 * This is so because we want to promote the making of quests.
 */
#define DOMAIN_RANKWEIGHT_FORMULA(q, c)  (100 + (q / 25) + ((-c) / 10000))

/*
 * REGULAR_REBOOT
 *
 * If defined, the hour during which the game will be rebooted.
 */
#undef REGULAR_REBOOT 9

/*
 * REGULAR_UPTIME
 * UPTIME_VARIATION
 *
 * Define this to specify the amount of hours uptime after which a reboot
 * shall follow. The variation is the total time window in hours on this
 * maximum uptime. I.e. an uptime of 10 hours and var of 2 means 9 - 11.
 */
#define REGULAR_UPTIME 192
#define UPTIME_VARIATION 2

/* 
 * NO_ALIGN_TITLE
 *
 * Defined if alignment title is not used
 */
#define NO_ALIGN_TITLE

#undef NO_SKILL_DECAY

/*
 * CYCLIC_LOG_SIZE
 *
 * This is a mapping containing the maximum file size for logs for a given
 * euid. Normally the maximum file size for a log is given with the euid '0',
 * however, if the name of the euid appears in this mapping, the owner of the
 * euid has the option of allowing the size grow to the given value before
 * cycling is done. 
 * If the size is less than 0, the log will be allowed to grow indefinately.
 */
#define CYCLIC_LOG_SIZE ([ 0 : 20000, "root" : -1, "Standard" : 100000 ])

/*
 * PRELOAD_FIRST
 *
 * Information on what files are to be preloaded before anything else.
 * This can either be a string and is then a filename holding all
 * files to be preloaded.
 *
 * It can also be an array holding the files to preload.
 */
#define PRELOAD_FIRST "/d/Standard/preload.data"

#endif


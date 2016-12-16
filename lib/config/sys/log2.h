/*
 * /config/sys/log2.h
 *
 * Define all extra logfiles here.
 */

/*
 * AOP_TEAM_LOGS
 *
 * This definition contains a list of logs in /syslog/log that should
 * be visible to members of the AoP team.
 */
#define AOP_TEAM_LOGS ({ "enter", "MONEY_LOG", "DELETED", "SUSPENDED" })

// STRANGE_LOGIN temporarily (tm) removed from AOP_TEAM_LOGS.

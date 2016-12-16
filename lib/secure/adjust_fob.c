/*
 * /secure/adjust_fob.c
 *
 * This module can be used to convert the KEEPERSAVE used in CDlib 00.31 to
 * that of 01.00. Call the function adjust_fob() in it manually and you
 * will find a new KEEPERSAVE module in KEEPERSAVE.new.o in the directory
 * that keeps that file. The domain commands and experience are reset. All
 * sanctions are removed.
 *
 * Make sure you define FOB_KEEP_CHANGE_TIME when you want to keep the time
 * the rank/level or domain was last changed in your data-structure in the
 * KEEPERSAVE as well.
 */

#pragma no_clone
#pragma no_inherit
#pragma strict_types

#include <macros.h>
#include <std.h>

#define KEEPERSAVE "/syslog/KEEPERSAVE"

/*
 * Function name: level_to_rank
 * Description  : Function converts the level number of a person to a rank
 *                number. Note that this function uses hardcoded numbers for
 *                the levels. Do not ever do that in normal code!
 * Arguments    : int level - the level to convert.
 * Returns      : int - the rank converted.
 */
static nomask int
level_to_rank(int level)
{
    switch(level)
    {
    case      0: return WIZ_MORTAL;     break;
    case      1: return WIZ_APPRENTICE; break;
    case  2.. 8: return WIZ_PILGRIM;    break;
    case      9: return WIZ_RETIRED;    break;
    case 10..19: return WIZ_NORMAL;     break;
    case 20..29: return WIZ_MAGE;       break;
    case 30..39: return WIZ_LORD;       break;
    case 40..49: return WIZ_ARCH;       break;
    case     50: return WIZ_KEEPER;     break;
    }
}

/*
 * Function name: adjust_fob
 * Description  : Call this function to convert the relevant data structures
 *                in KEEPERSAVE.
 */
nomask void
adjust_fob()
{
    mapping keepersave;
    mapping m_domains;
    mapping m_wizards;
    string *names;
    int     index = -1;
    int     size;

    setuid();
    seteuid(getuid());

    keepersave = restore_map(KEEPERSAVE);
    m_domains  = keepersave["m_domains"];
    m_wizards  = keepersave["m_wizards"];
    names      = sort_array(m_indices(m_wizards));
    size       = sizeof(names);

    while(++index < size)
    {
	m_wizards[names[index]] =
	    ({ level_to_rank(m_wizards[names[index]][0]) }) +
	    m_wizards[names[index]][0..1] +
#ifdef FOB_KEEP_CHANGE_TIME
	    ({ 0 }) +
#endif FOB_KEEP_CHANGE_TIME
	    m_wizards[names[index]][2..3]
#ifdef FOB_KEEP_CHANGE_TIME
	    + ({ 0 })
#endif FOB_KEEP_CHANGE_TIME
	    ;
    }

    names = m_indices(m_domains);
    index = -1;
    size  = sizeof(names);

    while(++index < size)
    {
	m_domains[names[index]] =
	    m_domains[names[index]][0..0] +
	    ({ lower_case(names[index][0..2]) }) +
	    m_domains[names[index]][1..2] +
	    m_domains[names[index]][6..6] +
	    m_domains[names[index]][3..3] +
	    ({ 0, 0, 0 });
    }

    keepersave["m_trainees"] = ([ ]);
    save_map(keepersave, (KEEPERSAVE + ".new"));
}

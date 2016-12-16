/*
 * /secure/finger_player.c
 *
 * This is a special player object that is used when a restore_object
 * is performed in the finger_player() function of the master object.
 * It saves a lot of time compared to loading a normal player object.
 */

#include <config.h>

inherit MAIL_INFO_OBJ;
inherit SPECIAL_INFO_OBJ;

#include <const.h>
#include <formulas.h>
#include <language.h>
#include <living_desc.h>
#include <login.h>
#include <options.h>
#include <ss_types.h>
#include <state_desc.h>
#include <std.h>

private string	race_name,
		title,
		name,
		mailaddr,
		player_file,
		login_from,
		al_title,
		password,
		*adj_desc,
		mentor,
		options,
		*student;

private int	exp_points,
		exp_combat,
                exp_general,
                exp_max_total,
		age_heart,
		gender,
		login_time,
		logout_time,
		is_ghost,
		*learn_pref,
		alignment,
		*acc_exp,
                restricted,
                scar;

private mapping m_remember_name = ([ ]);

private static int wiz_level;
private static int stats_set=0;
private static int *stats;

#define STAT if (!stats_set) acc_exp_to_stats()
static void acc_exp_to_stats();

void
create()
{ 
    seteuid(0);
}

void
finger_info()
{
    seteuid(getuid());
    finger_mail();
    finger_special();
    seteuid(0);
}

nomask public int
load_player(string pl_name)
{
    int ret;
   
    if (!pl_name)
	return 0;
    if (member_array(" ", explode(pl_name, "")) >= 0)
        return 0;

    seteuid(getuid(this_object()));
    ret = restore_object(PLAYER_FILE(pl_name));
    seteuid(0);

    wiz_level = (SECURITY->query_wiz_rank(pl_name) > WIZ_MORTAL);

    /* Protect against too short arrays. */
    acc_exp = acc_exp + allocate(SS_NO_STATS - sizeof(acc_exp));
    learn_pref = learn_pref + allocate(SS_NO_STATS - sizeof(learn_pref));

    return ret;
}

nomask void
master_set_name(string n)
{
    if (previous_object() != find_object(SECURITY))
	return;

    name = n;
}

public nomask void
open_player()
{
    if (previous_object() == find_object(SECURITY))
	seteuid(0);
}

/*
 * Function name: query_finger_player
 * Description  : This function identifies this object as a finger-player
 *                object.
 * Returns      : int 1 - always.
 */
nomask public int
query_finger_player()
{
    return 1;
}

public int
notmet_me(object obj)
{
    if (obj && query_interactive(obj))
	return !obj->query_met(this_object());
    return !this_player()->query_met(this_object());
}

string
query_race()
{
    string race = player_file->query_race();

    if (member_array(race, RACES) == -1)
    {
        return RACES[0];
    }

    return race;
}

string query_race_name() { return race_name; }

string query_title() { return title; }

string query_real_name() { return name; }

string query_name() { return capitalize(name); }

string query_met_name() { return capitalize(name); }

mixed
query_adj(int param)
{
    if (param)
	return adj_desc;
    else
	return adj_desc[0];
}

public string
query_nonmet_name()
{
    string *adj, str;

    if (sizeof((adj = query_adj(1))) > 0)
        str = implode(adj, " ") + " " +
            race_name +
            (wiz_level ? (" " + LD_WIZARD) : "");
    else
        str = race_name +
            (wiz_level ? (" " + LD_WIZARD) : "");

    if (is_ghost & 1)
        str += " " + LD_GHOST;

    return str;
}

string
query_the_name(object pobj)
{
    if (!objectp(pobj))
	pobj = previous_object(-1);

    if (notmet_me(pobj))
	return LD_THE + " " + query_nonmet_name();
    else
	return query_met_name();
}

string query_The_name(object pobj) { return capitalize(query_the_name(pobj)); }

string query_mailaddr() { return mailaddr; }

string query_player_file() { return player_file; }

string query_login_from() { return login_from; }

int    query_login_time() { return login_time; }

int query_logout_time()
{
    /* For the purpose of backward compatibility, we use the file time
     * if no logout_time was remembered.
     */
    return logout_time ? logout_time : file_time(PLAYER_FILE(name) + ".o");
}

string query_pronoun() { return LD_PRONOUN_MAP[gender]; }

string query_possessive() { return LD_POSSESSIVE_MAP[gender]; }

int    query_gender() { return gender; }

string query_gender_string() { return LD_GENDER_MAP[gender]; }

int    query_age() { return age_heart; }

int    query_alignment() { return alignment; }

int    query_scar() { return scar; }

/*
 * Function name: query_password
 * Description  : Return the password of the player. Only SECURITY may do
 *                this.
 * Returns      : string - the password, else 0.
 */
nomask string
query_password()
{
    if (previous_object() != find_object(SECURITY))
    {
	return "that's a secret ;-)";
    }

    return password;
}

/*
 * Function name: match_password
 * Description  : Match the password of a player with an arbitrary string
 *                that is claimed to be the password of a player. NOTE that
 *                if the player has NO password, everything matches.
 * Arguments    : string p - the password to match.
 * Returns      : int - true/false.
 */
nomask int
match_password(string p)
{
    if (!strlen(password))
    {
	return 1;
    }

    return (password == crypt(p, password));
}

nomask int
query_option(int opt)
{
    if (!stringp(options))
        options = OPT_DEFAULT_STRING;

    switch(opt)
    {
    case OPT_WHIMPY:
        return atoi(options[6..7]);
    default:
        return 0;
    }
}

int query_whimpy() { return query_option(OPT_WHIMPY); }

int query_exp() { return (exp_points + exp_combat + exp_general); }

int query_exp_combat() { return exp_combat; }

int query_exp_quest() { return exp_points; }

int query_exp_general() { return exp_general; }

int query_max_exp() { return exp_max_total; }

public string
query_exp_title()
{
    if (wiz_level) 
        return "";

    return "nonpresent mortal";
}

string short() { return query_name();}

int query_ghost() { return is_ghost; }

int query_wiz_level() { return wiz_level; }

string 
long(mixed for_obj)
{
    return LD_PRESENT_TO(this_object());
}    

public string
query_objective()
{
    return LD_OBJECTIVE_MAP[gender];
}

public string
query_presentation()
{
    string b;
    
    b = query_exp_title(); 

    return query_name() + 
        (strlen(title) ? (" " + title + ",") : "") +
        (strlen(b) ? (" " + b + ",") : "") + " " +
        query_gender_string() + " " + 
        race_name + 
        (strlen(al_title) ? (" (" + al_title + ")") : "");
}

public mixed
query_learn_pref(int stat)
{
    if (stat < 0) return learn_pref;

    if (stat < 0 || stat >= SS_NO_STATS)
        return -1;

    return learn_pref[stat];
}

public int
query_acc_exp(int stat)
{
    if (stat < 0 || stat >= SS_NO_STATS)
        return -1;
    return acc_exp[stat];
}

int
set_base_stat(int stat, int val)
{
    if (stat < 0 || stat >= SS_NO_STATS || val < 1 )
        return 0;
    stats[stat] = val;
    return val;
}
  
nomask int
exp_to_stat(int exp)
{
    return F_EXP_TO_STAT(exp);
}

public int
query_base_stat(int stat)
{
    STAT;
    if (stat < 0 || stat >= SS_NO_STATS)
        return -1;
    return stats[stat];
}

public int
query_average_stat()
{
    STAT;
    return (stats[0] + stats[1] + stats[2] + stats[3] +
        stats[4] + stats[5]) / 6;
}

static void
acc_exp_to_stats()
{
    int il, tmp;

    stats = allocate(SS_NO_STATS);
    stats_set = 1;

    for (il = SS_STR; il < SS_NO_STATS; il++)
    {
        if (query_base_stat(il) >= 0)
        {
            tmp = exp_to_stat(query_acc_exp(il) * 
                RACESTATMOD[query_race()][il] / 10);
            set_base_stat(il, tmp);
        }
    }
}

int
query_stat(int stat)
{
    int tmp;

    if (stat < 0 || stat >= SS_NO_STATS)
        return -1;

    tmp = query_base_stat(stat);

    return (tmp > 0 ? tmp : 1);
}

public string
round_stat(int stat)
{
    if (stat > 100000)
	return sprintf("%2.1fM", itof(stat) / 1000000.0);
    else if (stat > 1000)
	return sprintf("%2.1fk", itof(stat) / 1000.0);
		
    return sprintf("%d", stat);
}

string
stat_living()
{
    string str;
    object to;

    to = this_object();
    str = sprintf(
	  "Name: %-11s Rank: %-10s " +
#ifdef USE_WIZ_LEVELS
          "(%-2d) " +
#endif
	  "Gender: %-10s Race: %s (%s)\n" +
          "File: %-35s\n"  +
	  "-----------------------------------------------------------------------------\n" +
	  "Exp: %9d %8s)  Quest: %7d  Combat: %8d  General: %8d\n" +
 	  "\n" +
	  "Stat: %@7s\n"  +
          "Value:%@7d\n" +
          "Base: %@7d\n" +
          "Exp:  %@7s\n" +
	  "Learn:%@7d\n\n" +
	  "Align: %d   Scar: %d   Ghost: %d   Whimpy: %d%%   Av.Stat: %4d\n",

                  capitalize(query_real_name()),
		  WIZ_RANK_NAME(SECURITY->query_wiz_rank(query_real_name())),
#ifdef USE_WIZ_LEVELS
		  SECURITY->query_wiz_level(query_real_name()),
#endif
                  to->query_gender_string(),
                  to->query_race_name(),
                  to->query_race(),
                  to->query_player_file(),
		  to->query_exp(),
		  ((to->query_max_exp() > to->query_exp()) ? "(" + to->query_max_exp() : "(max"),
		  to->query_exp_quest(),
		  to->query_exp_combat(),
		  to->query_exp_general(),
                  SS_STAT_DESC,

                  ({ query_stat(SS_STR), query_stat(SS_DEX),
                     query_stat(SS_CON), query_stat(SS_INT),
                     query_stat(SS_WIS), query_stat(SS_DIS),
                     query_stat(SS_RACE), query_stat(SS_OCCUP),
                     query_stat(SS_LAYMAN), query_stat(SS_CRAFT) }),

                  ({ F_EXP_TO_STAT(query_acc_exp(SS_STR)),
                     F_EXP_TO_STAT(query_acc_exp(SS_DEX)),
                     F_EXP_TO_STAT(query_acc_exp(SS_CON)),
                     F_EXP_TO_STAT(query_acc_exp(SS_INT)),
                     F_EXP_TO_STAT(query_acc_exp(SS_WIS)),
                     F_EXP_TO_STAT(query_acc_exp(SS_DIS)),
                     F_EXP_TO_STAT(query_acc_exp(SS_RACE)),
                     F_EXP_TO_STAT(query_acc_exp(SS_OCCUP)),
                     F_EXP_TO_STAT(query_acc_exp(SS_LAYMAN)),
                     F_EXP_TO_STAT(query_acc_exp(SS_CRAFT)) }),

                  map(({ query_acc_exp(SS_STR), query_acc_exp(SS_DEX),
			 query_acc_exp(SS_CON), query_acc_exp(SS_INT),
			 query_acc_exp(SS_WIS), query_acc_exp(SS_DIS),
			 query_acc_exp(SS_RACE), query_acc_exp(SS_OCCUP),
			 query_acc_exp(SS_LAYMAN), query_acc_exp(SS_CRAFT) }), round_stat),

                  to->query_learn_pref(-1),
		  to->query_alignment(),
		  to->query_scar(),
		  to->query_ghost(),
		  to->query_whimpy(),                  
                  to->query_average_stat()
          );
    return str;
}

void 
remove_object()
{
    destruct();
}

public int
query_restricted()
{
    return restricted;
}

public string
query_mentor()
{
    if (!stringp(mentor))
	return "";

    return mentor;
}

public string *
query_student()
{
    if (!sizeof(student))
	return ({});

    return student;
}

public mapping
query_remember_name()
{
    if (mappingp(m_remember_name))
	return ([ ]) + m_remember_name;
    else
	return ([ ]);
}

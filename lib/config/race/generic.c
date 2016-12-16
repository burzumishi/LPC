/*
 * Copyright (c) 1991 Chalmers Computer Society
 *
 * This code may not be copied or used without the written permission
 * from Chalmers Computer Society.
 */
/*
   /d/Standard/race/generic.c

   The generic race object for all the races in Standard

*/
#pragma save_binary

inherit "/std/player_pub";
inherit "/d/Standard/std/mail_stuff";
inherit "/d/Standard/std/special_stuff";

inherit "/std/combat/humunarmed";

#include "/sys/const.h"
#include "/sys/macros.h"
#include "/sys/stdproperties.h"
#include "/config/login/login.h"

static	mapping 	com_sounds;

/*
 * Function name: query_combat_file
 * Description:   Gives the name of the file to use for combat.
 */
public nomask string
query_combat_file() { return "/std/combat/chumlock"; }

finger_info()
{
    finger_mail();
    finger_special();
}

public void
start_player()
{
    int val;

    if (!sizeof(this_object()->query_cmdsoul_list()))
    {
	this_object()->add_cmdsoul(RACESOULCMD[this_object()->query_race()]);
	this_object()->add_cmdsoul(RACEMISCCMD[this_object()->query_race()]);
    }

    if (!query_prop(CONT_I_WEIGHT))
    {
	val = RACEATTR[this_object()->query_race()][1];
	add_prop(CONT_I_WEIGHT, val *1000);
    }
    if (!query_prop(CONT_I_VOLUME))
    {
	val = RACEATTR[this_object()->query_race()][4];
	add_prop(CONT_I_VOLUME, val * 1000);
    }
    com_sounds = RACESOUND;

    ::start_player();
}

query_def_start()
{
    return RACESTART[this_object()->query_race()];
}

query_orig_stat()
{
    return RACESTAT[this_object()->query_race()];
}

public int
communicate(string str) /* Must have it here for special with ' */
{
    string verb;

    verb = query_verb();
    if (str == 0)
	str = "";
    if (verb[0] == "'"[0])
	str = extract(verb, 1) + " " + str;

    say(QCTNAME(this_object()) + " @@race_sound:" + file_name(this_object()) +
	"@@: " + str + "\n");
    write("Ok.\n");
    return 1;
}

public string
race_sound()
{
    object pobj;
    string raceto, racefrom;

    pobj = previous_object();

    raceto = pobj->query_race();
    if (member_array(raceto, RACES) < 0)
	return "says";

    return com_sounds[raceto][this_object()->query_race()];
}

/* For each of the functions below which has a name with "occ" in
 * it, there is an equivalent function for layman and racial guilds.
 * For those guild types, the function name is the same except that
 * "occ" is replaced by "lay" or "race" respectively.
 */

/*
 * Inherit /std/guild/guild_occ_sh.c for occupational guilds
 *         /std/guild/guild_lay_sh.c for layman guilds
 *         /std/guild/guild_race_sh.c for racial guilds
 */

#pragma strict_types

inherit "/std/guild/guild_occ_sh";

#include "guild.h"

#include <macros.h>
#include <ss_types.h>
#include <stdproperties.h>

/* the code for special attacks is in a seperate file
 * for easier debugging.  It's common to find all of
 * the code for a special attack in the soul, but placing
 * most of the code in the shadow can often make handling
 * specials easier.
 */
#include "special_attacks.c"

#define GUILD_SUBLOC  GUILD_NAME + " subloc"
#define MAX_GUILD_LEVEL 2

static string *titles = ({ "Trainee", "Fighter", "Warrior", });

/* Prototypes */
void remove_occ_member();

/* 
 * Function name: query_guild_tax_occ
 * Description:   Give the amount of tax the guild charges
 * Returns:       int - The tax amount
 */
int
query_guild_tax_occ() 
{ 
    return GUILD_TAX; 
}

/*
 * Function name: query_guild_style_occ
 * Description:   return the guild style for this guild
 * Returns:       string - the style
 */
string
query_guild_style_occ()
{
    return GUILD_STYLE;
}

/*
 * Function name: query_guild_name_occ
 * Description:   return the guild's name
 * Returns:       string - the guild name
 */
string
query_guild_name_occ()
{
    return GUILD_NAME;
}

/*
 * Function name: acceptable_member
 * Description:   test to see if the player is an acceptable member of this
 *                guild.
 * Arguments:     object who - the player to test
 * Returns:       0 or a string message saying why the player isn't acceptable
 */
mixed
acceptable_member(object who)
{
    /* throw out goblins */
    if ((who->query_race() == "goblin") && (who->query_race() != "ghost"))
    {
	return "We don't like goblins.\n";
    }

    return 0;
}
      
/* 
 * Function name: query_guild_keep_player
 * Description:   Test if we want to keep the player in the guild.
 * Arguments:     ob - the player.
 * Returns:	  1 if keeping, 0 if not.
 */
int
query_guild_keep_player(object ob)
{
    mixed why;

    if (stringp(why = acceptable_member(ob)))
    {
	ob->catch_msg(why);

        /* Normally we would just return 0 here, and the player would be
         * removed from the guild.  We want to make sure a few cleanup
         * actions happen, though, so we'll return 1 here and remove
         * the member ourselves.
         */

        set_alarm(1.0, 0.0, remove_occ_member);

        return 1;
    }

    return 1;
}

/* 
 * Function name: query_guild_not_allow_join_occ
 * Description:	  Check if we allow the player to join another guild.
 * Arguments:     player - the player
 *		  type, style, name - the type, style and name of the
 *			intended guild.
 * Returns:	  1 if we do not allow, else 0
 */
int
query_guild_not_allow_join_occ(object player, string type, string style,
				string name)
{
    /* This checks to make sure that the new guild is not occupational */
    if (::query_guild_not_allow_join_occ(player, type, style, name))
        return 1;

    /* We don't want our members joining thief guilds */
    if (style == "thief")
    {
        player->catch_msg("We brave fighters don't want thieves in our " +
            "ranks.\n");
        return 1;
    }

    return 0;
}

/* Function name: query_guild_level_occ
 * Description:   What is the member's level within the guild?
 * Returns:	  int - the level
 */
int
query_guild_level_occ()
{
    return MIN(MAX_GUILD_LEVEL, query_shadow_who()->query_stat(SS_OCCUP) / 50);
}
    
/* Function name: query_def_post
 * Description:   We change the default post office for our members.
 * Returns:	  string - the path to the new postoffice
 */
string
query_def_post() 
{
    /* Only bother using the guild's post office if the player is
     * using the guild's start location.
     */
    if (this_player()->query_default_start_location() == GUILD_STARTLOC)
    {
        return GUILD_POST;
    }

    return query_shadow_who()->query_def_post();
}

/*
 * Function name: query_guild_title_occ
 * Description:   Return the member's guild title
 * Returns:       string - the title
 */
string 
query_guild_title_occ()
{
    return titles[query_guild_level_occ()];
}
    
/* 
 * Function name: query_guild_skill_name
 * Description:   Give a names for the guild's special skills
 * Arguments:     type - the number of the skill
 * Returns:	  0 if no skill of mine, else the string.
 */
mixed
query_guild_skill_name(int type)
{
    if (type == SS_GUILD_SPECIAL_SKILL)
    {
        return "occ guild special skill";
    }

    return 0;
}

/*
 * Function name: query_guild_trainer_occ
 * Description:   Returns the skill trainers for the guild
 * Returns:       string * - an array of paths to the guild's
 *                trainers.
 */
mixed
query_guild_trainer_occ()
{
    return ({ GUILD_DIR + "train" });
}

/*
 * Function name: init_guild_member
 * Description:   Add necessary guild items to the player and
 *                do any other kind of initialization necessary.
 */
void
init_guild_member()
{
    object guild_emblem, who = query_shadow_who();

    /* clone the guild emblem and move it to the player */

    setuid();
    seteuid(getuid());
    
    if (guild_emblem = clone_object(GUILD_EMBLEM))
    {    
        guild_emblem->move(who, 1);
    }

    /* add the guild's soul to the player */
    who->add_cmdsoul(GUILD_CMDSOUL);
    who->update_hooks();

    /* add the guild subloc */
    who->add_subloc(GUILD_SUBLOC, this_object());
}

/*
 * Function name: init_occ_shadow()
 * Description:   This function is called from autoload_shadow and may
 *                be used to initialize the shadow when it's loaded.
 * Arguments:     The argument string sent to autoload_shadow.
 */
void init_occ_shadow(string arg)
{
    /* delay for a moment so that the player completes all
     * of login before we continue
     */
    set_alarm(1.0, 0.0, init_guild_member);
}

/* 
 * Function name: add_occ_shadow
 * Description:   Shadow an object with this shadow
 * Arguments:     object who - the object to shadow
 * Returns:       as shadow_me in /std/guild/guild_base.c
 */
int add_occ_shadow(object who)
{
    return shadow_me(who, "occupational", GUILD_STYLE, GUILD_NAME);
}

/*
 * Function name: show_subloc
 * Description:   This function is called each time someone looks at us
 * Arguments:     subloc  - Our subloc
 *		  me      - I
 *		  for_obj - The looker
 * Returns:	  The string the looker shall see
 */
string
show_subloc(string subloc, object me, object for_obj)
{
    string str;

    if (subloc != GUILD_SUBLOC)
    {
        return me->show_subloc(subloc, me, for_obj);
    }

    if (me->query_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS))
    {
	return "";
    }

    if (for_obj == me)
    {
	str = "You are ";
    }
    else
    {
	str = capitalize(me->query_pronoun()) + " is ";
    }

    return str + "a member of the " + GUILD_NAME + ".\n";
}

/*
 * Function name: remove_occ_member
 * Description:   remove this member from the guild
 */
void
remove_occ_member()
{
    object emblem, who = query_shadow_who();

    /* Remove special skills */ 
    who->remove_skill(SS_GUILD_SPECIAL_SKILL);

    /* Clear the player's guild stat */
    who->clear_guild_stat(SS_OCCUP);

    /* Remove any instances of the guild emblem on the player */
    while (emblem = present(GUILD_EMBLEM_ID, who))
    {
        emblem->remove_object();
    }

    /* If the player is using our start location, remove it */
    if (who->query_default_start_location() == GUILD_STARTLOC)
    {
        who->set_default_start_location(who->query_def_start());
    }

    /* Remove the guild soul */
    who->remove_cmdsoul(GUILD_CMDSOUL);
    who->update_hooks();

    /* Remove the guild shadow.  This also resets tax to 0. */
    remove_guild_occ();
}

/*
 * /lib/no_skill_decay.c
 *
 * This module can be used to protect the skills people may train from a
 * special guru or quest. It their names are secured in this module, they
 * may retrain their skill after it decayed.
 *
 * In order to use this module, you have to change the inheritance of
 * /lib/skill_raise in the object to /lib/no_skill_decay. This module
 * itself inherits the /lib/skill_raise so you still need to set that
 * module up like normal. Then set the directory of the save-filess with
 *
 *     void set_nsd_directory(string directory)
 *
 * In that directory you must create 26 directories, each with a letter of
 * the alphabet as name. When a player trains a certain skill that should
 * be retrainable, you must make a call to
 *
 *     void set_nsd_retrain(string player, int skill, int level)
 *
 * You must inform the players of the possibility to retrain their skills
 * yourself and you must tell them the syntax "retrain <skill name>" where
 * <skill name> is the same name as people would use with 'learn' and
 * 'improve'. To link the command to the players, make the following call
 * from your init() function where you also have init_skill_raise().
 *
 *     void init_nsd();
 *
 * WARNING: Before you may use this object to make a skill retrainable you
 *          _must_ get the approval of the administration. In the file that
 *          inherits this object you must include the name of the member of
 *          the administration who gave you the permission. You need this
 *          permission for each skill individually.
 *
 * /Mercade, July 14 1995
 */

inherit "/lib/skill_raise";

#pragma no_clone
#pragma resident
#pragma save_binary
#pragma strict_types

#include <macros.h>

/*
 * Global variable.
 */
static private string nsd_directory;

/*
 * Prototype.
 */
static nomask int nsd_retrain(string str);

#define RETRAIN_FILE(s) (nsd_directory + extract((s), 0, 0) + "/" + (s))
#define MAKE_STRING(s)  ("" + (s))

/*
 * Function name: init_nsd
 * Description  : Call this function to add the command 'retrain' to
 *                players.
 */
nomask static void
init_nsd()
{
    add_action(nsd_retrain, "retrain");
}

/*
 * Function name: set_nsd_directory
 * Description  : Use this function to set the directory in which the
 *                save-files for this module as saved in.
 * Arguments    : string directory - the name of the directory.
 */
nomask public void
set_nsd_directory(string directory)
{
    nsd_directory = directory;

    /* If there is no trailing slash '/' we add one. */
    if (!wildmatch("*/", nsd_directory))
    {
    	nsd_directory += "/";
    }
}

/*
 * Function name: query_nsd_directory
 * Description  : Return the name of the directory in which the save-files
 *                are stored.
 * Returns      : string - the name of the directory.
 */
nomask public string
query_nsd_directory()
{
    return nsd_directory;
}

/*
 * Function name: set_nsd_retrain
 * Description  : When a player trains a skill to a level that should be
 *                retrainable, this function must be called.
 * Arguments    : string player - the name of the player.
 *                int    skill  - the skill to make retrainable.
 *                int    level  - the level to that may be retrained.
 */
nomask public void
set_nsd_retrain(string player, int skill, int level)
{
    mapping file;

    setuid();
    seteuid(getuid());

    /* Retrieve the players file. */
    player = lower_case(player);
    
    catch(file = restore_map(RETRAIN_FILE(player)));
    if (!mappingp(file) ||
    	!m_sizeof(file))
    {
    	file = ([ ]);
    }

    /* See whether the new level is a new maximum. This would seem logical,
     * but I prefer to test it anyway.
     */
    if (level > file[MAKE_STRING(skill)])
    {
    	file[MAKE_STRING(skill)] = level;

	save_map(file, RETRAIN_FILE(player));
    }
}

/*
 * Function name: query_nsd_retrain
 * Description  : This function can be queried to find out which skills a
 *                player may retrain at this unit.
 * Arguments    : string player - the name of the player.
 * Returns      : mapping - the mapping with the skills the player can
 *                          retrain. The indices are the skill numbers and
 *                          the values are the levels to which the skill is
 *                          retrainable. WARNING: the skill numbers have
 *                          been turned into strings.
 */
nomask public mapping
query_nsd_retrain(string player)
{
    player = lower_case(player);
    return ([ ]) + restore_map(RETRAIN_FILE(player));
}

/*
 * Function name: nsd_hook_access_failure
 * Description  : The basic access failure.
 * Returns      : int 1/0 - 0 for notify_fail, 1 for write.
 */
public int
nsd_hook_access_failure()
{
    notify_fail(capitalize(query_verb()) + " which skill?\n");
    return 0;
}

/*
 * Function name: nsd_hook_unknown_skill
 * Description  : The skill the player tries to retrain is not known.
 * Returns      : int 1/0 - 0 for notify_fail, 1 for write.
 */
public int
nsd_hook_unknown_skill(string str)
{
    notify_fail("There is no skill named \"" + str +
	"\" known to be retrainable here.\n");
    return 0;
}

/*
 * Function name: nsd_hook_not_retrainable.
 * Description  : The player does not have the right to retrain that skill
 *                because it is know set for him.
 * Arguments    : int skill - the skill number.
 * Returns      : int 1 - should always return 1.
 */
public int
nsd_hook_not_retrainable(int skill)
{
    write("You do not have the right to retrain your ability to " +
    	sk_tdesc[skill][0] + " here.\n");
    return 1;
}

/*
 * Function name: nsd_hook_not_retrainable.
 * Description  : The player does not have the right to retrain that skill
 *                because it is know set for him.
 * Arguments    : int skill - the skill number.
 *                int level - the maximum to which the player may retrain
 *                            that skill.
 * Returns      : int 1 - should always return 1.
 */
public int
nsd_hook_skill_too_high(int skill, int level)
{
    write("You may only retrain your ability to " +
    	sk_tdesc[skill][0] + " to " + sk_rank(level) +
	" and your level in that skill is higher than that.\n");
    return 1;
}

/*
 * Function name: nsd_retrain_is_not_possible
 * Description  : Mask this function if you want to have some form of
 *                additional test to see whether the player may retrain.
 *                The function operates on this_player(). If you want to
 *                disallow the player to train, you must also print a
 *                message to him/her.
 * Arguments    : int skill - the skill the player wants to retrain.
 *                int level - the level to which the player is allowed to
 *                            retrain the skill.
 * Returns      : int 1/0 - true if it should be impossible for the player
 *                          to retrain the skill to the level indicated.
 */
public int
nsd_retrain_is_not_possible(int skill, int level)
{
    return 0;
}

/*
 * Function name: nsd_hook_retrain_skill
 * Description  : Give the player and the people in the room a nice message.
 * Arguments    : int skill - the skill the player retrains.
 *                int level - the level to which the player retrains the
 *                            skill.
 * Returns      : int 1 - the must always return 1!
 */
public int
nsd_hook_retrain_skill(int skill, int level)
{
    write("You retrain your ability to " + sk_tdesc[skill][0] +
    	" to the level of " + sk_rank(level) + ".\n");
    say(QCTNAME(this_player()) + " retrains " +
    	this_player()->query_possessive() + " ability to " +
    	sk_tdesc[skill][1] + ".\n");
    return 1;
}

/*
 * Function name: nsd_retrain
 * Descritpion  : This function is the actual retrain command that is called
 *                when a player wants to retrain his skill before it decays
 *                too much.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
nomask static int
nsd_retrain(string str)
{
    mapping file;
    string  name;
    int     skill;

    /* Access failure. */
    if (!strlen(str))
    {
    	return nsd_hook_access_failure();
    }

    /* Unknown skill. */
    if ((skill = sk_find_skill(str)) < 0)
    {
    	return nsd_hook_unknown_skill(str);
    }

    name = this_player()->query_real_name();
    file = restore_map(RETRAIN_FILE(name));

    /* The player has no right to retrain this skill. */
    if (!file[MAKE_STRING(skill)])
    {
    	return nsd_hook_not_retrainable(skill);
    }

    /* The players skill is higher than the level to which he or she can
     * retrain it.
     */
    if (file[MAKE_STRING(skill)] <= this_player()->query_base_skill(skill))
    {
    	return nsd_hook_skill_too_high(skill, file[skill]);
    }

    /* If this function returns true, there is a particular reason why
     * the player cannot retrain the skill. This function must be
     * defined by the inheriting object.
     */
    if (nsd_retrain_is_not_possible(skill, file[MAKE_STRING(skill)]))
    {
    	return 1;
    }

    /* Set the new skill in the player. */
    this_player()->set_skill(skill, file[MAKE_STRING(skill)]);

    /* Print a nice message to the player and the surroundings. */
    return nsd_hook_retrain_skill(skill, file[MAKE_STRING(skill)]);
}

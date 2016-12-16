/*
 * /std/guild/guild_craft_sh.c
 *
 * This is the standard guild shadow for a craft guild. It defines
 * The necessary functions.
 */

/*
 * Functions in this object.
 *
 * Arguments:    s = string, i = integer, o = object
 *
 * query_guild_tax_craft()   Should return the tax we want the player to pay.
 *			     If the tax is to change, i.e. the player learns a
 *			     new spell, call notify_guild_tax_changed() to set
 *			     the tax correctly.
 *
 * query_guild_type()        Always returns "craft"
 *
 * query_guild_style_craft() What type are we? 
 *
 * query_guild_name_craft()  The name of the guild.
 *
 * query_guild_title_craft() This is called from the player object to find
 *                           the title of this guild.
 *
 * query_guild_not_allow_join_craft(o, s, s, s) This is called when our member
 *			     tries to join another guild. The type, style and 
 *			     name of the other guild is sent as argument and we
 *			     can deny our member to join that guild if we want.
 *			     Observe, returning 1 will deny the player. This
 *			     function will also be called each time a player 
 *			     logs on, we could test if the race has changed or
 *			     anything....
 *
 * query_guild_member_craft() Is always true, ofcourse.
 *
 * query_guild_trainer_craft() Returns a reference to the file or files that
 *			     defines sk_query_max for the player in question.
 *			     The returned items could be a string, an object
 *			     pointer or an array of the same.
 *
 * query_guild_leader_craft() Define this function to return true if the player
 *                           is considered to be one of the leaders of his/her
 *                           guild in the form of a council-membership, et
 *                           cetera.
 *
 * query_guild_incognito_craft() Define this function so that it returns true
 *                           if the player's guild affiliation should be unknown.
 *
 * remove_guild_craft()      Call this if you are removing your guild from 
 *			     the player, and you are an craft guild.
 *
 * autoload_shadow(s)        Is called when a player enters and we shall load
 *			     the autoloading shadows.
 *
 * Note: You will probably want to redefine query_guild_tax_craft() and
 *       query_guild_not_allow_join_craft(s, s, s).
 */

#pragma strict_types

inherit "/std/guild/guild_base";

static private string svn_id = "$Id$";

#include <macros.h>
#include <ss_types.h>

/*
 * Function name: query_guild_tax_craft
 * Description:   What's the tax to pay at this guild? You should write your
 *                own version of this function.
 * Returns:       the tax this guild wants
 */
public int
query_guild_tax_craft()
{
    return 0;
}

/*
 * Function name: query_guild_type
 * Description:   What type of guild is this, occupational, layman, 
 *		  race or craft?
 * Returns:       a string with the type
 */
public nomask string
query_guild_type()
{
    return "craft";
}

/*
 * Function name: query_guild_style_craft
 * Description:   What styles is this guild?
 *		  You should redefine this function to suite you.
 * Returns:       string - holding the style of guild
 */
public string
query_guild_style_craft()
{
    return "dreamer";
}

/*
 * Function name: query_guild_name_craft
 * Description:   Returns the name of the guild this shadow represents
 *		  You should redefine this function to suite you.
 * Returns:	  The name of the guild
 */
public string
query_guild_name_craft()
{
    return "craft";
}

/*
 * Function name: query_guild_title_craft
 * Description  : This function should return the craft title off
 *                the player. Since this function is not called for wizards
 *                you do not have to distinct for them in this function.
 * Returns      : string - the title for the player.
 *                The title will be incorporated in the format
 *                "the <race title>, <occ title>, <lay title> and 
 *                 <craft title>" if the player is a member of all 
 *	          major guild types.
 */
public string
query_guild_title_craft()
{
    return "";
}

/*
 * Function name: query_allow_join_craft
 * Description:   Test if this guild allowed a player to join another
 * Arguments:     type  - the type of guild to join
 * 		  style - the style of the guild
 *		  name  - the name
 * Returns:       1 - I do not allow member to join another guild
 */
public int
query_guild_not_allow_join_craft(object player, string type, string style,
				 string name)
{
    if (type == "craft")
    {
        notify_fail("One craft guild is enough.\n");
	return 1;
    }

    return 0;
}

/*
 * Function name: query_guild_member_craft
 * Description:   This is an craft guild
 * Returns:	  1
 */
public nomask int
query_guild_member_craft()
{
    return 1;
}

/*
 * Function name: query_guild_trainer_craft
 * Description:   Return one or more references to the object that define
 *                sk_train_max for the player. The returned refernce can
 *                be a string reference, an object pointer or an array of
 *                those.
 * Returns:       See description.
 */
public mixed
query_guild_trainer_craft()
{
    return 0;
}

/*
 * Function name: query_guild_leader_craft
 * Description  : Define this function to return true if the person is a
 *                leader of his/her craft guild, in council-membership
 *                or something similar.
 * Returns      : int 1/0 - craft guild leader or not.
 */
public int
query_guild_leader_craft()
{
    return 0;
}

/*
 * Function name: query_guild_incognito_craft
 * Description:   Define this function so that it returns true if
 *                the player's guild affiliation should be unknown.
 * Returns:       1/0 - unknown/not
 */
public int
query_guild_incognito_craft()
{
    return 0;
}

/*
 * Function name: remove_guild_member_craft
 * Description:   Remove a player from the guild. Will take care of everything.
 * Returns:       1 if removed
 */
public nomask int
remove_guild_craft()
{
    if (!shadow_who->remove_autoshadow(MASTER + ":"))
	return 0;

    shadow_who->set_guild_pref(SS_CRAFT, 0);
    remove_shadow();
    return 1;
}

/*
 * Function name: init_craft_shadow()
 * Description: This function is called from autoload_shadow and may
 *              be used to initialize the shadow when it's loaded.
 * Arguments: The argument string sent to autoload_shadow.
 */
public void
init_craft_shadow(string arg)
{
}

/*
 * Function name: autoload_shadow
 * Description:   Called by the autoloading routine in /std/player_sec
 *		  to ensure autoloading of the shadow.
 * Arguments:	  str - the string holding type, style and name of the guild
 */
public nomask void
autoload_shadow(string str)
{
    if (shadow_who)
	return; /* Already shadowing */

    if (query_guild_not_allow_join_guild(this_player(), "craft",
		query_guild_style_craft(), query_guild_name_craft()))
    {
	write("Your guilds don't seem to get along. Your craft guild " +
	      "is removed.\n");
	this_player()->remove_autoshadow(MASTER);
	return;
    }

    if (!query_guild_keep_player(this_player()))
    {
	this_player()->remove_autoshadow(MASTER);
	return;
    }

    ::autoload_shadow(str);
    this_object()->init_craft_shadow(str);

    this_player()->set_guild_pref(SS_CRAFT, query_guild_tax_craft());
}

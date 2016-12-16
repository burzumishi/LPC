/*
 * /lib/commands.c
 *
 * This library contains several functions that make it a lot easier to make
 * commands that allow a player to interact with others. It is inherited into
 * /cmd/std/command_driver.c to allow access from souls, but you can also
 * inherit it from any other object.
 */

#pragma no_clone
#pragma no_shadow
#pragma save_binary
#pragma strict_types

#include <adverbs.h>
#include <cmdparse.h>
#include <composite.h>
#include <filter_funs.h>
#include <language.h>
#include <macros.h>
#include <stdproperties.h>

/* Prototypes. */
public object * check_block_action(object *targets, int cmd_attr);

/*
 * Global variable.
 *
 * parse_msg - the error message when parsing distances and access.
 */
static string parse_msg = "";

/*
 * Function name: desc_vbfc
 * Description  : This function takes an object and returns the macro QTNAME
 *                applied to it. We need this map function because it is not
 *                possible to treat macros like functionpointers.
 * Arguments    : object ob - the living object.
 * Returns      : string - the macro QTNAME applied to the object.
 */
public string
desc_vbfc(object ob)
{
    return QTNAME(ob);
}

/*
 * Function name: desc_pos_short
 * Description  : This function takes an object and returns LANG_THESHORT
 *                applied to it. We need this map function because it is not
 *                possible to treat macros like functionpointers.
 * Arguments    : object ob - the object.
 *                object for_obj - the looking object.
 * Returns      : string - the macro LANG_THESHORT applied to the object.
 */
public varargs string
desc_pos_short(object ob, object for_ob)
{
    if (ob->query_prop(HEAP_I_IS))
    {
        return LANG_POSS(ob->short(for_ob));
    }
    return "the " + LANG_POSS(ob->short(for_ob));
}

/*
 * Function name: desc_many
 * Description  : Gives the player dependant descriptions of all objects
 *                adressed by the actor. In short, it maps objects to the
 *                macro via desc_vbfc and then makes a composite string.
 * Arguments    : object *oblist - the objects to process and link.
 * Returns      : string - a compound description.
 */
public string
desc_many(object *oblist)
{
    return COMPOSITE_WORDS(map(oblist, desc_vbfc));
}

/*
 * Function name: actor
 * Description  : Prints the message to the performer of an action when
 *                acting towards a target. Note that if you omit the
 *                optional second string, a period is automatically added.
 *                A newline is always added. Note the position of the
 *                spaces in the examples. Possessive form on the target(s)
 *                is handled automatically.
 * Example      : actor("You smile at", oblist);
 *
 *                You smile at someone.
 *                You smile at Mrpr.
 *                You smile at the darkly robed human wizard.
 *
 *                actor("You give", oblist, " a hug.");
 *
 *                You give someone a hug.
 *                You give Mrpr a hug.
 *                You give the darkly robed human wizard a hug.
 *
 * Arguments    : string str     - the first part of the message to print.
 *                object *oblist - the targets of the emotion.
 *                string str1    - an optional second part of the message.
 *                                 Start with "'s" for possessive target form.
 */
public varargs void
actor(string str, object *oblist, string str1)
{
    int poss;

    /* Sanity check. */
    if (!sizeof(oblist))
        return;

    if (strlen(str1) && (str1[..1] == "'s"))
    {
	poss = 1;
	str1 = str1[2..];
    }

    if (living(oblist[0]))
    {
	function oname = (poss == 0 ?
	  &->query_the_name(this_player()) :
	  &->query_the_possessive_name(this_player()));

	write(str + " " +
	    COMPOSITE_WORDS(map(oblist, oname)) +
	    (strlen(str1) ? (str1 + "\n") : ".\n"));
    }
    else if (poss)
    {
        /* Only make a manual composition for the possessive form. */
	write(str + " " +
	    COMPOSITE_WORDS(map(oblist, &desc_pos_short(, this_player()))) +
	    (strlen(str1) ? (str1 + "\n") : ".\n"));
    }
    else
    {
        write(str + " " + COMPOSITE_DEAD(oblist) +
	    (strlen(str1) ? (str1 + "\n") : ".\n"));
    }

    this_player()->emote_hook_actor(query_verb(), oblist);
}

/*
 * Function name: target
 * Description  : Print the message that the target of an action gets. Note
 *                that a newline is added by this routine. Possessive form on
 *                on the target(s) is automatically handled.
 * Example      : target(" hugs you.", oblist);
 *
 *                Someone hugs you.
 *                Mrpr hugs you.
 *                The darkly robed human wizard hugs you.
 *
 * Arguments    : string str     - the message to print. Start with "'s" for
 *                                 possessive actor form.
 *                object *oblist - the people to get the message.
 *                string adverb  - the optional adverb if one was used.
 *                int cmd_attr   - the action attributes (from cmdparse.h)
 */
public varargs void
target(string str, object *oblist, string adverb = "", int cmd_attr = 0)
{
    int poss;
    object *players, *all_oblist;

    /* Sanity check. */
    if (!sizeof(oblist))
	return;

    /* Non-living targets see or feel no emotes, but we can still trigger
     * on it, so call the hook.
     */
    if (!living(oblist[0]))
    {
	oblist->emote_hook(query_verb(), this_player(), adverb, oblist,
	    cmd_attr, 1);
	return;
    }

    if (str[..1] == "'s")
    {
	poss = 1;
	str = str[2..];
    }

    all_oblist = oblist;

    /* Who gets to see the emote */
    if (ACTION_BLIND & cmd_attr)
    {
	oblist = FILTER_CAN_SEE_IN_ROOM(oblist);
	oblist = FILTER_IS_SEEN(this_player(), oblist);
    }

    /* Give only a hook to NPC's. */
    players = FILTER_PLAYERS(oblist);

    /* Tell the message to players. */
    if (sizeof(players))
    {
	function name = (poss ? this_player()->query_The_possessive_name :
	    this_player()->query_The_name);

	foreach(object player: players)
	{
	    player->catch_tell(name(player) + str + "\n");
	}
    }

    oblist->emote_hook(query_verb(), this_player(), adverb, all_oblist,
	cmd_attr, 1);
}

/*
 * Function name: targetbb
 * Description  : Same as the function target(), though if the target cannot
 *                see the actor, nothing is printed.
 * Example      : targetbb(" smiles happily.", oblist);
 * Arguments    : see target().
 */
public varargs void
targetbb(string str, object *oblist, string adverb = "", int cmd_attr = 0)
{
    target(str, oblist, adverb, cmd_attr | ACTION_BLIND);
}

/*
 * Function name: all
 * Description  : Print the message that all onlookers get when there is
 *                only one person acting. Ie. those who are watching someone
 *                do something alone. Note that a newline is always added by
 *                this routine added. Possessive form on the actor is handled
 *                automatically.
 * Example      : all(" screams loudly.");
 *
 *                Someone screams loudly.
 *                Fatty screams loudly.
 *                The big fat gnome wizard screams loudly.
 *
 * Arguments    : string str    - the message to print, starting with a space,
 *                                or start with "'s" for possessive actor form.
 *                string adverb - the optional adverb if one was used.
 *                int cmd_attr  - the action attributes (from cmdparse.h)
 */
public varargs void
all(string str, string adverb = "", int cmd_attr = 0)
{
    int poss;
    object *oblist, *players;

    if (str[..1] == "'s")
    {
	poss = 1;
	str = str[2..];
    }

    oblist = FILTER_OTHER_LIVE(all_inventory(environment(this_player())));

    if (ACTION_BLIND & cmd_attr)
    {
	oblist = FILTER_CAN_SEE_IN_ROOM(oblist);
	oblist = FILTER_IS_SEEN(this_player(), oblist);
    }

    oblist  = check_block_action(oblist, cmd_attr);
    players = FILTER_PLAYERS(oblist);

    /* Tell the message to players. */
    if (sizeof(players))
    {
	function name = (poss ? this_player()->query_The_possessive_name :
	    this_player()->query_The_name);

	foreach(object player: players)
	{
	    player->catch_tell(name(player) + str + "\n");
	}
    }

    oblist->emote_hook(query_verb(), this_player(), adverb, 0, cmd_attr, 0);
    this_player()->emote_hook_actor(query_verb(), oblist);
}

/*
 * Function name: allbb
 * Description  : Same as all(), but people who cannot see the person
 *                performing, won't see it happen.
 * Example      : allbb(" smiles happily.");
 * Arguments    : see all().
 */
public varargs void
allbb(string str, string adverb = "", int cmd_attr = 0)
{
    all(str, adverb, cmd_attr | ACTION_BLIND);
}

/*
 * Function name: all2act
 * Description  : Print the message that all onlookers get when two players
 *                interact. A newline is always added. If the second,
 *                optional argument is omitted, a period is also added.
 *                Possessive form on the actor and target(s) are handled
 *                automatically.
 * Example      : all2act(" tackles", oblist);
 *
 *                Someone tackles Mrpr.
 *                Fatty tackles the darkly robed human wizard.
 *                Fatty tackles someone.
 *                The big fat gnome wizard tackles Mrpr.
 *                (etc for the other unseen/met/nonmet combinations)
 *
 *                all2act(" pokes", oblist, " in the ribs.");
 *
 *                Fatty pokes the darkly robed human wizard in the ribs.
 *                (etcetera.)
 *
 * Arguments    : string str     - the first part of the message to print.
 *                                 Start with "'s" for possessive actor form.
 *                object *oblist - the targets of the emotion, NOT the people
 *                                 who are watching.
 *                string str1    - the optional second part of the message.
 *                                 Start with "'s" for possessive target form.
 *                string adverb  - the optional adverb if one was used.
 *                int cmd_attr   - the action attributes (from cmdparse.h)
 */
public varargs void
all2act(string str, object *oblist, string str1, string adverb = "",
    int cmd_attr = 0)
{
    int    a_poss, o_poss;
    object *livings, *players;

    /* Sanity check. */
    if (!sizeof(oblist))
	return;

    livings = all_inventory(environment(this_player()));
    livings = FILTER_OTHER_LIVE(livings - oblist);

    if (ACTION_BLIND & cmd_attr)
    {
	livings = FILTER_CAN_SEE_IN_ROOM(livings);
	livings = FILTER_IS_SEEN(this_player(), livings);
    }

    if (!sizeof(livings))
	return;

    if (str[..1] == "'s")
    {
	a_poss = 1;
	str = str[2..];
    }

    if (strlen(str1) &&
	(str1[..1] == "'s"))
    {
	o_poss = 1;
	str1 = str1[2..];
    }

    players = FILTER_PLAYERS(livings);
    if (sizeof(players))
    {
	function name = (a_poss ? this_player()->query_The_possessive_name :
	    this_player()->query_The_name);

	str1 = (strlen(str1) ? (str1 + "\n") : ".\n");
	if (living(oblist[0]))
	{
	    foreach(object player: players)
	    {
		function oname = (o_poss ? &->query_the_possessive_name(player) :
		    &->query_the_name(player));

		player->catch_tell(name(player) + str + " " +
		    COMPOSITE_WORDS(map(oblist, oname)) + str1);
	    }
	}
	else if (o_poss)
	{
	    foreach(object player: players)
	    {
		player->catch_tell(name(player) + str + " " +
		    COMPOSITE_WORDS(map(oblist, &desc_pos_short(, player))) + str1);
	    }
	}
	else
	{
	    foreach(object player: players)
	    {
		player->catch_tell(name(player) + str + " " +
		    FO_COMPOSITE_DEAD(oblist, player) + str1);
	    }
	}
    }

    livings->emote_hook_onlooker(query_verb(), this_player(), adverb, oblist,
	cmd_attr);
}

/*
 * Function name: all2actbb
 * Description  : This is to be used when people cannot see who is doing the
 *                action. I.e. you shouldn't see that someone smiles if you
 *                cannot see that person. For more information. See all2act.
 * Arguments    : See all2act.
 */
public varargs void
all2actbb(string str, object *oblist, string str1, string adverb = "",
    int cmd_attr = 0)
{
    all2act(str, oblist, str1, adverb, cmd_attr | ACTION_BLIND);
}


/*
 * Functon name: cmd_access
 * Description:  See if a command is blocked for a particular target.
 * Arguments:    object ob      - the command target
 *               object for_obj - the actor
 *               int cmd_attr  - the command's attributes (from cmdparse.h)
 * Returns:      1 - command not blocked
 *               0 - command blocked
 */
public int
cmd_access(object ob, object for_obj, int cmd_attr)
{
    mixed acs;
    object env;

    if (!(env = environment(for_obj)))
    {
	return 1;
    }

    if (!(acs = env->block_action(query_verb(), ob, for_obj, cmd_attr)))
    {
	if (!(acs = ob->block_action(query_verb(), for_obj, cmd_attr)))
	{
	    return 1;
	}
    }

    if (stringp(acs))
    {
	parse_msg += acs;
	return 0;
    }

    return !!acs;
}

/*
 * Function name: check_block_action
 * Description:   Check a set of targets to see if the current command
 *                can be performed on each.
 * Arguments:     object *targets - the targets of the command
 *                int cmd_attr    - the command's attributes (from cmdparse.h)
 * Returns:       An array containing the targets for whom the command was
 *                not blocked.  If any were blocked, the global variable
 *                "parse_msg" will contain error messages (if any were
 *                supplied).
 */
public object *
check_block_action(object *targets, int cmd_attr)
{
    parse_msg = "";

    return filter(targets, &cmd_access(, this_player(), cmd_attr));
}

/*
 * Function name: parse_this_one
 * Description  : Called only internally by parse_this() to parse one segment
 *                of targets.
 * Arguments    : see parse_this(), except cmd_attr isn't passed along.
 * Returns      : see parse_this()
 */
public object *
parse_this_one(string str, string form, int allow_self)
{
    object *oblist;

    if (str == "enemy")
    {
	oblist = ({ this_player()->query_attack() });
	if (!objectp(oblist[0]) ||
	    !CAN_SEE(this_player(), oblist[0]))
	{
	    return ({ });
	}

	return oblist;
    }

    if (str == "team")
    {
	oblist = this_player()->query_team_others();
	oblist = FILTER_PRESENT(oblist);
	oblist = FILTER_CAN_SEE(oblist, this_player());

	return oblist;
    }

    /* Some emotes may be performed on the actor himself. */
    if (allow_self &&
	((str == "me") ||
	 (str == "myself") ||
	 (str == this_player()->query_real_name())))
    {
	return ({ this_player() });
    }

    /* No objects found matching 'form'. */
    if (!parse_command(str, environment(this_player()), form, oblist))
    {
	return ({ });
    }

    /* For the '%o' option which only returns a single object. */
    if (objectp(oblist))
    {
	oblist = ({ oblist });

	if (oblist[0] == this_player())
	    return ({ });
	else
	    return oblist;
    }

    /* Use NORMAL_ACCESS to for instance get the 'second dwarf'. Filter for
     * livings only if the argument is "[preoposition ]all".
     */
    oblist = NORMAL_ACCESS(oblist - ({ this_player() }), 0, 0) - ({ 0 });

    if ((str == "all") ||
	(str[-4..] == " all"))
    {
	oblist = FILTER_LIVE(oblist);
    }

    return oblist;
}

/*
 * Function name: parse_this_and
 * Description  : Called only internally by parse_this() to parse a segment
 *                of targets containing concatenation with comma's and "and".
 * Arguments    : see parse_this(), except cmd_attr isn't passed along.
 * Returns      : see parse_this()
 */
public object *
parse_this_and(string str, string form, int allow_self)
{
    object *oblist = ({ });
    string *parts;
    int index;
    int size;

    /* Replace the word "and" by a comma. */
    if (wildmatch("* and *", str))
    {
	str = implode(explode(str, " and "), ",");
    }

    parts = explode(str, ",");
    index = -1;
    size = sizeof(parts);
    while(++index < size)
    {
	while(parts[index][0] == ' ')
	{
	    parts[index] = parts[index][1..];
	}

	oblist += parse_this_one(parts[index], form, allow_self);
    }

    return oblist;
}

/*
 * Function name: parse_this
 * Description  : This is a parser with some extra functions and checks,
 *                specially designed for the soul. It can be used on both
 *                livings and non-livings, but if the first argument is "all",
 *                then only livings are returned.
 * Arguments    : string str - the string to parse.
 *                string form - the parse-pattern.
 *                int cmd_attr - the command attributes (optional).
 *                int allow_self - if true, allow the emote on this_player()
 *                    too. (optional)
 * Returns      : object * - an array of matching objects.
 */
public varargs object *
parse_this(string str, string form, int cmd_attr = 0, int allow_self = 0)
{
    object *oblist;
    string target, except;

    /* Sanity checks. Player must be able to see in the room. */
    if (!strlen(str) ||
	!CAN_SEE_IN_ROOM(this_player()))
    {
	return ({ });
    }

    str = lower_case(str);

    /* Replace the word "but" by the word "except". */
    if (wildmatch("* but *", str))
    {
	str = implode(explode(str, " but "), " except ");
    }

    if (sscanf(str, "%s except %s", target, except) == 2)
    {
	oblist = parse_this_and(target, form, allow_self) -
	    parse_this_and(except, form, allow_self);
    }
    else
    {
	oblist = parse_this_and(str, form, allow_self);
    }

    return check_block_action(oblist, cmd_attr);
}

/*
 * Function name: parse_live
 * Description  : This is a wrapper around the parse_this() routine. It does
 *                exactly the same and then filters the result for livings.
 *                It is especially meant for using the %o form.
 * Arguments    : see 'sman parse_this'
 * Returns      : see 'sman parse_this'
 */
public varargs object *
parse_live(string str, string form, int attr, int self)
{
    return filter(parse_this(str, form, attr, self), living);
}

/*
 * Function name: parse_adverb
 * Destription  : This function is designed to separate the adverb a player
 *                uses from the rest of the command line.
 * Arguments    : str     - the command line string. This may not be "", so
 *                          should either be 0 or a string containing at
 *                          least one character.
 *                def_adv - the default adverb for this emotion
 *                trail   - 1 if the adverb comes behind the target
 * Returns      : array of string: ({ cmd_str, adv_str })
 *                cmd_str - the rest of the line
 *                adv_str - the adverb
 *
 * Examples:
 *     parse_adverb("happ", "sadly", 0/1) returns
 *          ({ "", "happily" })
 *     parse_adverb("hupp", "sadly", 0/1) returns
 *          ({ "hupp", "sadly" })
 *     parse_adverb("happ at the dwarf wizard", "sadly", 0)
 *          ({ "at the dwarf wizard", "happily" })
 *     parse_adverb("Mercade", "sadly", 0)
 *          ({ "Mercade", "sadly" })
 *     parse_adverb("Mercade merri", "gracefully", 1)
 *          ({ "Mercade", "merrily" })
 *     parse_adverb("merri Mercade", "gracefully", 1)
 *          ({ "merri Mercade", "gracefully" })
 */
public string *
parse_adverb(string str, string def_adv, int trail)
{
    int    index;
    string *words, adverb;

    /* No command line argument, so just return the default adverb. */
    if (!strlen(str))
    {
	return ({ 0, def_adv });
    }

    words = explode(str, " ");

    /* Only one word. */
    if (sizeof(words) == 1)
    {
	/* If there is a living present in the room that can be called by
	 * the name 'str', we assume that the player tries to point at the
	 * player. So if you 'smile grace', you are more likely to smile
	 * at a darling player named Grace in the room rather than to
	 * smile gracefully in general.
	 */
	if (objectp(present(str, environment(this_player()))))
	{
	    return ({ str, def_adv });
	}

	/* Now we check whether the word passed is an adverb. If so, the
	 * player probably only wants to use the adverb on a general emotion.
	 */
	if (strlen(adverb = FULL_ADVERB(str)) ||
	  strlen(adverb = this_player()->full_adverb(str)))
	{
	    return ({ 0, adverb });
	}

	/* This case returns all other cases, ie, when a player uses 'all'
	 * to do something to all people, or uses a plural noun. That is
	 * obviously not triggered with the 'present' efun.
	 */
	return ({ str, def_adv });
    }

    /* Distinguish whether the adverb should follow the target or whether
     * it should precede it. This is emote-dependant.
     */
    index = (trail ? sizeof(words) - 1 : 0);

    if (strlen(adverb = FULL_ADVERB(words[index])) ||
	strlen(adverb = this_player()->full_adverb(words[index])))
    {
	return ({ implode(exclude_array(words, index, index), " "), adverb });
    }

    return ({ str, def_adv });
}

/*
 * Function name: parse_adverb_with_space
 * Description  : This function returns the adverb from parse_adverb with a
 *                preceding space if it is an adverb and it returns an empty
 *                string if the adverb was the special adverb that means that
 *                the player does not want an adverb.
 * Arguments    : see parse_adverb
 * Returns      : see parse_adverb, the description and /sys/adverbs.h
 */
public string *
parse_adverb_with_space(string str, string def_adv, int trail)
{
    string *pa = parse_adverb(str, def_adv, trail);
    return ({ pa[0], ADD_SPACE_TO_ADVERB(pa[1]) });
}

/*
 * Function name: check_adverb
 * Description  : Returns the full adverb the player meant. If he did not
 *                specify an an adverb, return the default adverb.
 * Arguments    : str     - the pattern to match
 *                def_adv - the default adverb
 * Returns      : string  - the full adverb or NO_ADVERB
 */
public string
check_adverb(string str, string def_adv)
{
    string adverb;

    if (!strlen(str))
    {
	return def_adv;
    }

    if (strlen(adverb = FULL_ADVERB(str)) ||
	strlen(adverb = this_player()->full_adverb(str)))
    {
	return adverb;
    }

    return NO_ADVERB;
}

/*
 * Function name: check_adverb_with_space
 * Description  : This function returns the adverb from check_adverb with a
 *                preceding space if it is an adverb and it returns an empty
 *                string if the adverb was the special adverb that means that
 *                the player does not want an adverb.
 * Arguments    : see check_adverb
 * Returns      : see check_adverb, the description and <adverbs.h>
 */
public string
check_adverb_with_space(string str, string def_adv)
{
    return ADD_SPACE_TO_ADVERB(check_adverb(str, def_adv));
}


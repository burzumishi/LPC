/*
 * /cmd/live/speech.c
 *
 * General commands related to speech and communication are in this soul.
 * The following commands are defined:
 *
 * - asay
 * - ask
 * - commune
 * - converse
 * - reply
 * - rsay
 * - say
 * - shout
 * - tell
 * - whisper
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

inherit "/cmd/std/command_driver";

#include <adverbs.h>
#include <cmdparse.h>
#include <composite.h>
#include <files.h>
#include <filter_funs.h>
#include <flags.h>
#include <language.h>
#include <macros.h>
#include <mail.h>
#include <options.h>
#include <ss_types.h>
#include <std.h>
#include <stdproperties.h>

#define LANGUAGE_ALL_RSAY    (55) /* When will you understand all rsay     */
#define LANGUAGE_MIN_RSAY    (15) /* Below this you understand no rsay     */
#define DEPTH                (1)  /* How many rooms away shout is heard.   */

/*
 * Prototype.
 */
varargs int say_text(string str, string adverb = "");
public int say_to(string str, function display_speech);

/*
 * Function name: create
 * Description  : This function is called the moment this object is created
 *                and loaded into memory.
 */
void
create()
{
    seteuid(getuid(this_object())); 
}

/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
string
get_soul_id()
{
    return "speech";
}

/* **************************************************************************
 * This is a command soul.
 */
int
query_cmd_soul()
{
    return 1;
}

/* **************************************************************************
 * The list of verbs and functions. Please add new in alfabetical order.
 */
mapping
query_cmdlist()
{
    return ([
             "asay":"asay",
             "ask":"ask",

             "commune":"commune",
             "converse":"converse",

             "reply":"reply",
             "rsay":"rsay",

             "say":"say_text",
             "shout":"shout",

             "tell":"tell",

             "whisper":"whisper",
           ]);
}

/*
 * Function name: using_soul
 * Description  : Called once by the living object using this soul. Adds
 *                sublocations responsible for extra descriptions of the
 *                living object.
 */
public void 
using_soul(object live)
{
}

/*
 * Function name: notify_speech
 * Description  : This function is used by the various speech methods to
 *                notify the other livings in the room about the speech.
 * Arguments    : string verb - the verb the player used.
 *                string adverb - the adverb of the speech with prefix
 *                    space or "" if no adverb.
 *                object *oblist - the targets of the speech, if any.
 *                string text - the text spoken.
 */
public void
notify_speech(string verb, string adverb, object *oblist, string text)
{
    int target = !!sizeof(oblist);
    int is_target;
    object *livings;

    if (!environment(this_player()))
        return;
    
    livings = FILTER_OTHER_LIVE(all_inventory(environment(this_player())));

    foreach(object npc: livings)
    {
        if (is_target = target)
        {
            is_target = (IN_ARRAY(npc, oblist) ? 1 : -1);
        }
        npc->speech_hook(verb, this_player(), adverb, oblist, text, is_target);
    }
}

/* **************************************************************************
 * Here follows the actual functions. Please add new functions in the 
 * same order as in the function name list.
 * **************************************************************************/

/* **************************************************************************
 * ASay - Say something using an adverb.
 */
int
asay(string str)
{
    string *how;

    if (!strlen(str))
    {
        notify_fail("Syntax: asay <adverb> <text>\n");
        return 0;
    }

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 0);
    if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        notify_fail("Cannot resolve \"" + explode(str, " ")[0] +
            "\" to an adverb.\n");
        return 0;
    }

    if (!strlen(how[0]))
    {
        notify_fail("Syntax: asay <adverb> <text>\n");
        return 0;
    }

    return say_text(how[0], how[1]);
}

/* **************************************************************************
 * Ask - Ask someone something.
 */
int
ask(string str)
{
    object *oblist, person;
    object *livings;
    object *wizards;
    mixed  tmp;
    int    index;
    string msg;
    string cap_msg;

    if (!CAN_SEE_IN_ROOM(this_player()))
    {
        notify_fail("You can't see in here.\n");
        return 0;
    }

    if (!stringp(str))
    {
        notify_fail("Ask whom what?\n");
        return 0;
    }

    cap_msg = str;
    if (parse_command(lower_case(str), environment(this_player()), "%l %s",
        oblist, msg))
    {
        oblist = NORMAL_ACCESS(oblist, 0, 0);
        msg = cap_msg[-strlen(msg)..];
    }

    switch(sizeof(oblist))
    {
    case 0:
        notify_fail("Who do you want to ask the question to?\n");
        return 0;

    case 1:
        person = oblist[0];
        break;

    default:
        notify_fail("Asking one person at a time will suffice.\n");
        return 0;
    }

    if (person == this_player())
    {
        notify_fail("Don't ask me, I wouldn't know.\n");
        return 0;
    }

    if (!strlen(msg))
    {
        write("Ask what to " + person->query_the_name(this_player()) + "?\n");
        return 1;
    }

    if (tmp = this_player()->query_prop(LIVE_M_MOUTH_BLOCKED))
    {
        write(stringp(tmp) ? tmp : "You are gagged and cannot ask.\n");
        return 1;
    }
 
    if (this_player()->query_option(OPT_ECHO))
        actor("You ask", oblist, ": " + msg);
    else
        write("Ok.\n");

    this_player()->reveal_me(1);
    all2act(" asks", oblist, " something.");

    /* Give the message to all wizards, too. */
    livings = FILTER_OTHER_LIVE(all_inventory(environment(this_player()))) - oblist;
    wizards = FILTER_IS_WIZARD(livings);
    wizards->catch_tell("As wizard, you hear " +
        this_player()->query_possessive() + " question: " + msg + "\n");
    livings -= wizards;

    target(" asks you: " + msg, oblist);
    person->catch_question(msg);
    person->reveal_me(1);

    /* Onlookers don't get the question that was asked. */
    person->speech_hook("ask", this_player(), "", oblist, msg, 1);
    livings->speech_hook("ask", this_player(), "", oblist, "", -1);

    return 1;
}

/* **************************************************************************
 * Commune - talk with the wizards.
 *
 * This is supposed to be used in extreme emergencies only.
 */
int
commune(string str)
{
    object *wizards;
    object wizard;
    int flag = 0;
    string *names;
    string cname;
    string message;
    string timestamp = ctime(time())[11..15] + " ";

    if (!query_interactive(this_player()))
    {
        notify_fail("Only true players may commune with the deities.\n");
        return 0;
    }

    if (this_player() != this_interactive())
    {
        tell_object(this_interactive(),
            "Communing is a decision the player must make alone.\n");
        notify_fail("Commune rejected as you must commune alone.\n");
        return 0;
    }

    if (this_player()->query_wiz_level())
    {
        notify_fail("Communing is something relevant only to mortals, " +
            "seek an audience.\n");
        return 0;
    }

    if (!stringp(str))
    {
        write("Please do 'help commune' to see how this rite is performed. " +
            "But Beware! Mortals will be stricken by the ultimate wrath of " +
            "the deities supreme if communing for insufficient reasons.\n");
        return 1;
    }

    if (wildmatch("to *", str))
    {
        str = str[3..];
    }

    names = explode(str, " ");
    if (sizeof(names) < 2)
    {
        notify_fail("Please do 'help commune' to see how this rite " +
            "is performed.\n");
        return 0;
    }
    
    cname = lower_case(names[0]);
    str = capitalize(this_interactive()->query_real_name());
    message = implode(names[1..], " ") + "\n";

    if (LANG_IS_OFFENSIVE(message))
    {
        write("Your message appears to contain language that is not fitting " +
            "for the ears of the deities. If you must, please formulate " +
            "your plight using different terms. Also, make sure to do 'help " +
            "commune' to read and heed the warning!\nIf, after having " +
            "received this warning, you still use abusive or offensive " +
            "language towards the wizards, you may find yourself deleted " +
            "without further delay or hesitation.\n");

        /* Log the commune message in a public log. */
        SECURITY->commune_log(("Offensive: " + cname + ": " + message), 1);
        return 1;
    }

    switch(cname)
    {
    case "all":
        wizards = filter(users(), &->query_wiz_level());
        foreach(object wizard: wizards)
        {
            if (!(wizard->query_prop(WIZARD_I_BUSY_LEVEL) & BUSY_C))
            {
                tell_object(wizard,
                    (wizard->query_option(OPT_TIMESTAMP) ? timestamp : "") +
                    "COMMUNE anyone from " + str + ": " + message);
                flag = 1;
            }
        }
        break;

    case "here":
        if (!environment(this_player()))
        {
            notify_fail("Here? There is no here?\n");
            return 0;
        }

        cname = explode(file_name(environment(this_player())), "/")[2];
        if (!sizeof(names = (string *)SECURITY->query_domain_members(cname)))
        {
            notify_fail("Sorry. You cannot commune 'here' from this room.\n");
            return 0;
        }

        foreach(string name: names)
        {
            wizard = find_player(name);
            if (objectp(wizard) &&
                !(wizard->query_prop(WIZARD_I_BUSY_LEVEL) & BUSY_C))
            {
                tell_object(wizard,
                    (wizard->query_option(OPT_TIMESTAMP) ? timestamp : "") +
                    "COMMUNE " + cname + " from " + str + ": " + message);
                flag = 1;
            }
        }
        break;

    default:
        /* Wizard team or domain. */
        if (sizeof(names = SECURITY->query_team_list(cname)) ||
            sizeof(names = SECURITY->query_domain_members(capitalize(cname))))
        {
            cname = capitalize(cname);
            foreach(string name: names)
            {
                if (!objectp(wizard = find_player(name)))
                {
                    continue;
                }

                if (!wizard->query_wiz_level() ||
                    (wizard->query_prop(WIZARD_I_BUSY_LEVEL) & BUSY_C))
                {
                    continue;
                }

                tell_object(wizard,
                    (wizard->query_option(OPT_TIMESTAMP) ? timestamp : "") +
                    "COMMUNE to " + cname + " from " + str +  ": " + message);

                if (!wizard->query_invis())
                {
                    flag = 1;
                }
            }

            break;
        }

        wizard = find_player(cname);
        cname = capitalize(cname);

        if (!objectp(wizard) || 
            !wizard->query_wiz_level() ||
            (wizard->query_prop(WIZARD_I_BUSY_LEVEL) & BUSY_C) ||
            wizard->query_prop(OBJ_I_INVIS) > 0)
        {
            break;
        }
        
        if (this_player()->query_mana() >=
            this_player()->query_max_mana() / 10)
        {
            this_player()->add_mana(-(this_player()->query_max_mana() / 10));
        }
        else
        {
            write("You feel far too exhausted to do that.\n");
            return 1;
        }

        tell_object(wizard,
            (wizard->query_option(OPT_TIMESTAMP) ? timestamp : "") +
            "COMMUNE to you from " + str + ": " + message);
        flag = 1;
        break;
    }

    /* Log the commune message in a public log. */
    SECURITY->commune_log(cname + ": " + message);

    if (flag)
        write("You feel you have communed with the deities.\n");
    else
        write("Your prayers remain unheard.\n");
    
    return 1;
}

/* **************************************************************************
 * Converse - Carry on a conversation without having to type 'say'.
 */

/*
 * Function name: converse_more
 * Description  : Until the conversation is over, prompt the player for more
 *                text to say.
 * Arguments    : string str - the text the player wants to say.
 */
nomask void
converse_more(string str)
{
    if ((str == "**") ||
        (str == "~q"))
    {
        write("Left conversation mode.\n");
        return;
    }

    /* We can not allow any handwritten VBFC */
    while(wildmatch("*@@*", str)) 
    {
        str = implode(explode(str, "@@"), "#");
    }
 
    this_player()->set_say_string(str);
    say(QCTNAME(this_player()) + " @@race_sound:" + file_name(this_player()) +
        "@@: " + str + "\n");

    write("]");
    input_to(converse_more);
}

int
converse()
{
    write("Entering conversation mode.\nGive '**' or '~q' to stop.\n");
    write("]");
    input_to(converse_more);
    return 1;
}

/* **************************************************************************
 * reply - Allow mortals to reply when something is told to them.
 */
int
reply(string str)
{
    string *names;
    string who;
    object target;
    string timestamp = ctime(time())[11..15] + " ";

    /* Access failure. No command line argument. */
    if (!stringp(str))
    {
        notify_fail("Reply what [to whom]?\n");
        return 0;
    }

    /* Wizard may block mortals from replying again. */
    if (this_player()->query_wiz_level() &&
        wildmatch("stop *", str))
    {
        sscanf(lower_case(str), "stop %s", str);

        if (!objectp(target = find_player(str)))
        {
            notify_fail("No player '" + capitalize(str) + "' logged in.\n");
            return 0;
        }

        names = target->query_prop(PLAYER_AS_REPLY_WIZARD);
        who = this_player()->query_real_name();
        if (!pointerp(names) ||
            (member_array(who, names) == -1))
        {
            write("Player '" + capitalize(str) +
                  "' is not able to reply to you.\n");
            return 1;
        }

        names -= ({ who });
        if (sizeof(names))
        {
            target->add_prop(PLAYER_AS_REPLY_WIZARD, names);
        }
        else
        {
            target->remove_prop(PLAYER_AS_REPLY_WIZARD);
        }

        write("Removed your name from " + capitalize(str) +
            "'s reply list.\n");
        return 1;
    }

    /* See if any wizard has told anything to this player. */
    names = this_player()->query_prop(PLAYER_AS_REPLY_WIZARD);
    if (!pointerp(names) ||
        !sizeof(names))
    {
        notify_fail("You can only reply if someone told you something.\n");
        return 0;
    }

    /* If the mortal wants to reply to someone in particular, see get the
     * name and see whether that is possible, else we take the first wizard
     * from the list.
     */
    if (wildmatch("to *", str) &&
        (sscanf(str, "to %s %s", who, str) == 2))
    {
        who = lower_case(who);

        if (member_array(who, names) == -1)
        {
            write("You cannot reply to " + capitalize(who) +
                  " since that wizard has not spoken to you.\n");
            return 1;
        }
    }
    else
    {
        who = explode(lower_case(str), " ")[0];
        if (member_array(who, names) >= 0)
        {
            notify_fail("Use \"reply to <wizard> <message>\" to reply to a " +
                "specific wizard, like " + capitalize(who) + ".\n");
            return 0;
        }

        who = names[0];
    }

    /* Wizard is no longer logged in. */
    if (!objectp(target = find_player(who)))
    {
        write("You cannot reply, that person is no longer in the game.\n");
        return 1;
    }

    /* No point in replying to someone who is linkdead. */
    if (!interactive(target))
    {
        write(target->query_The_name(this_player()) +
            " is link dead at the moment, so you cannot reply to " +
            target->query_objective() + ".\n");
        return 1;
    }

    if (!this_player()->query_wiz_level() &&
        (target->query_prop(WIZARD_I_BUSY_LEVEL) & BUSY_M))
    {
        write(target->query_The_name() +
            " is not receptive for replies right now.\n");
        return 1;
    }

    if (target->query_wiz_level())
    {
        target->catch_tell((target->query_option(OPT_TIMESTAMP) ? timestamp : "") +
            capitalize(this_player()->query_real_name()) + " replies: " +
            str + "\n");
    }
    else
    {
        target->catch_tell(this_player()->query_The_name(target) +
            " replies: " + str + "\n");
    }
    if (this_player()->query_option(OPT_ECHO))
    {
        write("You replied to " + target->query_the_name(this_player()) +
            ": " + str + "\n");
    }
    else
    {
        write("You replied to " + target->query_the_name(this_player()) +
            ".\n");
    }
    return 1;
}

/* **************************************************************************
 * rsay - say something in your racial tongue.
 */

/*
 * Function name: race_text
 * Description  : Support routine for rsay to modify the spoken text to the
 *                recipients. It depends on your language skill how much
 *                you actually understand.
 * Arguments    : string race - the race of the person who speaks.
 *                string text - the spoken words.
 * Returns      : string - the (il)legible text.
 */
string
race_text(string race, string text)
{
    object player = previous_object(-1);
    int skill = player->query_skill(SS_LANGUAGE);
    string *words, to_print;
    int sentence_index, sentence_size;

    /* Wizards, players of the same race and people with a generally
     * high education in languages will understand the racial speech.
     */
    if (player->query_wiz_level() ||
        race == player->query_race_name() ||
        skill >= LANGUAGE_ALL_RSAY)
    {
        return text;
    }
    
    /* Other players will only hear a part of the text. */
    skill -= LANGUAGE_MIN_RSAY;
    to_print = "";

    words = explode(text, " ");
    sentence_index = -1;
    sentence_size = sizeof(words);
    
    while(++sentence_index < sentence_size)
    {
        if (strlen(to_print))
            to_print += " ";
        
        if (skill > 0 &&
            random(LANGUAGE_ALL_RSAY - LANGUAGE_MIN_RSAY) <= skill)
        {
            to_print += words[sentence_index];
        }
        else
        {
            to_print += extract("....................", 1,
                strlen(words[sentence_index]));
        }
    }
    
    return to_print;
}

/*
 * Function name: print_rsay_to
 * Description  : When the player uses "rsay to" this routine causes the text
 *                to be displayed after the "to" part was parsed.
 * Arguments    : object *oblist - the recipients of the text.
 *                string str - the text that was spoken.
 */
void
print_rsay_to(object *oblist, string str)
{
    string output;
    string comp_live;

    /* Store this variable for later use in QCOMPLIVE. */
    comp_live = COMPOSITE_ALL_LIVE(oblist);

    if (this_player()->query_option(OPT_ECHO))
        write("You say to " + comp_live + " in your own tongue: " + str + "\n");
    else
        write("Ok.\n");

    /* How much of this text is seen depends on the language skill */
    output = "@@race_text:" + file_name(this_object()) + "|" +
        this_player()->query_race_name() + "|" + str + "@@";

    say(QCTNAME(this_player()) + " says to " + QCOMPLIVE + " in " +
        this_player()->query_possessive() + " own tongue: " + output + "\n",
        (oblist + ({ this_player() }) ));
    oblist->catch_msg(QCTNAME(this_player()) + " says to you in " +
        this_player()->query_possessive() + " own tongue: " + output + "\n");

    notify_speech("rsay", "", oblist, str);
}

/*
 * Function name: remote_rsay_to
 * Description  : Relay function from rsay redefinition to allow for the
 *                functionpointer.
 * Arguments    : string str - "[the] <target> <modified text>"
 * Returns      : int 1/0 - success/failure.
 */
int
remote_rsay_to(string str)
{
    if (say_to(str, &print_rsay_to()))
    {
        return 1;
    }
    return 0;
}

int
rsay(string str)
{
    int     index;
    int     size;
    mixed   tmp;
    object *oblist;
    string  race = this_player()->query_race_name();
    string  pos = this_player()->query_possessive();
    int     skill;
    string  *words;
    int     sentence_size;
    int     sentence_index;
    string  to_print;

    if (!objectp(environment(this_player())))
    {
        return 0;
    }

    if (!stringp(str))
    {
        notify_fail("Say what in your racial language?\n");
        return 0;
    }
    
    if (tmp = this_player()->query_prop(LIVE_M_MOUTH_BLOCKED))
    {
        write(stringp(tmp) ? tmp : "You are gagged and cannot speak.\n");
        return 1;
    }

    if (wildmatch("to *", lower_case(str)))
    {
        if (say_to(extract(str, 3), &print_rsay_to()))
        {
            return 1;
        }
    }

    if (this_player()->query_option(OPT_ECHO))
        write("You say in your own tongue: " + str + "\n");
    else
        write("Ok.\n");

    say(QCTNAME(this_player()) + " says in " +
        this_player()->query_possessive() +
        " own tongue: @@race_text:" + file_name(this_object()) + "|" +
        this_player()->query_race_name() + "|" + str + "@@\n");
    notify_speech("rsay", "", ({ }), str);
    return 1;
}

/*
 * Function name: say_to
 * Description  : This function is called whenever the player starts his
 *                speech with 'to'. This usually indicates that the player
 *                wants to say something to some people in particular.
 * Arguments    : string str - the text to say (not including 'to').
 *                function display_speech - the routine to be called to
 *                    display the speech.
 * Returns      : int 1/0 - success/failure.
 */
public int
say_to(string str, function display_speech)
{
    object *oblist;
    string r_sound;
    string qcomp;
    string say_string;

    /* We must parse the lower case of the string 'str' since parse_command
     * does not find capitalized player names, so it would not trigger on
     * "say to Mercade Greetings!" However, since we want to keep the
     * capitals in the said text, we store the original text in the variable
     * 'say_string' and use that later.
     */
    say_string = str;

    /* Whisper to all people. */
    if (wildmatch("all *", str))
    {
        str = extract(str, 4);
        oblist = FILTER_OTHER_LIVE(all_inventory(environment(this_player())));
    }
    /* Whisper to my team. */
    else if (wildmatch("team *", str))
    {
        str = extract(str, 5);
        oblist = this_player()->query_team_others() &
            all_inventory(environment(this_player()));
    }
    /* Find out who we talk to. */
    else if (!parse_command(lower_case(str), environment(this_player()),
        "[to] [the] %i %s", oblist, str))
    {
        notify_fail("Say [how] what to whom/what?\n");
        return 0;
    }
    else
    {
        oblist = NORMAL_ACCESS(oblist, 0, 0) - ({ this_player() });
    }

    if (!sizeof(oblist) ||
        !strlen(str))
    {
        return 0;
    }

    /* Get the original say-string with capitals. */
    say_string = extract(say_string, -(strlen(str)));
    this_player()->set_say_string(say_string);
    
    display_speech(oblist, say_string);
    return 1;
}

/* **************************************************************************
 * say_text - say something to another player.
 *
 * This function is not called say() because of the simul-efun by that name.
 */

/*
 * Function name: print_say_to
 * Description  : When the player uses "[a]say to" this routine causes the
 *                text to be displayed after the "to" part was parsed.
 * Arguments    : string adverb - the adverb used with asay, if any.
 *                object *oblist - the recipients of the text.
 *                string str - the text that was spoken.
 */
void
print_say_to(string adverb, object *oblist, string str)
{
    string r_sound;
    string comp_live;

    /* Store this variable for later use in QCOMPLIVE. */
    comp_live = COMPOSITE_ALL_LIVE(oblist);

    if (this_player()->query_option(OPT_ECHO))
        write("You" + adverb + " " + this_player()->actor_race_sound() +
            " to " + comp_live + ": " + str + "\n");
    else
        write("Ok.\n");

    r_sound = (" @@race_sound:" + file_name(this_player()) + "@@ to ");
    say(QCTNAME(this_player()) + adverb + r_sound + QCOMPLIVE + ": " +
        str + "\n", (oblist + ({ this_player() }) ));
    oblist->catch_msg(QCTNAME(this_player()) + adverb + r_sound + "you: " +
        str + "\n");

    notify_speech("say", adverb, oblist, str);
}

varargs int
say_text(string str, string adverb = "")
{
    mixed tmp;

    if (!strlen(str))
    {
        notify_fail("What do you wish to say?\n");
        return 0;
    }

    if (tmp = this_player()->query_prop(LIVE_M_MOUTH_BLOCKED))
    {
        write(stringp(tmp) ? tmp : "You are gagged and cannot speak.\n");
        return 1;
    }

    /* We do not want people to add too many spaces and use the say command
     * as a way to generate emotions themselves. However, we do not want to
     * waste this on wizards and we also test whether people haven't used
     * too many spaces. You cannot make an emotion with only a few. This
     * wildmatch is 40% faster than the explode/implode stuff, so as long
     * as people don't use 8 spaces more than 40% of the time, this check
     * pays itself back.
     */
    if (!this_player()->query_wiz_level() &&
        wildmatch("*       *", str))
    {
        str = implode((explode(str, " ") - ({ "" }) ), " ");
    }

    /* This is a test for the command 'say to'. If it fails, we just default
     * to the normal say.
     */
    if (wildmatch("to *", lower_case(str)))
    {
        if (say_to(extract(str, 3), &print_say_to(adverb)))
        {
            return 1;
        }
    }

    this_player()->set_say_string(str);
    if (this_player()->query_option(OPT_ECHO))
    {
        write("You" + adverb + " " + this_player()->actor_race_sound() +
            ": " + str + "\n");
    }
    else
    {
        write("Ok.\n");
    }

    say(QCTNAME(this_player()) + adverb + " @@race_sound:" +
        file_name(this_player()) + "@@: " + str + "\n");
    notify_speech("say", adverb, ({ }), str);        
    return 1;
}

/* **************************************************************************
 * Shout - shout something.
 */

/*
 * Function name: shout_name
 * Description  : Called through VBFC to find the name/description of the
 *                person who does the shouting.
 * Returns      : string - the name/description of the living.
 */
string
shout_name()
{
    object pobj = previous_object(); /* Reciever of message */
    if (file_name(pobj) == VBFC_OBJECT)
    {
        pobj = previous_object(-1);
    }
    if (pobj->query_met(this_player()))
    {
        return this_player()->query_name();
    }
    return capitalize(LANG_ADDART(this_player()->query_gender_string())) +
        " " + this_player()->query_race_name() + " voice";
}

int
shout(string str)
{
    object *rooms;
    object troom;
    object *oblist;
    string *how;
    string cap_str;
    mixed  tmp;
    int    use_target = 0;
    int    index;
    int    size;
    string preposition;

    if (!strlen(str))
    {
        notify_fail("Shout what?\n", 0);
        return 0;
    }

    if (tmp = this_player()->query_prop(LIVE_M_MOUTH_BLOCKED))
    {
        write(stringp(tmp) ? tmp : "You are gagged and cannot shout.\n");
        return 1;
    }
 
    if ((strlen(str) > 60) &&
        (!this_player()->query_wiz_level()) &&
        (!this_player()->query_npc()))
    {
        notify_fail("Even your mouth is not big enough to shout all that.\n");
        return 0;
    }

    /* Note that [at][to] in a beautiful way tests both 'at' and 'to', while
     * wildmatch normally tests per letter, and not per word! */   
    if (wildmatch("[at][to] *", str))
    {
        preposition = extract(str, 0, 1);
        /* Shout at all people. */
        /* We already tested for at/to, so no repeat check necessary. */
        if (wildmatch("?? all *", str))
        {
            str = extract(str, 7);
            oblist =
                FILTER_OTHER_LIVE(all_inventory(environment(this_player())));
        }
        /* Shout to my team. */
        else if (wildmatch("?? team *", str))
        {
            str = extract(str, 8);
            oblist = this_player()->query_team_others() &
                all_inventory(environment(this_player()));
        }
        /* Find out who we shout to. */
        else if (parse_command(lower_case(cap_str = str),
            environment(this_player()), "[at] [to] [the] %i %s", oblist, str))
        {
            str = extract(cap_str, -(strlen(str)));
            oblist = NORMAL_ACCESS(oblist, 0, 0) - ({ this_player() });
        }
    }

    if (pointerp(oblist) && !sizeof(oblist))
    {
        notify_fail("Shout [what] at/to whom?\n");
        return 0;
    }

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 0);
    if (strlen(how[0]) &&
        how[1] != NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        str = how[0];
    }
    else
    {
        how[1] = NO_ADVERB;
    }

    /* Sanity check. */
    if (!(troom = environment(this_player())))
    {
        return 0;
    }

    /* For shouting, we don't want to find our own room. */
    rooms = (object *)SOUL_CMD->find_neighbour( ({ }), ({ troom }), DEPTH) - ({ troom });
    foreach(object room: rooms)
    {
        tell_room(room, "@@shout_name:" + file_name(this_object()) +
            "@@" + how[1] + " shouts: " + str + "\n", this_player());
    }

    if (sizeof(oblist))
    {
        if (this_player()->query_option(OPT_ECHO))
            actor("You" + how[1] + " shout " + preposition, oblist, ": " + str);
        else
            write("Ok.\n");

        all2act(how[1] + " shouts " + preposition, oblist, ": " + str);
        target(how[1] + " shouts " + preposition + " you: " + str, oblist);
        notify_speech("shout", how[1], oblist, str);
    }
    else
    {
        if (this_player()->query_option(OPT_ECHO))
            write("You" + how[1] + " shout: " + str + "\n");
        else
            write("Ok.\n");

        all(how[1] + " shouts: " + str);
    }

    notify_speech("shout", how[1], oblist, str);
    return 1;
}

/* **************************************************************************
 * Tell - tell something to someone over a distance.
 */
int
tell(string str)
{
    object ob;
    string who;
    string msg;
    int busy;

    /* For wizards, use the wizard "tell", and not this one. */
    if (this_player()->query_wiz_level())
    {
        return 0;
    }

    if (!(ARMAGEDDON->shutdown_active()))
    {
        notify_fail("This command is only valid when Armageddon is active in " +
            "the realms.\n");
        return 0;
    }

    if (!strlen(str) ||
	(sscanf(str, "%s %s", who, msg) != 2))
    {
	notify_fail("Tell who what?\n");
	return 0;
    }

    who = lower_case(who);
    if (who == "armageddon")
    {
	(ARMAGEDDON)->teleledningsanka();
	ob = find_object(ARMAGEDDON);
    }
    else
    {
	ob = find_player(who);
    }

    if (!objectp(ob))
    {
	notify_fail("No player named " + capitalize(who) + " present.\n");
	return 0;
    }

    if (!this_player()->query_met(ob))
    {
	notify_fail("There is no player called " + capitalize(who) +
	    " that you recall to have met.\n");
	return 0;
    }

    if (ob->query_wiz_level())
    {
	write("If you desparately need to speak with " +
	    capitalize(SECURITY->query_wiz_pretitle(ob)) + " " +
	    capitalize(who) + ", seek an audience with the 'commune' " +
	    "command. Make sure you read the help page first.\n");
	return 1;
    }

    if ((who != "armageddon") && (!query_interactive(ob)))
    {
	write(capitalize(who) + " is link dead right now.\n");
	return 1;
    }

    if (objectp(this_player()) &&
        (environment(ob) == environment(this_player())))
    {
        write("There is no need to tell because you are in the same room.\n");
        return 1;
    }

    busy = ob->query_prop(WIZARD_I_BUSY_LEVEL);

    if (busy & (BUSY_P|BUSY_S|BUSY_F))
    {
	write(capitalize(who) + " seems to be busy at the moment.\n");
	return 1;
    }

    tell_object(ob, this_player()->query_Art_name(ob) + " tells you: " +
	msg + "\n");

    if (this_player()->query_option(OPT_ECHO))
    {
	tell_object(this_player(), "You tell " + capitalize(who) + ": " +
	    msg + "\n");
    }
    else
    {
        write("Ok.\n");
    }

    return 1;
}

/* **************************************************************************
 * Whisper - whisper something to someone.
 */

/*
 * Function name: print_whisper_to
 * Description  : When the player uses "whisper to" this routine causes the
 *                text to be displayed after the "to" part was parsed.
 * Arguments    : string adverb - the adverb used, if any.
 *                object *oblist - the recipients of the text.
 *                string str - the text that was spoken.
 */
void
print_whisper_to(string adverb, object *oblist, string str)
{
    object *livings;
    object *wizards;
    
    if (this_player()->query_option(OPT_ECHO))
        actor("You whisper" + adverb + " to", oblist, ": " + str);
    else
        write("Ok.\n");

    all2act(adverb + " whispers something to", oblist);

    /* Give the message to all wizards, too. */
    livings = FILTER_OTHER_LIVE(all_inventory(environment(this_player()))) - oblist;
    wizards = FILTER_IS_WIZARD(livings);
    wizards->catch_tell("As wizard, you hear " +
        this_player()->query_objective() + " whisper: " + str + "\n");
    livings -= wizards;

    target(adverb + " whispers in your ear: " + str, oblist);
    oblist->catch_whisper(str);   

    /* Onlookers don't get what was being whispered. */
    livings->speech_hook("whisper", this_player(), adverb, oblist, "", -1);
    oblist->speech_hook("whisper", this_player(), adverb, oblist, str, 1);
}

int
whisper(string str)
{
    mixed tmp;
    string *how;

    if (!stringp(str))
    {
        notify_fail("Whisper [to] <whom> <what>?\n");
        return 0;
    }

    if (tmp = this_player()->query_prop(LIVE_M_MOUTH_BLOCKED))
    {
        write(stringp(tmp) ? tmp : "You are gagged and cannot whisper.\n");
        return 1;
    }

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 0);
    if (strlen(how[0]) && how[1] != NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        str = how[0];
    }
    else
    {
        how[1] = NO_ADVERB;
    }

    if (strlen(str))
    {
        if (wildmatch("to *", lower_case(str)))
            str = extract(str, 3);
        
        if (say_to(str, &print_whisper_to(how[1])))
        {
            return 1;
        }
    }
    
    notify_fail("Whisper [to] <whom> <what>?\n");
    return 0;
}

/*
 * /cmd/std/soul_cmd.c
 *
 * It is the basic soul that is meant to be inherited by the race-specific
 * souls.
 *
 * This soul contains the basic emotions for players. You can use it as an
 * example for your guild-soul if you want to add more emotions to the
 * members as well. If you are going to make any changes or additions to the
 * soul, please use one of the four basic formats here. Doing so will add
 * to the readability of the soul.
 *
 * NOTE
 * The order in which the message are printed to the players _must_ always
 * the 1) the actor, 2) the onlookers and 3) the target. This way it is
 * always guaranteed that the other people have received the messages if
 * the target should react instantly.
 */

#pragma no_clone
#pragma strict_types
#pragma save_binary

inherit "/cmd/std/command_driver";

#include <adverbs.h>
#include <cmdparse.h>
#include <composite.h>
#include <const.h>
#include <files.h>
#include <filter_funs.h>
#include <language.h>
#include <living_desc.h>
#include <macros.h>
#include <ss_types.h>
#include <stdproperties.h>
#include <options.h>

#define SUBLOC_SOULEXTRADESC ("_soul_cmd_extra")
#define SOULDESC(x)          (this_player()->add_prop(LIVE_S_SOULEXTRA, (x)))
#define DEPTH                (1)  /* How many rooms away scream is heard. */
#define DUMP_EMOTIONS_OUT    ("/open/dump_emotions")

/* **************************************************************************
 * Return a proper name of the soul in order to get a nice printout.
 */
string
get_soul_id()
{
    return "emotions";
}

/* **************************************************************************
 * The list of verbs and functions. Please add new in alphabetical order.
 */
mapping
query_cmdlist()
{
    return ([
             "ack":"ack",
             "admire":"admire",
             "agree":"agree",
             "apologize":"apologize",
             "applaud":"applaud",
             "avert":"avert",

             "back":"back",
             "bat":"bat",
             "beam":"beam",
             "beckon":"beckon",
             "beg":"beg",
             "bite":"bite",
             "blanch":"blanch",
             "blank":"blank",
             "blink":"blink",
             "blow":"blow",
             "blush":"blush",
             "boggle":"boggle",
             "bounce":"bounce",
             "bow":"bow",
             "brighten":"brighten",
             "brood":"brood",
             "burp":"burp",

             "cackle":"cackle",
             "caress":"caress",
             "cheer":"cheer",
             "choke":"choke",
             "chortle":"chortle",
             "chuckle":"chuckle",
             "clap":"clap",
             "clear":"clear",
             "comfort":"comfort",
             "complain":"complain",
             "compliment":"compliment",
             "conf":"confused",        /* A shortcut for lazy people */
             "confuse":"confuse",
             "confused":"confused",
             "congrat":"congratulate", /* A shortcut for lazy people */
             "congratulate":"congratulate",
             "cough":"cough",
             "cower":"cower",
             "cringe":"cringe",
             "cross":"cross",
             "cry":"cry",
             "cuddle":"cuddle",
             "curl":"curl",
             "curse":"curse",
             "curtsey":"curtsey",

             "dance":"dance",
             "despair":"despair",
             "disagree":"disagree",
             "drool":"drool",
             "duh":"duh",

             "eeks":"eeks",
             "excuse":"excuse",
             "explode":"explodes",
             "eyebrow":"eyebrow",

             "fart":"fart",
             "fawn":"fawn",
             "feign":"feign",
             "ffinger":"ffinger",
             "fidget":"fidget",
             "finger":"finger",
             "flex":"flex",
             "flip":"flip",
             "flirt":"flirt",
             "fondle":"fondle",
             "forgive":"forgive",
             "french":"french",
             "fret":"fret",
             "frown":"frown",
             "fume":"fume",

             "gag":"gag",
             "gasp":"gasp",
             "gesture":"gesture",
             "giggle":"giggle",
             "glare":"glare",
             "greet":"greet",
             "grimace":"grimace",
             "grin":"grin",
             "groan":"groan",
             "grope":"grope",
             "grovel":"grovel",
             "growl":"growl",
             "grumble":"grumble",

             "hang":"hang",
             "hiccup":"hiccup",
             "hmm":"hmm",
             "hold":"hold",
             "hug":"hug",
             "hum":"hum",

             "ignore":"ignore",

             "jump":"jump",

             "kick":"kick",
             "kiss":"kiss",
             "knee":"knee",
             "kneel":"kneel",

             "laugh":"laugh",
             "leer":"leer",
             "lick":"lick",
             "listen":"listen",
             "love":"love",

             "melt":"melt",
             "moan":"moan",
             "mourn":"mourn",
             "mumble":"mumble",

             "nibble":"nibble",
             "nod":"nod",
             "nudge":"nudge",
             "nuzzle":"nuzzle",

             "oops":"oops",
             "ouch":"ouch",

             "pace":"pace",
             "panic":"panic",
             "pant":"pant",
             "pat":"pat",
             "peer":"peer",
             "pet":"pet",
             "pinch":"pinch",
             "point":"point",
             "poke":"poke",
             "ponder":"ponder",
             "pounce":"pounce",
             "pout":"pout",
             "puke":"puke",
             "purr":"purr",

             "rolleyes":"rolleyes",
             "roar":"roar",
             "ruffle":"ruffle",

             "scold":"scold",
             "scowl":"scowl",
             "scratch":"scratch",
             "scream":"scream",
             "shake":"shake",
             "shiver":"shiver",
             "show":"show",
             "shrug":"shrug",
             "shudder":"shudder",
             "sigh":"sigh",
             "sing":"sing",
             "slap":"slap",
             "smell":"smell",
             "smile":"smile",
             "smirk":"smirk",
             "snap":"snap",
             "snarl":"snarl",
             "sneer":"sneer",
             "sneeze":"sneeze",
             "snicker":"snicker",
             "sniff":"sniff",
             "snore":"snore",
             "snuggle":"snuggle",
             "sob":"sob",
             "spank":"spank",
             "spit":"spit",
             "squeeze":"squeeze",
             "squirm":"squirm",
             "stare":"stare",
             "startle":"startle",
             "steam":"steam",
             "stick":"stick",
             "stomp":"stomp",
             "strangle":"strangle",
             "stretch":"stretch",
             "strut":"strut",
             "stumble":"stumble",
             "sulk":"sulk",
             "swallow":"swallow",
             "swear":"swear",
             "sweat":"sweat",
             "swoon":"swoon",

             "tackle":"tackle",
             "tap":"tap",
             "tease":"tease",
             "thank":"thank",
             "think":"think",
             "threaten":"threaten",
             "thumb":"thumb",
             "tickle":"tickle",
             "tingle":"tingle",
	     "touch":"touch",
             "tremble":"tremble",
             "trust":"trust",
             "twiddle":"twiddle",
             "twinkle":"twinkle",
             "twitch":"twitch",

             "wail":"wail",
             "wait":"wait",
             "wave":"wave",
             "weep":"weep",
             "whine":"whine",
             "whimper":"whimper",
             "whistle":"whistle",
             "wiggle":"wiggle",
             "wince":"wince",
             "wink":"wink",
	     "wonder":"wonder",
             "worry":"worry",
             "worship":"worship",
             "wring":"wring",

             "yawn":"yawn",
             "yodel":"yodel"
         ]);
}

/*
 * Function name: using_soul
 * Description  : Called once by the living object using this soul. Adds
 *                sublocations responsible for extra descriptions of the
 *                living object.
 * Arguments    : object live - the living using the soul.
 */
public void
using_soul(object live)
{
    live->add_subloc(SUBLOC_SOULEXTRADESC, file_name(this_object()));
}

/*
 * Function name: show_subloc
 * Description  : Shows the specific sublocation description for a living.
 * Arguments    : string subloc  - the subloc to display.
 *                object on      - the object to which the subloc is linked.
 *                object for_obj - the object that wants to see the subloc.
 * Returns      : string - the subloc description.
 */
public string
show_subloc(string subloc, object on, object for_obj)
{
    if ((subloc != SUBLOC_SOULEXTRADESC) ||
        on->query_prop(TEMP_SUBLOC_SHOW_ONLY_THINGS) ||
        (!strlen(subloc = on->query_prop(LIVE_S_SOULEXTRA))))
    {
        return "";
    }

    return (((for_obj == on) ? "You are " :
        (capitalize(on->query_pronoun()) + " is ")) + subloc + ".\n");
}

/*
 * Function name: query_cmd_soul
 * Description  : This is a command soul. This defines it as such.
 * Returns      : int 1 - always.
 */
public int
query_cmd_soul()
{
    return 1;
}

/*
 * Function name: dump_emotions
 * Description  : This function can be used to dump all emotions to a file
 *                in a sorted and formatted way. This dump can be used for
 *                the 'help emotions' document. The output of this function
 *                will be written to the file DUMP_EMOTIONS_OUT.
 * Returns      : int 1 - always.
 */
public nomask int
dump_emotions()
{
    int index = -1;
    int size = strlen(ALPHABET);
    string *words;

    setuid();
    seteuid(getuid());
    catch(rm(DUMP_EMOTIONS_OUT));
    while(++index < size)
    {
        words = filter(m_indices(query_cmdlist()),
            &wildmatch((ALPHABET[index..index] + "*")));

        if (!sizeof(words))
        {
            continue;
        }

        if (strlen(words[0]) < 12)
        {
            words[0] = (words[0] + "                ")[..11];
        }
        write_file(DUMP_EMOTIONS_OUT,
            sprintf("%-76#s\n\n", implode(sort_array(words), "\n")));
    }

    return 1;
}

/*
 * Function name: find_neighbour
 * Description  : This function will recursively search through the
 *                neighbouring rooms to a particular room to find the
 *                rooms a shout or scream will be heard in.
 * Arguments    : object *found  - the rooms already found.
 *                object *search - the rooms still to search.
 *                int    depth   - the depth still to search.
 * Returns      : object * - the neighbouring rooms.
 */
object *
find_neighbour(object *found, object *search, int depth)
{
    int index;
    int size;
    int index2;
    int size2;
    mixed *exit_arr;
    object *new_search, *rooms, troom, *doors;

    if (!depth)
    {
        return found;
    }

    rooms = found;
    new_search = ({ });

    index = -1;
    size = sizeof(search);
    while(++index < size)
    {
        exit_arr = (mixed *)search[index]->query_exit();

        index2 = -3;
        size2 = sizeof(exit_arr);
        while((index2 += 3) < size2)
        {
            if (functionp(exit_arr[index2]))
                continue;
            if (objectp(exit_arr[index2]))
                troom = exit_arr[index2];
            else
                troom = find_object(exit_arr[index2]);
            if (objectp(troom) &&
                (member_array(troom, rooms) < 0))
            {
                rooms += ({ troom });
                new_search += ({ troom });
            }
        }

        doors = search[index]->query_prop(ROOM_AO_DOOROB);

        index2 = -1;
        size2 = sizeof(doors);
        while (++index2 < size2)
        {
            if (objectp(troom = find_object(doors[index2]->query_other_room())) &&
                member_array(troom, rooms) < 0)
            {
                rooms += ({ troom });
            }
        }
    }

    return find_neighbour(rooms, new_search, depth - 1);
}

/* **************************************************************************
 * Here follows the actual functions. Please add new in alphabetical order.
 * **************************************************************************/

int
ack(string str)
{
    if (stringp(str))
    {
        notify_fail("Ack what?\n");
        return 0;
    }

    write("Ack!\n");
    all(" goes ack!", "", ACTION_AURAL);
    return 1;
}

int
admire(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %l", ACTION_INGRATIATORY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Admire whom?\n");
        return 0;
    }

    actor("You show", oblist, " your admiration.");
    all2act(" shows", oblist, " " + this_player()->query_possessive() +
        " admiration.", "", ACTION_INGRATIATORY);
    target(" shows you " + this_player()->query_possessive() + " admiration.",
        oblist, "", ACTION_INGRATIATORY);
    return 1;
}

int
agree(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "wholeheartedly", 0);

    if (!stringp(how[0]))
    {
        write("You agree" + how[1] + ".\n");
        allbb(" agrees" + how[1] + ".", how[1], ACTION_OTHER);
        return 1;
    }

    oblist = parse_this(how[0], "[with] [the] %l", ACTION_OTHER);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Agree [how] with whom?\n");
        return 0;
    }

    actor("You agree" + how[1] + " with", oblist);
    all2actbb(" agrees" + how[1] + " with", oblist, 0, how[1], ACTION_OTHER);
    targetbb(" agrees" + how[1] + " with you.", oblist, how[1], ACTION_OTHER);
    return 1;
}

int
apologize(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "remorsefully", 0);

    if (!stringp(how[0]))
    {
        write("You apologize" + how[1] + ".\n");
        all(" apologizes" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[to] [the] %l", ACTION_OTHER);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Apologize [how] to whom?\n");
        return 0;
    }

    actor("You apologize" + how[1] + " to" , oblist);
    all2act(" apologizes" + how[1] + " to", oblist, 0, how[1], ACTION_OTHER);
    target(" apologizes" + how[1] + " to you.", oblist, how[1], ACTION_OTHER);
    return 1;
}

int
applaud(string str)
{
    object *oblist;
    string *how;
    int attrs = ACTION_AURAL | ACTION_LACTIVITY;

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 0);

    if (!stringp(how[0]))
    {
        if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
        {
            write("You give a round of applause.\n");
            all(" gives a round of applause.");
        }
        else
        {
            write("You applaud" + how[1] + ".\n");
            all(" applauds" + how[1] + ".", how[1], attrs);
        }

        return 1;
    }

    attrs |= ACTION_INGRATIATORY;

    oblist = parse_this(how[0], "[for] / [to] [the] %l", attrs);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Applaud [how] to whom?\n");
        return 0;
    }

    if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        actor("You give a round of applause for", oblist);
        all2act(" gives a round of applause for", oblist, 0, "", attrs);
        target(" gives a round of applause for you.", oblist, "", attrs);
    }
    else
    {
        actor("You applaud" + how[1] + " to", oblist);
        all2act(" applauds" + how[1] + " to", oblist, 0, how[1], attrs);
        target(" applauds" + how[1] + " to you.", oblist, how[1], attrs);
    }

    return 1;
}

int
avert(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        write("You avert your eyes.\n");
        all(" averts " + this_player()->query_possessive() + " eyes.", "",
	    ACTION_OTHER);
        return 1;
    }

    oblist = parse_this(str, "[my] [eyes] [from] [the] %i", ACTION_OTHER);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Avert your eyes from whom/what?\n");
        return 0;
    }

    actor("You avert your eyes from", oblist, ".");
    all2act(" averts " + this_player()->query_possessive() +
        " eyes from", oblist, 0, "", ACTION_OTHER);
    target(" averts " + this_player()->query_possessive() +
        " eyes from you.", oblist, "", ACTION_OTHER);
    return 1;
}

int
back(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "slowly", 0);

    if (!stringp(how[0]))
    {
        write("You back away" + how[1] + ".\n");
        all(" begins to back away" + how[1] + ".", how[1],
	    ACTION_MACTIVITY | ACTION_VISUAL);
        return 1;
    }

    oblist = parse_this(how[0], "[away] [from] [the] %l",
        ACTION_MACTIVITY | ACTION_VISUAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Back [how] away from whom?\n");
        return 0;
    }

    actor("You back away" + how[1] + " from", oblist);
    all2actbb(" backs away" + how[1] + " from", oblist, 0, how[1],
        ACTION_MACTIVITY | ACTION_VISUAL);
    targetbb(" begins to back" + how[1] + " away from you.", oblist, how[1],
        ACTION_MACTIVITY | ACTION_VISUAL);
    return 1;
}

int
bat(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "coquetishly", 0);

    if (!stringp(how[0]))
    {
        write("You bat your eyelashes" + how[1] + ".\n");
        allbb(" bats " + this_player()->query_possessive() +
            " eyelashes" + how[1] + ".", how[1], ACTION_VISUAL);
        return 1;
    }

    oblist = parse_this(how[0], "[eyelashes] [at] [the] %l", ACTION_VISUAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Bat your eyelashes [how] at whom?\n");
        return 0;
    }

    actor("You bat your eyelashes" + how[1] + " at", oblist);
    all2actbb(" bats " + this_player()->query_possessive() +
        " eyelashes" + how[1] + " at", oblist, 0, how[1], ACTION_VISUAL);
    targetbb(" bats " + this_player()->query_possessive() +
        " eyelashes" + how[1] + " at you.", oblist, how[1], ACTION_VISUAL);
    return 1;
}

int
beam(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "joyfully", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("beaming" + how[1]);
        write("You beam" + how[1] + ".\n");
        allbb(" beams" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Beam [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("beaming" + how[1]);
    actor("You beam" + how[1] + " at", oblist);
    all2actbb(" beams" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" beams" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
beckon(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "endearingly", 1);

    if (!stringp(how[0]))
    {
        write("You beckon everyone" + how[1] + ".\n");
        allbb(" beckons everyone" + how[1] + ".", how[1],
	      ACTION_VISUAL | ACTION_LACTIVITY);
        return 1;
    }

    oblist = parse_this(how[0], "[the] %l", ACTION_VISUAL | ACTION_LACTIVITY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Beckon whom [how]?\n");
        return 0;
    }

    actor("You beckon", oblist, how[1] + ".");
    all2actbb(" beckons", oblist, how[1] + ".", how[1],
        ACTION_VISUAL | ACTION_LACTIVITY);
    targetbb(" beckons you" + how[1] + ".", oblist, how[1],
        ACTION_VISUAL | ACTION_LACTIVITY);
    return 1;
}

int
beg(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %l [for] 'forgiveness'");

    if (sizeof(oblist))
    {
        actor("You fall on your knees and beg" +
            (sizeof(oblist) > 1 ? " each of" : ""), oblist,
            " for forgiveness.");
        all2act(" falls on " + this_player()->query_possessive() +
            "knees and begs", oblist, " for forgiveness.", "",
	    ACTION_AURAL | ACTION_VISUAL);
        target(" falls on " + this_player()->query_possessive() +
            " knees and begs you for forgiveness.", oblist, "",
	    ACTION_AURAL | ACTION_VISUAL);
        return 1;
    }

    oblist = parse_this(str, "[the] %l [pardon]",
        ACTION_AURAL | ACTION_INGRATIATORY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Beg whose pardon?\n");
        return 0;
    }

    actor("You beg", oblist, "'s pardon.");
    all2act(" begs", oblist, "'s pardon.", "",
        ACTION_AURAL | ACTION_INGRATIATORY);
    target(" begs your pardon.", oblist, "",
        ACTION_AURAL | ACTION_INGRATIATORY);
    return 1;
}

int
bite(string str)
{
    if (stringp(str))
    {
        notify_fail("Bite what?\n");
        return 0;
    }

    write("You bite on your bottom lip.\n");
    allbb(" bites on " + this_player()->query_possessive() + " bottom lip.", "",
        ACTION_OTHER);
    return 1;
}

int
blanch(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "with horror", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("blanching" + how[1]);
        write("Your face blanches" + how[1] + ".\n");
        allbb("'s face blanches" + how[1] + ".", how[1], ACTION_VISUAL);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %l", ACTION_VISUAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Blanch [how] at the sight of who/what?\n");
        return 0;
    }

    SOULDESC("blanching" + how[1]);
    actor("Your face blanches" + how[1] + " at the sight of", oblist);
    all2actbb(" blanches" + how[1] + " at the sight of", oblist, 0, how[1],
        ACTION_VISUAL);
    targetbb("'s face blanches" + how[1] + " at the sight of you.", oblist,
        how[1], ACTION_VISUAL);
    return 1;
}

int
blank(string str)
{
    if (stringp(str))
    {
        notify_fail("Blank how?\n");
        return 0;
    }

    /* This emote will clear any emotion you are showing. */
    this_player()->remove_prop(LIVE_S_SOULEXTRA);

    write("You stare off into space, a blank look overcoming your face.\n");
    allbb(" stares off into space, a blank look overcoming " +
        this_player()->query_possessive() + " face.", "", ACTION_VISUAL);
    return 1;
}

int
blink(string str)
{
    if (stringp(str))
    {
        notify_fail("Blink how?\n");
        return 0;
    }

    write("You blink in disbelief.\n");
    allbb(" blinks in disbelief.", "", ACTION_VISUAL);
    return 1;
}

int
blow(string str)
{
    object *oblist;
    string *how;

    if (!strlen(str))
    {
        notify_fail("Blow [how] in whose ear or " +
            "blow a kiss [how] to whom?\n");
        return 0;
    }

    if (wildmatch("kiss *", str))
    {
        how = parse_adverb_with_space(extract(str, 5), BLANK_ADVERB, 0);

        if (!stringp(how[0]))
        {
            notify_fail("Blow [how] in whose ear or " +
                "blow a kiss [how] to whom?\n");
            return 0;
        }

        oblist = parse_this(how[0], "[to] / [at] [the] %i",
            ACTION_VISUAL | ACTION_INTIMATE | ACTION_LACTIVITY);

        if (!sizeof(oblist))
        {
            if (strlen(parse_msg))
            {
                write(parse_msg);
                return 1;
            }

            notify_fail("Blow [how] in whose ear or " +
                "blow a kiss [how] to whom?\n");
            return 0;
        }

        str = (living(oblist[0]) ? "to" : "at");
        actor("You" + how[1] + " blow a kiss " + str, oblist);
        all2actbb(how[1] + " blows a kiss " + str, oblist, "", how[1],
            ACTION_VISUAL | ACTION_INTIMATE | ACTION_LACTIVITY);
        targetbb(how[1] + " blows a kiss " + str + " you.", oblist, how[1],
            ACTION_VISUAL | ACTION_INTIMATE | ACTION_LACTIVITY);
        return 1;
    }

    how = parse_adverb_with_space(str, "gently", 0);

    if (!stringp(how[0]))
    {
        notify_fail("Blow [how] in whose ear or " +
            "blow a kiss [how] to whom?\n");
        return 0;
    }

    oblist = parse_this(how[0], "[the] %l",
        ACTION_CONTACT | ACTION_INTIMATE | ACTION_LACTIVITY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Blow [how] in whose ear or blow a kiss [how] to whom?\n");
        return 0;
    }

    actor("You blow" + how[1] + " in", oblist, "'s ear.");
    all2act(" blows" + how[1] + " in", oblist, "'s ear.", how[1],
        ACTION_CONTACT | ACTION_INTIMATE | ACTION_LACTIVITY);
    target(" blows" + how[1] + " in your ear.", oblist, how[1],
        ACTION_CONTACT | ACTION_INTIMATE | ACTION_LACTIVITY);

    return 1;
}

int
blush(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "profusely", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("blushing" + how[1]);
        write("Your cheeks are burning as you blush" + how[1] + ".\n");
        allbb(" blushes" + how[1] + ".", how[1], ACTION_VISUAL);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %l", ACTION_VISUAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Blush [how] at whose words?\n");
        return 0;
    }

    SOULDESC("blushing" + how[1]);
    actor("You blush" + how[1] + " at the words of", oblist);
    all2actbb(" blushes" + how[1] + " at the words of", oblist, 0, how[1],
        ACTION_VISUAL);
    targetbb(" blushes" + how[1] + " at your words.", oblist, how[1],
	ACTION_VISUAL);
    return 1;
}

int
boggle(string str)
{
    if (stringp(str))
    {
        notify_fail("Boggle how?\n");
        return 0;
    }

    write("Your mind boggles at that very concept.\n");
    all(" boggles at the very concept.", "", ACTION_OTHER);
    return 1;
}

int
bounce(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "uncontrollably", 0);

    if (!stringp(how[0]))
    {
        write("B O I N G !!\nYou bounce around" + how[1] + ".\n");
        all(" bounces around" + how[1] + ".", how[1],
            ACTION_HACTIVITY | ACTION_VISUAL);
        return 1;
    }

    oblist = parse_this(how[0], "[in] [to] [into] [the] %i",
        ACTION_CONTACT | ACTION_HACTIVITY | ACTION_VISUAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Bounce [how] into whom/what?\n");
        return 0;
    }

    actor("B O I N G !!\nYou bounce around" + how[1] +
        " until you bump into", oblist);
    all2act(" bounces around" + how[1] + " until " +
        this_player()->query_pronoun() + " bumps into", oblist, 0, how[1],
        ACTION_CONTACT | ACTION_HACTIVITY | ACTION_VISUAL);
    target(" bounces around" + how[1] + " until " +
        this_player()->query_pronoun() + " bumps into you.", oblist,
        how[1], ACTION_CONTACT | ACTION_HACTIVITY | ACTION_VISUAL);
    return 1;
}

int
bow(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "gracefully", 0);

    if (!stringp(how[0]))
    {
        write("You bow" + how[1] + ".\n");
        allbb(" bows" + how[1] + ".", how[1], ACTION_VISUAL | ACTION_MACTIVITY);
        return 1;
    }

    oblist = parse_this(how[0], "[to] / [before] [the] %i",
        ACTION_PROXIMATE | ACTION_VISUAL | ACTION_MACTIVITY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Bow [how] to whom/what?\n");
        return 0;
    }

    actor("You bow" + how[1] + " to", oblist);
    all2actbb(" bows" + how[1] + " to", oblist, 0, how[1],
        ACTION_PROXIMATE | ACTION_VISUAL | ACTION_MACTIVITY);
    targetbb(" bows" + how[1] + " before you.", oblist, how[1],
        ACTION_PROXIMATE | ACTION_VISUAL | ACTION_MACTIVITY);
    return 1;
}

int
brighten(string str)
{
    if (stringp(str))
    {
        notify_fail("Brighten what?\n");
        return 0;
    }

    write("You think about it, then it dawns on you! Your face brightens!\n");
    all(" thinks about it, then it dawns on " +
        this_player()->query_objective() + "... " +
        capitalize(this_player()->query_possessive()) + " face brightens.", "",
        ACTION_OTHER);
    return 1;
}

int
brood(string str)
{
    str = check_adverb_with_space(str, "deeply");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Brood how?\n");
        return 0;
    }

    SOULDESC("brooding" + str);
    write("You brood" + str + ".\n");
    all(" broods" + str + ".", str, ACTION_AURAL);
    return 1;
}

int
burp(string str)
{
    str = check_adverb_with_space(str, "rudely");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Burp how?\n");
        return 0;
    }

    write("Excuse yourself! You burp" + str + ".\n");
    all(" burps" + str + ".", str, ACTION_AURAL);
    return 1;
}

int
cackle(string str)
{
    str = check_adverb_with_space(str, "with glee");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Cackle how?\n");
        return 0;
    }

    SOULDESC("cackling" + str);
    write("You cackle" + str + ".\n");
    all(" throws " + this_player()->query_possessive() +
        " head back and cackles" + str + "!", str,
	ACTION_AURAL | ACTION_LACTIVITY);
    return 1;
}

int
caress(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "lovingly", 1);

    oblist = parse_this(how[0], "[the] %i",
        ACTION_CONTACT | ACTION_INTIMATE | ACTION_VISUAL | ACTION_LACTIVITY, 1);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Caress whom/what [how]?\n");
        return 0;
    }

    if (oblist[0] == this_player())
    {
        write("You caress yourself" + how[1] + ".\n");
        all(" caresses " + this_player()->query_objective() +
            "self" + how[1] + ".", how[1], ACTION_VISUAL | ACTION_LACTIVITY);
        return 1;
    }

    actor("You caress", oblist, how[1] + ".");
    all2act(" caresses", oblist, how[1] + ".", how[1],
        ACTION_CONTACT | ACTION_INTIMATE | ACTION_VISUAL | ACTION_LACTIVITY);
    target(" caresses you" + how[1] + ".", oblist, how[1], 
        ACTION_CONTACT | ACTION_INTIMATE | ACTION_VISUAL | ACTION_LACTIVITY);
    return 1;
}

int
cheer(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "enthusiastically", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("cheering" + how[1]);
        write("You cheer" + how[1] + ".\n");
        all(" cheers" + how[1] + ".", how[1],
            ACTION_AURAL | ACTION_MACTIVITY | ACTION_VISUAL);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i",
        ACTION_MACTIVITY | ACTION_AURAL | ACTION_VISUAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Cheer [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("cheering" + how[1]);
    actor("You cheer" + how[1] + " at", oblist);
    all2act(" cheers" + how[1] + " at", oblist, 0, how[1],
	    ACTION_MACTIVITY | ACTION_AURAL | ACTION_VISUAL);
    target(" cheers" + how[1] + " at you.", oblist, how[1],
	    ACTION_MACTIVITY | ACTION_AURAL | ACTION_VISUAL);
    return 1;
}

int
choke(string str)
{
    if (stringp(str))
    {
        notify_fail("Choke what?\n");
        return 0;
    }

    SOULDESC("choking on something");
    write("Cough, cough, cough, hark !!! You choke on something.\n");
    all("'s face colour slowly darkens as " +
        this_player()->query_pronoun() + " chokes.", "",
	ACTION_AURAL | ACTION_VISUAL);
    return 1;
}

int
chortle(string str)
{
    if (stringp(str))
    {
        notify_fail("Chortle what?\n");
        return 0;
    }

    SOULDESC("chortling");
    write("You chortle.\n");
    all(" chortles.", "", ACTION_AURAL);
    return 1;
}

int
chuckle(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "politely", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("chuckling" + how[1]);
        write("You chuckle" + how[1] + ".\n");
        all(" chuckles" + how[1] + ".", how[1], ACTION_AURAL);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i", ACTION_AURAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Chuckle [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("chuckling" + how[1]);
    actor("You chuckle" + how[1] + " at", oblist);
    all2act(" chuckles" + how[1] + " at", oblist, 0, how[1], ACTION_AURAL);
    target(" chuckles" + how[1] + " at you.", oblist, how[1], ACTION_AURAL);
    return 1;
}

int
clap(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "briefly", 0);

    if (!stringp(how[0]))
    {
        write("You clap" + how[1] + ".\n");
        all(" claps" + how[1] + ".", how[1], ACTION_AURAL | ACTION_LACTIVITY);
        return 1;
    }

    oblist = parse_this(how[0], "[for] / [to] [the] %i",
        ACTION_AURAL | ACTION_LACTIVITY | ACTION_INGRATIATORY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Clap [how] for whom/what?\n");
        return 0;
    }

    actor("You clap" + how[1] + " for", oblist);
    all2act(" claps" + how[1] + " for", oblist, 0, how[1],
        ACTION_AURAL | ACTION_LACTIVITY | ACTION_INGRATIATORY);
    target(" claps" + how[1] + " for you.", oblist, how[1],
        ACTION_AURAL | ACTION_LACTIVITY | ACTION_INGRATIATORY);
    return 1;
}

int
clear(string str)
{
    if (str != "throat")
    {
        notify_fail("Clear what? Your throat?\n");
        return 0;
    }

    write("You clear your throat to attract attention!\n");
    all(" clears " + this_player()->query_possessive() +
        " throat to attract attention.", "", ACTION_AURAL);
    return 1;
}

int
comfort(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %l", ACTION_INGRATIATORY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Comfort whom?\n");
        return 0;
    }

    actor("You comfort", oblist);
    all2act(" comforts", oblist, "", "", ACTION_INGRATIATORY);
    target(" comforts you.", oblist, "", ACTION_INGRATIATORY);
    return 1;
}

int
complain(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        SOULDESC("complaining miserably");
        write("You complain about the miserable state of being.\n");
        all(" complains about the miserable state of being.");
        return 1;
    }

    oblist = parse_this(str, "[to] [the] %i", ACTION_AURAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Complain to whom/what?\n");
        return 0;
    }

    SOULDESC("complaining miserably");
    actor("You complain about the miserable state of things to", oblist);
    all2act(" complains to", oblist);
    target(" complains about the miserable state of things to you.", oblist);
    return 1;
}

int
compliment(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %i", ACTION_AURAL | ACTION_INGRATIATORY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Compliment whom/what?\n");
        return 0;
    }

    actor("You give your deepest compliments to", oblist);
    all2act(" gives " + this_player()->query_possessive() +
        " deepest compliments to", oblist, "", "",
        ACTION_AURAL | ACTION_INGRATIATORY);
    target(" gives you " + this_player()->query_possessive() +
        " deepest compliments.", oblist, "",
	ACTION_AURAL | ACTION_INGRATIATORY);
    return 1;
}

int
confuse(string str)
{
    object *oblist;

    if ((str == "me") || (str == "myself"))
    {
        SOULDESC("looking very confused");
        write("You confuse yourself and look very confused.\n");
        all(" confuses " + this_player()->query_objective() +
            "self and looks very confused.");
        return 1;
    }

    oblist = parse_this(str, "[the] %l");

    if (!sizeof(oblist))
    {
        notify_fail("Confuse whom? Yourself or someone else?\n");
        return 0;
    }

    if (random(2))
    {
        actor("You try to confuse", oblist, "...\n... But fail...");
        all2act(" tries in vain to confuse", oblist);
        target(" tries in vain to confuse you.", oblist);
    }
    else
    {
        actor("You try to confuse", oblist, "...\n... And succeed... " +
            ((sizeof(oblist) == 1) ?
                capitalize(oblist[0]->query_pronoun()) + " is" :
                "They are") + " very confused.");
        all2act(" tries to confuse", oblist, ", who suddenly look" +
            ((sizeof(oblist) == 1) ? "s" : "") + " very confused indeed.");
        target(" tries to confuse you. You have no idea what " +
            this_player()->query_pronoun() +
            " did to confuse you, but you are very confused!", oblist);
    }

    return 1;
}

int
confused(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        SOULDESC("looking very confused");
        write("You look very confused.\n");
        all(" looks very confused.");
        return 1;
    }

    oblist = parse_this(str, "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Look confused at whom/what?\n");
        return 0;
    }

    SOULDESC("looking very confused");
    actor("You look very confused at", oblist);
    all2act(" looks very confused at", oblist);
    target(" looks very confused at you.", oblist);
    return 1;
}

int
congratulate(string str)
{
    object *oblist;
    object *femlist;

    oblist = parse_this(str, "[the] %l", ACTION_AURAL | ACTION_INGRATIATORY);

    if (!sizeof(oblist))
    {
        notify_fail("Congratulate whom?\n");
        return 0;
    }

    femlist = FILTER_GENDER(oblist, G_FEMALE);

    if (sizeof(femlist))
    {
        actor("You congratulate", femlist, ". What a gal!");
    }
    if (sizeof(oblist) > sizeof(femlist))
    {
        actor("You congratulate", (oblist - femlist), ". What a guy!");
    }

    all2act(" congratulates", oblist, "", "",
        ACTION_AURAL | ACTION_INGRATIATORY);
    target(" congratulates you.", oblist, "",
        ACTION_AURAL | ACTION_INGRATIATORY);
    return 1;
}

int
cough(string str)
{
    str = check_adverb_with_space(str, "noisily");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Cough how?\n");
        return 0;
    }

    SOULDESC("coughing" + str);
    write("Cover your mouth when you cough" + str + "!\n");
    all(" coughs" + str + ".", str);
    return 1;
}

int
cower(string str)
{
    if (stringp(str))
    {
        notify_fail("Cower what?\n");
        return 0;
    }

    write("You cower in the corner.\n");
    allbb(" cowers in the corner.");
    return 1;
}

int
cringe(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "in terror", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("cringing" + how[1]);
        write("You cringe" + how[1] + ".\n");
        allbb(" cringes" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[to] [the] [feet] [of] %l");

    if (!sizeof(oblist))
    {
        notify_fail("Cringe [how] at whom?\n");
        return 0;
    }

    SOULDESC("cringing" + how[1]);
    actor("You cringe" + how[1] + " at", oblist, "'s feet.");
    all2actbb(" cringes" + how[1] + " at the feet of", oblist, 0, how[1],
        ACTION_PROXIMATE | ACTION_VISUAL | ACTION_MACTIVITY);
    targetbb(" cringes" + how[1] + " at your feet.", oblist, how[1],
        ACTION_PROXIMATE | ACTION_VISUAL | ACTION_MACTIVITY);
}

int
cross(string str)
{
    if (str != "fingers")
    {
        notify_fail("Cross what? Your fingers?\n");
        return 0;
    }

    write("You cross your fingers for good luck.\n");
    all(" crosses " + this_player()->query_possessive() +
        " fingers for good luck.");
    return 1;
}

int
cry(string str)
{
    object *oblist;
    string *how;

    if (!stringp(str))
    {
        SOULDESC("crying");
        write("You burst into tears: Waaaaah!\n");
        all(" bursts into tears.");
        return 1;
    }

    how = parse_adverb_with_space(str, "softly", 0);

    if (!stringp(how[0]))
    {
	SOULDESC("crying" + how[1]);
	write("You cry" + how[1] + ".\n");
	all(" cries" + how[1] + ".", how[1]);
	return 1;
    }

    oblist = parse_this(how[0], "[on] [the] %l");

    if (!sizeof(oblist))
    {
        notify_fail("Cry [on] whom?\n");
        return 0;
    }

    SOULDESC("crying");
    str = this_player()->query_possessive();
    actor("You rest your head on", oblist,
        "'s shoulder and cry your heart out.");
    all2act(" rests " + str + " head on", oblist,
        "'s shoulder and cries " + str + " heart out.");
    target(" rests " + str + " head on your shoulder and cries " +
        str + " heart out.", oblist);
    return 1;
}

int
cuddle(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "lovingly", 1);

    oblist = parse_this(how[0], "[the] %i", ACTION_CONTACT | ACTION_INTIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Cuddle whom/what [how]?\n");
        return 0;
    }

    actor("You cuddle", oblist, how[1] + ".");
    all2act(" cuddles", oblist, how[1] + ".", how[1],
        ACTION_CONTACT | ACTION_INTIMATE);
    target(" cuddles you" + how[1] + ".", oblist, how[1],
	ACTION_CONTACT | ACTION_INTIMATE);
    return 1;
}

int
curl(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "happily", 1);

    oblist = parse_this(how[0], "[in] / [on] [the] %l");

    if (!sizeof(oblist))
    {
        notify_fail("Curl in whose lap [how]?\n");
        return 0;
    }

    actor("You curl up in", oblist, "'s lap and purr" + how[1] + ".");
    all2act(" curls up in", oblist, "'s lap and purrs" + how[1] + ".", how[1]);
    target(" curls up in your lap and purrs" + how[1] + ".", oblist, how[1]);
    return 1;
}

int
curse(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "colourfully", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("cursing" + how[1]);
        write("You curse" + how[1] + ".\n");
        allbb(" curses" + how[1] + ".", how[1],
            ACTION_AURAL | ACTION_OFFENSIVE);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i",
        ACTION_AURAL | ACTION_OFFENSIVE);

    if (!sizeof(oblist))
    {
        notify_fail("Curse [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("cursing" + how[1]);
    actor("You curse" + how[1] + " at", oblist);
    all2actbb(" curses" + how[1] + " at", oblist, "", how[1],
        ACTION_AURAL | ACTION_OFFENSIVE);
    targetbb(" curses" + how[1] + " at you.", oblist, how[1],
        ACTION_AURAL | ACTION_OFFENSIVE);
    return 1;
}

int
curtsey(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "gracefully", 0);

    if (!stringp(how[0]))
    {
        write("You curtsey" + how[1] + ".\n");
        all(" curtseys" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[to] / [before] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Curtsey [how] to whom/what?\n");
        return 0;
    }

    actor("You curtsey" + how[1] + " to", oblist);
    all2actbb(" curtseys" + how[1] + " to", oblist, 0, how[1]);
    targetbb(" curtseys" + how[1] + " to you.", oblist, how[1]);
    return 1;
}

int
dance(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        write("You dance a lively jig.\n");
        all(" dances a lively jig.");
        return 1;
    }

    oblist = parse_this(str, "[with] [the] %i", ACTION_CONTACT);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Dance with whom/what?\n");
        return 0;
    }

    actor("You sweep", oblist, " across the dance floor.");
    all2act(" sweeps", oblist, " across the dance floor.", "", ACTION_CONTACT);
    target(" sweeps you across the dance floor.", oblist, "", ACTION_CONTACT);
    return 1;
}

int
despair(string str)
{
    if (stringp(str))
    {
        notify_fail("Despair what?\n");
        return 0;
    }

    write("You tear a lump of hair off your scalp.\n");
    all(" tears a lump of hair off " + this_player()->query_possessive() +
        " scalp in despair.");
    SOULDESC("looking desperate");
    return 1;
}

int
disagree(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "totally", 0);

    if (!stringp(how[0]))
    {
        write("You disagree" + how[1] + ".\n");
        allbb(" disagrees" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[with] [the] %l");

    if (!sizeof(oblist))
    {
        notify_fail("Disagree [how] with whom?\n");
        return 0;
    }

    actor("You disagree" + how[1] + " with", oblist);
    all2actbb(" disagrees" + how[1] + " with", oblist, 0, how[1]);
    targetbb(" disagrees" + how[1] + " with you.", oblist, how[1]);
    return 1;
}

int
drool(string str)
{
    str = check_adverb_with_space(str, "excessively");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Drool how?\n");
        return 0;
    }

    SOULDESC("drooling" + str);
    write("You drool" + str + ".\n");
    all(" drools" + str + ".", str);
    return 1;
}

int
duh(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        SOULDESC("probably feeling very dumb");
        write("Uh, duhhh...  Feeling stupid, eh?\n");
        all(" goes duhhh.  Must be feeling dumb.");
        return 1;
    }

    oblist = parse_this(str, "[at] [the] %l");

    if (!sizeof(oblist))
    {
        notify_fail("Duh at whom?\n");
        return 0;
    }

    str = ((sizeof(oblist) == 1) ? oblist[0]->query_objective() : "them");

    actor("You let", oblist, " know what you think of " + str + "!");
    all2act(" goes duhhh at", oblist, ", making " + str + " feel pretty dumb!");
    target(" goes duhhh at you! Guess you have done something dumb!", oblist);
    return 1;
}

int
eeks(string str)
{
    if (stringp(str))
    {
        notify_fail("Eeks what?\n");
        return 0;
    }

    write("Eeks!  Oh my gosh!\n");
    all(" goes eeks!");
    return 1;
}

int
excuse(string str)
{
    object *oblist;
    string *how;

    if (!stringp(str))
    {
        str = "me";
    }

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 0);

    if (!stringp(how[0]))
    {
        how[0] = "me";
    }

    oblist = parse_this(how[0], "[the] %l", 0, 1);

    if (!sizeof(oblist))
    {
        notify_fail("Excuse whom [how]?\n");
        return 0;
    }

    if (oblist[0] == this_player())
    {
        if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
        {
            how[1] = ADD_SPACE_TO_ADVERB("politely");
        }

        write("You excuse yourself" + how[1] + ".\n");
        all(" excuses " + this_player()->query_objective() + "self" +
            how[1] + ".", how[1]);
        return 1;
    }

    if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        how[1] = ADD_SPACE_TO_ADVERB("considerately");
    }
    actor("You excuse", oblist, how[1] + ".");
    all2act(" excuses", oblist, how[1] + ".", how[1]);
    target(" excuses you" + how[1] + ".", oblist, how[1]);
    return 1;
}

int
explodes(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        write("You hold your breath and explode in anger!\n");
        all(" explodes with anger.");
        return 1;
    }

    oblist = parse_this(str, "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Explode at whom/what?\n");
        return 0;
    }

    actor("You explode in anger at", oblist, "!");
    all2act(" explodes with anger at", oblist, "!");
    target(" explodes with anger at you.", oblist);
    return 1;
}

int
eyebrow(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "inquisitively", 0);

    if (!stringp(how[0]))
    {
        write("You raise an eyebrow" + how[1] + ".\n");
        allbb(" raises " + this_player()->query_possessive() +
            " eyebrow" + how[1] + ".", how[1]);

        return 1;
    }

    oblist = parse_this(how[0], "[eyebrow] [at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Raise your eyebrow [how] at whom/what?\n");
        return 0;
    }

    actor("You raise your eyebrow" + how[1] + " at", oblist);
    all2actbb(" raises " + this_player()->query_possessive() +
        " eyebrow" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" raises " + this_player()->query_possessive() + " eyebrow" +
        how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
fart(string str)
{
    if (stringp(str))
    {
        notify_fail("Fart how?\n");
        return 0;
    }

    write("How rude, farting!\n");
    all(" lets off a real rip-roarer.");
    return 1;
}

int
fawn(string str)
{
    object *oblist;

    oblist = parse_this(str, "[at] / [over] [the] %i", ACTION_INGRATIATORY);

    if (!sizeof(oblist))
    {
        notify_fail("Fawn over whom/what?\n");
        return 0;
    }

    actor("You fawn over", oblist);
    all2act(" fawns over", oblist, "", "", ACTION_INGRATIATORY);
    target(" fawns over you.", oblist, "", ACTION_INGRATIATORY);
    return 1;
}

int
feign(string str)
{
    if (stringp(str))
    {
        notify_fail("Feign what?\n");
        return 0;
    }

    SOULDESC("feigning a look of innocence");
    write("You feign a look of innocence.\n");
    allbb(" feigns a look of innocence.");
    return 1;
}

int
ffinger(string str)
{
    object *oblist;

    oblist = parse_this(str, "[at] / [to] [the] %l");

    if (!sizeof(oblist))
    {
        notify_fail("Show finger to whom/what?\n");
        return 0;
    }

    actor("With your middle finger you make it very clear to", oblist,
        " that you want " +
        (sizeof(oblist) == 1 ? oblist[0]->query_objective(): "them") +
        " to fuck off.");
    all2actbb(" implies with " + this_player()->query_possessive() +
        " middle finger, that " + this_player()->query_pronoun() +
        " wants", oblist, " to fuck off.");
    targetbb(" implies with " + this_player()->query_possessive() +
        " middle finger, that " + this_player()->query_pronoun() +
        " wants you to fuck off.", oblist);
    return 1;
}

int
fidget(string str)
{
    str = check_adverb_with_space(str, "like a squirrel");
    
    if (str == NO_ADVERB_WITH_SPACE) 
    {   
        notify_fail("Fidget how?\n");
        return 0;
    }
    
    SOULDESC("fidgetting" + str);
    write("You fidget" + str + ".\n");
    all(" fidgets" + str + ".", str, ACTION_AURAL | ACTION_VISUAL);
    return 1;
}

int
finger(string str)
{
    notify_fail("Use \"ffinger\" to perform the emotion.\n");
    return 0;
}

int
flex(string str)
{
    str = check_adverb_with_space(str, "impressively");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Flex your muscles how?\n");
        return 0;
    }

    write("You flex your muscles" + str + ".\n");
    allbb(" flexes " + this_player()->query_possessive() + " muscles" + str +
        ".", str);
    return 1;
}

int
flip(string str)
{
    if (stringp(str))
    {
        notify_fail("Flip what?\n");
        return 0;
    }

    write("You flip head over heels.\n");
    all(" flips head over heels.");
    return 1;
}

int
flirt(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "outrageously", 0);

    oblist = parse_this(how[0], "[with] [the] %i", ACTION_INTIMATE);

    if (!sizeof(oblist))
    {
        notify_fail("Flirt [how] with whom/what?\n");
        return 0;
    }

    actor("You flirt" + how[1] + " with", oblist);
    all2actbb(" flirts" + how[1] + " with", oblist, "", how[1],
        ACTION_INTIMATE);
    targetbb(" flirts" + how[1] + " with you.", oblist, how[1],
        ACTION_INTIMATE);
    return 1;
}

int
fondle(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "absentmindedly", 1);

    oblist = parse_this(how[0], "[the] %i",
        ACTION_CONTACT | ACTION_INTIMATE, 1);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Fondle whom/what [how]?\n");
        return 0;
    }

    if (oblist[0] == this_player())
    {
        write("You fondle yourself" + how[1] + ".\n");
        allbb(" fondles " + this_player()->query_objective() + "self" +
            how[1], how[1], ACTION_OTHER);
        return 1;
    }

    actor("You fondle", oblist, how[1] + ".");
    all2act(" fondles", oblist, how[1] + ".", how[1],
        ACTION_CONTACT | ACTION_INTIMATE);
    target(" fondles you" + how[1] + ".", oblist, how[1],
        ACTION_CONTACT | ACTION_INTIMATE);
    return 1;
}

int
forgive(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "generously", 1);

    oblist = parse_this(how[0],
        "[the] %l [his] / [her] / [its] / [their] [sins]", 0, 1);

    if (!sizeof(oblist))
    {
        notify_fail("Forgive whom [how]?\n");
        return 0;
    }

    if (oblist[0] == this_player())
    {
        write("You forgive yourself" + how[1] + ".\n");
        allbb(" forgives " + this_player()->query_objective() + "self" +
            how[1], how[1]);
        return 1;
    }

    str = ((sizeof(oblist) == 1) ? oblist[0]->query_possessive() : "their");
    actor("You forgive", oblist, " " + str + " sins" + how[1] + ".");
    all2act(" forgives", oblist, " " + str + " sins" + how[1] + ".", how[1]);
    target(" forgives you your sins" + how[1] + ".", oblist, how[1]);
    return 1;
}

int
french(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %l", ACTION_CONTACT | ACTION_INTIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("French whom?\n");
        return 0;
    }

    actor("You give" + (sizeof(oblist) > 1 ? " each of" : ""), oblist,
        " a REAL kiss..it takes a long time..");
    all2act(" gives", oblist, " a deep and passionate kiss.\n" +
        "It seems to take forever...Sexy, eh?", "",
            ACTION_CONTACT | ACTION_INTIMATE);
    target(" gives you a deep and passionate kiss...\n" +
        "It seems to take forever...", oblist, "",
        ACTION_CONTACT | ACTION_INTIMATE);
    return 1;
}

int
fret(string str)
{
    str = check_adverb_with_space(str, "worriedly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Fret how?\n");
        return 0;
    }

    SOULDESC("fretting" + str);
    write("You fret" + str + ".\n");
    all(" frets" + str + ".", str);
    return 1;
}

int
frown(string str)
{
    object *oblist;
    string *how;

    /* Alternative: concernedly -> showing concern? */
    how = parse_adverb_with_space(str, "as if something is wrong", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("frowning" + how[1]);
        write("You frown" + how[1] + ".\n");
        allbb(" frowns" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Frown [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("frowning" + how[1]);
    actor("You frown" + how[1] + " at", oblist);
    all2actbb(" frowns" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" frowns" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
fume(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "angrily", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("fuming" + how[1]);
        write("You fume" + how[1] + ".\n");
        allbb(" fumes" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Fume [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("fuming" + how[1]);
    actor("You fume" + how[1] + " at", oblist);
    all2actbb(" fumes" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" fumes" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
gag(string str)
{
    if (stringp(str))
    {
        notify_fail("Gag what?\n");
        return 0;
    }

    SOULDESC("gagging");
    write("You gag.\n");
    all(" gags.");
    return 1;
}

int
gasp(string str)
{
    if (stringp(str))
    {
        notify_fail("Gasp what?\n");
        return 0;
    }

    SOULDESC("gasping in astonishment");
    write("You gasp in astonishment.\n");
    all(" gasps in astonishment!");
    return 1;
}

int
giggle(string str)
{
    str = check_adverb_with_space(str, "merrily");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Giggle how?\n");
        return 0;
    }

    SOULDESC("giggling" + str);
    write("You giggle" + str + ".\n");
    all(" giggles" + str + ".", str);
    return 1;
}

int
gesture(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "indifferently", 0);

    if (!stringp(how[0]))
    {
        write("You gesture" + how[1] + ".\n");
        allbb(" gestures" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Gesture [how] at whom/what?\n");
        return 0;
    }

    actor("You gesture" + how[1] + " at", oblist);
    all2actbb(" gestures" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" gestures" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
glare(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "stonily", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("glaring" + how[1]);
        write("You glare" + how[1] + ".\n");
        allbb(" glares" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Glare [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("glaring" + how[1]);
    actor("You glare" + how[1] + " at", oblist);
    all2actbb(" glares" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" glares" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
greet(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Greet whom/what?\n");
        return 0;
    }

    actor("You greet", oblist,".  How friendly!");
    all2act(" greets", oblist);
    target(" raises " + this_player()->query_possessive() +
        " hand and greets you!", oblist);
    return 1;
}

int
grimace(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 0);

    if (!stringp(how[0]))
    {
        if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
        {
            how[1] = ADD_SPACE_TO_ADVERB("painfully");
        }

        write("You grimace" + how[1] + ".\n");
        all(" grimaces" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Grimace [how] [at whom/what]?\n");
        return 0;
    }

    if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        how[1] = ADD_SPACE_TO_ADVERB("anxiously");
    }

    actor("You grimace" + how[1] + " at", oblist);
    all2act(" grimaces" + how[1] + " at", oblist, 0, how[1]);
    target(" grimaces" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
grin(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 0);

    /* Goodaligned players do not grin evilly by default. */
    if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        how[1] = ((this_player()->query_alignment() > 0) ?
            "merrily" : "evilly");
        how[1] = ADD_SPACE_TO_ADVERB(how[1]);
    }

    if (!stringp(how[0]))
    {
        SOULDESC("grinning" + how[1]);
        write("You grin" + how[1] + ".\n");
        allbb(" grins" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Grin [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("grinning" + how[1]);
    actor("You grin" + how[1] + " at", oblist);
    all2act(" grins" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" grins" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
groan(string str)
{
    str = check_adverb_with_space(str, "loudly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Groan how?\n");
        return 0;
    }

    SOULDESC("groaning" + str);
    write("You groan" + str + ".\n");
    all(" groans" + str + ".", str);
    return 1;
}

int
grope(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %i", ACTION_CONTACT | ACTION_INTIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Grope whom/what?\n");
        return 0;
    }

    actor("You grope", oblist, " in an unskilled manner.");
    all2act(" gropes", oblist, " in an unskilled manner.", "",
        ACTION_CONTACT | ACTION_INTIMATE);
    target(" gropes you in an unskilled manner.", oblist, "",
        ACTION_CONTACT | ACTION_INTIMATE);
    return 1;
}

int
grovel(string str)
{
    object *oblist;

    oblist = parse_this(str, "[to] / [for] / [before] [the] %i", ACTION_PROXIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Grovel in front of whom/what?\n");
        return 0;
    }

    actor("You fall to the ground in front of", oblist, ", groveling.");
    all2act(" falls to the ground and grovels in front of", oblist, 0, "",
        ACTION_PROXIMATE);
    target(" falls to the ground and grovels in front of you.", oblist, "",
        ACTION_PROXIMATE);
    return 1;
}

int
growl(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "menacingly", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("growling" + how[1]);
        write("You growl" + how[1] + ".\n");
        all(" growls" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Growl [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("growling" + how[1]);
    actor("You growl" + how[1] + " at", oblist);
    all2act(" growls" + how[1] + " at", oblist, 0, how[1]);
    target(" growls" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
grumble(string str)
{
    object *oblist;
    string *how;

    /* Quod licet Jovi, non licet bovi according to Plugh, Mercade ;-) */
    how = parse_adverb_with_space(str,
        (this_player()->query_wiz_level() ? "angrily" : "unhappily"), 0);

    if (!stringp(how[0]))
    {
        SOULDESC("grumbling" + how[1]);
        write("You grumble" + how[1] + ".\n");
        all(" grumbles" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Grumble [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("grumbling" + how[1]);
    actor("You grumble" + how[1] + " at", oblist);
    all2act(" grumbles" + how[1] + " at", oblist, 0, how[1]);
    target(" grumbles" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
hang(string str)
{
    if (stringp(str))
    {
        notify_fail("Hang what?\n");
        return 0;
    }

    write("You hang your head in shame.\n");
    allbb(" hangs " + this_player()->query_possessive() + " head in shame.");
    return 1;
}

int
hiccup(string str)
{
    if (stringp(str))
    {
        notify_fail("Hiccup what?\n");
        return 0;
    }

    SOULDESC("having the hiccups");
    write("Hic!\n");
    all(" hiccups.");
    return 1;
}

int
hmm(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        write("Hmmmm.\n");
        all(" goes hmmmm.");
        return 1;
    }

    oblist = parse_this(str, "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Go hmm at whom/what?\n");
        return 0;
    }

    actor("You go hmmmm at", oblist);
    all2act(" goes hmmmm at", oblist);
    target(" goes hmmmm at you.", oblist);
    return 1;
}

int
hold(string str)
{
    object *oblist;

    if (str == "all")
    {
        notify_fail("To hold all people, use \"hold all close\".\n");
        return 0;
    }
 
    oblist = parse_this(str, "[the] %l [close]", ACTION_CONTACT);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        oblist = parse_this(str, "[the] %i 'close'");

        if (!sizeof(oblist))
        {
            if (strlen(parse_msg))
            {
                write(parse_msg);
                return 1;
            }

            notify_fail("Hold whom/what?\n");
            return 0;
        }
    }

    actor("You hold", oblist, " close.");
    all2act(" holds", oblist, " close.", "", ACTION_CONTACT);
    target(" holds you close.", oblist, "", ACTION_CONTACT);
    return 1;
}

int
hug(string str)
{
    object *oblist;
    string *how;
    int    grouphug;

    how = parse_adverb_with_space(str, BLANK_ADVERB, 1);

    grouphug = (how[0] == "all");
    oblist = parse_this(how[0], "[the] %i", ACTION_CONTACT);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Hug whom/what [how]?\n");
        return 0;
    }

    if (grouphug && (sizeof(oblist) > 1))
    {
        actor("You engage in a group-hug with", oblist, " and hug them" +
            how[1] + ".");
        all(" engages in a group-hug with everyone in the room and hugs you" +
            how[1] + ".", how[1], ACTION_CONTACT);
        return 1;
    }

    actor("You hug", oblist, how[1] + ".");
    all2act(" hugs", oblist, how[1] + ".", how[1], ACTION_CONTACT);
    target(" hugs you" + how[1] + ".", oblist, how[1], ACTION_CONTACT);
    return 1;
}

int
hum(string str)
{
    if (!strlen(str))
    {
        str = "merry";
    }
    else
    {
        if (!parse_command(str, ({ }), "[a] %w [tune]", str))
        {
            notify_fail("What kind of tune do you want to hum?\n");
            return 0;
        }
    }

    str = LANG_ADDART(str);
    write("You hum " + str + " tune.\n");
    say(QCTNAME(this_player()) + " hums " + str + " tune.\n");
    return 1;
}

int
ignore(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Ignore whom/what?\n");
        return 0;
    }

    actor("You start to ignore", oblist);
    all2actbb(" turns " + this_player()->query_possessive() + " back on",
        oblist);
    targetbb(" turns " + this_player()->query_possessive() + " back on you " +
        "and starts to ignore you.", oblist);
    return 1;
}

int
jump(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 0);

    if (!stringp(how[0]))
    {
        if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
        {
            how[1] = ADD_SPACE_TO_ADVERB("unexpectedly");
        }

        write("You jump" + how[1] + ".\n");
        all(" jumps" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[on] / [over] [the] %i",
        ACTION_CONTACT | ACTION_HACTIVITY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Jump how [over whom/what]?\n");
        return 0;
    }

    if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        how[1] = ADD_SPACE_TO_ADVERB("absentmindedly");
    }

    actor("You jump" + how[1] + " all over", oblist);
    all2act(" jumps" + how[1] + " all over", oblist, 0, how[1],
        ACTION_CONTACT | ACTION_HACTIVITY);
    target(" jumps" + how[1] + " all over you.", oblist, how[1],
        ACTION_CONTACT | ACTION_HACTIVITY);
    return 1;
}

int
kick(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "fanatically", 1);

    oblist = parse_this(how[0], "[the] %i",
        ACTION_CONTACT | ACTION_MACTIVITY | ACTION_OFFENSIVE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Kick whom/what [how]?\n");
        return 0;
    }

    actor("You kick", oblist, how[1] + ".");
    all2act(" kicks", oblist, how[1] + ".", how[1],
        ACTION_CONTACT | ACTION_MACTIVITY | ACTION_OFFENSIVE);
    target(" kicks you" + how[1] + ".   OUCH!!!", oblist, how[1],
        ACTION_CONTACT | ACTION_MACTIVITY | ACTION_OFFENSIVE);
    return 1;
}

int
kiss(string str)
{
    object *oblist;
    string *how;
    string *zones;
    string *parts;
    int size;
    string location;

    if (!stringp(str))
    {
        notify_fail("Whom are you trying to kiss [how/where]?\n");
        return 0;
    }

    zones = ({ "forehead", "cheek", "lips", "nose", "hand", "feet", "chin",
        "neck", "ear" });

    parts = explode(str, " ");
    if ((size = sizeof(parts)) > 1)
    {
        if (member_array(parts[size - 1], zones) != -1)
        {
            location = parts[size - 1];
            str = implode(parts[..(size - 2)], " ");
        }
    }

    if (strlen(location))
    {
        oblist = parse_this(str, "[the] %l [on] [the]",
            ACTION_CONTACT | ACTION_INTIMATE);
    }
    else
    {
        how = parse_adverb_with_space(str, "gently", 1);
        oblist = parse_this(how[0], "[the] %i",
            ACTION_CONTACT | ACTION_INTIMATE);
    }

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Kiss whom [how/where]?\n");
        return 0;
    }

    if (strlen(location))
    {
        str = ((sizeof(oblist) == 1) ?
            (oblist[0]->query_possessive() + " " + location + ".") :
            ("their " + location + "s."));
        actor("You kiss", oblist, " on " + str);
        all2act(" kisses", oblist, " on " + str, "",
            ACTION_CONTACT | ACTION_INTIMATE);
        target(" kisses you on your " + location + ".", oblist, "",
            ACTION_CONTACT | ACTION_INTIMATE);
    }
    else
    {
        actor("You kiss", oblist, how[1] + ".");
        all2act(" kisses", oblist, how[1] + ".", how[1],
            ACTION_CONTACT | ACTION_INTIMATE);
        target(" kisses you" + how[1] + ".", oblist, how[1],
            ACTION_CONTACT | ACTION_INTIMATE);
    }

    return 1;
}

int
knee(string str)
{
    object *oblist;
    object *femlist;

    oblist = parse_this(str, "[the] %l",
        ACTION_CONTACT | ACTION_OFFENSIVE | ACTION_MACTIVITY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Knee whom?\n");
        return 0;
    }

    femlist = FILTER_GENDER(oblist, G_FEMALE);
    if (sizeof(femlist))
    {
        actor("You try to knee", femlist, ".\nNot very effective though.");
        all2act(" tries to knee", femlist, ".\nNot very effective though.", "",
            ACTION_CONTACT | ACTION_OFFENSIVE | ACTION_MACTIVITY);
        target(" tries to knee you, without much effect.", femlist, "",
            ACTION_CONTACT | ACTION_OFFENSIVE | ACTION_MACTIVITY);
    }

    oblist -= femlist;
    if (sizeof(oblist))
    {
        actor("You hit", oblist, " with your knee, sending " +
            ((sizeof(oblist) > 1) ? "them" : "him") +
            " to the ground, writhing in pain!");
        all2act(" suddenly raises " + this_player()->query_possessive() +
            " knee, sending", oblist, " to the floor, writhing in pain!", "",
            ACTION_CONTACT | ACTION_OFFENSIVE | ACTION_MACTIVITY);
        target(" hits you with " + this_player()->query_possessive() +
            " knee below your belt!\n" +
            "You double over and fall to the ground, writhing in " +
            "excrutiating pain,\nfeeling like you may throw up " +
            "everything you have eaten!", oblist, "",
            ACTION_CONTACT | ACTION_OFFENSIVE | ACTION_MACTIVITY);
    }

    return 1;
}

int
kneel(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "respectfully", 0);

    if (!stringp(how[0]))
    {
        write("You kneel" + how[1] + ".\n");
        allbb(" kneels" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[before] [the] %i", ACTION_PROXIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Kneel [how] before whom/what?\n");
        return 0;
    }

    actor("You kneel" + how[1] + " before", oblist);
    all2actbb(" kneels" + how[1] + " before", oblist, 0, how[1],
        ACTION_PROXIMATE);
    targetbb(" kneels" + how[1] + " before you.", oblist, how[1],
        ACTION_PROXIMATE);
    return 1;
}

int
laugh(string str)
{
    object *oblist;
    string *how;

    if (!stringp(str))
    {
        write("You fall down laughing.\n");
        all(" falls down laughing.");
        SOULDESC("laughing");
        return 1;
    }

    how = parse_adverb_with_space(str, "exuberantly", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("laughing" + how[1]);
        write("You laugh" + how[1] + ".\n");
        all(" laughs" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i", 0, 1);

    if (!sizeof(oblist))
    {
        notify_fail("Laugh how [at whom/what]?\n");
        return 0;
    }

    if (oblist[0] == this_player())
    {
        SOULDESC("laughing" + how[1]);
        write("You laugh" + how[1] + " at yourself.\n");
        all(" laughs" + how[1] + " at " + this_player()->query_objective() +
            "self.", how[1]);
        return 1;
    }

    SOULDESC("laughing" + how[1]);
    actor("You laugh" + how[1] + " at", oblist, ".");
    all2act(" laughs" + how[1] + " at", oblist, 0, how[1]);
    target(" laughs" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
leer(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "slyly", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("leering" + how[1]);
        write("You leer" + how[1] + " around.\n");
        allbb(" leers" + how[1] + " around.", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Leer [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("leering" + how[1]);
    actor("You leer" + how[1] + " at", oblist,".  Hmm......");
    all2actbb(" leers" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" leers" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
lick(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 1);

    if (!stringp(how[0]))
    {
        if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
        {
            how[1] = ADD_SPACE_TO_ADVERB("in anticipation");
        }

        write("You lick your lips" + how[1] + ".\n");
        allbb(" licks " + this_player()->query_possessive() + " lips" +
            how[1] + ".", how[1], ACTION_VISUAL);
        return 1;
    }

    oblist = parse_this(how[0], "[the] %i", ACTION_CONTACT | ACTION_INTIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Lick whom/what [how]?\n");
        return 0;
    }

    if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        how[1] = ADD_SPACE_TO_ADVERB("joyously");
    }

    actor("You lick", oblist, how[1] + ".");
    all2actbb(" licks", oblist, how[1] + ".", how[1],
        ACTION_CONTACT | ACTION_INTIMATE);
    target(" licks you" + how[1] + ".", oblist, how[1],
        ACTION_CONTACT | ACTION_INTIMATE);
    return 1;
}

int
listen(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "attentively", 0);

    if (!stringp(how[0]))
    {
        write("You listen" + how[1] + ".\n");
        allbb(" listens" + how[1] + ".", how[1]);
        SOULDESC("listening" + how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[to] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Listen [how] to whom/what?\n");
        return 0;
    }

    SOULDESC("listening" + how[1]);
    actor("You listen" + how[1] + " to", oblist);
    all2actbb(" listens" + how[1] + " to", oblist, 0, how[1]);
    targetbb(" listens" + how[1] + " to you.", oblist, how[1]);
    return 1;
}

int
love(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %i", ACTION_PROXIMATE | ACTION_INTIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Love whom/what?\n");
        return 0;
    }

    SOULDESC("hopelessly in love");
    actor("You tell your true feelings to", oblist);
    all2act(" whispers softly to", oblist, "", "",
        ACTION_PROXIMATE | ACTION_INTIMATE);
    target(" whispers sweet words of love to you.", oblist, "",
        ACTION_PROXIMATE | ACTION_INTIMATE);
    return 1;
}

int
melt(string str)
{
    object *oblist;

    oblist = parse_this(str, "[in] [the] %l", ACTION_CONTACT | ACTION_INTIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Melt in whose arms?\n");
        return 0;
    }

    actor("You melt in", oblist, "'s arms.");
    all2act(" melts in", oblist, "'s arms.", "",
        ACTION_CONTACT | ACTION_INTIMATE);
    target(" melts in your arms.", oblist, "",
        ACTION_CONTACT | ACTION_INTIMATE);
    SOULDESC("hopelessly in love");
    return 1;
}

int
moan(string str)
{
    str = check_adverb_with_space(str, BLANK_ADVERB);

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Moan how?\n");
        return 0;
    }

    SOULDESC("moaning" + str);
    write("You start to moan" + str + ".\n");
    all(" starts to moan" + str + ".", str);
    return 1;
}

int
mourn(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "sadly", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("mourning" + how[1]);
        write("You mourn" + how[1] + ".\n");
        allbb(" mourns" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[for] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Mourn [how] for whom/what?\n");
        return 0;
    }

    SOULDESC("mourning" + how[1]);
    actor("You mourn" + how[1] + " for", oblist);
    all2actbb(" mourns" + how[1] + " for", oblist, 0, how[1]);
    targetbb(" mourns" + how[1] + " for you.", oblist, how[1]);
    return 1;
}

int
mumble(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        SOULDESC("mumbling about something");
        write("You mumble something about something else.\n");
        all(" mumbles something about something else.");
        return 1;
    }

    if ((strlen(str) > 60) &&
        (!(this_player()->query_wiz_level())))
    {
        SOULDESC("mumbling about something");
        write("You mumble beyond the end of the line and become incoherent.\n");
        all(" mumbles too much and becomes incoherent.");
        return 1;
    }

    oblist = parse_this(str, "[about] [the] %i");

    if (!sizeof(oblist))
    {
        SOULDESC("mumbling about something");
        write("You mumble " + str + "\n");
        all(" mumbles " + str);
        return 1;
    }

    SOULDESC("mumbling about someone");
    actor("You mumble something about", oblist);
    all2act(" mumbles some about", oblist,
        " and it probably is a good thing you cannot understand it.");
    target(" mumbles about you and it probably is a good thing you cannot " +
        "understand it.", oblist);
    return 1;    
}

int
nibble(string str)
{
    object *oblist;

    oblist = parse_this(str, "[on] [the] %l", ACTION_CONTACT | ACTION_INTIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Nibble on whose ear?\n");
        return 0;
    }

    actor("You nibble on", oblist, "'s ear.");
    all2act(" nibbles on", oblist, "'s ear.", "",
        ACTION_CONTACT | ACTION_INTIMATE);
    target(" nibbles on your ear.", oblist, "",
        ACTION_CONTACT | ACTION_INTIMATE);
    return 1;
}

int
nod(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "solemnly", 0);

    if (!stringp(how[0]))
    {
        write("You nod" + how[1] + ".\n");
        allbb(" nods" + how[1] + ".", how[1]);
        SOULDESC("nodding" + how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Nod [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("nodding" + how[1]);
    actor("You nod" + how[1] + " at", oblist);
    all2actbb(" nods" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" nods" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
nudge(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %i", ACTION_CONTACT | ACTION_VISUAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Nudge whom/what?\n");
        return 0;
    }

    actor("You nudge", oblist);
    all2act(" nudges", oblist, "", ACTION_CONTACT | ACTION_VISUAL);
    target(" nudges you.", oblist, "", ACTION_CONTACT | ACTION_VISUAL);
    return 1;
}

int
nuzzle(string str)
{
    object *oblist;
    string *how;
    string pos;

    how = parse_adverb_with_space(str, "playfully", 1);

    oblist = parse_this(how[0], "[the] %l",
        ACTION_CONTACT | ACTION_INTIMATE | ACTION_VISUAL);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Nuzzle whom [how]?\n");
        return 0;
    }

    pos = ((sizeof(oblist) == 1) ? oblist[0]->query_possessive() : "their");
    str = this_player()->query_possessive();

    actor("You put your arms around", oblist, " and nuzzle your face" +
        how[1] + " in " + pos + " neck.");
    all2act(" puts " + str + " arms around" , oblist, " and nuzzles " + str +
        " face" + how[1] + " in " + pos + " neck.", how[1],
        ACTION_CONTACT | ACTION_INTIMATE | ACTION_VISUAL);
    target(" puts " + str + " arms around you and nuzzles " + str +
        " face" + how[1] + " in your neck.", oblist, how[1],
        ACTION_CONTACT | ACTION_INTIMATE | ACTION_VISUAL);
    return 1;
}

int
oops(string str)
{
    if (stringp(str))
    {
        notify_fail("Oops what?\n");
        return 0;
    }

    write("Oops!\n");
    say("Oops, " + QTNAME(this_player()) + " seems to have made a mistake!\n");
    return 1;
}

int
ouch(string str)
{
    if (stringp(str))
    {
        notify_fail("Ouch what?\n");
        return 0;
    }

    write("Ouch! That hurts!\n");
    say(QCTNAME(this_player()) + " goes ouch! That must have hurt!\n");
    return 1;
}

int
pace(string str)
{
    str = check_adverb_with_space(str, "restlessly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Pace how?\n");
        return 0;
    }

    SOULDESC("pacing" + str);
    write("You pace around" + str + ".\n");
    all(" paces around" + str + ".", str);
    return 1;
}

int
panic(string str)
{
    if (stringp(str))
    {
        notify_fail("Panic what?\n");
        return 0;
    }

    write("PANIC!!!!! You panic and look for a place to hide.\n");
    allbb(" panics and looks for a place to hide.");
    return 1;
}

int
pant(string str)
{
    str = check_adverb_with_space(str, "wearily");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Pant how?\n");
        return 0;
    }

    SOULDESC("panting" + str);
    write("You pant" + str + ".\n");
    all(" pants" + str + ".", str);
    return 1;
}

int
pat(string str)
{
    object *oblist;
    string *zones;
    string *parts;
    int    size;
    string location;

    zones = ({ "back", "forehead", "head", "shoulder", "tummy", "belly",
        "bottom" });

    /* When patting yourself, pat on your tummy. */
    str = (strlen(str) ? lower_case(str) : "tummy");
    if (member_array(str, zones) != -1)
    {
        write("You pat yourself on your " + str + ".\n");
        all(" pats " + this_player()->query_objective() +
           "self on " + this_player()->query_possessive() + " " + str + ".",
           "", ACTION_VISUAL);
        return 1;
    }

    parts = explode(str, " ");
    if ((size = sizeof(parts)) > 1)
    {
        if (member_array(parts[size - 1], zones) != -1)
        {
            location = parts[size - 1];
            str = implode(parts[..(size - 2)], " ");
        }
    }

    /* When patting someone else, pat on the back by default. */
    if (!stringp(location))
    {
        location = "back";
    }

    oblist = parse_this(str, "[the] %l [on] [the]", ACTION_CONTACT);
    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Pat [whom] [where]?\n");
        return 0;
    }

    str = ((sizeof(oblist) == 1) ?
           (oblist[0]->query_possessive() + " " + location + ".") :
           ("their " + location + "s."));

    actor("You pat", oblist, " on " + str);
    all2act(" pats", oblist, " on " + str, "", ACTION_CONTACT);
    target(" pats you on your " + location + ".", oblist, "", ACTION_CONTACT);
    return 1;
}

int
peer(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "quizzically", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("peering" + how[1]);
        write("You peer" + how[1] + " around.\n");
        allbb(" peers" + how[1] + " around.", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Peer [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("peering" + how[1]);
    actor("You peer" + how[1] + " at", oblist,".  Hmm......");
    all2actbb(" peers" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" peers" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
pet(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "adoringly", 1);

    if (!stringp(how[0]))
    {
        notify_fail("Pet whom [how]?\n");
        return 0;
    }

    oblist = parse_this(how[0], "[the] %l", ACTION_CONTACT);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Pet whom [how]?\n");
        return 0;
    }

    actor("You pet", oblist, how[1] + " on the head.");
    all2act(" pets", oblist, how[1] + " on the head.", how[1], ACTION_CONTACT);
    target(" pets you" + how[1] + " on the head.", oblist, how[1],
        ACTION_CONTACT);
    return 1;
}

int
pinch(string str)
{
    object *oblist;
    string *zones;
    string *parts;
    string location;
    int    size;

    zones = ({ "cheek", "ear", "nose", "arm", "bottom" });

    str = (strlen(str) ? lower_case(str) : "arm");
    if (member_array(str, zones) != -1)
    {
        write("You pinch yourself in your " + str + ".\n");
        all(" pinches " + this_player()->query_objective() +
           "self in " + this_player()->query_possessive() + " " + str + ".");
        return 1;
    }

    parts = explode(str, " ");
    if ((size = sizeof(parts)) > 1)
    {
        if (member_array(parts[size - 1], zones) != -1)
        {
            location = parts[size - 1];
            str = implode(parts[..(size - 2)], " ");
        }
    }

    if (!stringp(location))
    {
        location = "cheek";
    }

    oblist = parse_this(str, "[the] %l [in] [the]", ACTION_CONTACT);
    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Pinch [whom] [where]?\n");
        return 0;
    }

    actor("You pinch", oblist, "'s " + location + ".");
    all2act(" pinches", oblist, "'s " + location + ".", "", ACTION_CONTACT);
    target(" pinches your " + location + ".", oblist, "", ACTION_CONTACT);
    return 1;
}

#define POINT_DIRECTIONS ({ "up", "down", "north", "south", "west", "east", \
    "northwest", "southwest", "northeast", "southeast" })

int
point(string str)
{
    object *oblist;
    string *tmp;

    notify_fail("Where do you want to point?\n");

    if (!stringp(str))
    {
        write("You point in a general direction.\n");
        allbb(" points in a general direction.");
        return 1;
    }

    str = lower_case(str);
    if (member_array(str, POINT_DIRECTIONS) >= 0)
    {
        write("You point " + str + ".\n");
        allbb(" points " + str + ".");
        return 1;
    }

    oblist = parse_this(str, "[at] [the] %i", 0, 1);

    if (!sizeof(oblist))
    {
        tmp = explode(str, " ");
        if (sizeof(tmp) > 1 && tmp[0] == "at")
            str = implode(tmp[1 .. sizeof(tmp) - 1], " ");
        oblist = FIND_STR_IN_OBJECT(str, environment(this_player()));
        if (!sizeof(oblist))
        {
            if (environment(this_player())->item_id(str))
            {
                write("You point at the " + str + ".\n");
                allbb(" points at " + LANG_ADDART(str) + ".");
                return 1;
            }
            return 0;
        }

	if (oblist[0] != this_player())
	{
	    write("You point at " + LANG_THESHORT(oblist[0]) + ".\n");
	    allbb(" points at " + LANG_THESHORT(oblist[0]) + ".");
	    return 1;
	}
    }

    if (oblist[0] == this_player())
    {
        write("You point at yourself.\n");
        allbb(" points at " + this_player()->query_objective() + "self.");
        return 1;
    }

    actor("You point at", oblist);
    all2actbb(" points at", oblist);
    targetbb(" points at you.", oblist);
    return 1;
}

int
poke(string str)
{
    object *oblist;
    string *zones;
    string *parts;
    string location;
    int    size;

    zones = ({ "eye", "ear", "nose", "thorax", "abdomen", "shoulder", "ribs" });

    str = (strlen(str) ? lower_case(str) : "abdomen");
    if (member_array(str, zones) != -1)
    {
        write("You poke yourself in your " + str + ".\n");
        all(" pokes " + this_player()->query_objective() +
            "self in " + this_player()->query_possessive() + " " + str + ".");
        return 1;
    }

    parts = explode(str, " ");
    if ((size = sizeof(parts)) > 1)
    {
        if (member_array(parts[size - 1], zones) != -1)
        {
            location = parts[size - 1];
            str = implode(parts[..(size - 2)], " ");
        }
    }

    if (!stringp(location))
    {
        location = "ribs";
    }

    oblist = parse_this(str, "[the] %l [in] [the]", ACTION_CONTACT);
    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Poke [whom] [where]?\n");
        return 0;
    }

    actor("You poke", oblist, " in the " + location + ".");
    all2act(" pokes", oblist, " in the " + location + ".", "", ACTION_CONTACT);
    target(" pokes you in the " + location + ".", oblist, "", ACTION_CONTACT);
    return 1;
}

int
ponder(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        SOULDESC("pondering the situation");
        write("You ponder the situation.\n");
        all(" ponders the situation.");
        return 1;
    }

    if ((strlen(str) > 60) &&
        (!(this_player()->query_wiz_level())))
    {
        SOULDESC("pondering the situation");
        write("You ponder beyond the end of the line and wake up from " +
            "your reveries.\n");
        all(" ponders the situation.");
        return 1;
    }

    oblist = parse_this(str, "[about] [the] [proposal] [of] [the] %l");

    if (!sizeof(oblist))
    {
        SOULDESC("pondering about something");
        write("You ponder " + str + "\n");
        all(" ponders " + str);
        return 1;
    }

    SOULDESC("pondering about a proposal");
    actor("You ponder about the proposal of", oblist);
    all2act(" ponders about the proposal of", oblist);
    target(" ponders about your proposal.", oblist);
    return 1;    
}

int
pounce(string str)
{
    object *oblist;
    string pos;
    string obj;

    oblist = parse_this(str, "[on] [the] %l", ACTION_CONTACT);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Pounce on whom?\n");
        return 0;
    }

    pos = ((sizeof(oblist) > 1) ? "their" : oblist[0]->query_possessive());
    obj = ((sizeof(oblist) > 1) ? "them" : oblist[0]->query_objective());
    actor("You pounce on", oblist, " like a cat, knocking " + obj +
        " flat on " + pos + " back!");
    all2act(" pounces on", oblist, " like a cat, knocking " + obj +
        " flat on " + pos + " back!", "", ACTION_CONTACT);
    target(" pounces on you like a cat, knocking you flat on your back.",
        oblist, "", ACTION_CONTACT);
    return 1;
}

int
pout(string str)
{
    str = check_adverb_with_space(str, "petulantly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Pout how?\n");
        return 0;
    }

    SOULDESC("pouting" + str);
    write("You pout" + str + ".\n");
    all(" pouts" + str + ".", str);
    return 1;
}

int
puke(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        write("You puke on your shoes.\n");
        all(" doubles over and pukes.");
        return 1;
    }

    oblist = parse_this(str, "[on] / [over] [the] %i", 
        ACTION_PROXIMATE | ACTION_OFFENSIVE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Puke on whom/what?\n");
        return 0;
    }

    actor("You puke on", oblist);
    all2act(" pukes on", oblist, "", "",
        ACTION_PROXIMATE | ACTION_OFFENSIVE);
    target(" pukes all over you!", oblist, "",
        ACTION_PROXIMATE | ACTION_OFFENSIVE);
    return 1;
}

int
purr(string str)
{
    str = check_adverb_with_space(str, "contentedly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Purr how?\n");
        return 0;
    }

    SOULDESC("purring" + str);
    write("MMMMEEEEEEEEOOOOOOOWWWWWWW! You purr" + str + ".\n");
    all(" purrs" + str + ".", str, ACTION_AURAL);
    return 1;
}

int
roar(string str)
{
    str = check_adverb_with_space(str, "aggressively");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Roar how?\n");
        return 0;
    }

    SOULDESC("roaring" + str);
    write("RRRRROOOOOOAAAAAWWWWWW! You roar" + str + ".\n");
    all(" roars" + str + ".", str, ACTION_AURAL);
    return 1;
}

int
rolleyes(string str)
{
    str = check_adverb_with_space(str, "in exasperation");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Roll your eyes how?\n");
        return 0;
    }

    write("You roll your eyes" + str + ".\n");
    all(" rolls " + this_player()->query_possessive() + " eyes" +
        str + ".", str);
    return 1;
}

int
ruffle(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "playfully", 1);

    oblist = parse_this(how[0], "[the] %l", ACTION_CONTACT);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Ruffle whom [how]?\n");
        return 0;
    }

    actor("You ruffle", oblist, "'s hair" + how[1] + ".");
    all2act(" ruffles", oblist, "'s hair" + how[1] + ".", how[1],
        ACTION_CONTACT);
    target(" ruffles your hair" + how[1] + ".", oblist, how[1],
        ACTION_CONTACT);
    return 1;
}

int
scold(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        write("You scold at no-one in particular.\n");
        all(" scolds at no-one in particular.\n");
        return 1;
    }

    oblist = parse_this(str, "[the] %i", ACTION_OFFENSIVE | ACTION_AURAL);

    if (!sizeof(oblist))
    {
        notify_fail("Scold the hell out of whom/what?\n");
        return 0;
    }

    actor("You scold the hell out of", oblist);
    all2act(" scolds the hell out of", oblist, "", "",
        ACTION_OFFENSIVE | ACTION_AURAL);
    target(" scolds the hell out of you!", oblist, "",
        ACTION_OFFENSIVE | ACTION_AURAL);
    return 1;
}

int
scowl(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "menacingly", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("scowling" + how[1]);
        write("You scowl" + how[1] + ".\n");
        allbb(" scowls" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Scowl [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("scowling" + how[1]);
    actor("You scowl" + how[1] + " at", oblist);
    all2actbb(" scowls" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" scowls" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
scratch(string str)
{
    object *oblist;
    string *zones;
    string *parts;
    string location;
    int    size;

    zones = ({ "head", "chin", "back", "behind", "nose", "ear" });

    str = (strlen(str) ? lower_case(str) : "head");
    if (member_array(str, zones) != -1)
    {
        write("You scratch your " + str + ".\n");
        allbb(" scratches " + this_player()->query_possessive() +
            " " + str + ".");
        return 1;
    }

    parts = explode(str, " ");
    if ((size = sizeof(parts)) > 1)
    {
        if (member_array(parts[size - 1], zones) != -1)
        {
            location = parts[size - 1];
            str = implode(parts[..(size - 2)], " ");
        }
    }

    if (!stringp(location))
    {
        location = "head";
    }

    oblist = parse_this(str, "[the] %l [at] [the]", ACTION_CONTACT);

    if (!sizeof(oblist))
    {
        notify_fail("Scratch [whom] [where]?\n");
        return 0;
    }

    actor("You scratch", oblist, "'s " + location + ".");
    all2act(" scratches", oblist, "'s " + location + ".", "", ACTION_CONTACT);
    target(" scratches your " + location + ".", oblist, "", ACTION_CONTACT);
    return 1;
}

int
scream(string str)
{
    object *rooms, troom;
    int    index;
    int    size;

    if (stringp(str))
    {
        notify_fail("Scream what?\n");
        return 0;
    }

    if (!objectp(troom = environment(this_player())))
    {
        return 0;
    }

    rooms = find_neighbour( ({ }), ({ troom }), DEPTH) - ({ troom });

    index = -1;
    size = sizeof(rooms);
    while(++index < size)
    {
        tell_room(rooms[index], "@@shout_name:" + CMD_LIVE_SPEECH +
            "@@ screams loudly!\n", this_player());
    }

    all(" screams loudly. ARRGGGGGGHHHHHH!!!!");
    write("ARRGGGGGGHHHHHH!!!! Now that is what I call a scream!\n");
    return 1;
}

int
shake(string str)
{
    object *oblist;
    int attrs = ACTION_VISUAL | ACTION_LACTIVITY;

    if (!stringp(str))
    {
        write("You shake your head in disagreement.\n");
        allbb(" shakes " + this_player()->query_possessive() +
              " head in disagreement.", attrs);
        return 1;
    }

    if (wildmatch("head *", str))
    {
        oblist = parse_this(str, "[head] [at] [the] %i", attrs);

        if (!sizeof(oblist))
        {
            notify_fail("Shake your head in disagreement at whom/what?\n");
            return 0;
        }

        actor("You shake your head in disagreement at", oblist);
        all2act(" shakes " + this_player()->query_possessive() +
            " head in disagreement at", oblist, "", "", attrs);
        target(" shakes " + this_player()->query_possessive() +
            " head in disagreement at you.", oblist, "", attrs);
        return 1;
    }

    oblist = parse_this(str, "[hand] [hands] [with] [the] %l", attrs);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Shake hands with whom?\n");
        return 0;
    }

    actor("You shake hands with", oblist);
    all2act(" shakes", oblist, "'s hand.", "", attrs);
    target(" shakes your hand.", oblist, "", attrs);
    return 1;
}

int
shiver(string str)
{
    str = check_adverb_with_space(str, "from the cold");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Shiver how?\n");
        return 0;
    }

    SOULDESC("shivering" + str);
    write("You shiver" + str + ".\n");
    all(" shivers" + str + ".", str);
    return 1;
}

int
show(string str)
{
    string who;
    object *oblist;
    object obj;
    int full;
    int covertly = 0;
    int action = 0;

    if (!strlen(str))
    {
        notify_fail("Show what [to whom]?\n");
        return 0;
    }

    /* Show the long description to the onlookers. */
    if (full = wildmatch("full *", str))
    {
        str = extract(str, 5);
    }

    if (sscanf(str, "%s covertly to %s", str, who) == 2)
    {
        covertly = 1;
        action = ACTION_PROXIMATE;
    }
    else
    {
        sscanf(str, "%s to %s", str, who);
    }

    parse_command(lower_case(str), all_inventory(this_player()),
        "[the] %i", oblist);

    oblist = NORMAL_ACCESS(oblist, 0, 0);

    switch(sizeof(oblist))
    {
    case 0:
        notify_fail("Show what [to whom?]\n");
        return 0;

    case 1:
        obj = oblist[0];
        break;

    default:
        notify_fail("Please show only one item at a time.\n");
        return 0;
    }

    if (!strlen(who))
    {
        write("You show your " + LANG_SHORT(obj) + " around" +
            (full ? " in full" : "") + ".\n");
        str = " shows " + this_player()->query_possessive() + " " +
            LANG_SHORT(obj) + " around.\n" + (full ? obj->long() : "");
        say( ({ (this_player()->query_name() + str),
            ("The " + this_player()->query_nonmet_name() + str) , "" }) );
        FILTER_OTHER_LIVE(all_inventory(environment(this_player())))->show_hook(obj);
        return 1;
    }

    oblist = parse_this(who, "[the] %l", action);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Show the " + obj->short() +
            (covertly ? " covertly" : "") + " to whom?\n");
        return 0;
    }

    actor("You show your " + LANG_SHORT(obj) + (covertly ? " covertly" : "") +
        " to", oblist, (full ? " in full." : ""));
    all2actbb((covertly ? " covertly" : "") + " shows " +
        (covertly ? "something" : (this_player()->query_possessive() +
         " " + LANG_SHORT(obj))) + " to", oblist, "", action);
    targetbb((covertly ? " covertly" : "") + " shows you " +
        this_player()->query_possessive() + " " + LANG_SHORT(obj) + ".", oblist,
        "", action);
    oblist->show_hook(obj);

    if (full)
    {
        oblist->catch_tell(obj->long());
    }

    return 1;
}

int
shrug(string str)
{
    str = check_adverb_with_space(str, "helplessly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Shrug how?\n");
        return 0;
    }

    write("You shrug" + str + ".\n");
    allbb(" shrugs" + str + ".", str, ACTION_VISUAL | ACTION_LACTIVITY);
    return 1;
}

int
shudder(string str)
{
    if (!stringp(str))
    {
        SOULDESC("shuddering");
        write("You shudder at the thought.\n");
        all(" shudders at the thought.");
        return 1;
    }

    if ((strlen(str) > 60) &&
        (!(this_player()->query_wiz_level())))
    {
        SOULDESC("shuddering");
        write("That is an afwul lot to shudder to think at one time.\n");
        all(" shudders to think a complicated thought.");
        return 1;
    }

    SOULDESC("shuddering");
    write("You shudder to think that " + str + "\n");
    all(" shudders to think that " + str);
    return 1;
}

int
sigh(string str)
{
    str = check_adverb_with_space(str, "deeply");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Sigh how?\n");
        return 0;
    }

    SOULDESC("sighing" + str);
    write("You sigh" + str + ".\n");
    all(" sighs" + str + ".", str, ACTION_AURAL);
    return 1;
}

int
sing(string str)
{
    if (stringp(str))
    {
        notify_fail("Sing what?\n");
        return 0;
    }

    write("You sing a merry tune.\n");
    all(" sings a merry tune.", "", ACTION_AURAL);
    return 1;
}

int
slap(string str)
{
    object *oblist;
    int attrs = ACTION_CONTACT | ACTION_VISUAL | ACTION_AURAL |
        ACTION_MACTIVITY | ACTION_OFFENSIVE;

    oblist = parse_this(str, "[the] %i", attrs, 1);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Slap whom/what?\n");
        return 0;
    }

    if (oblist[0] == this_player())
    {
        write("You slap yourself.\n");
        all(" slaps " + this_player()->query_objective() + "self.", "", attrs);
        return 1;
    }

    actor("You slap", oblist);
    all2act(" slaps", oblist, "", attrs);
    target(" slaps you!", oblist, "", attrs);
    return 1;
}

int
smell(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "in a subtle manner", 1);

    if (!stringp(how[0]))
    {
        write("You smell the area around you" + how[1] + ".\n");
        allbb(" smells the area around " + this_player()->query_objective() +
            how[1] + ".", how[1]);
        environment(this_player())->hook_smelled();
        return 1;
    }

    oblist = parse_this(how[0], "[the] %i");

    switch(sizeof(oblist))
    {
    case 0:
        if (environment(this_player())->item_id(how[0]))
        {
            write("You smell the " + how[0] + how[1] + ".\n");
            allbb(" smells " + LANG_ADDART(how[0]) + how[1] + ".");
            environment(this_player())->hook_smelled(how[0]);
            return 1;
        }

        notify_fail("Smell whom/what [how]?\n");
        return 0;

    case 1:
        break;

    default:
        notify_fail("Please try smelling only one thing at a time.\n");
        return 0;
    }

    actor("You smell", oblist, how[1] + ".");
    all2actbb(" smells", oblist, how[1] + ".", how[1]);
    targetbb(" smells you" + how[1] + ".", oblist, how[1]);
    oblist->hook_smelled();
    return 1;
}

int
smile(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "happily", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("smiling" + how[1]);
        write("You smile" + how[1] + ".\n");
        allbb(" smiles" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Smile [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("smiling" + how[1]);
    actor("You smile" + how[1] + " at", oblist);
    all2actbb(" smiles" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" smiles" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
smirk(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, BLANK_ADVERB, 0);

    if (!stringp(how[0]))
    {
        SOULDESC("smirking" + how[1]);
        write("You smirk" + how[1] + ".\n");
        all(" smirks" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Smirk [how] at whom/what?\n");
        return 0;
    }

    actor("You smirk" + how[1] + " at", oblist, ".");
    all2act(" smirks" + how[1] + " at", oblist, 0, how[1]);
    target(" smirks" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
snap(string str)
{
    if (stringp(str))
    {
        notify_fail("Snap what?\n");
        return 0;
    }

    write("You snap your fingers.\n");
    all(" snaps " + this_player()->query_possessive() + " fingers.");
    return 1;
}

int
snarl(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "savagely", 0);

    oblist = parse_this(how[0], "[at] [the] %i", ACTION_AURAL);

    if (!sizeof(oblist))
    {
        notify_fail("Snarl [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("snarling" + how[1]);
    actor("You snarl" + how[1] + " at", oblist);
    all2act(" snarls" + how[1] + " at", oblist, 0, how[1], ACTION_AURAL);
    target(" snarls" + how[1] + " at you.", oblist, how[1], ACTION_AURAL);
    return 1;
}

int
sneer(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "contemptuously", 0);

    if (!strlen(how[0]))
    {
        SOULDESC("sneering" + how[1]);
        write("You sneer" + how[1] + ".\n");
        allbb(" sneers" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Sneer [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("sneering" + how[1]);
    actor("You sneer" + how[1] + " at", oblist,".");
    all2actbb(" sneers" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" sneers" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

int
sneeze(string str)
{
    if (stringp(str))
    {
        notify_fail("Sneeze what?\n");
        return 0;
    }

    write("You sneeze!\n");
    all(" sneezes!", "", ACTION_AURAL | ACTION_VISUAL);
    return 1;
}

int
snicker(string str)
{
    if (stringp(str))
    {
        notify_fail("Snicker what?\n");
        return 0;
    }

    SOULDESC("snickering");
    write("You snicker.\n");
    all(" snickers.", "", ACTION_AURAL);
    return 1;
}

int
sniff(string str)
{
    str = check_adverb_with_space(str, "pitifully");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Sniff how?\n");
        return 0;
    }

    SOULDESC("sniffing" + str);
    write("You sniff" + str + ".\n");
    all(" sniffs" + str + ".", str);
    return 1;
}

int
snore(string str)
{
    str = check_adverb_with_space(str, "loudly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Snore how?\n");
        return 0;
    }

    SOULDESC("snoring" + str);
    write("You snore" + str + ". Zzzzzzzzzz...\n");
    all(" snores" + str + ".", str, ACTION_AURAL);
    return 1;
}

int
snuggle(string str)
{
    object *oblist;
    int attrs = ACTION_CONTACT | ACTION_INTIMATE | ACTION_LACTIVITY;

    oblist = parse_this(str, "[with] / [up] [to] [the] %i", attrs);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Snuggle up to whom/what?\n");
        return 0;
    }

    actor("You snuggle up to", oblist);
    all2act(" snuggles up to", oblist, "", attrs);
    target(" snuggles up to you.", oblist, "", attrs);
    return 1;
}

int
sob(string str)
{
    str = check_adverb_with_space(str, "sadly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Sob how?\n");
        return 0;
    }

    SOULDESC("sobbing" + str);
    write("You sob" + str + ".\n");
    all(" sobs" + str + ".", str, ACTION_VISUAL | ACTION_AURAL);
    return 1;
}

int
spank(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "playfully", 1);

    oblist = parse_this(how[0], "[the] %i", ACTION_CONTACT);

    if (!sizeof(oblist))
    {
        notify_fail("Spank whom/what [how]?\n");
        return 0;
    }

    actor("You spank", oblist, how[1] + ".");
    all2act(" spanks", oblist, how[1] + ".", how[1], ACTION_CONTACT);
    target(" spanks you" + how[1] + ".", oblist, how[1], ACTION_CONTACT);
    return 1;
}

int
spit(string str)
{
    object *oblist;
    int attrs;

    if (!stringp(str))
    {
        write("You spit on the ground in disgust.\n");
        all(" spits on the ground in disgust.", ACTION_OTHER);
        return 1;
    }

    attrs = ACTION_PROXIMATE | ACTION_OFFENSIVE | ACTION_VISUAL;

    oblist = parse_this(str, "[on] [the] %i", attrs);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Spit at whom/what?\n");
        return 0;
    }

    actor("You spit on", oblist);
    all2act(" spits on", oblist, "", attrs);
    target(" spits on you!", oblist, "", attrs);
    return 1;
}

int
squeeze(string str)
{
    object *oblist;
    string *how;
    int attrs = ACTION_CONTACT | ACTION_MACTIVITY | ACTION_VISUAL;

    how = parse_adverb_with_space(str, "fondly", 1);

    oblist = parse_this(how[0], "[the] %i", attrs);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Squeeze whom/what [how]?\n");
        return 0;
    }

    actor("You squeeze", oblist, how[1] + ".");
    all2act(" squeezes", oblist, how[1] + ".", how[1], attrs);
    target(" squeezes you" + how[1] + ".", oblist, attrs);
    return 1;
}

int
squirm(string str)
{
    str = check_adverb_with_space(str, "uncomfortably");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Squirm how?\n");
        return 0;
    }

    write("You squirm" + str + ".\n");
    all(" squirms" + str + ".", str, ACTION_MACTIVITY | ACTION_VISUAL);
    return 1;
}

int
stare(string str)
{
    object *oblist;
    string *how;

    if (!stringp(str))
    {
        SOULDESC("staring into space");
        write("You stare into space.\n");
        allbb(" stares into space.");
        return 1;
    }

    how = parse_adverb_with_space(str, "dreamily", 0);

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Stare at whom/what?\n");
        return 0;
    }

    SOULDESC("staring" + how[1]);
    actor("You stare" + how[1] + " at", oblist);
    all2actbb(" stares" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" stares" + how[1]+ " into your eyes.", oblist, how[1]);
    return 1;
}

int
startle(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %l");

    if (!sizeof(oblist))
    {
        notify_fail("Startle whom?\n");
        return 0;
    }

    actor("You startle", oblist);
    all2act(" startles", oblist);
    target(" startles you.", oblist);
    return 1;
}

int
steam(string str)
{
    if (stringp(str))
    {
        notify_fail("Steam what?\n");
        return 0;
    }

    write("Steam comes boiling out of your ears.\n");
    allbb("'s face turns purple and steam starts to boil out of " +
       this_player()->query_possessive() + " ears.", "", ACTION_VISUAL);
    return 1;
}

int
stick(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, BLANK_ADVERB, 0);

    if (!stringp(how[0]))
    {
        write("You" + how[1] + " stick your tongue out.\n");
        all(how[1] + " sticks " + this_player()->query_possessive() +
            " tongue out.", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i",
       ACTION_VISUAL | ACTION_OFFENSIVE);

    if (!sizeof(oblist))
    {
        notify_fail("Stick your tongue out [how] at whom/what?\n");
        return 0;
    }

    actor("You" + how[1] + " stick your tongue out at", oblist, ".");
    all2act(how[1] + " sticks " + this_player()->query_possessive() +
        " tongue out at", oblist, 0, how[1], ACTION_VISUAL | ACTION_OFFENSIVE);
    target(how[1] + " sticks " + this_player()->query_possessive() +
        " tongue out at you.", oblist, how[1],
        ACTION_VISUAL | ACTION_OFFENSIVE);
    return 1;
}

int
stomp(string str)
{
    if (stringp(str))
    {
        notify_fail("Stomp what?\n");
        return 0;
    }

    write("You stomp your feet angrily on the ground.\n");
    all(" stomps " + this_player()->query_possessive() + " feet angrily " +
        "on the ground.", ACTION_MACTIVITY | ACTION_VISUAL | ACTION_AURAL);
    return 1;
}

int
strangle(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "angrily", 1);

    oblist = parse_this(how[0], "[the] %l",
        ACTION_CONTACT | ACTION_MACTIVITY | ACTION_OFFENSIVE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Strangle whom/what [how]?\n");
        return 0;
    }

    actor("You strangle", oblist, how[1] + ".");
    all2act(" strangles", oblist, how[1] + ".", how[1],
        ACTION_CONTACT | ACTION_MACTIVITY | ACTION_OFFENSIVE);
    target(" strangles you" + how[1] + ".", oblist, how[1],
        ACTION_CONTACT | ACTION_MACTIVITY | ACTION_OFFENSIVE);
    return 1;
}

int
stretch(string str)
{
    if (stringp(str))
    {
        notify_fail("Stretch what?\n");
        return 0;
    }

    write("You stretch your tired body out.\nAhh, that feels good.\n");
    allbb(" stretches " + this_player()->query_possessive() + " tired body.",
        ACTION_MACTIVITY);
    return 1;
}

int
strut(string str)
{
    str = check_adverb_with_space(str, "proudly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Strut how?\n");
        return 0;
    }

    write("You strut your stuff" + str + "!\n");
    all(" struts" + str + ".", str, ACTION_MACTIVITY);
    return 1;
}

int
stumble(string str)
{
    if (stringp(str))
    {
        notify_fail("Just stumbling will suffice.\n");
        return 0;
    }

    write("You stumble and fall to the ground.\n");
    all(" stumbles and falls to the ground.");
    return 1;
}

int
sulk(string str)
{
    if (stringp(str))
    {
        notify_fail("Sulk what?\n");
        return 0;
    }

    write("You sulk in the corner.\n");
    allbb(" sulks in the corner.");
    return 1;
}

int
swallow(string str)
{
    str = check_adverb_with_space(str, "uncomfortably");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Swallow how?\n");
        return 0;
    }

    write("You swallow" + str + ".\n");
    allbb(" swallows" + str + ".", str);
    return 1;
}

int
swear(string str)
{
    str = check_adverb_with_space(str, "loudly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Swear how?\n");
        return 0;
    }

    write("You swear" + str + ". Go wash your mouth.\n");
    all(" swears" + str + ".", str, ACTION_OFFENSIVE | ACTION_AURAL);
    return 1;
}

int
sweat(string str)
{
    str = check_adverb_with_space(str, "profusely");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Sweat how?\n");
        return 0;
    }

    SOULDESC("sweating" + str);
    write("Is it getting hot in here, that you have to sweat" + str + "?\n");
    all(" begins to sweat" + str + ".", str);
    return 1;
}

int
swoon(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "wistfully", 0);
    if (!stringp(how[0]))
    {
        write("You swoon" + how[1] + " and pass out momentarily as you " +
            "collapse to the ground.\n");
        all(" swoons" + how[1] + " and passes out momentarily as " +
            this_player()->query_pronoun() + " collapses to the ground.",
            how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[over] [the] %l", ACTION_CONTACT );
    if (!sizeof(oblist))
    {
        notify_fail("Swoon [how] [over whom]?\n");
        return 0;
    }

    actor("You swoon" + how[1] + " and pass out momentarily into the arms " +
        "of", oblist, ".");
    all2act(" swoons" + how[1] + " and passes out momentarily into the arms " +
        "of", oblist, 0, how[1], ACTION_CONTACT);
    target(" swoons" + how[1] + " and passes out momentarily into your arms.",
        oblist, how[1], ACTION_CONTACT);
    return 1;
}

int
tackle(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %l", ACTION_CONTACT | ACTION_HACTIVITY);
    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Tackle whom?\n");
        return 0;
    }

    if (random(7))
    {
        actor("You tackle", oblist);
        all2act(" tackles", oblist, ". " +
            ((sizeof(oblist) > 1) ? "They fall": "The latter falls") +
            " to the ground in a very unflattering way.", "",
            ACTION_CONTACT | ACTION_HACTIVITY);
        target(" comes running at you. " +
            capitalize(this_player()->query_pronoun()) +
            " attempts to tackle you and succeeds. You fall to the ground " +
            "in a very unflattering way.", oblist, "",
            ACTION_CONTACT | ACTION_HACTIVITY);
    }
    else
    {
        actor("You try to tackle", oblist, " but fall flat on your face.");
        all2act(" tries to tackle", oblist, " but misses and falls flat on " +
            this_player()->query_possessive() + " face.", "",
            ACTION_CONTACT | ACTION_HACTIVITY);
        target(" comes running at you. " +
            capitalize(this_player()->query_pronoun()) +
            " attempts to tackle you but misses and falls flat on " +
            this_player()->query_possessive() + " face.", oblist, "",
            ACTION_CONTACT | ACTION_HACTIVITY);
    }

    return 1;
}

int
tap(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, NO_DEFAULT_ADVERB, 0);

    if (!stringp(how[0]))
    {
        if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
        {
            how[1] = ADD_SPACE_TO_ADVERB("impatiently");
        }

        write("You tap your foot" + how[1] + ".\n");
        all(" taps " + this_player()->query_possessive() + " foot" + how[1] +
            ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[on] [the] %l", ACTION_CONTACT);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Tap [whom] [how]?\n");
        return 0;
    }

    if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        how[1] = NO_ADVERB;
    }

    this_player()->reveal_me(1);
    str = ((sizeof(oblist) == 1) ? oblist[0]->query_possessive() : "their");
    actor("You tap", oblist, how[1] + " on the shoulder to attract " + str +
        " attention.");
    all2act(" taps", oblist, how[1] + " on the shoulder to attract " + str +
        " attention.", how[1], ACTION_CONTACT);
    target(" taps you" + how[1] +
        " on the shoulder to attract your attention.", oblist, how[1],
        ACTION_CONTACT);
    oblist->reveal_me(1);

    return 1;
}

int
tease(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "playfully", 1);

    oblist = parse_this(how[0], "[the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Tease whom/what" + how[1] + "?\n");
        return 0;
    }

    actor("You tease", oblist, how[1] + ".");
    all2actbb(" teases", oblist, how[1] + ".", how[1]);
    targetbb(" teases you" + how[1] + ".", oblist, how[1]);
    return 1;
}

int
thank(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "gratefully", 1);

    oblist = parse_this(how[0], "[the] %i", ACTION_INGRATIATORY);

    if (!sizeof(oblist))
    {
        notify_fail("Thank whom/what [how]?\n");
        return 0;
    }

    actor("You thank", oblist, how[1] + ".");
    all2act(" thanks", oblist, how[1] + ".", how[1], ACTION_INGRATIATORY);
    target(" thanks you" + how[1] + ".", oblist, how[1], ACTION_INGRATIATORY);
    return 1;
}

int
think(string str)
{
    if (!stringp(str))
    {
        write("You try to look thoughtful but fail.\n");
        allbb(" tries to look thoughtful but fails.");
        SOULDESC("trying to look thoughtful");
        return 1;
    }

    if ((strlen(str) > 60) &&
        (!(this_player()->query_wiz_level())))
    {
        write("Geez.. That is a lot to think about at the same time.\n");
        allbb(" looks like " + this_player()->query_pronoun() +
            " is trying to think hard about a lot of things.");
        SOULDESC("thinking about a lot of things");
        return 1;
    }

    write("You think hard about " + str + "\n");
    allbb(" looks like " + this_player()->query_pronoun() +
        " is thinking hard about " + str);
    SOULDESC("thinking hard about something");
    return 1;
}

int
threaten(string str)
{
    object *oblist;
    string *how;

    if (!strlen(str))
    {
        str = "";
    }

    if (sizeof(how = explode(str, " with ")) == 2)
    {
        how[1] = " with " + how[1];

        if ((strlen(how[1]) > 60) &&
            (!(this_player()->query_wiz_level())))
        {
            SOULDESC("threatening everyone and everything");
            write("You threaten beyond the end of the line and become incoherent.\n");
            all(" threatens everyone with everything.");
            return 1;
        }
    }
    else
    {
        how = parse_adverb_with_space(str, BLANK_ADVERB, 1);
    }

    oblist = parse_this(how[0], "[the] %i", ACTION_AURAL | ACTION_OFFENSIVE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Threaten whom/what [how]?\n");
        return 0;
    }

    actor("You threaten", oblist, how[1] + ".");
    all2act(" threatens", oblist, how[1] + ".", how[1],
        ACTION_AURAL | ACTION_OFFENSIVE);
    target(" threatens you" + how[1] + ".", oblist, how[1],
        ACTION_AURAL | ACTION_OFFENSIVE);
    return 1;
}

int
thumb(string str)
{
    object *oblist;
    string *words;
    string direction = "upwards";

    if (strlen(str))
    {
        words = explode(str, " ");
        switch(words[0])
        {
        case "down":
        case "downwards":
            direction = "downwards";
            words = words[1..];
            break;

        case "up":
        case "upwards":
            direction = "upwards";
            words = words[1..];
            break;
        }

        str = implode(words, " ");
    }

    if (!strlen(str))
    {
        write("You hold your thumb " + direction + ".\n");
        allbb(" holds " + this_player()->query_possessive() + " thumb " +
            direction + ".");
        return 1;
    }

    oblist = parse_this(str, "[at] [the] %l");

    if (!sizeof(oblist))
    {
        notify_fail("Hold your thumb " + direction + " at whom?\n");
        return 0;
    }

    actor("You hold your thumb " + direction + " at", oblist);
    all2actbb(" holds " + this_player()->query_possessive() + " thumb " +
        direction + " at", oblist);
    targetbb(" holds " + this_player()->query_possessive() + " thumb " +
        direction + " at you.", oblist);
    return 1;
}

int
tickle(string str)
{
    object *oblist;
    string *how;
    string location;
    int    attrs;

    attrs = (ACTION_CONTACT | ACTION_VISUAL | ACTION_MACTIVITY | ACTION_INTIMATE);

    oblist = parse_this(str, "[the] %l [all] [in] [under] [the] 'feet' / " +
        "'foot' / 'chin' / 'neck' / 'abdomen' / 'belly' / 'tummy' / " +
        "'side' / 'nose' / 'over'", attrs);
    if (sizeof(oblist))
    {
        how = explode(str, " ");
        switch(location = how[sizeof(how) - 1])
        {
        case "over":
            target(" tickles you all over.", oblist, "", attrs);
            actor("You tickle", oblist, " all over.");
            all2act(" tickles", oblist, " all over.", "", attrs);
            return 1;

        case "nose":
            target(" tickles your nose.", oblist, "", attrs);
            actor("You tickle", oblist, "'s nose.");
            all2act(" tickles", oblist, "'s nose.", "", attrs);
            return 1;

        case "neck":
            target(" tickles you in the back of your neck.", oblist, "", attrs);
            actor("You tickle", oblist, " in the back of the neck.");
            all2act(" tickles", oblist, " in the back of the neck.", "", attrs);
            return 1;

        case "chin":
        case "feet":
        case "foot":
            target(" tickles you under your " + location + ".", oblist, "", attrs);
            actor("You tickle", oblist, " under the " + location + ".");
            all2act(" tickles", oblist, " under the " + location + ".", "", attrs);
            return 1;

        case "abdomen":
        case "belly":
        case "side":
        case "tummy":
            oblist->add_prop(LIVE_S_SOULEXTRA, "giggling merrily");
            target(" tickles you in your " + location + ".", oblist, "", attrs);
            actor("You tickle", oblist, " in the " + location + ".");
            all2act(" tickles", oblist, " in the " + location + ".", "", attrs);
            return 1;

        default:
            /* Intentional fallthrough in case it is a special parse name.*/
            if (!IN_ARRAY(str, PARSE_SPECIAL_NAMES))
            {
                write("Tickle whom [where / how]? Rather... this should " +
                    "not happen. Please make a sysbugreport about this.\n");
                return 1;
            }
        }
        /* No return 1 for the fallthrough. */
    }

    if (strlen(parse_msg))
    {
        write(parse_msg);
        return 1;
    }

    how = parse_adverb_with_space(str, "playfully", 1);

    oblist = parse_this(how[0], "[the] %l", attrs);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Tickle whom [how / where]?\n");
        return 0;
    }

    actor("You tickle", oblist, how[1] + ".");
    all2act(" tickles", oblist, how[1] + ".", how[1], attrs);
    target(" tickles you" + how[1] + ".", oblist, how[1], attrs);

    return 1;
}

int
tingle(string str)
{
    if (stringp(str))
    {
        notify_fail("Tingle how?\n");
        return 0;
    }

    write("You tingle with excitement.\n");
    allbb(" tingles with excitement.");
    return 1;
}

public int
touch(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %i", ACTION_CONTACT | ACTION_LACTIVITY);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Touch whom/what?\n");
        return 0;
    }

    actor("You touch", oblist, ".");
    all2act(" touches", oblist, ".", 0, ACTION_CONTACT | ACTION_LACTIVITY);
    target(" touches you.", oblist, 0, ACTION_CONTACT | ACTION_LACTIVITY);
    return 1;
}

int
tremble(string str)
{
    str = check_adverb_with_space(str, NO_DEFAULT_ADVERB);

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Whimper how?\n");
        return 0;
    }

    if (str == NO_DEFAULT_ADVERB_WITH_SPACE)
    {
        SOULDESC("trembling and quivering like a bowlful of jelly");
        write("You tremble in your boots, quivering like a bowlful of " +
            "jelly.\n");
        allbb(" trembles and quivers like a bowlful of jelly.");
        return 1;
    }

    SOULDESC("trembling" + str);
    write("You tremble" + str + ".\n");
    all(" trembles" + str + ".", str);
    return 1;
}

int
trust(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "implicitly", 1);

    oblist = parse_this(how[0], "[the] %i", ACTION_OTHER);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Trust whom/what [how]?\n");
        return 0;
    }

    actor("You trust", oblist, how[1] + ".");
    all2act(" trusts", oblist, how[1] + ".", how[1], ACTION_OTHER);
    target(" trusts you" + how[1] + ".", oblist, how[1], ACTION_OTHER);
    return 1;
}

int
twiddle(string str)
{
    if (stringp(str))
    {
        notify_fail("Twiddle what?\n");
        return 0;
    }

    write("You twiddle your thumbs.\n");
    allbb(" twiddles " + this_player()->query_possessive() + " thumbs.");
    return 1;
}

int
twinkle(string str)
{
    str = check_adverb_with_space(str, "merrily");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Twinkle how?\n");
        return 0;
    }

    write("You twinkle your eyes" + str + ".\n");
    allbb(" twinkles " + this_player()->query_possessive() + " eyes" +
          str + ".", str);
    return 1;
}

int
twitch(string str)
{
    str = check_adverb_with_space(str, "nervously");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Twitch how?\n");
        return 0;
    }

    write("Your left eye twitches" + str + ".\n");
    allbb("'s left eye twitches" + str + ".", str);
    return 1;
}

int
wail(string str)
{
    if (stringp(str))
    {
        notify_fail("Wail how?\n");
        return 0;
    }

    SOULDESC("wailing in agony");
    write("You wail in agony!\n");
    all(" wails in agony!", "", ACTION_AURAL);
    return 1;
}

int
wait(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "patiently", 0);

    if (!stringp(how[0]))
    {
        write("You wait" + how[1] + ".\n");
        allbb(" waits" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[for] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Wait [how] for whom/what?\n");
        return 0;
    }

    actor("You wait" + how[1] + " for", oblist);
    all2actbb(" waits" + how[1] + " for", oblist, 0, how[1]);
    targetbb(" waits" + how[1] + " for you.", oblist, how[1]);
    return 1;
}

int
wave(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "happily", 0);

    if (!stringp(how[0]))
    {
        write("You wave" + how[1] + ".\n");
        allbb(" waves" + how[1] + ".", how[1],
            ACTION_VISUAL | ACTION_LACTIVITY);
        return 1;
    }

    /* I am not sure whether you wave to or at someone, so I'll allow
     * both. If you know how it should be, let us know ;-)
     */
    oblist = parse_this(how[0], "[to] / [at] [the] %i",
        ACTION_VISUAL | ACTION_LACTIVITY);

    if (!sizeof(oblist))
    {
        notify_fail("Wave [how] to whom/what?\n");
        return 0;
    }

    actor("You wave" + how[1] + " in", oblist, "'s direction.");
    all2actbb(" waves" + how[1] + " in", oblist, "'s direction.", how[1],
        ACTION_VISUAL | ACTION_LACTIVITY);
    targetbb(" waves" + how[1] + " in your direction.", oblist, how[1],
        ACTION_VISUAL | ACTION_LACTIVITY);
    return 1;
}

int
weep(string str)
{
    str = check_adverb_with_space(str, "bitterly");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Weep how?\n");
        return 0;
    }

    SOULDESC("weeping" + str);
    write("You weep" + str + ".\n");
    all(" weeps" + str + ".", str, ACTION_AURAL | ACTION_VISUAL);
    return 1;
}

int
whimper(string str)
{
    str = check_adverb_with_space(str, "pathetically");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Whimper how?\n");
        return 0;
    }

    SOULDESC("whimpering" + str);
    write("You whimper" + str + ".\n");
    all(" whimpers" + str + ".", str, ACTION_AURAL);
    return 1;
}

int
whine(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "petulantly", 0);

    if (!stringp(how[0]))
    {
        SOULDESC("whining" + how[1]);
        write("You whine" + how[1] + ".\n");
        allbb(" whines" + how[1] + ".", how[1], ACTION_AURAL);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i", ACTION_AURAL);

    if (!sizeof(oblist))
    {
        notify_fail("Whine [how] at whom/what?\n");
        return 0;
    }

    SOULDESC("whining" + how[1]);
    actor("You whine" + how[1] + " to", oblist);
    all2actbb(" whines" + how[1] + " to", oblist, 0, how[1], ACTION_AURAL);
    targetbb(" whines" + how[1] + " to you.", oblist, how[1], ACTION_AURAL);
    return 1;
}

int
whistle(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "appreciatively", 0);

    if (!stringp(how[0]))
    {
        write("You whistle" + how[1] + ".\n");
        all(" whistles" + how[1] + ".", how[1], ACTION_AURAL);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i", ACTION_AURAL);

    if (!sizeof(oblist))
    {
        notify_fail("Whistle [how] at whom/what?\n");
        return 0;
    }

    actor("You whistle" + how[1] + " at", oblist);
    all2act(" whistles" + how[1] + " at", oblist, 0, how[1], ACTION_AURAL);
    target(" whistles" + how[1] + " at you.", oblist, how[1], ACTION_AURAL);
    return 1;
}

int
wiggle(string str)
{
    str = check_adverb_with_space(str, BLANK_ADVERB);

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Wiggle how?\n");
        return 0;
    }

    write("You wiggle your bottom" + str + ".\n");
    allbb(" wiggles " + this_player()->query_possessive() +
          " bottom" + str + ".", str, ACTION_MACTIVITY);
    return 1;
}

int
wince(string str)
{
    if (stringp(str))
    {
        notify_fail("Wince how?\n");
        return 0;
    }

    SOULDESC("wincing in pain");
    write("You wince in pain!\n");
    all(" winces in pain!");
    return 1;
}

int
wink(string str)
{
    object *oblist;
    string *how;

    how = parse_adverb_with_space(str, "suggestively", 0);

    if (!stringp(how[0]))
    {
        write("You wink" + how[1] + ".\n");
        allbb(" winks" + how[1] + ".", how[1]);
        return 1;
    }

    oblist = parse_this(how[0], "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Wink [how] at whom/what?\n");
        return 0;
    }

    actor("You wink" + how[1] + " at", oblist);
    all2actbb(" winks" + how[1] + " at", oblist, 0, how[1]);
    targetbb(" winks" + how[1] + " at you.", oblist, how[1]);
    return 1;
}

public int
wonder(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        write("You wonder what might happen.\n");
        all(" wonders what might happen.");
        return 1;
    }

    if ((strlen(str) > 60) &&
        (!(this_player()->query_wiz_level())))
    {
        write("You wonder beyond the end of the line and wake up from " +
            "your reveries.\n");
        all(" wonders what might happen.");
        return 1;
    }

    sscanf(str, "about %s", str);

    write("You wonder about " + str + "\n");
    all(" wonders about " + str);
    return 1;
}

int
worry(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        SOULDESC("worried about something");
        write("A worried look spreads across your face.\n");
        allbb(" has a worried look spreading across " +
            this_player()->query_possessive() + " face.");
        return 1;
    }

    oblist = parse_this(str, "[about] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Worry about whom/what?\n");
        return 0;
    }

    SOULDESC("worried about something");
    actor("You worry about", oblist);
    all2actbb(" looks worried about", oblist);
    targetbb(" looks worried about you.", oblist);
    return 1;
}

int
worship(string str)
{
    object *oblist;

    oblist = parse_this(str, "[the] %i", ACTION_PROXIMATE);

    if (!sizeof(oblist))
    {
        if (strlen(parse_msg))
        {
            write(parse_msg);
            return 1;
        }

        notify_fail("Worship whom/what?\n");
        return 0;
    }

    actor("You worship", oblist, ((sizeof(oblist) == 1) ? "." : " in order."));
    all2act(" falls on " + this_player()->query_possessive() +
        " knees in front of", oblist,
        ((sizeof(oblist) == 1) ? "." : " in order."), "", ACTION_PROXIMATE);
    target(" falls on " + this_player()->query_possessive() +
        " knees and shows how much " + this_player()->query_pronoun() +
        " worships you.", oblist, "", ACTION_PROXIMATE);
    return 1;
}

int
wring(string str)
{
    str = check_adverb_with_space(str, "in anguish");

    if (str == NO_ADVERB_WITH_SPACE)
    {
        notify_fail("Wring your hands how?\n");
        return 0;
    }

    SOULDESC("wringing " + this_player()->query_possessive() + " hands" + str);
    write("You wring your hands" + str + ".\n");
    all(" wrings " + this_player()->query_possessive() + " hands" + str + ".",
        str);
    return 1;
}

int
yawn(string str)
{
    object *oblist;

    if (!stringp(str))
    {
        write("You yawn. My, what big teeth you have!\n");
        all(" yawns.");
        SOULDESC("yawning");
        return 1;
    }

    oblist = parse_this(str, "[at] [the] %i");

    if (!sizeof(oblist))
    {
        notify_fail("Yawn at whom/what?\n");
        return 0;
    }

    str = ((sizeof(oblist) == 1) ? oblist[0]->query_objective() : "them");

    SOULDESC("yawning");
    actor("You yawn at", oblist, " to show " + str + " how much " +
        (sizeof(oblist) == 1 ? oblist[0]->query_pronoun() + " bores" :
        "they bore") + " you.");
    all2act(" yawns at", oblist, " to show " + str + " how boring " +
        (sizeof(oblist) == 1 ? oblist[0]->query_pronoun() + " is" :
        "they are") + ".");
    targetbb(" yawns at you. You must be boring to " +
        this_player()->query_objective() + ".", oblist);
    return 1;
}

int
yodel(string str)
{
    if (stringp(str))
    {
        notify_fail("Yodel what?\n");
        return 0;
    }

    write("Yodelii yodeluu!\n");
    all(" yodels a merry tune.", "", ACTION_AURAL);
    SOULDESC("yodeling");
    return 1;
}

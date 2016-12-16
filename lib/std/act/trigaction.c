/*
  /std/trigaction.c

  This package handles triggering of actions due to text encountered
  by the NPC.

  Observe that this includes text that is written to the NPC, ie not
  only tell_object() but also write(). This includes among other things
  room descriptions.

  NOTICE: Triggers are obsolete. Use hooks as much as possible! For emotes,
          use emote_hook() and emote_hook_onlooker().
*/

#pragma save_binary
#pragma strict_types

#define MAX_TRIG_VAR 10

static 	string	*trig_patterns,		/* Patterns that trig actions */
                *trig_functions;        /* Commands to execute */
static  object  *trig_oblist;           /* List of %l / %i objects */
static  int     num_arg;                /* Number of arguments */
static	mixed 	a1, a2, a3, a4, a5,
   		a6, a7, a8, a9, a10;	/* Arguments */
static	string	cur_text;		/* Text currently catched */

mixed trig_check(string str, string pat, string func);

/*
 * Function name: catch_tell
 * Description:   This is the text that normal players gets written to
 *                their sockets.
 */
void
catch_tell(string str)
{
    int il;
    string pattern, func, euid;

    if (query_interactive(this_object())) // Monster is possessed
    {
	write_socket(str);
	return;
    }

    if (!sizeof(trig_patterns))
	return;

    cur_text = str;

    for (il = 0; il < sizeof(trig_patterns); il++)
    {
	if (stringp(trig_patterns[il])) 
	{
	    pattern = process_string(trig_patterns[il], 1);
	    if (trig_check(str, pattern, trig_functions[il]))
		return;
	}
    }
}

/*
 * Description: Query for current arguments returned from parse_command
 */
mixed
trig_query_args()
{
    mixed arr;

    arr = ({ a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 });

    return slice_array(arr, 0, num_arg - 1);
}

/*
 * Description: Query for current catched text
 */
string trig_query_text() { return cur_text; }


mixed
trig_check(string str, string pat, string func)
{
    int pmatch;
    string *split, euid;
    mixed ob;

    if (!stringp(pat) || !stringp(func))
	return 0;

    split = explode("dummy" + pat + "dummy", "%");
    if ((sizeof(split) - 1) > MAX_TRIG_VAR)
    {
	return 0; /* Illegal pattern */
    }
    
    if (sizeof(trig_oblist))
	ob = trig_oblist;
    else
	ob = environment(this_object());

    if (!ob)
	return;

    switch (sizeof(split) - 1)
    {
    case 1:
	pmatch = parse_command(str, ob, pat, a1);
	break;
    case 2:
	pmatch = parse_command(str, ob, pat, a1,a2);
	break;
    case 3:
	pmatch = parse_command(str, ob, pat, a1,a2,a3);
	break;
    case 4:
	pmatch = parse_command(str, ob, pat, a1,a2,a3,a4);
	break;
    case 5:
	pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5);
	break;
    case 6:
	pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6);
	break;
    case 7:
	pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6,a7);
	break;
    case 8:
	pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6,a7,a8);
	break;
    case 9:
	pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6,a7,a8,a9);
	break;
    case 10:
	pmatch = parse_command(str, ob, pat, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
	break;
    }

    if (!pmatch)
	return 0;

    num_arg = sizeof(split) - 1;

    func = process_string(func, 1);

    if (!stringp(func))
	return func;

    switch (sizeof(split) - 1)
    {
    case 1:
	return call_other(this_object(), func, a1);
	break;
    case 2:
	return call_other(this_object(), func, a1,a2);
	break;
    case 3:
	return call_other(this_object(), func, a1,a2,a3);
	break;
    case 4:
	return call_other(this_object(), func, a1,a2,a3,a4);
	break;
    case 5:
	return call_other(this_object(), func, a1,a2,a3,a4,a5);
	break;
    case 6:
	return call_other(this_object(), func, a1,a2,a3,a4,a5,a6);
	break;
    case 7:
	return call_other(this_object(), func, a1,a2,a3,a4,a5,a6,a7);
	break;
    case 8:
	return call_other(this_object(), func, a1,a2,a3,a4,a5,a6,a7,a8);
	break;
    case 9:
	return call_other(this_object(), func, a1,a2,a3,a4,a5,a6,a7,a8,a9);
	break;
    case 10:
	return call_other(this_object(), func, a1,a2,a3,a4,a5,a6,a7,a8,a9,a10);
	break;
    }
    return 1;
}
/*
 * Description: Add a new pattern to trig on
 */
void
trig_new(string pat, string func)
{
    int pos;

    this_object()->set_tell_active(1); /* We want all messages sent to us */

    if ((pos = member_array(pat, trig_patterns)) >= 0)
	trig_functions[pos] = func;

    if (!sizeof(trig_patterns))
    {
	trig_patterns = ({});
	trig_functions = ({});
    }
    trig_patterns += ({ pat });
    trig_functions += ({ func });
}

/*
 *  Description: Delete a pattern from the ones to trig on
 */
void
trig_delete(string pat)
{
    int pos;

    if ((pos = member_array(pat, trig_patterns)) >= 0)
    {
	trig_patterns = exclude_array(trig_patterns, pos, pos);
	trig_functions = exclude_array(trig_functions, pos, pos);
    }
}

/*
 *  Description: Set the objects to trig on when pattern includes %i/%l
 */
void
trig_setobjects(object *obs)
{
    trig_oblist = obs;
}

/* This #if 0 makes that the routine isn't actually defined in the living, but
 * it is documented in sman. The reaosn for this is that it's faster to handle
 * at runtime if the routine doesn't exist than if it's empty.
 */
#if 0
/*
 * Function name: emote_hook
 * Description  : Whenever an emotion is performed on this living, or when it
 *                is performed in the room in general, this function is called
 *                to let the living know about the emotion. This way we can
 *                avoid the usage of all those costly triggers.
 * Arguments    : string emote - the name of the emotion performed. This
 *                    always is the command the player typed, query_verb().
 *                object actor - the actor of the emotion.
 *                string adverb - the adverb used with the emotion, if there
 *                    was one. When an adverb is possible with the emotion,
 *                    this argument is either "" or it will contain the used
 *                    emotion, preceded by a " " (space). This way you can
 *                    use the adverb in your reaction if you please without
 *                    having to parse it further.
 *                object *oblist - the targets of the emotion, if any.
 *                int cmd_attr - the command attributes, if any.
 *                int target - if true, this object was a target of the emote.
 */
public void
emote_hook(string emote, object actor, string adverb, object *oblist,
    int cmd_attr, int target)
{
}

/*
 * Function name: emote_hook_actor
 * Description  : Whenever a living performs an emote on others, this hook is
 *                called. This way we can avoid the usage of all those costly
 *                triggers.
 * Arguments    : string emote - the name of the emotion performed. This
 *                    always is the command the player typed, query_verb().
 *                object *oblist - the targets of the emotion.
 */
public void
emote_hook_actor(string emote, object *oblist)
{
}

/*
 * Function name: emote_hook_onlooker
 * Description  : Whenever this living sees an emotion being performed on
 *                someone else, this function is called to let the living know
 *                about the emotion. This way we can avoid the usage of all
 *                those costly triggers.
 * Arguments    : string emote - the name of the emotion performed. This
 *                    always is the command the player typed, query_verb().
 *                object actor - the actor of the emotion.
 *                string adverb - the adverb used with the emotion, if there
 *                    was one. When an adverb is possible with the emotion,
 *                    this argument is either "" or it will contain the used
 *                    emotion, preceded by a " " (space). This way you can
 *                    use the adverb in your reaction if you please without
 *                    having to parse it further.
 *                object *oblist - the targets of the emotion.
 *                int cmd_attr - the command attributes.
 */
public void
emote_hook_onlooker(string emote, object actor, string adverb, object *oblist,
    int cmd_attr)
{
}

/*
 * Function name: speech_hook
 * Description  : Whenever someone speaks, this function is called to let the
 *                livings in the room know about the speech. This way we can
 *                avoid the usage of all those costly triggers.
 * Arguments    : string verb - the name of the speech command used. This is
 *                    the command the player typed, query_verb(). For regular
 *                    speech "say" is used even for'.
 *                object actor - the actor of the speech.
 *                string adverb - the adverb used with the speech. If there is
 *                    one, it's prefixed with a space so you can use the
 *                    adverb in your reaction if you please without having
 *                    to parse it further.
 *                object *oblist - the targets of the emotion, if any.
 *                string text - the text that was uttered. Whispering is only
 *                    understood if you are the recipient.
 *                int target - can have three states:
 *                   -1 - speech was directed at someone else
 *                    0 - speech was directed at nobody in particular
 *                    1 - speech was directed at me.
 */
public void
speech_hook(string verb, object actor, string adverb, object *oblist,
    string text, int target)
{
}
#endif

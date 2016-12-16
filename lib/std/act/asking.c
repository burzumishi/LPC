/*
 * /std/act/asking.c
 *
 * A simple support for helping answering questions.
 */

#pragma save_binary
#pragma strict_types

#include <macros.h>
#include <stdproperties.h>

static	mixed	ask_arr;		/* The triggers and answers */
static	mixed	default_answer;		/* A default answer if you want. */
static	int	default_answer_cmd;	/* The default answer is a command. */
static	string	posed_question;		/* The exact question the player posed. */
static	string	not_here_func;	 	/* A function to call if player not here */
static	int	dont_answer_unseen;	/* Flag if not to answer unseen */

/*
 * Function name: set_dont_answer_unseen
 * Description:   This mobile will look confused if he can't see who asked
 *		  him and if this flag is set.
 * Arguments:     int flag - How the flag should be set (1/0)
 */
public void
set_dont_answer_unseen(int flag)
{
    dont_answer_unseen = flag;
}

/*
 * Function name: query_dont_answer_unseen
 * Description:   Ask about the state of the dont_answer_unseen flag.
 * Returns:       int - the flag
 */
public int
query_dont_answer_unseen()
{
    return dont_answer_unseen;
}

/*
 * Function name: ask_id
 * Description:   Identify questions in the object.
 * Arguments:     string str - question to test.
 * Returns:       int - if true the question exists.
 */
public int
ask_id(string str)
{
    int i;

    if (!ask_arr)
	return 0;

    for (i = 0; i < sizeof(ask_arr); i++)
    {
        if (member_array(str, ask_arr[i][0]) >= 0)
	    return 1;
    }
}

/*
 * Function name: add_ask
 * Description:   Adds an question this mobile can answer. The first argument
 *                is a single string or an array of strings holding the
 *                the possible question(s) of the item. The second argument is
 *                the long description of the answer. add_ask can be repeatedly
 *                called with new questions.
 * Arguments:     mixed questions - string or array of strings; the question(s)
 *                                  that lead to this answer.
 *                mixed answer    - the answer of the question, may be VBFC.
 *		  int command     - if true this answer is a command
 * Returns:       True or false.
 */
public varargs int
add_ask(mixed questions, mixed answer, int command)
{
    if (!pointerp(questions))
    {
	questions = ({ questions });
    }
    if (ask_arr)
    {
	ask_arr = ask_arr + ({ ({ questions, answer, command }) });
    }
    else
    {
	ask_arr = ({ ({ questions, answer, command }) });
    }
}

/*
 * Function name: query_ask
 * Description:   Get the additional questions array.
 * Returns:       Question array, possibly containing VBFC, see below:

  [0] = array
     [0] ({ "name1 of question1", "name2 of question1",... })
     [1] "This is the answer of the question1."
     [2] command (1 or 0)
  [1] = array
     [0] ({ "name1 of question2", "name2 of question2", ... })
     [1] "This is the answer of the question2."
     [2] command (1 or 0)
*/
public mixed
query_ask()
{
    return ask_arr;
}

/*
 * Function name: remove_asks
 * Description  : Clears the complete list of asks.
 */
public void
remove_asks()
{
    ask_arr = 0;
}

/*
 * Function name: remove_ask
 * Description:   Removes one additional answer from the additional item list
 * Arguments:     string question - question to answer to remove.
 * Returns:       True or false. (True if removed successfully)
 */
public int
remove_ask(string question)
{
    int i;

    if (!pointerp(ask_arr))
        return 0;

    for (i = 0; i < sizeof(ask_arr); i++)
    {
        if (member_array(question, ask_arr[i][0]) >= 0 )
        {
            ask_arr = exclude_array(ask_arr, i, i);
            return 1;
        }
    }
    return 0;
}

/*
 * Function name: set_default_answer
 * Description:   Set the default answer you want your creature to say if he
 * 		  cannot identify the question. The default answer will be
 *                echoed to the player, unless the cmd flag is set.
 *                When using VBFC for the default answer, this function should
 *                return the type string. If the NPC performed a command as
 *                default answer, the string "" should be returned.
 *		  NOTE that no reaction will come if a player asks a question
 * 		  to this mobile that we have no answer set for if this default
 *		  is not set.
 * Argument:	  mixed answer - The default answer, VBFC is allowed.
 *		  int cmd  - if true, the answer is executed as command.
 */
public varargs void
set_default_answer(mixed answer, int cmd = 0)
{
    default_answer = answer;
    default_answer_cmd = cmd;
}

/*
 * Function name: query_default_answer
 * Description:   Query the setting of the default answer as it was set. The
 *                possible VBFC is not resolved.
 * Returns:	  mixed - the default answer.
 */
public mixed
query_default_answer()
{
    return default_answer;
}

/* 
 * Function name: query_default_answer_cmd
 * Description:   Find out whether the default answer is a command or a text
 *                to be echoed to the user.
 * Returns:       int - if true, the default answer is a command.
 */
public int
query_default_answer_cmd()
{
    return default_answer_cmd;
}

/*
 * Function name: set_not_here_func
 * Description:   Set a function to call if the player who posed the question
 *		  has left the room before he got the answere. 
 * Arguments:	  func - The function name
 */
public void
set_not_here_func(string func)
{
    not_here_func = func;
}

/*
 * Function name: query_not_here_func
 * Description:   Query what the not here function is set to
 * Returns:	  The function name
 */
public string
query_not_here_func()
{
    return not_here_func;
}

/*
 * Function name: query_question
 * Description:   This function will return the true question the mortals posed
 * 		  to us. It can be called from a VBFC for example to do some 
 *		  testing or referring to the actual question if you want.
 * Returns: 	  question
 */
public string
query_question()
{
    return posed_question;
}

/*
 * Function name: unseen_hook
 * Description:   This function gets called if this mobile couldn't see
 *		  who asked the question and is not supposed to answer
 *		  in that case.
 */
public void
unseen_hook()
{
    command("confused");
    command("say Who asked me that?");
}

/*
 * Function name: answer_question
 * Description  : This function is called after a short delay when this mobile
 * 		  wants to react to a question
 * Arguments    : mixed msg - The message, may contain VBFC. It should NOT be
 *                    an array at this point.
 *                int cmd - if true, it's a command and not text to display.
 */
void
answer_question(mixed msg, int cmd)
{
    object env;

    if ((env = environment(this_object())) == environment(this_player()) ||
	    env == this_player() || (not_here_func &&
		call_other(this_object(), not_here_func, this_player())))
    {
	msg = this_object()->check_call(msg, this_player());

        if (!msg)
            return;
        
	if (cmd)
	    command(msg);
	else
	    tell_object(this_player(), msg);
    }
}

/*
 * Function name: catch_question
 * Description:	  This function is called in each living being someone asks a
 *		  question to.
 * Arguments:	  question - The question as put
 */
public void
catch_question(string question)
{
    int i;

    if (dont_answer_unseen && (!this_player()->check_seen(this_object()) ||
		!CAN_SEE_IN_ROOM(this_object())))
    {
	set_alarm(rnd() * 3.0 + 1.0, 0.0, unseen_hook);
	return;
    }

    /* Strip trailing period of question mark. */
    if (wildmatch("*[.\\?]", question))
    {
        question = extract(question, 0, -2);
    }

    posed_question = lower_case(question);

    for (i = 0; i < sizeof(ask_arr); i++)
    {
	if (member_array(posed_question, ask_arr[i][0]) >= 0)
	{
	    set_alarm(rnd() * 4.0, 0.0, &answer_question(ask_arr[i][1], ask_arr[i][2]));
	    return ;
	}
    }

    if (default_answer)
    {
	set_alarm(rnd() * 4.0, 0.0,
	    &answer_question(default_answer, default_answer_cmd));
    }
}

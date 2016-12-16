/*
  /std/seqaction.c

  This is the first attempt at intelligent npc's. The basic idea was
  from Zellski's command/function sequences.


  Typical usage:

          seq_new("talk"); seq_new("walk");
          
          seq_addfirst("talk", command_sequence1);
          seq_addfirst("walk", command_sequence2);

  Typical command sequence:

  ({
      "smile", 3, "say How are you doing?", "north", "@@find_dwarf"
  })

  Integers are delayvalues in heart_beats time the slow factor.

  The slow factor is set so that the game does not get overwhelmed with
  monster sequences.

  Observe the "@@" constructs. This is the way to make truly intelligent
  behaviour. Use the VBFC just as usual. Note also that effuserid will be 0
  in the call to these VBFC functions (as it normally is).

*/
#pragma save_binary
#pragma strict_types

#include <macros.h>
#include <seqaction.h>

static  mixed   *seq_commands;          /* Array of arrays holding actions
                                           to do each heart_beat */
static  string  *seq_names;             /* id of a sequence */
static  int     *seq_flags;             /* flags of a sequence */
static  int     seq_active,
                *seq_cpos;              /* Current position in array */

public void seq_restart();

/*
 *  Description: Called from living to initialize
 */
public void
seq_reset()
{
    seq_commands = ({});
    seq_names = ({});
    seq_flags = ({});
    seq_cpos = ({});
    seq_active = 0;
}

/*
 *   Description: The core function that actually runs the commands
 */
public void
seq_heartbeat(int steps)
{
    int il, newstep, stopseq, stopped;
    mixed cmd;
    mixed cmdres;
    mixed *calls;

    set_alarm(rnd() * SEQ_SLOW + SEQ_SLOW / 2.0, 0.0, &seq_heartbeat(1));

    stopseq = ((time() -
                this_object()->query_last_met_interactive()) > SEQ_STAY_AWAKE);

    newstep = 0;
    stopped = 0;
    if (!steps)
        steps = 1;

    if (stopseq)
        stopped = 1;
    
    for (il = 0; il < sizeof(seq_names); il++)
    {
        if (seq_cpos[il] < sizeof(seq_commands[il]))
        {
            if (!stopseq || (seq_flags[il] & SEQ_F_NONSTOP) || !stopped)
                stopped = 0;

            cmd = seq_commands[il][seq_cpos[il]];
            seq_cpos[il]++;

            if (stringp(cmd) || functionp(cmd))
                cmdres = this_object()->check_call(cmd);
            else if (intp(cmd))
                cmdres = cmd - steps;
            else
            {
                newstep = 1;
                continue;
            }

            if (stringp(cmdres))
            {
                command(cmdres);
                newstep = 1;
            }
            else if (intp(cmdres) && cmdres > 0)
            {
                newstep = ((newstep) && (cmdres > newstep) ? newstep : cmdres);
                seq_cpos[il]--;
                seq_commands[il][seq_cpos[il]] = cmdres;
            }
            else 
            {
                newstep = 1;
            }
        }
        else
        {
            seq_cpos[il] = 0;
            seq_commands[il] = ({});
        }
    }

    calls = get_all_alarms();
    if (stopped)
    {
        for (il = 0 ; il < sizeof(calls) ; il++)
            if (calls[il][1] == "seq_heartbeat")
                remove_alarm(calls[il][0]);
        this_object()->add_notify_meet_interactive("seq_restart");
        if (!newstep)
        {
            seq_active = 0;
        }
    }
    if (newstep > 1)
    {
        for (il = 0 ; il < sizeof(calls) ; il++)
            if (calls[il][1] == "seq_heartbeat")
                remove_alarm(calls[il][0]);
        set_alarm(itof(newstep) * (SEQ_SLOW / 2.0 + rnd() * SEQ_SLOW), 0.0,
                  &seq_heartbeat(newstep));
    }
}

/*
 * Called when the living encounters an interactive player
 * and sequences has been stopped
 */
public void
seq_restart()
{
    mixed *calls = get_all_alarms();
    int il;

    seq_active = 1;
    for (il=0 ; il<sizeof(calls) ; il++)
        if (calls[il][1] == "seq_heartbeat")
            remove_alarm(calls[il][0]);
    set_alarm(1.0, 0.0, &seq_heartbeat(1));
    this_object()->remove_notify_meet_interactive("seq_restart");
}

/*
 *  Description: New command sequence. Command sequences are independant
 *               named streams of commands. This function creates a stream.
 */
public varargs int 
seq_new(string name, int flags)
{
    if (!IS_CLONE)
        return 0;

    if (member_array(name, seq_names) >= 0)
        return 0;

    if (sizeof(seq_names) >= SEQ_MAX)
        return 0;

    seq_names += ({ name });
    seq_commands += ({ ({}) });
    seq_cpos += ({ 0 });
    seq_flags += ({ flags });
    return 1;
}

/*
 *   Description: Delete an entire sequence.
 */
void
seq_delete(string name)
{
    int pos;

    if ((pos = member_array(name, seq_names)) < 0)
        return 0;

    seq_names = exclude_array(seq_names, pos, pos);
    seq_commands = exclude_array(seq_commands, pos, pos);
    seq_cpos = exclude_array(seq_cpos, pos, pos);
    seq_flags = exclude_array(seq_flags, pos, pos);
}

/*
 *  Description: Add a command or an array of commands first in a sequence.
 *               Note that a command can be VBFC. This enables calls to
 *               functions that returns the actual command. This function
 *               can of course do all sorts of things. If 'cmd' is a number
 *               it is interpreted as a delayvalue (in heartbeats) until
 *               the next command is issued. 
 *
 *               The sequence 'name' must be created with seq_new(name)
 *               prior to the call of this function.
 */
int
seq_addfirst(string name, mixed cmd)
{
    int pos;

    if ((pos = member_array(name, seq_names)) < 0)
    {
        return 0;
    }

    if (!seq_active)
    {
        seq_restart();
    }

    if (!pointerp(cmd))
    {
        cmd = ({ cmd });
    }

    seq_commands[pos] = cmd + 
        seq_commands[pos][seq_cpos[pos]..sizeof(seq_commands[pos])];
    seq_cpos[pos] = 0;

    return 1;
}

/*
 *  Description: Add a command or an array of commands last in a sequence.
 *               Same as seq_addfirst except the command(s) are added last
 *               in the sequence. (see seq_addfirst)
 */
int
seq_addlast(string name, mixed cmd)
{
    int pos;

    if ((pos = member_array(name, seq_names)) < 0)
    {
        return 0;
    }

    if (!seq_active)
    {
        seq_restart();
    }

    if (!pointerp(cmd))
    {
        cmd = ({ cmd });
    }

    seq_commands[pos] = seq_commands[pos] + cmd;

    return 1;
}

/*
 *  Description: Returns the sequence of commands for a given sequence.
 *
 *               For a specific sequence 'name' it returns the remaining
 *               commands or an empty array. If no sequence 'name' exists
 *               then 0 is returned.
 */
string *
seq_query(string name)
{
    int pos;

    if ((pos = member_array(name, seq_names)) < 0)
        return 0;
    
    return slice_array(seq_commands[pos],
                       seq_cpos[pos], sizeof(seq_commands[pos]));
}

/*
 *  Description: Returns the flags for a given sequence.
 *
 */
int
seq_query_flags(string name)
{
    int pos;

    if ((pos = member_array(name, seq_names)) < 0)
        return 0;
    
    return seq_flags[pos];
}

/*
 *  Description: Returns the list of sequences (their names)
 */
string *
seq_query_names() { return seq_names + ({}); }

/*
 *  Description: Clears a given sequence from commands.
 */
void
seq_clear(string name)
{
    int pos;

    if ((pos = member_array(name, seq_names)) < 0)
        return;

    seq_commands[pos] = ({});
    seq_cpos[pos] = 0;
}

/*
   /std/act/chat.c

   Chatting: Standard action module for mobiles

   add_chat(string)	       Set a random chatstring 

   add_cchat(string)           Set a random combat chatstring 

   clear_chat()		       Clear random chatstring

   clear_cchat()	       Clear random combat chatstring

   set_chat_time(int)	       Set the mean value for chat intervall

   set_cchat_time(int)	       Set the mean value for combat chat intervall
*/

#pragma save_binary
#pragma strict_types

#include <macros.h>

static	int	monster_chat_time,     /* Intervall between chat */
		monster_cchat_time;    /* Intervall between combat chat */
static  string  *monster_chat,	       /* Chat strings */
    		*monster_cchat,        /* Combat Chat strings */
    		*monster_chat_left,    /* Chat string left */
    		*monster_cchat_left;   /* Combat Chat strings left */

#define SEQ_CHAT  "_mon_chat"

void monster_do_chat();

/*
 * Function name: add_chat
 * Description:   Adds a chat string that the monster will randomly say.
 * Arguments:	  str: Text
 */
void
add_chat(mixed str)
{
    if (!IS_CLONE)
	return;

    if (!this_object()->seq_query(SEQ_CHAT))
    {
	this_object()->seq_new(SEQ_CHAT);
    }
    this_object()->seq_clear(SEQ_CHAT);
    this_object()->seq_addfirst(SEQ_CHAT, monster_do_chat);

    if (!str)
	return;

    if (!sizeof(monster_chat))
	monster_chat = ({});

    monster_chat += ({ str });
    monster_chat_left = monster_chat;
}

string *query_chat() { return monster_chat; }

/*
 * Function name: add_cchat
 * Description:   Sets a combat chat string that the monster will randomly say
 * Arguments:	  str: Text
 */
void
add_cchat(string str)
{
    if (!IS_CLONE)
	return;

    add_chat(0); /* Init chat sequence */

    if (!sizeof(monster_cchat))
	monster_cchat = ({});

    monster_cchat += ({ str });
    monster_cchat_left = monster_cchat;
}

string *query_cchat() { return monster_cchat; }

/*
 * Function name: clear_chat
 * Description:   Clear random chatstring
 */
void
clear_chat()
{
    monster_chat = monster_chat_left = 0;
}

/*
 * Function name: clear_cchat
 * Description:   Clear random combat chatstring
 */
void
clear_cchat()
{
    monster_cchat = monster_cchat_left = 0;
}

/*
 * Function name: set_chat_time
 * Description:   Set the mean value for chat intervall
 * Arguments:	  tim: Intervall
 */
void
set_chat_time(int tim)
{
    monster_chat_time = tim;
}

/*
 * Function name: set_cchat_time
 * Description:   Set the mean value for cchat intervall
 * Arguments:	  tim: Intervall
 */
void
set_cchat_time(int tim)
{
    monster_cchat_time = tim;
}

/*
 * Sequence functions
 */

/*
 *  Description: The actual function chatting, called by VBFC in seq_heartbeat
 */
void
monster_do_chat()
{
    int il;
    string chatstr;

    if (!this_object()->query_attack()) 
    {
	if (!sizeof(monster_chat_left))
	    monster_chat_left = monster_chat;

	if (!(il=sizeof(monster_chat_left)))
	    return;

	il = random(il);
	chatstr = monster_chat_left[il];
	monster_chat_left = exclude_array(monster_chat_left,il,il);
	il = monster_chat_time;
    }
    else  /* In combat */
    {
	if (!sizeof(monster_cchat_left))
	    monster_cchat_left = monster_cchat;

	if (!(il = sizeof(monster_cchat_left)))
	    return;

	il = random(il);
	chatstr = monster_cchat_left[il];
	monster_cchat_left = exclude_array(monster_cchat_left,il,il);
	il = monster_cchat_time;
	
    }
    this_object()->seq_clear(SEQ_CHAT);
    this_object()->seq_addfirst(SEQ_CHAT,
	({ "say " + chatstr, il, monster_do_chat }) );
}

public string
query_seq_chat_name()
{
    return SEQ_CHAT;
}

#if 0
/*
 * Function name: catch_whisper
 * Description  : Called whenever someone whisper to this living. It does
 *                not indicate when this living is an onlooker. Use
 *                speech_hook for that.
 * Arguments    : string str - the text being whispered.
 */
public void
catch_whisper(string str)
{
}
#endif 0

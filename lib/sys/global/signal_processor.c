/* 
   
Functions
---------
int start_listen(string channel, string func)
    Start listening to "channel", messages will be sent to "func" in
    calling object. Returns 1 if successful, 0 if failed

int send_signal(string channel, mixed message)
    send "message" to "channel". Returns 1 if successful, 0 if failed

void stop_listen(string channel)
    Caller stops listening to "channel". Always successful.

void stop_listen_all()
    Caller stops listening to all channels. Always successful.

int secure_channel(string channel, object ob, string func)
     When an object tries to listen to a secured channel, func will be
     called in ob with the object as the first argument, and the channel
     as the second argument.  If func returns 1, then that object is
     allowed to listen to that channel.
     When an object tries to send a message on a secured channel, then
     func will be called in ob with the object as the first argument,
     the channel as the second argument, and 1 as the third argument.  If
     func returns 1, then the object will be allowed to send the message.
     secure_channel returns 0 if unsuccessful, and returns 1 + the number
     of objects kicked off the channel if successful.
     An object may call "secure_channel" again if it passes send security.

object *query_listeners(string channel)
     If you have permission to write to a channel, then you are allowed to
     get a list of the objects which are listenning to the channel.

string *query_channels(object ob)
     Returns the channels that that object is listenning to.
     
Data Structures
---------------
channels[channel][ob] = func;
  string channel            the name of the channel
  object ob                 the object listenning to that channel
  string func               the function in ob which is called when a message
                            is sent to that channel (with the message as arg)

NOTE:  An object may not listen to the same channel using 2 different
functions, but it can listen to different channels. 

listeners[ob] = channels_list
  object ob                 the object which is listenning
  string *channels_list     a list of the channels this ob listens to

secured[channel] = ({object, func})
  see secure_channel above.
*/
#define USE_CALL_OUT     
#pragma save_binary
#pragma strict_types
#pragma no_clone
#pragma no_inherit

mapping channels = ([]);
mapping listeners = ([]);
mapping secured = ([]);

static void restore_channels();

void
create()
{
    restore_channels();
}

void
remove_object()
{
    destruct();
}

static void
restore_channels()
{

}

static void
_stop_listen(string channel, object ob)
{
    if(!channel || !stringp(channel))
	return;
    
    if(channels[channel])
	m_delkey(channels[channel], ob);
    if (listeners[ob])
	listeners[ob] -= ({channel});
}

void
stop_listen(string channel)
{
    _stop_listen(channel, previous_object());
}

int
start_listen(string channel, string func)
{
    object ob = previous_object();
    
    if(!channel || !stringp(channel) ||
       !func || !stringp(func))
	return 0;
    
    if(listeners[ob] && member_array(channel, listeners[ob]) != -1)
	return 1; // we are already listening to this channel

    if(!channels[channel])
	channels[channel] = ([]);

    if (secured[channel])
	if (!secured[channel][0])
	    m_delkey(secured, channel);
	else if (!call_other(secured[channel][0], secured[channel][1],
			     ob, channel))
      return 0;

    channels[channel] += ([ob:func]);

    if (listeners[ob])
	listeners[ob] += ({channel});
    else
	listeners[ob] = ({channel});
    return 1;
}

static void
_stop_listen_all(object ob)
{
    int i, n;
    string *chans;
  
    if(listeners[ob])
    {
	n = sizeof(listeners[ob]);
	for(i = 0; i < n; ++i)
	    if(channels[listeners[ob][i]])
		m_delkey(channels[listeners[ob][i]], ob);
	m_delkey(listeners, ob);
    }
    n = sizeof(chans = m_indexes(secured));
    for (i = 0; i < n; ++i)
	if (secured[chans[i]][0] == ob)
	    m_delkey(secured, chans[i]);
}

void
stop_listen_all()
{
    _stop_listen_all(previous_object());
}

int
send_signal(string channel, mixed message)
{
    int i, n;
    object *obs;
  
    if(!channel || !stringp(channel) || !message)
	return 0;
    
    if(!channels[channel])
    {
	channels += ([channel:([])]);
	return 1;
    }

    if (secured[channel])
	if (!secured[channel][0])
	    m_delkey(secured, channel);
	else if (secured[channel][1] &&
		 !call_other(secured[channel][0], secured[channel][1],
			     previous_object(), channel, 1))
	    return 0;
  
    n = m_sizeof(channels[channel]);
    obs = m_indexes(channels[channel]);
    for(i = 0; i < n; ++i) // send the message to all the listeners 
	if (obs[i])
	{
#ifdef USE_CALL_OUT
	    set_alarm(0.3, 0.0, &call_other(obs[i], channels[channel][obs[i]], message));
#else
	    call_other(obs[i], channels[channel][obs[i]], message);
#endif
	}
	else // the object has been destructed (this should never happen)
	    _stop_listen_all(obs[i]);
    return 1;
}

object *
query_listeners(string channel)
{
    if(!channel || !stringp(channel))
	return 0;
    
    if (!channels[channel])
    {
	channels[channel] = ([]);
	return ({});
    }

    if (secured[channel])
	if (!secured[channel][0])
	    m_delkey(secured,channel);
	else if (!call_other(secured[channel][0], secured[channel][1],
			     previous_object(), channel, 1))
	    return 0;
  
    return m_indexes(channels[channel]) - ({ 0 });
}

int
secure_channel(string channel, object ob, string func)
{
    int i, kicked, n;
    object *obs;

    if (!channel || !stringp(channel) ||
	!ob || !objectp(ob))
	return 0;

    if(!func || !stringp(func))
    {
	func = 0;
    }
    else
    {
	if (!function_exists(func, ob))
	    func = 0;
    }	    
	
    if(!channels[channel])
	channels += ([channel:([])]);
    if (secured[channel])
	if (!secured[channel][0] ||
	    call_other(secured[channel][0], secured[channel][1],
		       previous_object(), channel, 1))
	    m_delkey(secured, channel);
	else
	    return 0;
    secured += ([channel:({ob, func})]);

    n = m_sizeof(channels[channel]);
    obs = m_indexes(channels[channel]);
    for(i = 0; i < n; ++i)
	if (!obs[i]) // the object has been destructed (should never happen)
	    _stop_listen_all(obs[i]);
	else if (!call_other(ob, func, obs[i], channel))
	{
	    _stop_listen(channel, obs[i]); // kick the unwanted object off
	    kicked += 1;
	}
      
    return kicked + 1;
}

string *
query_channels(object ob)
{
    return listeners[ob];
}

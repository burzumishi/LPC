#pragma save_binary
#pragma no_clone
#pragma no_inherit
#pragma no_shadow

inherit "/d/Standard/lib/multi_alarm";

#include "time.h"

nomask void do_scheduled_event(object ob, function func);

varargs nomask int
schedule(object ob, 
         function func, 
         int year   = -1,
         int month  = -1,
         int day    = -1,
         int hour   = -1,
         int minute = -1,
         int second = -1)
{
    int delay = EARTHSEA_CLOCK->seconds_until_time(year, month, day, hour, minute, second);
    int id;

    find_player("shiva")->catch_tell(delay + "\n");

    if (delay < 0)
    {
        return 0;
    }

#ifdef USE_MULTI_ALARM
    id = set_multi_alarm(itof(delay), &do_scheduled_event(ob, func));
#else
    id = set_alarm(itof(delay), 0.0, &do_scheduled_event(ob, func));
#endif

    find_player("shiva")->catch_tell(id + "\n");

    return id;
}

nomask void
do_scheduled_event(object ob, function func)
{
    find_player("shiva")->catch_tell("here\n");

    if (ob)
    {
         find_player("shiva")->catch_tell("2\n");
         func();
    }
}

nomask void
unschedule(int id)
{
#ifdef USE_MULTI_ALARM
    remove_multi_alarm(id);
#else
    remove_alarm(id);
#endif
}

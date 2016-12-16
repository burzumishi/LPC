    add_prop(ROOM_I_NO_CLEANUP, 1);
}

public void
clean_up()
{
    object *ob;
    int i;

    ob = all_inventory(this_object());
    for (i = 0 ; i < sizeof(ob) ; i++)
    {
	if (living(ob[i]))
	{
	    remove_alarm(Alarm);
	    Alarm = set_alarm(itof(Timeout), 0.0, clean_up);
	    return;
	}
    } 

    for (i = 0 ; i < sizeof(ob) ; i++)
	Master_ob->dispose_of(ob[i]);

    rm(MASTER_OB(TO) + ".c");
    remove_object();
}

public nomask void
enter_inv(object ob, object from)
{
    remove_alarm(Alarm);
    Alarm = set_alarm(itof(Timeout), 0.0, clean_up);
    ::enter_inv(ob, from);
}

public nomask void
set_cleanup_time(int time)
{
    Timeout = time;
    remove_alarm(Alarm);
    Alarm = set_alarm(itof(Timeout), 0.0, clean_up);
}

/*
 * /secure/wizday2003.c
 *
 * Wizday 2003 relay to copy playerfiles to safety just before the person dies.
 *
 * Created by mercade, August 28, 2003
 */

#include <files.h>
#include <std.h>

#include "/d/Genesis/ateam/aoe/events/wizday2003/wizday.h"

public void
store_wizday(object player)
{
    string name;
    string fromfile;
    string tofile;
    string text;
    int    index;

    if (!IS_PLAYER_OBJECT(player))
    {
        return;
    }
    player->save_me(0);

    setuid();
    seteuid(getuid());

    name = player->query_real_name();
    fromfile = PLAYER_FILE(name) + ".o";
    index = 1;
    tofile = fromfile + ".wizday." + index;
    while(file_size(tofile) > 0)
    {
        index++;
        tofile = fromfile + ".wizday." + index;
    }
    text = read_file(fromfile);
    write_file(tofile, text);
}

public void
pseudo_death(object killer, object victim)
{
    setuid();
    seteuid(getuid());

    write_file(LOG_XX_DEATHS, ctime(time()) + " " +
        victim->query_real_name() + " quasi-killed by " +
        killer->query_real_name() + ".\n");
}

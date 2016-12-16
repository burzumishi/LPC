/* 
 *  /std/act/domove.c
 *
 *  Random walk: Standard action module for mobiles.
 */

#pragma save_binary
#pragma strict_types

static  int      monster_ranmove;       /* Intervall between walks  */
static  string   monster_follow,        /* Name of person to follow */
                *monster_restrain_path, /* Delimiting path names    */
                 monster_home;          /* The home of the monster  */

#define SEQ_RWALK "_mon_ranwalk"
#define SEQ_FOLL  "_mon_ranwalk"

void monster_ranwalk();
mixed oke_to_move(string exit);

/*
 * Function name: set_random_move
 * Description:   Set the ability to walk around, and the
 *                time affecting limit.
 * Arguments:     time - The monster will move in 10 + random(time) seqaction
 *                       intervals.  If time is 0, random movement will be
 *                       stopped.
 *                flag - True if you want the monster to wander forever. If
 *                       0 or not present, the monster will wander randomly
 *                       for as long as it meets players and a short time
 *                       after that. The preferred behavior is to not wander
 *                       forever.
 */
varargs void
set_random_move(int time, int flag)
{
    monster_ranmove = time;

    if (time == 0)
    {
        this_object()->seq_delete(SEQ_RWALK);
        return;
    }

    if (!this_object()->seq_query(SEQ_RWALK))
    {
        this_object()->seq_new(SEQ_RWALK, flag);
    }

    this_object()->seq_clear(SEQ_RWALK);
    this_object()->seq_addfirst(SEQ_RWALK, monster_ranwalk);
}

/*
 * Function name: set_follow
 * Description:   Sets the name of someone to follow
 * Arguments:     name: Name of living (something that works with present() )
 *                int: if non-zero, don't wander at all, i.e. just follow
 */
varargs void
set_follow(string name, int flag)
{
    monster_follow = name;
    this_object()->trig_new("%s '" + capitalize(name) + "' %s","mtrig_follow");

    if (!flag)
    {
        set_random_move(1); /* Search for victim */
    }
}

/*
 * Function name: query_follow
 * Description    : Returns who this living is following.
 * Returns        : string - the name of the person, or 0.
 */
string
query_follow()
{
    return monster_follow;
}

/*
 * Function name: unset_follow
 * Description:   Stop following
 */
void
unset_follow()
{
    if (monster_follow)
    {
        this_object()->trig_delete("%s '"+capitalize(query_follow())+"' %s");
    }

    monster_follow = 0;
}

/*
 * Function name:   set_restrain_path
 * Description:     Set the path that delimits the exits the monster will
 *                  choose. Exits of rooms that begin with the pathname
 *                  are elegible as random walking exits. The others are
 *                  not considered. If a monster gets stuck this way, it
 *                  will teleport home, which is a room that can be set
 *                  with set_monster_home().
 * Arguments:       path: Either a path or an array of paths that form the
 *                        begin of legal room names.
 * See also:        set_monster_home(), query_restrain_path()
 */
void
set_restrain_path(mixed path)
{
    if (!path)
    {
        monster_restrain_path = ({ });
    }
    else if (stringp(path))
    {
        monster_restrain_path = ({ path });
    }
    else if (pointerp(path))
    {
        monster_restrain_path = path;
    }
}

/*
 * Function name:   query_restrain_path
 * Description:     Give back the restrain_path (if any)
 * Returns:         An array with pathstrings.
 * See also:        set_monster_home(), set_restrain_path()
 */
string *
query_restrain_path()
{
    return (monster_restrain_path || ({}));
}

/*
 * Function name:   set_monster_home
 * Description:     Set the room filename to which the monster will teleport
 *                  if it managed to get itself stuck, that is, if it ended
 *                  up in a room with no elegible exits.
 * Arguments:       The filename of the room.
 * See also:        set_restrain_path(), query_restrain_path()
 */
void
set_monster_home(string home)
{
    monster_home = home;
}

/*
 * Function name:   query_monster_home
 * Description:     Return the filename of the home to which a monster will
 *                  teleport if it got itself stuck.
 * Returns:         That filename.
 * See also:        set_monster_home(), set_restrain_path()
 */
string
query_monster_home()
{
    return monster_home;
}

/*
 * Function name:   filter_exits
 * Description:     Filter out all non-wanted exits of a given array.
 *                  Non-wanted exits are exits that do not fall within the
 *                  given path that was set with set_restrain_path().
 * Arguments:       exits: An array of exits, as delivered by query_exits().
 * Returns:         0 if no exits, or an array with legal exits.
 */
mixed *
filter_exits(mixed *exits)
{
    string tmp, *path_arr;
    mixed *res_exits;
    int i, j;

    if (!exits || !sizeof(exits))
    {
        return ({ });
    }

    if (!monster_restrain_path || !sizeof(monster_restrain_path))
    {
        return exits;
    }

    seteuid(getuid());
    res_exits = ({ });
    for (i = 0; i < sizeof(exits); i += 3)
    {
        for (j = 0; j < sizeof(monster_restrain_path); j++)
        {
            if (sscanf(exits[i], monster_restrain_path[j] + "%s", tmp))
            {
                res_exits += exits[i..i+2];
                break;
            }
        }
    }

    return res_exits;
}

/*
 * Function name:   mtrig_follow
 * Description:     The text trig function
 * Arguments:       s1, s2: The strings returned by the trig-function.
 * Returns:         1
 */
int
mtrig_follow(string s1, string s2)
{
    string *words;
    mixed *ex, *calls;
    int il, i;

    if (!environment(this_object()))
    {
        return 1;
    }

    if (!this_object()->seq_query(SEQ_FOLL))
    {
        this_object()->seq_new(SEQ_FOLL);
    }

    this_object()->seq_clear(SEQ_FOLL);

    s1 = (s1 ? s1 : "") + " " + (s2 ? s2 : "");
    s1 = implode(explode(s1 + "\n", "\n"), " ");

    words = explode(s1 + " "," ");

    if ((member_array("says:", words) >= 0) ||
        (member_array("shouts:", words) >= 0))
    {
        return 0;
    }

    ex = (mixed*)environment(this_object())->query_exit();
    calls = get_all_alarms();

    for (il = 1; il < sizeof(ex); il += 3)
    {
        if ((member_array(ex[il], words) >= 0) ||
            (member_array(ex[il] + ".", words) >= 0))
        {
            for (i=0 ; i<sizeof(calls) ; i++)
                if (calls[i][1] == "command")
                    remove_alarm(calls[i][0]);
            set_alarm(2.0, 0.0, &(this_object())->command(ex[il]));
            return 1;
        }
    }

    return 0;
}

/*
 * Sequence functions
 */

/*
 * Function name:   monster_ranwalk
 * Description:     Add a random direction to walk. Add the function call too.
 */
void
monster_ranwalk()
{
    mixed *exits;
    int il;
    string ex;

    if (!environment(this_object()))
    {
        return;
    }

    if (monster_follow && present(monster_follow, environment(this_object())))
    {
        return;
    }

    this_object()->seq_clear(SEQ_RWALK);

    if (this_object()->query_attack())
    {
        il = 10 + random(monster_ranmove);
        this_object()->seq_addfirst(SEQ_RWALK, ({ il, monster_ranwalk }) );
        return;
    }

    exits = filter_exits(environment(this_object())->query_exit());

    if (!sizeof(exits))
    {
        if (monster_home)
        {
            seteuid(getuid(this_object()));
            this_object()->move_living("home", monster_home);
        }
        il = 10 + random(monster_ranmove);
        this_object()->seq_addfirst(SEQ_RWALK, ({ il, monster_ranwalk }) );
        return;
    }

    il = random(sizeof(exits)) / 3;
    ex = exits[il * 3 + 1];

    il = 10 + random(monster_ranmove);
    this_object()->seq_addfirst(SEQ_RWALK,
        ({ &oke_to_move(ex), il, monster_ranwalk }) );
}

/*
 * Function name: oke_to_move
 * Description:   Checks whether the npc is fighting someone, if he is in
 *                in combat, the move-command will be delayed till the 
 *                war is over.
 * Arguments:     exit  : the exit that is generated for the monster to take.
 * Returns:       0     : if in combat
 *                string: the exit that the monster takes if not in combat.
 */
mixed
oke_to_move(string exit)
{
    if (this_object()->query_attack())
    {
        return 0;
    }

    return exit;
}

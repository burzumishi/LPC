/* Cygnus (Dave Richards) */

#pragma save_binary
#pragma strict_types

#define CD
#undef NIGHTMARE
#undef OVERDRIVE

#ifdef CD
inherit "/cmd/std/command_driver";
#include <filepath.h>
#endif

#ifdef NIGHTMARE
#include <std.h>
inherit DAEMON;
#endif

#ifdef OVERDRIVE
#include "/secure/bin/defs.h"
#endif

#define	DV_NO_LOAD	0
#define	DV_COUNT	1
#define	DV_DATE		2
#define	DV_INHERIT	3
#define	DV_IARC1	4
#define	DV_IARC2	5
#define	DV_OARC		6
#define	DV_SIZE		7

#ifdef MUDOS
int
file_time(string path)
{
    mixed *stat_vec;

    stat_vec = stat(path);
    if (pointerp(stat_vec) && sizeof(stat_vec) == 3)
	return stat_vec[1];

    return 0;
}
#endif

#ifdef MUDOS
int
object_time(object ob)
{
    mixed *stat_vec;

    stat_vec = stat(file_name(ob) + ".c");
    if (pointerp(stat_vec) && sizeof(stat_vec) == 3)
	return stat_vec[2];

    return -1;
}
#endif

#ifdef MUDOS
mapping
m_delete(mapping m, mixed key)
{
    map_delete(m, key);

    return m;
}
#endif

#ifdef MUDOS
mixed *
m_indexes(mapping m)
{
    return keys(m);
}

#endif

#ifdef MUDOS
mixed *
m_values(mapping m)
{
    return values(m);
}
#endif

#ifdef MUDOS
int
m_sizeof(mapping m)
{
    return sizeof(m);
}
#endif

#ifdef CD
string
get_soul_id()
{
    return "make";
}
#endif

#ifdef CD
int
query_tool_soul()
{
    return 1;
}
#endif

#ifdef CD
mapping
query_cmdlist()
{
    return ([ "make" : "verb_make" ]);
}
#endif

object
load_object(string path)
{
    string error;

    error = catch(path->teleledningsanka());
    if (stringp(error))
	write("Cannot load " + path + "\n");

    return find_object(path);
}

object
find_or_load_object(string path)
{
    object ob;

    ob = find_object(path);
    if (!objectp(ob))
    {
	ob = load_object(path);
	if (objectp(ob))
	    write("Loaded " + file_name(ob) + ".c\n");
    }

    return ob;
}

mixed
make_dependency_graph(mapping *data_vec, string path)
{
    object ob;
    string *inherit_vec;
    string *oarc_vec;
    int i;

    if (data_vec[DV_NO_LOAD][path])
	return 0;

    ob = find_or_load_object(path);
    if (!objectp(ob))
    {
	data_vec[DV_NO_LOAD][path] = 1;
	return 0;
    }

    path = file_name(ob) + ".c";

    if (pointerp(data_vec[DV_INHERIT][path]))
	return path;

    data_vec[DV_INHERIT][path] = inherit_list(ob);

#ifdef MUDOS
    inherit_vec = data_vec[DV_INHERIT][path] + ({});
#endif
#ifdef CD
    inherit_vec = data_vec[DV_INHERIT][path];
    i = member_array(path, inherit_vec);
    inherit_vec = exclude_array(inherit_vec, i, i);
#endif

    oarc_vec = ({});

    for (i = 0; i < sizeof(inherit_vec); i++)
    {
	inherit_vec[i] = make_dependency_graph(data_vec, inherit_vec[i]);
	if (stringp(inherit_vec[i]))
	    oarc_vec += ({ inherit_vec[i] });
	else
	    data_vec[DV_DATE][path] = -1;
    }

    data_vec[DV_OARC][path] = oarc_vec;

    return path;
}

void
add_dependency(mapping *data_vec, string path1, string path2)
{
    data_vec[DV_COUNT][path1]++;

    if (!pointerp(data_vec[DV_IARC1][path2]))
	data_vec[DV_IARC1][path2] = ({ path1 });
    else
	data_vec[DV_IARC1][path2] += ({ path1 });
}

void
remove_dependency(mapping *data_vec, string path)
{
    int i;

    for (i = 0; i < sizeof(data_vec[DV_IARC2][path]); i++)
	data_vec[DV_COUNT][data_vec[DV_IARC2][path][i]]--;

    data_vec[DV_COUNT] = m_delete(data_vec[DV_COUNT], path);
    data_vec[DV_IARC2] = m_delete(data_vec[DV_IARC2], path);
}

string *
topological_sort(mapping *data_vec)
{
    string *path1;
    mixed *path2;
    int i;
    int j;
    string *path3;
    status cycle;

    path1 = m_indexes(data_vec[DV_OARC]);
    path2 = m_values(data_vec[DV_OARC]);

    for (i = 0; i < sizeof(path1); i++)
    {
	if (!pointerp(data_vec[DV_IARC1][path1[i]]))
	    data_vec[DV_IARC1][path1[i]] = ({});
	for (j = 0; j < sizeof(path2[i]); j++)
	    add_dependency(data_vec, path1[i], path2[i][j]);
    }

    path3 = ({});

    data_vec[DV_IARC2] = data_vec[DV_IARC1] + ([]);

    while (m_sizeof(data_vec[DV_IARC2]) != 0)
    {
	cycle = 1;
	path1 = m_indexes(data_vec[DV_IARC2]);
	for (i = 0; i < sizeof(path1); i++)
	{
	    if (data_vec[DV_COUNT][path1[i]] == 0)
	    {
		cycle = 0;
		path3 += ({ path1[i] });
		remove_dependency(data_vec, path1[i]);
	    }
	}
	if (cycle)
	{
	    write("Cyclic dependency detected.\n");
	    return ({ });
	}
    }

    return path3;
}

void
propogate(mapping *data_vec, string path, int date)
{
    int i;

    if (date == -1)
	for (i = 0; i < sizeof(data_vec[DV_IARC1][path]); i++)
	    data_vec[DV_DATE][data_vec[DV_IARC1][path][i]] = -1;
    else
	for (i = 0; i < sizeof(data_vec[DV_IARC1][path]); i++)
	{
	    if (data_vec[DV_DATE][data_vec[DV_IARC1][path][i]] == -1)
		continue;
	    if (date > data_vec[DV_DATE][data_vec[DV_IARC1][path][i]])
		data_vec[DV_DATE][data_vec[DV_IARC1][path][i]] = date;
	}
}

status
dependency_change(string *inherit_vec1, string *inherit_vec2)
{
    if (sizeof(inherit_vec1) != sizeof(inherit_vec2))
	return 1;

    if (sizeof(inherit_vec1 - inherit_vec2) != 0)
	return 1;

    return 0;
}

status
update_object(mapping *data_vec, string path)
{
    int date;
    object ob;
    int i;

    date = file_time(path);
    if (date > data_vec[DV_DATE][path])
	data_vec[DV_DATE][path] = date;

    if (data_vec[DV_DATE][path] == -1)
	return 0;

    if (file_size(path) < 0)
    {
	write("Cannot make " + path + "\n");
	data_vec[DV_DATE][path] = -1;
	return 0;
    }

    ob = find_object(path);
    if (objectp(ob))
    {
	if (object_time(ob) >= data_vec[DV_DATE][path])
	    return 0;
#ifdef CD
	ob->remove_object();
#endif
#ifdef NIGHTMARE
	ob->remove();
#endif
#ifdef OVERDRIVE
	destruct(ob);
#endif
	if (objectp(ob))
	{
	    write("Could not update " + path + "\n");
	    data_vec[DV_DATE][path] = -1;
	    return 0;
	}
    }
    ob = load_object(path);
    if (!objectp(ob))
    {
	data_vec[DV_DATE][path] = -1;
	return 0;
    }
    write("Updated " + path + "\n");
    date = object_time(ob);
    if (date > data_vec[DV_DATE][path])
	data_vec[DV_DATE][path] = date;
    if (dependency_change(data_vec[DV_INHERIT][path], inherit_list(ob)))
	return 1;
    return 0;
}

status
check_dependency_graph(mapping *data_vec, string *path)
{
    int i;
    mixed *stat_vec;
    object ob;

    for (i = 0; i < sizeof(path); i++)
    {
	if (update_object(data_vec, path[i]))
	    return 1;
	propogate(data_vec, path[i], data_vec[DV_DATE][path[i]]);
    }

    return 0;
}

void
make(string path)
{
    mapping *data_vec;
    int i;
    status changed;

    for (;;)
    {
	data_vec = allocate(DV_SIZE);
	for (i = 0; i < DV_SIZE; i++)
	    data_vec[i] = ([]);
	make_dependency_graph(data_vec, path);
	changed = check_dependency_graph(data_vec,
	    topological_sort(data_vec));
	if (!changed)
	    return;
    }
}

status
#ifdef CD
verb_make(string command)
#endif
#ifdef NIGHTMARE
cmd_make(string command)
#endif
#ifdef OVERDRIVE
do_command(string command)
#endif
{
    object ob;
    int argc;
    string *argv;
    int i;

#ifdef OVERDRIVE
    CHECK_VALID(EXEC, START_POWER)
#endif

#if defined(NIGHTMARE) || defined(OVERDRIVE)
    seteuid(geteuid(this_player()));
#endif

    if (command == "-v")
    {
	write("Version " + explode("1.2", " ")[1] + "\n");
	return 1;
    }


    if (!stringp(command))
    {
	ob = environment(this_player());
	if (objectp(ob))
	    make(file_name(ob));
    }
    else
    {
	argv = explode(command, " ") - ({ "" });
	argc = sizeof(argv);

	for (i = 0; i < argc; i++)
	{
#ifdef CD
	    make(FTPATH(this_interactive()->query_path(), argv[i]));
#endif
#ifdef NIGHTMARE
	    make(resolv_path(this_player()->get_path(), argv[i]));
#endif
#ifdef OVERDRIVE
	    make((string)this_player()->resolve_path(argv[i]));
#endif
	}
    }

    return 1;
}

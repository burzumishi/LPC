inherit "/std/mapsquare";

object map;

/*
 * Prototypes
 */
int get_map(string str);

void
create_random_encounter()
{
    map = clone_object("/std/map");
    map->set_center(0, 0);
    map->set_radius(5);
    map->move(this_object());
}

void
create_mapsquare(int wlx, int wly, int slx,int sly)
{
    ::create_mapsquare(wlx, wly, slx, sly);
	
    set_short("Space");

    if (query_random_encounter())
	if (random(9, wlx + 6) == slx && random(9, wly) == sly)
	    create_random_encounter();

    set_long("There is nothing but empty space here.\n" +
	     "You can go in any direction.\n");
    set_noshow_obvious(1);
}

void
enter_inv(object ob, object from)
{
    if (map && present(map, this_object()))
	add_action(get_map, "get");
}

int
get_map(string str)
{
    if(str != "map") 
	return 0;

    /*
     * This is not implemented yet so we give a debug message instead
     */
#if 0
    move_random_encounter();
#else
    write("You got the map and the encounter was moved\n");
#endif
    return 1;
}

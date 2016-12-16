inherit "/std/mapsquare";

void
create_mapsquare(int wlx, int wly, int slx,int sly)
{
    ::create_mapsquare(wlx, wly, slx, sly);
	
    set_short("Black hole");

    set_long("This is a black hole. You ought to be sucked away.\n" +
	     "You can go in any direction.\n");
    set_noshow_obvious(1);
}

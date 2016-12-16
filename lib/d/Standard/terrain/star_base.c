inherit "/std/mapsquare";

string get_pdesc(string *a);

void
create_mapsquare(int wlx, int wly, int slx, int sly)
{
    string *paths, pdesc;
    
    ::create_mapsquare(wlx, wly, slx, sly);
    set_short("Star base");
    paths = query_roadpath(wlx, wly, slx, sly);
    if (sizeof(paths) > 0)
	pdesc = "\n" + "There are paths leading " + get_pdesc(paths) + ".";
    else
	pdesc = "";

    if (slx == 4 && sly == 4)
    {
	set_long("This is the centre of the starbase. " +
		 "They just started building the power plant here.\n" +
		 "You can fly in any direction.\n");
    }
    else if ((slx > 2 && slx < 6) && (sly > 2 && sly < 6))
    {
	set_long("This is near the midle of the base. " +
		 "There are lots odd framework in place here.\n" +
		 "You can walk in any direction.\n");
    }
    else if ((slx > 1 && slx < 7) && (sly > 1 && sly < 7))
    {
	set_long("This is the level of the base that will contain quarters." +
		 pdesc + "\n" +
		 "You can fly in any direction.\n");
    }
    else if ((slx > 0 && slx < 8) && (sly > 0 && sly < 8))
    {
	set_long("You are just inside the hull of the star base. " +
		 "This is where the cargo holds will be." +
		 pdesc + "\n" +
		 "You can fly in any direction.\n");
    }
    else
    {
	set_long("You are just outside what looks like a half finished " +
		 "star base bigger than Darth Vader's Death Star. " +
                 "Right now the place seems to be deserted." +
		 pdesc + "\n" +
		 "You can fly in any direction.\n");
    }
    set_noshow_obvious(1);
}

string
get_pdesc(string *a)
{
    if (!a)
	return 0;
    else if (sizeof(a) == 1)
	return a[0];
    else if (sizeof(a) == 2)
	return implode(a, " and ");
    else
	return implode(a[0..sizeof(a)-2], ", ") + " and " + a[sizeof(a)-1];
}

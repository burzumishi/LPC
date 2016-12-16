inherit "/d/Emerald/std/ship";

void
create_ship()
{
    object cap;
    
    seteuid(getuid());
    set_cabin("/d/Emerald/plugh/ship/cabin");
    set_deck("/d/Emerald/plugh/ship/deck");
    cap=clone_object("/d/Emerald/plugh/ship/captain");
    set_captain(cap);
    set_places_to_go(({"/d/Emerald/room/lake/pier",
		       "/d/Emerald/room/lake/lake1",
		       "/d/Emerald/room/lake/lake2",
		       "/d/Emerald/room/lake/channel1",
		       "/d/Emerald/room/lake/channel2",
		       "/d/Emerald/room/lake/channel3",
		       "/d/Emerald/room/mine/beach",
		       "/d/Standard/start/human/town/pier3",
		       "/d/Emerald/room/mine/beach",
		       "/d/Emerald/room/lake/channel3",
		       "/d/Emerald/room/lake/channel2",
		       "/d/Emerald/room/lake/channel1",
		       "/d/Emerald/room/lake/lake2",
		       "/d/Emerald/room/lake/lake1"}));
    set_time_between_stops(({24,
			     3,
			     3,
			     3,
			     3,
			     3,
			     3,
			     24,
			     3,
			     3,
			     3,
			     3,
			     3,
			     3}));
    set_ticket_id("the emerald line");
    set_name("ship");
    add_name("boat");
    add_adj("nifty");
    set_long("A really neat ship.\n");
}

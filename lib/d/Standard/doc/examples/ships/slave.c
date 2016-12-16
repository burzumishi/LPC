inherit "/std/monster";

#include <ss_types.h>

string
random_color()
{
    int i;
	
    i = random(27);
    if (i < 3)
	return "black";
    else if (i < 6)
	return "white";
    else if (i < 7)
	return "chinese";
    else if (i < 8)
	return "japanese";
    else if (i < 9)
	return "scandinavian";
    else if (i < 10)
	return "norwegian";
    else if (i < 11)
	return "finnish";
    else if (i < 12)
	return "american";
    else if (i < 13)
	return "canadian";
    else if (i < 14)
	return "dutch";
    else if (i < 15)
	return "australian";
    else if (i < 16)
	return "british";
    else if (i < 17)
	return "danish";
    else if (i < 18)
	return "german";
    else if (i < 19)
	return "gothenburg";
    else if (i < 20)
	return "chalmers";
    else if (i < 21)
	return "archwizard";
    else if (i < 22) 
	return "mit";
    else if (i < 23)
	return "linkoping";
    else if (i < 24)
	return "icelandic";
    else if (i < 25)
	return "kth";
    else
	return "hellfire";
}

void
create_monster()
{
    string color;

    color = random_color();
    
    set_name("slave");
    add_adj("strong");
    add_adj(color);
    set_long("It as a very strong, "+color+" slave.\n");

    default_config_npc(20);

    set_base_stat(SS_STR, 120);
    set_base_stat(SS_CON, 120);
    set_hp(1000);

    seq_new("do_stuff");
    seq_addfirst("do_stuff", ({ "@@get_oar", "say Let's get this ship moving, shall we?"}));
}

void
get_oar()
{
    object oar;

    seteuid(getuid());
    oar=clone_object("/d/Emerald/plugh/ship/oar");
    if (oar) {
	oar->move(this_object());
	command("wield oar");
    }
}

inherit "/std/monster";
inherit "/lib/trade";

#include <stdproperties.h>
#include <macros.h>
#include <ss_types.h>
#include <money.h>

#define	NUM	sizeof(MONEY_TYPES)

object	ship;

void
do_die(object killer)
{
    ::do_die(killer);
    if (ship)
	ship->stop_ship("The ship stops; someone killed the captain!\n");
}

void
set_ship(object s)
{
    ship=s;
}

void
create_monster()
{
    set_name("crunch");
    add_name("captain");
    set_living_name("crunch");
    set_race_name("human");
    add_adj("friendly");
    add_adj("animated");

    set_stats(({ 120, 180, 120, 180, 150, 190 }));
    set_skill(SS_SWIM, 100);
    heal_hp(10000);

    config_default_trade();

    trig_new("%w 'introduces' %s", "react_introduce");
}

string
query_title()
{
    return "the famous captain";
}

void
init_living()
{
    add_action("buy", "buy");
}

int
buy(string str)
{
    object buy_ob;
    string what, for_c, get_c, change;
    int    *result, m_res, price;

    if (!str) {
	notify_fail("Buy what?\n");
	return 0;
    }
    if (sscanf(str, "%s with %s and get %s", what, for_c, get_c) != 3)
	if (sscanf(str, "%s with %s", what, for_c) != 2)
	    what = str;
    if (what != "ticket") {
	notify_fail("I have no such thing for sale.\n");
	return 0;
    }
    buy_ob=clone_object("/d/Emerald/plugh/ship/ticket");
    if (!buy_ob) {
	notify_fail("I seem to be out of tickets.\n");
	return 0;
    }
    price=buy_ob->query_prop(OBJ_I_VALUE);
    result=pay(price,this_player(),for_c,0,0,get_c);
    if (sizeof(result)==1) {
	buy_ob->remove_object();
	return 0;
    }
    if (buy_ob->move(this_player())) {
	write("You drop the " + buy_ob->short() + " on the ground.\n");
	say(QCTNAME(this_player()) + " drops one " + buy_ob->short() +
	    " on the ground.\n");
	buy_ob->move(environment(this_player()));
    }
    else {
	write("You get your " + buy_ob->short() + ".\n");
	say(QCTNAME(this_player()) + " buys a " + buy_ob->short() + ".\n");
    }
    write("You pay " + text(result[0 .. NUM - 1]) + ".\n");
    if (change)
	write("You get " + change + " as change.\n");
    return 1;
}

void
react_introduce(string person,string who)
{
    call_out("introduce_me", 4);
}

void
introduce_me()
{
    command("introduce myself");
}

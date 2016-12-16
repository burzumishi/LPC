inherit "/std/monster";

#include <const.h>

string
ask_fun()
{
    command("say I'll tell you what I think is fun...eating " +
        this_player()->query_race_name() + "!!!!");
    command("laugh");
    return ""; 
}

void
create_monster()
{
    set_name("goblin");
    set_long("It's very small but it looks back on you and shows its " +
        "teeth.\n");
    set_race_name("goblin");
    set_adj("small");

    set_gender(G_NEUTER);

    set_stats(({ 10, 10, 10, 10, 10, 10 }));

    /* This is the message given when someone asks us about something we
     * don't know about (anything not defined in add_ask())
     */
    set_default_answer("The Goblin says: I don't know what you are talking " +
        "about.\n");

    /* Give an answer when asked about "weather" or "time" */
    add_ask(({"weather", "time"}), "The goblin frowns: I don't care about " +
        "weather nor time.\n");

    /* Give an answer when asked about "fun", but we want to do more than
     * just give a message.
     */
    add_ask("fun", ask_fun);
}

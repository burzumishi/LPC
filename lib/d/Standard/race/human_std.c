/*
 *  human_std.c
 *
 *  This is the race object used for players of race: human
 */
#pragma strict_types

inherit "/config/race/generic";

void
start_player()
{
    start_mail("/d/Standard/start/mailroom");
    ::start_player();
}

string
query_race()
{
    return "human";
}

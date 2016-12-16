inherit "/std/object";

void
init()
{
    ::init();

    set_alarm(0.5, 0.0, &tell_object(this_player(),
        "\nPostmaster is busy burning messages nobody is interested in.\n" +
        "He is not able to handle your mail right now.\n"));
    set_alarm(1.0, 0.0, remove_object);
}

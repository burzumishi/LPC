/*
 * /d/Standard/std/special_stuff.c
 */
#pragma strict_types
#pragma no_clone

#define WIZ_PATH "/d/Standard/doc/infowiz/"

void
start_special(mixed qroom)
{
    call_other(qroom, "query_mail");
}

void
finger_special()
{
    string file, nam;

    nam = this_object()->query_real_name(); 
    file = WIZ_PATH + nam; 
    if (file_size(file) >= 0)
    {
	write("--------- Special mud info on: " + capitalize(nam) + "\n");
	cat(file);
    }
}
 

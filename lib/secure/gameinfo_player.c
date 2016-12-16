/*
 * gameinfo.c
 * 
 * Game information system.


 Reads the file GAMEINFO_INFO (as defined in config.h)

    This file is on the form:

    ##section number##
    Text.......
    ##section number##
    Text.......
    ##section number##
    Text.......

    Sectionnumbers MUST be ordered and are on the form 'n.m.k.l. etc'
    Text can hold '#include "filename"'

 */
#pragma strict_types

#include <config.h>

static mixed    *sect_array;	/* The array holding section information */
static string   *text_array;    /* Array holding all sections text */
static string	cur_sect,       /* Current section to display */
                cur_menu;       /* Current menu */
		

static void write_info();
static void do_menu(string s_num);
void quit();
string fix_menu(string str);

void
enter_game()
{
    string data, *split, *split2, str, fnam;
    int il, il2;

    data = read_file(GAMEINFO_INFO);
    if(!data)
    {
	write_socket("Panic! Couldn't read: " + GAMEINFO_INFO + ".\n");
	quit();
    }

    split = explode(data + "##", "##");
/* discard first empty element */
    split = split[1..sizeof(split)];

    sect_array = ({});
    text_array = ({});

    for (il = 0; il < sizeof(split); il += 2)
    {
	sect_array += ({ split[il] });
	split2 = explode(split[il + 1], "#include");
	if (sizeof(split2) < 2)
	    text_array += ({ split[il + 1] });
	else
	{
	   for (il2 = 1; il2 < sizeof(split2); il2++)
	   {
	       if (sscanf(split2[il2],"\"%s\"%s", fnam, str) == 2)
		   split2[il2] = read_file(fnam) + str;
	   }
	   text_array += ({ implode(split2, " ") });
        }
    }

    enable_commands();
    write_socket("\nType 'quit' to leave the gameinfo mode.\n\n");
    cur_sect = sect_array[0];
    cur_menu = fix_menu(cur_sect);
    write_info();
}

static void
do_menu(string s_num)
{
    int max, num;

    if (s_num == "quit")
    {
	quit();
	return;
    }

    num = member_array(s_num, sect_array);
    if (num >= 0)
	cur_sect = s_num;
    else
	write_socket("No such section: " + s_num + "\n");
/*
    sscanf(s_num, "%d", num);

    max = sizeof(c_arr);

    if (!sizeof(c_arr[1]))
    {
	if (c_arr[0] == "end")
	    return 1;

	c_arr = info_array;
	write_info();
	return;
    }

    if (num < 1 || num >= max)
    {
	c_arr = p_arr;
	write_info();
	return;
    }

    c_arr = c_arr[num];
*/
    write_info();
}

void
time_out()
{
    write_socket("Gameinfo time out.\n");
    quit();
}

void
write_info()
{
    mixed *calls = get_all_alarms();
    int pos;

    for (pos=0 ; pos<sizeof(calls) ; pos++)
	if (calls[pos][1] == "time_out")
	    remove_alarm(calls[pos][0]);
    set_alarm(600.0, 0.0, time_out);
    
    pos = member_array(cur_sect, sect_array);
    if (pos >= 0)
	write_socket(process_string(text_array[pos], 1));
    else
	write_socket("No such section: " + cur_sect + "\n");
    
    if (pos < (sizeof(sect_array)-1) &&
	strlen(sect_array[pos+1]) > strlen(sect_array[pos]))
	cur_menu = fix_menu(cur_sect);

    write_socket("\n" + cur_menu);
    
    input_to(do_menu);
}

static string
fix_menu(string str)
{
    int pos, il, clen;

    return "Give section: ";
// This looks like dead code, I wonder what it was supposed to do /Tintin
#if 0
    pos = member_array(str, sect_array);
    
    if (cur_menu)
    {
	pos = member_array(str, sect_array);
	if (pos < 0 || pos == sizeof(sect_array))
	    return cur_menu;

	if (strlen(sect_array[pos+1]) <= strlen(sect_array[pos]))
	    return cur_menu;
    }

    if (pos < 0)
	return "No sections available: 0-0";

    clen = strlen(cur_sect);
    for (il = pos + 1; il < sizeof(sect_array); il++)
    {
    }
#endif
}

static void
quit()
{
    destruct();
}

string
query_real_name()
{
    return "gameinfo";
}

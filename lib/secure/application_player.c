/*
 * application_player.c
 * 
 * Blocked site player application procedures.
 */

#pragma no_inherit
#pragma save_binary
#pragma strict_types

#include <config.h>
#include <std.h>
#include <macros.h>

/*
 * Prototypes
 */
static void get_mline(string str);
static void get_nline(string str);
static void get_sline(string str);
static void time_out();

/*
 * Global variables.
 */
static string result = "";
static string *info;
static int    pos = 0;
static int    time_out_alarm;

#define SINGLE_TIME_OUT 300.0
#define MULTI_TIME_OUT  900.0

/*
 * Function name: write_info
 * Description  : This function will display the next line of the application
 *                process to the player and input the next line. If the
 *                player is done, the message will be inserted in the
 *                application board.
 */
void
write_info()
{
    string *part, epz, whom, ip, in;
    object ob;

    remove_alarm(time_out_alarm);

    if (pos >= sizeof(info))
    {
	catch(LOAD_ERR(APPLICATION_BOARD_LOC));
	ob = present("application board", find_object(APPLICATION_BOARD_LOC));

	if (!objectp(ob))
	{
	    write_socket("\nThe application could not be saved. Please try " +
		"again at a later date when the bug has been found and " +
		"fixed. If you know someone with access to the game, ask " +
		"him or to mention the problem to the administration.\n");
	}
	else
	{
	    whom = query_ip_ident(this_object());
	    whom = strlen(whom) > 0 ? whom : "Unknown";
	    in = query_ip_name(this_object());
	    ip = query_ip_number(this_object());
	    whom += "@" + (in == ip ? in : in + " (" + ip + ")");
	    ob->new_msg(whom);
	    result = "Application from " + whom + 
		" at " + ctime(time()) + "\n" + result + "\n";
	    ob->done_editing(result);
	    write_socket("\nYour application has been filed.\n");
	}

	destruct();
	return;
    }

    part = explode(info[pos++], "&&");

    write_socket(part[2] + "\nApplication entry> ");

    if ((epz = explode(part[0], "\n")[0]) == "")
    {
	epz = explode(part[0], "\n")[1];
    }

    if (epz == "m")
    {
	time_out_alarm = set_alarm(MULTI_TIME_OUT, 0.0, time_out);
	result += explode(part[1], "\n")[1];
	input_to(get_mline);
    }
    else if (epz == "-")
    {
	time_out_alarm = set_alarm(SINGLE_TIME_OUT, 0.0, time_out);
	input_to(get_nline);
    }
    else
    {
	time_out_alarm = set_alarm(SINGLE_TIME_OUT, 0.0, time_out);
	result += explode(part[1], "\n")[1];
	input_to(get_sline);
    }
}

/*
 * Function name: enter_game
 * Description  : This function is called from the login object and enables
 *                the player to connect. It takes care of the necessary
 *                initialization before the player is allowed to file his
 *                request.
 */
void
enter_game()
{
    string data;
    int bl;

    set_screen_width(80);

    bl = SECURITY->check_newplayer(query_ip_number(this_object()));
    if (bl == 0)
    {
	write_socket("Your site isn't blocked. Log in as usual.\n");

	destruct();
	return;
    }
    else if (bl == 1)
    {
	write_socket("Your site has been banned, not just simply blocked.\n" +
	    "No one will be admitted from your site.\n");

	destruct();
	return;
    }
    
    data = read_file(APPLICATION_INFO);
    if (!strlen(data))
    {
	write_socket("Panic! Couldn't read: " + APPLICATION_INFO + ".\n");

	destruct();
	return;
    }

    info = explode(data, "##");

    enable_commands();
    write_info();
}

/*
 * Function name: quit
 * Description  : Remove the object from memory when the player aborts.
 */
static void
quit()
{
    write_socket("\nAborting application.\n");

    destruct();
}

/*
 * Function name: get_sline
 * Description  : Get single line. This function is called when the player
 *                may enter a single line of text based on a question.
 * Arguments    : string str - the answer of the player.
 */
static void
get_sline(string str)
{
    if (str == "quit")
    {
	quit();
	return;
    }
    
    result += str + "\n";
    write_info();
}

/*
 * Function name: get_nline
 * Description  : Get new line. This function waits until the player pressed
 *                'return' when the script wants him to.
 * Arguments    : string str - the newline.
 */
static void
get_nline(string str)
{
    if (str == "quit")
    {
	quit();
	return;
    }

    write_info();
}

/*
 * Function name: get_mline
 * Description  : Get multi line. Get input from the player when (s)he is
 *                writing the comment part of the request.
 * Arguments    : string str - the input from the player.
 */
static void
get_mline(string str)
{
    if (str == "quit")
    {
	quit();
	return;
    }

    if (str == "**")
    {
	write_info();
	return;
    }

    result += str + "\n\t\t";
    write_socket("Application entry> ");
    input_to(get_mline);
}

/*
 * Function name: time_out
 * Description  : Called after some time to close the connection.
 */
static void
time_out()
{
    write_socket("Application time out.\n");

    destruct();
}

/*
 * Function name: query_real_name
 * Description  : Return the real name of the this object.
 * Returns      : string - always 'application'.
 */
public string
query_real_name()
{
    return "application";
}

/*
 * Function name: query_prevent_shadow
 * Description  : Prevent shadowing of this object.
 * Returns      : int 1 - always.
 */
nomask public int
query_prevent_shadow()
{
    return 1;
}

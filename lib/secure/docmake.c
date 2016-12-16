/*
   docmake.c

----------------------------------------------------------------------------
               THIS FILE MISUSES READ_FILE IN A MAJOR WAY. 

    DO NOT DO ANYTHING EVEN REMOTELY LIKE THIS ANYWHERE ELSE IN THE MUD
----------------------------------------------------------------------------

   This little thing reads source files and extract useful things
   such as comments and documentation. The result of its efforts are
   reaped by the 'sman' and 'lman' commands.

   With some good recommendations this object might actually improve
   the level of existing documentation in the MUD. As it expects comments
   to be formated in a special way it might standarize the way code are
   commented too.

   Wizards can either use this object through the command 'sdoc' or by
   a direct call to 'doc_file'

   From commandline you can do:
        'sdoc docdir mainfile'

   or call the function:
        doc_file(string docdir, string file)

   This object is also used directly by 'sman' and 'lman' when the
   sourcefiles are newer than the documentation files. 

   Mainfile is the name of the file to document and also the name in
   the documentation directory. If the mainfile includes other ".c" files
   such as /std/living.c does, then these are automatically found and
   scanned by the functionscanner.

   NOTE!
         If the beginning of the pathnames 'docdir' and 'mainfile' are the
	 same that common part will be deleted from 'mainfile' when put
	 under 'docdir'. This ensures for example:

	    sdoc /d/Genesis/doc /d/Genesis/obj/knife
	        ^^^^^^^^^^^^^^ ^^^^^^^^^^^^^^^^^^^^
		    docdir           mainfile

	 will end up under /d/Genesis/doc/obj/knife and
         NOT: /d/Genesis/doc/d/Genesis/obj/knife 

   Example:  To document "/std/npc.c" which includes "/std/seqaction.c" and
             "/std/trigaction.c" in /doc/sman we would do:
    
	  doc_file("/doc/sman", "/std/npc")

   This would give us the subdir entry "/doc/sman/std/npc".
   This subdir would include one file for each commented function in
   the three files above. Each file consists of one index line and any
   number of comment lines.

   As default the comment from the source are put on the comment lines. This
   can be replaced by editing the file though. When the docmaker is run again
   on the same source file. The comment will NOT be replaced. It is only the
   index line that will change. The index line holds a reference to the
   source code. Making the 'sman -s' command possible.

   Comments are expected to look like below over create:
   It may apart from 'Description:' hold entries describing arguments etc.

*/
#pragma strict_types

#define READ_CHUNK 100                 /* Lines / read when funcsearching */
#define FIND_TRIES 10                  /* Number of tries to find funcs */
#define WRITE_CHUNK 8                  /* Number of files to write / turn */
#define MAX_FUNHEAD_LINES 4	       /* Max lines in a function head */ 

#define DOC_TRIG "/*"

#include <filepath.h>
#include <macros.h>
#include <std.h>
#include <stdproperties.h>

static 	mixed 	*acc_func;             /* Accumulated functions */
static	string	*left_lines;	       /* Lines left from last read */
static	string	*files_left;	       /* #included files left to search */
static  string	mainobfile;	       /* Current obj being documented */
static  string  *who_told_us;	       /* Who should be told that doc ready */
static	mixed	*order_stack;          /* docorders, each entry on the form:
					 ({ docdir, mainfile, ({ wiznames }) })
    				        */

void find_funcs(string file, function call_back_fun, string arg);
int check_comment(string *lines, int lin, int start, string src);
int potential_func(string *lines, int flin, int clin, int start, string cmt, string src);
void doc_found(mixed *arr, string mainpath);
void accumul_funcs(string file, function fun, int line, mixed arg);
void doc_create(mixed *arr, string path, string new_euid);
void doc_clean_obsolete(string *functions, string path, string new_euid);
public void doc_next();

/*
 * Function name: create
 * Description:   Initializes this object and resets its status
 */
void
create()
{
    acc_func = 0; 
    order_stack = ({});
}

public void
reset_euid(string e) { seteuid(e); }

/*
 * Call this to get a copy of the orders pending
 */
public mixed *
doc_query_orders()
{
    if (!pointerp(order_stack))
	return ({});
    else
	return order_stack + ({});
}

/*
 * Call this to get the status of the current documentation
 */
public string
doc_query_status()
{
    if (pointerp(acc_func))
	return "Documenting: " + mainobfile + " in " + order_stack[0][0] +
	    " " + sizeof(acc_func) + " functions found.\n";
    else
	return "No ongoing documentation.\n";
}

/*
 * Call this to reset the docmaker
 */
public void
doc_reset()
{
    mixed *calls = get_all_alarms();
    int i;
    object player;

    if (pointerp(acc_func))
    {
	for (i = 0; i < sizeof(who_told_us); i++)
	{
	    player = find_player(who_told_us[i]);
	    if (player && this_interactive())
		tell_object(player,
			    "Docscribe tells you: Documentation of: " +
			    mainobfile + " aborted by " + 
			    this_interactive()->query_real_name() + "\n");
	}
    }

    for (i=0 ; i<sizeof(calls) ; i++)
	if (calls[i][1] == "accumul_funs" ||
	    calls[i][1] == "doc_next" ||
	    calls[i][1] == "doc_create")
	    remove_alarm(calls[i][0]);
    
    acc_func = 0;
    order_stack = ({});
}

/*
 * Call this to start documenting a file
 */
public void
doc_file(string docdir, string mainfile)
{
    int i;

    docdir = FTPATH((string)this_player()->query_path() + "/", docdir);
    mainfile = FTPATH((string)this_player()->query_path() + "/", mainfile);

    for (i = 0; i < sizeof(order_stack); i++)
    {
	if (order_stack[i][0] == docdir &&
	    order_stack[i][1] == mainfile)
	{	    
	    order_stack[i][2] += ({ this_player()->query_real_name() });
	    return;
	}
    }

    if (!pointerp(order_stack))
	order_stack = ({});
    order_stack += 
	({ ({ docdir, mainfile, ({ this_player()->query_real_name() }) }) });

    if (!pointerp(acc_func))
	doc_next();
}

/*
 * Document the next file in the order stack
 */
void
doc_next()
{
    string mainpath, *dd, *fpath, msg, file;
    object player;
    int okeffuser, i;

    /* Can not doc another one until this one is ready
    */
    if (pointerp(acc_func))
	return;

    if (!sizeof(order_stack))
	return;

    who_told_us = order_stack[0][2];
    file = order_stack[0][1];

    if (sscanf(file, "%s.c", mainpath) != 1)
    {
	mainpath = file; file += ".c";
    }

    dd = explode(order_stack[0][0], "/");
    fpath = explode(mainpath, "/");

    while (sizeof(dd) && sizeof(fpath) && (dd[0] == fpath[0]))
    {
	fpath = fpath[1..sizeof(fpath)];
	dd = dd[1..sizeof(dd)];
    }

    mainpath = order_stack[0][0] + "/" + implode(fpath, "/");
    okeffuser = 0;
    if (file_size(file) >= 0)
	msg = break_string("Docscribe tells you: Starting to document: " +
	    file + ", in: " + mainpath, 76) + "\n" ;
    else
	msg = "Docscribe tells you: No such file: " + file + "\n";

    for (i = 0; i < sizeof(who_told_us); i++)
    {
	player = find_player(who_told_us[i]);
	if (player)
	    tell_object(player,msg);
			
	if (!okeffuser &&
	    SECURITY->valid_write(mainpath, who_told_us[i], "write_file") &&
	    SECURITY->valid_read(file, who_told_us[i], "read_file"))
	{
	    okeffuser = 1;
	    seteuid(who_told_us[i]);
	}
    }
    if (!okeffuser || file_size(file) < 0)
    {
	for (i = 0; i < sizeof(who_told_us); i++)
	{
	    player = find_player(who_told_us[i]);
	    if (player)
		tell_object(player,
			    break_string("Docscribe tells you: Can't document: " + file + ", in: " + mainpath + ", no access.\n", 76));
	}
	order_stack = order_stack[1..sizeof(order_stack)];
    }
    else
    {
	mainobfile = file;
	files_left = ({});
	find_funcs(file, doc_found, mainpath);
    }
}


/*
 * Description:   Finds the commented functions in a file.
 * Returns:	  Gives an array of arrays: public/static, funcname, startlin,
 *                comment.
 *		  The array is constructed and returned
 *		  to a given callback function.
 */
void
find_funcs(string file, function call_back_fun, string call_back_arg)
{
    string 	*lines;
    int		start;

    acc_func = ({});
    left_lines = ({});

    set_alarm(1.0, 0.0, &accumul_funcs(file, call_back_fun, 0, call_back_arg));
}

/*
 * Description:   Accumulates functions  
 * Arguments:
 *       arg_arr[0] == The file to document
 *       arg_arr[1] == The callback function
 *       arg_arr[2] == The line to start reading from in the file
 *       arg_arr[3] == The argument to the callback function (doc subdirname)
 */
void
accumul_funcs(string file, function fun, int line, mixed arg)
{
    string 	*lines, data, *find, str, a, b;
    int		i, lin, start, numtries, numleft;
    object	player;
    
    /* The docmaker has been reset, we assume the reset took care of
       starting up for the next file
    */
    if (!pointerp(acc_func))
	return;

    numtries = 0; start = line;
    
    while (numtries < FIND_TRIES)
    {
	data = read_file(file, start, READ_CHUNK);

	start += READ_CHUNK;

	/* If we are ready, tell the callback function 
	 */
	if (!data) 
	{
	    fun(acc_func, arg);
	    return;
	}

	data = (sizeof(left_lines) ?
		implode(left_lines, "\n") + "\n" : "") + data;
	numleft = sizeof(left_lines);
	left_lines = ({});

	/*
	    Handle #include .c files
	 */
	find = explode("dummy" + data + "dummy", "#include");
	if (sizeof(find) > 1)
	{
	    for (lin = 0; lin < sizeof(find); lin++)
	    {
		if (sscanf(find[lin], "%s\"%s.c%s", a, str, b) == 3)
		{
		    files_left += ({ str + ".c" });
		}
	    }
	}

	find = explode(" " + data + " ", DOC_TRIG);
	
	if (sizeof(find) > 1)
	    break;

	numtries++;
    }

    set_alarm(1.0, 0.0, &accumul_funcs(file, fun, start, arg));
	
    if (numtries == FIND_TRIES)
	return;

    lines = explode(data, "\n");

    /* Search for functions after comments that start first on a line
    */
    for (lin = 0; lin < sizeof(lines); lin++)
    {
	if (extract(lines[lin], 0, 1) == "/*")
	    lin = check_comment(lines, lin, 
				start - READ_CHUNK - numleft, file);
    }
    return;
}

/*
 * Description: Find the end of a comment
 * Returns:     The last line looked at
 */
int 
check_comment(string *lines, int lin, int start, string src)
{
    int il;
    string cmt;

    for (il = lin; il < sizeof(lines); il ++)
    {
	if (sizeof(explode("dummy" + lines[il] + "dummy", "*/")) > 1)
	{
	    cmt = implode(slice_array(lines, lin, il), "\n");
	    return potential_func(lines, il + 1, lin, start, cmt, src);
	}
    }

    left_lines = slice_array(lines, lin, sizeof(lines));
    return sizeof(lines);
}


/*
 * Description: Try to parse a function trailing a comment. If the comment is 
 *              incomplete its first part is stored in 'left_lines' and is
 *		then added first on the next readchunk (in accumul_funcs).
 * Arguments:
 *             lines - All the current source lines under scrutiny
 *             flin  - Line to start looking for a function
 *             clin  - Line where comment starts
 *             start - Line number in file of lines[0]
 *             cmt   - The comment as one string
 *             src   - The filename of the file under scrutiny
 * Returns: 
 *             The last line in lines looked at
 */
int 
potential_func(string *lines, int flin, int clin, 
	       int start, string cmt, string src)
{
    int il, max;
    string str, a, b, c, d, *e;
    mixed *fund;

    if (flin >= sizeof(lines))
    {
	left_lines = slice_array(lines, clin, sizeof(lines));
	return sizeof(lines);
    }

    if (sscanf(lines[flin], "%s(%s)%s{%s", a, b, c, d) != 4)
    {
	for (max = flin + 1; ((max - flin) < MAX_FUNHEAD_LINES); max++)
	{
	    if (max < sizeof(lines) && (strlen(lines[max]) > 1) &&
		lines[max][0] == '/' && lines[max][1] == '*')
	    {
		max--;
		break;
	    }
	}

	if (max >= sizeof(lines))
	{
	    left_lines = slice_array(lines, clin, sizeof(lines));
	    return sizeof(lines);
	}
	str = implode(slice_array(lines, flin, max), " ");
    }
    else
    {
	str = lines[flin];
	max = flin;
    }

    if (sscanf(str, "%s(%s)%s{%s", a, b, c, d) == 4)
    {
	e = explode(a + " ", " "); /* type and name */
	a = e[sizeof(e) - 1]; 	   /* last one is the name */
	if (sizeof(e) > 1)
	    c = implode(slice_array(e, 0, sizeof(e) - 2), " ");
	else
	    c = "untyped";

	/* Add an entry to the accumulated functions on the form:
	    ({ type, name, args, startlin in file, comment text, srcfile })
	*/

	acc_func += ({ ({ c, a, b, start + clin, cmt, src }) });
    } else
	max--;

    return max;
}    


/*
 * Description: Handle a batch of accumulated functions.
 *		
 * Arguments:  arr: All the accumulated functions. Each on the form:
 *              ({ type, name, args, startlin in file, comment text, srcfile })
 *             mainpath: The path to the dir where the documentation should be 
 *                       put.
 */
void
doc_found(mixed *arr, string mainpath)
{
    object player;
    string *files;
    int il;
    float pause;

    /*
     * Is there #included files that we should scan for functions?
     * If so, do that first.
     */
    if (sizeof(files_left))
    {
	set_alarm(1.0, 0.0, &accumul_funcs(files_left[0], doc_found, 0, mainpath));
	files_left = slice_array(files_left, 1, sizeof(files_left));
	left_lines = ({});
	
	return;
    }


    il = 0; pause = 1.0;
    while (il < sizeof(arr))
    {
	set_alarm(pause, 0.0, &doc_create(slice_array(arr, il, il + (WRITE_CHUNK-1)), mainpath,
		    geteuid(this_object())));
	il += WRITE_CHUNK;
	pause+=1.0;
    }
    
    for (il = 0, files = ({}); il < sizeof(arr); il++)
	files += ({ arr[il][1] });

    set_alarm(pause + 5.0, 0.0, &doc_clean_obsolete(files, mainpath, geteuid(this_object())));

    for (il = 0; il < sizeof(who_told_us); il++)
    {
	player = find_player(who_told_us[il]);
	if (player)
	    tell_object(player, 
			break_string("Docscribe tells you: " +
				     "Documentation of: " +
				     mainobfile + ", " + sizeof(arr) +
				     " documentation files will " +
				     "be created under: " + 
				     mainpath, 76) + "\n");
    }
    seteuid(0);
    acc_func = 0;
    order_stack = order_stack[1..sizeof(order_stack)];
}

/*
 * Make necessary subdirs upon creation of documentation path
 */
int doc_makepath(string path)
{
    string *split,file;
    int il;

    split = explode(path + "/", "/");
    
    for (file = "", il = 1; il < sizeof(split); il++)
    {
	file += "/" + split[il];
	if (file_size(file) == -1)
	    mkdir(file);
	else if (file_size(file) > 0)
	{
	    return 0;
	}
    }
    return 1;
}

/*
 * Description: Create a batch of documentation files
 * Arguments:  argarr
 *		[0]: All the accumulated functions. Each on the form:
 *              ({ type, name, args, startlin in file, comment text, srcfile })
 *		  
 *              [1]: The path to the dir where the documentation should be 
 *                   put.
 *              [2]: euid 
 */
void
doc_create(mixed *arr, string path, string new_euid)
{
    string file, data, *lines, cmt, euid;
    object player;
    mixed *calls;
    int il;

    euid = geteuid(this_object());
    set_alarm(1.0, 0.0, &reset_euid(euid));
    seteuid(new_euid);

    if (file_size(path) != -2)
    {
	if (!doc_makepath(path))    /* Could not create path */
	{
	    for (il = 0; il < sizeof(who_told_us); il++)
	    {
		player = find_player(who_told_us[il]);
		if (player)
		    tell_object(player, "Docscribe tells you: " +
				"I can't use the path: " + path + 
				" using the euid: " + geteuid(this_object()) +
				"\n");
	    }
	    return;
	}
    }

    path += "/";

    for (il = 0; il < sizeof(arr); il++)
    {
	file = path + arr[il][1];

	if (file_size(file) > 0)
	{
	    data = read_file(file);
	    lines = explode(data, "\n");
	    if ((sizeof(lines) > 2) && extract(lines[2], 0, 1) != "/*")
		cmt = implode(slice_array(lines, 2, sizeof(lines)), "\n");
	    else
		cmt = arr[il][4];
	    rm(file);
	}
	else
	    cmt = arr[il][4];

	write_file(file,arr[il][3] + ":" + arr[il][5] +
		   " (" + mainobfile + ")\n(" + arr[il][0] + ") " +
		   arr[il][1] +  "(" + arr[il][2] + ")\n" + cmt + "\n");
    }

    calls = get_all_alarms();
    for (il=0 ; il<sizeof(calls) ; il++)
	if (calls[il][1] == "reset_euid")
	    remove_alarm(calls[il][0]);

    seteuid(euid);
}

/*
 * Function name:	doc_clean_obsolete
 * Description: 	Clean up Create a batch of documentation files
 * Arguments:  		argarr
 *			  [0]: All the accumulated functions.
 *              	     ({ fun1, fun2, fun3 .... funN })
 *		  
 *              	  [1]: The path to the dir where the
 *			       documentation has been put.
 *              	  [2]: euid 
 */
void
doc_clean_obsolete(string *functions, string path, string new_euid)
{
    string *mfiles, *dfiles, euid, msg;
    mixed *calls;
    int il;
    object player;

    euid = geteuid(this_object());
    set_alarm(1.0, 0.0, &reset_euid(euid));
    seteuid(new_euid);

    dfiles = get_dir(path + "/*");

    if (pointerp(mfiles) && pointerp(dfiles))
    {
	/* Get the obsolete files in dfiles
	 */
	dfiles -= mfiles;
	if (sizeof(dfiles))
	{
	    if (file_size(path + "/.obsolete") != -2)
		mkdir(path + "/.obsolete");

	    for (il = 0; il < sizeof(dfiles); il++)
	    {
		rename(path + "/" + dfiles[il], 
		       path + "/.obsolete/" + dfiles[il]);
	    }
	}
    }

    for (il = 0; il < sizeof(who_told_us); il++)
    {
	player = find_player(who_told_us[il]);
	if (player)
	    tell_object(player, "Docscribe tells you: Done!\n"); 
    }

    calls = get_all_alarms();
    for (il=0 ; il<sizeof(calls) ; il++)
	if (calls[il][1] == "reset_euid")
	    remove_alarm(calls[il][0]);

    seteuid(euid);

    set_alarm(1.0, 0.0, doc_next);
}

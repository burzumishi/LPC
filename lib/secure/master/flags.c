/*
 * /secure/admin/flags.c
 *
 * This file maintains the quest-flag log.
 */

#pragma strict_types

#define NONAME "LOGON"		/* A name never used by a wiz */
#define MAXBITS 500		/* Max number of quest bits */

string          Wizzes;		/* All the wizards in a string */

static int FreeBit(int bit);
static varargs int authorization(object pobj, int mintype);
static string WizName(int bit);
static int list_flags(string who);
static int NextFreeBit();
static int AcquireBit(int bit, string wiz);
int query_wiz_level(string name); /* from fob */

/*
 * Function name: find_flag_owner
 * Description:   Return the owner of a specified flag.
 */
string 
find_flag_owner(int nr)
{
    return WizName(nr);
}

/*
  Questflag logger, Ver 1.1    JnA: 901221
  Modified	    Ver 1.2	RW: 910103

  This is the object logging the owner of the bits in the bitfields in
  player.c

  Methods:

  int NextFreeBit()
       // Returns the first free bit in the field not used by any Wiz
       // If return negative no free bits available

  int AcquireBit(bitnum,wizname)
       // Marks a bit as taken by the the wiz
       // Return 1 if that is ok

  int FreeBit(bitnum)
       // Marks a bit as free
       // Return 1 if that is ok

  string WizName(bitnum)
       // Returns the name of the owner of the bit

*/

static int 
find_flag()
{
    write("First free flag: " + NextFreeBit() + "\n");
    return 1;
}

static int 
allocate_flag(string arg)
{

    int flag;
    string name, *list;

    if (!arg) {
	notify_fail("Syntax: flag allocate # wizname\n");
	return 0;
    }

    list = explode(arg, " ");
    if ((sizeof(list) != 2) || (sscanf(list[0], "%d", flag) != 1)) {
	notify_fail("Syntax: flag allocate # wizname\n");
	return 0;
    }

    if (!query_wiz_level(list[1]))
    {
	notify_fail("There is no Wizard with that name.\n");
	return 0;
    }

    if (!AcquireBit(flag, list[1]))
	write("Couldn't allocate flag #:" + flag + ".\n");
    
    return 1;
}

static int 
free_flag(string flag)
{
    string          name;
    int             the_flag;

    if ((!flag) || (sscanf(flag, "%d", the_flag) != 1)) {
	notify_fail("Syntax: flag free #\n");
	return 0;
    }

    if (!FreeBit(the_flag))
	write("Flag #" + the_flag + " not previously allocated.\n");

    return 1;
}

static int 
owner_flag(string flag)
{
    int             the_flag;

    if ((!flag) || (sscanf(flag, "%d", the_flag) != 1)) {
	notify_fail("Syntax: flag owner #\n");
	return 0;
    }
    
    write("Owner of flag #" + the_flag + " : " + WizName(the_flag) + "\n");
    return 1;
}

static int 
list_flags(string who)
{
    int nr, i;
    string *wizarr;

    if (!who) {
	notify_fail("Syntax: flag list wizardname\n");
	return 0;
    }

    if (!Wizzes) {
	notify_fail("No flags are allocated.\n");
	return 0;
    }

    wizarr = explode(Wizzes, " ");
    nr = 0;
    for (i = 0; i < sizeof(wizarr); i++) {
	if (wizarr[i] == who) {
	    nr++;
	    write("Flag #" + i + "\n");
	}
    }

    if (!nr)
	write(capitalize(who) + " has no flags allocated.\n");
    else
	write("Total number of flags allocated to " + capitalize(who) + ": " + nr + "\n");
    return 1;
}

static string 
WizName(int bit)
{
    string *wizarr;

    if (!Wizzes)
	return 0;
    wizarr = explode(Wizzes, " ");
    if (bit >= sizeof(wizarr))
	return 0;
    if (bit < 0)
	return 0;
    if (wizarr[bit] == NONAME)
	return 0;

    return wizarr[bit];
}

static int 
NextFreeBit()
{
    int cnt;
    string *wizarr;

    if (!Wizzes)
	return 0;

    wizarr = explode(Wizzes, " ");

    for (cnt = 0; cnt < sizeof(wizarr); cnt++)
	if (wizarr[cnt] == NONAME)
	    return cnt;

    if (sizeof(wizarr) > MAXBITS)
	return -1;

    return sizeof(wizarr);
}

static int 
AcquireBit(int bit, string wiz)
{
    int cnt;
    string *wizarr;

    if (!authorization(previous_object()))
	return 0;
    if (WizName(bit))
	return 0;

    if (!Wizzes) {
	wizarr = allocate(bit + 1);
	for (cnt = 0; cnt < bit + 1; cnt++)
	    wizarr[cnt] = NONAME;
    } else
	wizarr = explode(Wizzes, " ");

    /* Realloc */
    if (bit >= sizeof(wizarr))
    {
	Wizzes = implode(wizarr, " ");
	cnt = bit - sizeof(wizarr) + 1;
	while (cnt)
	{
	    Wizzes = Wizzes + " " + NONAME;
	    cnt--;
	}

	wizarr = explode(Wizzes + " ", " ");
    }

    if (wizarr[bit] == NONAME)
	wizarr[bit] = wiz;

    Wizzes = implode(wizarr, " ");
    save_object(SAVEFILE);
    if (wizarr[bit] == wiz) {
	write("You have marked bit: " + bit + " for use by: " + wiz + "\n");
	return 1;
    } else
	return 0;
}

static int 
FreeBit(int bit)
{
    int cnt;
    string str, *wizarr;

    if (!authorization(previous_object()))
	return 0;

    if (!Wizzes)
	return 0;
    wizarr = explode(Wizzes, " ");
    if (bit >= sizeof(wizarr))
	return 0;
    str = wizarr[bit];
    wizarr[bit] = NONAME;
    Wizzes = implode(wizarr, " ");
    save_object(SAVEFILE);
    write("You have freed bit: " + bit + " it was used by: " + str + "\n");
    return 1;
}



/*
 * Function name: look_flag
 * Description:   Commands for looking at flags.
 */
int 
look_flag(string arg)
{
    int i;
    string *list, cmd, args;

    list = explode(arg, " ");
    cmd = list[0];
    if (sizeof(list) == 2)
	args = list[1];
    if (sizeof(list) > 2)
    {
	for (i = 0; i < sizeof(list) - 1; i++)
	{
	    list[i] = list[i + 1];
	}
	list[i] = "";
	args = implode(list, " ");
    }

    if (cmd == "list")
	return list_flags(args);

    if (cmd == "owner")
	return owner_flag(args);

    if (!authorization(previous_object()))
    {
	notify_fail("flag: Unknown subcommand.\n");
	return 0;
    }

    if (cmd == "first")
	return find_flag();

    if (cmd == "allocate")
	return allocate_flag(args);

    if (cmd == "free")
	return free_flag(args);

    notify_fail("flag: No such subcommand.\n");
    return 0;

}

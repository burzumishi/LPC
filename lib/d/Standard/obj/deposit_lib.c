/*
 * /d/Standard/obj/deposit_lib.c
 *
 * This is the deposit of the Gnomes of Standard. It is a bank that can save
 * money for players. To offer the service, just clone this object into your
 * bank room and everything will be taken care of.
 *
 * Coded by Tintin, 1992
 *
 * Version 2.0 by Mercade, July 1995
 */

#pragma no_clone
#pragma save_binary
#pragma strict_types

inherit "/std/object";

#define DEPOSIT_CC   "cc"
#define DEPOSIT_SC   "sc"
#define DEPOSIT_GC   "gc"
#define DEPOSIT_PC   "pc"
#define DEPOSIT_TIME "tm"
#define DEPOSIT_FEE  "fd"

#define DEPOSIT_INDICES ({ DEPOSIT_CC, DEPOSIT_SC, DEPOSIT_GC, DEPOSIT_PC })

#include <macros.h>
#include <money.h>
#include <stdproperties.h>

#undef _april_fools

/*
 * Global variables.
 */
static private string  terms;
static private string  usage;
static private int     fee;
static private string *money_types;
static private string  accounts;
static private string  coin_file;

/*
 * Prototypes
 */
int account(string str);
int deposit(string str);
int read(string str);
int withdraw(string str);
static mapping remove_fee(mapping acc);


/*
 * Function name: create_object
 * Description  : This function is called to create the strongroom.
 */
nomask void
create_object() 
{
    set_name("strongroom");
    add_name("deposit");
    add_name("hut");

    add_adj("gnome");
    add_adj("money");

    set_short("strongroom");
    set_long("It is a small fort-like hut. It has an opening protected " +
	"with steel bars and a large sign. Behind the opening there is " +
	"a small wizened gnome.\n");

    add_item( ({ "sign", "large sign" }),
	"A large sign with large, crooked letters at the top and a lot " +
	"of fine print at the bottom.\n");
    add_item( ({ "gnome", "small gnome", "wizened gnome",
	"small wizened gnome" }),
	"A small, greedy-looking gnome with a large nose.\n");
    add_item( ({ "bars", "bar", "steel bars", "steel bar" }),
    	"The steel bars look like they will not bulge even under great " +
    	"pressure.\n");
    add_item( ({ "opening" }),
	"It is the counter, protected by steel bars. Behind it you find a " +
	"small wizened gnome.\n");

    add_prop(OBJ_M_NO_GET, "It is not just a strongroom, it is a very heavy " +
	"strongroom as well. No way you can lift it.\n");

    this_object()->create_deposit();

    setuid();
    seteuid(getuid());
}

/*
 * Function name: init
 * Description  : This function is called to add the actions to the player
 *                when he enters the room the strongroom is in.
 */
void
init() 
{
    ::init();

    add_action(account,  "account");
    add_action(deposit,  "deposit");
    add_action(read,     "read");
    add_action(withdraw, "withdraw");
}

/*
 * Function name: enter_env
 * Description  : If we enter a room, let people know that we are here.
 * Arguments    : object dest - the destination of the strongroom.
 *                object old  - where we came from.
 */
void
enter_env(object dest, object old)
{
    dest->add_my_desc("There is a gnome money deposit here.\n",
	this_object());
}

/*
 * Function name: leave_env
 * Description  : The strongroom leaves a room, so remove our add-desc.
 * Arguments    : object old  - the room we leave.
 *                object dest - the destination of the strongroom.
 */
void
leave_env(object old, object dest)
{
    old->remove_my_desc(this_object());
}

/*
 * Function name: set_fee
 * Description  : Set the dayly fee in coins of the lowest denomination.
 * Arguments    : int coins - the fee.
 */
void
set_fee(int coins)
{
    fee = coins;
}

/*
 * Function name: set_terms
 * Description  : Set the terms of the deposit.
 * Arguments    : string text - the terms.
 */
void
set_terms(string text)
{
    terms = text;
}

/*
 * Function name: set_usage
 * Description  : Set the usage of the deposit.
 * Arguments    : string text - the usage.
 */
void
set_usage(string text)
{
    usage = text;
}

/*
 * Function name: set_accounts
 * Description  : Set the module that stores the accounts.
 * Arguments    : string file - the filename of the module.
 */
void
set_accounts(string file)
{
    accounts = file;
}

/*
 * Function name: set_money_types
 * Description  : Set the array with the names of the money types. The
 *                smallest denomination first.
 * Arguments    : string *types - the types of money.
 */
void
set_money_types(string *types)
{
    money_types = types;
}

/*
 * Function name: set_coin_file
 * Description  : Set the file that is used for coins.
 * Arguments    : string file - the filename.
 */
void
set_coin_file(string file)
{
    coin_file = file;
}

/*
 * Function name: read
 * Description  : Called when the player wants to read the text on the sign.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
int
read(string str)
{
    if ((str == "sign") ||
        (str == "large sign"))
    {
	write("The sign reads:\n\n" +
	    "GNoMeS oF GeNeSiS STRoNGRooM MoNey DePoSiT\n" +
	    "- aBSoLuTeLy SaFe\n" +
	    "- FRieNDLy SeRViCe\n" +
	    "- aVaiLaBLe eVeRyWHeRe\n" +
	    "- aLL iMPoRTaNT PLaCeS aNyWay\n" +
	    "- LoW FeeS\n" +
	    "- HoBBiTS SPeCiaL RaTeS\n\n" +
	    "After that there is a lot of fine print regulating the terms " +
	    "of deposit and giving instructions on using the GoG deposit.\n");

	return 1;
    }

    if ((str == "print") ||
    	(str == "fine print"))

    {
	write(terms + "\n" + usage);
	return 1;
    }

    if (str == "terms")
    {
        write(terms);
        return 1;
    }

    if ((str == "usage") || (str == "instructions"))
    {
        write(usage);
        return 1;
    }

    notify_fail("If you want to read, you should find something readable.\n");
    return 0;
}

/*
 * Function name: print_account
 * Description  : Print the contents of an account the the player:
 * Arguments    : mapping acc - the account to print.
 */
static void
print_account(mapping acc)
{
    write(sprintf("%7d %s coins\n", acc[DEPOSIT_PC], money_types[3]));
    write(sprintf("%7d %s coins\n", acc[DEPOSIT_GC], money_types[2]));
    write(sprintf("%7d %s coins\n", acc[DEPOSIT_SC], money_types[1]));
    write(sprintf("%7d %s coins\n", acc[DEPOSIT_CC], money_types[0]));

    if (acc[DEPOSIT_FEE])
    {
	write(sprintf("\n%7d %s coins due in fees\n", acc[DEPOSIT_FEE],
	    money_types[0]));
    }

    acc = remove_fee(acc);
    write("\n" + 
        capitalize(MONEY_TEXT_SPLIT(MONEY_MERGE(({ acc[DEPOSIT_CC],
                                                   acc[DEPOSIT_SC],
                                                   acc[DEPOSIT_GC],
					           acc[DEPOSIT_PC], })))) +
         "\n");
}

/*
 * Function name: fee_time
 * Description  : This function return the amount of days that the player
 *                owed fee for in total. Hobbits enjoy the special rate.
 * Arguments    : int - the time() the player has manipulated the account.
 * Returns      : int - the number of days extra due.
 */
static int
fee_time(int start_time)
{
    start_time = ((time() - start_time) / 86400) + 1; 
    start_time = ((start_time <= 7) ? start_time : 7) + (start_time - 7) / 7;
    
    if (start_time > 2)
    {
	return start_time;
    }
    if (this_player()->query_race_name() != "hobbit")
    {
	return start_time;
    }
    return 2;
}

#ifdef _april_fools
/*
 * Function name: april_fools
 * Description  : ACK! On April fools day, all accounts will be stolen!
 * Returns      : int - TRUE when it is April fools.
 */
int
april_fools()
{
    if (ctime(time())[4..9] != "Apr  1")
    {
        return 0;
    }

    write("As you approach the deposit, the gnomish clerk exclaims:\n");
    say("As " + QTNAME(this_player()) +
       " approaches the deposit, the gnomish clerk exclaims:\n");
    tell_room(environment(this_player()),
       "    OH!ItIsTerrible!WeHaveBeenROBBED!AllThoseBeautifulCoinsGONE!\n\n" +
       "The clerk cries out in agony and continues:\n" +
       "    ItHappenedLastNight!TheyWereHoodedAndMasked!AndTheyCarriedSwords!\n" +
       "    AndHeavyHammersToo!TheyFledThatWay!\n\n" +
       "The clerk points out in a hasty gesture and starts to sob silently again.\n");
    return 1;
}
#endif _april_fools

/*
 * Function name: deposit
 * Description  : This function is called when the player wants to make
 *                a deposit.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
int
deposit(string str)
{
    mapping acc;
    int     index;
    int     deposit_flag;
    string  user;
    string  coin_spec, except_spec;
    mixed   coin_arr, except_coin_arr;

#ifdef _april_fools
    if (april_fools())
    {
        return 1;
    }
#endif _april_fools

#ifndef TESTING
    if (this_player()->query_wiz_level())
    {
        notify_fail("There is no need for you to deposit money.\n");
        return 0;
    }
#endif

    if (!strlen(str))
    {
        notify_fail("You have to deposit a sum to open the account\n");
        return 0;
    }

    if (!parse_command(str, ({}), 
        "%s 'except' [for] %s", coin_spec, except_spec))
    {
        coin_spec = str;
        except_coin_arr = allocate(SIZEOF_MONEY_TYPES);
    }
    else
    {
        except_coin_arr = MONEY_PARSE(except_spec);

        if (!pointerp(except_coin_arr))
	{
            notify_fail("Except for what?\n");
            return 0;
	}
    }

    coin_arr = MONEY_PARSE_OB(coin_spec, this_player());

    if (!pointerp(coin_arr))
    {
        notify_fail("Deposit what?\n");
        return 0;
    }

    /* If you want to make some checks before the player deposits any money,
     * you should define the function no_deposit_possible() and if the
     * function returns true, this means that no deposit is possible. You
     * should print your own error message.
     */
    if (this_object()->no_deposit_possible())
    {
        return 1;
    }

    user = this_player()->query_real_name();
    acc = accounts->query_account(user);
    if (!mappingp(acc) || !m_sizeof(acc))
    {
        acc = ([ DEPOSIT_CC  : 0,
                 DEPOSIT_SC  : 0,
                 DEPOSIT_GC  : 0,
                 DEPOSIT_PC  : 0,
                 DEPOSIT_TIME: time(),
                 DEPOSIT_FEE : 0 ]);
    }

    if ((time() - acc[DEPOSIT_TIME]) > 300)
    {
        acc[DEPOSIT_FEE] += (fee_time(acc[DEPOSIT_TIME]) * fee);
        acc[DEPOSIT_TIME] = time();
    }

    for (index = 0; index < sizeof(coin_arr); index++)
    {
        if (except_coin_arr[index])
	{
            if (except_coin_arr[index] == -1) /* Except all <type> coins */
	    {
                /* We don't want to deposit any coins of this type */
                coin_arr[index] = 0;
	    }
            else /* Except <number> <type> coins */
	    {
                /* We don't want to deposit a specified number of coins of
                 * this type.
                 */
                coin_arr[index] -= except_coin_arr[index];
	    }
	}

        if (coin_arr[index] < 1)
	{
            continue;
	}

        if (MONEY_MOVE(coin_arr[index], MONEY_TYPES[index], this_player(), 0))
        {
            write("You don't have that many " + MONEY_TYPES[index] + 
                " coins.\n");
            continue;
        }

        deposit_flag = 1;
        acc[DEPOSIT_INDICES[index]] += coin_arr[index];
    }

    if (!deposit_flag)
    {
        write("No coins deposited.\n");
        return 1;
    }

    write("You deposited " + MONEY_TEXT(coin_arr) + ".\n");
    write("Your total account now amounts to:\n");
    print_account(acc);
    say(QCTNAME(this_player()) +
        " deposits some coins in the care of the Gnomes of Standard.\n");
    
    /* Save the account. */
    accounts->set_account(user, acc);
    
    return 1;
}

/*
 * Function name: account
 * Description  : When the player wants to know the amount on his account,
 *                this is the command to use.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
int
account(string str)
{
    mapping acc;

#ifdef _april_fools
    if (april_fools())
    {
        return 1;
    }
#endif _april_fools

    if (strlen(str))
    {
        notify_fail("Accounts what?\n");
        return 0;
    }

    acc = accounts->query_account(this_player()->query_real_name());

    if (!mappingp(acc) || !m_sizeof(acc))
    {
        notify_fail("The gnome says: Sorry, but you do not seem to have " +
            "an account with us.\n");
        return 0;
    }

    if ((time() - acc[DEPOSIT_TIME]) > 300)
    {
        acc[DEPOSIT_FEE] += (fee_time(acc[DEPOSIT_TIME]) * fee);
    }

    write("You have placed the following money in our trust:\n");
    print_account(acc);
    say(QCTNAME(this_player()) + " inquires about " +
        this_player()->query_possessive() +
        " account at the Gnomes of Standard.\n");
    return 1;
}

/*
 * Function name: remove_fee
 * Description  : This function removes the fee from the account.
 * Arguments    : mapping - the account to remove the fee from.
 * Returns      : mapping - the account minus the fee.
 */
static mapping
remove_fee(mapping acc)
{
    int *list;
    int  index;
    int  amount = acc[DEPOSIT_FEE];
    int  max = sizeof(money_types);

    list = ({ MONEY_VALUES[0] * acc[DEPOSIT_CC],
	      MONEY_VALUES[1] * acc[DEPOSIT_SC],
	      MONEY_VALUES[2] * acc[DEPOSIT_GC],
	      MONEY_VALUES[3] * acc[DEPOSIT_PC] });

    /* If the account is not big enough to pay the fee, clear the account. */
    if ((list[0] + list[1] + list[2] + list[3]) <= amount)
    {
    	return ([ DEPOSIT_PC : 0,
    		  DEPOSIT_GC : 0,
    		  DEPOSIT_SC : 0,
    		  DEPOSIT_CC : 0 ]);
    }

    for (index = 0; index < max; index++)
    {
        if (amount <= list[index])
        {
            list[index] -= amount;
            break;
        }
        else
        {
            amount -= list[index];
            list[index] = 0;
        }
    }

    amount = 0;
    for (index = max - 1; index >= 0; index--)
    {
        list[index] += amount;
        amount = list[index] % MONEY_VALUES[index];
        list[index] = list[index] / MONEY_VALUES[index];
    }
 
    return ([ DEPOSIT_PC  : list[3],
    	      DEPOSIT_GC  : list[2],
    	      DEPOSIT_SC  : list[1],
    	      DEPOSIT_CC  : list[0],
    	      DEPOSIT_FEE : 0,
    	      DEPOSIT_TIME: acc[DEPOSIT_TIME] ]);
}

/*
 * Function name: withdraw
 * Description  : This fucntion is called whenever the player wants to
 *                withdwar some money from his/her account.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
int
withdraw(string str)
{
    mapping acc;
    int     withdraw_flag;
    int     index;
    int     num_coins;
    string  user;
    string  coin_type;
    string  coin_spec, except_spec;
    object  money;
    mixed   coin_arr, except_coin_arr;

#ifdef _april_fools
    if (april_fools())
    {
        return 1;
    }
#endif _april_fools

    if (!strlen(str))
    {
        notify_fail("The gnome says: You have to state amount and coin " +
            "type to withdraw.\n");
        return 0;
    }

    if (!parse_command(str, ({}), 
        "%s 'except' [for] %s", coin_spec, except_spec))
    {
        coin_spec = str;
        except_coin_arr = allocate(SIZEOF_MONEY_TYPES);
    }
    else
    {
        except_coin_arr = MONEY_PARSE(except_spec);

        if (!pointerp(except_coin_arr))
	{
            notify_fail("Except for what?\n");
            return 0;
	}
    }

    coin_arr = MONEY_PARSE(coin_spec);

    if (!pointerp(coin_arr))
    {
        notify_fail("Bad coinage!\n");
        return 0;
    }

    /* If you want to make some checks before the player withdraws any money,
     * you should define the function no_withdrawal_possible() and if the
     * function returns true, this means that no withdrawal is possible.
     * You should print your own error message.
     */
    if (this_object()->no_withdrawal_possible())
    {
        return 1;
    }

    user = this_player()->query_real_name();
    acc = accounts->query_account(user);

    if (!mappingp(acc))
    {
        write("The gnome says: Sorry, but you do not seem to have an " +
            "account with us.\n");
        return 1;
    }
        
    if ((time() - acc[DEPOSIT_TIME]) > 300)
    {
        acc[DEPOSIT_FEE] += (fee_time(acc[DEPOSIT_TIME]) * fee);
        acc[DEPOSIT_TIME] = time();
    }

    acc = remove_fee(acc);

    write("The gnome does some calculations on an abacus and writes " +
        "something with red ink in a huge ledger.\n");

    if ((!acc[DEPOSIT_PC]) &&
        (!acc[DEPOSIT_GC]) &&
        (!acc[DEPOSIT_SC]) &&
        (!acc[DEPOSIT_CC]))
    {
        write("The gnome says: The money deposited in your accout does " +
            "not even cover our fee. I am forced to close your account " +
            "and register you as having a low reliability. We gnomes do " +
            "not like to deal with people who are not solvent!\n");

        accounts->remove_account(user);
        return 1;
    }

    for (index = 0; index < sizeof(coin_arr); index++)
    {
        if (except_coin_arr[index])
	{
            if (except_coin_arr[index] == -1) /* Except all <type> coins */
	    {
                /* We don't want to withdraw any coins of this type */
                coin_arr[index] = 0;
	    }
            else /* Except <number> <type> coins */
	    {
                /* We don't want to withdraw a specified number of coins of
                 * this type.
                 */
                coin_arr[index] -= except_coin_arr[index];
	    }
	}

        coin_type = MONEY_TYPES[index];

        if (acc[DEPOSIT_INDICES[index]] < coin_arr[index])
        {
            write("The gnome says: You don't have that many " + coin_type +
                " coins deposited. After fee deduction you would have had " +
                "the following money in your account:\n");
            print_account(acc);
            return 1;
        }
    }

    for (index = 0; index < sizeof(coin_arr); index++)
    {
        if (!coin_arr[index])
	{
            /* No coins of this type are wanted */
            continue;
	}

        if (coin_arr[index] < 0)
	{
            /* All coins of this type are wanted, except a few maybe */
            coin_arr[index] = max(0,
                coin_arr[index] + acc[DEPOSIT_INDICES[index]] + 1);
	}

        coin_type = MONEY_TYPES[index];

        if (objectp(money = present((coin_type + " coin"), this_player())))
        {
            num_coins = money->num_heap();
        }
        else
        {
            money = clone_object(coin_file);
            money->set_coin_type(coin_type);
            num_coins = 0;
        }

        if ((this_player()->query_prop(CONT_I_MAX_WEIGHT) -
            this_player()->query_prop(OBJ_I_WEIGHT)) <
            (coin_arr[index] * money->query_prop(HEAP_I_UNIT_WEIGHT)))
        {
            coin_arr[index] = ((this_player()->query_prop(CONT_I_MAX_WEIGHT) -
                this_player()->query_prop(OBJ_I_WEIGHT)) /
                money->query_prop(HEAP_I_UNIT_WEIGHT));
    
            if (coin_arr[index] <= 0)
            {
                write("You are completely encumbered and cannot handle even " +
                    "a single " + coin_type + " coin extra. None are " +
                    "withdrawn.\n");
                continue;
            }
    
            write("You cannot carry that many " + coin_type +
                " coins. Therefore only " + coin_arr[index] + " " + coin_type +
                " coin" + ((coin_arr[index] == 1) ? "" : "s") +
                " will be paid out to you.\n");
        }

        withdraw_flag = 1;    
        money->set_heap_size(num_coins + coin_arr[index]);
        money->move(this_player());

        acc[DEPOSIT_INDICES[index]] -= coin_arr[index];
    }

    if (!withdraw_flag)
    {
        write("No coins withdrawn.\n");
        return 1;
    }

    write("You are handed " + MONEY_TEXT(coin_arr) + ".\n");

    /* If there are coins left, save the account, else remove it. */
    if ((acc[DEPOSIT_PC]) ||
        (acc[DEPOSIT_GC]) ||
        (acc[DEPOSIT_SC]) ||
        (acc[DEPOSIT_CC]))
    {
        accounts->set_account(user, acc);

        write("After deduction of the fee and the withdrawal of the coins, " +
            "your account amounts to:\n");
        print_account(acc);
    }
    else
    {
        accounts->remove_account(user);

        write("The gnome says: Since you no longer have any coins in our " +
            "trust, I shall close your account.\n");
    }

    write("The gnome says: Thank you for putting your money in our trust.\n");
    say(QCTNAME(this_player()) + 
        " withdraws some coins from the Gnomes of Standard.\n");
    return 1;
}

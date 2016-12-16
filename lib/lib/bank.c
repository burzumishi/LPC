/*
 * A supporting file for banks
 *
 * Made by Nick
 */
#pragma strict_types

inherit "/lib/trade";

#include <stdproperties.h>
#include <macros.h>
#include <money.h>

static	int	testflag;	/* To indicate that a test is going on, no money given. */
static	int	bank_fee;	/* The bank fee */
static	int	money_num;	/* Num of money types. */

int change(string str);
int minimize(string str);
int test(string str);

/*
 * Function name: config_trade_data
 * Description:   Here we configure our own settings for the trade data
 */
void
config_trade_data()
{
    /* You have to set these two to the same number in order to get the
     * right calculations.
     */
    set_money_greed_buy(100 + bank_fee);
    set_money_greed_change(100 + bank_fee);

    /* A bank is rich. And if you can't give out the max with each type
     * of money you'll have to use another formula than I have below,
     * I think.
     */
	             /* Copper Silver Gold  Platinum  */
    set_money_give_out(({40000, 4000, 2000, 200}));
    set_money_give_max(40000);
    set_money_give_reduce(({0, 0, 0, 0}));

    money_num = sizeof(query_money_types());
}

/*
 * Function name: standard_bank_sign
 * Description:   returns a string with a sign message
 * Arguments:     none
 * Returns:       message string
 */
string
standard_bank_sign()
{
    return "You read:\n\n" + break_string(
	"Our bank provides you with a cheap way to relieve you of the burden "+
	"of heavy coins. " +
	"For each transaction we ask only a " + bank_fee + "% fee." +
	"", 70) + "\n" +
	"This is an example of what you can do:\n" +
	"    change copper and silver for gold\n" +
	"  This would make you change all your copper and silver coins\n" +
	"  into as much gold as possible.\n" +
	"    test change copper and silver for gold\n" +
	"  This would show you the outcome of the command behind\n" +
	"  the 'test' word.\n" +
	"    change 1 platinum for copper\n" +
	"  This would change 1 platinum coin to copper coins.\n" +
	"    change platinum for 100 silver\n" +
        "  This would change platinum coins to 100 silver coins + " +
	"the change\n" +
	"  that would be left from the platinum coins needed after " +
	"100 silver\n" +
	"  coins has been given.\n" +
	"    minimize coins\n" +
	"  This changes all coins into the most valuable types possible.\n" +
	"\n";
}

/*
 * Function name: bank_init
 * Description:   Add commands to someone who enters the room
 */
void
bank_init()
{
    add_action(change, "change");
    add_action(change, "exchange");
    add_action(minimize, "minimize");
    add_action(test, "test");
}

/*
 * Function name: set_bank_fee
 * Description:   Set the fee in % we take for our services
 * Argument:	  fee - The fee
 */
void set_bank_fee(int fee) { bank_fee = fee; }

/*
 * Function name: query_bank_fee
 * Description:   Query the fee we take
 * Returns:	  The fee
 */
int query_bank_fee() { return bank_fee; }

/*
 * Function name: bank_hook_pay
 * Description:   Change this function if you want a pay message of your own
 * Argument:	  text - The text describing what coins we payed
 */
void
bank_hook_pay(string text)
{
    write("You pay " + text + ".\n");
}

/*
 * Function name: bank_hook_change
 * Description:   This function is called when a change text is supposed to
 *		  be written. Redefine this if you wish.
 * Arguments:     text - The text describing the change
 */
void
bank_hook_change(string text)
{
    write("You get " + text + " back.\n");
}

/*
 * Function name: bank_hook_other_see
 * Description:   This function writes what the other players sees.
 *		  Redefine it if you want own messages.
 * Arguments:	  test - If a test is going on
 */
void
bank_hook_other_see(int testflag)
{
    if (!testflag)
	say(QCTNAME(this_player()) + " changes some money.\n");
    else
	say(QCTNAME(this_player()) + " seems to calculate something.\n");
}

/*
 * Function name: bank_hook_already_minimized
 * Description:   Write this when your coins already are minimized
 */
void
bank_hook_already_minimized()
{
    write("Your coins are already minimized.\n");
}

/*
 * Function name: bank_hook_no_idea
 * Description:   When there is no idea to minimize since it will all be
 *   		  eaten up by the tax
 */
void
bank_hook_no_idea()
{
    write("There is no idea for you to minimize your coins now.\n");
}

/*
 * Function name: bank_hook_minimized
 * Description:   Player minimized his coins
 */
void
bank_hook_minimized(string pay_text, string got_text)
{
    write("Your coins are now minimized.\n");
    say(QCTNAME(this_player()) + " minimized " +
	this_player()->query_possessive() + " coins.\n");
}

/*
 * Function name: valid_type
 * Description:   Find if a str is holding a valid type of money
 * Arguments:     str - The string describing the types
 * Returns:       The array number of the 'lowest' type of money identified
 */
int
valid_type(string str)
{
    int i, j, *tmp_arr;
    string *m_names;

    m_names = explode(str, " ");
    tmp_arr = allocate(money_num);

    for (i = 0; i < sizeof(m_names); i++)
        for (j = 0; j < money_num; j++)
            if (m_names[i] == query_money_types()[j])
                tmp_arr[j] = 1;

    for (i = 0; i < money_num; i++)
	if (tmp_arr[i] == 1)
	    return i;

    return -1;		/* This should not happen */
}

/*
 * Function name: change
 * Description:   Perform a change of money in the player
 * Arguments:     str - A string describing what to change into what
 */
int
change(string str)
{
    string str1, str2, dummy, change;
    int price, i, j, *arr, *hold_arr, *change_arr, number, greed;

    greed = query_money_greed_buy();

    if (!str)
    {
	notify_fail(capitalize(query_verb()) + " what?\n");
	return 0;
    }

    if (!parse_command(str, ({}), "%s 'to' / 'for' %s", str1, str2))
    {
	notify_fail("You have to define what you want to change and " +
		"to what it will be.\n");
	return 0;
    }

    notify_fail("You can only change coins you hold.\n");

    /* First find out how many coins player maximum can change to
     * Arguments: price = 0, changer = this_player(), str1 = what changer
     * wants to change, 1 = this is a test, 0 = a nil object (we),
     * str2 = how changer wants the change 
     *
     * These settings returns an array of what the changer wants to
     * change and how much that would be in the change the changer has
     * chosen. Then we can calculate the exact amount to change.
     */
    if (sizeof(arr = pay(0, this_player(), str1, 1, 0, str2)) == 1)
	return 0;

    hold_arr = exclude_array(arr, money_num, 2 * money_num - 1);
    change_arr = exclude_array(arr, 0, money_num - 1);

    if ((i = valid_type(str1)) >= 0)
    {
	if (sscanf(str1, "%d %s", number, dummy) == 2)
        {
	    if (number > hold_arr[i])
	    {
		notify_fail("You can't spend more coins than you have.\n");
		return 0;
	    }
	    hold_arr[i] = number;
	    change_arr = calc_change(0, hold_arr, str2);
	    notify_fail("Don't you think that you ought to give a number higher " +
	 	"than 0?\n");
	}

	if (hold_arr[i] <= 0) 
	    return 0; 

	if ((i = valid_type(str2)) >= 0)
	{
	/* We need the price in order to take money from the player */
	    if (sscanf(str2, "%d %s", number, dummy) == 2)
            {
	        if (number > change_arr[i])
	        {
	   	    notify_fail("You cannot afford to change to that amount.\n");
		    return 0;
	        }
	        change_arr[i] = number;
		notify_fail("Don't you think you ought to give a number higher " +
			"than 0?\n");
	        if (change_arr[i] <= 0)
		    return 0;
	    }
	    if ((change_arr[i] * query_money_values()[i] * greed / 100) >
				query_money_give_max())
		change_arr[i] = (query_money_give_max() * 100 / greed) /
				query_money_values()[i];

	    for (j = i - 1; j >= 0; j--)
	        change_arr[j] = 0;

    	    price = money_merge(change_arr);
	    if (price < 1)
	    {
		notify_fail("Oooops, you couldn't afford that change with that " +
			"little money.\n");
		return 0;
	    }
        }
        else
        {
	    notify_fail("You have to choose a valid type to change to.\n");
            return 0;
        }
    }
    else
    {
	notify_fail("You have to choose a valid type of money to change.\n");
	return 0;
    }
  
    /* Here is the actual change taking place */
    if (!(arr = pay(price, this_player(), str1, testflag, 0, str2)))
	return 0;
    give(price, this_player(), str2, testflag, 0, 0);

    /* Now, in the pay() the player could have been given some change back,
     * add it.
     */
    for (i = 0; i < money_num; i++)
	change_arr[i] = change_arr[i] + arr[i + money_num];

    /*
     * Some hooks for people who wants different messages.
     */
    bank_hook_pay(text(arr[0 .. money_num - 1]));
    if (change = text(change_arr))
	bank_hook_change(change);
    bank_hook_other_see(testflag);

    return 1;
}

/*
 * Function name: minimize
 * Description:   changes all coins into the most expensive type, minus a fee
 *		  BUGS - This function still not supports unstandard coins
 * Arguments:     str: predicate
 * Returns:       success
 */
int minimize(string str)
{
    int *money_arr, *money_arr2, value, i, new_sum, total_sum;
    string change;

    if (str != "coins")
    {
        notify_fail("Minimize what? Your coins?\n");
        return 0;
    }

    money_arr = what_coins(this_player());

    money_arr2 = split_values(total_sum = money_merge(money_arr));
    for (i = 0; i < money_num; i++)
    {
	money_arr2[i] -= money_arr[i];
	if (money_arr2[i] < 0)
	    money_arr2[i] = 0;
    }
    value = money_merge(money_arr2);
    if (!value)
    {
	bank_hook_already_minimized();
	return 1;
    }
    new_sum = total_sum - bank_fee * value / 100;
    money_arr2 = split_values(new_sum);
    for (i = 0; i < money_num; i++)
    {
	money_arr2[i] -= money_arr[i];
	if (money_arr2[i] < 0)
	    money_arr2[i] = 0;
    }
    value = money_merge(money_arr2);
    if (!value)
    {
	bank_hook_no_idea();
	return 1;
    }

    MONEY_ADD(this_player(), new_sum);
    MONEY_ADD(this_player(), -total_sum);

    money_arr2 = what_coins(this_player());

    bank_hook_minimized(text(money_arr),text(money_arr2));

    return 1;
}

/*
 * Function name: test
 * Description:   To allow the player to see what would happen with a change 
 *                command about to be given
 * Arguments:     str - The string holding the change command
 */
int
test(string str)
{
    int i;
    string str1;
    
    notify_fail("Test what?\n");
    if (!str)
	return 0;
    
    write("This would be the result of that change command:\n");

    if (parse_command(str, ({}), "'change' / 'exchange' %s", str1))
    {
	testflag = 1;
	i = change(str1);
	testflag = 0;
	return i;
    }
}

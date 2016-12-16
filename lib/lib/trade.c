/* trade.c

   This is a standard trade object with some nice funktions.
   Made by Nick, the ever smiling.

   If you want to have a shop or a pub or anything that handles
   money you can include this object and configure it to your
   liking, if you don't settle with the default that is ofcourse.

   Keep on smiling, /Nick

   Typical use:

      inherit "trade.c";

      create_room() {
        config_default_trade();
        .... 
        rest of the create
      }

      then call:

   pay(int amount, object who_pays, string str, int test,
       object who_gets, string str2, int silent)
   give(int amount, object who_gets, string str, int test,
       object who_pays, int silent)

      Both functions return an array doubble size of money types
      with money data if they succeed. First part is money connected 
      to the buying/selling object and last part to this_object.

      If test then no money is being switch, you just get the array

      If there is a str it will be used to see what types of money
      the object wants to pay with, or what types the object wants.
      If str2 is used in pay() it will be used to determin how the
      buyer wants the change, if any.

      If there isn't the first object, this_player() is presumed.

      If the last object is not set, the money will be destructed if pay
      or created out of nowhere on give. If you want to give the money
      from an object, you better be sure the money is present in it.

      If silent flag is set there will be no writing from money moving
      routine if there is an error, i.e. the player can't hold all the
      coins the player is getting and drops some.

      You can call text() with the array from pay/give to get a
      configured text of what money have been in the transaction.
      Observe, text wants just one part of that array at a time.

   User functions in this object
   -----------------------------

   Arguments:     s = string, i=integer, o = object, a = array

   set_money_give_max(), set_money_greed_buy(), _sell() and _change() all
   can be set to VBFC's.

   All set functions have corresponding query function.

   All arrays should be in form: ({ copper, silver, gold, platinum })

   config_default_trade()     Will set up the default configuration for
	or                    trade. If you don't want to set all the
   default_config_trade()     set functions by yourself, call this one.
        both works :)         Se below on notes for about default trade.

   set_money_give_max(i)      Set max value of money the shop gives out.

   set_money_give_out(ai)     Set max money the shop gives of each type.

   set_money_give_reduce(ai)  Set how much on each type to reduce, setting
                              this for silver will give you a wider range of
                              copper coins given out that 0-11. if you reduce
                              silver with one, copper will be in 0-23 format.
                              If in the calculous it has arrived you would
                              get 10 silver coins and reduce is set to 5 you
                              would get 5 silver coins instead and the rest of
                              worth in copper coins.

   set_money_accept(ai)       Set what money we accept.

   set_money_greed_buy(i)     Set greed when person pays for a price, in %.
                              100 means the price in the pay() call will not
                              be changed. >100 will make the player pay more.

   set_money_greed_sell(i)    Set greed when person sells something, in %
                              100 means the price in the give() call will not
                              be changed. >100 will make the player get less.

   set_money_greed_change(i)  Set greed when the person is about to get change
                              100 means the change the person gets will not
                              be changed. >100 will make the player get less
                              change.

   set_money_values(ai)       Set values of each coin type (in cc ofcourse). Do
                              not change this if you haven't got good reasons.

   set_money_types(as)        Set the text strings for each type recognize. Do
			      not change this if you haven't got VERY good
			      reasons.

   pay(i, o, s, i, o, i)      Documented above. Only first i (price) is 
			      necessary.

   give(i, o, s, i, o, i)     Documented above. Only first i (price) is 
			      necessary.


   Play around with them, but not too much.

   Default trade is as follows:

     Max money given out in cc is 1000
     All types of greed is set to 100, which means the price in the calls will 
       be used.
     Types and values of the coins will be used as they are in /sys/money.h
     The trade object accepts all kinds of money
     The trade gives maximum out: 2 platinum, 20 gold, 400 silver and 10000 
       copper coins. Note that if you don't set the copper give out = to max 
       give out you might cut off the copper given out so buyer won't get the
       right money. You can ofcourse check yourself for this and take 
       apropriate action....
     The trade reduces the payout of coins with: 8 platinum, 8 gold, 2 silver
       and 0 copper coins. Note that if you reduce copper the buyer, or seller,
       will be given that amount short. One reason to reduce is that you will
       only pay out platinum coins if the seller hands you a very nice object
       with value > than your reduce factor on platinum, not just as quickly
       you get a value > the value of platinum.


   More functions to call are:

	money_merge(ai)        It merges the money in the array into a sum in
                               cc, it calculates with the values set by you.

	what_coins(o)          It returns an array of what coins the object
                               containes, needed if you use sepcial types.

	split_values(i)        It returns an array of the value splitted into
                               the different coin types.

	can_pay(i, o)	       Checks if the object can pay the price (integer
			       in cc) with the money the object carries.

	can_pay_arr(i, a)      Checks if the array can pay the price.

	want_to_pay(ai, s)     Reduces the array to the types specified in the
			       string

	we_accept(ai)	       Reduces the array to what coins we accept

	calc_change_str(ai ,s) Reduces an array to the types specified in the
			       string, all lesser types are intact.

	calc_change(i, ai, s)  Caluculates the change given back if one pays
			       the price (integer) with the array. The string
                               might tell us how the the change is wanted.

	reduce_coin(i, ai)     Reduces the array according to the price from 
			       the top.

	calc_coins(i, ai, s)   Reduces the array according to the price from
 			       bottom, and calls the calc_change() later with s.

	money_move(s, i, o, o) Moves money of type s and number i from first
			       object to the second object.

	text(ai)	       Returns a formated string describing the array
			       in words

	config_default_trade() Configurate the default trade settings.

	pay() and give()       Documented above.

        query_extra_money_holder(i, i, i) If there was an error when trying to
			       Move the money TO us from the player this
			       function is called with what type it was, the
			       amount and what error we got. It wants an object
			       in return, where to move the money instead.

*/
#pragma strict_types
#pragma save_binary

#include "/sys/stdproperties.h"
#include "/sys/macros.h"
#include "/sys/money.h"

#define MONEY_GIVE_MAX     MONEY_VALUES[3] 
  /* Good thing to set the copper max to this give max so no money will be lost
  */
  /*                          copper  silver  gold  platinum                */
#define MONEY_GIVE_OUT     ({ 10000,    400,    3,    0 }) 
#define MONEY_GIVE_REDUCE  ({     0,      2,    8,    8 })
#define MONEY_ACCEPT       ({     1,      1,    1,    1 })

/* How greedy are we? 100 will give original price. */
/* The higher the number the less to the liking of  */
/* our fellow mortal player. */
#define MONEY_GREED_BUY     100
#define MONEY_GREED_SELL    100
#define MONEY_GREED_CHANGE  100 

static 	mixed 	money_give_max;
static 	int 	*money_give_out;
static 	int 	*money_give_reduce;
static 	int 	*money_accept;
static 	mixed 	money_greed_buy;
static 	mixed 	money_greed_sell;
static 	mixed 	money_greed_change;
static 	int 	*money_values;
static 	string 	*money_types;

static int num_of_types; 	/* Holds the number of coin types.*/

/*
 * Function name: set_money_give_max
 * Description:   Set the maximum value this trade will hand out
 * Arguments:     m - The value (integer or mixed)
 */
void set_money_give_max(mixed m) { money_give_max = m; }

/*
 * Function name: set_money_give_out
 * Description:   Set the maximum munber of coins we hand out at once
 * Arguments:     a - The array of integers
 */
void set_money_give_out(int *a) { money_give_out = a; }

/*
 * Function name: set_money_give_reduce
 * Description:   Set the reduce number for each coin type
 * Arguments:     a - The array of integers
 * Returns:
 */
void set_money_give_reduce(int *a) { money_give_reduce = a; }

/*
 * Function name: set_money_accept
 * Description:   Set what coins we accept as payment
 * Arguments:     a - The array of flags (1 integers)
 */
void set_money_accept(int *a) { money_accept = a; }

/*
 * Function name: set_money_greed_buy
 * Description:   Set the greed when player buys something
 *                Can't be lower than 90
 * Arguments:     m - The greed in %, VBFC support
 */
void set_money_greed_buy(mixed m) { money_greed_buy = m; }

/*
 * Function name: set_money_greed_sell
 * Description:   Set the greed when player sells something to us
 *                Can't be lower than 90
 * Arguments:     m - The greed in %, VBFC support
 */
void set_money_greed_sell(mixed m) { money_greed_sell = m; }

/*
 * Function name: set_money_greed_change
 * Description:   Set the greed when player shuold get change back
 *                Can't be lower than 90
 * Arguments:     m - The greed in %, VBFC support
 */
void set_money_greed_change(mixed m) { money_greed_change = m; }

/*
 * Function name: set_money_values
 * Description:   Set how valuable each money type is
 * Arguments:     a - Array of values (integer) (values in cc)
 */
void set_money_values(int *a) { money_values = a; }

/*
 * Function name: set_money_types
 * Description:   Set what types of money we trade with
 * Arguments:     a - The array of types (string)
 */
void set_money_types(string *a)
{
    money_types = a;
    num_of_types = sizeof(a);
}

int query_money_give_max()
{
    return this_object()->check_call(money_give_max);
}

int query_money_greed_buy()
{
    int val;
    val = this_object()->check_call(money_greed_buy);

    return (val < 90 ? 90 : val);
}

int query_money_greed_sell()
{
    int val;
    
    val = this_object()->check_call(money_greed_sell);
    return (val < 90 ? 90 : val);
}

int query_money_greed_change()
{
    int val;
    
    val = this_object()->check_call(money_greed_change);
    return (val < 90 ? 90 : val);
}

string *query_money_types() { return money_types; }
int *query_money_give_out() { return money_give_out; }
int *query_money_give_reduce() { return money_give_reduce; }
int *query_money_accept() { return money_accept; }
int *query_money_values() { return money_values; }

/*
 * Function name: stat_trade
 * Description:   Get some statistics on the trade settings
 * Returns:       The statistics as a string
 */
string
stat_trade()
{
    string str;
    int i;

    str = sprintf("\n%-10s %6s %8s %|7s %9s\n", "Type", "Value", "Accept",
                "Give", "Reduce");
    str += sprintf("%-10s %6s %8s %|7s %9s\n", "====", "=====", "======",
                "====", "======");
    for (i = 0; i < num_of_types; i++)
        str += sprintf("%-10s %5d  %|8d %7d %|9d\n", money_types[i],
                money_values[i], money_accept[i], money_give_out[i],
                money_give_reduce[i]);

    str += "\ngive max: " + query_money_give_max() + "   greed buy: " +
        query_money_greed_buy() + "   greed sell: " +
        query_money_greed_sell() + "   greed change: " +
        query_money_greed_change() + "\n";

    return str;
}

/*
 * Functione name: query_extra_money_holder
 * Description:    If the money from the player couldn't be moved to
 *                 the object specified move it to the object returned
 *                 from here instead. Redefine this function if you have
 *                 a trader that might have to drop some coins and you
 *                 you want to keep track of everything.
 * Arguments:      i - number of the money type we tried to move
 *                 sum - the amount we tried to move
 *                 error - the error code we got when we tried
 * Returns:        nothing - destruct the money
 */
varargs mixed
query_extra_money_holder(int type, int sum, int error) { return ; }

/*
 * Function name: money_merge
 * Description:   Calculates the worth of an array of coins in cc.
 * Arguments:     arr - the array to calculate
 * Returns:       Sum in cc.
 */
int
money_merge(int *arr)
{
    int v, i;
    
    if (sizeof(arr) != num_of_types)
	return 0;
    
    for (v = 0, i = 0; i < num_of_types; i++)
	v += arr[i] * money_values[i];

    return v;
}

/*
 * Function name: what_coins
 * Description:   Finds out what of the cointypes a certain object contains.
 *                Works even if you have special types, I hope :)
 * Argument:      ob: The object in which to search for coins
 * Returns:       Array: ( num copper, num silver, num gold, num platinum )
 *                or the way you defined the types.....
 */
int *
what_coins(object ob)
{
    object cn;
    int i, *nums;
    string *ctypes;

    ctypes = money_types;
    nums = allocate(num_of_types);

    for (i = 0; i < num_of_types; i++)
    {
        cn = present(ctypes[i] + " coin", ob);
        if (!cn)
        {
            nums[i] = 0;
            continue;
        }
        else
            nums[i] = cn->num_heap();
    }
    return nums;
}

/*
 * Function name: split_values
 * Description:   Splits a 'copper' value into the types you are using
 * Argument:      value - value in copper coins
 * Returns:       the array
 */
int *
split_values(int value)
{
    int *arr, i;

    arr = allocate(num_of_types);

    if (value > 0)
	for (i = num_of_types - 1; i>= 0; i--)
	{
	    arr[i] = value / money_values[i];
	    value %= money_values[i];
	}

    return arr;
}

/*
 * Function name: can_pay
 * Description:   check if ob can pay the price
 * Arguments:     price - the price to pay
 *                ob - the object who should pay
 * Returns:       1 - can pay, 0 - can't
 */
int
can_pay(int price, object ob)
{
    if (!(ob = this_player()))
	return 0;
    return price <= money_merge(what_coins(ob));
}

/*
 * Function name: can_pay_arr
 * Description:   check if the array is enough to pay the price
 * Arguments:     price - the price to pay
 *                arr - the array holding the coins
 * Returns:       1 - can pay, 0 - can't
 */
int
can_pay_arr(int price, int *arr)
{
    return price <= money_merge(arr);
}

/*
 * Function name: want_to_pay
 * Description:   Check the string and sort out what types of coins to pay with
 *                or better to work with
 * Arguments:     arr - the array holding the coins
 *                str - string describing the coins object wants to work with
 */
void
want_to_pay(int *arr, string str)
{
    string *m_names;
    int i, *tmp_arr, j;
    
    if (!str || (str == ""))
	return;
    
    tmp_arr = allocate(num_of_types);
    m_names = explode(str, " ");

    for (i = 0; i < sizeof(m_names); i++)
	for (j = 0; j < num_of_types; j++)
	    if (m_names[i] == money_types[j])
		tmp_arr[j] = 1;

    for (i = 0; i < num_of_types; i++)
	arr[i] *= tmp_arr[i];
}

/*
 * Function name: we_accept
 * Description:   Compare array with what money we accept
 * Arguments:     arr - the array holding the coins
 */
void
we_accept(int *arr)
{
    int i;
    for (i = 0; i < num_of_types; i++)
	if (!money_accept[i] && arr[i])
	{
	    notify_fail("No " + money_types[i] + " coins accepted.\n");
	    arr[i] = 0;
	}
}

/*
 * Function name: calc_change_str
 * Description:   Consider what coins the object wants to be payed in
 * Arguments:     arr - the array holding the coins
 *                str - the string describing what types the object wants
 * Returns:       The array depending on what object wants
 */
int *
calc_change_str(int *arr, string str)
{
    int i, j, k, *tmp_arr;
    string *m_names;
    
    m_names = explode(str, " ");
    tmp_arr = allocate(num_of_types);

    for (i = 0; i < sizeof(m_names); i++)
	for (j = 0; j < num_of_types; j++)
	    if (m_names[i] == money_types[j])
		for (k = j; k >= 0; k--)
		    tmp_arr[k] = 1;

    for (i = 0; i < num_of_types; i++)
	arr[i] = arr[i] * tmp_arr[i];

    return arr;
}

/*
 * Function name: calc_change
 * Description:   Check what change to give the object considered the price
 *                and what is about to be payed. If no array then it's a sale
 *                and the changed is based on the price alone.
 * Arguments:     price - the price in cc
 *                arr - the array describing what we already pays
 *                str - the string describing what types the object wants
 * Returns:       The array holding the coins the object should be given
 */
int *
calc_change(int price, int *arr, string str)
{
    int *c_arr, new_price, i, sum, *tmp_arr, max, old, tmp;

    c_arr = allocate(num_of_types);
    if (price < 0)
	return c_arr;

    if (arr) /* No greed on change if this isnt change */
        new_price = (money_merge(arr) - price) * 100 /
            query_money_greed_change();
    else
	new_price = price;

    if (str && (str != "")) /* If specified how to get change */
	tmp_arr = calc_change_str(money_give_out + ({ }), str);
    else
	tmp_arr = money_give_out;

    /* this is done once to reduce effect if random give max. */  
    max = this_object()->check_call(money_give_max);
    if (new_price > 0)
	for (i = num_of_types - 1; i >= 0; i--)
	    if (new_price >= money_values[i] && tmp_arr[i])
	    {
		sum = new_price / money_values[i];
    /* This is to allow only high prices to pay platinum */
		sum -= money_give_reduce[i];
		if (sum > tmp_arr[i])
		    sum = tmp_arr[i];
		if (sum > 0)
		{
		    c_arr[i] = sum;
                    if (money_merge(c_arr) > max)
    /* Don't give more than the max */
                        c_arr[i] = (max - old) / money_values[i];
		    new_price -= (tmp = c_arr[i] * money_values[i]);
                    old += tmp;
                }
    }

    return c_arr;
}

/*
 * Function name: calc_coins
 * Description:   Reduce from bottom of array cosidering the price
 * Arguments:     price - the price to pay
 *                arr - the array holding the coins
 *                str - a string describing how object wants to get change
 * Returns:       The array object should pay, reduced
 */
int *
calc_coins(int price, int *arr, string str)
{
    int *new_arr, i, new_price, j, tmp;

    new_arr = allocate(num_of_types);
    new_price = price; /* Keep orignial price intact */

    for (i = 0; i < num_of_types; i++)
    { 
	new_arr[i] = arr[i];
	
	if (new_price <= (tmp = new_arr[i] * money_values[i]))
	{
	    new_arr[i] = new_price / money_values[i];
	    if (new_price > new_arr[i] * money_values[i])
		new_arr[i] += 1;
	    break;
	} else
	    new_price -= tmp;;
    }

    new_price = price;
    for (i = num_of_types - 1; i >= 0; i--)
    {
	arr[i] = new_arr[i];
	if (arr[i] * money_values[i] >= new_price)
   	{
	    for (j = i - 1; j >= 0; j--)
		arr[j] = 0;
	    arr[i] = new_price / money_values[i];
	    if (new_price > arr[i] * money_values[i])
	 	arr[i] += 1;
	    break;
	}

	new_price -= arr[i] * money_values[i];
    }

    return arr + calc_change(price, arr, str);
}

/*
 * Function name: money_move
 * Description:   Moves the money indicated from object indicated to object
 *                indicated
 * Arguments:     str - the type of coins to move
 *                num - the amount to move
 *                from - where to take money, if 0 money is created
 *                to - where to put the money, if 0 money is destroyed
 * Returns:       0 on sucess, else the error number from move() or -1
 */
int
money_move(string str, int num, object from, object to)
{
    object cn, cf;
    int max;

    if (!str || (num <= 0 )) /* Only positive and existing money */
	return -1;

    if (from)
    {
	cf = present(str + " coin", from);
	if (cf && function_exists("create_heap", cf) != "/std/coins")
	    cf = 0;
    }
    else
	cf = MONEY_MAKE(num, str);

    if (!cf || !(max = cf->num_heap()) || num > max )
	return -1;   /* Not enough money to move */

    if (to)
    {
	if (num < max)
	    cf->split_heap(num);
	return cf->move(to);
    }

    if (num < max)
	cf->set_heap_size(max-num);
    else
	cf->remove_object();

    return 0;
}

/*
 * Function name: change_money
 * Description:   Do the moves described in the array between us and the player
 * Arguments:     arr - the array holding the coins, twice as big as number of
 *		        money types, first part is from
 *                from - the one who pays and later gets change if any
 *                to - where to move the money and take change from, or 0
 *                silent - No error code written if set
 * Returns:       > 0 The error from a move() or
 *		  -1 when the money were not to be found in the specified
 *		     object. This also happens if the players has false money.
 *                These codes are multiplied with 10 if the error took place
 *                   when we tried to take money from the player. Also we break
 *                   the transaction if there was an error here.
 *
 *                -10 if the array was of wrong size.
 */
varargs int
change_money(int *arr, object from, object to, int silent)
{
    int i, error1, error2, move_error;

    move_error = 0;

    /* Not a valid array to change money. */
    if (sizeof(arr) != num_of_types * 2)
	return -10;

    for (i = 0; i < num_of_types; i++) /* Get money from the buyer. */
	if (arr[i] && (error1 = money_move(money_types[i],
		 arr[i], from, to)))
	{
	    error2 = money_move(money_types[i], arr[i],
		 from, query_extra_money_holder(i, arr[i], error1));
	    if (error2 != 0)
	    {
		if (!silent && (error2 < 0))
		    write("Error when taking money from the player,\n" +
		        "are there false money around?\n");
/*
   There shouldn't be any errors from move since the environment should be a 
   room, if however a move error has arrived we don't print any text.
   Nor is there a warning if we drop the money on the floor.
*/
		return error2 * 10;
	    }
	}

    for (i = num_of_types; i < 2 * num_of_types; i++)
	if (arr[i] && (error1 = money_move(money_types[i - num_of_types],
		 arr[i], to, from)) && from)
	{
	    error2 = money_move(money_types[i - num_of_types],
				 arr[i], to, environment(from));

            if ((move_error == 0) && (error1 != 0))
		move_error = error1;
	
	    if (!silent && error1)
	    {
		/* move error detected */
		write("You drop some of the change.\n");
		say(QCTNAME(from) + " drops some money.\n");
		silent = 1;
	    }
	    else  if (!silent && (error2 < 0))
	    {
		write("Error: No money found in the from object (us), "+
		    "no money given.\n");
		silent = 1;
	    }

            if (error2 < 0)
		move_error = error2;

	}
    /* There shouldn't be an error if noone has messed around with the coins */
    return move_error;
}

/*
 * Function name: text
 * Description:   Generates a text string from the array describing the coins
 * Arguments:     arr - the array holding the coins to describe
 */
mixed
text(int *arr)
{
    string *t_arr, coin_str;
    int i, j;
    
    if (sizeof(arr) < num_of_types)  /* Not a valid array. */
	return ;

    t_arr = ({ });

    for (i = num_of_types - 1; i >= 0; i--)
	if (arr[i] > 0)
	{
	    j += arr[i]; /* Total number of coins */
	    t_arr += ({ arr[i] + " " + money_types[i] });
	}

    coin_str = " coin";
    if (j > 1)
	coin_str += "s";

    j = sizeof(t_arr);

    if (j < 1)
	return;

    if (j == 1)
	return t_arr[0] + coin_str;
    else
	return implode(t_arr[0 .. j - 2], ", ") + " and " +
	       t_arr[j - 1] + coin_str;
}

/*
 * Function name: config_default_trade
 * Description:   Configure the default settings, must be done if not all
 *                set functions are going to be called from the outside
 */
void
config_default_trade()
{
    set_money_give_max(MONEY_GIVE_MAX);
    set_money_give_out(MONEY_GIVE_OUT);
    set_money_give_reduce(MONEY_GIVE_REDUCE);
    set_money_accept(MONEY_ACCEPT);
    set_money_greed_buy(MONEY_GREED_BUY);
    set_money_greed_sell(MONEY_GREED_SELL);
    set_money_greed_change(MONEY_GREED_CHANGE);
    set_money_values(MONEY_VALUES);
    set_money_types(MONEY_TYPES);
}

void
default_config_trade() { config_default_trade(); }

/*
 * Function name: pay
 * Description:   Let the first object pay the price to the second 
 *                object if not test
 * Arguments:     price - the price to pay
 *                ob - the object who should pay
 *                str - the string describing how first object wants to pay
 *                test - set if this is a test - no money being switched
 *                ob2 - the object who should get the money, 0 if none
 *                str2 - string describing how buyer wants the change
 *		  silent - No message will be written if fail to move money
 * Returns:       An array describing the object payed and the change it got
 *                Unless something went wrong, then it returns:
 *                  0 - Noone to pay the price found
 *		    1 - The specified money is not enough to pay the price
 * 		    2 - Choosed to pay with money object haven't got
 * 		    3 - Haven't got enough money
 *  		    4 - Can' pay the price after we compare what player has
 * 		        to what we accept, i.e. can't pay in acceptable coins
 *                Also, the last item in the array holds error code from the
 *                money moving routine.
 *                  If > 0 then the player couldn't hold the money it
 * 		    was getting and dropped it.
 *                  If = -1 then one couldn't move the money from the object
 *                  that was specified.
 *                Normally you don't have to bother at all about this one.
 */
varargs int *
pay(int price, object ob, string str, int test, object ob2, string str2,
    int silent)
{
    int *arr, i;

    if (!ob)
	ob = this_player();
    if (!ob)
    {
	notify_fail("No buyer recorded.\n");
	return 0;  /* No buyer */
    }

    /* How greedy are we? The original price should be the value of object. */
    price = price * query_money_greed_buy() / 100;
    arr = what_coins(ob);

    if (str) /* Reduce money array to what we wants to pay with */
	want_to_pay(arr, str);

    if (!can_pay_arr(price, arr))
    {
	if (str && (str != "") && money_merge(arr))
	{
	    notify_fail(break_string("You have to give me more coins to " +
		"choose from, " + text(arr) + " is not enough to pay the " +
		"price.\n", 70));
	    return ({ 1 });
	}
	else  if (str && (str != ""))
	{
	    notify_fail("I believe you chosed to pay with money you don't " +
		"carry.\n");
	    return ({ 2 });
	}
	notify_fail("You haven't got enough money to pay the price.\n");
	return ({ 3 });
    }

    we_accept(arr);   /* Reduce to what money we accept */

    /* Still can pay? If not we_accept() have set the notify_fail call */
    if (!can_pay_arr(price, arr))
	return ({ 4 });

    if ((price == 0) && test) 
        arr = arr + calc_change(price, arr, str2);
    else
        arr = calc_coins(price, arr, str2);

    if (!test)
	arr += ({ change_money(arr, ob, ob2, silent) });

    return arr;
}

/*
 * Function name: give
 * Description:   Let the first object give the price to the second object if
 *		  not test
 * Arguments:     price - the price to give
 *                ob - the object who should pay
 *                str - string describing what money buy wants to get
 *                test - set if this is a test - no money being switched
 *                ob2 - the object who should get the money, 0 if none
 *		  silent - No message will be written if fail to move money
 * Returns:       An array describing the object payed and the change it got
 */
varargs int *
give(int price, object ob, string str, int test, object ob2, int silent)
{
    int *arr, *tmp_arr, i;
    
    if (!ob && !test)
	ob = this_player();
    if (!ob && !test) {
	notify_fail("Nobody to give the money.\n");
	return 0;
    }

    tmp_arr = allocate(num_of_types);
    price = price * 100 / query_money_greed_sell();
    arr = calc_change (price, 0, str);
    arr = tmp_arr + arr;

    if (!test)
	arr += ({ change_money(arr, ob, ob2, silent) });

    return arr;
}

/*
 * Support for selling food and drinks
 */
#pragma strict_types

#include <stdproperties.h>
#include <macros.h>
#include <money.h>
#include <language.h>

static	mixed	drinks;	/* Array holding all drink data */
static	mixed	dr_id;	/* Array holding all names the drinks can be identified
			 * with when bought */
static	mixed	food;
static	mixed	fo_id;

#define MAX_PURCHASED 20

int order(string str);

/*
 * Function name: init_pub
 * Description:   This function adds the buy and order command to this_player()
 *		  call it from your init()
 */
void
init_pub()
{
    add_action(order, "buy");
    add_action(order, "order");
}

/*
 * Function name: fix_drink
 * Description:   Set up a drink and return the object
 * Arguments:	  data - The drink data from drinks arrayen
 * Returns:	  The configured drink object
 */
object
fix_drink(mixed data)
{
    object ob;

    seteuid(getuid(this_object()));
    ob = clone_object("/std/drink");
    ob->set_name(data[0]);
    ob->set_adj(data[1]);
    ob->set_soft_amount(data[2]);
    ob->set_alco_amount(data[3]);
    ob->set_short(data[5]);
    ob->set_pshort(data[6]);
    ob->set_long(data[7]);
    ob->add_prop(HEAP_I_UNIT_VOLUME, data[2] + data[3]);
    ob->add_prop(HEAP_I_UNIT_WEIGHT, data[2] + data[3]);
    ob->add_prop(HEAP_I_UNIT_VALUE, data[4]);

    return ob;
}

/*
 * Function name: fix_food
 * Description:   Configure the food object and return it
 * Arguments:	  data - The data for the food
 * Returns:	  The food object
 */
object
fix_food(mixed data)
{
    object ob;

    seteuid(getuid(this_object()));
    ob = clone_object("/std/food");
    ob->set_name(data[0]);
    ob->set_adj(data[1]);
    ob->set_amount(data[2]);
    ob->set_short(data[4]);
    ob->set_pshort(data[5]);
    ob->set_long(data[6]);
    ob->add_prop(HEAP_I_UNIT_VOLUME, data[2]);
    ob->add_prop(HEAP_I_UNIT_WEIGHT, data[2]);
    ob->add_prop(HEAP_I_UNIT_VALUE, data[3]);

    return ob;
}

/*
 * Function name: pub_hook_cant_pay
 * Description:   A hook to redefine if you want own message when player can't 
 *		  pay the price.
 * Arguments:     price - The price he should had payed
 */
void
pub_hook_cant_pay(int price)
{
    write("You haven't got enough money to pay the price.\n");
}

/*
 * Function name: pay_hook_cant_carry
 * Description:   A hook to redefine if you want own message when player can't
 *		  carry what he ordered.
 * Arguments:     ob - The object he couldn't carry
 */
void
pub_hook_cant_carry(object ob)
{
    write(capitalize(ob->short()) + " is too heavy for you. You drop it " +
	"to the ground.\n");
    say(capitalize(ob->short()) + " is too heavy for " + QTNAME(this_player()) +
	" and falls to the ground.\n");
}

/*
 * Function name: pay_hook_player_buys
 * Description:   A hook to redefine if you want own message when player
 *		  gets his/hers order.
 * Arguments:     ob - The object player ordered
 *		  price - The price the player payed for the object
 */
void
pub_hook_player_buys(object ob, int price)
{
    write("You pay " + price + " coppers for " + ob->short() + ".\n");
    say(QCTNAME(this_player()) + " orders " + ob->short() + ".\n");
}

/*
 * Function name: pub_hook_ordered_too_many
 * Description  : Tells the player that he ordered too many items.
 * Arguments    : int num - the amount the player ordered.
 * Returns      : int 0/1 - 0 for a notify_fail, 1 for a write.
 */
int
pub_hook_ordered_too_many(int num)
{
    notify_fail("You cannot buy more than " + MAX_PURCHASED +
	" meals or drinks at a time.\n");
    return 0;
}

/*
 * Function name: pub_hook_invalid_order
 * Description:   Notify the player that the ordered item is not in stock.
 * Arguments:     string str - the item the player tried to order
 * Returns:       1/0
 */
int
pub_hook_invalid_order(string str)
{
    notify_fail("No " + str + " in stock.\n");
    return 0;
}

/*
 * Function name: pub_hook_syntax_failure
 * Description:   Notify the player of invalid syntax.
 * Arguments:     string str - arguments to the command
 * Returns:       1/0
 */
int
pub_hook_syntax_failure(string str)
{
    notify_fail(capitalize(query_verb()) + " what?\n");
    return 0;
}

/*
 * Function name: order
 * Description:   The player has ordered something, let's see if we can satisfy
 *		  him.
 * Arguments:	  str - The order from the player
 * Returns:	  1 or 0
 */
int
order(string str)
{
    string *words;
    int num, tmp, i, price;
    object ob;

    if (!str)
    {
        return pub_hook_syntax_failure(str);
    }

    words = explode(str, " ");
    num = 1;
    if (sizeof(words) > 1)
    {
	tmp = LANG_NUMW(words[0]);
	if (!tmp)
	    sscanf(words[0], "%d", tmp);
	if (tmp > 0)
	{
	    num = tmp;
	    str = implode(words[1 .. sizeof(words)], " ");
	}
    }

    if (num > MAX_PURCHASED)
    {
	return pub_hook_ordered_too_many(num);
    }

    for (i = 0; i < sizeof(dr_id); i++)
	if (member_array(str, dr_id[i]) >= 0)
	{
	    ob = fix_drink(drinks[i]);
	    price = num * drinks[i][4];
	    ob->set_heap_size(num);
	    break;
	}

    if (!ob)
	for (i = 0; i < sizeof(fo_id); i++)
	    if (member_array(str, fo_id[i]) >= 0)
	    {
		ob = fix_food(food[i]);
		price = num * food[i][3];
		ob->set_heap_size(num);
		break;
	    }

    if (!ob)
    {
        return pub_hook_invalid_order(str);
    }

    if (!MONEY_ADD(this_player(), -price))
    {
	pub_hook_cant_pay(price);
	ob->remove_object();
	return 1;
    }

    pub_hook_player_buys(ob, price);

    ob->move(this_player(), 1);
/*
 * I can't get this to work so let the players be loaded down with food if
 * they want to. Anyone willing to fix this bug, please send us a patch.
 * /Nick
 *
    tmp = num - ob->num_heap();
    if (tmp > 0)
    {
	ob->set_heap_size(tmp);
	ob->remove_remove_object_call();
	ob->move(environment(this_player()));
	pub_hook_cant_carry(ob);
    }
 */

    return 1;
}

/*
 * Function name: add_drink
 * Description:   Add a drink to the menu list
 * Arguments:	  id	    - An array with names to id the drink when buying
 *		  names     - The names of the drink
 *		  adj	    - The adjectives of the drink
 * 		  soft	    - The soft amount (integer)
 *		  alco	    - The alcoholic amount (integer)
 * 		  price	    - The price of the drink (cost to buy)
 *		  short	    - Short description of the drink (if special)
 *		  pshort    - A plural short description
 *		  long	    - Long description of the drink
 *                dummy     - Obsolete, kept for backward compability
 */
varargs void
add_drink(mixed id, mixed names, mixed adj, int soft, int alco, int price,
	  string short, string pshort, string long, mixed dummy)
{
    if (!pointerp(id))
	id = ({ id });
    if (!pointerp(names))
	names = ({ names });
    if (!pointerp(adj))
	adj = ({ adj });
    if (!drinks)
	drinks = ({ });
    if (!dr_id)
	dr_id = ({ });

    dr_id += ({ id });
    drinks += ({ ({ names, adj, soft, alco, price, short,
		    pshort, long }) });
}

/*
 * Function name: query_drinks
 * Description:   Query the drink array
 * Returns:	  The drink array
 */
mixed
query_drinks() { return drinks; }

/*
 * Function name: query_drink_id
 * Description:   Query the drink id:s
 * Returns:	  The array holding all drink id:s
 */
mixed
query_drink_id() { return dr_id; }

/*
 * Function name: remove_drink
 * Description:   Remove a special drink, identified with id
 * Arguments:	  id - A identifying string
 * Returns:	  1 if removed
 */
int
remove_drink(string id)
{
    int i;

    for (i = 0; i < sizeof(dr_id); i++)
	if (member_array(id, dr_id[i]) >= 0)
	{
	    dr_id = exclude_array(dr_id, i, i);
	    drinks = exclude_array(drinks, i, i);
	    return 1;
	}

    return 0;
}

/*
 * Function name: add_food
 * Description:   Add an item of food  to the menu list
 * Arguments:     id        - An array with names to id the food when buying
 *                names     - The names of the food
 *                adj       - The adjectives of the food
 *                amount    - The amount (integer)
 *                price     - The price of the food (cost to buy)
 *                short     - Short description of the food (if special)
 *		  pshort    - The plural short description (if special)
 *                long      - Long description of the food
 *                dummy     - Obsolete, kept for backward compability
 */
varargs void
add_food(mixed id, mixed names, mixed adj, int amount, int price,
         string short, string pshort, string long, mixed dummy)
{
    if (!pointerp(id))
        id = ({ id });
    if (!pointerp(names))
        names = ({ names });
    if (!pointerp(adj))
        adj = ({ adj });
    if (!food)
        food = ({ });
    if (!fo_id)
        fo_id = ({ });

    fo_id += ({ id });
    food += ({ ({ names, adj, amount, price, short, pshort, long }) });
}

/*
 * Function name: query_food
 * Description:   Query the food array
 * Returns:       The food array
 */
mixed
query_food() { return food; }

/*
 * Function name: query_food_id
 * Description:   Query the food id:s
 * Returns:       The array holding all food id:s
 */
mixed
query_food_id() { return fo_id; }

/*
 * Function name: remove_food
 * Description:   Remove a special food, identified with id
 * Arguments:     id - A identifying string
 * Returns:       1 if removed
 */
int
remove_food(string id)
{
    int i;

    for (i = 0; i < sizeof(fo_id); i++)
        if (member_array(id, fo_id[i]) >= 0)
        {
            fo_id = exclude_array(fo_id, i, i);
            food = exclude_array(food, i, i);
            return 1;
        }

    return 0;
}


/*
 * /sys/glocal/money.c
 *
 * This object contains the code for the global money defines.
 */

#pragma save_binary

#include <composite.h>
#include <language.h>
#include <macros.h>
#include <money.h>
#include <stdproperties.h>

/* Prototypes. */
int *what_coins(mixed ob);

/*
 * Function name: split_values 
 * Description:   Splits a 'copper' value into pc, gc, sc, cc
 * Argument:      v: value in copper coins
 * Returns:       Array: ({ cc, sc, gc, pc })
 */
int *
split_values(int v)
{
    int *ret;
    int index = SIZEOF_MONEY_TYPES;
  
    ret = allocate(SIZEOF_MONEY_TYPES);
    if (v > 0)
    {
        while(--index >= 0)
        {
            ret[index] = v / MONEY_VALUES[index];
            v %= MONEY_VALUES[index];
        }
    }

    return ret;
}

/*
 * Function name: merge_values 
 * Description:   Merges different coins into the value in copper coins
 * Argument:      av: Array ({ cc, sc, gc, pc })
 * Returns:       int - the value in copper coins.
 */
int
merge_values(int *av)
{
    int value = 0;
    int index = -1;

    if (sizeof(av) != SIZEOF_MONEY_TYPES)
    {
        return 0;
    }

    while(++index < SIZEOF_MONEY_TYPES)
    {
        value += av[index] * MONEY_VALUES[index];
    }

    return value;
}

/*
 * Function name: make_coins
 * Description:   Makes a certain number of coins of a certain type
 * Argument:      str: Cointype: copper,silver,gold or platinum
 *                num: Number of coins
 * Returns:       Objectpointer to the coins object or 0.
 */
object
make_coins(string str, int num)
{
    object cn;
  
    if (!str ||
        !num)
    {
        return 0;
    }
  
    cn = clone_object("/std/coins");
    cn->set_heap_size(num);
    cn->set_coin_type(str);
    return cn;
}

/*
 * Function name: move_coins
 * Description:   Moves a certain number of coins.
 * Argument:      str: Cointype: copper,silver,gold or platinum
 *                num: Number of coins
 *                from: From which inventory or 0 if create new
 *                to: To which inventory or 0 if destruct
 * Returns:       -1 if not found, 0 == moved, >0 move error code
 */
int
move_coins(string str, int num, mixed from, mixed to)
{
    object cn, f, t, cf;
    int max, okflag;
  
    if (!str || (num <= 0)) 
        return -1;
  
    if (stringp(from))
    {
        f = find_object(from);
        if (!f)
            f = find_player(from);
    } else if (objectp(from))
        f = from;
    else
        f = 0;

    if (stringp(to))
    {
        t = find_object(to);
        if (!t)
            t = find_player(to);
    }
    else if (objectp(to))
        t = to;
    else
        t = 0;

    if (f)
        cf = present(str+" coin",f);
    else
        cf = make_coins(str, num);

    if (!cf || !(max = cf->num_heap()))
        return -1;

    if (num > max)
        return -1;

    if (t)
    {
        if (num < max)
            cf->split_heap(num);
        return cf->move(t);
    }

    if (!t && num < max)
        cf->set_heap_size(max-num);
    else
        cf->remove_object();

    return 0;
}

/*
 * Function name: move_cointypes
 * Description:   Move a certain number of each coin type.
 * Arguments:     (int *)  An integer array containing the number of each
 *                         coin type to move.
 *                (object) Where to take the coins from--0 if they are to
 *                         be newly created.
 *                (object) Where to put the coins--0 if they are to be
 *                         destroyed.
 * Returns:       -1 - Not enough coins found
 *                 0 - Move successful
 *                >0 - Move error code
 */
public int
move_cointypes(int *coins, object from, object to)
{
    int i, j, res;
    int *all_coins;

    if (from)    
    {
        all_coins = what_coins(from);

        /* Check that coins are present before we start removing them.
         * This lessens the chance of having to restore coins later.
         */
        for (i = 0; i < sizeof(MONEY_TYPES); i++)
        {
            if (coins[i] > all_coins[i])
            {
                return -1;
            }
        }
    }
    
    for (i = 0; i < sizeof(MONEY_TYPES); i++)
    {
        if (coins[i] == 0)
        {
            continue;
        }
    
        if ((res = move_coins(MONEY_TYPES[i], coins[i], from, to)) != 0)
        {
            /* We were unable to move some coins, so we have to undo
             * previous transfers.
             */
            for (j = 0; j < i; j++)
            {
                move_coins(MONEY_TYPES[j], coins[j], to, from);
            }
    
            return res;
        }
    }
    
    return 0;
}

/*
 * Function name: what_coins
 * Description:   Finds out what of the normal cointypes a certain object
 *                contains.
 * Argument:      ob: The object in which to search for coins
 * Returns:       Array: ( num copper, num silver, num gold, num platinum )
 */
int *
what_coins(mixed ob)
{
    object pl, cn;
    int index = -1;
    int *nums;

    if (objectp(ob))
        pl = ob;
    else if (stringp(ob))
    {
        pl = find_object(ob);
        if (!pl)
        {
            pl = find_player(ob);
        }
    }
    else
        return 0;

    nums = allocate(SIZEOF_MONEY_TYPES);

    while(++index < SIZEOF_MONEY_TYPES)
    {
        cn = present(MONEY_TYPES[index] + " coin",pl);
        if (!cn)
        {
            nums[index] = 0;
        }
        else
        {
            nums[index] = cn->num_heap();
        }
    }
    return nums;
}

/* 
 * Function name: total_money
 * Description: calculates the total amount of money on a living
 */
public int
total_money(object who)
{
    return merge_values(what_coins(who));
}

/* 
 * Function name: give_money
 * Description: gives a certain sum back to this object
 */
public int
give_money(object who, int amount)
{
    object ob;
    int to_do, i, n_coins, c_flag;

    to_do = amount;
    i = SIZEOF_MONEY_TYPES - 1;
    c_flag = 0;

    for (i = SIZEOF_MONEY_TYPES - 1; i >= 0 && to_do; i--)
    {
        n_coins = to_do / MONEY_VALUES[i];
        to_do = to_do % MONEY_VALUES[i];
        if(n_coins > 0)
        {
            ob = make_coins(MONEY_TYPES[i], n_coins);
            if((int)ob->move(who))
            {
                ob->move(environment(who));
                c_flag = 1;
            }
        }
    }

    if (c_flag)
    {
        tell_object(who, "You cannot carry this much money, you drop it on " +
            "the ground.\n");
        say(QCTNAME(who) + " drops some money on the ground.\n", who);
    }
    return 1;
}

/* 
 * Function name: take_money
 * Description: reduces the money of someone with a given amount
 *              also handles giving back money, if necessary
 * Returns:     0:   player doesn't have enough money  
 *              1:   okay, money subtracted from player's money
 */
public int
take_money(object who, int amount)
{
    int *money_list, i, rest, c_flag;
    object *ob_list, ob;
    
    if (total_money(who) < amount)
    {
        return 0;
    }
    
    money_list = allocate(SIZEOF_MONEY_TYPES);
    ob_list = allocate(SIZEOF_MONEY_TYPES);
    
    for (i = 0; i < SIZEOF_MONEY_TYPES; i++)
    {
        ob = present(MONEY_TYPES[i] + " coin", who);
        if (ob)
        {
            ob_list[i] = ob;
            money_list[i] = (int) ob->query_prop(OBJ_I_VALUE);
        }
    }
    
    for (i = 0; i < SIZEOF_MONEY_TYPES; i++)
    {
        if (amount <= money_list[i])
        {
            money_list[i] -= amount;
            break;
        }
        else
        {
            amount -= money_list[i];
            money_list[i] = 0;
        }
    }

    rest = 0;
    i = SIZEOF_MONEY_TYPES;
    while(--i >= 0)
    {
        money_list[i] += rest;
        rest = money_list[i] % MONEY_VALUES[i];
        money_list[i] = money_list[i] / MONEY_VALUES[i];

        if (ob_list[i])
            ob_list[i]->set_heap_size(money_list[i]);
        else
        {
            if (money_list[i] > 0)
            {
                ob = make_coins(MONEY_TYPES[i], money_list[i]);
                if((int)ob->move(who))
                {
                    ob->move(environment(who));
                    c_flag = 1;
                }
            }
        }
    }

    if (c_flag)
    {
        tell_object(who, "You cannot carry this much money, you drop it on " +
            "the ground.\n");
        say(QCTNAME(who) + " drops some money on the ground.\n", who);
    }
    return 1;
}

/* 
 * Function name: add_money
 * Description:   Gives money to or takes money from a living
 *                smallest possible denominators are taken,
 *                largest possible denominators are given
 * Arguments:     who: Object pointer to a living object
 *                amount: Amount to be given in copper coins
 *                        negative amount means take coins
 * Returns:       1 - success, 0 - fail
 */
public int
add_money(object who, int amount)
{
    return ((amount < 0) ?
        take_money(who, ABS(amount)) : give_money(who, amount));
}

/*
 * Function name: money_text
 * Description  : This function will return a string describing the money
 *                int the array that is passed as argument.
 * Arguments    : int *coins - the array with the coins, smallest denomination
 *                             first.
 *                int numerical - if true, use numerical values.
 * Returns      : string - a string describing the coins.
 */
public varargs string
money_text(int *coins, int numerical = 0)
{
    string *text = ({ });
    int    total = 0;
    int    index = SIZEOF_MONEY_TYPES;

    if (sizeof(coins) != SIZEOF_MONEY_TYPES)
    {
        return "a strange number of coins";
    }

    while(--index >= 0)
    {
        if (!coins[index]) continue;

        total += coins[index];
        if (numerical)
        {
            text += ({ coins[index] + " " + MONEY_TYPES[index] });
        }
        else
        {
            text += ({ ((coins[index] == 1) ? "a" : LANG_WNUM(coins[index]) ) +
                " " + MONEY_TYPES[index] });
        }
    }

    if (total == 1)
    {
        return text[0] + " coin";
    }

    index = sizeof(text);
    switch(index)
    {
    case 0:
        return "no money at all";
        break;

    case 1:
        return text[0] + " coins";
        break;

    default:
        return COMPOSITE_WORDS(text) + " coins";
    }
}

/*
 * Function name: money_col_text
 * Description  : This function will return a string describing the money
 *                using the short codes, fit for use in columns.
 * Arguments    : int *coins - the array with the coins, smallest denomination
 *                             first.
 *                int width - the number of digits per column (default: 2)
 * Returns      : string - a string describing the coins.
 */
public varargs string
money_col_text(int *coins, int width = 2)
{
    string text = "";
    int index = SIZEOF_MONEY_TYPES;

    if (sizeof(coins) != SIZEOF_MONEY_TYPES)
    {
        return "a strange number of coins";
    }
    
    while (--index >= 0)
    {
        if (coins[index] > 0)
        {
            text += sprintf(" %*d %s", width, coins[index], MONEY_SHORT[index]);
        }
        else
        {
            /* This assumes a short code is 2 characters. */
            text += sprintf(" %*s", width + 3, "");
        }
    }
    /* Skip the first space. */
    return text[1..];
}

/*
 * Function name: money_condense
 * Description  : This function will take the coin objects in the inventory of
 *                the object 'obj' and store the total value into the property
 *                OBJ_M_HAS_MONEY. Then those coins are destructed.
 * Arguments    : object obj - the object to check and condense.
 */
void
money_condense(object obj)
{
    int index = -1;

    obj->add_prop(OBJ_M_HAS_MONEY, what_coins(obj));

    while(++index < SIZEOF_MONEY_TYPES)
    {
        present((MONEY_TYPES[index] + " coin"), obj)->remove_object();
    }
}

/*
 * Function name: money_expand
 * Description  : This function will check the property OBJ_M_HAS_MONEY on the
 *                the object 'obj' and store the equivalent of real coins in
 *                the inventory. When the value of the property is an array,
 *                use exactly that amount of coins.
 * Arguments    : object obj - the object to check and expand.
 */
void
money_expand(object obj)
{
    mixed value = obj->query_prop(OBJ_M_HAS_MONEY);
    int index;
    int split;

    /* No money, no action. */
    if (!value)
    {
        return;
    }

    /* If it is an integer, generate the amount of coins with some random. */
    if (intp(value))
    {
        value = split_values(value);
        index = SIZEOF_MONEY_TYPES;
        while(--index > 0)
        {
            if (value[index] <= 0)
            {
                continue;
            }

            /* Split either no coins, split one or two coins. */
            split = random(MIN(value[index], 3));
            if (split <= 0)
            {
                continue;
            }

            /* Notice, this requires there being an equal factor between all
             * subsequent money values.
             */
            value[index] -= split;
            value[index - 1] +=
                (split * (MONEY_VALUES[index] / MONEY_VALUES[index - 1]));
        }
    }

    /* If it is an array, give exactly that amount of coins. Also fallthrough
     * from conversion if an integer value was given that is now converted.
     */
    if (pointerp(value))
    {
        /* Sanity check. Add zeros if necessary. */
        if (sizeof(value) < SIZEOF_MONEY_TYPES)
        {
            value += allocate(SIZEOF_MONEY_TYPES);
        }

        index = -1;
        while(++index < SIZEOF_MONEY_TYPES)
        {
            /* Move with force. Be lazy, don't check for weights. */
            make_coins(MONEY_TYPES[index], value[index])->move(obj, 1);
        }

        obj->remove_prop(OBJ_M_HAS_MONEY);
        return;
    }
}

/*
 * Function:    edit_coin_str
 * Description: Removes extra whitespace, "and", ",", and "all" 
 *              from a string
 * Arguments:   string str - the string to be edited
 * Returns:     the edited string
 */
string
edit_coin_str(string str)
{
    /* Remove extra whitespace and the words "and" and "all" */
    str = implode(explode(str, " ") - ({ "", "and", "all" }), " ");
    /* Remove commas */
    str = implode(explode(str, ","), " ");

    return str;
}
 
/*
 * Function:    parse_coins
 * Description: Parse a string that specifies a set of coins.
 * Arguments:   string str - a string describing the coins
 *              object ob  - where the described coins can be found (optional)
 * Returns:     An array specifying the number of each coin type described
 *              by the string ( number copper, number silver, number gold,
 *              number platinum ).  -1 for a coin type means that the user
 *              has specified that all coins of that type should be used.  If
 *              the optional second argument is given, -1 will never be
 *              returned: the actual number of coins found on the object will
 *              be given instead.
 */
varargs mixed
parse_coins(string str, object ob)
{
    int *coins, *all_coins, i, j, amnt;
    string *tmp;
   
    coins = allocate(sizeof(MONEY_TYPES));

    if (parse_command(str, ({}), "[all] [my] [the] 'coins'"))
    {
        return (ob ? what_coins(ob) : map(coins, &constant(-1)));
    }
   
    tmp = explode(edit_coin_str(str), " ");

    if (!sizeof(tmp))
    {
        return 0;
    }

    /* The coin specification is relatively simple, so I've decided to handle
     * it this way rather than using parse_command(), which adds a lot of
     * additional processing (especially dealing with the heap objects) and
     * always requires "coin" to be used ("10 gold" is not recognized; "10
     * gold coins" is required).
     */
    i = 0;
    while (i < sizeof(tmp))
    {
        if ((j = member_array(tmp[i], MONEY_TYPES)) < 0)
        {
            /* The word doesn't specify a coin type, so it needs to be a
             * number followed by a coin type.
             */

            /* Check to see if the following word is a coin type.  If not,
             * we have an invalid specification.
             */ 
            if (((i + 1) >= sizeof(tmp)) ||
                ((j = member_array(tmp[i + 1], MONEY_TYPES)) < 0))
            {
                return 0;
            }
       
            /* Check to see if the word is a number.  If not, we have an
             * invalid specification.
             */
            if (!sscanf(tmp[i], "%d", amnt) && !(amnt = LANG_NUMW(tmp[i])))
            {
                return 0;
            }

            /* Negative amounts not accepted */
            if (amnt < 0)
            {
                return 0;
            }

            i++;
        }
        else
        {
            /* All coins of this type were specified */
            coins[j] = -1;
        }

        if (coins[j] >= 0)
        {
            coins[j] += amnt;
        }

        i++;
     
        if ((sizeof(tmp) > i) && ((tmp[i] == "coins") || (tmp[i] == "coin")))
        {
            if ((tmp[i] == "coin") && (coins[j] == -1))
            {
                coins[j] = 1;
            }
            i++;
        }
    }

    /* If we know where the coins are, go back and replace -1 values with
     * the actual number of coins found in the object.
     */
    if (ob)
    {
        all_coins = what_coins(ob);

        for (i = 0; i < sizeof(coins); i++)
        {
            if (coins[i] == -1)
            {
                coins[i] = all_coins[i];
            }
        }
    }
                
    return coins;
}

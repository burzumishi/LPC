/*
   /std/drink.c
   
   This is the standard object used for any form of drinkable stuff.

 Usage:

        inherit "/std/drink";

        void
        create_drink()
        {
            set_soft_amount(amount_in_millilitres);
            set_alco_amount(amount_in_millilitres);
            set_name("name of drink");
            set_drink_msg("extra message to get when drinking");
            set_short(...);
            set_long(....);
        }

        An imperial pint is 540 millilitres.
        
    If you want any special effect in your drink you can define
    special_effect() which is called when drinking it.
    The number of drinks you drank is passed as argument to special_effect();

    Drinks that are coded in a special file will recover. With a special
    file I mean that they aren't created using a clone of /std/drink and
    calling the necessary functions externally, like /lib/pub does.
*/

#pragma save_binary
#pragma strict_types

inherit "/std/heap";

#include <cmdparse.h>
#include <composite.h>
#include <files.h>
#include <macros.h>
#include <stdproperties.h>

static  int     soft_amount,
                alco_amount;

static  string  drink_msg;

/*
 * Function name: create_drink
 * Description  : This function is called to create the drink. You should
 *                redefine this function to set the descriptions and the
 *                amount of alco, etcetera.
 */
public void
create_drink()
{
}

/*
 * Function name: create_heap
 * Description  : Constructor. It calls create_drink(), the function you
 *                must redefine. The default heap-size is 1.
 */
public nomask void
create_heap()
{
    set_heap_size(1);
    add_prop(OBJ_M_NO_SELL, "Drinks are not to be resold.\n");

    set_name("drink");
    set_pname("drinks");

    create_drink();

    if (!query_prop(HEAP_S_UNIQUE_ID))
    {
        add_prop(HEAP_S_UNIQUE_ID,
            MASTER + ":" + singular_short(this_object()));
    }
}

/*
 * Function name: reset_heap
 * Description  : When reset has been enabled, this function is called at a.
 *                regular interval. It calls the function reset_drink() that
 *                you should define.
 */
public nomask void
reset_heap()
{
    this_object()->reset_drink();
}

/*
 * Function name: set_short
 * Description  : When the short is set/changed, also make sure that we do set
 *                the unique identifier for the heap.
 * Arguments    : mixed new_short - the short description.
 */
void
set_short(mixed new_short)
{
    ::set_short(new_short);

    add_prop(HEAP_S_UNIQUE_ID,
        MASTER + ":" + singular_short(this_object()));
}

/*
 * Function name: set_drink_msg
 * Description  : Sets the extra message given to drinker when
 *                s/he consumes the drink.
 * Arguments    : string str - the string to be written.
 */
public void
set_drink_msg(string str)
{
    drink_msg = str;
}

/*
 * Function name: set_soft_amount
 * Description  : sets the amount of liquid in the drink.
 * Arguments    : int a - the amount of liquid.
 */
public void
set_soft_amount(int a)
{
    soft_amount = a;
}

/*
 * Function name: query_soft_amount
 * Description  : Gives the amount of liquid in the drink.
 * Returns      : int - the amount.
 */
public int
query_soft_amount()
{
    return soft_amount;
}

/*
 * Function name: set_alco_amount
 * Description  : Sets the amount of alcohol in the drink.
 * Arguments    : int a - the amount of alcohol.
 */
public void
set_alco_amount(int a)
{
    alco_amount = a;
}

/*
 * Function name: query_alco_amount
 * Description  : Gives the amount of alcohol in the drink.
 * Returns      : int - the amount.
 */
public int
query_alco_amount()
{
    return alco_amount;
}

/*
 * Function name: command_drink
 * Description:   drink this drink
 * Returns:       string - an error message (failure)
 *                1 - drink successfully imbibed
 */
public mixed
command_drink()
{
    int am1, am2, pstuff, num, i;

    am1 = query_soft_amount();
    am2 = query_alco_amount();
    num = num_heap();

    for (i = 0; i < num; i++)
    {
        /* See if we are still thirsty */
        if (!this_player()->drink_soft(am1))
        {
            /* We couldn't drink all of the drinks in this heap, so split
             * it and drink what we can.
             */

            if (i == 0)
            {
                return "The " + singular_short() + " is too much for you.\n";
            }

            split_heap(i);

            this_object()->special_effect(i);

            return 1;
        }

        /* See if we are too drunk */
        if (!this_player()->drink_alco(am2))
        {
            /* We couldn't drink all of the drinks in this heap, so split
             * it and drink what we can.
             */

            this_player()->drink_soft(-am1);

            if (i == 0)
            {
                return "The " + singular_short() + " is too strong for you.\n";
            }

            split_heap(i);

            this_object()->special_effect(i);

            return 1;
        }
    }
    
    this_object()->special_effect(num);

    return 1;
}

public void
remove_drink()
{
    if (leave_behind > 0)
    {
        set_heap_size(leave_behind);
    }
    else
    {
        remove_object();
    }
}
    
/*
 * Function name: config_split
 * Description  : This is called before inserting this heap into the game.
 *                It is meant to copy all relevant variables to the copy
 *                of the heap. In this case, it is the alco-amount and the
 *                soft amount.
 * Arguments    : int new_num - the number of items in the new heap.
 *                object orig - the original heap we are copying.
 */
void
config_split(int new_num, object orig)
{
    ::config_split(new_num, orig);
    set_soft_amount(orig->query_soft_amount());
    set_alco_amount(orig->query_alco_amount());
}

/*
 * Function name: config_merge
 * Description  : When merging two identical drinks with different volumes,
 *                average them by the number.
 * Arguments    : object child - the child merged into this drink.
 */
void
config_merge(object child)
{
    int total, count;

    ::config_merge(child);

    /* If there is a difference, average it. */
    if (query_soft_amount() != child->query_soft_amount())
    {
        total = (query_soft_amount() * num_heap()) + (child->query_soft_amount() * child->num_heap());
        count = num_heap() + child->num_heap();
        set_soft_amount(total / count);
    }
    if (query_alco_amount() != child->query_alco_amount())
    {
        total = (query_alco_amount() * num_heap()) + (child->query_alco_amount() * child->num_heap());
        count = num_heap() + child->num_heap();
        set_alco_amount(total / count);
    }
}

/*
 * Function name: stat_object
 * Description  : This function is called when a wizard wants to get more
 *                information about an object.
 * Returns      : string str - the information.
 */
string
stat_object()
{
    return ::stat_object() +
        "Soft: " + soft_amount + "\n" +
        "Alco: " + alco_amount + "\n";
}

/*
 * Function name: query_recover
 * Description  : This function is called to see whether this object may
 *                recover. It will only function for drinks that have a
 *                real file rather than being cloned from /std/drink.c
 *                since only the amount of drinks on the heap is saved.
 */
public string
query_recover()
{
    string file = MASTER;

    /* Don't recover bare /std/drinks since we only recover the amount of
     * drinks and no descriptions.
     */
    if (file == DRINK_OBJECT)
    {
        return 0;
    }

    return file + ":heap#" + num_heap() + "#";
}

/*
 * Function name: init_recover
 * Description  : This function is called when the drinks recover.
 * Arguments    : string str - the recover argument.
 */
public void
init_recover(string str)
{
    string foobar;
    int    num;

    if (sscanf(str, "%sheap#%d#%s", foobar, num, foobar) == 3)
    {
        set_heap_size(num);
    }
}

/*
 * Function name: special_effect
 * Description  : Define this routine if you want to do some special effect
 *                if a player consumes this drink. By default, it displays
 *                the message set with set_drink_msg(), if any.
 * Arguments    : int amount - the number of drinks consumed.
 */
public void
special_effect(int amount)
{
    if (strlen(drink_msg))
    {
        write(drink_msg);
    }
}

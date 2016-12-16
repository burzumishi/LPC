/*
 * /lib/store_support.c
 *
 * This file is to be inherited into all storerooms for shops. It makes sure
 * that there are just a few items in stock in the shop. Too many items
 * occupy too much memory.
 *
 * To make the store work you have to include the following function in your
 * store-room code:
 *
 *   public void
 *   enter_inv(object obj, object from)
 *   {
 *       ::enter_inv(obj, from);
 *       store_update(obj);
 *   }
 *
 * It is also possible to give the store a default stock, that will be
 * replenished every time the room resets. Do enable this, add the following
 * function to your store room, and use the function set_default_stock() to
 * set the default stock of the store room. To give the store its default
 * stock upon creation, you can call reset_store() from your create_room()
 * after using set_default_stock().
 *
 *   public void
 *   reset_room()
 *   {
 *       reset_store();
 *   }
 */

#pragma save_binary
#pragma strict_types

#include <filter_funs.h>
#include <macros.h>

/*
 * MAX_ITEMS     - The maximum number of items allowed in this store.
 * MAX_IDENTICAL - The maximum number of identical items allowed in this store.
 * MAX_DEFAULT   - The maximum number of default items allowed in this store.
 */
#define MAX_ITEMS     (30)
#define	MAX_IDENTICAL (10)
#define MAX_DEFAULT   (10)

static int     max_items      = MAX_ITEMS;
static int     max_identical  = MAX_IDENTICAL;
static int     stock_alarm_id = 0;
static object *remove_list    = ({ });
static mixed   default_stock  = ({ });

/*
 * Function name: set_max_items
 * Description  : Set the maximum number of items allowed in this store. This
 *                maximum may not be above the predefined maximum.
 * Arguments    : int items - the maximum number of items allowed.
 */
void
set_max_items(int items)
{
    if ((items >= 1) ||
        (items <= MAX_ITEMS))
    {
	max_items = items;
    }
}

/*
 * Function name: query_max_items
 * Description  : Query the maximum number of items allowed in this store.
 * Returns      : int - the number.
 */
int
query_max_items()
{
    return max_items;
}

/*
 * Function name: set_max_identical
 * Description  : Set the maximum number of identical items allowed in this
 *                store. As identical items count those items that have
 *                identical long descriptions. This maximum may not be above
 *                the predefined maximum.
 * Arguments    : int items - the maximum number of identical items allowed.
 */
void
set_max_identical(int identical)
{
    if ((identical >= 1) ||
        (identical <= MAX_IDENTICAL))
    {
	max_identical = identical;
    }
}

/*
 * Function name: query_max_identical
 * Description  : Query the maximum number of identical items allowed in this
 *                store.
 * Returns      : int - the number.
 */
int
query_max_identical()
{
    return max_identical;
}

/*
 * Function name: set_max_values
 * Description  : Set maximum number of items in a store room and the maximum
 *                number of identical items.
 * Arguments    : int items     - max number of items in store room.
 *                int identical - max number of identical items in store room.
 */
void
set_max_values(int items, int identical)
{
    set_max_items(items);
    set_max_identical(identical);
}

/*
 * Function name: store_remove_items
 * Description  : Called with a little delay to actually remove items from
 *                the store when there are too many items.
 */
void
store_remove_items()
{
    /* Only remove items that are actually in this store still. */
    remove_list &= all_inventory(this_object());

    remove_list->remove_object();

    remove_list = ({ });
    stock_alarm_id = 0;
}

/*
 * Function name: store_update
 * Description  : Update the contents of the storeroom, remove excess items.
 * Arguments    : object obj - the object that is added to the store.
 */
void 
store_update(object obj)
{
    int inv_size;
    object *inv;
    object *identical = ({ });

    /* Livings are not a part of the store inventory. */
    if (living(obj))
    {
	return;
    }

    inv = FILTER_DEAD(all_inventory(this_object()) - ({ obj }) - remove_list);

    if (max_identical)
    {
        /* We find two items identical if their long descriptions are
         * identical.
         */
        identical = filter(inv, &operator(==)(obj->long()) @ &->long());
        if (sizeof(identical) >= max_identical)
        {
            remove_list += identical[..(sizeof(identical) - max_identical)];
            inv -= remove_list;
        }
    }

    inv_size = sizeof(inv);
    if (inv_size >= max_items)
    {
        /* Remove excess items, but don't remove items that belong to the
         * default stock. */
        foreach (object item : inv)
        {
            if ((inv_size-- >= max_items) &&
                !IN_ARRAY(MASTER_OB(item), default_stock))
            {
                remove_list += ({ item });
            }
        }
    }

    /* Items targetted for removal? */
    if (!stock_alarm_id && sizeof(remove_list))
    {
        stock_alarm_id = set_alarm(1.0, 0.0, store_remove_items);
    }
}

/*
 * Function name: set_default_stock
 * Descripton   : Set the default stock for this store room. Every time the
 *                room resets, the store will be updated to contain a minimum
 *                stock.
 *                The default stock is represented as an array of the form:
 *
 *                ({ (string)filename1, (int)stock1, 
 *                   (string)filename2, (int)stock2,
 *                })
 *
 * Arguments    : mixed - the default stock.
 * Example      : An example of the default stock might be (for a default of
 *                two ponchos, one oil lamp and five flasks of lamp oil):
 *
 *                ({ "/d/Domain/arm/poncho", 2,
 *                   "/d/Domain/obj/oil_lamp", 1,
 *                   "/d/Domain/obj/lamp_oil", 5
 *                })
 */
void
set_default_stock(mixed stock)
{
    int index;

    index = sizeof(stock);
    while((index -= 2) >= 0)
    {
        /* Strip the .c suffix from file names. */
        if (wildmatch("*.c", stock[index]))
        {
            stock[index] = extract(stock[index], 0, -3);
        }

        /* Allow only a maximum number of identical default items. */
        if (stock[index + 1] > MAX_DEFAULT)
        {
            stock[index + 1] = MAX_DEFAULT;
        }
    }

    default_stock = stock;
}

/*
 * Function name: reset_store
 * Description  : If you use a default stock in this store room, you must call
 *                reset_store() from your reset_room() call. In order to start
 *                the store with its default stock upon creation, from your
 *                create_room() function make a call to reset_store() after you
 *                have initialised the default stock with set_default_stock().
 *
 *                Each reset the store checks its inventory to judge whether
 *                the stock must be replenished. If there are more items than
 *                the default stock describes, items are not removed. If the
 *                stock size is set to two or more, a little random is used to
 *                clone one more or one less than the assigned default, i.e. a
 *                default amount of 1 is always 1 and a default amount of 2
 *                can be 1,2,3 and a default of 4 can be 3,4,5.
 */
void
reset_store()
{
    int index = -2;
    int size  = sizeof(default_stock);
    int total;
    int counted;
    object *inv;

    inv = all_inventory();
    inv -= FILTER_PLAYERS(inv);
    
    /* For each of the items in the default stock, check the amount of items
     * in stock and clone new items if necessary.
     */
    while ((index += 2) < size)
    {
        total = ((default_stock[index + 1] == 1) ? default_stock[index + 1] :
            (default_stock[index + 1] - 1 + random(3)));

        counted = sizeof(filter(inv, &operator(==)(default_stock[index]) @
                             &extract(, 0, (strlen(default_stock[index]) - 1)) @
                             file_name));
        
        while(++counted <= total)
        {
            clone_object(default_stock[index])->move(this_object(), 1);
        }
    }
}

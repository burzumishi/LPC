/* /d/Standard/lib/craftsman.c
 * 
 * An inheritable module for creating shops to sell items with
 * player-selected attributes.
 *
 * Example usage:
 *
 * inherit "/d/Standard/lib/craftsman";
 * inherit "/std/monster";
 *
 * public void
 * create_monster()
 * {
 *     // possible names for the item sold
 *     craftsman_set_sold_item_names(({ "cloak" }));
 * }
 *
 * public void
 * craftsman_configure_order(int order_id, string arg)
 * {
 *     // the file for the item to clone
 *     craftsman_set_item_file_order_id, "/d/Emerald/shiva/newcloak");
 * 
 *     // time in seconds to complete creation of the item
 *     craftsman_set_time_to_complete(order_id, 30);
 * 
 *     // the cost of an item (in cc)
 *     craftsman_set_item_cost(order_id, 800);
 *
 *     // attributes for the items with possible values
 *     craftsman_add_attribute(order_id, "material",
 *         ({ "leather", "silk", "velvet" }));
 *     craftsman_add_attribute(order_id, "color",
 *         ({ "ivory", "purple", "grey" }));
 *     craftsman_add_attribute(order_id, "quality",
 *         ({ "crude", "splendid", "shoddy" }));
 * }
 *
 * public void
 * init_living()
 * {
 *     ::init_living();
 * 
 *     // add the craftsman's commands
 *     craftsman_init();
 * }
 * 
 * public void
 * craftsman_configure_item(object ob, mapping attrs, int order_id)
 * {
 *     ob->set_adj(m_values(attrs));
 *
 *     switch (attrs["material"])
 *     {
 *         case "leather":
 *             ob->set_ac(10);
 *             break;
 *         case "silk":
 *             ob->set_ac(2);
 *             break;
 *         case "velvet":
 *             ob->set_ac(4);
 *             break;
 *         case "wool":
 *             ob->set_ac(6);
 *             break;
 *     }
 * }
 */

#pragma strict_types

inherit "/lib/trade";

#include <language.h>
#include <money.h>

#define INPUT_OBJ "/d/Standard/obj/input_object"

#define STATUS_IN_PROGRESS 0
#define STATUS_CONFIGURED  1
#define STATUS_COMPLETED   2

#define ORDER_STATUS           0
#define ORDER_OWNER            1
#define ORDER_ATTRS            2
#define ORDER_SELECTIONS       3
#define ORDER_FILE             4
#define ORDER_TIME             5
#define ORDER_COMPLETION_TIME  6
#define ORDER_COST             7
#define ORDER_HOLD             8
#define ORDER_TIMEOUT          9
#ifdef INPUT_OBJ
#define ORDER_INPUT           10
#define ORDER_SIZE            11
#else
#define ORDER_SIZE            10
#endif

static string *item_names = ({});

static int completion_alarm;
static int next_completion_time;
static int max_total_orders = 10;
static int max_user_orders = 1;
static int selection_timeout = 120;

mapping m_orders = ([]);

void craftsman_config_attributes(int id, int pos);
void craftsman_completed_config(int id);
void craftsman_completed_order(int id);
void craftsman_reset_completion_time();
void craftsman_clean_completed_orders();
int craftsman_calc_item_cost(object ob, int id);
public int craftsman_query_completion_time(int id);
public string craftsman_query_item_file(string order_name);
public int craftsman_new_order(string owner);
public void craftsman_configure_order(int id, string str);
public void craftsman_cancel_order(int id);
public int craftsman_query_time_to_complete(int id);
public int craftsman_query_hold_time(int id);

/*
 * Function name: craftsman_purchase_syntax_failure_hook
 * Description:   Redefine this to change the message given when the user
 *                gives incorrect arguments to the "purchase" command
 * Arguments:     (string) the arguments given to the "purchase" command.
 * Returns:       1/0
 */
public int
craftsman_purchase_syntax_failure_hook(string str)
{
    notify_fail("We don't sell those here.\n");
    return 0;
}

/*
 * Function name: craftsman_purchase_exceeded_max_total_orders_hook
 * Description:   Redefine this to change the message given when the user has
 *                exceeded the maximum number of orders allowed for all users.
 * Arguments:     (string) arguments to the order command (the item being
 *                         ordered)
 * Returns:       0/1
 */
public int
craftsman_purchase_exceeded_max_total_orders_hook(string str)
{
    write("Cannot accept any more orders at this time.\n");
    return 1;
}

/*
 * Function name: craftsman_purchase_exceeded_max_user_orders_hook
 * Description:   Redefine this to change the message given when the user has
 *                exceeded the maximum number of orders allowed for a single
 *                user.
 * Arguments:     (string) arguments to the order command (the item being
 *                         ordered
 * Returns:       0/1
 */
public int
craftsman_purchase_exceeded_max_user_orders_hook(string str)
{
    write("Cannot accept any more orders at this time.\n");
    return 1;
}

/* 
 * Function name: craftsman_purchase_hook
 * Description:   Redefine this to change the message given when a user tries
 *                to purchase an item.
 * Arguments:     (string) the name of the item purchased
 */
public void
craftsman_purchase_hook(string str)
{
    write("How would you like your " + str + " made?\n");
}

/*
 * Function name: craftsman_config_prompt_attribute_hook
 * Description:   Redefine this to change the message given to prompt the user
 *                to enter a attribute value.
 * Arguments:     1. (string)   the attribute name
 *                2. (string *) an array of valid selections
 */
public void
craftsman_config_prompt_attribute_hook(string attr, string *valid_attrs)
{
    write(sprintf("\nSelect " + LANG_ADDART(attr) + 
        " from the following (~q to exit):\n%-#50s\n:", 
        implode(valid_attrs, "\n")));
}

/*
 * Function name: craftsman_config_invalid_attribute_value_hook
 * Description:   Redefine this to change the message given when the user
 *                selects an invalid value for an attribute
 * Arguments:     (string) the attribute name
 *                (string) the value selected for the attribute
 */
public void 
craftsman_config_invalid_attribute_value_hook(string attr, string value)
{
    write("Invalid selection.\n");
}

/*
 * Function name: craftsman_config_validate_chosen_attribute_hook
 * Description:   Validate an attribute chosen and give any appropriate
 *                messages.
 * Arguments:     1. (string)   the name of the attribute
 *                2. (string)   the value chosen
 *                3. (string *) list of all the valid values for the attribute
 *                4. (int)      the order id
 * Returns:       1 - value is valid
 *                0 - value is invalid and should be reentered
 *               -1 - valud is invalid, and configuration should be exited
 */
public int
craftsman_config_validate_chosen_attribute_hook(string attr_nm, string value,
    string *attrs, int id)
{
    if (sizeof(attrs) && (member_array(value, attrs) < 0))
    {
        /* Invalid selection.  Prompt and ask again */
        craftsman_config_invalid_attribute_value_hook(attr_nm, value);
        return 0;
    }

    return 1;
}

/*
 * Function name: craftsman_config_validate_chosen_attributes_hook
 * Description:   Validate the attributes chosen and give any appropriate
 *                messages.  This differs from 
 *                craftsman_config_validate_chosen_attribute_hook in that
 *                it is only called after all attributes have been set.
 * Arguments:     1. (mapping) the selected attributes
 *                2. (int)     the order id
 * Returns:       1 - values are valid
 *                0 - values are invalid and configuration should be exited
 */
public int
craftsman_config_validate_chosen_attributes_hook(mapping attrs, int id)
{
    return 1;
}

/*
 * Function name: craftsman_desc_order
 * Description:   Redefine this to change how individual orders are presented
 * Arguments:     (int) the order id
 * Returns:       the formatted order description
 */
public string
craftsman_desc_order(int id)
{
    mixed *order = m_orders[id];
    string *attrs, str = "";
    int i;
    mapping attr_map = order[ORDER_SELECTIONS];

    attrs = m_indices(attr_map);
    for (i = 0; i < sizeof(attrs); i++)
    {
        /* Attribute names prepended with a '_' are considered to
         * be hidden.
         */
        if (attrs[i][0] == '_')
        {
            continue;
        }

        str += sprintf("%-13s %s\n", attrs[i] + ":", 
            attr_map[attrs[i]]);
    }

    return str;
}

/*
 * Function name: craftsman_notify_completion_time_hook
 * Description:   Redefine this to change the message given to notify
 *                the user of the order's completion time
 * Arguments:     1. (int) the order's completion time
 *                2. (int) the order id
 */
public void
craftsman_notify_completion_time_hook(int t, int id)
{
    string str;

    if (t < 60)
    {
        str = "less than a minute";
    }
    else
    {
        /* drop seconds */
        t = t / 60;
        str = "about " + t + ((t == 1) ? " minute" : " minutes");
    }

    write("Your order should take " + str + " to complete.\n");
}

/*
 * Function name: craftsman_notify_cost_hook
 * Descriptions:  Redefine this to change the message given to notify the user
 *                of the cost of an item.
 * Arguments:     (int) the order id
 */
public void
craftsman_notify_cost_hook(int id)
{
    int cost;

    if (cost = craftsman_calc_item_cost(0, id))
    {
        write("That will cost " + MONEY_TEXT_SPLIT(cost) + ".\n");
    }
}

/*
 * Function name: craftsman_notify_cancel_hook
 * Description:   Redefine this to change the message given when instructing
 *                the user how to cancel an order.
 * Arguments:     (int) the order id
 */
public void
craftsman_notify_cancel_hook(int id)
{
    write("You may \"cancel order\" at any time.\n");
}

/*
 * Function name: craftsman_notify_check_hook
 * Description:   Redefine this to change the message given when instructing
 *                the user how to check an order
 * Arguments:     (int) the order id
 */
public void
craftsman_notify_check_hook(int id)
{
    write("You may \"check order\" to check on the status " +
        "of your order and to retrieve completed orders.\n");
}

/*
 * Function name: craftsman_notify_hold_time_hook
 * Description:   Redefine this to change the message given to notify the user
 *                of the amount of time that the completed order will be held.
 * Arguments:     1. (int) the hold time in seconds
 *                2. (int) the order id
 */
public void
craftsman_notify_hold_time_hook(int hold, int id)
{
    if (hold > 0)
    {
        write("Your order will only be held for a limited time " +
            "after completed.\n");
    }
}

/*
 * Function name: craftsman_config_completed_hook
 * Description:   Redefine this to change the message given when
 *                the user finishes selecting configuration options.
 * Arguments:     (int) the order id
 */
public void
craftsman_config_completed_hook(int id)
{
    mixed *order = m_orders[id];
    mapping attr_map = order[ORDER_SELECTIONS];

    if (m_sizeof(attr_map))
    {
        write("You have selected the following:\n" +
            craftsman_desc_order(id) + "\n");
    }

    craftsman_notify_completion_time_hook(craftsman_query_time_to_complete(id),
        id);
    craftsman_notify_cost_hook(id);
    craftsman_notify_check_hook(id);
    craftsman_notify_cancel_hook(id);
    craftsman_notify_hold_time_hook(craftsman_query_hold_time(id), id);
}

/*
 * Function name: craftsman_abort_hook
 * Description:   Redefine this to change the message given when the user
 *                aborts from an input prompt.
 */
public void
craftsman_abort_hook()
{
    write("Aborted.\n");
}

/*
 * Function name: craftsman_cancel_syntax_failure_hook
 * Description:   Redefine this to change the message given when incorrect
 *                arguments are given to the "cancel" command.
 * Arguments:     (string str) the arguments given to the "cancel" command.
 * Returns:       0/1
 */
public int 
craftsman_cancel_syntax_failure_hook(string str)
{
    notify_fail("Do you wish to \"cancel order\"?\n");
    return 0;
}

/*
 * Function name: craftsman_cancel_unpurchased_item_hook
 * Description:   Redefine this to change the message given when a user who
 *                has purchased nothing tries to cancel an order.
 * Returns:       0/1
 */
public int
craftsman_cancel_unpurchased_item_hook()
{
    write("You haven't purchased anything.\n");
    return 1;
}

/*
 * Function name: craftsman_cancel_order_hook
 * Description:   Redefine this to change the message given when an order is
 *                cancelled.
 * Arguments:     (int) the order id
 */
public void
craftsman_cancel_order_hook(int id)
{
    write("Ok.\n");
}

/*
 * Function name: craftsman_timeout_hook
 * Description:   Redefine this to change the message given when an order
 *                times out in the selection stage.
 * Arguments:     (int) the order id
 */
public void
craftsman_timeout_hook(int id)
{
    write("Timeout.\n");
}

/*
 * Function name: craftsman_list_orders_hook
 * Description:   Redefine this to change how order lists are presented.  It
 *                is important that the ordering of the orders is preserved.
 * Arguments:     (int *) the ids of the orders to list
 */
public void
craftsman_list_orders_hook(int *ids)
{
    int i;

    for (i = 0; i < sizeof(ids); i++)
    {
        write(sprintf("%2d) %=-70s\n",
            i + 1, craftsman_desc_order(ids[i])));
    }
}

public int
craftsman_query_object_present(object ob)
{
    return ((environment(ob) == this_object()) ||
        (environment(ob) == environment(this_object())));
}

/*
 * Function name: craftsman_completed_order_hook
 * Descriptions:  Redefine this to change the message a user
 *                receives when an order is completed.
 * Arguments:     1. (int)    the order id
 *                2. (string) the owner
 */
public void
craftsman_completed_order_hook(int id, string owner)
{
    object who;

    if ((who = find_player(owner)) && craftsman_query_object_present(who))
    {
        who->catch_tell("Your order has been completed.\n");
    }
}

/*
 * Function name: craftsman_cancel_menu_hook
 * Description:   Redefine this to change the message given to prompt the user
 *                to select an order to cancel.
 * Arguments:     (int *) the ids for the orders belonging to the user
 */ 
public void
craftsman_cancel_menu_hook(int *ids)
{
    craftsman_list_orders_hook(ids);
    write("Select an order to cancel (1 - " + sizeof(ids) + 
        " or ~q to exit): ");
}

/*
 * Function name: craftsman_check_syntax_failure_hook
 * Description:   Redefine this to change the message given when the wrong
 *                arguments are given to the "check" command
 * Arguments:     (string) the argument given to the "check" command
 * Returns:       0/1
 */
public int
craftsman_check_syntax_failure_hook(string str)
{
    notify_fail("Do you wish to \"check order\"?\n");
    return 0;
}

/*
 * Function name: craftsman_check_unpurchased_item_hook
 * Description:   Redefine this to change the message give when a user who has
 *                purchased nothing uses the "check" command.
 * Returns:       0/1
 */
public int
craftsman_check_unpurchased_item_hook()
{
    write("You haven't purchased anything.\n");
    return 1;
}

/*
 * Function name: craftsman_check_complete_order_hook
 * Description:   Redefine this to change the message given when an order is
 *                complete.
 * Arguments:     (int *) the ids of the user's completed orders
 */
public void
craftsman_check_complete_order_hook(int *ids)
{
    write("Your " + ((sizeof(ids) == 1) ? "order is" : "orders are") +
        " complete.\n");
}

/*
 * Function name: craftsman_check_incomplete_order_hook
 * Description:   Redefine this to change how unfinished orders are presented.
 * Arguments:     (int *) the ids of the user's incomplete orders
 */
public void
craftsman_check_incomplete_order_hook(int *ids)
{
    if (!sizeof(ids))
    {
        write("You have no other orders.\n");
        return;
    }

    write("You still have " + LANG_WNUM(sizeof(ids)) +
        " " + ((sizeof(ids) > 1) ? "orders" : "order") + " being made:\n");

    craftsman_list_orders_hook(ids);
}

/*
 * Function name: craftsman_check_cannot_pay_hook
 * Description:   Redefine this to change the message given when
 *                the user cannot pay for an item.
 * Arguments:     (int)    the cost of the item in cc
 *                (object) the item itself
 *                (int)    the order id
 */
public void
craftsman_check_cannot_pay_hook(int cost, object ob, int id)
{
    write("You can't afford it!\n");
}

/*
 * Function name: check_move_player_failed_hook
 * Description:   Redefine this to change the message when the item could not
 *                be moved to the user.
 * Arguments:     (object) the item
 */
public void
craftsman_check_move_player_failed_hook(object ob)
{
    write("You can't carry the " + ob->short() + "!\n");
}

/*
 * Function name: craftsman_check_move_env_failed_hook
 * Description:   Redefine this to change the message when the item could not
 *                be moved to the user AND it could not be moved to the user's
 *                environment.
 * Arguments:     (object) the item
 */
public void
craftsman_check_move_env_failed_hook(object ob)
{
    write("You can't carry the " + ob->short() + "!\n");
}

/*
 * Function name: craftsman_check_receive_order_hook
 * Description:   Redefine this to change the message when the user receives
 *                his item.
 * Arguments:     1. (object) the item
 *                2. (int)    the order id
 */
public void
craftsman_check_receive_order_hook(object ob, int id)
{
    write("You receive your " + ob->short() + ".\n");
}

/*
 * Function name: craftsman_query_orders_by_status
 * Description:   Get all orders matching a particular status or, optionally,
 *                all of a specified user's orders matching a particular
 *                status.
 * Arguments:     1. (int)    the status
 *                2. (string) optional user name
 * Returns:       An array of order ids which match the status (and user)
 */
public varargs int *
craftsman_query_orders_by_status(int ostatus, string name)
{
    int *ids;

    if (!name)
    {
        ids = m_indices(m_orders);
    }
    else
    {
        // Get the ids of the entries which match the owner name
        ids = m_indices(filter(m_orders,
            &operator(==)(name) @ &operator([])(, ORDER_OWNER)));
    }

    // Get the ids of the entries with match the order status
    return filter(ids, &operator(==)(ostatus) @
        &operator([])(, ORDER_STATUS) @ &operator([])(m_orders));
}

/*
 * Function name: craftsman_query_orders_in_progress
 * Description:   Get all orders in progress or all of a particular user's
 *                orders in progress
 * Arguments:     (string) optional user name
 * Returns:       An array of in-progress order ids
 */
public varargs int *
craftsman_query_orders_in_progress(string name)
{
    return craftsman_query_orders_by_status(STATUS_IN_PROGRESS, name);
}

/*
 * Function name: craftsman_query_orders_configured
 * Description:   Get all configured orders or all of a particular user's
 *                configured orders
 * Arguments:     (string) optional user name
 * Returns:       An array of configured order ids
 */
public varargs int *
craftsman_query_orders_configured(string name)
{
    return craftsman_query_orders_by_status(STATUS_CONFIGURED, name);
}

/*
 * Function name: craftsman_query_orders_completed
 * Description:   Get all completed orders or all of a particular user's
 *                completed orders
 * Arguments:     (string) optional user name
 * Returns:       An array of completed order ids
 */
public varargs int *
craftsman_query_orders_completed(string name)
{
    return craftsman_query_orders_by_status(STATUS_COMPLETED, name);
}

/*
 * Function name: craftsman_set_attribute
 * Description:   Called to set the desired value for each attribute,
 *                depending on the user's input.
 * Arguments:     1. (string)  the value specified for the current attribute
 *                2. (int)     the order id
 *                3. (int)     the attribute number of the current attribute
 * Returns:       1/0 - attribute set/not set
 */
public int
craftsman_set_attribute(string value, int id, int attr_number)
{
    mixed *order = m_orders[id];
    mixed attrs;
    string attr_nm;
    int valid;

    if (value == "~q")
    {
        craftsman_abort_hook();
        craftsman_cancel_order(id);
        return 0;
    }

    if (!sizeof(order))
    {
        return 0;
    }

    attr_nm = order[ORDER_ATTRS][attr_number][0];
    attrs = order[ORDER_ATTRS][attr_number][1];

    if (functionp(attrs))
    {
        attrs = attrs(id, attr_nm, order[ORDER_SELECTIONS]);
    }

    if (pointerp(attrs) && (sizeof(attrs) > 1))
    {
        /* Check if the input value is invalid for some other reason */
        valid = craftsman_config_validate_chosen_attribute_hook(attr_nm, value,
            attrs, id);
    
        if (valid < 0)
        {
            /* Value is invalid, and we should stop */
            return 0;
        }
        
        if (valid == 0)
        {
            /* Value is invalid, and we should try again */
            craftsman_config_attributes(id, attr_number);
            return 0;
        }
    }

    /* Add the attribute and value to the mapping */        
    order[ORDER_SELECTIONS][attr_nm] = value;
    
    if (++attr_number < sizeof(order[ORDER_ATTRS]))
    {
        /* There are still attributes that are unset, so get the next */
        craftsman_config_attributes(id, attr_number);
        return 1;
    }

    /* There are no more attributes to be set, so finish up */
    craftsman_completed_config(id);
    return 1;
}

/*
 * Function name: craftsman_config_attributes
 * Description:   Prompt the user to input a value for an attribute.
 * Arguments:     1. (int) the order id
 *                2. (int) the attribute number of the current attribute
 */
public void
craftsman_config_attributes(int id, int attr_number)
{
    mixed *order = m_orders[id];

    if (attr_number < sizeof(order[ORDER_ATTRS]))
    {
        /* The first attribute in the array is the one we will set */
        string attr_nm = order[ORDER_ATTRS][attr_number][0];
        mixed attrs = order[ORDER_ATTRS][attr_number][1];

        if (functionp(attrs))
        {
            attrs = attrs(attr_nm, order[ORDER_SELECTIONS]);
        }

        /* prompt the user for input */
        craftsman_config_prompt_attribute_hook(attr_nm, attrs);
    
        /* catch the player's input and set the attribute */

#ifdef INPUT_OBJ
        {
            object input_obj;

            setuid();
            seteuid(getuid());
            input_obj = clone_object(INPUT_OBJ);
            input_obj->get_input(&craftsman_set_attribute(, id, attr_number));
            
            order[ORDER_INPUT] = input_obj;
        }
#else
        input_to(&craftsman_set_attribute(, id, attr_number));
#endif
    }
}

/*
 * Function name: craftsman_completed_config
 * Description:   Called when the user has finished selecting attributes, this
 *                function adds the item to the list of items being created.
 * Arguments:     (int) the order id
 * Returns:       1/0 - configuration successful/unsuccessful
 */
public int
craftsman_completed_config(int id)
{
    int t;
    mixed *order = m_orders[id];
    mapping attr_map = order[ORDER_SELECTIONS];

    order[ORDER_STATUS] = STATUS_CONFIGURED;
    
    if (order[ORDER_TIMEOUT])
    {
        remove_alarm(order[ORDER_TIMEOUT]);
        order[ORDER_TIMEOUT] = 0;
    }

    /* Validate the chosen values */
    if (!craftsman_config_validate_chosen_attributes_hook(attr_map, id))
    {
        craftsman_cancel_order(id);
        return 0;
    }

    craftsman_config_completed_hook(id);

    t = order[ORDER_COMPLETION_TIME] = craftsman_query_completion_time(id);

    /* Start creation */

    if (!next_completion_time || (t < next_completion_time))
    {
        if (completion_alarm)
        {
            remove_alarm(completion_alarm);
        }

        next_completion_time = t;
        completion_alarm = set_alarm(itof(t - time()), 0.0, 
            &craftsman_completed_order(id));
    }

    /* This is a good point to clean up old completed orders */
    craftsman_clean_completed_orders();

    return 1;
}

/* 
 * Function name: craftsman_calc_item_cost
 * Descriptions:  Give the cost in cc for an item.  By default, this simply
 *                returns the value given in set_item_cost(), but it can be
 *                redefined to calculate the cost for an item dynamically.
 * Arguments:     1. (object) the item itself.  If the item has not been
 *                            completed this will be 0.
 *                2. (int)    the order id
 * Returns:       (int) the cost of the item in copper
 */
public int
craftsman_calc_item_cost(object ob, int id)
{
    return m_orders[id][ORDER_COST];
}

/*
 * Function name: craftsman_completed_order
 * Description:   Called when creation of an item is complete, this function
 *                adds the item to the list of completed items and starts
 *                creation of the next item.
 * Arguments:     (int) the order id
 */
public void
craftsman_completed_order(int id)
{
    mixed *order = m_orders[id];

    order[ORDER_STATUS] = STATUS_COMPLETED;

    craftsman_completed_order_hook(id, order[ORDER_OWNER]);

    /* Start the alarm for the next item */
    craftsman_reset_completion_time();
}

/*
 * Function name: craftsman_timeout_selection
 * Description:   Cause the attribute selection process to time out.  The
 *                user will have to start over.
 * Arguments:     (int) the order id
 */   
public void
craftsman_timeout_selection(int id)
{
    mixed *order = m_orders[id];

    if (sizeof(order) && (order[ORDER_STATUS] == STATUS_IN_PROGRESS))
    {
        craftsman_cancel_order(id);
        craftsman_timeout_hook(id);
    }
}

/*
 * Function name: purchase
 * Description:   the "purchase" command.  Starts off item configuration.
 * Arguments:     (string) argument string to the "purchase" command
 * Returns:       1/0 - success/failure
 */ 
public int
craftsman_purchase(string str)
{
    int num_orders, id;
    string name;

    if (member_array(str, item_names) < 0)
    {
        return craftsman_purchase_syntax_failure_hook(str);
    }

    num_orders = sizeof(craftsman_query_orders_in_progress()) +
                 sizeof(craftsman_query_orders_configured());

    if ((max_total_orders >= 0) && (num_orders >= max_total_orders))
    {
        return craftsman_purchase_exceeded_max_total_orders_hook(str);
    }

    name = this_player()->query_real_name();

    num_orders = sizeof(craftsman_query_orders_in_progress(name)) +
                 sizeof(craftsman_query_orders_configured(name));

    if ((max_user_orders >= 0) && (num_orders >= max_user_orders))
    {
        return craftsman_purchase_exceeded_max_user_orders_hook(str);
    }

    id = craftsman_new_order(name);

    craftsman_purchase_hook(str);

    craftsman_configure_order(id, str);

    if (selection_timeout > 0)
    {
        m_orders[id][ORDER_TIMEOUT] = set_alarm(itof(selection_timeout), 0.0,
            &craftsman_timeout_selection(id));
    }

    craftsman_config_attributes(id, 0);

    return 1;
}

/*
 * Function name: reset_completion_time
 * Description:   Start a new alarm to signal the completion of the next item.
 */
public void
craftsman_reset_completion_time()
{
    int *ids, id, i;
    mixed *order;

    if (get_alarm(completion_alarm))
    {
        remove_alarm(completion_alarm);
    }

    next_completion_time = 0;

    ids = m_indices(m_orders);
    for (i = 0; i < sizeof(ids); i++)
    {
        order = m_orders[ids[i]];

        if (order[ORDER_STATUS] != STATUS_CONFIGURED)
        {
            continue;
        }

        if (!next_completion_time || 
            (order[ORDER_COMPLETION_TIME] < next_completion_time))
        {
            next_completion_time = order[ORDER_COMPLETION_TIME];
            id = ids[i];
         }
    }

    if (next_completion_time)
    {
        set_alarm(itof(next_completion_time - time()),
            0.0, &craftsman_completed_order(id));
    }
}

/*
 * Function name: craftsman_cancel_order
 * Description:   This function cancels the given order
 * Arguments:     (int) the order id
 */
public void
craftsman_cancel_order(int id)
{
    mixed *order = m_orders[id];

    m_orders = m_delete(m_orders, id);

    if (order[ORDER_TIMEOUT])
    {
        remove_alarm(order[ORDER_TIMEOUT]);
    }

#ifdef INPUT_OBJ
    if (order[ORDER_INPUT])
    {
        order[ORDER_INPUT]->remove_object();
    }
#endif

    craftsman_reset_completion_time();
}

/*
 * Function name: cancel_order_input
 * Description:   Accept user input and cancel the specified order
 * Arguments:     (string) the order number (position) to cancel
 * Returns:       1/0 - order cancelled/not cancelled
 */
public int
cancel_order_input(string str)
{
    int i, *ids;
    string name;

    if (str == "~q")
    {
        craftsman_abort_hook();
        return 0;
    }

    name = this_player()->query_real_name();
    ids = sort_array(craftsman_query_orders_configured(name));
    
    if (!sscanf(str, "%d", i) || (i < 1) || (i > sizeof(ids)))
    {
        craftsman_cancel_menu_hook(ids);
        input_to(cancel_order_input);
        return 0;
    }
    
    craftsman_cancel_order(ids[i - 1]);
    craftsman_cancel_order_hook(ids[i - 1]);

    return 1;
}

/*
 * Function name: craftsman_cancel
 * Description:   The "cancel" command.  This function cancels the user's
 *                order or prompts the user to select an order to cancil if
 *                he has more than one being made.
 * Arguments:     (string) the arguments to the "cancel" command
 * Returns:       1/0 - success/failure
 */
public int
craftsman_cancel(string str)
{
    int *ids;
    string name;

    if (str != "order")
    {
        return craftsman_cancel_syntax_failure_hook(str);
    }

    name = this_player()->query_real_name();

    if (!sizeof(ids = craftsman_query_orders_configured(name)))
    {
        return craftsman_cancel_unpurchased_item_hook();
    }

    if (sizeof(ids) == 1)
    {
        craftsman_cancel_order(ids[0]);
        craftsman_cancel_order_hook(ids[0]);
        return 1;
    } 

    craftsman_cancel_menu_hook(ids);
    input_to(cancel_order_input);
    return 1;
}

/*
 * Function name: craftsman_configure_item
 * Description:   Redefine this to configure your final item
 * Arguments:     1. (object)  the item to be configured
 *                2. (mapping) a mapping that maps set attribute names to the
 *                             values specified by the user.
 *                3. (int)     the order id
 */
void craftsman_configure_item(object item, mapping attrs, int id)
{
}

/*
 * Function name: craftsman_clone_item
 * Description:   clone the item to be configured.
 * Arguments:     1. (string) the file to clone
 *                2. (int)    the order id
 * Returns:       An instance of the item to be sold.
 */
public object
craftsman_clone_item(string file, int id)
{
    setuid();
    seteuid(getuid());
    return clone_object(file);
}

/*
 * Function name: craftsman_move_item
 * Description:   move the purchased item to the user
 * Arguments:     (object) the item to be moved
 * Returns:       1 - successfully moved to player or player's environment. 
 *                0 - could not be moved to player or player's environment.
 */
public int
craftsman_move_item(object ob)
{
    if (ob->move(this_player()))
    {
        if (ob->move(environment(this_player())))
        {
            craftsman_check_move_env_failed_hook(ob);
            ob->remove_object();
            return 0;
        }

        craftsman_check_move_player_failed_hook(ob);
        return 1;
    }

    return 1;
}  

/* 
 * Function name: craftsman_charge_for_item
 * Description:   Charge for the ordered item
 * Arguments:     1. (object) the item to be sold
 *                2. (int)    the order id
 * Returns:       1/0 - item was paid for/was not paid for
 */ 
public int
craftsman_charge_for_item(object ob, int id)
{
    int cost = craftsman_calc_item_cost(ob, id);
    if (!MONEY_ADD(this_player(), -cost))
    {
        craftsman_check_cannot_pay_hook(cost, ob, id);
        ob->remove_object();
        return 0;
    }
 
    return 1;
}
      
/*
 * Function name: craftsman_present_order
 * Description:   Clone a fresh item, configure it, and give it to the user
 * Arguments:     (int) the order id
 * Returns:       1 - purchase was resolved and can be removed
 *                0 - purchase was not resolved
 */
public int
craftsman_present_order(int id)
{
    object ob;
    mixed *order = m_orders[id];

    setuid();
    seteuid(getuid());

    ob = craftsman_clone_item(order[ORDER_FILE], id);
    craftsman_configure_item(ob, order[ORDER_SELECTIONS], id);

    if (!craftsman_charge_for_item(ob, id))
    {
        return 0;
    }

    if (!craftsman_move_item(ob))
    {
        return 0;
    }

    craftsman_check_receive_order_hook(ob, id);
    return 1;
}

/*
 * Function name: craftsman_check
 * Description:   The "check" command.  Tell the user about the
 *                status of his orders.  If any are complete, let him
 *                have them.
 * Arguments:     (string) the arguments give to the "check" command
 * Returns:       1/0 - success/failure
 */
public int
craftsman_check(string str)
{
    string name;
    int *incomplete_orders, *complete_orders;
    int i;

    if (str != "order")
    {
        return craftsman_check_syntax_failure_hook(str);
    } 

    name = this_player()->query_real_name();
    incomplete_orders = craftsman_query_orders_configured(name);
    complete_orders   = craftsman_query_orders_completed(name);

    if (!sizeof(incomplete_orders) && !sizeof(complete_orders))
    {
        return craftsman_check_unpurchased_item_hook();
    }

    if (sizeof(complete_orders))
    {
        craftsman_check_complete_order_hook(complete_orders);

        for (i = 0; i < sizeof(complete_orders); i++)
        {
            if (craftsman_present_order(complete_orders[i]))
            {
                craftsman_cancel_order(complete_orders[i]);
            }
        }
       
        if (sizeof(incomplete_orders))
        {
            craftsman_check_incomplete_order_hook(incomplete_orders);
        }

        return 1;
    }

    craftsman_check_incomplete_order_hook(incomplete_orders);

    return 1;
}

/*
 * Function name: craftsman_clean_completed_orders
 * Description:   Remove old orders that have not been picked up
 */
public void
craftsman_clean_completed_orders()
{
    int i, t, *ids, hold;
    mixed *order;

    ids = craftsman_query_orders_completed();
    for (i = 0; i < sizeof(ids); i++)
    {
        order = m_orders[ids[i]];

        t = time();
        hold = craftsman_query_hold_time(ids[i]);
        // Remove orders which have been held long enough
        if ((hold > 0) && ((order[ORDER_COMPLETION_TIME] + hold) < t))
        {
            craftsman_cancel_order(ids[i]);
        }
    }
}

/*
 * Function name: craftsman_init
 * Description:   adds the craftsman's commands.  Call this from the
 *                init()/init_living().
 */
public void
craftsman_init()
{
    add_action(craftsman_purchase, "purchase");
    add_action(craftsman_purchase, "buy");
    add_action(craftsman_purchase, "order");
    add_action(craftsman_cancel,   "cancel");
    add_action(craftsman_check,    "check");
}

/*
 * Function name: craftsman_set_sold_item_names
 * Description:   give the names of the item which the craftsman sells.
 *                These will be used to determine correct syntax for the
 *                "purchase" command.
 * Arguments:     (string *) names of items that can be purchased
 */
public void
craftsman_set_sold_item_names(string *names)
{
    item_names = names;
}    

public string *
craftsman_query_sold_item_names()
{
    return item_names + ({});
}

/*
 * Function name: craftsman_set_selection_timeout
 * Description:   Indicate that the attribute selection process should time
 *                out in the specified number of seconds.  0 timeout indicates
 *                no timeout at all.
 * Arguments:     (int) timeout delay in seconds
 */
public void
craftsman_set_selection_timeout(int seconds)
{
    selection_timeout = seconds;
}

/*
 * Function name: craftsman_set_max_total_orders
 * Description:   Set the max number of orders that can be active at once
 * Arguments:     (int) the max number of orders
 */
public void
craftsman_set_max_total_orders(int i)
{
    max_total_orders = i;
}

/*
 * Function name: craftsman_set_max_user_orders
 * Description:   Set the max number of orders that can be active at once for
 *                a single order.
 * Arguments:     (int) the max number of orders
 */
public void
craftsman_set_max_user_orders(int i)
{
    max_user_orders = i;
} 

/*
 * Function name: craftsman_set_item_cost
 * Description:   Set the cost of an item
 * Arguments:     1. (int) the order id
 *                2. (int) the cost
 */
public void
craftsman_set_item_cost(int id, int cost)
{
    m_orders[id][ORDER_COST] = cost;
}

public int
craftsman_query_item_cost(int id)
{
    return m_orders[id][ORDER_COST];
}

/*
 * Function name: craftsman_set_hold_time
 * Description:   Set the minimum amount of time that the craftsman will hold a
 *                completed order.  If set to a negative value, orders will
 *                be held indefinitely.
 * Arguments:     1. (int) the order id
 *                2. (int) hold time in seconds
 */
public void
craftsman_set_hold_time(int id, int i)
{
    m_orders[id][ORDER_HOLD] = i;
}

public int
craftsman_query_hold_time(int id)
{
    return m_orders[id][ORDER_HOLD];
}
    
/*
 * Function name: craftsman_add_attribute
 * Description:   add a configuration option.  The user will be prompted to
 *                select a value from the given values.
 * Arguments:     1. (int)    the order id
 *                2. (string) The name of the attribute.
 *                3. (mixed)  Possible values for the attribute. 
 */
public int
craftsman_add_attribute(int id, string attr_name, mixed attrs)
{
    mixed *order = m_orders[id];
    mixed *old_attrs = order[ORDER_ATTRS];

    /* A mapping might seem more appropriate, but it's desirable to maintain
     * the order of the attributes.
     */
    old_attrs += ({ ({ attr_name, attrs }) });
    order[ORDER_ATTRS] = old_attrs;
    return sizeof(old_attrs) - 1;
}

public void
craftsman_remove_attribute(int id, string attribute)
{
    mixed *order = m_orders[id];
    order[ORDER_ATTRS] = filter(order[ORDER_ATTRS],
        &operator(!=)(attribute) @ &operator([])(, 0));
}

public void
craftsman_clear_attributes(int id)
{
    m_orders[id][ORDER_ATTRS] = ({});
}


public void
craftsman_add_selection(int id, string attr_name, mixed selection)
{
    m_orders[id][ORDER_SELECTIONS][attr_name] = selection;
}

/*
 * Function name: craftsman_set_item_file
 * Description:   set the pathname to the item which will be created
 * Arguments:     1. (int)    the order id
 *                2. (string) the filename
 */
public void
craftsman_set_item_file(int id, string f)
{
    m_orders[id][ORDER_FILE] = f;
}

/*
 * Function name: craftsman_query_item_file
 * Description:   Get the item file for the given order
 * Arguments:     (int) the order id
 * Returns:       The item file
 */
public string
craftsman_query_item_file(int id)
{
    return m_orders[id][ORDER_FILE];
}

/*
 * Function name: craftsman_set_time_to_complete
 * Description:   Give the amount of time in seconds it takes to create
 *                an item.
 * Arguments:     1. (int) the order id
 *                2. (int) completion time in seconds
 */
public void
craftsman_set_time_to_complete(int id, int t)
{
    m_orders[id][ORDER_TIME] = t;
}

/*
 * Function name: craftsman_query_time_to_complete
 * Description:   Get the amount of time in seconds required to create a
 *                given order.
 * Arguments:     (int) the order id
 * Returns:       the completion time
 */
public int
craftsman_query_time_to_complete(int id)
{
    return m_orders[id][ORDER_TIME];
}

public int
craftsman_query_completion_time(int id)
{
    return time() + craftsman_query_time_to_complete(id);
}

public int
craftsman_query_status(int id)
{
    return m_orders[id][ORDER_STATUS];
}

public string
craftsman_query_owner(int id)
{
    return m_orders[id][ORDER_OWNER];
}

public mixed *
craftsman_query_attributes(int id)
{
    return m_orders[id][ORDER_ATTRS] + ({});
}

public string
craftsman_query_attribute_name(int id, int attr_number)
{
    return m_orders[id][ORDER_ATTRS][attr_number][0];
}

public string
craftsman_query_attribute_values(int id, int attr_number)
{
    return m_orders[id][ORDER_ATTRS][attr_number][1];
}

public string
craftsman_query_attribute_selection(int id, int attr_number)
{
    return m_orders[id][ORDER_SELECTIONS][craftsman_query_attribute_name(id, attr_number)];
}

public mapping
craftsman_query_selections(int id)
{
    return m_orders[id][ORDER_SELECTIONS] + ([]);
}

/*
 * Function name: craftsman_new_order
 * Description:   Add a new order and add default values
 * Arguments:     (string) the user
 * Returns:       the order id
 */
public int
craftsman_new_order(string owner)
{
    mixed *order = allocate(ORDER_SIZE);
    int id;

    // Default values
    order[ORDER_STATUS] = STATUS_IN_PROGRESS;
    order[ORDER_OWNER]  = owner;
    order[ORDER_TIME]   = 30;
    order[ORDER_HOLD]   = 900;
    order[ORDER_ATTRS]  = ({});
    order[ORDER_SELECTIONS]  = ([]);

    id = (m_sizeof(m_orders) ? applyv(max, m_indices(m_orders)) + 1 : 1);
    m_orders[id] = order;

    return id;
}

/*
 * Function name: craftsman_configure_order
 * Description:   Use this function to configure a new order.
 * Arguments:     1. (int)    the order id
 *                2. (string) arguments to the "purchase" command
 */
public void
craftsman_configure_order(int id, string str)
{
}

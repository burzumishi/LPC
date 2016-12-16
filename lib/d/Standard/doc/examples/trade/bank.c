/*
 * A simple example of a standard bank.  This does not include a deposit.
 */

inherit "/std/room";
inherit "/lib/bank";

#include <stdproperties.h>

void
create_room()
{
    set_short("The Local Bank Office");
    set_long(
        "You are in the local bank office. Here you can change money or " +
       "minimize your coins. There is a sign with instructions on here.\n");

    /* Add instructions for using the bank */
    add_item( ({ "sign", "instructions" }), standard_bank_sign);
    add_cmd_item( ({ "sign", "instructions" }), "read", standard_bank_sign);

    add_exit("pub", "west");

    add_prop(ROOM_I_INSIDE, 1);

    /* Set the exchange fee (% of changed amount)
     *
     * IMPORTANT
     * This needs to be called *before* config_trade_data() is called
     * or else the fee will not be taken out.
     */
    set_bank_fee(30); 

    /* Initialize the bank */
    config_trade_data();
}

void
init()
{
    ::init();
    bank_init(); /* Add the bank commands */
}


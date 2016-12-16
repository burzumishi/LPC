/*
 * /secure/donation.c
 *
 * Genesis is a toroid. Holy is the donut! This object keeps track of those
 * people who contributed to Genesis, making it possible to purchase a machine
 * dedicated to running Genesis.
 *
 * Author: Mercade
 * Date  : August 4, 1998
 */

#pragma no_clone
#pragma no_inherit
#pragma strict_types

#include "/d/Genesis/obj/donation/donation.h"
#include <composite.h>
#include <macros.h>
#include <mail.h>
#include <std.h>

/*
 * Global variables. These variables will be saved into the save file.
 *
 * mapping donors_1998
 *     ([ player name (string) : donation in dollars (int) ])
 * mapping donors_2003
 *     ([ player name (string) : donation in euros (int) ])
 *
 * mapping seconds
 *     ([ player name (string) : main player name (string) ])
 */
private mapping donors_1998;
private mapping donors_2003;
private mapping seconds;

/*
 * Global variable. This variables is not saved.
 */
private static int total_donations;

/*
 * Function name: create
 * Description  : Constructor. Initialises the save file if not present.
 */
nomask void
create()
{
    setuid();
    seteuid(getuid());

    restore_object(DONATION_SAVE);

    if (!mappingp(donors_1998))
    {
        donors_1998 = ([ ]);
    }
    if (!mappingp(donors_2003))
    {
        donors_2003 = ([ ]);
    }
    if (!mappingp(seconds))
    {
        seconds = ([ ]);
    }

    total_donations = reduce( &operator(+)(,) , m_values(donors_2003) );
}

/*
 * Function name: short
 * Description  : Returns the short description for this object.
 * Returns      : string - the short description.
 */
nomask string
short()
{
    return "the gold coin in the centre of the universe";
}

/*
 * Function name: save_master
 * Description  : This function saves the donation master object.
 */
nomask void
save_master()
{
    setuid();
    seteuid(getuid());

    save_object(DONATION_SAVE);
}

/*
 * Function name: eur_to_usd_2003
 * Description  : Conversion routine necessary to compare the 2003 euros to
 *                the 1998 us dollars. Rate used is 1 usd = 0.9 eur.
 * Arguments    : int euro - the amount in EUR 2003 style.
 * Returns      : int - the amount in USD 2003 style.
 */
int
eur_to_usd_2003(int euro)
{
   return (((euro * 111) + 55) / 100);
}

/*
 * Function name: log_donation
 * Description  : Logs the donation action into the central log file. The
 *                actor of this is this_interactive().
 * Arguments    : string name - the person subject to the action.
 *                string str  - a string describing the action.
 */
nomask static void
log_donation(string name, string str)
{
    string actor = this_interactive()->query_real_name();

    setuid();
    seteuid(getuid());

    write_file(DONATION_LOG, sprintf("%s %-11s > %-11s %s\n",
        ctime(time()), (strlen(actor) ? capitalize(actor) : "unknown!"),
        capitalize(name), str));
}

/*
 * Function name: query_total_donations
 * Description  : Will return the total amount of money collected in donations.
 * Returns      : int - the total amount of donations.
 */
nomask int
query_total_donations()
{
    return total_donations;
}

/*
 * Function name: query_donated
 * Description  : Query whether a particular player has made a donation.
 * Arguments    : string name - the lower case name of the player.
 * Returns      : int - 1/0 - donated or not.
 */
nomask int
query_donated(string name)
{
    if (seconds[name])
    {
        name = seconds[name];
    }

    return !!(donors_1998[name] || donors_2003[name]);
}

/*
 * Function name: query_donors
 * Description  : Returns all donors matching a particular mask. When
 *                omitted, return all donors.
 * Arguments    : string mask - the mask to match.
 * Returns      : string * - the names of the donors matching the mask.
 */
nomask string *
query_donors(string mask)
{
    string *names = ({ });

    /* Access failure. Return all names by default. */
    if (!strlen(mask))
    {
        mask = "*";
    }

    /* Get the names of the donors and the seconds. */
    names = m_indices(donors_1998) + m_indices(donors_2003) +
        m_indices(seconds);

    /* Sort and filter the names. */
    return filter(sort_array(names), &wildmatch(mask, ) );
}

/*
 * Function name: query_brooch_metal_type
 * Description  : Will return the metal type of the toroid brooch the player
 *                is entitled to.
 * Arguments    : string name - the name of the player.
 * Returns      : string - the type of the metal if present, else 0.
 */
nomask string
query_brooch_metal_type(string name)
{
    int amount;
    int usd_2003;

    /* Resolve for seconds. */
    name = lower_case(name);
    if (strlen(seconds[name]))
    {
        name = seconds[name];
    }

    /* Use the 1998 amount plus any excess of 2003. If there is no 1998,
     * use 2003.
     */
    usd_2003 = eur_to_usd_2003(donors_2003[name]);
    if (amount = donors_1998[name])
    {
        amount += ((usd_2003 > DONATION_LIMIT_PLATINUM) ? (usd_2003 - DONATION_LIMIT_PLATINUM) : 0);
    }
    else
    {
        amount = usd_2003;
    }

    if (amount >= DONATION_LIMIT_PLATINUM)
    {
        return "platinum";
    }
    if (amount >= DONATION_LIMIT_GOLD)
    {
        return "gold";
    }
    if (amount >= DONATION_LIMIT_SILVER)
    {
        return "silver";
    }
    if (amount >= DONATION_LIMIT_BRONZE)
    {
        return "bronze";
    }

    return 0;
}

/*
 * Function name: query_topping_metal_type
 * Description  : Will return the metal type of the topping on the toroid
 *                brooch the player is entitled to.
 * Arguments    : string name - the name of the player.
 * Returns      : string - the type of the metal if present, else 0.
 */
nomask string
query_topping_metal_type(string name)
{
    int amount;
    int usd_2003;

    /* Resolve for seconds. */
    name = lower_case(name);
    if (strlen(seconds[name]))
    {
        name = seconds[name];
    }

    /* Use any excess from the combined donations. */
    usd_2003 = eur_to_usd_2003(donors_2003[name]);
    if (amount = donors_1998[name])
    {
        amount = usd_2003 +
            ((donors_1998[name] > DONATION_LIMIT_PLATINUM) ? (donors_1998[name] - DONATION_LIMIT_PLATINUM) : 0);
    }
    else if (usd_2003 > DONATION_LIMIT_PLATINUM)
    {
        amount = (usd_2003 - DONATION_LIMIT_PLATINUM);
    }
    else
    {
        amount = 0;
    }

    if (amount >= DONATION_LIMIT_PLATINUM)
    {
        return "platinum";
    }
    if (amount >= DONATION_LIMIT_GOLD)
    {
        return "gold";
    }
    if (amount >= DONATION_LIMIT_SILVER)
    {
        return "silver";
    }
    if (amount >= DONATION_LIMIT_BRONZE)
    {
        return "bronze";
    }

    return 0;
}

/*
 * Function name: query_restore_brooch
 * Description  : Called from domain_link.c to find out whether a player is
 *                eligible for a brooch restoration.
 * Arguments    : object player - the player to test.
 * Returns      : int 1/0 - if true, the player is eligible for restoration.
 */
nomask public int
query_restore_brooch(object player)
{
    string name = player->query_real_name();

    /* Returns true if the person has no brooch on him, and has made a
     * donation in either 1998 or 2003.
     */
    return (!present(DONOR_BROOCH_ID, player) &&
        (donors_1998[name] || donors_2003[name]));
}

/*
 * Function name: restore_brooch
 * Description  : Called from domain_link.c to actually restore the brooch to
 *                the player (but test eligibility first).
 * Arguments    : object player - the player to give the brooch to.
 * Returns      : int 1/0 - if true, the player got the brooch.
 */
nomask public int
restore_brooch(object player)
{
    /* Test to be sure that the player is entitled. */
    if (query_restore_brooch(player))
    {
        /* This returns the negation of the result from the move(). */
        return !(clone_object(DONATION_BROOCH)->move(player, 1));
    }
    return 0;
}

/*
 * Function name: reward_player
 * Description  : When the donation from a player has been received, reward
 *                his or her generosity. This means that the player is mailed
 *                a confirmation and that the toroid brooch is issued.
 * Arguments    : string name - the name of the player who donated. (Must be
 *                     in lower case).
 */
nomask static void
reward_player(string name)
{
    string  brooch_metal = query_brooch_metal_type(name);
    string  topping_metal = query_topping_metal_type(name);
    int     amount;
    object  player;
    object  brooch;
    mapping playerfile;

    /* No rewards for anonymous. */
    if (name == "anonymous")
    {
        return;
    }

    amount = (stringp(seconds[name]) ? donors_2003[seconds[name]] : donors_2003[name]);

    /* Mail the confirmation. Subject, author, to, cc, body. */
    CREATE_MAIL("Confirmation of donation", "donation", name, "",
        "Dear " + capitalize(name) + ",\n\nWhen the continued existance " +
        "of Genesis was at stake, the bravest forces of\nthe realms " +
        "gathered and to gain a stunning victory! The lands of Genesis\n" +
        "shall remain open to travellers and explorers who shall fight " +
        "their own\nbattles within.\n\nIt is our pleasure to confirm that " +
        "we have received your contribution to\nthe Great Battle by the " +
        "amount of EUR " + amount + ".\n\nThe mightiest wizards of Genesis " +
        "have the pleasure of presenting to you\na token of their " +
        "appreciation, a toroid shaped brooch, made of the finest\n" +
        brooch_metal +
        (strlen(topping_metal) ? ", and with a delicate " + topping_metal + " topping" : "") +
        ", in recognition of your generous contribution to the Great " +
        "Battle.\nThe brooch should already be in your possession. Please " +
        "examine \"broochhelp\"\n for details on the features of the jewel. " +
        "For any questions or comments,\nplease send mail to 'Donation'.\n\n" +
        "Thank you,\n\nThe Genesis contribution collection team.");

    /* Issue the brooch, either directly to the player when he is active, or
     * store it into the playerfile.
     */
    if (objectp(player = find_player(name)))
    {
        brooch = clone_object(DONATION_BROOCH);
        brooch->move(player, 1);
        brooch->init_arg("first");
    }
    else
    {
        playerfile = restore_map(PLAYER_FILE(name));
        playerfile["auto_load"] += ({ DONATION_BROOCH + ":first" });
        save_map(playerfile, PLAYER_FILE(name));
    }
}

/*
 * Function name: add_second
 * Description  : When a donation is contributed to more characters of the same
 *                person, this function registers that.
 * Arguments    : string name   - the name of the main character.
 *                string second - the name of the second to register.
 *                     (Must all be in lower case)
 * Returns      : int 1/0 - success/failure.
 */
nomask int
add_second(string name, string second)
{
    int index;

    /* Illegal call. May only be called from treasury office. */
    if (file_name(previous_object()) != DONATION_OFFICE)
    {
        write("Illegal call! May only be called from treasury office.\n");
        return 0;
    }

    /* Only mark seconds to players who have donated. */
    if (!(donors_1998[name] || donors_2003[name]))
    {
        write("No donation has been registerd to player " +
            capitalize(name) + ".\n");
        return 0;
    }

    if (donors_1998[second] || donors_2003[second])
    {
        write("For player " + capitalize(second) + " a donation has " +
            "been registered. Cannot mark him/her as second.\n");
        return 0;
    }

    if (seconds[second])
    {
        write("Player " + capitalize(second) + " is already marked as " +
            "second of " + capitalize(seconds[second]) +
            ". Cannot mark him/her as second again.\n");
        return 0;
    }

    seconds[second] = name;
    log_donation(name, "Added second " + capitalize(second) + ".");
    write("Marked " + capitalize(second) + " as second of " +
        capitalize(name) + ".\n");
    reward_player(second);

    save_master();
    return 1;
}

/*
 * Function name: add_donation
 * Description  : Called to add a donation.
 * Arguments    : string name - the name of the person whose donation we add.
 *                    (Must be in lower case).
 *                string currency - the local currency the person used.
 *                int amount - the amount the player donated in local currency.
 *                int euro_amount - the amount in dollars.
 * Returns      : int - success/failure.
 */
nomask int
add_donation(string name, string currency, int amount, int euro_amount)
{
    /* Illegal call. May only be called from treasury office. */
    if (file_name(previous_object()) != DONATION_OFFICE)
    {
        write("Illegal call! May only be called from treasury office.\n");
        return 0;
    }

    if (!SECURITY->exist_player(name) && (name != "anonymous"))
    {
        write("There is no player named " + capitalize(name) + ".\n");
        return 0;
    }

    if (member_array(currency, DONATION_CURRENCIES) == -1)
    {
        write("There is no currency '" + currency +
            "'. Possible currencies are: " +
            COMPOSITE_WORDS(DONATION_CURRENCIES) + ".\n");
        return 0;
    }

    if (amount < 1)
    {
        write("The amount of " + amount +
            " in local currency should be positive.\n");
        return 0;
    }

    if (euro_amount < 1)
    {
        write("The amount of " + euro_amount +
            " in EUR should be positive.\n");
        return 0;
    }

    write("Registered donation by " + capitalize(name) + " of " + currency +
        " " + amount + " (EUR " + euro_amount + ").\n");
    log_donation(name, "Donation " + currency + " " + amount + " (EUR " +
        euro_amount + ").");
    total_donations += euro_amount;
    donors_2003[name] += euro_amount;
    save_master();
    reward_player(name);

    return 1;
}

/*
 * Function name: remove_donation
 * Description  : Called to cancel a donation or the registration of a second.
 * Arguments    : string name - the name of the person whose donation to
 *                    cancel. (Must be in lower case).
 * Returns      : int - success/failure.
 */
nomask int
remove_donation(string name)
{
    string *names;
    int    index;

    /* Illegal call. May only be called from treasury office. */
    if (file_name(previous_object()) != DONATION_OFFICE)
    {
        write("Illegal call! May only be called from treasury office.\n");
        return 0;
    }

    if (seconds[name])
    {
        write("Unregistered " + capitalize(name) + " as second from " +
            capitalize(seconds[name]) + ". That donation is not removed.\n");
        log_donation(name, "Removed as second from " +
            capitalize(seconds[name]));
        seconds = m_delete(seconds, name);
        save_master();
        return 1;
    }

    if (!donors_2003[name])
    {
        write("No donation registered for " + capitalize(name) + ".\n");
        return 0;
    }

    total_donations -= donors_2003[name];

    write("Removed donation from " + capitalize(name) + ".\n");
    log_donation(name, "Removed donation");
    donors_2003 = m_delete(donors_2003, name);

    names = m_indices(seconds);
    index = sizeof(names);
    while(--index >= 0)
    {
        if (seconds[names[index]] == name)
        {
            log_donation(names[index], "Removed as second from " +
                capitalize(name));
            seconds = m_delete(seconds, names[index]);
            write("Also removed " + capitalize(names[index]) +
                " as second to " + capitalize(name) + ".\n");
        }
    }

    save_master();
    return 1;
}

/*
 * Function name: report_statistics
 * Description  : Print some statistics about the donations.
 * Arguments    : int details - 1/0 - If true, print detailed statistics for the
 *                    donation team. If false, print standard statistics for
 *                    players and wizards.
 */
nomask void
report_statistics(int details)
{
    int    size_mortals;
    int    size_wizards;
    int    size_players;
    int    total_mortals;
    int    total_wizards;
    string *names = m_indices(donors_2003);
    int    size = sizeof(names);

    while(--size >= 0)
    {
        if (SECURITY->query_wiz_rank(names[size]))
        {
            size_wizards++;
            total_wizards += donors_2003[names[size]];
        }
        else
        {
            size_mortals++;
            total_mortals += donors_2003[names[size]];
        }
    }

    write("                   Number Average Total EUR\n");
    write(sprintf("Donated by mortals  %3i  *  $%3i  = $ %5i\n",
        size_mortals, (total_mortals / size_mortals), total_mortals));
    write(sprintf("Donated by wizards  %3i  *  $%3i  = $ %5i\n",
        size_wizards, (total_wizards / size_wizards), total_wizards));

    size_players = m_sizeof(donors_2003);
    write(sprintf("Donated in total    %3i  *  $%3i  = $ %5i\n",
        size_players, (total_donations / size_players), total_donations));
}

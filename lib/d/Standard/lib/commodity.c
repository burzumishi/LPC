/* === /d/Standard/lib/commodity.c ========================================
 * ============================================ Raven 2001 (c) Standard ===
 *
 * Proposal       : Cirion                                      18.04.2000
 * Implementation : Raven                                       25.08.2001
 *
 * The module is a support for a product ,  that can be bought and sold by
 * the related commercant.
 *
 * === definition functions ==============================================
 * =============================================================== *** ===
 *
 * set_commodity_name(s)     functions sets the unique commodity  name  of
 *                           the object
 *
 * set_commodity_value(i,i)  function sets the commodity value of the obj-
 *                           ect ,  the real value then updated to be much
 *                           less, so the commodity wouldnt be sold in the
 *                           shops, the arguments are: commodity value and
 *                           the real value adjust flag (default 1)
 *
 * =======================================================================
 * =============================================================== *** ===
 */

#pragma save_binary
#pragma strict_types

#include <files.h>
#include <macros.h>
#include <stdproperties.h>

/*
 * === global variables ==================================================
 * =============================================================== *** ===
 */

#define COMMOD_VALUE_LOG     ("BAD_COM_VAL")
#define COMMOD_VALUE_RED     (10)

static string commodity_name ; /* the name of commodity                 */
static int    commodity_value; /* the value of commodity                */

/*
 * === supporting functions ==============================================
 * =============================================================== *** ===
 */

/*
 * Function name : update_commodity_value
 * Description   : function updates the OBJ_I_VALUE to be less than the
 *                 commodity value , the reduction happens according to
 *                 the COMMOD_VALUE_RED ,  which defines the percent of
 *                 value reduction
 * Arguments     : void    - no arguments
 * Returns       : void    - nothing is returned
 */
static nomask void
update_commodity_value()
{
    if (IS_HEAP_OBJECT(this_object()))
    {
        this_object()->add_prop(HEAP_I_UNIT_VALUE,
            (COMMOD_VALUE_RED * commodity_value) / 100);
    }
    else
    {
        this_object()->add_prop(OBJ_I_VALUE,
            (COMMOD_VALUE_RED * commodity_value) / 100);
    }
}

/*
 * Function name : secure_commodity_value
 * Description   : function checks the prices of certain commodity in
 *                 different domains, and logs any conflicts found in
 *                 the domain's COMMOD_VALUE_LOG
 * Arguments     : string  - the name of commodity
 *                 int     - the value set by domain
 * Returns       : int     - the maximum value found in domains
 */
public nomask int
secure_commodity_value(string commod, int setval)
{
    object *domlin, result;
    string *source;
    int     number, maxval, comval;

    setuid(); seteuid(getuid());

    domlin = map(SECURITY->query_domain_links() , find_object);
    source = map(domlin, &->check_commodity(commod)) - ({ 0 });
    source = source - ({ MASTER_OB(this_object()) });

    if ((number = sizeof(source)) <= 0)
    {
        return setval;
    }

    while (--number >= 0)
    {
        if (!objectp(result = find_object(source[number])))
        {
            LOAD_ERR(source[number]);
            result = find_object(source[number]);
        }

        comval = result->query_commodity_value();

        if ((comval != setval) && result)
        {
            log_file(COMMOD_VALUE_LOG, SECURITY->creator_object(result) +
                " defines the value of '" + commodity_name + "' as " +
                comval + "!\n");
        }

        maxval = max(maxval, comval);
    }

    return max(setval, maxval);
}

/*
 * === associated functions ==============================================
 * =============================================================== *** ===
 */

/*
 * Function name : check_commodity
 * Description   : this is a commodity
 * Arguments     : void    - no arguments
 * Returns       : int     - always 1
 */
public nomask int
check_commodity()
{
    return 1;
}

/*
 * Function name : set_commodity_name
 * Description   : function sets the unique commodity name of the object
 * Arguments     : string  - the name
 * Returns       : void    - nothing is returned
 */
public nomask void
set_commodity_name(string name)
{
    commodity_name = name;

    if (commodity_value)
    {
        commodity_value = secure_commodity_value(name, commodity_value);
    }
}

/*
 * Function name : query_commodity_name
 * Description   : function returns the commodity name of the object
 * Arguments     : void    - no arguments
 * Returns       : string  - commodity name
 */
public nomask string
query_commodity_name()
{
    return commodity_name;
}

/*
 * Function name : set_commodity_value
 * Description   : function sets the value of the commodity, the real
 *                 value then adjusted to always be less than the
 *                 commodity_value so player wouldnt prefer to sell
 *                 the item in a shop
 * Arguments     : int     - the commodity value
 *                 int     - the real value adjust flag (default 1)
 * Returns       : void    - nothing is returned
 */
public nomask varargs void
set_commodity_value(int value, int adjust = 1)
{
    commodity_value = (!commodity_name ? value :
        secure_commodity_value(commodity_name, value));

    if (adjust)
    {
        set_alarm(0.5, 0.0, update_commodity_value);
    }
}

/*
 * Function name : query_commodity_value
 * Description   : function returns the commodity value of the object
 * Arguments     : void    - no arguments
 * Returns       : int     - the commodity value
 */
public nomask int
query_commodity_value()
{
    return commodity_value;
}

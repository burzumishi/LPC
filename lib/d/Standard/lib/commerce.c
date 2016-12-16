/* === /d/Standard/lib/commerce.c =========================================
 * ============================================ Raven 2001 (c) Standard ===
 *
 * Proposal       : Cirion                                      18.04.2000
 * Implementation : Raven                                       25.08.2001
 *
 * The module is a support for a series of commerce libraries  that  allow
 * wizards to "commerce-enable " a room or NPC.  The room or NPC will then
 * either buy or sell a certain commodity to or from the player , who will
 * get a reward of cash and general experience. The reward will be greated
 * depending on the amount of strategy the player applies in their trades.
 *
 * === definition functions ==============================================
 * =============================================================== *** ===
 *
 * init_commerce()           function initiates commerce related actions ,
 *                           and should be called from init(_living())
 *
 * remove_commerce()         functions saves commerce data upon a  removal
 *                           of module , and  should  be  called  from the
 *                           remove_object() (in case savefile is set)
 *
 * set_commerce_savefile(s)  functions defines a path of savefile  of  the
 *                           module and enables module's stock save
 *
 * add_demanded_commodity(s,i,i,f)    function adds commodity to the stock
 *                           of the demanded commodity, the arguments are:
 *                           commodity name , consume span (minutes) , max
 *                           capacity and cost factor
 *
 * add_supplied_commodity(s,i,i,f)    function adds commodity to the stock
 *                           of the supplied commodity, the arguments are:
 *                           commodity name, increase span (minutes) , max
 *                           capacity and cost factor
 *
 * add_supply_requirement(s,m,i);     function sets requirements for supp-
 *                           lied commodity production, the arguments are:
 *                           supplied commodity name, name of demanded co-
 *                           mmodity and the demanded amount of it;   when
 *                           there is a need to define a couple of reuire-
 *                           ments for supplied commodity function may  be
 *                           used this way: add_supply_requirements("ink",
 *                           ({"water", 3, "color", 1 }));
 *
 * =======================================================================
 * =============================================================== *** ===
 */

#pragma save_binary
#pragma strict_types

inherit "/lib/trade";
#include <cmdparse.h>
#include <composite.h>
#include <files.h>
#include <language.h>
#include <macros.h>
#include <money.h>

/*
 * === associated variables ==============================================
 * =============================================================== *** ===
 */

#define F_COMMERCE_GENEXP(cost) (ftoi(pow(cost, 0.8)) + 1)

#define COMMODITY_LIB           ("/d/Standard/lib/commodity")
#define IS_COMMODITY_LIB(o)     (COMMODITY_LIB == \
        function_exists("check_commodity", (o)))

#define UPDATE_SPAN      (60)  /* global commerce update span (in secs) */

#define COMM_UPSPAN      (0)   /* (int  ) commodity's update span slot  */
#define COMM_UPTIME      (1)   /* (int  ) commodity's update time slot  */
#define COMM_MAXCAP      (2)   /* (int  ) commodity's max capacity slot */
#define COMM_COSTFC      (3)   /* (float) commodity's cost factor slot  */
#define COMM_AMOUNT      (4)   /* (int  ) commodity's amount slot       */
#define COMM_RESERV      (5)   /* (int  ) commodity's reserve slot      */
#define COMM_PRIORT      (6)   /* (int  ) commodity's priority slot     */
#define COMM_SIZEOF      (7)   /* array size in the commodity mappings  */

private static  int     commer_uptime = time(); /* latest update time   */
private static  string  commer_svfile; /* the save file of commerce     */
private static  mapping commer_demand, /* the demanded commodity data   */
                        commer_supply, /* the supplied commodity data   */
                        commer_requir; /* the supply requirement data   */

/*
 * === supporting functions ==============================================
 * =============================================================== *** ===
 */

/*
 * Function name : find_commodity
 * Description   : function searches for commodity filename thorough the
 *                 domain_links with a priority to local domain commodity
 * Arguments     : string  - the name of commodity
 * Returns       : object  - the commodity master object
 */
public nomask object
find_commodity(string commod)
{
    object *domlin,  result;
    string *mudcom, *domcom;

    setuid(); seteuid(getuid());

    domlin = map(SECURITY->query_domain_links() , find_object);
    mudcom = map(domlin, &->query_commodity(commod)) - ({ 0 });

    domcom = filter(mudcom, &wildmatch("/?/" + geteuid() + "/*", ));
    commod = (sizeof(domcom) ? domcom[0] : (sizeof(mudcom) ? mudcom[0] : 0));

    if (commod && !objectp(result = find_object(commod)))
    {
        LOAD_ERR(commod);
        return find_object(commod);
    }

    return result;
}

/*
 * Function name : clone_commodity
 * Description   : function clones the requested commodity and returns it
 * Arguments     : string  - the filename of commodity
 *                 int     - the amount to clone
 * Returns       : object* - the commodities array
 */
public nomask varargs object*
clone_commodity(string source, int amount = 1)
{
    object  result, *oblist;

    if (!objectp(result = find_object(source)))
    {
        LOAD_ERR(source);

        if (!objectp(result = find_object(source)))
        {
            return ({ });
        }
    }

    if (IS_HEAP_OBJECT(result))
    {
        result = clone_object(source);
        result->set_heap_size(max(1, amount));

        return ({ result });
    }

    oblist = ({ });

    while (--amount >= 0)
    {
        oblist += ({ clone_object(source) });
    }

    return oblist;
}

/*
 * Function name : remove_commodity
 * Description   : function removes a commodity, it was made to support
 *                 heaps removal , splited heaps get their size reduced
 * Arguments     : object  - the commodity to remove
 * Returns       : void    - nothing is returned
 */
public nomask void
remove_commodity(object entity)
{
    int     amount;

    if (!IS_HEAP_OBJECT(entity))
    {
        return entity->remove_object();
    }

    amount = entity->num_heap();
    entity->restore_heap();

    if (amount == entity->num_heap())
    {
        return entity->remove_object();
    }

    entity->set_heap_size(entity->num_heap() - amount);
}

/*
 * Function name : update_commerce
 * Description   : function updates the commodities amounts, it is called
 *                 each UPDATE_SPAN seconds from various functions in the
 *                 library
 * Arguments     : void    - no arguments
 * Returns       : void    - nothing is returned
 */
public nomask void
update_commerce()
{
    string *commod;

    if ((itof(time() - commer_uptime) / itof(UPDATE_SPAN)) < 1.0)
    {
        return;
    }

    if (sizeof(commod = m_indices(commer_demand || ([]))))
    {
        map(commod, &(this_object())->update_demanded_commodity());
    }

    if (sizeof(commod = m_indices(commer_supply || ([]))))
    {
        map(commod, &(this_object())->update_supplied_commodity());
    }

    commer_uptime = time();
}

/*
 * === associated functions ==============================================
 * =============================================================== *** ===
 */

/*
 * Function name : set_commerce_savefile
 * Description   : function sets the file name of the commerce save ,  it
 *                 as well would load the commerce with an alarm to allow
 *                 other commerce settings to take place before the load
 * Arguments     : string  - the file name of commerce save
 * Returns       : void    - nothing is returned
 */
public void
set_commerce_savefile(string source)
{
    setuid(); seteuid(getuid());

    if (strlen(source = (source[-2..] == ".o") ? source[..-3] : source))
    {
        commer_svfile = source;
        set_alarm(0.2, 0.0, &(this_object())->load_commerce());
    }
}

/*
 * Function name : query_commerce_savefile
 * Description   : function returns the file name of commerce save
 * Arguments     : void    - no arguments
 * Returns       : string  - the file name of commerce save
 */
public string
query_commerce_savefile()
{
    return commer_svfile;
}

/*
 * Function name : add_demanded_commodity
 * Description   : function adds commodity to the demanded commodity list,
 *                 the function is varargs requiring 1 argument at minimum
 * Arguments     : string  - the name of the commodity
 *                 int     - the update span (in minutes) (default 32m)
 *                 int     - the maximum commodity amount (default 100)
 *                 float   - the commodity cost factor    (default 1.0)
 * Returns       : void    - nothing is returned
 */
public varargs void
add_demanded_commodity(string commod, int upspan = 32, int maxcap = 100,
    float costfc = 1.0)
{
    if (!objectp(find_commodity(commod)))
    {
        throw("Commodity " + commod + " wasnt defined.\n");
        return;
    }

    if (!mappingp(commer_demand))
    {
        commer_demand = ([ ]);
    }

    if (!pointerp(commer_demand[commod]))
    {
        commer_demand[commod] = allocate(COMM_SIZEOF);

        commer_demand[commod][COMM_AMOUNT] = max(maxcap / 25, 0);
        commer_demand[commod][COMM_RESERV] = max(maxcap / 25, 0);
    }

    commer_demand[commod][COMM_UPSPAN] = max(upspan * 60, UPDATE_SPAN);
    commer_demand[commod][COMM_UPTIME] = commer_uptime;
    commer_demand[commod][COMM_MAXCAP] = max(maxcap,   1);
    commer_demand[commod][COMM_COSTFC] = max(costfc, 0.0);
    commer_demand[commod][COMM_PRIORT] = 1;
}

/*
 * Function name : remove_demanded_commodity
 * Description   : function removes commodity from demanded commodity list
 * Arguments     : string  - the name of the commodity
 * Returns       : void    - nothing is returned
 */
public void
remove_demanded_commodity(string commod)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        if (!m_sizeof(commer_demand = m_delete(commer_demand, commod)))
        {
            commer_demand = 0;
        }
    }
}

/*
 * Function name : update_demanded_commodity
 * Description   : function updates the amounts of demanded commodity,
 *                 in this case updated value is the "needed amount"
 * Arguments     : string  - the name of commodity
 * Returns       : void    - nothing is returned
 */
public nomask void
update_demanded_commodity(string commod)
{
    mixed   record;
    int     period, amount, reserv, maxcap, uplift;

    record = commer_demand[commod];

    if (record[COMM_PRIORT] <= 0)
    {
        record[COMM_UPTIME] = time();
        return;
    }

    period = time() - record[COMM_UPTIME];

    if (period  < record[COMM_UPSPAN])
    {
        return;
    }

    amount = record[COMM_AMOUNT];
    reserv = record[COMM_RESERV];
    maxcap = record[COMM_MAXCAP];

    if ((amount + reserv) >= maxcap)
    {
        record[COMM_UPTIME] = time();
        return;
    }

    uplift = min(period / record[COMM_UPSPAN], maxcap - (amount + reserv));

    record[COMM_RESERV] += uplift;
    record[COMM_UPTIME]  = time() - (period % record[COMM_UPSPAN]);

    this_object()->hook_commerce_reserve_update(commod, uplift);
}

/*
 * Function name : exist_demanded_commodity
 * Description   : function checks whether a demanded commodity exists
 * Arguments     : string  - the name of commodity
 * Returns       : int     - 1 if exists or 0 otherwise
 */
public int
exist_demanded_commodity(string commod)
{
    return (mappingp(commer_demand) && pointerp(commer_demand[commod]));
}

/*
 * Function name : query_demanded_commodity
 * Description   : function returns the names of demanded commodities
 * Arguments     : void    - no arguments
 * Returns       : string* - the names of demanded commodities
 */
public string*
query_demanded_commodity()
{
    return m_indices(commer_demand || ([ ]));
}

/*
 * Function name : add_supplied_commodity
 * Description   : function adds commodity to the supplied commodity list,
 *                 the function is varargs requiring 1 argument at minimum
 * Arguments     : string  - the name of the commodity
 *                 int     - the update span (in minutes) (default 32m)
 *                 int     - the maximum commodity amount (default 100)
 *                 float   - the commodity cost factor    (default 1.0)
 * Returns       : void    - nothing is returned
 */
public varargs void
add_supplied_commodity(string commod, int upspan = 32, int maxcap = 100,
    float costfc = 1.0)
{
    if (!objectp(find_commodity(commod)))
    {
        throw("Commodity " + commod + " wasnt defined.\n");
        return;
    }

    if (!mappingp(commer_supply))
    {
        commer_supply = ([ ]);
    }

    if (!pointerp(commer_supply[commod]))
    {
        commer_supply[commod] = allocate(COMM_SIZEOF);

        commer_supply[commod][COMM_AMOUNT] = max(maxcap / 25, 0);
        commer_supply[commod][COMM_RESERV] = max(maxcap / 25, 0);
    }

    commer_supply[commod][COMM_UPSPAN] = max(upspan * 60, UPDATE_SPAN);
    commer_supply[commod][COMM_UPTIME] = commer_uptime;
    commer_supply[commod][COMM_MAXCAP] = max(maxcap,   1);
    commer_supply[commod][COMM_COSTFC] = max(costfc, 0.0);
    commer_supply[commod][COMM_PRIORT] = 1;
}

/*
 * Function name : remove_supplied_commodity
 * Description   : function removes commodity from supplied commodity list
 * Arguments     : string  - the name of the commodity
 * Returns       : void    - nothing is returned
 */
public void
remove_supplied_commodity(string commod)
{
    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        if (!m_sizeof(commer_supply = m_delete(commer_supply, commod)))
        {
            commer_supply = 0;
        }
    }
}

/*
 * Function name : update_supplied_commodity
 * Description   : function updates the amounts of supplied commodity,
 *                 in this case updated value is the "current amount"
 * Arguments     : string  - the name of commodity
 * Returns       : void    - nothing is returned
 */
public nomask void
update_supplied_commodity(string commod)
{
    mixed   record;
    string *reqcom;
    int    *tmparr, period, amount, reserv, maxcap,
            uplift, number, reqsiz, needed;

    record = commer_supply[commod];

    if (record[COMM_PRIORT] <= 0)
    {
        record[COMM_UPTIME] = time();
        return;
    }

    period = time() - record[COMM_UPTIME];

    if (period  < record[COMM_UPSPAN])
    {
        return;
    }

    amount = record[COMM_AMOUNT];
    reserv = record[COMM_RESERV];
    maxcap = record[COMM_MAXCAP];

    record[COMM_UPTIME] = time() - (period % record[COMM_UPSPAN]);

    if (!this_object()->exist_supply_requirement(commod))
    {
        if (amount >= maxcap)
        {
            return;
        }

        uplift = min(period / record[COMM_UPSPAN], maxcap - amount);
        record[COMM_AMOUNT] += uplift;
        this_object()->hook_commerce_amount_update(commod, uplift);
        return;
    }

    /* the simple case of supplied commodity (one without requirements) */
    /* just adds amount each update span, but the one with requirements */
    /* acts a bit differently , I will use example to explain the code: */
    /*                                                                  */
    /* a brewer needs 2 wheat and 3 water to create one beer, currently */
    /* he is ready to make 1 beer (the reserve of beer), if the time is */
    /* matching we add the ready to be made beer to stock and remove it */
    /* from the reserve                                                 */

    uplift = min(period / record[COMM_UPSPAN], maxcap - amount, reserv);

    if (uplift >= 1)
    {
        record[COMM_AMOUNT] += uplift;
        record[COMM_RESERV] -= uplift;
        this_object()->hook_commerce_amount_update(commod, uplift);
        this_object()->hook_commerce_reserve_update(commod, uplift);
    }

    /* now we scan the requirements, aka wheat and water, the amount of */
    /* beers the brewer can make is min(wheat / 2, water / 3), when the */
    /* previous min function returns number bigger than 0 - it means we */
    /* are ready to produce another beer, this beer is moved to reserve */
    /* and would be actually added to the stock at the next update call */

    reqcom = this_object()->query_supply_requirement(commod);
    reqsiz = sizeof(reqcom);
    tmparr = ({ });
    number = -1;

    while (++number < reqsiz)
    {
        amount  = commer_demand[reqcom[number]][COMM_AMOUNT];
        needed  = commer_requir[commod][reqcom[number]];
        tmparr += ({ max(0, amount) / needed });
    }

    uplift = maxcap - record[COMM_AMOUNT] - record[COMM_RESERV];

    if ((uplift = min(uplift, applyv(min, tmparr))) >= 1)
    {
        record[COMM_RESERV] += uplift;

        number = -1;

        while (++number < reqsiz)
        {
            commer_demand[reqcom[number]][COMM_AMOUNT] -=
                (uplift * commer_requir[commod][reqcom[number]]);
        }

        this_object()->hook_commerce_reserve_update(commod, uplift);
    }
}

/*
 * Function name : exist_supplied_commodity
 * Description   : function checks whether a supplied commodity exists
 * Arguments     : string  - the name of commodity
 * Returns       : int     - 1 if exists or 0 otherwise
 */
public int
exist_supplied_commodity(string commod)
{
    return (mappingp(commer_supply) && pointerp(commer_supply[commod]));
}

/*
 * Function name : query_supplied_commodity
 * Description   : function returns the names of supplied commodities
 * Arguments     : void    - no arguments
 * Returns       : string* - the names of supplied commodities
 */
public string*
query_supplied_commodity()
{
    return m_indices(commer_supply || ([ ]));
}

/*
 * Function name : add_supply_requirement
 * Description   : function adds requirements for producing a commodity
 * Arguments     : string  - the supplied commodity name
 *                 mixed   - the required component or array of required
 *                           components and their amounts
 *                 int     - (optional) component quantity (applies only
 *                           when component value is a string
 * Returns       : void    - nothing is returned
 */
public varargs void
add_supply_requirement(string commod, mixed reqcom, int reqamo = 1)
{
    int     number, amount;

    if (!exist_supplied_commodity(commod))
    {
        add_supplied_commodity(commod);
    }

    if (!mappingp(commer_requir))
    {
        commer_requir = ([ ]);
    }

    if (!pointerp(reqcom))
    {
        reqcom = ({ reqcom, reqamo });
    }

    number = -2;
    amount = sizeof(reqcom) - 1;

    while ((number += 2) < amount)
    {
        if (!stringp(reqcom[number]) || !intp(reqcom[number + 1]))
        {
            throw("Wrong parameters to add_supply_requirement.\n");
            return;
        }

        if (!exist_demanded_commodity(reqcom[number]))
        {
            add_demanded_commodity(reqcom[number]);
        }

        if (!mappingp(commer_requir[commod]))
        {
            commer_requir[commod] = ([ ]);
        }

        commer_requir[commod][reqcom[number]] = reqcom[number + 1];
    }
}

/*
 * Function name : remove_supply_requirement
 * Description   : function removes requirement for producing a commodity
 * Arguments     : string  - the supplied commodity name
 *                 string  - the required component to remove
 * Returns       : void    - nothing is returned
 */
public void
remove_supply_requirement(string commod, string reqcom)
{
    if (!mappingp(commer_requir) || !mappingp(commer_requir[commod]))
    {
        return;
    }

    if (!mappingp(commer_requir[commod][reqcom]))
    {
        return;
    }

    commer_requir[commod][reqcom] = m_delete(
        commer_requir[commod][reqcom], reqcom);

    if (!m_sizeof(commer_requir[commod]))
    {
        commer_requir[commod] = m_delete(commer_requir[commod], commod);
    }
}

/*
 * Function name : exist_supply_requirement
 * Description   : function checks whether a commodity has supply reqs
 * Arguments     : string  - the supplied commodity name
 * Returns       : int     - 1 if requirements exist or 0 otherwise
 */
public int
exist_supply_requirement(string commod)
{
    return (mappingp(commer_requir) && mappingp(commer_requir[commod]));
}

/*
 * Function name : query_supply_requirement
 * Description   : function returns data on commodity supply requirements
 * Arguments     : string  - the name of the supplied commodity
 *                 string  - (optional) the required component
 * Returns       : mixed   - if component is -1 returns the list of all
 *                           components required for creating the
 *                           commodity; returns the required quantity of
 *                           component for creating the named commodity
 */
public varargs mixed
query_supply_requirement(string commod, mixed compon = -1)
{
    mapping record;

    if (mappingp(commer_requir) && mappingp(commer_requir[commod]))
    {
        record = commer_requir[commod];
    }

    if (intp(compon) && (compon == -1))
    {
        return (mappingp(record) ? m_indices(record) : ({ }));
    }

    return ((mappingp(record) && record[compon]) ? record[compon] : 0);
}

/*
 * Function name : set_commodity_upspan
 * Description   : function sets commodity's increase/consume update span
 * Arguments     : string  - the name of commodity
 *                 int     - the increase/consume update span (in minutes)
 * Returns       : void    - nothing is returned
 */
public void
set_commodity_upspan(string commod, int upspan)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        commer_demand[commod][COMM_UPSPAN] = upspan * 60;
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        commer_supply[commod][COMM_UPSPAN] = upspan * 60;
    }
}

/*
 * Function name : query_commodity_upspan
 * Description   : function returns commodity's increase/consume update span
 * Arguments     : string  - the name of commodity
 * Returns       : int     - the increase/consume update span (in minutes)
 */
public int
query_commodity_upspan(string commod)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        return commer_demand[commod][COMM_UPSPAN] / 60;
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        return commer_supply[commod][COMM_UPSPAN] / 60;
    }

    return 0;
}

/*
 * Function name : set_commodity_capacity
 * Description   : function sets commodity's maximum capacity
 * Arguments     : string  - the name of commodity
 *                 int     - the maximum capacity
 * Returns       : void    - nothing is returned
 */
public void
set_commodity_capacity(string commod, int maxcap)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        commer_demand[commod][COMM_MAXCAP] = maxcap;
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        commer_supply[commod][COMM_MAXCAP] = maxcap;
    }
}

/*
 * Function name : query_commodity_capacity
 * Description   : function returns the maximum capacity of commodity
 * Arguments     : string  - the name of commodity
 * Returns       : int     - the maximum capacity of commodity
 */
public int
query_commodity_capacity(string commod)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        return commer_demand[commod][COMM_MAXCAP];
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        return commer_supply[commod][COMM_MAXCAP];
    }

    return 0;
}

/*
 * Function name : set_commodity_costf
 * Description   : function sets commodity's cost factor
 * Arguments     : string  - the name of commodity
 *                 float   - the cost factor
 * Returns       : void    - nothing is returned
 */
public void
set_commodity_costf(string commod, float costfc)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        commer_demand[commod][COMM_COSTFC] = costfc;
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        commer_supply[commod][COMM_COSTFC] = costfc;
    }
}

/*
 * Function name : query_commodity_costf
 * Description   : function returns the cost factor of commodity
 * Arguments     : string  - the name of commodity
 * Returns       : float   - the cost factor of commodity
 */
public float
query_commodity_costf(string commod)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        return commer_demand[commod][COMM_COSTFC];
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        return commer_supply[commod][COMM_COSTFC];
    }

    return 0.0;
}

/*
 * Function name : set_commodity_amount
 * Description   : function sets commodity's amount
 * Arguments     : string  - the name of commodity
 *                 int     - the amount to set
 * Returns       : void    - nothing is returned
 */
public void
set_commodity_amount(string commod, int amount)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        commer_demand[commod][COMM_AMOUNT] = amount;
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        commer_supply[commod][COMM_AMOUNT] = amount;
    }
}

/*
 * Function name : add_commodity_amount
 * Description   : function adds commodity's amount
 * Arguments     : string  - the name of commodity
 *                 int     - the amount to add
 * Returns       : void    - nothing is returned
 */
public void
add_commodity_amount(string commod, int amount)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        commer_demand[commod][COMM_AMOUNT] += amount;
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        commer_supply[commod][COMM_AMOUNT] += amount;
    }
}

/*
 * Function name : query_commodity_amount
 * Description   : function returns the amount of commodity
 * Arguments     : string  - the name of commodity
 * Returns       : int     - the amount of commodity
 */
public int
query_commodity_amount(string commod)
{
    update_commerce();

    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        return commer_demand[commod][COMM_AMOUNT];
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        return commer_supply[commod][COMM_AMOUNT];
    }

    return 0;
}

/*
 * Function name : set_commodity_reserve
 * Description   : function sets commodity's "possible to make/buy amount"
 * Arguments     : string  - the name of commodity
 *                 int     - the "possible to make/buy amount" amount
 * Returns       : void    - nothing is returned
 */
public void
set_commodity_reserve(string commod, int reserv)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        commer_demand[commod][COMM_RESERV] = reserv;
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        commer_supply[commod][COMM_RESERV] = reserv;
    }
}

/*
 * Function name : add_commodity_reserve
 * Description   : function adds commodity's "possible to make/buy amount"
 * Arguments     : string  - the name of commodity
 *                 int     - the "possible to make/buy amount" to add
 * Returns       : void    - nothing is returned
 */
public void
add_commodity_reserve(string commod, int reserv)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        commer_demand[commod][COMM_RESERV] += reserv;
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        commer_supply[commod][COMM_RESERV] += reserv;
    }
}

/*
 * Function name : query_commodity_reserve
 * Description   : function returns the "possible to make/buy amount"
 * Arguments     : string  - the name of commodity
 * Returns       : int     - the "possible to make/buy amount"
 */
public int
query_commodity_reserve(string commod)
{
    update_commerce();

    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        return commer_demand[commod][COMM_RESERV];
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        return commer_supply[commod][COMM_RESERV];
    }

    return 0;
}

/*
 * Function name : set_commodity_priority
 * Description   : function sets commodity's priority flag, priority can
 *                 only have two values 1 (default) and 0, when set to 0
 *                 commodity amounts will not be updated (one can say it
 *                 will be "frozen in time")
 * Arguments     : string  - the name of commodity
 *                 int     - the priority flag
 * Returns       : void    - nothing is returned
 */
public void
set_commodity_priority(string commod, int prflag)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        commer_demand[commod][COMM_PRIORT] = prflag;
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        commer_supply[commod][COMM_PRIORT] = prflag;
    }
}

/*
 * Function name : query_commodity_priority
 * Description   : function returns the priority flag of the commodity
 * Arguments     : string  - the name of commodity
 * Returns       : int     - the priority flag of the commodity
 */
public int
query_commodity_priority(string commod)
{
    if (mappingp(commer_demand) && pointerp(commer_demand[commod]))
    {
        return commer_demand[commod][COMM_PRIORT];
    }

    if (mappingp(commer_supply) && pointerp(commer_supply[commod]))
    {
        return commer_supply[commod][COMM_PRIORT];
    }

    return 0;
}

/*
 * === hooked functions ==================================================
 * =============================================================== *** ===
 */

/*
 * Function name : hook_commerce_amount_update
 * Description   : function is called when amount of commodities in the
 *                 stock were updated, try to avoid any file loading or
 *                 cloning when using this function ,  since runtime or
 *                 eval_cost may cause bad updation of other commodities
 * Arguments     : string  - the the name of commodity
 *                 int     - the amount added
 * Returns       : void    - nothing is returned
 */
public void
hook_commerce_amount_update(string commod, int amount)
{
}

/*
 * Function name : hook_commerce_reserve_update
 * Description   : function is called when reserve of commodity in the
 *                 stock were updated, try to avoid any file loading or
 *                 cloning when using this function ,  since runtime or
 *                 eval_cost may cause bad updation of other commodities
 * Arguments     : string  - the the name of commodity
 *                 int     - the amount added to reserve
 * Returns       : void    - nothing is returned
 */
public void
hook_commerce_reserve_update(string commod, int amount)
{
}

/*
 * Function name : appeal
 * Description   : the appeal of commerce module to a player, this hook
 *                 is supposed to open new oportunities in hooks masking
 *                 if your commercant is a goblin and he hates dwarves
 *                 badly, you could mask this hook to return "filthy
 *                 dwarven creature" for each dwarf, thus totally changing
 *                 commercant's behaviour
 * Arguments     : object  - the player commersant appeals to
 * Returns       : string  - the appeal string
 */
public varargs string
appeal(object player = this_player())
{
    switch (player->query_gender())
    {
        case  0: return "milord";
        case  1: return "milady";
        default: return "weird one";
    }
}

/*
 * Function name : hook_commerce_answer_dark_room
 * Description   : refuse to deal when the room is dark
 * Arguments     : void    - no arguments
 * Returns       : int     - 0 for nofity_fail or 1 for the rest
 */
public int
hook_commerce_answer_dark_room()
{
    command("say I cant see you, light a torch first, please.");
    return 1;
}

/*
 * Function name : hook_commerce_answer_invisible
 * Description   : refuse to deal with player who is invisible for commerce
 * Arguments     : void    - no arguments
 * Returns       : int     - 0 for nofity_fail or 1 for the rest
 */
public int
hook_commerce_answer_invisible()
{
    command("say I can't see you, where are you hiding?");
    command("peer suspiciously");
    return 1;
}

/*
 * Function name : hook_commerce_buy_faulty_syntax
 * Description   : called when player tries to buy with faulty syntax
 * Arguments     : string  - argument writen by the player
 * Returns       : int     - 0 for nofity_fail or 1 for the rest
 */
public int
hook_commerce_buy_faulty_syntax(string cmdarg)
{
    if (!strlen(cmdarg) || !living(this_object()))
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    command("say What " + cmdarg + "? I dont have any of it, " +
        appeal() + ".");
    command("ponder");
    return 1;
}

/*
 * Function name : hook_commerce_buy_notify_header
 * Description   : gives a header message when player tries to buy
 * Arguments     : string  - argument writen by the player
 * Returns       : void    - nothing is returned
 */
public void
hook_commerce_buy_notify_header(string cmdarg)
{
    if (!living(this_object()))
    {
        return;
    }

    write("You order " + cmdarg + " from " + this_object()->
        query_the_name(this_player()) + ".\n");

    say(QCTNAME(this_player()) + " orders " + cmdarg + " from " +
        QTNAME(this_object()) + ".\n");
}

/*
 * Function name : hook_commerce_buy_faulty_amount
 * Description   : called when player tries to buy more than in stock
 * Arguments     : object  - the commodity player tries to buy
 *                 int     - the quantity player tries to buy
 *                 int     - the quantity of commodity in stock
 * Returns       : int     - 0 for nofity_fail or 1 for the rest
 */
public int
hook_commerce_buy_faulty_amount(object commod, int amount, int in_stock)
{
    if (!living(this_object()))
    {
        if (!in_stock)
        {
            write("There are no " + commod->plural_short() +
                " currently for sale.\n");
           return 1;
        }

        write("There only " + (in_stock == 1 ? "is " : "are ") +
            LANG_WNUM(in_stock) + " " + (in_stock == 1 ? commod->short() :
            commod->plural_short()) + " available for sale currently.\n");

        return 1;
    }
    
    if (!in_stock)
    {
        command("say I am sorry, " + appeal() + ", but I dont have " +
            commod->plural_short() + " for sale at the moment.");
        return 1;
    }

    command("say I have only " + LANG_WNUM(in_stock) + " " +
        (in_stock == 1 ? commod->short() : commod->plural_short()) +
        " for sale, " + appeal() + ".");
 
    return 1;
}

/*
 * Function name : hook_commerce_buy_cannot_afford
 * Description   : notify that player doesnt have the needed money
 * Arguments     : object  - item player cannot afford
 *                 int     - the quantity of item he tried to buy
 * Returns       : int     - 0 for nofity_fail or 1 for the rest
 */
public int
hook_commerce_buy_cannot_afford(object commod, int amount)
{
    command("frown");
    command("say Why bother asking, " + appeal() + "?");
    command("say You cannot afford " + (amount == 1 ?
        LANG_THESHORT(commod) : (LANG_WNUM(amount) + " " +
        LANG_PWORD(LANG_THESHORT(commod)))) + ".");
    return 1;
}

/*
 * Function name : hook_commerce_buy_prevent_buy
 * Description   : function is the last validator in the chain of buy
 *                 validators, when it returns false specified commodity
 *                 would be bought for sure, and when it returns 1 it
 *                 would not, this hook must generate it's own messages
 *                 when returning true
 * Arguments     : object  - the master object of commodity player buys
 *                 int     - the quantity of item he tries to buy
 * Returns       : int     - 1 when buy is prevented or 0 otherwise
 */
public int
hook_commerce_buy_prevent_buy(object commod, int amount)
{
    return 0;
}

/*
 * Function name : hook_commerce_buy_notify_finish
 * Description   : deliver the items from stock to the player
 * Arguments     : object* - items player bought
 *                 int*    - array of coins payed by the player
 *                 int*    - array of change player got
 * Returns       : int     - 0 for nofity_fail or 1 for the rest
 */
public int
hook_commerce_buy_notify_finish(object *oblist, int *charge, int *change)
{
    int     number = -1, amount = sizeof(oblist);
    string  result;

    if (!living(this_object()))
    {
        write("You pay " + text(charge) + " for " +
            COMPOSITE_DEAD(oblist) + ".\n");

        if (strlen(result = text(change)))
        {
            write("You get " + result + " back.\n");
        }

        oblist = filter(oblist, &->move(this_player()));

        if (sizeof(oblist))
        {
            write("You drop " + COMPOSITE_DEAD(oblist) + ".\n");
            oblist->move(environment(this_player()));
        }

        return 1;
    }

    write("You pay " + this_object()->query_the_name(this_player()) +
        " " + text(charge) + ".\n");

    say(QCTNAME(this_player()) + " hands some coins to " +
        QTNAME(this_object()) + ".\n");

    if (strlen(result = text(change)))
    {
        write("You get " + result + " back.\n");
        say(QCTNAME(this_object()) + " gives some change back to " +
            QTNAME(this_player()) + ".\n");
    }

    oblist->move(this_object(), 1);

    while (++number < amount)
    {
        command("give " + OB_NAME(oblist[number]) + " to " +
            OB_NAME(this_player()));

        if (environment(oblist[number]) == this_object())
        {
            command("frown");
            command("say You don't seem able to carry " +
                LANG_THESHORT(oblist[number]) + ", " + appeal() + ".");
            command("put " + OB_NAME(oblist[number]));

            while (++number < amount)
            {
                command("put " + OB_NAME(oblist[number]));
            }
        }
    }

    return 1;
}

/*
 * Function name : hook_commerce_sell_faulty_syntax
 * Description   : notify that specified string doesnt exist in player
 * Arguments     : string  - argument writen by the player
 * Returns       : int     - 0 for nofity_fail or 1 for the rest
 */
public int
hook_commerce_sell_faulty_syntax(string cmdarg)
{
    if (!strlen(cmdarg) || !living(this_object()))
    {
        notify_fail(capitalize(query_verb()) + " what?\n");
        return 0;
    }

    command("say What " + cmdarg + "? You dont seem to have it, " +
        appeal() + ".");
    command("ponder");
    return 1;
}

/*
 * Function name : hook_commerce_sell_notify_header
 * Description   : gives a header message when player tries to sell
 * Arguments     : string  - argument writen by the player
 * Returns       : void    - nothing is returned
 */
public void
hook_commerce_sell_notify_header(string cmdarg)
{
    if (!living(this_object()))
    {
        return;
    }

    write("You offer " + cmdarg + " to " + this_object()->
        query_the_name(this_player()) + ".\n");

    say(QCTNAME(this_player()) + " offers " + cmdarg + " to " +
        QTNAME(this_object()) + ".\n");
}

/*
 * Function name : hook_commerce_sell_object_filter
 * Description   : function filters out the unsellable items out of array
 *                 of objects specified by the player, all the unsellable
 *                 items would pass thru hook_commerce_sell_object_nosell
 *                 function, so there is no need to add any textual reac-
 *                 tions to it
 * Arguments     : object  - one of the objects selected by player
 * Returns       : int     - true if the object is sellable or 0 otherwise
 */
public int
hook_commerce_sell_object_filter(object item)
{
    if (!IS_COMMODITY_LIB(item) || !this_object()->
        exist_demanded_commodity(item->query_commodity_name()) ||
        item->query_prop(OBJ_M_NO_SELL) ||
        item->query_prop(OBJ_M_NO_DROP) ||
        item->query_prop(OBJ_M_NO_GIVE))
    {
        return 0;
    }

    return 1;
}

/*
 * Function name : hook_commerce_sell_object_nosell
 * Description   : notify player that object is unsellable, note that
 *                 all the OBJ_M_NO_DROP, OBJ_M_NO_SELL and all the
 *                 not commodities or commodities that arent buyed by
 *                 this module go here, if you want an original message
 *                 for each of these, just check each object
 * Arguments     : object  - the object that isnt buyed by this module
 * Returns       : void    - nothing is returned
 */
public void
hook_commerce_sell_object_nosell(object item)
{
    if (!living(this_object()))
    {
        write("It seems you are stuck with " + LANG_THESHORT(item) +".\n");
        return;
    }

    command("say I don't want " + LANG_THESHORT(item) + ", " +
        appeal() + ".");
}

/*
 * Function name : hook_commerce_sell_object_noneed
 * Description   : notify player that object is unneeded, this hook
 *                 called when there is already plenty of the commodity
 *                 in stock and the module dont want anymore
 * Arguments     : object  - the object that is unneed
 * Returns       : void    - nothing is returned
 */
public void
hook_commerce_sell_object_noneed(object item)
{
    if (!living(this_object()))
    {
        write(capitalize(LANG_THESHORT(item)) + " is not needed at the" +
            " moment.\n");
        return;
    }

    command("say I have plenty of " + item->plural_short() + ", at the" +
        " moment, " + appeal() + ", no deal!");
}

/*
 * Function name : hook_commerce_sell_prevent_sell
 * Description   : function is the last validator in the chain of sell
 *                 validators, when it returns false specified commodity
 *                 would be sold for sure, and when it returns 1 it
 *                 would not, this hook must generate it's own messages
 *                 when returning true (please do not destruct or split
 *                 the passed object, p.s. parsing macros split heaps)
 * Arguments     : object  - the commodity object player sells
 * Returns       : int     - 1 when sell is prevented or 0 otherwise
 */
public int
hook_commerce_sell_prevent_sell(object commod)
{
    return 0;
}

/*
 * Function name : hook_commerce_sell_notify_finish
 * Description   : notify delivery of items from player to stock
 * Arguments     : object* - the items array
 *                 int*    - array of coins player got
 * Returns       : int     - 0 for nofity_fail or 1 for the rest
 */
public int
hook_commerce_sell_notify_finish(object *oblist, int *charge)
{
    if (!living(this_object()))
    {
        write("You sold " + COMPOSITE_DEAD(oblist) + ".\n");
        say(QCTNAME(this_player()) + " sold " + QCOMPDEAD + ".\n");

        write("You got " + text(charge) + ".\n");
        return 1;
    }

    write("You sold " + COMPOSITE_DEAD(oblist) + " to " +
       this_object()->query_the_name(this_player()) + ".\n");

    say(QCTNAME(this_player()) + " sold " + QCOMPDEAD + " to " +
          QTNAME(this_object()) + ".\n");

    write(this_object()->query_The_name(this_player()) + " gives you " +
       text(charge) + ".\n");

    return 1;
}

/*
 * === activating functions ==============================================
 * =============================================================== *** ===
 */

/*
 * Function name : action_commerce_buy
 * Description   : function handles attempts to buy from commerce object
 * Arguments     : string  - argument writen by the player
 * Returns       : int     - 1 for success , 0 for failure
 */
public int
action_commerce_buy(string cmdarg)
{
    string  coname;
    int     amount, stocka, maxcap, number, genexp;
    float   costfa;
    mixed   commod, cost = 1.0;

    if (!strlen(cmdarg))
    {
        return hook_commerce_buy_faulty_syntax(cmdarg);
    }

    hook_commerce_buy_notify_header(cmdarg);

    if (living(this_object()))
    {
        if (!CAN_SEE_IN_ROOM(this_object()))
        {
            return hook_commerce_answer_dark_room();
        }

        if (!CAN_SEE(this_object(), this_player()))
        {
            return hook_commerce_answer_invisible();
        }
    }

    if (parse_command(cmdarg, ({}), "%d %s", amount, coname))
    {
        cmdarg = coname;
    }
    else
    {
        amount = 1;
    }

    /* the following search might look unneeded, but it is, commodity   */
    /* has only one 'commodity_name' while can be refered by adjectives */
    /* or alike in many cases, so we parse with master_obs              */
    
    setuid(); seteuid(getuid());

    commod = map(query_supplied_commodity(), find_commodity);

    if (!sizeof(commod = FIND_STR_IN_ARR(cmdarg, commod)))
    {
        return hook_commerce_buy_faulty_syntax(cmdarg);
    }

    commod = commod[0];
    coname = commod->query_commodity_name();

    if (!(stocka = query_commodity_amount(coname)) || (amount > stocka))
    {
        return hook_commerce_buy_faulty_amount(commod, amount, stocka);
    }

    number  = -1;
    maxcap  = query_commodity_capacity(coname);
    costfa  = itof(commod->query_commodity_value());
    costfa *= query_commodity_costf(coname);

    while (++number < amount)
    {
        cost += ((1.0 - (itof(stocka - number) / itof(maxcap))) / 0.5);
    }

    cost *= costfa;
    genexp = F_COMMERCE_GENEXP(cost);

    if (sizeof(cost = pay(ftoi(cost), this_player(), "", 0, 0,"",1)) <= 1)
    {
        if (!objectp(this_player()))
        {
            throw("this_player() is zero.\n");
        }

        return hook_commerce_buy_cannot_afford(commod, amount);
    }

    if (hook_commerce_buy_prevent_buy(commod, amount))
    {
        return 1;
    }

    this_player()->add_exp_general(genexp);

    commer_supply[coname][COMM_AMOUNT] -= amount;

    return hook_commerce_buy_notify_finish(
        clone_commodity(MASTER_OB(commod), amount),
        cost[0..(SIZEOF_MONEY_TYPES - 1)],
        cost[SIZEOF_MONEY_TYPES..(2 * SIZEOF_MONEY_TYPES - 1)]);
}

/*
 * Function name : action_commerce_sell
 * Description   : function handles attempts to sell to the commerce object
 * Arguments     : string  - argument writen by the player
 * Returns       : int     - 1 for success , 0 for failure
 */
public int
action_commerce_sell(string cmdarg)
{
    int     amount, number, stocka, reserv, maxcap;
    float   comfee;
    string  coname;
    object *oblist, *nosell;
    mixed   cost = 0.0;

    if (!strlen(cmdarg))
    {
        return hook_commerce_sell_faulty_syntax(cmdarg);
    }

    hook_commerce_sell_notify_header(cmdarg);

    if (living(this_object()))
    {
        if (!CAN_SEE_IN_ROOM(this_object()))
        {
            return hook_commerce_answer_dark_room();
        }

        if (!CAN_SEE(this_object(), this_player()))
        {
            return hook_commerce_answer_invisible();
        }
    }

    if (!sizeof(oblist = FIND_STR_IN_OBJECT(cmdarg, this_player())))
    {
        return hook_commerce_sell_faulty_syntax(cmdarg);
    }

    nosell = filter(oblist, &not() @ hook_commerce_sell_object_filter);

    if (!(amount = sizeof(oblist -= nosell)))
    {
        if (member_array(cmdarg, ({ "all", "items", "things" })) >= 0)
        {
            return hook_commerce_sell_faulty_syntax(cmdarg);
        }

        map(nosell, hook_commerce_sell_object_nosell);
        return 1;
    }

    nosell = ({ });

    while (--amount >= 0)
    {
        coname = oblist[amount]->query_commodity_name();

        if ((reserv = query_commodity_reserve(coname)) <= 0)
        {
            nosell += ({ oblist[amount] });
            oblist -= ({ oblist[amount] });
            continue;
        }

        if (hook_commerce_sell_prevent_sell(oblist[amount]))
        {
            oblist -= ({ oblist[amount] });
            continue;
        }

        if (IS_HEAP_OBJECT(oblist[amount]))
        {
            if ((number = oblist[amount]->num_heap()) > reserv)
            {
                oblist[amount]->split_heap(number = reserv);
            }
        }
        else
        {
            number = 1;
        }

        stocka  = query_commodity_amount(coname);
        maxcap  = query_commodity_capacity(coname);
        comfee  = itof(oblist[amount]->query_commodity_value());
        comfee *= query_commodity_costf(coname);

        add_commodity_amount(coname, number);
        add_commodity_reserve(coname, -number);

        while (--number >= 0)
        {
            cost += (comfee * ((1.0 - (itof(stocka - (number + 1)) /
                itof(maxcap))) / 0.5));
        }
    }

    if (!sizeof(oblist))
    {
        /* this weird double map makes sure that the commersant gives */
        /* error message only once for each commodity type            */

        map(map(unique_array(nosell, &->query_commodity_name()),
            &operator([])(, 0)), hook_commerce_sell_object_noneed);
        return 1;
    }

    this_player()->add_exp_general(F_COMMERCE_GENEXP(cost));

    if (!pointerp(cost = give(ftoi(cost), this_player(), "", 0, 0, 1)))
    {
        if (!objectp(this_player()))
        {
            throw("this_player() is zero.\n");
        }

        return 1;
    }

    hook_commerce_sell_notify_finish(oblist,
        cost[SIZEOF_MONEY_TYPES..(2 * SIZEOF_MONEY_TYPES - 1)]);

    map(oblist, remove_commodity);

    return 1;
}

/*
 * Function name : stat_demanded_commerce
 * Description   : function returns the statistics of demanded commodity
 * Arguments     : void    - no arguments
 * Returns       : string  - the statistics of demanded commodity
 */
public nomask string
stat_demanded_commodity(string commod)
{
    mixed   record = commer_demand[commod];

    return sprintf("%-13s|%5d|%5.3O|%5d|%5d|%5d|\n",
        commod, record[COMM_UPSPAN] / 60, record[COMM_COSTFC],
        record[COMM_AMOUNT], record[COMM_RESERV], record[COMM_MAXCAP]);
}

/*
 * Function name : stat_supplied_commerce
 * Description   : function returns the statistics of supplied commodity
 * Arguments     : void    - no arguments
 * Returns       : string  - the statistics of supplied commodity
 */
public nomask string
stat_supplied_commodity(string commod)
{
    mixed   record = commer_supply[commod];
    string  result, *comreq;
    int     number;

    result = sprintf("%-13s|%5d|%5.3O|%5d|%5d|%5d|",
        commod, record[COMM_UPSPAN] / 60, record[COMM_COSTFC],
        record[COMM_AMOUNT], record[COMM_RESERV], record[COMM_MAXCAP]);

    if (!exist_supply_requirement(commod))
    {
        return result + "\n";
    }

    comreq = query_supply_requirement(commod);
    number = sizeof(comreq);

    while (--number >= 0)
    {
        result += sprintf(" %-15s (%5d)\n", comreq[number],
            query_supply_requirement(commod, comreq[number]));

        if (number >= 2)
        {
            result += sprintf("%42s |", "");
        }
        else if (number == 1)
        {
            result += sprintf("%'.'43s|", "");
        }
    }

    return result;
}

/*
 * Function name : stat_commerce
 * Description   : function returns the statistics of commerce module
 * Arguments     : void    - no arguments
 * Returns       : string  - the statistics of commerce module
 */
public nomask string
stat_commerce()
{
    string *commod, result = "";

    update_commerce();

    if (sizeof(commod = m_indices(commer_demand || ([ ]))))
    {
        result += sprintf("%'-'74s\nDemanded Com | Spn | Cof | Amo |" +
            " Res | Max |\n%'-'74s\n", "", "");
        result += implode(map(commod, stat_demanded_commodity), "");
    }

    if (sizeof(commod = m_indices(commer_supply || ([ ]))))
    {
        result += sprintf("%'-'74s\nSupplied Com | Spn | Cof | Amo |" +
            " Res | Max | Requirements\n%'-'74s\n", "", "");
        result += implode(map(commod, stat_supplied_commodity), "");
    }

    return (strlen(result) ? (result + sprintf("%'-'74s\n", "")) : "");
}

/*
 * === initiating functions ==============================================
 * =============================================================== *** ===
 */

/*
 * Function name : init_commerce
 * Description   : adds actions to the commerce object environment
 * Arguments     : void    - no arguments
 * Returns       : void    - nothing is returned
 */
public void
init_commerce()
{
    if (mappingp(commer_supply))
    {
        add_action(action_commerce_buy ,  "buy");
    }

    if (mappingp(commer_demand))
    {
        add_action(action_commerce_sell, "sell");
    }
}

/*
 * Function name : remove_commerce
 * Description   : saves commerce data into the savefile
 * Arguments     : void    - no arguments
 * Returns       : void    - nothing is returned
 */
public nomask void
remove_commerce()
{
    if (strlen(query_commerce_savefile()))
    {
        catch(this_object()->save_commerce());
    }
}

/*
 * Function name : save_commerce
 * Description   : function saves the vital part of commerce mappings
 * Arguments     : void    - no arguments
 * Returns       : void    - nothing is returned
 */
public nomask void
save_commerce()
{
    mapping record = ([]);
    string *commod;
    int     amount;

    if (!strlen(commer_svfile))
    {
        return;
    }

    commod = m_indices((commer_demand || ([])) + (commer_supply || ([])));
    amount = sizeof(commod);

    while (--amount >= 0)
    {
        record[implode(explode(commod[amount], " "), "+")] = ({
            query_commodity_amount(commod[amount]),
            query_commodity_reserve(commod[amount]), });
    }

    setuid(); seteuid(getuid());
    save_map(record, commer_svfile);
}

/*
 * Function name : load_commerce
 * Description   : function loads the vital part of commerce mappings
 * Arguments     : void    - no arguments
 * Returns       : void    - nothing is returned
 */
public nomask void
load_commerce()
{
    mapping record;
    string *commod, result;
    int     amount;

    if (!strlen(commer_svfile))
    {
        return;
    }

    setuid(); seteuid(getuid());

    if (!mappingp(record = restore_map(commer_svfile)))
    {
        return;
    }

    commod = m_indices(record);
    amount = sizeof(commod);

    while (--amount >= 0)
    {
        result = implode(explode(commod[amount], "+"), " ");

        set_commodity_amount(result, record[commod[amount]][0]);
        set_commodity_reserve(result, record[commod[amount]][1]);
    }
}

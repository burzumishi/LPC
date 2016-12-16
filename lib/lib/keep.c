/* 
 * /lib/keep.c
 *
 * This small module will enable the object it is inherited into to become
 * 'keepable'. This means that the player holding the object will have the
 * ability to set OBJ_M_NO_SELL flag in the object, indicating that he or
 * she wants to hold onto this object even if commands like 'sell all' are
 * given. In order to make the object 'keepable', all the wizard has to do
 * is inherit this module into his/her code. By default the object will not
 * be 'kept' though. To make the object 'kept' when it is cloned, the
 * function:
 *
 *     set_keep();   or   set_keep(1);
 *
 * must be called from the create function. As you see, set_keep() without
 * argument will be defaulted to 'keep' the object. To remove the keep
 * protection again, the function:
 *
 *     set_keep(0);
 *
 * can be called in the object. The player can call these functions by using
 * the mudlib-commands 'keep' and 'unkeep', defined in /cmd/live/things.c
 *
 * If the object in which this module is inherited is recoverable, it is
 * nice if the 'keep' status is recovered too. To do so, you have to make
 * the following calls from your query_recover() and init_recover()
 * functions.
 *
 * string
 * query_recover()
 * {
 *     return MASTER + ":" + <other recover stuff> + query_keep_recover();
 * }
 *
 * void
 * init_recover(string arg)
 * {
 *     <other recover stuff>
 *     init_keep_recover(arg);
 * }
 */

#pragma save_binary
#pragma strict_types

#include <language.h>
#include <stdproperties.h>

/* Prototype */
public int query_unsellable();

/* 
 * Function name: query_keepable
 * Description  : This function will always return true to signal that the
 *                object it is called in is indeed keepable. Notice that this
 *                does not say anything on whether the object is actually
 *                keep-protected at this point.
 * Returns      : int 1 - always.
 */
nomask int
query_keepable()
{
    return 1;
}

/*
 * Function name: keep_obj_m_no_sell
 * Description  : This function is called by VBFC from the OBJ_M_NO_SELL
 *                property if this object is 'keep' protected. If will return
 *                a proper fail message.
 * Returns      : string - the fail message.
 */
public string
keep_obj_m_no_sell()
{
    return capitalize(LANG_THESHORT(this_object())) + " rejects being sold " +
        "since it is 'keep' protected. In case you want to sell it, " +
	"'unkeep' it first.\n";
}

/*
 * Function name: set_keep
 * Description  : Call this function in order to set or remove the 'keep'
 *                protection of this object. If no argument is passed to the
 *                function, the default will be 1 - i.e. set the 'keep'
 *                protection. If the OBJ_M_NO_SELL property is not set to
 *                our own function, keep the item kept.
 * Arguments    : int 1 - set the 'keep' protection.
 *                    0 - remove the 'keep' protection.
 */
public void
set_keep(int keep = 1)
{
    if (keep)
    {
        if (!this_object()->query_prop_setting(OBJ_M_NO_SELL))
        {
            this_object()->add_prop(OBJ_M_NO_SELL, keep_obj_m_no_sell);
        }
    }
    else if (!query_unsellable())
    {
        this_object()->remove_prop(OBJ_M_NO_SELL);
    }
}

/*
 * Function name: remove_keep
 * Description  : Call this function in order to remove the 'keep' protection
 *                of this object. This is the same as doing: set_keep(0);
 */
public void
remove_keep()
{
    set_keep(0);
}

/* 
 * Function name: query_keep
 * Description  : Call this function to query the current 'keep' status of
 *                this object.
 * Returns      : int 1/0 - 'keep' protected or not.
 */
public int
query_keep()
{
    return (this_object()->query_prop_setting(OBJ_M_NO_SELL) != 0);
}

/*
 * Function name: query_unsellable
 * Description  : Find out whether this object is unsellable by definition.
 *                That is, if OBJ_M_NO_SELL is set, it wasn't set by us.
 * Returns      : int 1/0 - if true, it cannot be sold.
 */
public int
query_unsellable()
{
    mixed pvalue = this_object()->query_prop_setting(OBJ_M_NO_SELL);

    return (functionp(pvalue) &&
        !wildmatch("*->keep_obj_m_no_sell", function_name(pvalue)));
}

/*
 * Function name: appraise_keep
 * Description  : Composte a text about the keepability of this item.
 * Arguments    : int num - a certain number related to appraise skill.
 * Returns      : string - the description. Note it must start with a space.
 */
public string
appraise_keep(int num)
{
    if (query_keep())
    {
        return " It rejects being sold as it is keep protected.";
    }
    else
    {
        return " It could reject being sold by keep protecting it.";
    }
}

/*
 * Function name: query_keep_recover
 * Description  : This function will return the keep-recovery string.
 * Returns      : string - the keep-recovery string.
 */
public nomask string
query_keep_recover()
{
    /* We only need to add a recover string if the object is 'kept'. */
    if (query_keep())
    {
	return "~Kp~";
    }

    return "";
}

/*
 * Function name: init_keep_recover
 * Description  : After recovering this object, the recovery argument needs
 *                to be checked and the 'keep' protection needs to be
 *                enabled if the player had 'kept' the item before the
 *                reboot.
 * Arguments    : string arg - the recovery argument.
 */
public nomask void
init_keep_recover(string arg)
{
    set_keep(wildmatch("*~Kp~*", arg));
}

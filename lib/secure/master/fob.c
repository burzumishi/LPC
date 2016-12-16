/*
 * /secure/master/fob.c
 *
 * Subpart of /secure/master.c
 *
 * Handles all domain, wizard and application administration in the game.
 */

#include "/sys/composite.h"
#include "/sys/const.h"

/*
 * These global variables are stored in the KEEPERSAVE.
 */
private int     dom_count;      /* The next domain number to be added */
private mapping m_domains;      /* The domain mapping */
private mapping m_wizards;      /* The wizard mapping */
private mapping m_applications; /* The applications mapping */
private mapping m_global_read;  /* The global read mapping */
private mapping m_teams;        /* The arch team mapping */

/**************************************************************************
 *
 * The 'm_domains' mapping holds the domain name as index and an array with
 * the following items as value:
 *
 * [0] - The domain number. It is stored as tag in the player along with
 *       the quest bit set by the domain objects.
 *
 * [1] - The short name of the domain. This should be a unique abbreviation
 *       of exactly three characters.
 *
 * [2] - The name of the domain lord.
 *
 * [3] - The name of the steward of the domain, if any.
 *
 * [4] - An array holding the names of all members, including the lord and 
 *       the steward.
 *
 * [5] - The max number of members the domain wants to accept.
 *
 * [6] - The number of quest xp given to mortals
 *
 * [7] - The number of combat xp given to mortals
 *
 * [8] - The number of commands executed in the domain.
 *
 **************************************************************************
 *
 * The 'm_wizards' mapping holds the wizard name as index and an array with
 * the following items as value:
 *
 * [0] - The rank of the wizard.
 *
 * [1] - The level of the wizard.
 *
 * [2] - The name of the wizard who last changed the level/rank.
 *
 * [3] - The domain the wizard is a member of.
 *
 * [4] - The name of the wizard who last changed the domain.
 *
 * [5] - Rrestriction information regarding the wizard
 *
 * [6] - Mentor of the wizard, if any
 *
 * [7] - Students to the wizards, if any
 *
 **************************************************************************
 *
 * The 'm_applications' mapping has the domain-names as index and the values
 * are arrays with the names of the wizards applying for membership.
 *
 **************************************************************************
 *
 * The 'm_global_read' mapping contains all wizards who have global read
 * access. Their names are the incides. The values is an array with the
 * following information:
 *
 * [0] - The wizards who granted the access.
 *
 * [1] - A short string describing the reason the wizard has global read.
 *
 **************************************************************************
 *
 */

/* The maximum number of members a Lord may allow to his domain. */
#define DOMAIN_MAX        9

/* These are the indices to the arrays in the domain-mapping. */
#define FOB_DOM_NUM       0
#define FOB_DOM_SHORT     1
#define FOB_DOM_LORD      2
#define FOB_DOM_STEWARD   3
#define FOB_DOM_MEMBERS   4
#define FOB_DOM_MAXSIZE   5
#define FOB_DOM_QXP       6
#define FOB_DOM_CXP       7
#define FOB_DOM_CMNDS     8

/* These are the indices to the arrays in the wizard-mapping. */
#define FOB_WIZ_RANK      0
#define FOB_WIZ_LEVEL     1
#define FOB_WIZ_CHLEVEL   2
#define FOB_WIZ_DOM       3
#define FOB_WIZ_CHDOM     4
#define FOB_WIZ_RESTRICT  5
#define FOB_WIZ_MENTOR    6
#define FOB_WIZ_STUDENTS  7

/*
 * Function name: load_fob_defaults
 * Description  : This function is called from master.c when the KEEPERAVE
 *                file cannot be found. The defined values can be found in
 *                config.h.
 */
static void
load_fob_defaults()
{
    m_wizards = DEFAULT_WIZARDS;
    m_domains = DEFAULT_DOMAINS;
    dom_count = m_sizeof(m_domains);
    m_applications = ([ ]);
    m_global_read = ([ ]);
    m_teams = ([ ]);
}

/*
 * Function name: getwho
 * Description  : This function gets the name of the interactive command
 *                giver. The euid of the interactive player and the euid
 *                of the previous object must be equal in order to function.
 * Returns      : string - the name or "" in case of inconsistencies.
 */
static string
getwho()
{
    string euid;

    euid = geteuid(this_interactive());
    return ((euid == geteuid(previous_object())) ? euid : "");
}

/************************************************************************
 *
 * The domain administration code.
 *
 */

/*
 * Function name: query_domain_name
 * Description  : Find the domain name from the number.
 * Arguments    : int number - the domain number.
 * Returns      : string - the domain name if found, else 0.
 */
string
query_domain_name(int number)
{
    foreach(string dname: m_indices(m_domains))
    {
        if (m_domains[dname][FOB_DOM_NUM] == number)
            return dname;
    }

    return 0;
}

/*
 * Function name: query_domain_number
 * Description  : Find the number of a domain.
 * Arguments    : string dname - the name of the domain.
 * Returns      : int - the number if found, -1 otherwise.
 */
int
query_domain_number(string dname)
{
    dname = capitalize(dname);

    if (!sizeof(m_domains[dname]))
        return -1;

    return m_domains[dname][FOB_DOM_NUM];
}

/*
 * Function name: query_domain_short
 * Description  : Find the short name of the domain.
 * Arguments    : string dname - the name of the domain.
 * Returns      : string - the short name if found, "" otherwise.
 */
string
query_domain_short(string dname)
{
    dname = capitalize(dname);

    if (!sizeof(m_domains[dname]))
        return "";

    return m_domains[dname][FOB_DOM_SHORT];
}

/*
 * Function name: set_domain_short
 * Description  : Set the short name of a domain. The short name is an
 *                appropriate abbreviation of exactly three characters of
 *                the domain-name. This function may only be called from
 *                the Lord soul, so we do not have to make additional
 *                checks.
 * Arguments    : string dname - the name of the domain (capitalized).
 *                string sname - the short name of the domain (lower case).
 * Returns      : int 1/0 - success/failure.
 */
int
set_domain_short(string dname, string sname)
{
    /* This function may only be called from the Lord soul. */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;

    m_domains[dname][FOB_DOM_SHORT] = sname;
    save_master();
    return 1;
}

/*
 * Function name: query_domain_lord
 * Description  : Find the lord of a domain.
 * Arguments    : string dname - the name of the domain.
 * Returns      : string the domain lord if found, "" otherwise.
 */
string
query_domain_lord(string dname)
{
    dname = capitalize(dname);

    if (!sizeof(m_domains[dname]))
        return "";

    return m_domains[dname][FOB_DOM_LORD];
}

/*
 * Function name: query_domain_steward
 * Description  : Find the steward of a domain.
 * Arguments    : string dname - the name of the domain.
 * Returns      : string the steward if the domain exists, "" otherwise.
 */
string
query_domain_steward(string dname)
{
    dname = capitalize(dname);

    if (!sizeof(m_domains[dname]))
        return "";

    return m_domains[dname][FOB_DOM_STEWARD];
}

/*
 * Function name: query_domain_members
 * Description  : Find and return the member array of a given domain.
 * Arguments    : string dname - the domain.
 * Returns      : string * - the array if found, ({ }) otherwise.
 */
string *
query_domain_members(string dname)
{
    dname = capitalize(dname);

    if (!sizeof(m_domains[dname]))
        return ({ });

    return secure_var(m_domains[dname][FOB_DOM_MEMBERS]);
}

/*
 * Function name: query_domain_list
 * Description  : Return a list of all domains.
 * Returns      : string * - the list.
 */
string *
query_domain_list()
{
    return m_indices(m_domains);
}

/*
 * Function name: set_domain_max
 * Description  : Set maximum number of wizards wanted. This function may
 *                only be called from the Lord soul so we do not have to
 *                make any additional checks.
 * Arguments    : string dname - the capitalized domain name.
 *                int    max   - the new maximum.
 * Returns      : int 1/0 - success/failure.
 */
int
set_domain_max(string dname, int max)
{
    /* May only be called from the Lord soul. */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;

    m_domains[dname][FOB_DOM_MAXSIZE] = max;
    save_master();
    return 1;
}

/*
 * Function name: query_domain_max
 * Description  : Find and return the maximum number of wizards wanted. 
 * Arguments    : string dname - the name of the domain.
 * Returns      : int - the max number if found, -1 otherwise.
 */
int
query_domain_max(string dname)
{
    dname = capitalize(dname);

    if (!sizeof(m_domains[dname]))
        return -1;

    return m_domains[dname][FOB_DOM_MAXSIZE];
}

/*
 * Function name: query_default_domain_max
 * Description  : Get the default maximum number of members in a domain.
 * Returns      : int - the value defined in DOMAIN_MAX.
 */
int
query_default_domain_max()
{
    return DOMAIN_MAX;
}

/*
 * Function name: make_domain
 * Description  : Create a new domain.
 * Arguments    : string dname - the domain name.
 *                string sname - the short domain name.
 *                string wname - the name of the domain lord.
 * Returns      : int 1/0 - success/failure.
 */
int
make_domain(string dname, string sname, string wname)
{
    string cmder;

    /* Only accept calls from the arch command soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
        return 0;

    cmder = getwho();
    if (sizeof(m_domains[dname]))
    {
        notify_fail("The domain " + dname + " already exists.\n");
        return 0;
    }

    if ((strlen(dname) > 11) ||
        (strlen(sname) != 3))
    {
        notify_fail("The domain name must be 11 characters long at most and " +
            "the short name must be exactly three characters long.\n");
        return 0;
    }

    if (sizeof(filter(m_indices(m_domains),
            &operator(==)(sname) @ query_domain_short)))
        notify_fail("The short domain name " + sname +
		    " is already in use.\n");

    if ((!pointerp(m_wizards[wname])) ||
        (m_wizards[wname][FOB_WIZ_RANK] != WIZ_APPRENTICE))
    {
        notify_fail("The player " + capitalize(wname) +
            " is not an apprentice.\n");
        return 0;
    }

    set_auth(this_object(), "root:root");

    switch(file_size("/d/" + dname))
    {
    case -1:
        /* Check for old discarded instances of the domain. If there is one,
         * we must use it or else you shall have to rename or remove it
         * first.
         */
        if (file_size(DISCARD_DOMAIN_DIR + "/" + dname) == -2)
        {
            if (rename((DISCARD_DOMAIN_DIR + "/" + dname), ("/d/" + dname)))
                write("Revived the old discarded " + dname + ".\n");
            else
            {
                write("Failed to discard old instance of " + dname +
                    ". Do this manually or rename it.\n");
                return 1;
            }
        }
        else
        {
            /* Create the domain subdirectory. */
            if (!mkdir("/d/" + dname))
            {
                write("Can not create the domain subdir!\n");
                return 1;
            }

            write("Created directory for new domain " + dname + ".\n");
        }
        break;

    case -2:
        write("Directory for " + dname + " already exists.\n");
        break;

    default:
        write("There is a file named /d/" + dname +
              ", making it impossible to create the domain.\n");
        return 1;
    }

    /* Add the domain entry to the domain mepping. */
    m_domains[dname] =
        ({ dom_count++, sname, wname, "", ({ }), DOMAIN_MAX, 0, 0, 0 });

    /* Make the apprentice a lord. This will also save the domain mapping. */
    add_wizard_to_domain(dname, wname, cmder);
    do_change_rank(wname, WIZ_LORD, cmder);

    return 1;
}

/*
 * Function name: remove_domain
 * Description  : Remove a domain, demote the wizards, move the code.
 * Arguments    : string dname - the domain to remove.
 * Returns      : int 1/0 - success/failure.
 */
int
remove_domain(string dname)
{
    int    index;
    int    size;
    string cmder;

    /* May only be called from the arch soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
        return 0;

    cmder = getwho();
    dname = capitalize(dname);
    if (!sizeof(m_domains[dname]))
    {
        notify_fail("No such domain '" + dname + "'.\n");
        return 0;
    }

    if (dname == WIZARD_DOMAIN)
    {
        write("The domain " + WIZARD_DOMAIN + " cannot be removed.\n");
        return 1;
    }

    if (file_size(DISCARD_DOMAIN_DIR + "/" + dname) != -1)
    {
        notify_fail("There already is something called " + DISCARD_DOMAIN_DIR +
            "/" + domain + ". Rename or remove this first.\n");
        return 0;
    }

    foreach(string wname: m_domains[dname][FOB_DOM_MEMBERS])
    {
        /* Don't demote mages, arches and keepers. */
        switch(m_wizards[wname][FOB_WIZ_RANK])
        {
        case WIZ_MAGE:
        case WIZ_ARCH:
        case WIZ_KEEPER:
            break;

        default:
            do_change_rank(wname, WIZ_APPRENTICE, cmder);
            write("Demoted " + capitalize(wname) +
                " to apprentice.\n");
        }

        /* Though expel everyone. */
        add_wizard_to_domain("", wname, cmder);
        write("Expelled " + capitalize(wname) + ".\n");
    }

    /* Remove all sanctions given out to or received by the domain. */
    remove_all_sanctions(dname);

    /* Move the domain directory to the discarded domain directory. */
    mkdir(DISCARD_DOMAIN_DIR);
    if (!rename("/d/" + dname, DISCARD_DOMAIN_DIR + "/" + dname))
    {
        write("Unable to move the domain directory to " + DISCARD_DOMAIN_DIR +
            ".\n");
        return 1;
    }

    /* Delete the domain from the domain mapping. */
    m_delkey(m_domains, dname);
    save_master();

    write("You have just obliterated " + dname + ".\n");
    return 1;
}

/*
 * Function name: tell_domain
 * Description  : Tell something to all in the domain save one, who gets a
 *                special message.
 * Arguments    : string dname - the domain.
 *                string wname - special message to this wiz.
 *                string wmess - wizard message.
 *                string dmess - domain message.
 */
static void 
tell_domain(string dname, string wname, string wmess, string dmess)
{
    object wiz;
    string *wlist;
    object *wizards;

    if (objectp(wiz = find_player(wname)))
        tell_object(wiz, wmess);

    wlist = (string *)m_domains[dname][FOB_DOM_MEMBERS] - ({ wname });
    wizards = filter(map(wlist, find_player), objectp);
    wizards->catch_tell(dmess);
}

/*
 * Function name: transform_mortal_into_wizard
 * Description  : This function gives the mortal the apprentice scroll and
 *                alters the start location of the player, both if they
 *                player is logged in or now.
 * Arguments    : string wname - the name of the mortal.
 *                string cmder - the name of the wizard doing the honour.
 */
static void
transform_mortal_into_wizard(string wname, string cmder)
{
    object  wizard;
    object  scroll;
    object *players;
    mapping playerfile;
    int     fingered;

    /* Update the wizard-mapping. This just adds an empty slot for the
     * player. He isn't apprentice yet.
     */
    m_wizards[wname] = ({ WIZ_MORTAL, WIZ_RANK_START_LEVEL(WIZ_MORTAL),
                          cmder, "", cmder, RESTRICT_NEW_WIZ, "", ({}) });
    save_master();

    if (objectp(wizard = find_player(wname)))
    {
        if (catch(scroll = clone_object(APPRENTICE_SCROLL_FILE)))
        {
            write("Error cloning the apprentice scroll.\n");
            tell_object(wizard, "Error cloning the apprentice scroll.\n");
        }
        else
        {
            write("Apprentice scroll moved to " + capitalize(wname) + ".\n");
            tell_object(wizard, "You have been given a scroll containing " +
                "valuable information.\n");
            scroll->move(wizard, 1);
        }

        wizard->set_default_start_location(WIZ_ROOM);
        wizard->save_me(1);        
    }
    else
    {
        playerfile = restore_map(PLAYER_FILE(wname));

        if (!pointerp(playerfile["auto_load"]))
            playerfile["auto_load"] = ({ });

        playerfile["auto_load"] += ({ APPRENTICE_SCROLL_FILE });
        playerfile["default_start_location"] = WIZ_ROOM;

        save_map(playerfile, PLAYER_FILE(wname));

        fingered = 1;
        wizard = SECURITY->finger_player(wname);
    }

    players = users() - ({ this_interactive(), wizard });
    players -= QUEUE->queue_list(0);

    players->catch_tell("There is a flash of pure magic, and suddenly you " +
        "realize that " + capitalize(wname) + " has been " +
        "accepted into the ranks of the wizards.\n");

    SECURITY->log_public("NEW_WIZ",
        sprintf("%s   %-11s   Avg = %3d; Age = %3d\n", ctime(time()),
        capitalize(wname), wizard->query_average_stat(),
        (wizard->query_age() / 43200)), -1);

    /* Clean up after ourselves. */
    if (fingered)
        do_debug("destroy", wizard);
}

/************************************************************************
 *
 * The wizard administration code.
 *
 */

/*
 * Function name: add_wizard_to_domain
 * Description  : Add a wizards to a domain and removes the wizard from his/
 *                her pervious domain. If you add a wizard to the domain "",
 *                it removes the wizard from his current domain.
 *                This is an interal function only that just adds or removes
 *                the people without additional checks. Those must have been
 *                made earlier. Note that this function does not alter the
 *                rank of the wizard. This must have been altered before.
 * Arguments    : string dname - the domain to add the wizard to. If "",
 *                               the wizard is removed from his/her domain.
 *                string wname - the wizard to add/remove.
 *                string cmder - who does it.
 * Returns      : int 1/0 - success/failure.
 */
static int
add_wizard_to_domain(string dname, string wname, string cmder)
{
    string old_domain;
    string new_dir;

    dname = capitalize(dname);
    wname = lower_case(wname);
    old_domain = m_wizards[wname][FOB_WIZ_DOM];

    /* Mages, arches and keepers cannot be without a domain. They will be
     * moved to WIZARD_DOMAIN if you try to remove them from their domain.
     */
    if ((!strlen(dname)) &&
        ((m_wizards[wname][FOB_WIZ_RANK] == WIZ_MAGE) ||
         (m_wizards[wname][FOB_WIZ_RANK] >= WIZ_ARCH)))
        dname = WIZARD_DOMAIN;

    m_wizards[wname][FOB_WIZ_DOM] = dname;
    m_wizards[wname][FOB_WIZ_CHDOM] = cmder;


    /* If the person leaves an old domain, update the membership and tell
     * the people.
     */
    if (strlen(old_domain))
    {
        m_domains[old_domain][FOB_DOM_MEMBERS] -= ({ wname });

        tell_domain(old_domain, wname, ("You are no longer a member of " +
            old_domain + ".\n"), (capitalize(wname) +
            " is no longer a member of " + old_domain + ".\n"));
    }
    else
        old_domain = WIZARD_DOMAIN;

    /* Leaving the domain and not joining another domain. */
    if (!strlen(dname))
    {
        save_master();

        return 1;
    }

    /* Joining a new domain means no more applications. */
    remove_all_applications(wname);

    /* Add him/her to the domain-list. */
    m_domains[dname][FOB_DOM_MEMBERS] += ({ wname });

    tell_domain(dname, wname, ("You are now a member of " + dname +".\n"),
        capitalize(wname) + " is now a member of " + dname + ".\n");

    set_auth(this_object(), "root:root");

    new_dir = "/w/" + wname;

    /* If there isn't a directory yet, create one. */
    if (file_size(new_dir) != -2)
    {
        /* This isn't nice, but the player still is a new member of the
         * domain. That hasn't changed.
         */
        if (file_size(new_dir) != -1)
        {
            write("Failed to create " + new_dir +
                " since there is already a file with that name.\n");
            return 1;
        }

        mkdir(new_dir);
    }

    return 1;
}

/*
 * Function name: draft_wizard_to_domain
 * Description  : With this function an archwizard or keeper can draft a
 *                wizard to a domain.
 * Arguments    : string dname - the domain to draft the wizard to.
 *                string wname - the wizard to draft.
 * Returns      : int 1/0 - success/failure.
 */
int
draft_wizard_to_domain(string dname, string wname)
{
    string cmder;

    /* May only be called from the arch soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
        return 0;

    cmder = getwho();
    dname = capitalize(dname);
    wname = lower_case(wname);

    /* Check validity of the wizard name. */
    if (!pointerp(m_wizards[wname]))
    {
        notify_fail("There is no wizard named " + capitalize(wname) + ".\n");
        return 0;
    }

    /* Check validity of the domain name. */
    if (!pointerp(m_domains[dname]))
    {
        notify_fail("There is no domain named " + dname + ".\n");
        return 0;
    }

    /* No vacancies in the domain. */
    if (sizeof(m_domains[dname][FOB_DOM_MEMBERS]) >=
        m_domains[dname][FOB_DOM_MAXSIZE])
    {
        notify_fail("There are no vacancies in " + dname + ".\n");
        return 0;
    }

    /* Don't draft people who are in another domain. */
    if (strlen(m_wizards[wname][FOB_WIZ_DOM]) &&
        (m_wizards[wname][FOB_WIZ_DOM] != WIZARD_DOMAIN))
    {
        notify_fail(capitalize(wname) + " already is a member of " + 
            m_wizards[wname][FOB_WIZ_DOM] + ".\n");
        return 0;
    }

    /* Only draft apprentices. */
    if ((m_wizards[wname][FOB_WIZ_RANK] != WIZ_APPRENTICE) &&
        (m_wizards[wname][FOB_WIZ_RANK] < WIZ_NORMAL))
    {
        notify_fail(capitalize(wname) + " is not an apprentice.\n");
        return 0;
    }

    /* Apprentices should be made full wizard. */
    if (m_wizards[wname][FOB_WIZ_RANK] == WIZ_APPRENTICE)
        do_change_rank(wname, WIZ_NORMAL, cmder);

    write("Drafting " + capitalize(wname) + " to " + dname + ".\n");
    return add_wizard_to_domain(dname, wname, cmder);
}

/*
 * Function name: expel_wizard_from_domain
 * Description  : Expel a wizard from a domain.
 * Arguments    : string wname - the wizard to expel.
 * Returns      : int 1/0 - success/failure.
 */
int
expel_wizard_from_domain(string wname)
{
    string cmder;

    /* May only be called from the Lord soul. */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;

    wname = lower_case(wname);
    if (!strlen(wname))
    {
        notify_fail("Expel whom?\n");
        return 0;
    }

    if (!sizeof(m_wizards[wname]))
    {
        notify_fail(capitalize(wname) + " is a mortal player!\n");
        return 0;
    }

    if (!strlen(m_wizards[wname][FOB_WIZ_DOM]))
    {
        notify_fail(capitalize(wname) + " is not a member of any domain.\n");
        return 0;
    }

    /* Lords may only boot their own domain members. Use <= WIZ_LORD to
     * also include expelling stewards.
     */
    cmder = getwho();
    if ((m_wizards[cmder][FOB_WIZ_RANK] <= WIZ_LORD) &&
        (m_wizards[wname][FOB_WIZ_DOM] != m_wizards[cmder][FOB_WIZ_DOM]))
    {
        write(capitalize(wname) + " is not a member of your domain " +
              m_wizards[wname][FOB_WIZ_DOM] + ".\n");
        return 1;
    }

    /* Try to add mages, arches and keepers to WIZARD_DOMAIN. */
    if ((m_wizards[wname][FOB_WIZ_RANK] == WIZ_MAGE) ||
        (m_wizards[wname][FOB_WIZ_RANK] >= WIZ_ARCH))
    {
        if (m_wizards[wname][FOB_WIZ_DOM] == WIZARD_DOMAIN)
        {
            write(capitalize(wname) + " cannot leave the domain " +
                WIZARD_DOMAIN + ".\n" );
            return 1;
        }

        return add_wizard_to_domain(WIZARD_DOMAIN, wname, cmder);
    }

    /* Demote them and kick them out of the domain. */
    do_change_rank(wname, WIZ_APPRENTICE, cmder);
    return add_wizard_to_domain("", wname, cmder);
}

/*
 * Function name: leave_domain
 * Description  : Someone wants to leave his/her domain.
 * Returns      : int 1/0 - success/failure.
 */
int
leave_domain()
{
    string cmder;

    /* May only be called from the normal wizards soul. */
    if (!CALL_BY(WIZ_CMD_NORMAL))
        return 0;

    /* Mages, arches and keepers cannot leave WIZARD_DOMAIN. */
    cmder = getwho();
    if ((m_wizards[cmder][FOB_WIZ_DOM] == WIZARD_DOMAIN) &&
        ((m_wizards[cmder][FOB_WIZ_RANK] >= WIZ_ARCH) ||
         (m_wizards[cmder][FOB_WIZ_RANK] == WIZ_MAGE)))
    {
        write("You cannot leave " + WIZARD_DOMAIN + " for good.\n");
        return 1;
    }

    /* Domain-member or not? */
    if (!strlen(m_wizards[cmder][FOB_WIZ_DOM]))
    {
        write("You are not a member of any domain.\n");
        return 1;
    }

    /* Demote the wizard if he isn't a mage, arch or keeper. */
    if ((m_wizards[cmder][FOB_WIZ_RANK] != WIZ_MAGE) &&
        (m_wizards[cmder][FOB_WIZ_RANK] < WIZ_ARCH))
        do_change_rank(cmder, WIZ_APPRENTICE, cmder);

    /* Add the wizard to the empty domain, i.e leave the domain. */
    return add_wizard_to_domain("", cmder, cmder);
}

/*
 * Function name: rename_wizard
 * Description  : A service function to update the name of the wizard in
 *                case a wizard changes his/her name. It makes no validity
 *                checks.
 * Arguments    : string oldname - the old name of the wizard.
 *                string newname - the new name of the wizard.
 */
static int
rename_wizard(string oldname, string newname)
{
    string dname = m_wizards[oldname][FOB_WIZ_DOM];

    /* Rename the wizard in the wizard mapping. */
    write("Wizard status copied to " + capitalize(newname) + ".\n");
    m_wizards[newname] = secure_var(m_wizards[oldname]);
    m_delkey(m_wizards, oldname);

    /* Update global read. */
    if (m_global_read[oldname])
    {
        write("Global read copied to " + capitalize(newname) + ".\n");
        m_global_read[newname] = secure_var(m_global_read[oldname]);
        m_delkey(m_global_read, oldname);
    }

    /* Update domain information. */
    if (strlen(dname))
    {
        write("Domain membership of " + capitalize(newname) + " updated.\n");
        m_domains[dname][FOB_DOM_MEMBERS] -= ({ oldname });
        m_domains[dname][FOB_DOM_MEMBERS] += ({ newname });

        switch(m_wizards[newname][FOB_WIZ_RANK])
        {
        case WIZ_STEWARD:
            m_domains[dname][FOB_DOM_STEWARD] = newname;
            break;

        case WIZ_LORD:
            m_domains[dname][FOB_DOM_LORD] = newname;
            break;
        }

        if (rename(("/w/" + oldname), ("/w/" + newname)))
            write("Home directory successfully renamed.\n");
        else
            write("Failed to rename home directory.\n");
    }

    save_master();
    log_file("LEVEL",
        sprintf("%s %-11s: renamed to %-11s by %s.\n",
        ctime(time()),
        capitalize(oldname),
        capitalize(newname),
        capitalize(this_interactive()->query_real_name())));
}

/************************************************************************
 *
 * The applications administration code.
 *
 */

/*
 * Function name: apply_to_domain
 * Description  : Apply to a domain.
 * Arguments    : string dname - domain the application goes to.
 * Returns      : int 1/0 - success/failure.
 */
int
apply_to_domain(string dname)
{
    string wname;
    string *appl;
    object wiz;

    /* May only be called from the apprentice soul. */
    if (!CALL_BY(WIZ_CMD_APPRENTICE))
        return 0;

    if (!strlen(dname))
    {
        notify_fail("Apply to what domain?\n");
        return 0;
    }

    dname = capitalize(dname);
    wname = getwho();
    
    switch(m_wizards[wname][FOB_WIZ_RANK])
    {
    case WIZ_MAGE:
    case WIZ_ARCH:
    case WIZ_KEEPER:
        if (m_wizards[wname][FOB_WIZ_DOM] != WIZARD_DOMAIN)
        {
            notify_fail("You are already a member of the domain " + 
                m_wizards[wname][FOB_WIZ_DOM] + ".");
            return 0;
        }
        break;

    case WIZ_PILGRIM:
        notify_fail("Pilgrims are doomed to wander the world; " +
            "they may not settle down.\n");
        return 0;
        break;

    default:
        if (m_wizards[wname][FOB_WIZ_DOM] != "")
        {
            notify_fail("You are already a member of the domain " + 
                m_wizards[wname][FOB_WIZ_DOM] + ".\n");
            return 0;
        }
        break;
    }

    if (!sizeof(m_domains[dname]))
    {
        notify_fail("There is no domain named '" + dname + "'.\n");
        return 0;
    }

    /* See if there is an array of people for that domain already. */
    if (!pointerp(m_applications[dname]))
        m_applications[dname] = ({ });

    /* else see if the player already applied. */
    else if (member_array(wname, m_applications[dname]) != -1)
    {
        write("You already filed an application to " + dname + "!\n");
        return 1;
    }

    /* See whether there is room in the domain. This does not terminate
     * the application process if it fails.
     */
    if (sizeof(m_domains[dname][FOB_DOM_MEMBERS]) >=
        m_domains[dname][FOB_DOM_MAXSIZE])
    {
        write("The domain " + dname + " is already full, but your " +
              "application will still be registered.\n");
    }

    /* Add the person to the list of people who applied. */
    m_applications[dname] += ({ wname });
    save_master();

    /* Tell the wizard and the members in the domain. */
    tell_domain(dname, wname, ("You applied to " + dname + ".\n"),
        (capitalize(wname) + " just applied for membership to " +
        dname + ".\n"));

    return 1;
}

/*
 * Function name: accept_application
 * Description  : A lord accepts an application. 
 * Arguments    : string wname - the wizard to accept.
 * Returns      : int 1/0 - success/failure.
 */
int
accept_application(string wname)
{
    string dname;
    string cmder;

    /* May only be called from the Lord soul. */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;

    if (!strlen(wname))
    {
        notify_fail("Accept which wizard?\n");
        return 0;
    }

    cmder = getwho();
    if (m_wizards[cmder][FOB_WIZ_RANK] > WIZ_LORD)
    {
        write("You are not the liege or steward of any domain!\n");
        return 1;
    }

    wname = lower_case(wname);
    dname = m_wizards[cmder][FOB_WIZ_DOM];

    if (sizeof(m_domains[dname][FOB_DOM_MEMBERS]) >=
        m_domains[dname][FOB_DOM_MAXSIZE])
    {
        notify_fail("There are no vacancies in " + dname + ".\n");
        return 0;
    }

    if (!pointerp(m_applications[dname]))
    {
        write("Nobody has asked to join your domain.\n");
        return 1;
    }
    else if (member_array(wname, m_applications[dname]) == -1)
    {
        write(capitalize(wname) + " has not asked to join your domain.\n");
        return 1;
    }

    if ((m_wizards[wname][FOB_WIZ_RANK] != WIZ_APPRENTICE) &&
         (m_wizards[wname][FOB_WIZ_DOM] != WIZARD_DOMAIN))
    {
        notify_fail(capitalize(wname) +
            " is neither an apprentice, nor a member of the domain " +
            WIZARD_DOMAIN + ".\n");
        return 0;
    }

    /* Joining means no more applications. */
    remove_all_applications(wname);

    /* People who aren't a wizard already should become full wizard. */
    if (m_wizards[wname][FOB_WIZ_RANK] == WIZ_APPRENTICE)
        do_change_rank(wname, WIZ_NORMAL, cmder);

    return add_wizard_to_domain(dname, wname, cmder);
}

/*
 * Function name: deny_application
 * Description  : The lord denies an application of a wizard.
 * Arguments    : string wname - the wizard name.
 * Returns      : int 1/0 - success/failure.
 */
int
deny_application(string wname)
{
    string dname;
    string cmder;
    object wiz;

    /* May only be called from the Lord soul. */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;
    
    cmder = getwho();
    if (m_wizards[cmder][FOB_WIZ_RANK] > WIZ_LORD)
    {
        notify_fail("You are not the liege or steward of any domain!\n");
        return 0;
    }

    wname = lower_case(wname);
    dname = m_wizards[cmder][FOB_WIZ_DOM];

    if (!pointerp(m_applications[dname]))
    {
        write("Nobody has asked to join your domain.\n");
        return 1;
    }
    else if (member_array(wname, m_applications[dname]) == -1)
    {
        write(capitalize(wname) + " has not asked to join " + dname + ".\n");
        return 1;
    }

    /* Remove the application. */
    m_applications[dname] -= ({ wname });

    /* Remove the domain-entry if this was the last application. */
    if (!sizeof(m_applications[dname]))
        m_delkey(m_applications, dname);

    if (objectp(wiz = find_player(wname)))
        tell_object(wiz, "Your application to the domain '" + dname +
		    "' was denied.\n");

    write("You denied the application to " + dname + " by " +
          capitalize(wname) + ".\n");
    return 1;
}

/*
 * Function name: regret_application
 * Description  : A wizard regrets an application to a domain.
 * Arguments    : string dname - the domain name.
 * Returns      : int 1/0 - success/failure.
 */
int
regret_application(string dname)
{
    string wname;

    /* May only be called from the apprentice soul. */
    if (!CALL_BY(WIZ_CMD_APPRENTICE))
        return 0;

    if (!strlen(dname))
    {
        notify_fail("Regret your application to which domain?");
        return 0;
    }

    wname = getwho();
    dname = capitalize(dname);

    /* See if the wizard applied to the domain. */
    if (member_array(wname, m_applications[dname]) == -1)
    {
        notify_fail("You have no application pending to the domain " +
            dname + ".\n");
        return 0;
    }

    /* Remove the application from the list. */
    m_applications[dname] -= ({ wname });

    /* If there are no other applications, remove the empty array. */
    if (!sizeof(m_applications[dname]))
        m_delkey(m_applications, dname);

    save_master();

    tell_domain(dname, wname, ("Regretted your application to " + dname +
        ".\n"), (capitalize(wname) + " just regretting applying to " +
        dname + ".\n"));
    return 1;
}

/*
 * Function name: remove_all_applications
 * Description  : Remove all application by a wizard from the application list.
 * Arguments    : string wname - the name of the wizard in lower case.
 */
static void
remove_all_applications(string wname)
{
    foreach(string dname: m_indices(m_applications))
    {
        /* Remove the application from the wizard if it exists. */
        if (pointerp(m_applications[dname]))
        {
            write("Removing application from " + capitalize(wname) + " to " +
                dname + ".\n");
            m_applications[dname] -= ({ wname });

            /* If this was the last wizard, remove the domain from the list. */
            if (!sizeof(m_applications[dname]))
                m_delkey(m_applications, dname);
        }
    }

    /* Save the master object. */
    save_master();
}

/*
 * Function name: filter_applications
 * Description  : This function checks whether a particular wizard is in the
 *                list of people who have applied to a domain. 
 * Arguments    : string *wizards - the people who have applied to a domain.
 *                string wname    - the wizard to check.
 * Returns      : int 1/0 - true if the wizard applied to the domain.
 */
static int
filter_applications(string *wizards, string wname)
{
    return (member_array(wname, wizards) != -1);
}

/*
 * Function name: list_applications_by_wizard
 * Description  : This function lists all applications made by a wizard.
 * Arguments    : string wname  - the name of the wizard to list.
 *                int list_self - true if someone inquires about himself.
 * Returns      : int 1/0 - success/failure.
 */
static int
list_applications_by_wizard(string wname, int list_self)
{
    mapping domains;

    wname = lower_case(wname);
    domains = filter(m_applications, &filter_applications(, wname));

    wname = (list_self ? "You have" : (capitalize(wname) + " has"));

    if (!m_sizeof(domains))
    {
        write(wname + " no pending applications to any domain.\n");
        return 1;
    }

    write(wname + " applications pending to " +
        COMPOSITE_WORDS(m_indices(domains)) + ".\n");
    return 1;
}

/*
 * Function name: list_applications
 * Description  : List applications. Domain-members may see the people that
 *                have applied to the domain. Non-domain members may see
 *                only their own applications. Arches and keepers may see
 *                all applications.
 * Arguments    : string str - the command line argument.
 * Returns      : int 1/0 - success/failure.
 */
int
list_applications(string str)
{
    string cmder;
    string dname;
    string *words;
    int    index;
    int    size;
    int    rank;

    cmder = getwho();
    if (!strlen(cmder))
    {
        notify_fail("You are not allowed to do this.\n");
        return 0;
    }

    rank = m_wizards[cmder][FOB_WIZ_RANK];

    /* Arches or keepers. */
    if (rank >= WIZ_ARCH)
    {
        /* Basic operation: list all applications. */
        if (!strlen(str))
        {
            if (!m_sizeof(m_applications))
            {
                write("No applications have been filed.\n");
                return 1;
            }

            write("Domain      Applications\n----------- ------------\n");

            foreach(string word: m_indices(m_applications))
            {
                write(FORMAT_NAME(word) + " " +
                    COMPOSITE_WORDS(sort_array(map(m_applications[word],
                    capitalize))) + "\n");
            }
            return 1;
        }

        words = explode(str, " ");
        if ((sizeof(words) != 2) ||
          (words[0] != "clear"))
        {
            notify_fail("Syntax: applications clear <domain> / <wizard>\n");
            return 0;
        }

        /* Remove all applications to a certain domain. */
        if (sizeof(m_domains[capitalize(words[1])]))
        {
            dname = capitalize(words[1]);
            if (!sizeof(m_applications[dname]))
            {
                write("No apprentices applied to " + dname + ".\n");
                return 1;
            }

            m_delkey(m_applications, dname);
            save_master();
            write("Removed all applications to " + dname + ".\n");
            return 1;
        }

        /* Remove all applications by a certain player. */
        write("Checking applications for " + capitalize(words[1]) + ".\n");
        remove_all_applications(lower_case(words[1]));
        return 1;
    }

    /* People in a domain may list the applications to their domain. */
    if ((rank >= WIZ_NORMAL) &&
        (m_wizards[cmder][FOB_WIZ_DOM] != WIZARD_DOMAIN))
    {
        if (strlen(str) &&
            (rank == WIZ_LORD))
            return list_applications_by_wizard(str, 0);

        if (!sizeof(m_applications[m_wizards[cmder][FOB_WIZ_DOM]]))
        {
            write("No wizards have applied to your domain.\n");
            return 1;
        }            

        write("The following people have applied to your domain: " +
            COMPOSITE_WORDS(sort_array(map(m_applications[m_wizards[cmder][FOB_WIZ_DOM]],
            capitalize))) + ".\n");
        return 1;
    }

    /* List your own applications. */
    return list_applications_by_wizard(cmder, 1);
}

/************************************************************************
 *
 * The experience administration code.
 *
 */

/*
 * Function name: bookkeep_exp
 * Description  : Note the xp domains give to the mortals and what kind.
 *                This is not saved each time to KEEPERSAVE, we trust it
 *                to be saved at one time or other. Exact bookkeeping is not
 *                absolutely crucial and it would take a lot of time.
 * Arguments    : string type - the type of experience being added, can
 *                    be "quest", "combat" and general".
 *                int exp - the amount of experience added.
 */
public void
bookkeep_exp(string type, int exp)
{
    int    cobj = 0;
    int    should_log = 0;
    string dname = "";
    string log;
    object pobj = previous_object();
    object giver = previous_object(-1);

    /* This test is necessary as long as the obsolete function add_exp()
     * exists. This is because there is an extra call_other in it.
     */
    if (pobj == giver)
        giver = previous_object(-2);

    /* It should be a mortal player, not an NPC and it should not be fixup
     * of accumulated stats, not should it be a 'jr' wizhelper character.
     */
    if (!interactive(pobj) ||
        (geteuid(pobj) != BACKBONE_UID) ||
        ((pobj == giver) &&
         (ABS(exp) < 2)) ||
        wildmatch("*jr", pobj->query_real_name()))
        return;

#ifdef EXP_FROM_COMBAT_OBJECT
    /* If it is combat XP, we want to get the living, not the combat
     * object, otherwise tracing is real hard.
     */
    if ((type == "combat") &&
        objectp(giver->qme()))
    {
        giver = giver->qme();
        cobj = 1;
    }
#endif EXP_FROM_COMBAT_OBJECT

    set_auth(this_object(), "root:root");
#ifdef LOG_BOOKKEEP_ERR
    /* BAD wiz! Won't be wiz much longer. */
    if (objectp(this_interactive()))
    {
        if (SECURITY->query_wiz_level(dname =
            this_interactive()->query_real_name()))
        {
            log_file(LOG_BOOKKEEP_ERR,
                sprintf("%s %-12s %-7s(%8d) by %-12s\n", ctime(time()),
                capitalize(pobj->query_real_name()), type, exp,
                capitalize(dname)), 10000);
        }
    }
    /* This indicates that it wasn't a combat object giving the combat
     * experience.
     */
#ifdef EXP_FROM_COMBAT_OBJECT
    else if ((type == "combat") && (!living(giver) || !cobj))
#else
    else if ((type == "combat") && !living(giver))
#endif EXP_FROM_COMBAT_OBJECT 
    {
        log_file(LOG_BOOKKEEP_ERR,
            sprintf("%s %-12s combat (%8d) object %s\n", ctime(time()),
            capitalize(pobj->query_real_name()), exp,
            file_name(giver)), 10000);
    }
#endif LOG_BOOKKEEP_ERR

    /* Get the euid of the experience giving object. */
    dname = geteuid(giver);
    if (sizeof(m_wizards[dname]))
        dname = m_wizards[dname][FOB_WIZ_DOM];

    /* Experience can only be given by a domain. This object had a bad
     * euid. Nonexistant or apprentice.
     */
    if (!sizeof(m_domains[dname]))
    {
#ifdef LOG_BOOKKEEP_ERR
        /* If this is the case, it is a playerkill and we do not want to
         * log that.
         */
        if ((dname != BACKBONE_UID) &&
            (file_name(giver) != CMD_LIVE_THIEF))
        {
            log_file(LOG_BOOKKEEP_ERR,
                sprintf("%s %-12s %-7s(%8d) %-1s\n", ctime(time()),
                capitalize(pobj->query_real_name()), type, exp,
                file_name(giver)), 10000);
        }
#endif LOG_BOOKKEEP_ERR
        return;
    }

    /* Give the experience to the domain. Not necessary to save it, that
     * will happen next time something needs to be saved.
     */
    switch(type)
    {
    case "quest":
        m_domains[dname][FOB_DOM_QXP] += exp;
#ifdef LOG_BOOKKEEP
        if (ABS(exp) > LOG_BOOKKEEP_LIMIT_Q)
            should_log = 1;
#endif LOG_BOOKKEEP
        break;

    case "combat":
        m_domains[dname][FOB_DOM_CXP] += exp;
#ifdef LOG_BOOKKEEP
        if (ABS(exp) > LOG_BOOKKEEP_LIMIT_C)
            should_log = 1;
#endif LOG_BOOKKEEP
        break;

    case "general":
//      m_domains[dname][FOB_DOM_GXP] += exp;
#ifdef LOG_BOOKKEEP
        if (ABS(exp) > LOG_BOOKKEEP_LIMIT_G)
            should_log = 1;
#endif LOG_BOOKKEEP
        break;
    }

#ifdef LOG_BOOKKEEP
    /* This is the log all normal combat experience and quest experience
     * ends up. It is only logged if it exceeds a certain level.
     */
    if (should_log)
    {
        log_file(LOG_BOOKKEEP, sprintf("%s %-12s %-7s(%8d) %-1s\n",
            ctime(time()), capitalize(pobj->query_real_name()),
            type, exp, file_name(giver)), 1000000);
    }
#endif LOG_BOOKKEEP

#ifdef LOG_BOOKKEEP_ROTATE
    /* Cycle the log with the same day number every month. */
    log = "/syslog/log/" + LOG_BOOKKEEP_ROTATE + "." + TIME2FORMAT(time(), "dd");
    if ((file_size(log) > 0) &&
        ((file_time(log) + 86400) < time()))
    {
        rm(log);
    }

    write_file(log, sprintf("%s %-12s %-7s(%8d) %-1s\n",
            ctime(time()), capitalize(pobj->query_real_name()),
            type, exp, file_name(giver)));
#endif LOG_BOOKKEEP_ROTATE
}

/*
 * Function name: do_decay
 * Description  : The mapping m_domains is mapped over this function to
 *                do the actual decay.
 * Arguments    : mixed *darr - the array of the individual domains.
 * Returns      : mixed * - the modified array of the individual domains.
 */
static mixed *
do_decay(mixed *darr)
{
    int decay;
#ifdef DECAY_XP
    decay = DECAY_XP;
#else
    decay = 100;
#endif
    if (!decay)
        return darr;

    if (darr[FOB_DOM_QXP] >= decay || (-darr[FOB_DOM_QXP]) >= decay)
        darr[FOB_DOM_QXP] -= darr[FOB_DOM_QXP] / decay;
    else if (darr[FOB_DOM_QXP] < 0)
        darr[FOB_DOM_QXP] += 1;
    else if (darr[FOB_DOM_QXP] > 0)
        darr[FOB_DOM_QXP] -= 1;

    if (darr[FOB_DOM_CXP] >= decay || (-darr[FOB_DOM_CXP]) >= decay)
        darr[FOB_DOM_CXP] -= darr[FOB_DOM_CXP] / decay;
    else if (darr[FOB_DOM_CXP] < 0)
        darr[FOB_DOM_CXP] += 1;
    else if (darr[FOB_DOM_CXP] > 0)
        darr[FOB_DOM_CXP] -= 1;

    darr[FOB_DOM_CMNDS] = darr[FOB_DOM_CMNDS] / DECAY_XP;

    return darr;
}

/*
 * Function name: do_decay_cmd
 * Description  : This decay function will take 1% off the argument value.
 * Arguments    : int count - the value to decay.
 * Returns      : int - the same value minus one per cent.
 */
int
do_decay_cmd(int count)
{
    return count - count / 100;
}

/*
 * Function name: decay_exp
 * Description:   Let the accumulated xp / domain decay over time. This
 *                is called from check_memory which is called at regular
 *                intervalls. check_memory also saves to KEEPERSAVE.
 */
static void
decay_exp()
{
    m_domains = map(m_domains, do_decay);
}

/*
 * Function name: query_domain_commands
 * Description  : Gives the total number of commands executed by mortal
 *                players in a domain.
 * Arguments    : string dname - the domain name.
 * Returns      : int - the number of commands.
 */
int
query_domain_commands(string dname)
{
    dname = capitalize(dname);

    if (!sizeof(m_domains[dname]))
        return 0;

    return m_domains[dname][FOB_DOM_CMNDS];
}

/*
 * Function name: query_domain_qexp
 * Description  : Gives the quest experience gathered by a domain.
 * Arguments    : string dname - the domain name.
 * Returns      : int - the accumulated experience.
 */
int
query_domain_qexp(string dname)
{
    dname = capitalize(dname);

    if (!sizeof(m_domains[dname]))
        return 0;

    return m_domains[dname][FOB_DOM_QXP];
}

/*
 * Function name: query_domain_cexp
 * Description  : Gives the combat experience gathered by a domain.
 * Arguments    : string dname - the domain name. 
 * Returns      : int - the accumulated experience.
 */
int
query_domain_cexp(string dname)        
{
    dname = capitalize(dname);

    if (!sizeof(m_domains[dname]))
        return 0;

    return m_domains[dname][FOB_DOM_CXP];
}

/*
 * Function name: domain_clear_xp
 * Description  : With this function archwizards and keepers may remove the
 *                experience gathered by a domain. This is for debugging
 *                purposes and when errors have been made and the experience
 *                is not correct any more
 * Arguments    : string dname - the domain-name.
 * Returns      : int 1/0 - success/failure.
 */
int
domain_clear_xp(string dname)
{
    string wname;

    /* May only be called from the arch soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
        return 0;

    /* Check whether such a domain indeed exists. */
    dname = capitalize(dname);
    if (!sizeof(m_domains[dname]))
    {
        notify_fail("No domain " + dname + ".\n");
        return 0;
    }

    /* Wipe the experience, save the master, log possible and tell the
     * wizard.
     */
    m_domains[dname][FOB_DOM_CXP] = 0;
    m_domains[dname][FOB_DOM_QXP] = 0;

    save_master();

#ifdef LOG_BOOKKEEP
    wname = getwho();
    log_file(LOG_BOOKKEEP, sprintf("%s (%s) Cleared by: %s\n",
        ctime(time()), dname, capitalize(wname)), -1);
#endif LOG_BOOKKEEP

    write("Cleared the experience of the domain " + dname + ".\n");
    return 1;
}

/************************************************************************
 *
 * The wizard administration code.
 *
 */

/*
 * Function name: query_wiz_rank
 * Desciption   : Return the rank of the wizard.
 * Arguments    : string wname - the name of the wizard.
 * Returns      : int - the rank.
 */
int
query_wiz_rank(string wname)
{
    wname = lower_case(wname);

    if (!sizeof(m_wizards[wname]))
        return WIZ_MORTAL;

    return m_wizards[wname][FOB_WIZ_RANK];
}

/*
 * Function name: query_wiz_level
 * Description  : Return the level of a wizard. If wiz levels are not used,
 *                this will return the rank.
 * Arguments    : string wname - the wizard.
 * Returns      : int - the level.
 */
int
query_wiz_level(string wname)
{
#ifdef USE_WIZ_LEVELS
    wname = lower_case(wname);

    if (!sizeof(m_wizards[wname]))
        return 0;

    return m_wizards[wname][FOB_WIZ_LEVEL];
#else
    return query_wiz_rank(wname);
#endif USE_WIZ_LEVELS
}

/*
 * Function name: query_wiz_chl
 * Description  : Return the name of the level changer.
 * Arguments    : string wname - the wizard.
 * Returns      : string - the changer.
 */
string
query_wiz_chl(string wname)
{
    wname = lower_case(wname);

    if (!sizeof(m_wizards[wname]))
        return "";

    return m_wizards[wname][FOB_WIZ_CHLEVEL];
}

/*
 * Function name: query_wiz_dom
 * Description  : Return the domain of a wizard.
 * Arguments    : string wname - the wizard.
 * Returns      : string - the domain, else "".
 */
string
query_wiz_dom(string wname)
{
    wname = lower_case(wname);

    if (!sizeof(m_wizards[wname]))
        return "";

    return m_wizards[wname][FOB_WIZ_DOM];
}

/*
 * Function name: query_wiz_chd
 * Description  : Return the name of the domain changer.
 * Arguments    : string wname - the wizard.
 * Returns      : string - the changer.
 */
string
query_wiz_chd(string wname)
{
    wname = lower_case(wname);

    if (!sizeof(m_wizards[wname]))
        return "";

    return m_wizards[wname][FOB_WIZ_CHDOM];
}

/*
 * Function name: query_wiz_list
 * Description  : Return a list of all wizards of a given rank.
 * Arguments    : int - the rank to get; -1 = all.
 * Returns      : string * - the names of the wizards with that name.
 */
string *
query_wiz_list(int rank)
{
    if (rank == -1)
        return m_indices(m_wizards);

    return filter(m_indices(m_wizards), &operator(==)(rank) @ query_wiz_rank);
}

/*
 * Function name: reset_wiz_uid
 * Description:   Set up a wizard's uid.
 * Arguments:     object wiz - the wizard.
 */
public nomask void
reset_wiz_uid(object wiz)
{
    /* Access failure. This is only acceptable for a player object. */
    if (!IS_PLAYER_OBJECT(wiz))
        return;

    if (!query_wiz_level(wiz->query_real_name()))
        return;

    set_auth(wiz, wiz->query_real_name() + ":#");
}

/*
 * Function name: do_change_rank
 * Description  : This function actually changes the rank of the person.
 *                It is an internal function that does not make any
 *                additional checks. They should have been made earlier.
 *                When someone is made liege, the previous liege is made
 *                into normal wizard.
 * Arguments    : string wname - the wizard who rank is changed.
 *                int rank     - the new rank of the wizard.
 *                string cmder - who forces the change.
 */
static void
do_change_rank(string wname, int rank, string cmder)
{
    string dname;
    object wizard;

    /* Log the change of the rank. */
    log_file("LEVEL",
#ifdef USE_WIZ_LEVELS
        sprintf("%s %-11s: %-4s (%2d) -> %-4s (%2d) by %s.\n",
#else
        sprintf("%s %-11s: %-4s -> %-4s by %s.\n",
#endif USE_WIZ_LEVELS
        ctime(time()),
        capitalize(wname),
        WIZ_RANK_SHORT_NAME(m_wizards[wname][FOB_WIZ_RANK]),
#ifdef USE_WIZ_LEVELS
        m_wizards[wname][FOB_WIZ_LEVEL],
#endif USE_WIZ_LEVELS
        WIZ_RANK_SHORT_NAME(rank),
#ifdef USE_WIZ_LEVELS
        WIZ_RANK_START_LEVEL(rank),
#endif USE_WIZ_LEVELS
        capitalize(cmder)));

    /* If the person was Liege, inform the domain. */
    if (m_wizards[wname][FOB_WIZ_RANK] == WIZ_LORD)
    {
        tell_domain(m_wizards[wname][FOB_WIZ_DOM],
            m_domains[m_wizards[wname][FOB_WIZ_DOM]][FOB_DOM_LORD],
            ("You are no longer the liege of " +
            m_wizards[wname][FOB_WIZ_DOM] + ".\n"),
            (capitalize(wname) + " no longer is the liege of " +
            m_wizards[wname][FOB_WIZ_DOM] + ".\n"));

        m_domains[m_wizards[wname][FOB_WIZ_DOM]][FOB_DOM_LORD] = "";
    }

    /* If the person was steward, inform the domain. */
    if (m_wizards[wname][FOB_WIZ_RANK] == WIZ_STEWARD)
    {
        tell_domain(m_wizards[wname][FOB_WIZ_DOM],
            m_domains[m_wizards[wname][FOB_WIZ_DOM]][FOB_DOM_STEWARD],
            ("You are no longer the steward of " +
            m_wizards[wname][FOB_WIZ_DOM] + ".\n"),
            (capitalize(wname) + " no longer is the steward of " +
            m_wizards[wname][FOB_WIZ_DOM] + ".\n"));

        m_domains[m_wizards[wname][FOB_WIZ_DOM]][FOB_DOM_STEWARD] = "";
    }

    /* If the person becomes Liege, inform the domain and demote the old
     * Liege, if there was one. Similar for the steward.
     */
    if (rank == WIZ_LORD)
    {
        /* If there already is a Lord, demote the old one. This must be
         * done before the new one is marked as such.
         */
        dname = m_wizards[wname][FOB_WIZ_DOM];
        if (strlen(m_domains[dname][FOB_DOM_LORD]))
            do_change_rank(m_domains[dname][FOB_DOM_LORD], WIZ_NORMAL, cmder);

        m_domains[dname][FOB_DOM_LORD] = wname;

        tell_domain(dname, wname,
            ("You are now the liege of " + dname + ".\n"),
            (capitalize(wname) + " is made the liege of " + dname + ".\n"));
    }

    if (rank == WIZ_STEWARD)
    {
        dname = m_wizards[wname][FOB_WIZ_DOM];
        if (strlen(m_domains[dname][FOB_DOM_STEWARD]))
            do_change_rank(m_domains[dname][FOB_DOM_STEWARD], WIZ_NORMAL,
			   cmder);

        m_domains[dname][FOB_DOM_STEWARD] = wname;

        tell_domain(dname, wname,
            ("You are now the steward of " + dname + ".\n"),
            (capitalize(wname) + " is made the steward of " + dname + ".\n"));
    }

    /* Tell the player about the change in rank. */
    if (objectp(wizard = find_player(wname)))
    {
        tell_object(wizard, "You have been " +
            ((rank > m_wizards[wname][FOB_WIZ_RANK]) ? "promoted" :
            "demoted") + " to " + WIZ_RANK_NAME(rank) +
#ifdef USE_WIZ_LEVELS
            " (" + WIZ_RANK_START_LEVEL(rank) + ")" +
#endif USE_WIZ_LEVELS
            " by " + capitalize(cmder) +
            ".\n");
    }

    /* Make the actual change. */
    m_wizards[wname][FOB_WIZ_RANK] = rank;
    m_wizards[wname][FOB_WIZ_LEVEL] = WIZ_RANK_START_LEVEL(rank);
    m_wizards[wname][FOB_WIZ_CHLEVEL] = cmder;

    save_master();

    if (objectp(wizard))
    {
        wizard->reset_userids();
        wizard->update_hooks();
    }
}

/*
 * Function name: wizard_change_rank
 * Description  : Using this function, a liege, arch or keeper can try to
 *                alter the rank of a player.
 * Arguments    : string wname - the wizard whose rank is to be changed.
 *                int rank     - the new rank.
 * Returns      : int 1/0 - success/failure.
 */
int
wizard_change_rank(string wname, int rank)
{
    string cmder;
    int    old_rank;
    string dname;
    object wizard;
    string *seconds;

    /* May only be called from the Lord soul. */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;

    cmder = getwho();
    wname = lower_case(wname);
    if (sizeof(m_wizards[wname]))
    {
        old_rank = m_wizards[wname][FOB_WIZ_RANK];
        dname = m_wizards[wname][FOB_WIZ_DOM];
    }
    else
    {
        old_rank = WIZ_MORTAL;
        dname = "";
    }

    /* Allow non-existant wizards to be demoted to mortal. */
    if (!exist_player(wname) &&
        (rank != WIZ_MORTAL))
    {
        write("Non-existant wizards may only be demoted to mortal.\n");
        return 1;
    }

    /* See whether the wizard is allowed to alter the rank. Lords may only
     * promote or demote 'normal' wizards to and from steward and vice versa.
     * Stewards may not meddle in ranks.
     */
    if (m_wizards[cmder][FOB_WIZ_RANK] == WIZ_STEWARD)
    {
        write("You are not allowed to alter the rank of any wizard.\n");
        return 1;
    }
    else if (m_wizards[cmder][FOB_WIZ_RANK] == WIZ_LORD)
    {
        if (m_wizards[cmder][FOB_WIZ_DOM] != m_wizards[wname][FOB_WIZ_DOM])
        {
            write("You may only handle people in your own domain.\n");
            return 1;
        }

        if ((old_rank != WIZ_STEWARD) ||
            (rank != WIZ_NORMAL))
        {
            write("You are only allowed to make a steward into a normal " +
              "wizard.\n");
            return 1;
        }
    }
    /* Arches may not meddle with other arches or keepers. However, they may
     * demote themselves to mage.
     */
    else if (old_rank >= m_wizards[cmder][FOB_WIZ_RANK])
    {
        if (cmder != wname)
        {
            write("You are not allowed to alter the rank of " +
                LANG_ADDART(WIZ_RANK_NAME(old_rank)) + ".\n");
            return 1;
        }
        if (rank != WIZ_MAGE)
        {
            write("You may only demote yourself to mage.\n");
            return 1;
        }
    }

    /* See whether the transition is legal. */
    if (member_array(rank, WIZ_RANK_POSSIBLE_CHANGE(old_rank)) == -1)
    {
        write("It is not possible to " +
            ((rank > old_rank) ? "promote" : "demote") + " someone from " +
            WIZ_RANK_NAME(old_rank) + " to " + WIZ_RANK_NAME(rank) + ".\n");
        return 1;
    }

    /* Promote a mortal into wizardhood. */
    if (old_rank == WIZ_MORTAL)
    {
        transform_mortal_into_wizard(wname, cmder);
    }

    /* Do it. */
    do_change_rank(wname, rank, cmder);

    /* For some ranks, we need to do something after the actual change. */
    switch(rank)
    {
    case WIZ_MORTAL:
        /* If the wizard is in a domain, kick him out. */
        if (strlen(dname))
            add_wizard_to_domain("", wname, cmder);

        /* Burry all evidence of his/her existing. */
        m_delkey(m_wizards, wname);
        remove_all_sanctions(wname);

        /* Delete the Jr, if any. */
        if (this_object()->remove_playerfile(wname + "jr",
            "Wizard " + capitalize(wname) + " demoted."))
        {
            write("Automatically deleted " + capitalize(wname) + "jr.\n");
        }
        /* Banish the wizard just for good measure. */
        if (!sizeof(this_object()->banish(wname, 2)))
        {
            write("Banished the name " + capitalize(wname) + ".\n");
        }
        if (sizeof(seconds = this_object()->query_seconds(wname)))
        {
            write("Listed second" + ((sizeof(seconds) == 1) ? "" : "s")+ ": " +
                COMPOSITE_WORDS(sort_array(map(seconds, capitalize))) + "\n");
        }

        /* Tell him/her the bad news and boot him. */
        if (objectp(wizard = find_player(wname)))
        {
            tell_object(wizard, "You are being reverted back to mortal by " +
                capitalize(cmder) + ". This means that you have to " +
                "create a new character again to continue playing.\n");

            /* ... and ... POOF! ;-) */
            wizard->quit();
            if (objectp(wizard))
                do_debug("destroy", wizard);
        }

        /* Rename the file after the booting. We might change our mind ;-) */
        rename(PLAYER_FILE(wname) + ".o", PLAYER_FILE(wname) + ".o.wizard");
        break;

    case WIZ_APPRENTICE:
    case WIZ_PILGRIM:
    case WIZ_RETIRED:
        if (strlen(dname))
            add_wizard_to_domain("", wname, cmder);

        break;
    }

    write(((rank > old_rank) ? "Promoted " : "Demoted ") +
        capitalize(wname) + " to " + WIZ_RANK_NAME(rank) +
#ifdef USE_WIZ_LEVELS
        " (" + WIZ_RANK_START_LEVEL(rank) + ")" +
#endif USE_WIZ_LEVELS
        ".\n");
    return 1;
}

#ifdef USE_WIZ_LEVELS
/*
 * Function name: wizard_change_level
 * Description  : Using this function, a liege, arch or keeper can try to
 *                alter the level of a player.
 * Arguments    : string wname - the wizard whose level is to be changed.
 *                int rank     - the new level.
 * Returns      : int 1/0 - success/failure.
 */
int
wizard_change_level(string wname, int level)
{
    string cmder;
    object wizard;

    /* May only be called from the Lord soul. */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;

    cmder = getwho();
    wname = lower_case(wname);
    if (!sizeof(m_wizards[wname]))
    {
        write(capitalize(wname) + " is not a wizard. To promote " +
              "him/her to wizard, use 'promote " + wname + " <rank>'.\n");
        return 1;
    }

    if ((m_wizards[cmder][FOB_WIZ_RANK] <= m_wizards[wname][FOB_WIZ_RANK]) ||
        (((m_wizards[cmder][FOB_WIZ_RANK] == WIZ_LORD) ||
          (m_wizards[cmder][FOB_WIZ_RANK] == WIZ_STEWARD)) &&
         (m_wizards[cmder][FOB_WIZ_DOM] != m_wizards[wname][FOB_WIZ_DOM])))
    {
        write("You have no control over the level of " + capitalize(wname) +
              ".\n");
        return 1;
    }

    if (m_wizards[wname][FOB_WIZ_LEVEL] == level)
    {
        write(capitalize(wname) + " already has level " + level + ".\n");
        return 1;
    }

    if (level < WIZ_RANK_MIN_LEVEL(m_wizards[wname][FOB_WIZ_RANK]))
    {
        write("The minimum level for " +
              LANG_ADDART(WIZ_RANK_NAME(m_wizards[wname][FOB_WIZ_RANK])) +
              " is " + WIZ_RANK_MIN_LEVEL(m_wizards[wname][FOB_WIZ_RANK]) +
              ".\n");
        return 1;
    }

    if (level > WIZ_RANK_MAX_LEVEL(m_wizards[wname][FOB_WIZ_RANK]))
    {
        write("The maximum level for " +
              LANG_ADDART(WIZ_RANK_NAME(m_wizards[wname][FOB_WIZ_RANK])) +
              " is " + WIZ_RANK_MAX_LEVEL(m_wizards[wname][FOB_WIZ_RANK]) +
              ".\n");
        return 1;
    }

    log_file("LEVEL",
        sprintf("%s %-11s: %-4s (%2d) -> %-4s (%2d) by %s.\n",
        ctime(time()),
        capitalize(wname),
        WIZ_RANK_SHORT_NAME(m_wizards[wname][FOB_WIZ_RANK]),
        m_wizards[wname][FOB_WIZ_LEVEL],
        WIZ_RANK_SHORT_NAME(m_wizards[wname][FOB_WIZ_RANK]),
        level,
        capitalize(cmder)));

    if (objectp(wizard = find_player(wname)))
    {
        tell_object(wizard, "You been " +
            ((level > m_wizards[wname][FOB_WIZ_LEVEL]) ? "promoted" :
            "demoted") + " to level " + level + " by " +
            capitalize(cmder) + ".\n");
    }

    write(((level > m_wizards[wname][FOB_WIZ_LEVEL]) ? "Promoted " :
           "Demoted ") + capitalize(wname) + " to level " + level + ".\n");

    m_wizards[wname][FOB_WIZ_LEVEL] = level;
    m_wizards[wname][FOB_WIZ_CHLEVEL] = cmder;

    save_master();

    return 1;
}
#endif USE_WIZ_LEVELS

/*
 * Function name: set_keeper
 * Description  : We have a special command to make keepers. Only keepers
 *                may use it to stress its special importance, but hacking
 *                the master save file should not be necessary.
 * Arguments    : string wname - the arch to make keeper (or vice versa)
 *                int promote  - 1 = make keeper, 0 = make arch.
 * Returns      : int 1/0 - success/ failure.
 */
int
set_keeper(string wname, int promote)
{
    /* May only be called from the keeper soul. */
    if (!CALL_BY(WIZ_CMD_KEEPER))
        return 0;

    do_change_rank(wname, (promote ? WIZ_KEEPER : WIZ_ARCH), getwho());
    return 1;
}

/*
 * Function name: create_wizard
 * Description  : Certain objects in the game are allowed to promote a mortal
 *                into wizardhood. This function should be called to do the
 *                transformation. The mortal is then made into apprentice.
 * Arguments    : string name - the name of the mortal to be promoted.
 */
void
create_wizard(string name)
{
    if (member_array(function_exists("make_wiz", previous_object()),
        WIZ_MAKER) == -1)
        return;

    transform_mortal_into_wizard(name, ROOT_UID);
    do_change_rank(name, WIZ_APPRENTICE, ROOT_UID);
}

/*
 * Function name: query_wiz_pretitle
 * Description  : Gives a pretitle for a specific wizard (type).
 * Arguments    : mixed wiz - the wizard object or name.
 * Returns      : string - the pretitle for the wizard.
 */
string
query_wiz_pretitle(mixed wiz)
{
    string name;
    int gender;

    /* Knowing the name, find the objectpointer for the gender. */
    if (stringp(wiz))
    {
        name = wiz;
        wiz = find_player(wiz);
    }

    /* If there is no such object, clone the finger-player and clean up
     * after ourselves. Else, pick up the info from the wizard object.
     */
    if (!objectp(wiz))
    {
        set_auth(this_object(), "root:root");
        wiz = finger_player(name);

        if (!objectp(wiz))
            return "";

        gender = wiz->query_gender();
        wiz->remove_object();
    }
    else
    {
        gender = wiz->query_gender();
        name = wiz->query_real_name();
    }

    switch(query_wiz_rank(name))
    {
    case WIZ_MORTAL:
        return "";

    case WIZ_APPRENTICE:
        return ({ "Squire", "Maid", "Some" })[gender];

    case WIZ_PILGRIM:
        return ({ "Brother", "Sister", "Brother" })[gender];

    case WIZ_RETIRED:
        return ({ "Mister", "Miss", "Some" })[gender];

    case WIZ_NORMAL:
        return ({ "Sir", "Dame", "It" })[gender];

    case WIZ_MAGE:
        return "Honourable";

    case WIZ_STEWARD:
        return "Warden";

    case WIZ_LORD:
        return ({ "Lord", "Lady", "Liege" })[gender];

    case WIZ_ARCH:
        return "Proctor";

    case WIZ_KEEPER:
        return "Master";
    }

    /* Should never end up here. */
    return "";
}

/*
 * Function name: query_wiz_path
 * Description  : Gives a default path for a wizard
 * Arguments    : string wname - the wizard name.
 * Returns      : string - the default path for the wizard.
 */
string
query_wiz_path(string wname)
{
    string dname;

    /* A domains path. */
    dname = capitalize(wname);
    if (sizeof(m_domains[dname]))
        return "/d/" + dname;

    /* Root. */
    wname = lower_case(wname);
    if (wname == ROOT_UID)
        return "/syslog";

    /* Not a wizard, ie no path. */
    if (!sizeof(m_wizards[wname]))
        return "";

    /* A wizard who is a domain member. */
    if (strlen(m_wizards[wname][FOB_WIZ_DOM]))
        return "/w/" + wname;

    /* Non-domain members, ie apprentices, pilgrims and retired people. */
    return "/doc";
}

/*
 * Function name: query_mage_links
 * Description  : Finds all WIZARD_DOMAIN mages and makes a list of all links.
 * Returns      : string * - the list.
 */
public string *
query_mage_links()
{
    string *links;
    int index;

    links = sort_array( ({ }) + m_domains[WIZARD_DOMAIN][FOB_DOM_MEMBERS]);
    index = sizeof(links);
    while(--index >= 0)
        links[index] = "/w/" + links[index] + "/" + WIZARD_LINK;

    return links;
}

/*
 * Function name: query_domain_links
 * Description  : Make a list of domainfiles to link.
 * Returns      : string * - the list.
 */
public string *
query_domain_links()
{
    string *links;
    int index;

    links = sort_array(m_indices(m_domains));
    index = sizeof(links);
    while(--index >= 0)
        links[index] = "/d/" + links[index] + "/" + DOMAIN_LINK;

    return links;
}

/*
 * Function name: retire_wizard
 * Description  : Wizards may retire from active coding as they please.
 *                They loose their coding rights and are placed in the
 *                special retired wizards rank.
 * Returns      : int 1/0 - success/failure.
 */
int
retire_wizard()
{
    string cmder;
    int    rank;

    /* May only be called from the 'normal' wizard soul. */
    if (!CALL_BY(WIZ_CMD_NORMAL))
        return 0;

    cmder = getwho();
    rank  = m_wizards[cmder][FOB_WIZ_RANK];

    /* People who aren't full wizards cannot retire. */
    if (rank < WIZ_NORMAL)
    {
        notify_fail("You may not retire. If you want to be retired, " +
            "ask a member of the administration for permission.\n");
        return 0;
    }

    if (rank >= WIZ_ARCH)
    {
        notify_fail("No way. Archwizards and keepers cannot retire.\n");
        return 0;
    }

    /* Do it. */
    do_change_rank(cmder, WIZ_RETIRED, cmder);
    add_wizard_to_domain("", cmder, cmder);

    write("You just retired from coding.\n");
    return 1;
}

/************************************************************************
 *
 * The global read administration code.
 *
 */

/*
 * Function name: add_global_read
 * Description  : Add a wizard to the global read list.
 * Arguments    : string wname   - the name of the wiz to be added.
 *                string comment - a suitable comment to store along with
 *                                 the name telling why the player has it.
 * Returns      : int 1/0 - success/failure.
 */
int
add_global_read(string wname, string comment)
{
    string cmder;
    object wiz;

    /* May only be called from the arch soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
        return 0;

    cmder = getwho();

    /* Only full wizards can have global read access. */
    wname = lower_case(wname);
    if (m_wizards[wname][FOB_WIZ_RANK] < WIZ_NORMAL)
    {
        notify_fail("Not a full wizard: " + capitalize(wname) + ".\n");
        return 0;
    }

    if (!strlen(comment))
    {
        notify_fail("No comment added to the command line.\n");
        return 0;
    }

    /* Only add global access if the player does not have it already. */
    if (m_global_read[wname])
    {
        notify_fail("The wizard " + capitalize(wname) +
            " already has global access.\n");
        return 0;
    }

    /* Add the stuff, save the master and tell the caller. */
    m_global_read[wname] = ({ cmder, comment });
    save_master();

    if (objectp(wiz = find_player(wname)))
        tell_object(wiz, "Global read access has been granted to you by " +
		    capitalize(cmder) + ".\n");

    write("Added " + capitalize(wname) + " to have global read access.\n");
    return 1;
}

/*
 * Function name: remove_global_read
 * Description  : Remove a wizard from the global access list.
 * Arguments    : string wname - the wizard to remove.
 * Returns      : int 1/0 - true if successful.
 */
int
remove_global_read(string wname)
{
    string cmder;
    object wiz;

    /* May only be called from the arch soul. */
    if (!CALL_BY(WIZ_CMD_ARCH))
        return 0;

    cmder = getwho();

    /* See whether the person indeed has global read access. */
    wname = lower_case(wname);
    if (!m_global_read[wname])
    {
        notify_fail("Wizard " + capitalize(wname) + " not registered.\n");
        return 0;
    }

    /* Remove the entry, save the master and notify the caller. */
    m_delkey(m_global_read, wname);
    save_master();

    if (objectp(wiz = find_player(wname)))
    {
        tell_object(wiz, "Your global read access has been revoked by " +
            capitalize(cmder) + ".\n");
    }

    write("Removed global read access from " + capitalize(wname) + ".\n");
    return 1;
}

/*
 * Function name: query_global_read
 * Description  : This function returns a mapping with the names of the
 *                people that have global read access. For the format of
 *                the file, see the header of this file.
 * Returns      : mapping - the global read list.
 */
mapping 
query_global_read()
{
    return secure_var(m_global_read);
}

/************************************************************************
 *
 * The mentor administration code.
 *
 */

/*
 * Function name: set_mentor
 * Description  : Set the identity of a wizard mentor
 * Arguments    : string mentor - the mentor to add
 *                string student - the student to set in
 * Returns      : int 1 - success, 0 - error
 */
int
set_mentor(string mentor, string student)
{
    string cmder = getwho();

    /* Only stewards++ can do this */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;

    /* The student has to be at least a normal wiz. */
    if (mentor != "none" && 
	mentor != "" &&
	query_wiz_rank(student) < WIZ_NORMAL)
        return 0;

    /* Erase the previous record */
    if (mentor == "none" || mentor == "")
    {
	if (sizeof(m_wizards[student]))
	{
	    m_wizards[student][FOB_WIZ_MENTOR] = "";
	    save_master();
	}
        return 1;
    }

    /* The mentor has to be at least a normal wiz. */
    if (query_wiz_rank(mentor) < WIZ_NORMAL)
        return 0;

    /* It's impossible to overwrite a mentor record. */
    if (strlen(m_wizards[student][FOB_WIZ_MENTOR]) > 0)
        return 0;

    /* It's impossible to be a mentor and student at the same time. */
    if (strlen(m_wizards[mentor][FOB_WIZ_MENTOR]) > 0 ||
        sizeof(m_wizards[student][FOB_WIZ_STUDENTS]) > 0)
        return 0;

    /* The student and mentor should belong to the same domain, except
     * for archwizards of course.
     */
    if (m_wizards[cmder][FOB_WIZ_RANK] < WIZ_ARCH &&
	m_wizards[mentor][FOB_WIZ_RANK] < WIZ_ARCH)
        if (m_wizards[student][FOB_WIZ_DOM] != m_wizards[mentor][FOB_WIZ_DOM])
            return 0;

    m_wizards[student][FOB_WIZ_MENTOR] = mentor;
    save_master();
    return 1;
}

/*
 * Function name: query_mentor
 * Description  : Get the identity of a wizard mentor.
 * Arguments    : string student - the wiz to query.
 * Returns      : string - the mentor.
 */
string
query_mentor(string student)
{
    if (!sizeof(m_wizards[student]))
        return "";

    return m_wizards[student][FOB_WIZ_MENTOR];
}

/*
 * Function name: add_student
 * Description  : Add a student to the internal list.
 * Arguments    : string mentor - the mentor to add in.
 *                string student - the wizard to add.
 * Returns      : int 1 - success, 0 - error.
 */
int
add_student(string mentor, string student)
{
    string cmder = getwho();

    /* Only stewards++ can do this. */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;

    /* The mentor has to be at least a normal wiz. */
    if (query_wiz_rank(mentor) < WIZ_NORMAL)
        return 0;

    /* The student has to be at least a normal wiz. */
    if (query_wiz_rank(student) < WIZ_NORMAL)
        return 0;

    /* It's impossible to be a mentor and student at the same time. */
    if (strlen(m_wizards[mentor][FOB_WIZ_MENTOR]) > 0 ||
        sizeof(m_wizards[student][FOB_WIZ_STUDENTS]) > 0)
        return 0;

    /* The student and mentor should belong to the same domain, except
     * for archwizards of course.
     */
    if (m_wizards[cmder][FOB_WIZ_RANK] < WIZ_ARCH &&
	m_wizards[mentor][FOB_WIZ_RANK] < WIZ_ARCH)
        if (m_wizards[mentor][FOB_WIZ_DOM] != m_wizards[student][FOB_WIZ_DOM])
            return 0;

    /* The student must be new. */
    if (member_array(student, m_wizards[mentor][FOB_WIZ_STUDENTS]) >= 0)
        return 0;

    m_wizards[mentor][FOB_WIZ_STUDENTS] += ({ student });
    save_master();
    return 1;
}

/*
 * Function name: remove_student
 * Description  : Remove a student from the internal list.
 * Arguments    : string mentor - the mentor to remove from.
 *                string student - the student to remove.
 * Returns      : int 1 - success, 0 - error.
 */
int
remove_student(string mentor, string student)
{
    /* Only stewards++ can do this. */
    if (!CALL_BY(WIZ_CMD_LORD))
        return 0;

    /* He isn't a wizard. */
    if (!sizeof(m_wizards[mentor]))
        return 0;

    /* It doesn't matter if he's in the list or not. */
    m_wizards[mentor][FOB_WIZ_STUDENTS] -= ({ student });
    save_master();
    return 1;
}

/*
 * Function name: query_students
 * Description  : Get the list of students.
 * Arguments    : string mentor - the mentor to query.
 * Returns      : string * - the student(s) of this mentor.
 */
string *
query_students(string mentor)
{
    if (!sizeof(m_wizards[mentor]))
        return ({});

    return secure_var(m_wizards[mentor][FOB_WIZ_STUDENTS]);
}

/************************************************************************
 *
 * The restriction code administration
 *
 */

/*
 * Function name: set_restrict
 * Description  : This function is used to impose a restriction on a
 *                wizard of some kind.
 * Arguments    : string wiz - The wizard to restrict.
 *                int res - The restriction to impose.
 */
int
set_restrict(string wiz, int res)
{
    /* Only mentors and steward++ can do this. */
    if (!CALL_BY(WIZ_CMD_NORMAL))
        return 0;

    if (sizeof(m_wizards[wiz]) == 0)
        return 0;

    /* Impose a restriction, just be certain even though
     * only the wiz soul can call.
     */
    if ((m_wizards[wiz][FOB_WIZ_RANK] < WIZ_NORMAL) ||
        (m_wizards[wiz][FOB_WIZ_RANK] > WIZ_MAGE))
        return 0;
    
    m_wizards[wiz][FOB_WIZ_RESTRICT] |= res;
    save_master();

    return 1;
}

/*
 * Function name: remove_restrict
 * Description  : This function is used to remove a restriction from a
 *                wizard of some kind.
 * Arguments    : string wiz - The wizard to restrict.
 *                int res - The restriction to remove.
 */
int
remove_restrict(string wiz, int res)
{
    if (!CALL_BY(WIZ_CMD_NORMAL))
        return 0;

    if (sizeof(m_wizards[wiz]) == 0)
        return 0;
    
    m_wizards[wiz][FOB_WIZ_RESTRICT] ^= res;
    save_master();

    return 1;
}

/*
 * Function name: query_restrict
 * Description  : This function is used to return the restriction of a wizard.
 * Arguments    : string wiz - The wizard to query.
 */
int
query_restrict(string wiz)
{
    if (sizeof(m_wizards[wiz]) == 0)
        return 0;
    
    return m_wizards[wiz][FOB_WIZ_RESTRICT];
}

/************************************************************************
 *
 * The code for team admins
 *
 */

/*
 * Function name: update_teams
 * Description  : Update the list of teams.
 */
void
update_teams(void)
{
    foreach(string team: m_indices(m_teams))
    {
        /* Remove non-wizards. */
        foreach(string wname: m_teams[team])
        {
            if (m_wizards[wname][FOB_WIZ_RANK] < WIZ_NORMAL)
            {
                m_teams[team] -= ({ wname });
            }
        }
        /* Remove empty teams. */
        if (!sizeof(m_teams[team]))
        {
            m_delkey(m_teams, team);
        }
    }

    save_master();
}

/*
 * Function name: add_team_member
 * Description  : Add a meber to a team.
 * Arguments    : string team - the team.
 *                string member - the member to add.
 * Returns      : 1/0 - success/failure.
 */
int
add_team_member(string team, string member)
{
    /* Only arches can do this. */
    if (!CALL_BY(WIZ_CMD_ARCH))
        return 0;

    team = lower_case(team);
    member = lower_case(member);

    /* The member must be a full wiz. */
    if (m_wizards[member][FOB_WIZ_RANK] < WIZ_NORMAL)
        return 0;

    /* Make sure he only ends up there once. */
    if (!pointerp(m_teams[team]))
        m_teams[team] = ({ member });
    else
    {
        m_teams[team] |= ({ member });
    }

    save_master();

    return 1;
}

/*
 * Function name: remove_team_member
 * Description  : Remove a member from a team.
 * Arguments    : string team - the team.
 *                string member - the member to remove.
 * Returns      : 1/0 - success/failure.
 */
int
remove_team_member(string team, string member)
{
    /* Only arches can do this. */
    if (!CALL_BY(WIZ_CMD_ARCH))
        return 0;

    team = lower_case(team);
    if (pointerp(m_teams[team]))
    {
        member = lower_case(member);
        m_teams[team] -= ({ member });
        update_teams();
    }

    return 1;
}

/*
 * Function name: query_team_list
 * Description  : Query a list of members.
 * Arguments    : string team - the team to query.
 * Returns      : string * - the wizards in the team.
 */
string *
query_team_list(string team)
{
    team = lower_case(team);
    if (!pointerp(m_teams[team]))
       return ({ });

    return secure_var(m_teams[team]);
}

/*
 * Function name: query_team_member
 * Description  : Determine membership status.
 * Arguments    : string team - the team to query.
 *              : string member - the member to query.
 * Returns      : 1/0 - member/not.
 */
int
query_team_member(string team, string member)
{
    team = lower_case(team);
    if (!pointerp(m_teams[team]))
        return 0;

    member = lower_case(member);
    return (member_array(member, m_teams[team]) >= 0);
}

/*
 * Function name: query_teams
 * Description  : Query the list of teams.
 * Returns      : string * - the list of team names.
 */
string *
query_teams(void)
{
    return m_indices(m_teams);
}

/* Function name: query_team_membership
 * Description  : Return a list of teams the player is a member of
 * Arguments    : string wname - the member to query about
 * Returns      : string * - the list of teams
 */
string *
query_team_membership(string wname)
{
    string *rlist = ({ });

    wname = lower_case(wname);
    foreach(string team: m_indices(m_teams))
    {
        if (IN_ARRAY(wname, m_teams[team]))
        {
            rlist += ({ team });
        }
    }

    return rlist;
}

/************************************************************************
 *
 * The code securing the channels.
 *
 */

/*
 * Function name: set_channels
 * Description  : This function is used by the apprentice commandsoul to
 *                store the mapping with the multi-wizline channels in a
 *                safe place.
 * Arguments    : mapping channels - the mapping with the channels.
 */
int
set_channels(mapping channels)
{
    /* May only be called from the apprentice soul. */
    if (!CALL_BY(WIZ_CMD_APPRENTICE))
        return 0;

    set_auth(this_object(), "root:root");
    save_map(channels, CHANNELS_SAVE);
    return 1;
}

/*
 * Function name: query_channels
 * Description  : This function is used by the apprentice commandsoul to
 *                retrieve the channels from disk. This is done since they
 *                are stored in a safe place.
 * Returns      : mapping - the mapping with the channels.
 */
mapping
query_channels()
{
    if (!CALL_BY(WIZ_CMD_APPRENTICE))
        return 0;

    set_auth(this_object(), "root:root");
    return restore_map(CHANNELS_SAVE);
}

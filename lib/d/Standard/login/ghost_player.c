/*
 * /d/Standard/login/ghost_player.c
 *
 * This player file is used when a player logging in does not have a valid
 * player file. There can be three different reasons for not having a
 * player_file:
 *
 *       1 - If this is a new character, then manage the creation process.
 *
 *       2 - The players racefile is not loadable, a new body must be
 *           choosen.
 *
 *       3 - The players racefile is not a legal playerfile, a new body
 *           must be choosen.
 *
 * Revision history:
 *    July 26/96 - Khail.
 *    Basically just removed the virtual coins as no longer necessary
 *    and documented existing code properly.
 */

#pragma strict_types
#pragma save_binary

inherit "/std/player_pub";

#include <files.h>
#include <macros.h>
#include <std.h>
#include <stdproperties.h>
#include "login.h"

#define NEWBIE_RACE_NAME "newbie"
#define GHOSTLY_GENDER_STRING "ghostly"

static private int     time_out_alarm;  /* The id of the alarm */
static private string  rhost;           /* The real host */
static         string  *new_queries;

/*
 * Prototypes
 */
public void ghost_start();
void next_query();
void ask_player();
static void check_identity1(string id);

/*
 * Function name: time_out
 * Description  : Remove this object if it has been idle too long.
 */
void
time_out()
{
    write("Time out\n");
    this_object()->remove_object();
}

/*
 * Function name:   legal_player
 * Description:     This is called from the master object to check that
 *                  a specific player object is legal. This is used to
 *                  decide if restore_object / save_object in /players
 *                  is possible.
 * Arguments:       ob: The object to check
 * Returns:         True for legal player object
 */
nomask public int
legal_player(object ob)
{
    string m;

    m = MASTER_OB(ob);
    if (m == MASTER)
    {
        return 1;
    }

    if (member_array(m, m_values(RACEMAP)) >= 0)
    {
        return 1;
    }

    return 0;
}

/*
 * Function name:  enter_new_player
 * Description  :  This is the function called from the login object.
 *                 It is responsible for initialising the correct actions
 *                 to be performed.
 * Arguments    :  name - The name of the player entering the game.
 *                 pass - The password to attempt entering with. "" if
 *                        simply entering a new body.
 */
public nomask void
enter_new_player(string name, string pass)
{
    object pob = previous_object();

    /* Remove linkdead player objects */
    if (!interactive(this_object()))
    {
        write("I'm linkdead, bye bye!\n");
        this_object()->remove_object();
        return;
    }

    /*
     * Prevent intrusion into more privileged players. This function can
     * only be called from /secure/login or this file.
     */
    if (!pob ||
        ((MASTER_OB(pob) != LOGIN_OBJECT) &&
         (MASTER_OB(pob) != MASTER)))
    {
        log_file("SECURITY", "enter_new_player() called from " +
            file_name(pob) + "\n");
        this_object()->remove_object();
        return;
    }

    /* Set the name of this object to the name of the player attempting to
     * enter the game.
     */
    set_name(name);
    seteuid(0);

    /* Preserve the login time. */
    if (MASTER_OB(pob) == MASTER)
    {
        set_login_time(pob->query_login_time());
    }

    /* Get the host name of the player attempting connection */
    rhost = query_ip_name(this_object());

    /*
     * Check if the player exist
     */
    if (!SECURITY->load_player())
    {
        /* Reset euid */
        seteuid(getuid(this_object()));
        /* Set the 'name's playerfile to this file (a ghost player) */
        set_player_file(MASTER);

        /* This can never be a valid password, so we use this to trigger
         * internal calls.
         */
        if (pass != "")
        {
            set_password(pass);
        }

        /* name'jr' is a wizard helper if it is not an old player. */
        if (wildmatch("*jr", name))
        {
            write("\nOk, you say you're a wizard. Let's see if it's true.\n" +
                  "And do remember: Max two helpers per wizard!\n");
            write("\nInput your wizard name: ");
            input_to(check_identity1, 0);
            time_out_alarm = set_alarm(120.0, 0.0, time_out);
            return;
        }

#ifdef Standard
        /* Is the mud open to new players? */
        if (file_size(LOGIN_NO_NEW) > 0)
        {
            cat(LOGIN_NO_NEW);
            log_file("REFUSED_ENTRY", ctime(time()) + ": " + name + "\n");
            this_object()->remove_object();
            return;
        }
#endif

        write("Creating a new player.\n");
        log_file("CREATE_PLAYER", ctime(time()) + " " + query_name() +
            " (" + query_ip_name(this_object()) + ")\n", -1);

        /* This is a new player, so use GP_NEW so they'll go through the
         * entire character creation sequence.
         */
        set_ghost(GP_NEW);

        /* Send the new player some login info, from file defined in login.h */
        cat(LOGIN_FILE_NEW_PLAYER_INFO);

        /* Some questions are still needed */
        ask_player();
        return;
    }

    seteuid(getuid(this_object()));

    /*
     * Is this a normal CDlib player with a bad playerfile ?
     */
    if (query_player_file() != MASTER)
    {
        /* Mark the player as dead and needing a body only */
        set_ghost(GP_BODY | GP_DEAD);

        /* Set the player's playerfile to this one, since their old is
         * missing or corrupt. This permits entering the game regardless,
         * as a ghost.
         */
        set_player_file(MASTER);
        write("You have a bad body and will have to choose a new.\n");
    }

    /* Start the player up as a ghost */
    ghost_start();
}

/*
 * Function name: check_identity2
 * Description  : Verify the password of the a wizard. This is used for
 *                wizards creating a 'jr' character, to confirm that they
 *                belong to a real wizard.
 * Arguments    : p - Password to test against the named wizard.
 *                id - Name of the wizard to test.
 */
static void
check_identity2(string p, string id)
{
    object wiz;

    write("\n");
    remove_alarm(time_out_alarm);

    /* Get an object pointer to the 'wizard to test', find_player if they're
     * in the game, finger_player if not. finger_player() will create a
     * 'dummy' object that simulates the wizard being logged in.
     */
    if (!objectp(wiz = find_player(id)))
    {
        wiz = SECURITY->finger_player(id);
    }

    /* Make the actual password check. */
    if (!(wiz->match_password(p)))
    {
        /* Password test failed, if the 'wiz to test' pointer was created
         * with finger_player(), remove it from memory.
         */
        if (wiz->query_finger_player())
        {
            wiz->remove_object();
        }

        /* Now remove this object, as it's failed this part of the 'jr'
         * creation sequence.
         */
        write("Wrong password!\n");
        this_object()->remove_object();
        return;
    }

    /* Log the creation of a wizard's junior. */
    log_file("HELPER", ctime(time()) + ": " +
        capitalize((wiz->query_real_name())) + " -> " +
        query_name() + "\n", -1);

    write("Copying your wizards email address.\n");
    set_mailaddr(wiz->query_mailaddr());

    /* If 'wiz to test' object pointer was obtained with finger_player,
     * remove the dummy object.
     */
    if (wiz->query_finger_player())
    {
        wiz->remove_object();
    }

    /* Mark this player as being brand-new, requiring a real body, mangling
     * features, and skills. Then start him up as a ghost, as opposed to a
     * player.
     */
    set_ghost(GP_NEW);
    ghost_start();
}

/*
 * Function name: check_identity1
 * Description  : Called if someone is trying to create a 'jr' character.
 *                They are prompted to give their wizard character's name
 *                to avoid abuse (2 jr max, and creation is logged). This
 *                First part checks to ensure that the wizard name given
 *                belongs to a true wizard. Afterwards it passes evaluation
 *                along to check_identity2 for password confirmation.
 * Arguments    : id - The name of the wizard to test.
 */
static void
check_identity1(string id)
{
    /* Stop the time out alarm */
    remove_alarm(time_out_alarm);

    /* Make sure the player supplied a name to test as being that of a
     * wizard. If not, remove this object.
     */
    if (!strlen(id))
    {
        write("No name entered.\n");
        this_object()->remove_object();
        return;
    }

    /* Check with SECURITY if the name to test has any wizard rank.
     * If not, remove this object.
     */
    if (!(SECURITY->query_wiz_rank(id)))
    {
        write(capitalize(id) + " is not a wizard.\n");
        this_object()->remove_object();
        return;
    }

    /* Ok, 'id' does name a real wizard, so lets see if the person creating
     * this jr character knows id's password. Give him 2 minutes to answer.
     * Response is evaluated by check_identity2.
     */
    write("Input your wizard password: ");
    input_to(&check_identity2(, lower_case(id)), 1);
    time_out_alarm = set_alarm(120.0, 0.0, time_out);
    return;
}

/*****************************************************************
 *
 * The questions to ask an entirely new player, which is not handled
 * in the configuration process.
 *
 */

/*
 * Function name: ask_player
 * Description:   Ask some questions of new players
 */
static void
ask_player()
{
    new_queries = ({ "dummy", "q_mail" });
    next_query();
    return;
}

/*
 * Function name: end_query
 * Description  : All the questions have been answered, so start the player
 *                as a ghost for character creation.
 */
static void
end_query()
{
    ghost_start();
}

/*
 * Function name: next_query
 * Description:   Asks the next question of the user interactively.
 */
void
next_query()
{
    /* Remove the 2 minute timeout limit. */
    remove_alarm(time_out_alarm);

    /* This loop will run infinitely until end_query() it returns out of
     * the loop.
     */
    while (1)
    {
        if (sizeof(new_queries) < 2)
        {
            return end_query();        /* does not return */
        }

        new_queries = slice_array(new_queries, 1, sizeof(new_queries));
        if (call_other(this_object(), new_queries[0] + "_pretext"))
        {
            time_out_alarm = set_alarm(120.0, 0.0, time_out);
            input_to(new_queries[0]);
            return;
        }
    }
}

/*
 * Function name: again_query
 * Description:   Asks the same question again.
 */
static void
again_query()
{
    /* If the player hasn't set the current query, keep sending the
     * responses to it. If they have, move on to the next query.
     */
    if (call_other(this_object(), new_queries[0] + "_pretext"))
    {
        input_to(new_queries[0]);
        return;
    }
    next_query();
}

/*
 * Function name: q_mail_pretext
 * Description  : Determines if the player has set an email address yet
 *                or not. If they haven't, ask the question again so players
 *                know what response is expected of them.
 * Returns      : 1 - No email set.
 *                0 - Email set.
 */
int
q_mail_pretext()
{
    /* Do not ask if there is already an email address. */
    if (query_mailaddr())
    {
        return 0;
    }

    write("Please enter your email address (or 'none'): ");
    return 1;
}

/*
 * Function name: q_mail
 * Description  : This function is called using input_to, and sets the
 *                email adress of this player.
 */
static void
q_mail(string maddr)
{
    set_mailaddr(maddr);
    next_query();
}


/*
 * Function name: ghost_start
 * Description  : Starts up the player as a 'ghost', rather than a normal
 *                player.
 */
public void
ghost_start()
{
    /* Remove the current timeout alarm */
    remove_alarm(time_out_alarm);

    /* If the player doesn't have a race name set, give the temporary name
     * 'newbie' and create a save file for him.
     */
    if (!query_race_name())
    {
        set_race_name(NEWBIE_RACE_NAME);
        /* Only needed if you are a firstimer isn't it? */
        save_me(0);
    }

    /* Make sure this_player() is pointing to this_object(), since this is
     * supposed to be an interactive object.
     */
    if (this_player() != this_object())
    {
        set_this_player(this_object());
    }

    /* We now have a correct .o file
     * Send the 'ghost' to ghost to the bodies room via enter_game.
     */
    write("\nEntering the hall of the bodies in waiting...\n\n");
    enter_game(query_real_name(), "");
}

/*
 * Function name: start_player
 * Description  : Masks the normal start_player in order to place
 *                the ghost cmdsouls if the player has none as of yet.
 *                Then continue the thread through the normal start_player.
 */
void
start_player()
{
    if (!sizeof(this_object()->query_cmdsoul_list()))
    {
        this_object()->add_cmdsoul("/d/Standard/cmd/soul_cmd_ghost");
        this_object()->add_cmdsoul("/d/Standard/cmd/misc_cmd_ghost");
    }
    ::start_player();
}

/*
 * Function name: query_default_start_location
 * Description  : Masks the normal query_default_start_location, in order
 *                to reroute ghosts to the proper room in the character
 *                creation sequence, based upon the current value of their
 *                ghost variable. This is a security precaution for players
 *                who LD or quit out in the middle of the sequence, so they
 *                should restart at or near where they left off.
 */
public string
query_default_start_location()
{
    if (query_ghost() & GP_BODY)
        return PATH + "bodies";
    else if (query_ghost() & GP_MANGLE)
        return PATH + "mirrors_1";
    else if (query_ghost() & GP_FEATURES)
        return PATH + "features";
    else if (query_ghost() & GP_SKILLS)
        return PATH + "skills";

    /* This shouldn't occur normally, unless the player's normal body was
     * messed up, or someone removed their ghost variable without replacing
     * this as their playerfile, or if their ghost var wasn't properly set
     * to include GP_BODY when they entered a body. This is a particularly
     * odd circumstance, as query_ghost() should _never_ be 0 in a player
     * that's using this body, but we'll let it pass in case it's a corrupt
     * playerfile that caused it. We will, however, log it.
     */
    else if (query_ghost() == 0)
    {
        log_file("GHOST_START_LOG",
            this_object()->query_real_name() + " logged in as " +
            file_name(this_object()) + " with a ghost_var of 0, on " +
            ctime(time()) + ".\n");
        return RACESTART[query_race()];
    }

    /* Should not happen. */
    return PATH + "bodies";
}

/*
 * Function name: damn_stubborn_object
 * Description  : This function is apparently needed to get rid of objects
 *                that refuse to destruct the first time.
 */
void
damn_stubborn_object()
{
    set_alarm(1.0, 0.0, damn_stubborn_object);
    destruct();
}

/*
 * Function name: ghost_ready
 * Description  : Moves the player into a 'real' body out of the ghost player
 *                body.
 */
public int
ghost_ready()
{
    string plfile;
    object ob, badge;
    int wizlev;

    /* Make sure the players ghost variable isn't set before 'unghosting' */
    if (query_ghost())
    {
        write("You still have things to do before entering the world.\n");
        return 0;
    }

    /* Make sure the player's current race name has a corresponding
     * playerfile.
     */
    plfile = RACEMAP[query_race()];
    if (!plfile)
    {
        write("You cannot be a " + query_race() +
              ", choose a new body!\n");

        /* Mark the player as being dead and in need of a new body. */
        set_ghost(GP_BODY | GP_DEAD);

        /* Restart sequence to put the player in the bodies room */
        enter_new_player(query_real_name(), "");
        return 0;
    }

    /* Clone a real body for the player to inhabit, ensuring that the object
     * cloned properly (i.e. that the playerfile is valid and functional)
     */
    ob = clone_object(plfile);
    if (!ob)
    {
        write(capitalize(query_race()) +
            " is a faulty race. Choose a new body!\n");

        /* Mark the player as dead and needing a new body. */
        set_ghost(GP_BODY | GP_DEAD);

        /* Re-enter the game in the bodies room. */
        enter_new_player(query_real_name(), "");
        return 0;
    }

    /*
     * Prepare the new player object. We must change:
     *      We should have the correct souls.
     *      We should have the correct playerfile.
     * Switch the 'connection' from this_object() to the newly cloned body
     * for the player.
     */
    exec(ob, this_object());

    /* Set the player's playerfile to the correct one. Up until now, it will
     * be this object's filename.
     */
    set_player_file(plfile);

    /* Reset the players movement messages from ghost messages to normal. */
    this_object()->transmsg_reset();

    /* Move them to their racial start location, and save the new data. */
    set_default_start_location(RACESTART[query_race()]);
    save_me(0);

    /*
     * Add a property to show that the player has just been created.
     */
    ob->add_prop("just_created", 1);

    /*
     * Enter the game and load the save file
     * Try to make the player enter the game with the new playerfile. If
     * enter_game failed, they'll have to try a new body.
     */
    if (!ob->enter_game(query_real_name(), ""))
    {
        write(capitalize(query_race()) +
            " is an illegal race. Choose a new body!\n");

        /* Mark the player as being dead and needing a new body. */
        set_ghost(GP_BODY | GP_DEAD);
        /* Re-enter the game as a ghost. */
        enter_new_player(query_real_name(), "");
        return 0;
    }

    /* Update the player's command hooks, as cmdsouls may have changed. */
    ob->update_hooks();
    ob->save_me(0);

    /* Notify security of the fact that a new player enters the game.
     * Let the wizards know a new player logged in.
     */
    SECURITY->notify(ob, 0);

    /* Now if we hammer hard enough it probably goes away. */
    set_alarm(1.0, 0.0, damn_stubborn_object);
    /* Use this_object()-> in order to also remove shadows. */
    this_object()->remove_object();
    destruct();
}

/*
 * Function name: reincarnate_me
 * Description:   Called by a player object that has been killed and must be
 *                turned into a 'ghost'.  This switches the command
 *                connection from dead player object to this ghost object.
 */
public void
reincarnate_me()
{
    object pl, gh;
    string n;

    pl = previous_object();

    /* Make sure the calling object is interactive and has a ghost variable
     * of some kind set.
     */
    if (!interactive(pl) || !pl->query_ghost())
        return;
    /* Mark the player as being dead and needing a body. */
    pl->set_ghost(GP_BODY | GP_DEAD);
    /* Remove any temporary starting locations this player has. */
    pl->set_temp_start_location(0);
    /* Save the current variables. */
    pl->reset_race_name();
    pl->save_me(0);

    /* This makes the real hostname carry on to the new player object */
    rhost = query_ip_name(pl);

    /* Without this uid/euid setting, this file does not have permission
     * to clone a copy of itself.
     */
    setuid();
    seteuid(getuid());

    /* Clone a 'copy' of this object. */
    gh = clone_object(MASTER);

    /* Switch command connection from the the 'dead' player body to the
     * freshly-cloned ghost body.
     */
    exec(gh, pl);

    /* Copy the player's bits from the dead player object over into the
     * ghost object.
     */
    gh->set_bit_array(pl->query_bit_array());

    /* Get the dead player's name, destroy the dead body, and make the
     * re-enter the game as a ghost.
     */
    n = pl->query_real_name();
    catch(pl->remove_object());
    if (objectp(pl))
    {
        SECURITY->do_debug("destroy", pl);
    }
    gh->enter_new_player(n);
}

/*
 * Function name: query_race
 * Description  : Always returns the true race of the player.
 * Returns      : string - the name of the race.
 */
nomask public string
query_race()
{
    return ::query_race();
}

/*
 * Function name: set_race_name
 * Description  : Disallow setting a non-standard race name.
 * Arguments    : string - the name to set.
 */
public void
set_race_name(string race)
{
    if ((race == NEWBIE_RACE_NAME) ||
        (member_array(race, RACES) > -1))
    {
        ::set_race_name(race);
    }
}

/*
 * Function name: query_gender_string
 * Description  : A ghost does not have a gender ;-) This is also done
 *                since the race cannot reflect ghost status anymore.
 * Returns      : string - always 'ghostly'
 */
public string
query_gender_string()
{
    return GHOSTLY_GENDER_STRING;
}

/*
 * Function name: stats_to_acc_exp
 * Description:   Translates the current base stats into acc_exp. This is used
 *                used when getting stats from a body.
 */
public nomask void
stats_to_acc_exp()
{
    ::stats_to_acc_exp();
}

public void
set_exp_quest(int e)
{
    ::set_exp_quest(e);
}

public void
set_exp_combat(int e)
{
    ::set_exp_combat(e);
}

public void
update_acc_exp(int e)
{
    ::update_acc_exp(e);
}

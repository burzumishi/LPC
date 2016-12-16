/*
 * /secure/master/mail_gc.c
 *
 * The mail system garbage collector.
 *
 * This object is part of the mail system, and is used to garbage collect
 * unreferenced messages.  It should be run periodically to remove extraneous
 * mail messages.
 *
 * This object is dedicated to Marvin.  It was written predominantly in a
 * functional style, as advocated by the practitioners of his arcane religion.
 * (Prove the correctness, Ancient Marvin, of this object.)
 *
 *	Cygnus (Dave Richards)
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

#include <mail.h>
#include <std.h>

#define	MBOXES_PER_ALARM (5)

static void step2(mapping map1, mapping map2);
static void step3(mapping map);
static void step4(string *mbox_dirs, string mbox_path, string *mboxes,
    mapping msg_map);
static void step5(mapping msg_map);

/*
 * function name: log
 * Description:   Log garbage collector messages.
 * Arguments:     msg - Message to log.
 * Returns:       None.
 */
static void
log(string msg)
{
    write_file("/syslog/log/MAIL_GC", msg);
}

/*
 * function name: directoryp
 * Description:   This function is a "directory predicate".  It indicates
 *		  whether the specified base_name + file_name refer to an
 *		  existing directory.
 * Arguments:     file_name - The directory name.
 *		  base_name - The parent directory name.
 * Returns:       1 - Directory exists, 0 - Directory does not exist.
 */
static status
directoryp(string file_name, string base_name)
{
    return file_size(base_name + file_name) == -2;
}

/*
 * function name: filep
 * Description:   This function is a "file predicate".  It indicates
 *		  whether the specified base_name + file_name refer to an
 *		  existing file.
 * Arguments:     file_name - The file name.
 *		  base_name - The parent directory name.
 * Returns:       1 - File exists, 0 - File does not exist.
 */
static status
filep(string fn, string bn)
{
    return file_size(bn + fn) >= 0;
}

/*
 * function name: scan_msg_dirs
 * Description:   This function is called repeatedly to store all message
 *		  file names in msg_map.  Messages will be removed from the
 *		  mapping as pointers to them from mboxes are scanned.
 * Arguments:     msg_dir - Message directory (e.g. "d0")
 *		  msg_map - Message mapping.
 * Returns:       0.
 */
static status
scan_msg_dirs(string msg_dir, mapping msg_map)
{
    msg_map[msg_dir] = get_dir(MSG_DIR + msg_dir + "/");

    return 0;
}

/*
 * function name: step1
 * Description:   Each message directory and message file name in MSG_DIR
 *		  is saved in a mapping.
 * Arguments:     None.
 * Returns:       None.
 */
static void
step1(void)
{
    string *msg_dirs;
    mapping msg_map;

    /*
     * Log an introduction.
     */

    log("Garbage collection started at " + ctime(time()) + ".\n");

    /*
     * Get the contents of the message directory.
     */

    msg_dirs = get_dir(MSG_DIR);

    /*
     * Remove elements which are not of the form "^d[0-9]+$".
     */

    msg_dirs = regexp(msg_dirs, "^d[0-9][0-9]*$");

    /*
     * Remove elements which are not directories.
     */

    msg_dirs = filter(msg_dirs, &directoryp(, MSG_DIR));

    /*
     * Create an empty mapping.
     */

    msg_map = ([]);

    /*
     * Generate a mapping of the form:
     *
     *     ([ <message directory> : ({ <message>, ... }), ... ])
     */

    filter(msg_dirs, &scan_msg_dirs(, msg_map));

    /*
     * Schedule step 2.
     */

    set_alarm(2.0, 0.0, &step2(msg_map, ([])));
}

/*
 * function name: step2
 * Description:   This function is called repeatedly to process each
 *		  message directory.  The goal is to eliminate message
 *		  file names which are incorrect.  This is done to reduce
 *		  storage requirements, and ensure that future analysis
 *		  is done correctly.
 * Arguments:     map1 - The old message map.
 *		  map2 - The new message map.
 * Returns:       None.
 */
static void
step2(mapping map1, mapping map2)
{
    string msg_dir;
    string *msgs;

    /*
     * If there are no elements in map1, then step 2 is complete.
     */

    if (m_sizeof(map1) == 0)
    {
	set_alarm(2.0, 0.0, &step3(map2, ([])));
	return;
    }

    /*
     * Get the next message directory.
     */

    msg_dir = m_indexes(map1)[0];

    /*
     * Get the messages in the message directory.
     */

    msgs = map1[msg_dir];

    /*
     * Remove elements which are not of the form "^m[0-9]+\.o$".
     */

    msgs = regexp(msgs, "^m[0-9][0-9]*\\.o$");

    /*
     * Remove elements which are not files.
     */

    msgs = filter(msgs, &filep(, MSG_DIR + msg_dir + "/"));

    /*
     * Maintain a mapping of message directories to messages, which meet
     * the above criteria.
     */

    if (sizeof(msgs) != 0)
	map2[msg_dir] = msgs;

    /*
     * Schedule the next cycle of step 2.
     */

    set_alarm(2.0, 0.0, &step2(m_delete(map1, msg_dir), map2));
}

/*
 * function name: step3
 * Description:   This function saves the names of all mbox directories
 *		  in an array.
 * Arguments:     msg_map - The message map.
 * Returns:       None.
 */
static void
step3(mapping msg_map)
{
    string *mbox_dirs;

    /*
     * Get the contents of the mail directory.
     */

    mbox_dirs = get_dir(MAIL_DIR);

    /*
     * Remove elements which are not of the form "^[a-z]$".
     */

    mbox_dirs = regexp(mbox_dirs, "^[a-z]$");

    /*
     * Remove elements which are not directories.
     */

    mbox_dirs = filter(mbox_dirs, &directoryp(, MAIL_DIR));

    /*
     * Schedule step 4.
     */

    set_alarm(2.0, 0.0, &step4(mbox_dirs, "", ({}), msg_map));

    SECURITY->mail_tell_wizard("MAIL ADMINISTRATOR ->> " +
	"Garbage collection preparation done.\n");
}

/*
 * function name: scan_mail
 * Description:   This function is called once for each message in an
 *		  mbox.  It removes the message from msg_map.
 * Arguments:     mail_map - The mail message.
 *		  msg_map - The message map.
 * Returns:       None.
 */
static void
scan_mail(mapping mail_map, mapping msg_map)
{
    int date;
    string msg_dir;
    string msg;
    string *msgs;

    /*
     * If the message is not a mapping, then the mbox is corrupt.
     */

    if (!mappingp(mail_map))
	return;

    /*
     * Get the message date.
     */

    date = mail_map[MAIL_DATE];

    /*
     * Compute the message directory from the date.
     */

    msg_dir = sprintf("d%d", date % HASH_SIZE);

    /*
     * Compute the message from the date.
     */

    msg = sprintf("m%d.o", date);

    /*
     * Get the messages in the message directory.
     */

    msgs = msg_map[msg_dir];

    /*
     * Remove the message from the message directory.
     */

    if (pointerp(msgs))
	msg_map[msg_dir] = msgs - ({ msg });
}

/*
 * function name: scan_mbox
 * Description:   This function is called once for each mbox.  It calls
 *		  scan_mail once for each message in the mbox.
 * Arguments:     mbox_map - The mbox map.
 *		  msg_map - The message map.
 * Returns:       None.
 */
static void
scan_mbox(mapping mbox_map, mapping msg_map)
{
    mapping *mail_vec;

    /*
     * Get the list of mail messages.
     */

    mail_vec = mbox_map[MAIL_MAIL];

    /*
     * Scan each message in the mailbox.
     */

    if (pointerp(mail_vec))
	filter(mail_vec, &scan_mail(, msg_map));
}

/*
 * function name: step4
 * Description:   This function is called repeated to scan mboxes and remove
 *		  all messages in msg_map which it finds in at least one mbox.
 * Arguments:     mbox_dirs - Mailbox directories (not yet processed).
 *		  mbox_path - Path to current mailbox directory.
 *		  mboxes - List of unprocessed mailboxes in mbox_path.
 *		  msg_map - The message map.
 * Returns:       None.
 */
static void
step4(string *mbox_dirs, string mbox_path, string *mboxes, mapping msg_map)
{
    int i;
    string mbox;
    mapping mbox_map;

    /*
     * If the mboxes array is empty, process the next mbox directory.
     */

    if (sizeof(mboxes) == 0)
    {
	/*
	 * If the mbox_dirs array is empty, then we're done with step 4.
	 * Schedule step 5.
	 */

	if (sizeof(mbox_dirs) == 0)
	{
	    SECURITY->mail_tell_wizard("MAIL ADMINISTRATOR ->> " +
		"Started deleting the garbage.\n");

	    set_alarm(2.0, 0.0, &step5(msg_map));
	    return;
	}

	SECURITY->mail_tell_wizard("MAIL ADMINISTRATOR ->> " +
	    "Collecting directory \"" + mbox_dirs[0] + "\".\n");

	/*
	 * Compute the next mbox_path.
	 */

	mbox_path = MAIL_DIR + mbox_dirs[0] + "/";

	/*
	 * Update mbox_dirs.
	 */

	mbox_dirs = exclude_array(mbox_dirs, 0, 0);

	/*
	 * Get the contents of the mbox directory.
	 */

	mboxes = get_dir(mbox_path);

	/*
	 * Remove elements which are not of the form "^[a-z]+\.o$".
	 */

	mboxes = regexp(mboxes, "^[a-z][a-z]*\\.o$");

	/*
	 * Remove elements which are not files.
	 */

	mboxes = filter(mboxes, &filep(, mbox_path));
    }
    else
    {
	/*
	 * Process mboxes.
	 */

	for (i = 0; i < MBOXES_PER_ALARM; i++)
	{
	    /*
	     * If the mboxes array is empty, stop.
	     */

	    if (sizeof(mboxes) == 0)
		break;

	    /*
	     * Get the next mbox.
	     */

	    mbox = mboxes[0];

	    /*
	     * Remove the .o suffix.
	     */

	    mbox = mbox[0..-3];

	    /*
	     * Update the mboxes array.
	     */

	    mboxes = exclude_array(mboxes, 0, 0);

	    /*
	     * Load the mbox.
	     */

	    catch(mbox_map = restore_map(mbox_path + mbox));

	    /*
	     * If we were unable to read the mbox, don't panic.
	     */

	    if (!mappingp(mbox_map))
		continue;

	    /*
	     * Remove all msgs in msg_map, referenced in mbox_map.
	     */

	    scan_mbox(mbox_map, msg_map);
	}
    }

    /*
     * Schedule the next cycle of step 4.
     */

    set_alarm(2.0, 0.0, &step4(mbox_dirs, mbox_path, mboxes, msg_map));
}

/*
 * function name: remove_msg
 * Description:   This function removes and logs each garbage message file.
 * Arguments:     msg - The message file.
 *		  msg_dir - The message directory.
 * Returns:       0.
 */
static status
remove_msg(string msg, string msg_dir)
{
    string path;

    path = sprintf("%s%s/%s", MSG_DIR, msg_dir, msg);

    if (rm(path) == 1)
	log(sprintf("Removed: %s\n", path));

    return 0;
}

/*
 * function name: step5
 * Description:   This function is called after all mboxes have been processed.
 *		  All messages found in all mboxes have been removed from the
 *		  message map.  Therefore, all messages that remain in the
 *		  message map are "garbage".
 * Arguments:     msg_map - The message map (garbage messages).
 * Returns:       None.
 */
static void
step5(mapping msg_map)
{
    string msg_dir, *msgs;

    /*
     * If the message map is empty, then we are done.
     */

    if (m_sizeof(msg_map) == 0)
    {
	/*
	 * Log a benediction.
	 */

	log("Garbage collection ended at " + ctime(time()) + ".\n");

	SECURITY->purge_crosscheck_done();

	destruct();
	return;
    }

    /*
     * Get the next message directory to process.
     */

    msg_dir = m_indexes(msg_map)[0];

    /*
     * Get the garbage messages in the message directory.
     */

    msgs = msg_map[msg_dir];

    /*
     * Remove the message directory from the message map.
     */

    m_delkey(msg_map, msg_dir);

    /*
     * Remove the messages.
     */

    filter(msgs, &remove_msg(, msg_dir));

    /*
     * Re-schedule step 5.
     */

    set_alarm(2.0, 0.0, &step5(msg_map));
}

/*
 * function name: start_garbage_collector
 * Description:   This function is called to start the garbage collection.
 *		  If one is already in progress a second will not be started.
 * Arguments:     None.
 * Returns:       None.
 */
void
start_garbage_collector()
{
    /*
     * This module may only be called from SECURITY.
     */

    if (previous_object() != find_object(SECURITY))
    {
	return;
    }

    /*
     * If a collection is in progress, don't start another one.
     */

    if (sizeof(get_all_alarms()) != 0)
	return;

    /*
     * Set our UID.
     */

    setuid();
    seteuid(getuid());

    /*
     * Start garbage collection.
     */

    step1();
}

/*
 * function name: remove_object
 * Description:   This function is called when the garbage collector is
 *		  updated/destroyed.  A message is logged to indicate ths.
 * Arguments:     None.
 * Returns:       None.
 */
void
remove_object()
{
    if (sizeof(get_all_alarms()) != 0)
    {
	log("Garbage collection aborted at " + ctime(time()) + ".\n");
	SECURITY->mail_tell_wizard("MAIL ADMINISTRATOR ->> " +
	    "Garbage collection aborted.\n");
    }

    destruct();
}

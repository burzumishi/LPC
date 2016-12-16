/*
 * /sys/global/adverbs.c
 *
 * This file holds the functions for manipulating adverbs. I stripped the
 * adverbs from /cmd/std/soul_cmd.c and made it accessible for everyone.
 * However, if you want to make a soul that uses adverbs, you'd rather access
 * the adverbs throught /cmd/std/command_driver.c which holds the functions
 * parse_adverb and parse_adverb_with_space. For more information... see that
 * file.
 *
 * The algorithm has been implemented such that it will always find the
 * first adverb in the list that matches the queried pattern in a very fast
 * way. Using binary search, even a list of 500 uses only 2log(500) ~= 10
 * probes to find the right adverb.
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

#include <adverbs.h>
#include <composite.h>
#include <macros.h>

#define DUMP_ADVERBS_OUT ("/open/dump_adverbs")

/*
 * Global variables. This is the list of all adverbs known to the mudlib. For
 * the algorithm it is VITAL that new adverbs are added in alphabetical order
 * for the search algorithm uses the fact that the list is ordered
 * alphabetically. Therefore, in create, we will sort the list for people who
 * are too sloppy to sort the list themselves ;-) The variable adverbs_size
 * contains the size of the array. The adverb_replacements is a mapping with
 * special replacements, formatted  ([ (string)adverb : (string)replacement ])
 */
static string *adverbs;
static int     adverbs_size;
static mapping adverb_replacements = ([ ]);

/*
 * Function name: create
 * Description  : Called upon initialization to read the adverbs into
 *                memory.
 */
nomask void
create()
{
    string *lines;
    string adverb;
    string replacement;
    int    index = -1;
    int    size;

    setuid();
    seteuid(getuid());

    /* Read the adverbs-file if possible. */
    if (file_size(ADVERB_SAVE_FILE) > 0)
    {
	adverbs = sort_array(explode(read_file(ADVERB_SAVE_FILE), "\n"));
	adverbs_size = sizeof(adverbs);
    }

    if (!adverbs_size)
    {
        adverbs = DEFAULT_ADVERB_ARRAY;
        adverbs_size = sizeof(adverbs);
    }

    /* Read the replacement adverbs-file if possible. */
    if (file_size(ADVERB_REPLACEMENT_SAVE_FILE) > 0)
    {
        lines = explode(read_file(ADVERB_REPLACEMENT_SAVE_FILE), "\n");
        size = sizeof(lines);
        while(++index < size)
        {
            if (sscanf(lines[index], "%s:%s", adverb, replacement) == 2)
            {
                adverb_replacements[adverb] = replacement;
            }
        }
    }
}

/*
 * Function name: short
 * Description  : Returns the short description of this object.
 * Returns      : string - the short description.
 */
nomask string
short()
{
    return "the fabric of adverbs";
}

/*
 * Function name: pattern_is_adverb
 * Description  : Returns whether a pattern matches the adverb at 'position'
 *                in the array of adverbs
 * Arguments    : string pattern - the pattern to match.
 *                int position   - the index of the wanted adverb.
 * Returns      : int 1 - match.
 *                    0 - no match.
 */
static nomask int
pattern_is_adverb(string pattern, int position)
{
    return wildmatch((pattern + "*"), adverbs[position]);
}

/*
 * Function name: member_adverb
 * Description  : This routine uses a binary search to match a pattern against
 *                all adverbs known. The binary search will be a lot faster
 *                than a check with a for-loop. It is implemented with a loop
 *                rather than with recursion to avoid a too deep recursion
 *                problem if the adverb list extends. The search function will
 *                be guaranteed to find the first adverb in the list that
 *                matches a certain patter.
 * Arguments    : string pattern - the pattern to check on being an adverb.
 * Returns      : int >= 0 - the position of the adverb.
 *                    -1   - if the adverb does not exist.
 *                    -2   - it is a special service adverb.
 */
public nomask int
member_adverb(string pattern)
{
    int low;
    int high;
    int half;

    /* Not a real adverb. */
    if (member_array(pattern, SERVICE_ADVERBS_ARRAY) != -1)
    {
        return -2;
    }

    /* You need at least three characters to identify an adverb. */
    if (strlen(pattern) < 3)
    {
        return -1;
    }

    /* Special consideration for "sad" -> "sadly" rather than "sadistically".
     * If there are more of these special considerations, it should be made
     * into a configuration file just like the ADVERB_REPLACEMENTS. Mercade
     */
    if (pattern == "sad")
    {
        return member_array("sadly", adverbs);
    }

    low = 0;
    high = (adverbs_size - 1);

    /* This is the actual binary search loop. It searches until it has found
     * an interval of size two that includes the adverb we were looking for.
     * Even for a list of 500 adverbs it will only rank 2log(500) ~= 10 turns
     * of the loop to find the adverb, so no additional overhead to check
     * whether the low boundary might match the adverb is added.
     */
    while ((low + 1) < high)
    {
        half = ((low + high) / 2);

        if (pattern < adverbs[half])
        {
            high = half;
            continue;
        }
        low = half;
    }

    /* Now the adverb can be either the low or the high end of this range
     * of two. If not, the adverb was not found and we return -1.
     */
    if (pattern_is_adverb(pattern, low))
    {
        return low;
    }
    if (pattern_is_adverb(pattern, high))
    {
        return high;
    }

    return -1;
}

/*
 * Function name: full_adverb
 * Description  : If the first part of an adverb is given, the complete
 *                adverb is returned. If the pattern was not recognized,
 *                an empty string will be returned.
 * Arguments    : string pattern - the pattern to check on being an adverb.
 * Returns      : string string - the complete adverb or "".
 */
public nomask string
full_adverb(string pattern)
{
    int index = member_adverb(pattern);

    /* Adverb is really a service adverb. Just return in. */
    if (index == -2)
    {
        return pattern;
    }

    /* Adverb is not found. */
    if (index == -1)
    {
        return NO_ADVERB;
    }

    /* Adverb is found, but should be replaced by a better phrase. */
    if (stringp(adverb_replacements[adverbs[index]]))
    {
        return adverb_replacements[adverbs[index]];
    }

    /* Adverb is found and returned. */
    return adverbs[index];
}

/*
 * Function name: adverb_at_pos
 * Description  : Give the adverb with a certain index in the array
 * Arguments    : int index - the index in the array.
 * Returns      : string - the index'th adverb if the array is that long.
 */
public nomask string
adverb_at_pos(int index)
{
    if ((index >= 0) &&
    	(index < adverbs_size))
    {
        return adverbs[index];
    }

    return NO_ADVERB;
}

/*
 * Function name: dump_adverbs
 * Description  : This function can be used to dump all adverbs to a file
 *                in a sorted and formatted way. This dump can be used for
 *                the 'help adverbs' document. The output of this function
 *                will be written to the file DUMP_ADVERBS_OUT.
 * Returns      : int 1 - always.
 */
public nomask int
dump_adverbs()
{
    int index = -1;
    int size = strlen(ALPHABET);
    string *words;

    catch(rm(DUMP_ADVERBS_OUT));
    while(++index < size)
    {
	words = filter(adverbs, &wildmatch((ALPHABET[index..index] + "*")));

	if (!sizeof(words))
	{
	    continue;
	}

	if (strlen(words[0]) < 16)
	{
	    words[0] = (words[0] + "                ")[..15];
	}
	write_file(DUMP_ADVERBS_OUT,
	    sprintf("%-76#s\n\n", implode(words, "\n")));
    }

    return 1;
}

/*
 * Function name: query_all_adverbs
 * Description  : Returns an array with all adverbs known to the mudlib. This
 *                list is sorted alphabetically.
 * Returns      : string * - the list of adverbs.
 */
string *
query_all_adverbs()
{
    return adverbs + ({ });
}

/*
 * Function name: query_all_adverb_replacements
 * Description  : Returns a mapping with all the adverbs that are replaced by
 *                a more suitable phrase. The format is:
 *                    ([ (string)adverb : (string)replacement ])
 * Returns      : mapping - the array with replaced adverbs.
 */
mapping
query_all_adverb_replacements()
{
    return adverb_replacements + ([ ]);
}

/*
 * /lib/cache.c
 *
 * This module functions as a cache on disk reads. If you need to re-read
 * some information each time a player uses a particular tool or feature,
 * a read cache may be very handy. What it does is store the last <size>
 * different read files in memory and if you query information that already
 * is in the cache, a cpu-intensive disk read will be avoided! Note that
 * this has its best results if the individual data files are relatively
 * small.
 *
 * The default size of the cache is 10 data files. If you think more
 * different files need to be accessible at the same time, you may extend
 * the cache size to a maximum of 25. To do this, call the following
 * function from your create_whatever() function.
 *
 *     void set_cache_size(int size)
 *
 * Read caching is only possible when replacing the efuns save_map() and
 * restore_map(). It is not possible to cache on objects that are restored
 * via the efun restore_object(). However, all objects that function on
 * restore_object() can easily be adapted to use save_map() and restore_map().
 * To use the cache just replace calls to the efuns
 *
 *     mapping restore_map(string filename)
 *     void save_map(mapping data, string filename)
 *     int rm(string filename)
 *
 * with calls to the cache-functions
 *
 *     mapping read_cache(string filename)
 *     void save_cache(mapping data, string filename)
 *     int rm_cache(string filename)
 *
 * The arguments and return values of these functions replacing the efuns are
 * identical. There are three additional cache-management functions. The first
 * returns the filenames of all data files currently in the cache, the second
 * will return whether or not a particular data file is in the cache and the
 * third can be used to remove a certain data file from the cache. Finally,
 * you may want to flush the cache and remove all files from it.
 *
 *     string *query_cache()
 *     int in_cache(string filename)
 *     void remove_from_cache(filename)
 *     void reset_cache()
 *
 * WARNING
 * If you want to rename a data file that is (or might be) in the cache, you
 * have to remove it from the cache first, because the cache does NOT support
 * a cache version of the rename() efun.
 */

#pragma no_clone
#pragma save_binary
#pragma strict_types

#define DEFAULT_CACHE 10
#define MINIMUM_CACHE 10
#define MAXIMUM_CACHE 25

/*
 * These global variables are private and static. They will not be saved
 * and are invisible to inheriting objects.
 */
static private mapping  cache_map   = ([ ]);
static private string  *cache_order = ({ });
static private int      cache_size  = DEFAULT_CACHE;
static private int      cache_tries = 0;
static private int      cache_hits  = 0;

/*
 * Function name: set_cache_size
 * Description  : With this function you can set the cache size. It is
 *                defaulted to 10 and values ranging from 10 to 25 will
 *                be accepted.
 * Arguments    : int size - the size of the cache.
 */
nomask static void
set_cache_size(int size)
{
    if ((size >= MINIMUM_CACHE) &&
	(size <= MAXIMUM_CACHE))
    {
	cache_size = size;
    }
}

/*
 * Function name: query_cache_size
 * Description  : This function returns the size of the cache.
 * Returns      : int - the size of the cache.
 */
nomask public int
query_cache_size()
{
    return cache_size;
}

/*
 * Function name: in_cache
 * Description  : Call this function to find out whether the contents of a
 *                particular data file is in the cache.
 * Arguments    : string filename - the filename to check.
 * Returns      : int 1/0 - true if the data file is in the cache.
 */
nomask static int
in_cache(string filename)
{
    return (member_array(filename, cache_order) > -1);
}

/*
 * Function name: read_cache
 * Description  : This function is the real cache. It takes the filename of
 *                the data file as argument and if the information is not
 *                found in the cache, it is read from disk and stored in the
 *                cache for later reference. To use the cache, just replace
 *                call to the efun restore_map() to the lfun read_cache().
 *                The arguments and return values are exactly the same.
 * Arguments    : string filename - the filename to read.
 * Returns      : mapping - the data file read from the save-file or ([ ]) in
 *                          case of an error.
 */
nomask static mapping
read_cache(string filename)
{
    mapping data;
    int     pos;

    /* Count the number of tries to the cache. */
    cache_tries++;

    /* Remove the trailing ".o" if it is added to the path. */
    sscanf(filename, "%s.o", filename);

    /* See whether the information with that name is already in the
     * cache. Yes, HIT, no load == cpu saved!
     */
    if ((pos = member_array(filename, cache_order)) > -1)
    {
	/* Count the number of hits in the cache. */
	cache_hits++;

	/* If the requested information is not on the top of the cache,
	 * put it on top.
	 */
	if (pos > 0)
	    cache_order = ({ filename }) + cache_order[..(pos - 1)] +
		cache_order[(pos + 1)..];

	/* Return the cached information, that is... a copy of it. */
	return secure_var(cache_map[filename]);
    }

    /* The information is apparently not in the cache. Read the file from
     * disk. In case of an error, return the empty mapping. This will
     * probably not happen because even if you try to restore a non-existant
     * mapping, the efun restore_map will return an empty mapping. *duh*
     */
    if (catch(data = restore_map(filename)) ||
	!mappingp(data))
	data = ([ ]);

    /* If the cache is too large, reduce its size. */
    while((pos = sizeof(cache_order)) >= cache_size)
    {
	m_delkey(cache_map, cache_order[pos - 1]);
	cache_order = cache_order[..(pos - 2)];
    }

    /* Add the read information to the cache. */
    cache_order = ({ filename }) + cache_order;
    cache_map[filename] = data;

    /* Return the read information, that is... a copy of it. */
    return secure_var(data);
}

/*
 * Function name: save_cache
 * Description  : When information that is in the cache is saved, then not
 *                only the information on disk should be changed, but the
 *                information in the cache should be altered too. The
 *                arguments to this function are exactly the same as to
 *                the efun save_map(). This function does NOT cache on the
 *                save, i.e. when the data is not in the cache when it is
 *                saved, it will not be added to the cache. However, in
 *                practice I don't think a file will ever be saved without
 *                being in the cache.
 * Arguments    : mapping data     - the data to save.
 *                string  filename - the filename to save to.
 */
nomask static void
save_cache(mapping data, string filename)
{
    /* Remove the trailing ".o" if there is one. */
    sscanf(filename, "%s.o", filename);

    /* If the entry is in the cache, update it. */
    if (in_cache(filename))
    	cache_map[filename] = secure_var(data);

    /* Save the information as usual. */
    save_map(data, filename);
}

/*
 * Function name: rm_cache
 * Description  : When you remove a file that is currently in the cache,
 *                you should use this function rather than the efun rm()
 *                in order to remove the information from the cache too.
 *                If you fail to do this, the information will remain in
 *                the cache while the original file does not exist anymore.
 *                The argument and return value are exactly the same as
 *                those of the efun rm().
 * Arguments    : string filename - the filename to remove.
 * Returns      : int 1/0 - the return value from the efun rm.
 */
nomask static int
rm_cache(string filename)
{
    /* Remove the trailing ".o" if there is one. */
    sscanf(filename, "%s.o", filename);

    /* If the information is in the cache, remove it from the cache. */
    if (member_array(filename, cache_order) > -1)
    {
    	cache_order -= ({ filename });
    	m_delkey(cache_map, filename);
    }

    /* Remove the file as usual. */
    return rm(filename + ".o");
}

/*
 * Function name: remove_from_cache
 * Description  : It may be necessary to remove a data file from the cache.
 *                This function does that for you. An example of a situation
 *                in which this is necessary is if you want to rename a
 *                data file that is in the cache, for there is no cache
 *                version of the efun rename() because of implementational
 *                difficulties. There is no return function because it will
 *                always succeed.
 * Arguments    : string filename - the filename to remove.
 */
nomask static int
remove_from_cache(string filename)
{
    /* Remove the trailing ".o" if there is one. */
    sscanf(filename, "%s.o", filename);

    /* If the information is in the cache, remove it from the cache. */
    if (member_array(filename, cache_order) > -1)
    {
    	cache_order -= ({ filename });
    	m_delkey(cache_map, filename);
    }
}

/*
 * Function name: reset_cache
 * Description  : This function will flush the cache and remove all data
 *                files from it. This will save memory, but has no other
 *                effects. The cache size will not be reset.
 */
nomask static void
reset_cache()
{
    cache_map   = ([ ]);
    cache_order = ({ });
}

/*
 * Function name: query_cache
 * Description  : This will return an array containing the file names of all
 *                data files stored in the cache. The list is sorted, with
 *                the data file most recently queried is on top of the array,
 *                etcetera.
 * Returns      : string * - an array with the file names of all data files
 *                           stored in the cache.
 */
nomask static string *
query_cache()
{
    return secure_var(cache_order);
}

/*
 * Function name: cache_report
 * Description  : To get a very small report on the efficiency of the cache,
 *                call this function. The results will be printed with write().
 */
nomask public void
cache_report()
{
    write(sprintf("Cache tries %6d\nCache hits  %6d\nHit ratio   %6d%%\n",
	cache_tries, cache_hits, ((cache_hits * 100) / cache_tries)));
}

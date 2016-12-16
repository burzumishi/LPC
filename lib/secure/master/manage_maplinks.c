/*
  /secure/master/manage_maplinks.c

  Module to /secure/master.c
  Handles all registering of special WL coordinates in the map

*/
#include "/sys/map.h"

int find_remote_map_sym(string wlc);
void set_domain_bit(int x, int y);

/*
   This mapping holds the mapmanagers of each Domain
*/
mapping map_managers;


/*
  The mapping holding all map links 

  NOTE
      This is made static and not saved to KEEPERSAVE because each domain
      should in the preload phase link themselves to the map.
*/
static mapping m_maplinks = ([]);     
static mapping single_locs = ([]);	    /* ([ loc:domain ]) */
static mixed *boxes = ({});		    /* ({ ({ box, domain }), ... }) */

/*
 * Function name:   set_map_manager
 * Description:     Notifies us that a domain has a map manager file
 *		    This file will be called to add a maplink
 *		    (see 'add_maplink)
 * Arguments:       file: Filename of the file to call 'make_link' in when
 *			  the maplink is to be added.
 * Returns:         True if successfull.
 */
public int
set_map_manager(string file)
{
    string domain;

    if (!mappingp(map_managers))
	map_managers = ([]);

    domain = geteuid(previous_object());

    if (this_object()->query_domain_number(domain) < 0)
    {
        domain = this_object()->query_wiz_dom(domain);
        if (this_object()->query_domain_number(domain) < 0)
    	    return 0;
    }

    map_managers[domain] = file;
    seteuid(ROOT_UID);
    save_object(SAVEFILE);
}

/*
 * Function name:   setup_maplinks
 * Description:     Orders all the domains map managers to setup maplinks
 */
void
setup_maplinks()
{
    int il, sflag;
    string *managers, *dix;

    if (!mappingp(map_managers))
	return 0;

    managers = m_values(map_managers);
    dix = m_indexes(map_managers);
    sflag = 0;

    for (il = 0; il < sizeof(managers); il++)
    {
	if (stringp(catch(call_other(managers[il], "teleledningsanka"))))
	{
	    m_delkey(map_managers, dix[il]);
	    sflag = 1;
	    continue;
	}
	call_out("setup_one_maplink", 1, managers[il]);
    }
    if (sflag)
    {
	seteuid(ROOT_UID);
	save_object(SAVEFILE);
    }
}

/*
 * Function name:   setup_one_maplinks
 * Description:     Orders one of domains map managers to setup maplinks
 */
void
setup_one_maplink(string manager)
{
    manager->make_link();
}

    
/*
 * Function name:   add_maplink
 * Description:     Adds a new link to the global map.
 *		    Each domain can add one such link.
 * Arguments:       file: Filename of the file to call 'find_maplink' in when
 *			  a wlc is within one of the defined boxes.
 *		    box:  An square on the map defined as:
 *			       "xnnnn-nnnnynnnn-nnnn"
 *		    singles: A list of single maplocations on the form:
 *			       "xnnnnynnnn"
 * Returns:         True if successfull.
 */
public int
add_maplink(string file, string box, string *singles)
{
    string domain;
    mixed *old;
    int il, il2, x, xd, y, yd;

    if (!mappingp(m_maplinks))
	m_maplinks = ([]);

    domain = geteuid(previous_object());

    if (this_object()->query_domain_number(domain) < 0) {
        domain = this_object()->query_wiz_dom(domain);
        if (this_object()->query_domain_number(domain) < 0)
    	    return 0;
    }

    old = m_maplinks[domain];
    if (pointerp(old))
    {
	if (pointerp(old[2]))
	    for (il = 0; il < sizeof(old[2]); il++)
		m_delkey(single_locs, old[2][il]);
	
	if (pointerp(boxes))
	    for (il = 0; il < sizeof(boxes); il++)
		if (boxes[il][1] == domain)
		    boxes = exclude_array(boxes, il, il);
    }

    if (stringp(singles))
	singles = ({ singles });

    m_maplinks[domain] = ({ file, box, singles });
    
    for (il = 0; il < sizeof(singles); il++)
    {
	single_locs[singles[il]] = domain;
	if (sscanf(singles[il], "x%dy%d", x, y) == 2)
	    set_domain_bit(x,y);
    }

    boxes += ({ ({ box, domain }) });
    if (stringp(box) && sscanf(box, "x%d-%dy%d-%d", x, xd, y, yd) == 4)
    {
	for (il = x; il < xd; il++)  for (il2 = y; il2 < yd; il2++) 
	    set_domain_bit(il, il2);
    }
    return 1;
}

static int 
set_domain_bit(int x, int y)
{
    "/std/map/map"->set_domain_bit(x, y, 1);
}

/*
 * Function name:   query_maplink
 * Description:     Gives the maplink for a specific domain
 * Arguments:       domain: The name of the domain.
 * Returns:         Maplink on the form: ({ file, box, singles }) or 0
 */
public string
query_maplink(string domain)
{
    if (!mappingp(m_maplinks))
	m_maplinks = ([]);

    return m_maplinks[domain];
}

/*
 * Function name:   query_maplink_domains
 * Description:     Gives all domains that are linked to the map
 * Returns:         Array of domain names
 */
public string *
query_maplink_domains()
{
    if (!mappingp(m_maplinks))
	return ({});

    return m_indexes(m_maplinks);
}

public string *query_singles() { return m_indexes(single_locs); }
public string *query_singles_doms() { return m_values(single_locs); }


/*
 * Function name:   within_box
 * Description:     Decides wheter a wlc is within a box
 * Arguments:       wlc: Location on the form "xnnnnynnnn"
 *		    box:  An square on the map defined as:
 *			       "xnnnn-nnnnynnnn-nnnn"
 * Returns:	    True if within box		    
 */
public int within_box(string wlc, string box)
{
    int x, xd, y, yd, xp, yp;


    if (!strlen(wlc) || sscanf(wlc, "x%dy%d", xp, yp) != 2)
	return 0;

    if (!strlen(box) || sscanf(box, "x%d-%dy%d-%d", x, xd, y, yd) != 4)
	return 0;

    if (xp >= x && xp <= xd && yp >= y && yp <= yd)
	return 1;

    return 0;
}

/*
 * Function name:   find_maplink
 * Description:     Finds out if there is a special file linked to a given WL,
 *    		    otherwise it queries the external database handler for
 *		    a map char for the WL and decides which file that is.
 * Arguments:       wlc: WL coordinate as a string, "xnnnnynnnn"
 * Returns:         Filename of the file to instance for this WL
 */
string
find_maplink(string wlc)
{
    int char, pos, il;
    int wlx, wly, slx, sly;
    string file, str, dom, newwlc;
    mixed link;

    char = find_remote_map_sym(wlc);

    sscanf(wlc,"x%d.%dy%d.%d",wlx,slx,wly,sly);
    newwlc = "x" + wlx + "y" + wly;

    if((char & DOMAIN_BIT) && mappingp(m_maplinks))
    {
	dom = single_locs[newwlc];
	if (!dom)
	    for (il = 0; il < sizeof(boxes); il++)
		if (within_box(newwlc, boxes[il][0]))
		    dom = boxes[il][1];

	if (dom)
	{
	    link = m_maplinks[dom];
	    file = call_other(link[0], "find_maplink", wlc);
	    if (stringp(file) && file_size(file + ".c"))
	        return file;
	}

    }

    file = MAP_ROOMS[char & PLAIN];
    if (stringp(file) && file_size(file + ".c"))
	return file;

    return MAP_DEFAULT_ROOM;
}

/*
 * Function name:   create_maproom
 * Description:     Creates a room in the maproom directory
 * Arguments:       xyc: Coordinates as a string, "xnnnn.mmynnnn.mm"
 *		    file_info: The text to be written into the file.
 * Returns:         True if successfull
 */
int
create_maproom(string xyc, string file_info)
{
   int wlx,wly,slx,sly;
   string fname;

   if (sscanf(xyc,"x%d.%dy%d.%d",wlx,slx,wly,sly) != 4)
       return 0;
   
   seteuid(ROOT_UID);

   fname = MAP_PATH + "/" + xyc + ".c";
   if (file_size(fname) != -1)
       rm(fname);
   write_file(fname, file_info);
   return 1;
}

/*
 * Function name:   remove_maproom
 * Description:     Removes a room in the maproom directory
 * Returns:         True if successfull
 */
int
remove_maproom()
{
    string fname;
    int wlx,slx,wly,sly;

    fname = file_name(previous_object());

    if (sscanf(fname, MAP_PATH + "/x%d.%dy%d.%d",wlx,slx,wly,sly) != 4)
       return 0;

    seteuid(ROOT_UID);
    return rm(fname + ".c");
}

#define SUBSQ_SIZE ((MAP_SL_SIZE + 1) / 3)
#define MAP_MIX_PROB(sl) (-25 + 25 * ABS((sl) - 4))

#define TERRAIN_PRIORITY ({ 1,  2,  3,  4,  5,  6,  7,  8,\
                            9, 10, 11, 12, 13, 14, 15, 16,\
                           17, 18, 19, 20, 21, 22, 23, 24,\
                           25, 26, 27, 28, 29, 30, 31, 32})

#define SEED1 4547
#define SEED2 2393
#define SEED3 6679

/*
 *  This seed algorithm is popularly known as the "prayer" algorithm,
 *  since it probably hasn't one of being very good.  Substitutions
 *  would be welcomed.
 */

private int
map_seed(int wlx, int wly)
{
    return SEED1 * wlx  + SEED2 * wly + SEED3;
}

int
prob(int slx,int sly)
{
    int a, b;

    a = MAP_MIX_PROB(slx);
    b = MAP_MIX_PROB(sly);

    return MAX(a,b);
}

/*
 * Function name:   find_remote_map_sym
 * Description:     Finds the corresponding map symbol for a given WLC
 * Arguments:       wlc: Coordinates as a string, "xnnnnynnnn"
 * Returns:         Symbol as string containing one char
 */
int
find_remote_map_sym(string wlc)
{
    int this_type;
    int other_type;
    int check_type;
    int wlx, wly, slx, sly; 
    int ssx, ssy;
    mixed *terrain;
 
    sscanf(wlc,"x%d.%dy%d.%d", wlx, slx, wly, sly);
 
    terrain = "/std/map/map"->get_nine(wlx, wly);
    this_type = terrain[0][4];

    ssx = slx % SUBSQ_SIZE - 1;
    ssy = sly % SUBSQ_SIZE - 1;

    if (!ssx && !ssy)
        return this_type;

    other_type = terrain[0][4 + 3*ssx - ssy];

    if (ssx && ssy) {
        check_type = terrain[0][4 - ssy];

        if (TERRAIN_PRIORITY[this_type & PLAIN] >
            (TERRAIN_PRIORITY[other_type & PLAIN]))
            other_type = check_type;

        check_type = terrain[0][4 + 3*ssx];

        if (TERRAIN_PRIORITY[this_type & PLAIN] >
            (TERRAIN_PRIORITY[check_type & PLAIN]))
            other_type = check_type;
    }
     
    if ((this_type & DOMAIN_BIT) || (other_type & DOMAIN_BIT))
        return this_type;
 
    if (TERRAIN_PRIORITY[this_type & PLAIN] >
        (TERRAIN_PRIORITY[other_type & PLAIN]))
        return this_type; 
 
    if (random(100, map_seed(wlx, wly)) < prob(slx, sly))
        return other_type;
 
    return this_type;
}

/*
 * Function name:   find_remote_map_char
 * Description:     Finds the corresponding map character for a given WLC
 * Arguments:       wlc: Coordinates as a string, "xnnnnynnnn"
 * Returns:         True if successfull
 */
string
find_remote_map_char(string wlc)
{
    string ch;
    int sym;
    
    sym = find_remote_map_sym(wlc);
    ch = MAP_CHARS[sym & PLAIN]; 
    if (stringp(ch))
	return ch;
    else
	return MAP_DEFAULT_CHAR;
}

	

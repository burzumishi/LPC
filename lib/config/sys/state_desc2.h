/*
   sys/state_desc2.h

   Holds all textual descriptions of state descriptions of livings.
   
   Here local changes are made to the arrays defined in
   /sys/state_desc.h
*/

/*
   If you insert changes you are recommended to copy /sys/state_desc.h
   to here and make changes. It is important that the below define is
   defined afterwards:
  
#ifndef SD_DEFINED
#define SD_DEFINED
#endif

*/

/*
 * SD_AV_NUM_TITLES - the number of titles.
 * SD_AV_TITLES - the titles themselves.
 * SD_AV_LEVELS - the stat level from which you get a title.
 * 
 * The mortal 'level' titles.
 */
#define SD_AV_NUM_TITLES 4

#define SD_AV_TITLES ({ "novice",           \
			"apprentice",       \
			"veteran",          \
			"champion" })

#define SD_AV_LEVELS ({  10, \
			 40, \
			 80, \
			130 })


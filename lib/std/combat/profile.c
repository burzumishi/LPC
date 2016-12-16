
/*
 * To use this profiling package, just include it in the object,
 * insert PROFILE() macros where you are interested, and #include
 * this file as necessary.  A call to start_profiling() is required
 * in the objects constructor.
 */
#define DO_PROFILE
#ifdef  DO_PROFILE
#    define PROFILE(func) "/std/combat/cbase"->profile(func)
#else
#    define PROFILE(func) ;
#endif

static mapping profile_data;

void
start_profiling()
{
    if (mappingp(profile_data))
	return;
    if(file_name(this_object()) != "/std/combat/cbase")
    {
	"/std/combat/cbase"->start_profiling();
        return;
    }
    profile_data = ([]);
}

void
profile(string func)
{
    profile_data[func]++; /* This caused a runtime error /Cmd */
}

void
print_profile(int max_values)
{
    string *funcs;
    int *values;
    int n;
    mixed temp;
    int pass;
    int i,j,k;
    int newk;

    if(file_name(this_object())!="/std/combat/cbase")
    {
        "/std/combat/cbase"->print_profile();
        return;
    }
    funcs = m_indexes(profile_data);
    values = m_values(profile_data);
    n = sizeof(values);
/*
 * I apologise for this abomination.  It does a heapsort in place.
 * I made the "pass" variable in order to reuse the code for both
 * building the heap and restoring the heap during the sort.  Pass 0
 * builds the heap, pass 1 does the sort.
 *
 * The while loop contains the main "make_heap" code.  
 *
 * The algorithm is much simpler than this implementation, but this
 * is done without benefit of recursion, global variables, or in/out
 * parameter passing.  
 */

    for( pass = 0; pass <= 1; pass++)
        for(i = (n-1)/(2-pass); i >= 0; i--)
        {
            if(pass==0)
            {
                k = i;
                j = n-1;
            }
            else
            {
		/* Interchange root and last element.*/
		temp = funcs[i];
		funcs[i] = funcs[0];
		funcs[0] = temp;
		temp = values[i];
		values[i] = values[0];
		values[0] = temp;
                k = 0;
                j = i-1;
            }
            /*
             * HEAPIFY procedure
             */

	    while ((k < (j+1) / 2 - 1)
		&& ((values[2*k+1] < values[k]) 
		|| (values[2*k+2] < values[k])))
	    {
		/* Find the son which is lowest. */
		if (values[2*k+1] > values[2*k+2])
                    newk = 2*k+2; 
                else
                    newk = 2*k+1; 

		/* Interchange parent and son */
		temp = funcs[newk];
		funcs[newk] = funcs[k];
		funcs[k] = temp;

		temp = values[newk];
		values[newk] = values[k];
		values[k] = temp;

		k = newk;
	    }
        }

/* End abomination */


    write (sprintf("/std/combat/cbase function call statistics.\n"));
    write (sprintf("-------------------------------------------\n"));
    for(i = 0; (i < n) && (i < max_values); i++)
        write(sprintf("%20s : %i\n",funcs[i], values[i]));

}

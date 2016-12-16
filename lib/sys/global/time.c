/*
 * /sys/global/time.c
 *
 * This library object includes some time-functions that may be handy when
 * converting time to strings. The functions are available through the header
 * file #include <time.h>
 */

#pragma no_clone
#pragma no_inherit
#pragma save_binary
#pragma strict_types

/*
 * These private variables keep the number of days, hours, minutes and
 * seconds of a given time. They are kept globally in this module for
 * easy access.
 */
private int time_day  = 0;
private int time_hour = 0;
private int time_min  = 0;
private int time_sec  = 0;

private mapping MonthToNumMap = ([ "Jan" : 1, "Feb" : 2, "Mar" : 3,
    "Apr" : 4, "May" : 5, "Jun" : 6, "Jul" : 7, "Aug" : 8, "Sep" : 9,
    "Oct" : 10, "Nov" : 11, "Dec" : 12 ]);

/*
 * Function name: time_to_numbers
 * Description  : This function is an internal function in this module. It
 *                converts the number of days, hours, minutes and seconds
 *                from a given time and stores them in the appropriate
 *                private global variables.
 * Arguments    : int time - the time to convert.
 */
nomask private void
time_to_numbers(int time)
{
    int n;

    /* Compute the number of days. */
    time_day = (n = time / 86400);
    time -= n * 86400;

    /* Compute the number of hours. */
    time_hour = (n = time / 3600);
    time -= n * 3600;

    /* Compute the number of minutes. */
    time_min = (n = time / 60);
    time -= n * 60;

    /* Whatever remains are the seconds. */
    time_sec = time;
}

/*
 * Function name: time2num
 * Description  : This function will convert a time to its component
 *                number of days, hours, minutes and seconds and returns
 *                these values in an array. If you include <time.h> into
 *                your file, you can access this function via the macro
 *                TIME2NUM(t).
 * Arguments    : int time - the time to convert.
 * Returns      : int *    - an array with the days, hours, minutes and
 *                           seconds.
 */
nomask public int *
time2num(int time)
{
    time_to_numbers(time);

    return ({ time_day, time_hour, time_min, time_sec });
}

/*
 * Function name: convtime
 * Description  : This function will convert a given time into a string
 *                of the form 'dd days hh hours mm minutes ss seconds'
 *                where only those elements that are non-zero are printed.
 *                Include <time.h> in your file and you can access this
 *                function via the macro CONVTIME(t).
 * Arguments    : int time - the time in seconds to print.
 * Returns      : string   - the time in string-form.
 */
nomask public string
convtime(int time)
{
    string res = "";

    time_to_numbers(time);

    if (time_day)
    {
	res = time_day + ((time_day == 1) ? " day" : " days");
    }

    if (time_hour)
    {
	if (strlen(res))
	    res += " ";

	res += time_hour + ((time_hour == 1) ? " hour" : " hours");
    }

    if (time_min)
    {
	if (strlen(res))
	    res += " ";

	res += time_min + ((time_min == 1) ? " minute" : " minutes");
    }

    if (time_sec)
    {
	if (strlen(res))
	    res += " ";

	res += time_sec + ((time_sec == 1) ? " second" : " seconds");
    }

    return res;
}

/*
 * Function name: time2str1
 * Description  : This function will return the time in a string with only
 *                the largest significant element, i.e. days, hours, minutes
 *                or seconds. The name will be abbreviated to only one
 *                character.
 * Returns      : string   - the string describing the time.
 */
nomask private string
time2str1()
{
    if (time_day)
    {
	return time_day + " d";
    }

    if (time_hour)
    {
	return time_hour + " h";
    }

    if (time_min)
    {
	return time_min + " m";
    }

    return time_sec + " s";
}

/*
 * Function name: time2str2
 * Description  : This function returns a string containing a textual
 *                representation of a time using only two significant
 *                time-elements.
 * Returns      : string   - the converted time.
 */
nomask private string
time2str2()
{
    /* Return days. */
    if (time_day)
    {
	/* Returns days and hours. */
	if (time_hour)
	{
	    return sprintf("%3d d %2d h", time_day, time_hour);
	}

	/* Returns days and minutes. */
	if (time_min)
	{
	    return sprintf("%3d d %2d m", time_day, time_min);
	}

	/* Return days and seconds. */
	return sprintf("%3d d %2d s", time_day, time_sec);
    }

    /* Return hours. */
    if (time_hour)
    {
	/* Return hours and minutes. */
	if (time_min)
	{
	    return sprintf("%3d h %2d m", time_hour, time_min);
	}

	/* Return hours and seconds. */
	return sprintf("%3d h %2d s", time_hour, time_sec);
    }

    /* Return minutes and seconds. */
    if (time_min)
    {
	return sprintf("%3d m %2d s", time_min, time_sec);
    }

    /* Return only seconds. */
    return sprintf("      %2d s", time_sec);
}

/*
 * Function name: time2str3
 * Description  : This function will take the number of days, hours, minutes
 *                and seconds in the time put in and return a string with
 *                the three most significant time-elements.
 * Returns      : string - the result.
 */
nomask private string
time2str3()
{
    /* Return days. */
    if (time_day)
    {
	/* Return days and hours. */
	if (time_hour)
	{
	    /* Return days, hours and minutes. */
	    if (time_min)
	    {
		return sprintf("%3d d %2d h %2d m", time_day, time_hour,
		    time_min);
	    }

	    /* Returns days, hours and seconds. */
	    return sprintf("%3d d %2d h %2d s", time_day, time_hour, time_sec);
	}

	/* Returns days, minutes and seconds. */
	return sprintf("%3d d %2d m %2d s", time_day, time_min, time_sec);
    }

    /* Return hours, minutes and seconds. */
    if (time_hour)
    {
	return sprintf("%3d h %2d m %2d s", time_hour, time_min, time_sec);
    }

    /* Return only minutes and seconds. */
    if (time_min)
    {
	return sprintf("      %2d m %2d s", time_min, time_sec);
    }

    /* Return only seconds. */
    return sprintf("           %2d s", time_sec);
}

/*
 * Function name: time2str4
 * Description  : This function takes the number of days, hours, minutes
 *                and seconds in the time that was put in and returns a
 *                string with all those four elements, their names
 *                abbreviated to only one letter.
 * Returns      : string - the result.
 */
nomask private string
time2str4()
{
    /* Just return all types. */
    return sprintf("%3d d %2d h %2d m %2d s", time_day, time_hour,
	time_min, time_sec);
}

/*
 * Function name: time2str
 * Description  : This function returns a string representing a certain
 *                time in 'sig' significant time elements. See the manual
 *                page on 'TIME2STR' for more information and examples.
 * Arguments    : int time - the time to converts.
 *                int sig  - the number of significant time-elements.
 * Returns      : string   - the resulting string.
 */
nomask public string
time2str(int time, int sig)
{
    time_to_numbers(time);

    switch(sig)
    {
    case 0:
	return "";

    case 1:
	return time2str1();

    case 2:
	return time2str2();

    case 3:
	return time2str3();

    default:
	return time2str4();
    }
}

/*
 * Function name: time2format
 * Description  : Converts a time stamp into a formatted time. The format
 *                string accepts the following formats:
 *                yyyy - year in four digits (Example: "2001")
 *                yy   - year in two digis (Example: "01", try to avoid this!)
 *                mmm  - month in string (Example: "Sep")
 *                mm   - month in number with prefix 0 (Example: "09")
 *                -m   - month in number in 2 characters (Example: " 9")
 *                m    - month in number without prefix 0 (Example: "9")
 *                ddd  - day of the week in string (Example: "Mon")
 *                dd   - day of the month in number with prefix 0 (Example: "03")
 *                -d   - day of the month in number in 2 characters (Example: " 3")
 *                d    - day of the month in number without prefix 0 (Example: "3")
 *                All other characters are literally copied to the target string.
 * Examples     : "d mmm yyyy" yields "3 Sep 2001"
 *                "yyyymmdd" yields "20010903"
 * Arguments    : int timestamp - the timestamp to convert.
 *                string format - the format to print the timestamp in.
 * Returns      : string - the formatted time.
 */
nomask public string
time2format(int timestamp, string format)
{
    string timestring = ctime(timestamp);
    string result = "";
    
    while(strlen(format))
    {
        if (format[0..3] == "yyyy")
        {
            result += timestring[20..23];
            format = format[4..];
            continue;
        }
        if (format[0..1] == "yy")
        {
            result += timestring[22..23];
            format = format[2..];
            continue;
        }
        if (format[0..2] == "mmm")
        {
            result += timestring[4..6];
            format = format[3..];
            continue;
        }
        if (format[0..1] == "mm")
        {
            result += sprintf("%02d", MonthToNumMap[timestring[4..6]]);
            format = format[2..];
            continue;
        }
        if (format[0..1] == "-m")
        {
            result += sprintf("%2d", MonthToNumMap[timestring[4..6]]);
            format = format[2..];
            continue;
        }
        if (format[0..0] == "m")
        {
            result += MonthToNumMap[timestring[4..6]];
            format = format[1..];
            continue;
        }
        if (format[0..2] == "ddd")
        {
            result += timestring[0..2];
            format = format[3..];
            continue;
        }
        if (format[0..1] == "dd")
        {
            result += sprintf("%02d", atoi(timestring[8..9]));
            format = format[2..];
            continue;
        }
        if (format[0..1] == "-d")
        {
            result += sprintf("%2d", atoi(timestring[8..9]));
            format = format[2..];
            continue;
        }
        if (format[0..0] == "d")
        {
            result += atoi(timestring[8..9]);
            format = format[1..];
            continue;
        }
        result += format[0..0];
        format = format[1..];
    }
   
    return result;
}

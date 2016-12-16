/* === getopt.c ==========================================================
 * =========================================== Araven 2002 (c) Standard ===
 *
 * This library holds the implementation of posix style getopt.  The func-
 * tion designed to parse command line options, offering a standard way of
 * dealing with them.    Idea of creating this function belongs to Salisan
 * (Demons Gate) the implementation is totally different however.
 *
 * === example ===========================================================
 * =============================================================== *** ===
 *
 * inherit "/cmd/std/command_driver";
 * inherit "/d/Standard/lib/getopt";
 *
 * public mapping
 * query_cmdlist()
 * {
 *     return ([ "ls" : "ls" ]);
 * }
 *
 * public int
 * ls(string phrase)
 * {
 *     mixed   record;
 *     int     format_option, sorter_option;
 *
 *     record = getopt(phrase, "lt");
 *
 *     while (pointerp(record[0]))
 *     {
 *         switch (record[0][0])
 *         {
 *             case 'l':
 *                 format_option = 'l';
 *                 break;
 *
 *             case 't':
 *                 sorter_option = 't';
 *                 break;
 *
 *             case '?'
 *                 write("Unknown option: -" + record[0][1] + "\n");
 *                 break;
 *         }
 *
 *         record = record[1..];
 *     }
 *
 *     ......
 * }
 *
 * =======================================================================
 * =============================================================== *** ===
 */

/*
 * Function name : getopt
 * Description   : function is designed to parse command line options, it
 *                 similar in its functionality to posix getopt and would
 *                 return array looking like that: ({ ({ 'a', "a_arg" }),
 *                 ({ 'b', "b_arg" }), ({ '?', "unknown_option_char"  }),
 *                 command_line_remnant })
 * Arguments     : string  - the command line written by the player, if
 *                           the line contains "--" then string following
 *                           it would be considered to be unbelonging to
 *                           options even if it contains things like "-s"
 *                 string  - option string describing a possible options,
 *                           a char followed by ':' suggests that this
 *                           option takes an argument
 * Returns       : mixed   - array as described above
 */
static nomask mixed
getopt(string phrase, string oplist)
{
    mixed   result;
    int     opsize;
    string *opchar, *opargs, *oprest;

    if (sizeof(result = explode(phrase || "", "--")) >= 2)
    {
        opargs = explode(implode(result[..0], "--"), " ") - ({ "" });
        oprest = explode(implode(result[1..], "--"), " ") - ({ "" });
    }
    else
    {
        opargs = explode(phrase || "", " ") - ({ "" });
        oprest = ({ });
    }

    opchar = filter(opargs, &wildmatch("-*", ));
    opargs = opargs - opchar;
    opchar = explode(implode(map(opchar, &extract(, 1)), ""), "");

    result = ({ });
    opsize = sizeof(opchar);

    while (--opsize >= 0)
    {
        if (wildmatch("*" + opchar[0] + "*", oplist))
        {
            if (wildmatch("*" + opchar[0] + ":*", oplist) && sizeof(opargs))
            {
                result += ({ ({ opchar[0][0], opargs[0] }) });
                opargs  = opargs[1..];
            }
            else
            {
                result += ({ ({ opchar[0][0], "" }) });
            }
        }
        else
        {
            result += ({ ({ '?', opchar[0] }) });
        }

        opchar = opchar[1..];
    }

    return result + ({ implode(opargs + oprest, " ") });
}

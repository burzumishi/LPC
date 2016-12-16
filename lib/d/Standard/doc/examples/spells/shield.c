/* This spells summons a magic shield to protect a player. It works as
 * a forcefield protecting from blows. The duration of the shield depends
 * on the intelligence of the caster. Defence spell skill and wisdom
 * controls if this is a success. The magic shiled has an ac of 6.
 */

#include <ss_types.h>
#include <stdproperties.h>
#include <comb_mag.h>
#include <macros.h>

#define CAST_LIMIT    30   /* The limit for this to become a success */
#define SPELL_DIR	"/d/Standard/doc/examples/spells/"

/* Function name: shield
 * Description:   Will summon a magic shield, protecting someone
 * Arguments:     str - the rest of the shield command
 * Returns:       1/0
 */
nomask mixed
shield(string str)
{
    object who, shield;
    int mana_limit, sum;

    if (str)
        who = present(lower_case(str), environment(this_player()));
    if (!who)
    {
        if (!str || (str == "me"))
            who = this_player();
        else
            return "Who do you want to cast this spell upon.\n";
    }

    if (NPMAGIC(who)) /* See if it is possible to cast a magic spell on obj. */
        return "Something prevents you from cast this spell.\n";

    if (present("mag_shield_prot", who))
        return "Only one protective shield spell at a time on a person.\n";

    mana_limit = 30;
    if (who == this_player()) /* Costs more to shield another person. */
        mana_limit = 20;

    if (this_player()->query_mana() < mana_limit)
        return "You can't concetrate enough to cast this spell.\n";

    if (random(this_player()->query_skill(SS_SPELLCRAFT) +
        this_player()->query_stat(SS_WIS)) < random(CAST_LIMIT))
    {
        write("You failed to pronounce the words right.\n");
        this_player()->add_mana(-mana_limit / 3);
        return 1;
    }

    this_player()->add_mana(-mana_limit);
    write("You start to pronounce the ancient words.\n");
    say(QCTNAME(this_player()) + " mumbles something.\n");

    seteuid(getuid(this_object()));
    shield = clone_object(SPELL_DIR + "obj/mag_shield");
    shield->set_duration(random(this_player()->query_stat(SS_INT) / 3) +
        random(this_player()->query_skill(SS_FORM_ABJURATION) / 2) + 180);
    shield->move(who);
    return 1;
}

/* Function name:
 * Description:
 * Arguments:
 * Returns:
 */


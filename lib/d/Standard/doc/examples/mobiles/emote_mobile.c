/*
 * /doc/examples/mobiles/emote_mobile.c
 *
 * In the beginning of time, the honourable Tricky created an alchemist. Nick
 * called this 'an excellent example on how to use triggers' and he stripped
 * the triggering part of it for this example, or rather, Nick stole it as he
 * said ;-) Now we don't use triggers anymore for emotions, but a large hook
 * to 'trigger' on the emotions.
 *
 * Originally coded by Tricky as alchemist for Hobbiton, Genesis.
 * Adapted as emote-trigger-example by Nick.
 * Converted to emote-hook-example by Mercade, March 28 1996.
 */

inherit "/std/monster";

#include <macros.h>
#include <stdproperties.h>

/* These are the emotions that we react on. */
#define HOOKED_EMOTIONS ({ "sigh", "smile", "smirk", "frown", "giggle", \
    "nod", "shake", "laugh", "growl", "cackle", "shrug", "bow", "wave", \
    "grin", "spit" })

/* Since we need this check in each emote-reaction, we put it in a simple
 * define for easy coding. Note that we use the variable 'actor' in this
 * define, so we _must_ call the variable by that name in all functions we
 * use this define in.
 */
#define CHECK_ACTOR if (!present(actor, environment())) { return; }

/*
 * Function name: create_monster
 * Description  : Constructor. This function is called to create this monster.
 */
void
create_monster()
{
    if (!IS_CLONE)
	return;

    set_name("alchemist");
    set_race_name("alchemist"); 
    set_living_name("_alchemist_");
    set_adj("town");
    set_long(break_string("You are looking at the local alchemist, who is " +
	"known to possess the power to transform certain objects into " +
	"other objects. Simply give him an object, so he can judge it.", 75) +
        "\n");

    add_prop(CONT_I_WEIGHT, 47000);   /* 47 Kg */
    add_prop(CONT_I_HEIGHT, 87);      /* 87 cm */
    add_prop(LIVE_I_NEVERKNOWN, 1);

    /* str dex con int wis dis */
    set_stats( ({ 22, 27, 21, 70, 70, 34 }) );
}

/* 
 * Function name: add_introduced
 * Description  : This function is called whenever someone introduces him or
 *                herself to this alchemist.
 * Arguments    : string who - the name of the person introducing.
 */
void
add_introduced(string who)
{
    object obj;

    obj = present(who, environment());
    switch(random(3))
    {
    case 0:
	command("say Nice to meet you, " + obj->query_name() + ".");
	break;

    case 1:
	command("say Be welcome, " + obj->query_race() + ".");
	break;

	/* Indeed, if random == 2, then we will only bow. */
    }

    command("bow to " + who);
}

/********************************************************************
 *
 *  Some trigger feelings to make the alchemist more vivid.
 *  The feelings are split in two parts: the part that is called by
 *  a trigger function, and a part that is called after a random time.
 *  This is done to get a feeling of reality in the game.
 */

void
sigh(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say Why are you depressed, " + actor->query_nonmet_name() +
		"?");
	return;

    case 1:
	command("comfort " + OB_NAME(actor));
	return;

    default:
	command("say Is life tough for you, " + actor->query_nonmet_name() +
		"?");
    }
}

void
smile(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say Life is great, isn't it, " + actor->query_nonmet_name() +
		"?");
	return;

    case 1:
	command("smile at " + OB_NAME(actor));
	return;

    default:
	/* Here we see the beauty of the adverb-format. If no adverb is used,
	 * the variable will be only "", making the comma follow the word
	 * 'smiling' directly. However, if there is an adverb, is will be
	 * properly separated from by the space that is preceding it.
	 */
	command("say It is great to see you smiling" + adverb + ", " +
		actor->query_nonmet_name() + ".");
    }
}

void
smirk(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say I sense irony, " + actor->query_nonmet_name() + "...");
	return;

    case 1:
	command("smirk");
	return;

    default:
	command("grin at " + OB_NAME(actor));
    }
}

void
grin(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say What cunning plan have you forged, " +
		((actor->query_gender() == G_MALE) ? "master" : "lady") +
		"?");
	return;

    case 1:
	command("grin");
	return;
	
    default:
	command("say Get that grin off your face, " +
		actor->query_race() + ".");
    }
}

void
nod(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say I'm glad you agree, " +
		((actor->query_gender() == G_MALE) ? "master" : "lady") +
		".");
	return;

    case 1:
	command("pat " + OB_NAME(actor));
	return;

    default:
	command("say Indeed, " + actor->query_race() + ".");
    }
}

void
shake(object actor, string adverb)
{
    CHECK_ACTOR;

    /* I must admit that at this time there is no way to see whether the
     * actor is shaking his head in general or shaking the hand of this
     * NPC. For now I shall assume the shake is in general. If you have a
     * good way [that doesn't consume too much CPU] to find out, please
     * let me know. /Mercade.
     */
    switch(random(3))
    {
    case 0:
	command("say So you disagree, " + actor->query_race() + "?");
	return;

    case 1:
	command("say I agree with you, " +
		((actor->query_gender() == G_MALE) ? "master" : "lady") + ".");
	return;

    default:
	command("say Why do " + LANG_PWORD(actor->query_race()) +
		" always disagree?");
    }
}

void
laugh(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say Very funny indeed...");
	return;

    case 1:
	command("laugh");
	return;

    default:
	command("giggle");
    }
}

void
growl(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say You frighten me with your growling, " +
		actor->query_race() + "...");
	return;

    case 1:
	command("say Why so hostile, " + actor->query_race() + "?");
	return;

    default:
	command("frown");
    }
}

void
cackle(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say You sound like a duck, " + actor->query_race() + ".");
	return;

    case 1:
	command("say " + capitalize(LANG_PWORD(actor->query_race())) +
		" cackle very often.");
	return;

    default:
	command("giggle");
    }
}

void
shrug(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say Is there anything you " +
		LANG_PWORD(actor->query_race()) + " do know?");
	return;

    case 1:
	command("say Don't look at me... I don't know either!");
	return;

    default:
	command("shrug");
    }
}

void
bow(object actor, string adverb)
{
    CHECK_ACTOR;

    command("bow to " + OB_NAME(actor));
}

void
wave(object actor, string adverb)
{
    CHECK_ACTOR;

    command("wave to " + OB_NAME(actor));
}

void
frown(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say Is something wrong, " + actor->query_race());
	return;

    case 1:
	command("say It wasn't my fault!");
	return;

    default:
	command("say I had nothing to do with it, I assure you!");
    }
}

void
spit(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say Damn " + LANG_PWORD(actor->query_race()) + "!");
	return;

    case 1:
	command("say Hey! Don't do that! Don't spit in here!");
	return;

    default:
	command("say " + capitalize(LANG_PWORD(actor->query_race())) +
		" are such rude people!");
    }
}

void
giggle(object actor, string adverb)
{
    CHECK_ACTOR;

    switch(random(3))
    {
    case 0:
	command("say Funny, eh " + actor->query_race() + "?");
	return;

    case 1:
	command("say Ah, " + LANG_PWORD(actor->query_race()) +
		" are such merry people.");
	return;

    default:
	command("giggle");
    }
}

/*
 * Function name: emote_hook
 * Description  : This hook is called whenever an emotion is performed on this
 *                NPC. If it is an emotion that we hook on, call a little
 *                alarm and then call the return function.
 * Arguments    : string emote  - the emotion performed.
 *                object actor  - the actor of the emotion.
 *                string adverb - the adverb used in the emotion.
 */
void
emote_hook(string emote, object actor, string adverb)
{
    /* Only continue for emotions that we hook on. Since Tricky added hooks
     * for 15 emotions, we do it this way. If only one or two emotions were
     * triggered upon, we can do the answer directly.
     */
    if (member_array(emote, HOOKED_EMOTIONS) == -1)
    {
	return;
    }

    /* Now, we wait for three seconds before we react. Using the efun
     * set_alarmv(), we can give an array of arguments that will be called
     * as separate arguments to the function when it is called. [See the
     * functions above].
     */
    set_alarmv(3.0, 0.0, emote, ({ actor, adverb }) );
}

/*
 * Function name: emote_hook
 * Description  : This hook is called whenever an emotion is performed on a
 *                third party in the room. Normally we don't bother to react
 *                on emotions on others, unless someone is spitting. As the
 *                alchemist doesn't like spitting at all, he reacts to it.
 * Arguments    : string emote    - the emotion performed.
 *                object actor    - the actor of the emotion.
 *                string adverb   - the adverb used in the emotion.
 *                object *targets - the targets of the emotion.
 */
void
emote_hook_onlooker(string emote, object actor, string adverb, object *targets)
{
    /* Only react to spitting. */
    if (emote == "spit")
    {
	/* As we know the function we are going to call, we can use it in
	 * the alarm directly in the form of a functionpointer. Also, since
	 * spitting doesn't require an adverb, we can skip that.
	 */
	set_alarm(3.0, 0.0, &spit(actor));
    }
}

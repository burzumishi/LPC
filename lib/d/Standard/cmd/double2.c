
/* 
 * /d/Standard/cmd/double.c 
 * 
 * Double-emotes soul, allows people to combine two emotes into one sentence
 * 
 * Coded by Maniac@Standard, May 1997
 * 
 * Copyright (C) Miguel Leith 1997
 *
 * History: 
 * Updates, Maniac, 1998 
 * Added options for external double compatible emotes, Maniac, 22/7/01 
 * 
 */ 

#undef DEBUG_VREC
#ifdef EMOTE_VAR 
#undef EMOTE_VAR
#endif
#define EMOTE_VAR "XX" 

inherit "/cmd/std/command_driver";

/* prototypes */ 
int resolve_parallelism(mixed sen1, mixed sen2);
string resolve_verb(object onl, object seen, string *vf, int ing); 
string resolve_sbj(object onl, object seen, int cap, int pf); 
string resolve_act_obj(object onl, object seen, int cap, int pf); 
string resolve_trg_obj(object onl, object seen, int cap, int pf); 
string resolve_pos(object onl, object seen, int cap, int pf); 
mixed resolve_sen_el(object onl, object act, object trg, mixed el, 
                     string adv, int cap, int ing, int apf, int tpf); 
void wde_for_itr(object onl, mixed e1det, mixed e2det, string conj); 
void wde_for_npc(object onl, mixed e1det, mixed e2det); 
void evh_notify_fail(string vb, int trans, int ad, int trail); 
varargs mixed get_emote_details(string vb, string arg, object trg); 
int write_double_emote(string vb1, string arg1, 
                       string vb2, string arg2, 
                       string conj); 
int double(string str); 


#include <stdproperties.h>
#include <macros.h>
#include <ss_types.h>
#include <wa_types.h>
#include <formulas.h>
#include <cmdparse.h>
#include <language.h>
#include <composite.h>
#include <std.h>
#include <filter_funs.h>
#include <adverbs.h>
#include "emotes.h"
#include "emotes.c"

int
query_cmd_soul()
{
    return 1;
}


string
get_soul_id()
{
    return "Double";
}


mapping
query_cmdlist()
{
    return
    ([
        "2" : "double", 
        "double" : "double", 
    ]);
}


/*
 * Function name: resolve_parallelism
 * Description  : resolves whether a pair of sentences has subject parallelism
 *                e.g. "fred jumps around" and "fred ponders the future"
 * Arguments    : mixed sen1, sen1 - semantic form of the two sentences  
 * Returns      : int              - whether subject parallell or not 
 */
int
resolve_parallelism(mixed sen1, mixed sen2)
{
    return ((sen1[0] == EVH_ACT_SBJ) && (sen2[0] == EVH_ACT_SBJ));  
}


/*
 * Function name: resolve_verb
 * Description  : resolves the form of a verb to write  
 * Arguments    : object onl  - living onlooker 
 *                object seen - living seen doing the verb  
 *                string *vf  - verb forms e.g. ({"jump", "jumps", "jumping"}) 
 *                int ing     - whether ing-form is required. 
 * Returns      : string      - the verb form to write 
 */
string 
resolve_verb(object onl, object seen, string *vf, int ing) 
{
    if (ing) 
        return vf[EVH_VF_ING]; 
    else if (onl == seen) 
        return vf[EVH_VF_FPS]; 
    else
        return vf[EVH_VF_TPS]; 
}


/*
 * Function name: resolve_sbj
 * Description  : resolves the sentence subject to write   
 * Arguments    : object onl  - living onlooker 
 *                object seen - living seen doing the verb  
 *                int cap     - whether capitalization is required
 *                int pf      - whether pro-form is required e.g. "he"
 * Returns      : string      - the sentence subject to write 
 */
string
resolve_sbj(object onl, object seen, int cap, int pf)
{
    string r;

    if (onl == seen)  
        r = "you"; 
    else if (pf) 
        r = seen->query_pronoun(); 
    else 
        r = seen->query_the_name(onl); 

    if (cap)
        return capitalize(r); 
    else 
        return r; 
}

/*
 * Function name: resolve_act_obj
 * Description  : resolves the sentence object to write   
 * Arguments    : object onl  - living onlooker 
 *                object seen - living seen doing the verb  
 *                int cap     - whether capitalization is required
 *                int pf      - whether pro-form is required e.g. "himself"
 * Returns      : string      - the sentence object to write 
 */
string
resolve_act_obj(object onl, object seen, int cap, int pf)
{
    string r;

    if (onl == seen)  
        r = "yourself"; 
    else if (pf) 
        r = (seen->query_objective() + "self"); 
    else 
        r = seen->query_the_name(onl); 

    if (cap)
        return capitalize(r); 
    else 
        return r; 
}


/*
 * Function name: resolve_trg_obj
 * Description  : resolves the sentence object to write   
 * Arguments    : object onl  - living onlooker 
 *                object seen - living seen doing the verb  
 *                int cap     - whether capitalization is required
 *                int pf      - whether pro-form is required e.g. "him"
 * Returns      : string      - the sentence object to write 
 */
string
resolve_trg_obj(object onl, object seen, int cap, int pf)
{
    string r;

    if (onl == seen)  
        r = "you"; 
    else if (pf) 
        r = seen->query_objective(); 
    else 
        r = seen->query_the_name(onl); 

    if (cap)
        return capitalize(r); 
    else 
        return r; 
}


/*
 * Function name: resolve_pos 
 * Description  : resolves the sentence possessive to write   
 * Arguments    : object onl  - living onlooker 
 *                object seen - living seen doing the verb  
 *                int cap     - whether capitalization is required
 *                int pf      - whether pro-form is required e.g. "his"
 * Returns      : string      - the sentence possessive to write 
 */
string
resolve_pos(object onl, object seen, int cap, int pf)
{
    string r;

    if (onl == seen)  
        r = "your"; 
    else if (pf) 
        r = seen->query_possessive(); 
    else 
        r = (seen->query_the_name(onl) + "'s"); 

    if (cap)
        return capitalize(r); 
    else 
        return r; 
}


/*
 * Function name: resolve_sen_el 
 * Description  : resolves the sentence element to write   
 * Arguments    : object onl  - living onlooker 
 *                object act  - actor of the emote 
 *                object trg  - target of the emote
 *                mixed  el   - the semantic form of the sentence element 
 *                string adv  - the adverb if any 
 *                int cap     - whether to capitalize 
 *                int ing     - whether ing-form required
 *                int apf     - whether actor pro form required
 *                int tpf     - whether target pro form required
 * Returns      : mixed       - an array ({ str, ar, tr }) where ar 
 *                              is 1 if the actor was referred to, tr 
 *                              is 1 if the target was referred to, str  
 *                              is the string to write. 
 */
mixed
resolve_sen_el(object onl, object act, object trg, mixed el, 
               string adv, int cap, int ing, int apf, int tpf)
{
    string ps; 

    if (cap)  
        ps = ""; 
    else 
        ps = " "; 

    if (pointerp(el))  /* a verb */ 
        return ({ ps + resolve_verb(onl, act, el, ing), 0, 0 });
    else if (intp(el)) {
        switch (el) {  
             case EVH_ACT_SBJ :  
                return ({ ps + resolve_sbj(onl, act, cap, apf), 
                          1, 0 });   
                break;
             case EVH_ACT_OBJ : 
                return ({ ps + resolve_act_obj(onl, act, cap, apf), 
                          1, 0 });  
                break;
             case EVH_ACT_POS : 
                return ({ ps + resolve_pos(onl, act, cap, apf), 
                          1, 0 }); 
                break;
             case EVH_TRG_SBJ : 
                return ({ ps + resolve_sbj(onl, trg, cap, tpf), 
                          0, 1 });  
                break;
             case EVH_TRG_OBJ :  
                return ({ ps + resolve_trg_obj(onl, trg, cap, tpf), 
                         0, 1 });  
                break;
             case EVH_TRG_POS : 
                return ({ ps + resolve_pos(onl, trg, cap, tpf), 
                         0, 1 }); 
                break;
             case EVH_ADVERB : /* no pre-string for adverbs */ 
                return ({ adv, 0, 0 }); 
                break;
         } 
    } 
    else if (strlen(el)) { 
        if (cap) 
            el = capitalize(el); 
        return ({ ps + el, 0, 0 }); 
    } 
    else  
        /* Dunno how we ended up here, but oh well, it'll give a bug clue */ 
        return ({ el, 0, 0 }); 
}


/*
 * Function name: wde_for_npc   (write double emote for npc)
 * Description  : Informs an npc of a double emote if it's perceived. 
 * Arguments    : object onl  - living onlooker, the npc
 *                mixed e1det - semantic details of the first emote
 *                mixed e2det - semantic details of the second emote
 * Returns      : void
 */
void
wde_for_npc(object onl, mixed e1det, mixed e2det)
{
    int p1, p2; 

    /* Find out whether this living perceived the emote */ 
    p1 = (e1det[EVH_VDI_TGBL] || (CAN_SEE(onl, e1det[EVH_VDI_ACT]) && 
                              CAN_SEE_IN_ROOM(onl)));    
    p2 = (e2det[EVH_VDI_TGBL] || (CAN_SEE(onl, e2det[EVH_VDI_ACT]) && 
                              CAN_SEE_IN_ROOM(onl)));    

    /* First emote: if it's perceived ahd the npc was the target 
                    or there was no target, inform the npc */ 
    if (p1 && (!e1det[EVH_VDI_TRG] || (onl == e1det[EVH_VDI_TRG])))
        onl->emote_hook(e1det[EVH_VDI_VERB], e1det[EVH_VDI_ACT], 
                        e1det[EVH_VDI_ADV]);  

    /* Second emote: if it's perceived ahd the npc was the target 
                     or there was no target, inform the npc */ 
    if (p2 && (!e2det[EVH_VDI_TRG] || (onl == e2det[EVH_VDI_TRG])))
        onl->emote_hook(e2det[EVH_VDI_VERB], e2det[EVH_VDI_ACT], 
                        e2det[EVH_VDI_ADV]);  
}


/*
 * Function name: wde_for_itr   (write double emote for interactive)
 * Description  : Writes a double emote to an interactive if it's perceived. 
 * Arguments    : object onl  - living onlooker, the interactive 
 *                mixed e1det - semantic details of the first emote
 *                mixed e2det - semantic details of the second emote
 *                string conj - the conjunction (and/while/whatever) 
 * Returns      : void
 */
void
wde_for_itr(object onl, mixed e1det, mixed e2det, string conj)
{
    string msg; 
    int i, p1, p2, rp, co, ing, apf, tpf, s; 
    object trg1, trg2, act1, act2; 
    string ad1, ad2; 
    mixed sen1, sen2, elr; 

    /* unpacking the arrays makes the code below more readable */ 
    act1 = e1det[EVH_VDI_ACT]; 
    act2 = e2det[EVH_VDI_ACT]; 
    trg1 = e1det[EVH_VDI_TRG]; 
    trg2 = e2det[EVH_VDI_TRG]; 
    sen1 = e1det[EVH_VDI_SEN]; 
    sen2 = e2det[EVH_VDI_SEN]; 
    ad1 = e1det[EVH_VDI_ADV];
    ad2 = e2det[EVH_VDI_ADV];

    /* Find out whether the interactive perceived each emote */ 
    p1 = (e1det[EVH_VDI_TGBL] || (CAN_SEE(onl, act1) && 
                              CAN_SEE_IN_ROOM(onl)));    
    p2 = (e2det[EVH_VDI_TGBL] || (CAN_SEE(onl, act2) && 
                              CAN_SEE_IN_ROOM(onl)));    

    /* If the interactive perceived neither, just exit */ 
    if (!p1 && !p2) 
        return;
    /* Else if they perceived both, co-ordination comes into play */ 
    else if (p1 && p2) { 
        co = 1; 
        rp = resolve_parallelism(sen1, sen2); 
        if ((conj == "while") && rp) 
            ing = 1; 
    } 

    /* pronoun flags */ 
    apf = 0; 
    tpf = 0; 

    msg = ""; 

    s = sizeof(sen1); 

    /* 
     * If the first emote was perceived, construct the message
     * by resolving all the sentence elements, keeping a running 
     * count of actor and target references. 
     */ 
    if (p1) { 
        i = 0; 
        while (i < s) {   
            elr = resolve_sen_el(onl, act1, trg1, sen1[i], ad1, !i, 
                                 0, apf, tpf);
            msg += elr[0]; 
            apf += elr[1]; 
            tpf += elr[2]; 
            i++; 
        } 
    } 

    /* Co-ordinating conjunction */ 
    if (co) 
        msg += (" " + conj); 

    if (trg1 != trg2) 
       tpf = 0; 

    s = sizeof(sen2); 


    /* 
     * If the second emote was perceived, construct the message
     * by resolving all the sentence elements, keeping a running 
     * count of actor and target references. 
     */ 
    if (p2) {  
        i = rp; 
        while (i < s) {   
            elr = resolve_sen_el(onl, act2, trg2, sen2[i], ad2, (!i && !co),
                                 ing, apf, tpf); 
            msg += elr[0]; 
            apf += elr[1]; 
            tpf += elr[2]; 
            i++;
        } 
        msg += e2det[EVH_VDI_ENDING]; 
    } 
    else 
        msg += e1det[EVH_VDI_ENDING]; 

    msg += "\n"; 

    tell_object(onl, msg); 
}


/*
 * Function name: evh_notify_fail
 * Description  : Sets up an appropriate notify_fail for a double emote verb
 * Arguments    : string vb   - the emote verb 
 *                int trans   - whether the verb was transitive or not
 *                int ad      - whether the verb has an adverb slot or not 
 *                int trail   - whether the verb's adverb is trailing or not
 * Returns      : void
 */
void
evh_notify_fail(string vb, int trans, int ad, int trail)
{
    string a;

    if (ad) { 
        if (trans) { 
            if (trail) 
                a = " whom [how]"; 
            else 
                a = " [how] whom"; 
        } 
        else 
            a = " how"; 
    } 
    else if (trans)  
        a = " whom"; 
    else
        a = ""; 

    notify_fail(capitalize(vb) + a + "?\n");  
}


/*
 * Function name: get_emote_details
 * Description  : Parse an emote from a double emote 
 * Arguments    : string vb   - the emote verb 
 *                string arg  - the argument of the verb 
 *                object trg  - target of 1st double emote if this is the 2nd
 * Returns      : mixed       - the parsed semantic details of the emote 
 */
varargs mixed
get_emote_details(string vb, string arg, object trg)
{
    string *how, iad, tad, ad; 
    int trans, intrans; 
    mixed vrec; 
    object *oblist; 

    /* Find a verb record for the emote in the verb mappings */  
    if (vb < EVH_DELIM_1_2)
         vrec = verb_rec1[vb];
    else if (vb < EVH_DELIM_2_3)
         vrec = verb_rec2[vb];
    else if (vb < EVH_DELIM_3_4)
         vrec = verb_rec3[vb];
    else
         vrec = verb_rec4[vb];

    if (!vrec) { 
        if (this_player()->is_double_emote_verb(vb)) 
            return this_player()->get_emote_details(vb, arg, trg); 

        notify_fail("Didn't recognise the verb: " + vb + ".\n"); 
        return 0;
    } 
    else if (stringp(vrec)) 
        return call_other(this_object(), vrec, arg, trg); 


    iad = vrec[EVH_INTRANS_ADV]; 
    tad = vrec[EVH_TRANS_ADV]; 
    trans = pointerp(vrec[EVH_TRANS]); 
    intrans = pointerp(vrec[EVH_INTRANS]); 

    if (stringp(iad) || stringp(tad)) {  
        how = parse_adverb_with_space(arg, NO_DEFAULT_ADVERB, 
                                      vrec[EVH_TRAILADV]);  
        arg = how[0]; 
        if (how[1] == NO_DEFAULT_ADVERB_WITH_SPACE) { 
            if (!strlen(arg) && strlen(iad)) 
                ad = ADD_SPACE_TO_ADVERB(iad); 
            else if (strlen(arg) && strlen(tad)) 
                ad = ADD_SPACE_TO_ADVERB(tad); 
            else 
                ad = ""; 
        } 
        else 
            ad = how[1]; 
    } 
    else { 
        if (stringp(arg)) {  
            if (!trans) { 
                evh_notify_fail(vb, 0, 0, vrec[EVH_TRAILADV]); 
                return 0;
            } 
        } 
        else { 
            if (trans && !intrans) { 
                evh_notify_fail(vb, 1, 0, vrec[EVH_TRAILADV]); 
                return 0;
            } 
        } 
    }    

    if (stringp(arg)) { 
        if (!trans) { 
            evh_notify_fail(vb, 0, stringp(ad), vrec[EVH_TRAILADV]);  
            return 0;
        } 
        /* resolve objective pronoun */ 
        if (objectp(trg) && ((trg->query_objective() == arg) || 
                              (arg == "it")))  
            oblist = ({ trg }); 
        else { 
            oblist = parse_this(arg, "[the] %l");  
            if (!sizeof(oblist)) { 
                evh_notify_fail(vb, 1, stringp(ad), vrec[EVH_TRAILADV]);  
                return 0;
            } 
            if (sizeof(oblist) > 1) { 
                notify_fail("You can't <" + vb + "> more than one target " +
                            "in double emotes!\n"); 
                return 0;
            } 
        } 
    } 
    else { 
        if (trans && !intrans) { 
            evh_notify_fail(vb, 1, stringp(ad), vrec[EVH_TRAILADV]);  
            return 0;
        } 
    } 

    return ({ this_player(), (sizeof(oblist) ? oblist[0] : 0), 
              ad, vrec[EVH_TANGIBLE], 
              (sizeof(oblist) ? vrec[EVH_TRANS] : vrec[EVH_INTRANS]), 
              vb, vrec[EVH_ENDING] }); 
}



/*
 * Function name: write_double_emote
 * Description  : main marshalling function to write a double emote
 * Arguments    : string vb1  - the first emote verb 
 *                string arg2 - the first emote argument 
 *                string vb2  - the second emote verb 
 *                string arg2 - the second emote argument 
 *                string conj - the conjunction (and/while/whatever) 
 * Returns      : int         - whether double emote be parsed and executed
 */
int
write_double_emote(string vb1, string arg1, 
                   string vb2, string arg2, 
                   string conj)
{
    int i;
    mixed e1det, e2det; 
    object *liv, *itr; 

    if (!(e1det = get_emote_details(vb1, arg1)))   
        return 0;
#ifdef DEBUG_VREC
    dump_array(e1det); 
#endif
    if (!(e2det = get_emote_details(vb2, arg2, e1det[EVH_VDI_TRG])))  
        return 0;
#ifdef DEBUG_VREC
    dump_array(e2det); 
#endif

    liv = FILTER_LIVE(all_inventory(environment(e1det[EVH_VDI_ACT])));  

    itr = filter(liv, interactive); 
    liv = (liv - itr);  

    /* Send details to the interactives first as npcs may react instantly */ 
    map(itr, &wde_for_itr(, e1det, e2det, conj)); 

    map(liv, &wde_for_npc(, e1det, e2det));  

    return 1;
}


/* 
 * Function name: double
 * Description  : action function for the "double" command 
 * Arguments    : string str  - the double emote argument 
 * Returns      : int         - whether the action was successful or not
 */
int
double(string str)
{
    string conj, *s, *el, vb1, arg1, vb2, arg2, al, rep; 
    int n; 

    notify_fail("Syntax: double <emote1> <and/while> <emote2>\n"); 
    if (!stringp(str) || !strlen(str)) 
        return 0; 

    if (str == "list") { 
        this_player()->more("STANDARD DOUBLE EMOTE VERBS\n\n" + 
            sprintf("%-80#s\n", 
                implode(sort_array(m_indexes(verb_rec1)) + 
                    sort_array(m_indexes(verb_rec2)) + 
                    sort_array(m_indexes(verb_rec3)) + 
                    sort_array(m_indexes(verb_rec4)), 
                    "\n")) + 
                (sizeof(s = this_player()->query_double_emote_verbs()) ? 
                 ("\nADDITIONAL DOUBLE EMOTE VERBS\n\n" + 
                  sprintf("%-80#s\n", implode(sort_array(s), "\n"))) : 
                 "")); 
        return 1; 
    } 

    /* first handle any double emote aliases */ 
    if (sscanf(str, "(%s) %s", al, rep) == 2) { 
        al += " "; 
        s = explode(al, EMOTE_VAR);  
        str = implode(s, rep);  
    } 

    s = explode(str, " and "); 
    if (sizeof(s) == 2) 
        conj = "and"; 
    else { 
        s = explode(str, " while "); 
        if (sizeof(s) == 2) 
            conj = "while"; 
        else 
            return 0;
    } 

    el = explode(s[0], " "); 
    vb1 = el[0]; 

    if (sizeof(el) > 1) 
        arg1 = implode(el[1..], " "); 
    else 
        arg1 = 0; 

    el = explode(s[1], " "); 
    vb2 = el[0]; 

    if (sizeof(el) > 1) 
        arg2 = implode(el[1..], " "); 
    else 
        arg2 = 0; 

    if (vb1 == vb2) { 
        notify_fail("Not much point in using the same verb twice!\n");  
        return 0;
    } 

    return write_double_emote(vb1, arg1, vb2, arg2, conj); 
}


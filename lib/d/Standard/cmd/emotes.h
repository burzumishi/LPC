
/* 
   EVH (emote verb handler) defs 
   Defined by Maniac@Standard

   Copyright (C) Miguel Leith 1997
 */ 

#ifndef EVH_DEFS
#define EVH_DEFS

/* identifiers for sentence elements */
#define EVH_ACT_SBJ 11 /* actor subject */ 
#define EVH_ACT_OBJ 12  /* actor object */ 
#define EVH_ACT_POS 13  /* actor possessive */ 

#define EVH_TRG_SBJ 21  /* target subject */ 
#define EVH_TRG_OBJ 22  /* target object */ 
#define EVH_TRG_POS 23  /* target possessive */ 

#define EVH_ADVERB 31 /* adverb goes here */ 

/* verb forms */
#define EVH_VF_FPS 0  /* first person singular */ 
#define EVH_VF_TPS 1  /* third person singular */ 
#define EVH_VF_ING 2  /* ing form */ 

/* common standard verb e.g. "jump", "jumps", "jumping" */ 
#define EVH_VERB(vb) ({ vb, vb+"s", vb+"ing" })

/* standard with e ending, e.g. "tickle", "tickles", "tickling" */ 
#define EVH_VERBE(vb) ({ vb, vb+"s", extract(vb, 0, -2)+"ing" })

/* standard with es inflection, e.g. "caress", "caresses", "caressing" */ 
#define EVH_VERBES(vb) ({ vb, vb+"es", vb+"ing" })

/* standard with duplicate letter in ing form, e.g. "beg", "begs", "begging" */ 
#define EVH_VERBD(vb) ({ vb,vb+"s",vb+extract(vb, -1)+"ing"})

/* standard with y ending, e.g. "cry", "cries", "crying" */ 
#define EVH_VERBY(vb) ({ vb, extract(vb, 0, -2)+"ies", vb + "ing" })

/* standard with c ending, e.g. "panic", "panics", "panicking" */ 
#define EVH_VERBC(vb) ({ vb, vb+"s", vb + "king" })

/* The indices for emote specifications */ 
#define EVH_INTRANS 0     /* intransitive sentence */ 
#define EVH_INTRANS_ADV 1 /* intransitive default adverb */ 
#define EVH_TRANS 2       /* transitive sentence */ 
#define EVH_TRANS_ADV 3   /* transitive default adverb */ 
#define EVH_TANGIBLE 4    /* 1 if tangible, perceived without visual */ 
#define EVH_TRAILADV 5    /* 1 if trail adverb */ 
#define EVH_ENDING 6      /* sentence ending i.e. ! or . */ 

#define EVH_DELIM_1_2 "fart"
#define EVH_DELIM_2_3 "melt"
#define EVH_DELIM_3_4 "snap"

/*
 * Verb detail indices.
 * These index the structure produced after parsing an emote 
 * and finding all the details for its execution.
 */
#define EVH_VDET_SIZE 7 /* number of verb detail elements */
#define EVH_VDI_ACT 0   /* the actor object */
#define EVH_VDI_TRG 1  /* the target object */
#define EVH_VDI_ADV 2   /* the adverb */
#define EVH_VDI_TGBL 3  /* whether emote is still noticed if actor unseen */
#define EVH_VDI_SEN 4   /* the emote sentence, an array */  
#define EVH_VDI_VERB 5  /* the verb e.g. ({"grin", "grins", "grinning" }) */ 
#define EVH_VDI_ENDING 6 /* the ending i.e. ! or . */  

#endif


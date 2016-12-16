/*
 * ss_types2.h
 *
 * This file defines the available stats and skills that are locally
 * configured. The only use of this file is that it should be included
 * by /sys/ss_types.h.
 */

/* These are used in guild_support.c */
#define GS_PRIMARY_FOCUS        100
#define GS_SECONDARY_FOCUS       60
#define GS_DEFAULT_FOCUS         30

/* The highest skill number available for mudlib skills */
#define SS_MUDLIB_SKILL_END    4999

/*Define this if you want skills to be limited by stats*/
#define STAT_LIMITED_SKILLS

/* 
 * Limits are defined using the limiting stat and limiting weight in the
 * mapping SS_SKILL_DESC 
 */

/* List of skills as is going to be used */
#define SS_WEP_FIRST               0   /* The first weapon index */

#define SS_WEP_SWORD            SS_WEP_FIRST + 0  /* W_SWORD */
#define SS_WEP_KNIFE            SS_WEP_FIRST + 3  /* W_KNIFE */
#define SS_WEP_MISSILE          SS_WEP_FIRST + 5  /* W_MISSILE */

/* Description, Costfactor(0-100) Limiting Stat, Limit Weight, Max adv guild */

#define SS_SKILL_DESC ([ \
/* Weapon skills */                                                    \
    SS_WEP_SWORD:     ({ "sword",                 95, SS_DEX, 125, 30 }), \
    SS_WEP_KNIFE:     ({ "knife",                 46, SS_DEX, 125, 30 }), \
    SS_WEP_MISSILE:   ({ "missiles",              70, SS_DEX, 125, 30 }), \
/* General fighting skills */                                           \
    SS_2H_COMBAT:     ({ "two handed combat",     100, SS_DEX, 110, 0 }), \
    SS_UNARM_COMBAT:  ({ "unarmed combat",        90, SS_STR, 110, 30 }), \
    SS_BLIND_COMBAT:  ({ "blindfighting",         95, SS_DEX, 110, 20 }), \
    SS_PARRY:         ({ "parry",                 80, SS_STR, 110,  0 }), \
    SS_DEFENCE:       ({ "defence",               80, SS_DEX, 110, 30 }), \
    SS_MOUNTED_COMBAT:({ "mounted combat",        100, SS_STR, 150,  0 }), \
/* Magic skills */                                                   \
    SS_SPELLCRAFT:    ({ "spellcraft",            70, SS_INT, 125, 20 }), \
    SS_HERBALISM:     ({ "herbalism",             70, SS_WIS, 125, 20 }), \
    SS_ALCHEMY:       ({ "alchemy",               70, SS_INT, 125, 20 }), \
\
    SS_FORM_TRANSMUTATION: ({ "transmutation spells", 90, SS_INT, 110, 0 }), \
    SS_FORM_ILLUSION:    ({ "illusion spells",    70, SS_INT, 110, 0 }), \
    SS_FORM_DIVINATION:  ({ "divination spells",  70, SS_INT, 110, 0 }), \
    SS_FORM_ENCHANTMENT: ({ "enchantment spells", 80, SS_INT, 110, 0 }), \
    SS_FORM_CONJURATION: ({ "conjuration spells", 80, SS_INT, 110, 0 }), \
    SS_FORM_ABJURATION:  ({ "abjuration spells",  70, SS_INT, 110, 0 }), \
    SS_ELEMENT_FIRE:     ({ "fire spells",        70, SS_WIS, 110, 20 }), \
    SS_ELEMENT_AIR:      ({ "air spells",         70, SS_WIS, 110, 20 }), \
    SS_ELEMENT_EARTH:    ({ "earth spells",       70, SS_WIS, 110, 20 }), \
    SS_ELEMENT_WATER:    ({ "water spells",       70, SS_WIS, 110, 20 }), \
    SS_ELEMENT_LIFE:     ({ "life spells",        80, SS_WIS, 110, 20 }), \
    SS_ELEMENT_DEATH:    ({ "death spells",       90, SS_WIS, 110, 20 }), \
/* Thief skills */                                                  \
    SS_OPEN_LOCK:     ({ "open lock",             70, SS_DEX, 110, 20 }), \
    SS_PICK_POCKET:   ({ "pick pocket",           70, SS_DEX, 110, 20 }), \
    SS_ACROBAT:       ({ "acrobat",               70, SS_DEX, 110, 20 }), \
    SS_FR_TRAP:       ({ "find and remove traps", 70, SS_DEX, 110, 20 }), \
    SS_SNEAK:         ({ "sneak",                 70, SS_DEX, 125, 20 }), \
    SS_HIDE:          ({ "hide",                  70, SS_DEX, 125, 20 }), \
    SS_BACKSTAB:      ({ "backstab",              70, SS_DEX, 110,  0 }), \
/* General skills */                                                  \
    SS_APPR_MON:      ({ "appraise enemy",        50, SS_WIS, 150, 30 }), \
    SS_APPR_OBJ:      ({ "appraise object",       50, SS_WIS, 150, 30 }), \
    SS_APPR_VAL:      ({ "appraise value",        50, SS_WIS, 150, 30 }), \
    SS_SWIM:          ({ "swim",                  50, SS_STR, 150, 30 }), \
    SS_CLIMB:         ({ "climb",                 50, SS_STR, 125, 30 }), \
    SS_ANI_HANDL:     ({ "animal handling",       50, SS_WIS, 125, 30 }), \
    SS_LOC_SENSE:     ({ "location sense",        50, SS_WIS, 110, 30 }), \
    SS_TRACKING:      ({ "tracking",              50, SS_WIS, 125, 30 }), \
    SS_HUNTING:       ({ "hunting",               50, SS_INT, 125, 30 }), \
    SS_LANGUAGE:      ({ "language",              50, SS_INT, 110, 30 }), \
    SS_AWARENESS:     ({ "awareness",             50, SS_WIS, 110, 30 }), \
    SS_TRADING:       ({ "trading",               50, SS_INT, 110, 30 }), \
    SS_RIDING:        ({ "riding",                75, SS_DEX, 125, 30 }), \
])



/* 
 * Double-emote specifications. Included into the double emote soul. 
 * 
 * Coded by Maniac@Standard, May 1997
 * 
 * Copyright (C) Miguel Leith 1997
 */ 

static mapping verb_rec1 = 
    ([ 
          "ack" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBES("go"), "ack" }), 0, 
             0, 0, 
             1, 0, "!" }), 
          "admire" : 
          ({ 0, 0, 
            ({ EVH_ACT_SBJ, EVH_VERB("show"), EVH_TRG_OBJ, 
               EVH_ACT_POS, "admiration" }), 0, 
             1, 0, "." }), 
          "agree" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("agree"), EVH_ADVERB }), 
              "wholeheartedly", 
             ({ EVH_ACT_SBJ, EVH_VERB("agree"), EVH_ADVERB, 
                "with", EVH_TRG_OBJ }), "wholeheartedly", 
             1, 0, "." }), 
          "apologize" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("apologize"), EVH_ADVERB }), 
              "remorsefully", 
             ({ EVH_ACT_SBJ, EVH_VERBE("apologize"), EVH_ADVERB, 
                "to", EVH_TRG_OBJ }), "remorsefully", 
             1, 0, "." }), 
          "applaud" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("applaud"), EVH_ADVERB }), 
              "loudly", 
             ({ EVH_ACT_SBJ, EVH_VERB("applaud"), EVH_TRG_OBJ, 
                EVH_ADVERB }), "loudly", 
             1, 1, "." }), 
          "back" : 
          ({ ({ EVH_ACT_SBJ,EVH_VERB("back"),"away",EVH_ADVERB }), "slowly", 
             ({ EVH_ACT_SBJ,EVH_VERB("back"),"away", "from", EVH_TRG_OBJ, 
                EVH_ADVERB }), "slowly", 
             0, 0, "." }), 
          "bat" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("bat"), EVH_ACT_POS, "eyelashes", 
                EVH_ADVERB }), "coquetishly", 
             ({ EVH_ACT_SBJ, EVH_VERBD("bat"), EVH_ACT_POS, "eyelashes", 
                EVH_ADVERB, "at", EVH_TRG_OBJ }), "coquetishly", 
             0, 0, "." }), 
          "beckon" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("beckon"),"everyone", EVH_ADVERB }),
               "endearingly", 
             ({ EVH_ACT_SBJ, EVH_VERB("beckon"),EVH_TRG_OBJ, EVH_ADVERB }), 
               "endearingly", 
             0, 1, "." }), 
          "bite" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("bite"), "on", EVH_ACT_POS, 
                "bottom lip" }), 0, 
             0, 0, 
             0, 0, "." }), 
          "blank" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("stare"), "into space with a blank "+
              "expression on", EVH_ACT_POS, "face" }), 0, 
             0, 0, 
             0, 0, "." }), 
          "blink" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("blink"), "in disbelief" }), 0, 
             0, 0, 
             0, 0, "!" }), 
          "blow" :
          ({ 0, 0, 
            ({ EVH_ACT_SBJ, EVH_VERB("blow"), EVH_ADVERB, "in", EVH_TRG_POS,
               "ear" }), "gently", 
             1, 0, "." }), 
          "blush" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBES("blush"), EVH_ADVERB }), "profusely", 
            ({ EVH_ACT_SBJ, EVH_VERBES("blush"), EVH_ADVERB, "at", 
               EVH_TRG_OBJ }), "profusely", 
             1, 0, "." }), 
          "boggle" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("boggle"), "at the concept" }), 0, 
             0, 0, 
             0, 0, "." }), 
          "bounce" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("bounce"), "around", EVH_ADVERB }), 
             "uncontrollably", 
             0, 0, 
             1, 0, "." }), 
          "bow" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("bow"), EVH_ADVERB }), "gracefully", 
             ({ EVH_ACT_SBJ, EVH_VERB("bow"),EVH_ADVERB,"to",EVH_TRG_OBJ }), 
             "gracefully",
             0, 0, "." }), 
          "brighten" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("brighten"), "considerably" }), 0, 
             0, 0, 
             0, 0, "." }), 
          "burp" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("burp"), EVH_ADVERB }), "rudely", 
             0, 0, 
             1, 0, "." }), 
          "cackle" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("cackle"), EVH_ADVERB }), "with glee", 
             0, 0, 
             1, 0, "." }), 
          "caress" : 
          ({ 0, 0, 
            ({ EVH_ACT_SBJ, EVH_VERBES("caress"),EVH_TRG_OBJ,EVH_ADVERB }), 
            "lovingly", 
             1, 1, "." }), 
          "cheer" : 
          ({ ({ EVH_ACT_SBJ,EVH_VERB("cheer"),EVH_ADVERB }), 
              "enthusiastically", 
             ({EVH_ACT_SBJ,EVH_VERB("cheer"),EVH_ADVERB,"at",EVH_TRG_OBJ }),
             "enthusiastically", 
              1, 0, "." }), 
          "choke" : 
          ({ ({EVH_ACT_POS,"face slowly darkens as",EVH_ACT_SBJ, 
               EVH_VERBE("choke") }), 0, 
             0, 0, 
             1, 0, "." }), 
          "chortle" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("chortle") }), 0, 
             0, 0, 
             1, 0, "." }), 
          "chuckle" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("chuckle"), EVH_ADVERB }), "politely", 
             0, 0, 
             1, 0, "." }), 
          "clap" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("clap"), EVH_ADVERB }), "briefly",
             ({ EVH_ACT_SBJ, EVH_VERBD("clap"), EVH_ADVERB, "for", 
                EVH_TRG_OBJ }), "briefly", 
             1, 0, "." }), 
          "comfort" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("comfort"), EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
          "complain" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("complain"), "about the miserable " + 
                "state of being" }), 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("complain"), "about the miserable " + 
                "state of things to", EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
          "compliment" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("give"), EVH_ACT_POS, "deepest "+ 
                "compliments to", EVH_TRG_OBJ }), 0,  
             1, 0, "." }), 
          "confused" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("look"), "very confused" }), 0,
             ({ EVH_ACT_SBJ, EVH_VERB("look"), "very confused", "at", 
                EVH_TRG_OBJ }), 0, 
             0, 0, "." }), 
          "congratulate" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("congratulate"), EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
          "cough" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("cough"), EVH_ADVERB }), "noisily", 
             0, 0, 
             1, 0, "." }), 
          "cower" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("cower"), "in the corner" }), 0, 
             0, 0, 
             1, 0, "." }), 
          "cringe" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("cringe"), "in terror" }), 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("cringe"), "in terror at", 
                EVH_TRG_POS, "feet" }), 0, 
             1, 0, "." }), 
          "cry" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("burst"), "into tears" }), 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("rest"), EVH_ACT_POS, "head on", 
                EVH_TRG_POS, "shoulder and", EVH_VERBY("cry"), EVH_ACT_POS, 
                "heart out" }), 0, 
             1, 0, "." }), 
          "cuddle" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("cuddle"), EVH_TRG_OBJ, EVH_ADVERB }), 
             "lovingly", 
             1, 0, "." }), 
          "curl" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("curl"), "up in", EVH_TRG_POS, "lap" }), 
             0, 
             1, 0, "." }), 
          "curtsey" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("curtsey"),EVH_ADVERB}), "gracefully", 
             ({EVH_ACT_SBJ,EVH_VERB("curtsey"),EVH_ADVERB,"to",EVH_TRG_OBJ}),
             "gracefully", 
             0, 0, "." }), 
          "dance" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBES("do"), "the disco duck" }), 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("sweep"), EVH_TRG_OBJ, 
                         "across the dance floor" }), 0, 
             1, 0, "." }), 
          "despair" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("tear"), "a lump off", EVH_ACT_POS, 
                "scalp in despair" }), 0, 
             0, 0, 
             0, 0, "." }), 
          "disagree" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("disagree"), EVH_ADVERB }), "totally", 
             ({ EVH_ACT_SBJ, EVH_VERB("disagree"), EVH_ADVERB,"with",
               EVH_TRG_OBJ }), "totally", 
             1, 0, "." }), 
          "drool" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("drool"), EVH_ADVERB }), "excessively",
             0, 0, 
             0, 0, "." }), 
          "duh" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBES("go"), "duhhh" }), 0, 
             ({ EVH_ACT_SBJ, EVH_VERBES("go"), "duhhh at", EVH_TRG_OBJ }),0,
             1, 0, "." }), 
          "eeks" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBES("go"), "eeks" }), 0, 
             0, 0, 
             1, 0, "!" }), 
          "explode" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("explode"), "with anger" }), 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("explode"), "with anger at", 
                EVH_TRG_OBJ }), 0, 
              1, 0, "!" }), 
          "eyebrow" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("raise"), "an eyebrow", EVH_ADVERB }),
              "inquisitively", 
             ({ EVH_ACT_SBJ, EVH_VERBE("raise"), "an eyebrow", EVH_ADVERB, 
                "at", EVH_TRG_OBJ }), "inquisitively", 
              0, 0, "." }) 
        ]); 

static mapping verb_rec2 = 
      ([ 
          "fart" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("let"),"off a real rip-roarer" }), 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("fart"), "in", EVH_TRG_POS,  
                "general direction" }), 0, 
             1, 0, "." }), 
          "fawn" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("fawn"), "over", EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
          "feign" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("feign"), "a look of innocence" }), 0, 
             0, 0, 
             0, 0, "." }), 
          "fidget" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("fidget"), EVH_ADVERB }), 
              "like a squirrel", 0, 0, 
             1, 0, "." }), 
          "finger" : 
          ({ 0, 0, 
            ({ EVH_ACT_SBJ, EVH_VERBY("imply"), "with", EVH_ACT_POS, 
               "middle finger that", EVH_ACT_SBJ, EVH_VERB("want"), 
               EVH_TRG_OBJ, "to fuck off" }), 0, 
             0, 0, "." }), 
          "flex" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBES("flex"), EVH_ACT_POS, "muscles", 
                EVH_ADVERB }), "impressively", 
             0, 0, 
             0, 0, "." }),
          "flip" : 
          ({ ({EVH_ACT_SBJ, EVH_VERBD("flip"), "head over heels" }), 0, 
             0, 0, 
             0, 0, "." }), 
          "flirt" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("flirt"), EVH_ADVERB, "with", 
               EVH_TRG_OBJ }), "outrageously", 
             1, 0, "." }), 
          "fondle" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("fondle"), EVH_TRG_OBJ, EVH_ADVERB }), 
             "absentmindedly", 
             1, 1, "." }), 
          "forgive" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("forgive"), EVH_TRG_OBJ, 
                EVH_TRG_POS, "sins", EVH_ADVERB }), "generously", 
             1, 1, "." }), 
          "frown" : 
          ({ ({EVH_ACT_SBJ, EVH_VERB("frown") }), 0, 
             0, 0, 
             0, 0, "." }), 
          "fume" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("fume"), EVH_ADVERB }),"angrily",
             ({ EVH_ACT_SBJ, EVH_VERBE("fume"), EVH_ADVERB, "at", 
               EVH_TRG_OBJ }), "angrily", 
             1, 0, "." }), 
          "gag" : 
          ({ ({EVH_ACT_SBJ, EVH_VERBD("gag") }), 0, 
             0, 0, 
             0, 0, "." }), 
          "gasp" : 
          ({ ({EVH_ACT_SBJ, EVH_VERB("gasp"), "in astonishment" }), 0, 
             0, 0, 
             1, 0, "." }), 
          "gesture" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("gesture"), EVH_ADVERB }),"obscenely",
             ({ EVH_ACT_SBJ, EVH_VERBE("gesture"), EVH_ADVERB, "at", 
               EVH_TRG_OBJ }), "obscenely", 
             0, 0, "." }), 
          "giggle" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("giggle"), EVH_ADVERB }), "merrily", 
             0, 0, 
             1, 0, "." }), 
          "glare" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("glare"), EVH_ADVERB }), "stonily", 
             ({ EVH_ACT_SBJ, EVH_VERBE("glare"), EVH_ADVERB, "at", 
                EVH_TRG_OBJ }), "stonily", 
             0, 0, "." }), 
          "greet" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("greet"), EVH_TRG_OBJ, "warmly" }), 0, 
             1, 0, "." }), 
          "grimace" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("grimace"), EVH_ADVERB }), "painfully", 
             ({ EVH_ACT_SBJ, EVH_VERBE("grimace"), EVH_ADVERB, "at", 
                EVH_TRG_OBJ }), "anxiously", 
             0, 0, "." }), 
          "grin" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("grin"), EVH_ADVERB }), "evilly", 
             ({ EVH_ACT_SBJ, EVH_VERBD("grin"), EVH_ADVERB, "at", 
                EVH_TRG_OBJ }), "evilly", 
             0, 0, "." }), 
          "groan" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("groan"), EVH_ADVERB }), "loudly", 
             0, 0, 
             1, 0, "." }), 
          "grope" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("grope"), EVH_TRG_OBJ, 
                "in an unskilled manner" }), 0, 
             1, 0, "." }), 
          "grovel" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("grovel"), "before", EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
          "growl" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("growl"), EVH_ADVERB }), "menacingly", 
             ({ EVH_ACT_SBJ, EVH_VERB("growl"), EVH_ADVERB, "at", 
                EVH_TRG_OBJ }), "menacingly", 
             1, 0, "." }), 
          "grumble" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("grumble"), EVH_ADVERB }), "unhappily", 
             ({ EVH_ACT_SBJ, EVH_VERBE("grumble"), EVH_ADVERB, "at", 
                EVH_TRG_OBJ }), "unhappily", 
             1, 0, "." }), 
          "hang" : 
          ({ ({EVH_ACT_SBJ, EVH_VERB("hang"),EVH_ACT_POS,"head in shame" }),0,
             0, 0, 
             0, 0, "." }), 
          "hiccup" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("hiccup") }), 0, 
             0, 0, 
             1, 0, "!" }), 
          "hmm" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBES("go"), "hmmmm" }), 0, 
             0, 0, 
             1, 0, "." }), 
          "hold" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("hold"), EVH_TRG_OBJ, "close" }), 0, 
             1, 0, "." }), 
          "hug" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBD("hug"),EVH_TRG_OBJ,EVH_ADVERB }),"", 
             1, 1, "." }), 
          "ignore" : 
          ({ 0, 0, 
            ({EVH_ACT_SBJ,EVH_VERB("turn"),EVH_ACT_POS,"back on",EVH_TRG_OBJ,
                "and", EVH_VERB("start"), "to ignore", EVH_TRG_OBJ }), 0, 
             0, 0, "." }), 
          "jump" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("jump"), EVH_ADVERB }), "unexpectedly", 
             ({ EVH_ACT_SBJ, EVH_VERB("jump"), EVH_ADVERB, "all over", 
                EVH_TRG_OBJ }), "absentmindedly", 
              1, 0, "." }), 
          "kick" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("kick"), EVH_TRG_OBJ, EVH_ADVERB }),  
             "fanatically", 
             1, 1, "!" }), 
          "kiss" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBES("kiss"), EVH_TRG_OBJ, EVH_ADVERB }), 
             "gently", 
             1, 1, "." }), 
          "knee" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ,EVH_VERB("knee"),EVH_TRG_OBJ,"where it hurts"}),  
             0,  
             1, 1, "!" }), 
          "laugh" : 
          ({ ({EVH_ACT_SBJ,EVH_VERB("laugh"),EVH_ADVERB}),"exuberantly",  
             ({EVH_ACT_SBJ,EVH_VERB("laugh"),EVH_ADVERB,"at",EVH_TRG_OBJ }), 
             "exuberantly", 
             1, 0, "." }), 
          "lick" : 
          ({ ({EVH_ACT_SBJ,EVH_VERB("lick"),EVH_ACT_POS,"lips",EVH_ADVERB }),
              "in anticipation",  
             ({ EVH_ACT_SBJ, EVH_VERB("lick"), EVH_TRG_OBJ, EVH_ADVERB }),  
             "joyously", 
             1, 0, "." }), 
          "listen" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("listen"), EVH_ADVERB }), "attentively", 
             ({ EVH_ACT_SBJ, EVH_VERB("listen"), EVH_ADVERB, "to", 
                EVH_TRG_OBJ }), "attentively",  
             0, 0, "." }), 
          "love" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("whisper"), "sweet words of love to", 
                EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
     ]); 

static mapping verb_rec3 = 
      ([ 
          "melt" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ,EVH_VERB("melt"),"in",EVH_TRG_POS, "arms" }), 0, 
             1, 0, "." }), 
          "moan" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("start"), "to moan" }), 0, 
             0, 0, 
             1, 0, "." }), 
          "mumble" : 
          ({ ({ EVH_ACT_SBJ,EVH_VERBE("mumble"),"about something" }), 0, 
             ({ EVH_ACT_SBJ,EVH_VERBE("mumble"),"something about",
                EVH_TRG_OBJ }),0,
             1, 0, "." }), 
          "nibble" : 
          ({ 0, 0, 
             ({EVH_ACT_SBJ,EVH_VERBE("nibble"),"on",EVH_TRG_POS,"ear" }),0,
             1, 0, "." }), 
          "nod" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("nod"), EVH_ADVERB }), "solemnly", 
             ({ EVH_ACT_SBJ,EVH_VERBD("nod"),EVH_ADVERB,"at",EVH_TRG_OBJ }), 
             "solemnly", 
             0, 0, "." }), 
          "nudge" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("nudge"), EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
          "nuzzle" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("nuzzle"), EVH_TRG_OBJ,EVH_ADVERB }),
             "playfully", 
             1, 0, "." }), 
          "oops" : 
           ({ ({ EVH_ACT_SBJ, EVH_VERBES("go"), "oops" }), 0, 
              0, 0, 
              1, 0, "!" }), 
          "panic" : 
           ({ ({EVH_ACT_SBJ, EVH_VERBC("panic"), "and", EVH_VERB("look"),
               "for a place to hide" }), 0, 
              0, 0, 
              1, 0, "!" }), 
          "pat" : "pat", 
          "pause" : 
           ({ ({ EVH_ACT_SBJ, EVH_VERBE("pause") }), 0, 
              0, 0, 
              0, 0, "." }), 
          "peer" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("peer"),EVH_ADVERB,"around" }), 
             "quizzically", 
             ({ EVH_ACT_SBJ,EVH_VERB("peer"),EVH_ADVERB,"at",EVH_TRG_OBJ }), 
             "quizzically", 
             0, 0, "." }), 
          "pinch" : "pinch", 
          "poke" : "poke", 
          "ponder" : "ponder", 
          "point" : "point", 
          "pounce" : 
          ({ 0, 0, 
            ({ EVH_ACT_SBJ, EVH_VERBE("pounce"), "on", EVH_TRG_OBJ, 
              "like a cat, knocking", EVH_TRG_OBJ, "flat on", EVH_TRG_POS, 
              "back" }), 0, 
             1, 0, "!" }), 
          "pout" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("pout"), EVH_ADVERB }), "petulantly", 
             0, 0, 
             0, 0, "." }), 
          "puke" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("double"), "over and", 
                EVH_VERBE("puke") }), 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("puke"), "all over", EVH_TRG_OBJ }), 
                0, 
             1, 0, "." }), 
          "purr" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("purr"), EVH_ADVERB }), "contentedly", 
             0, 0, 
             1, 0, "." }), 
          "roar" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("roar"), EVH_ADVERB }), "aggressively", 
             0, 0, 
             1, 0, "." }), 
          "rolleyes" : 
          ({ ({EVH_ACT_SBJ,EVH_VERB("roll"), EVH_ACT_POS,"eyes",EVH_ADVERB }), 
             "in exasperation", 
             0, 0, 
             1, 0, "." }), 
          "ruffle" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("ruffle"), EVH_TRG_POS, "hair",
                EVH_ADVERB }), "playfully",
             1, 1, "." }), 
          "salute" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("salute"), EVH_TRG_OBJ }), 0, 
             0, 0, "." }), 
          "scold" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("scold"),"at no-one in particular" }),0,
             ({ EVH_ACT_SBJ, EVH_VERB("scold"), "the hell out of", 
                EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
          "scowl" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("scowl"), EVH_ADVERB }), "menacingly", 
             ({ EVH_ACT_SBJ,EVH_VERB("scowl"),EVH_ADVERB,"at",EVH_TRG_OBJ }),
             "menacingly", 
             0, 0, "." }),
          "scratch" : "scratch", 
          "shake" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("shake"), EVH_ACT_POS, "head "+ 
                "in disagreement" }), 0, 
             ({ EVH_ACT_SBJ,EVH_VERBE("shake"),EVH_TRG_POS, "hand" }), 0, 
             1, 0, "." }), 
          "shiver" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("shiver"), EVH_ADVERB }), 
               "from the cold", 
             0, 0, 
             1, 0, "." }), 
          "shrug" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("shrug"), EVH_ADVERB }), "helplessly", 
             0, 0, 
             0, 0, "." }), 
          "shudder" : 
          ({ ({EVH_ACT_SBJ,EVH_VERB("shudder"),EVH_ADVERB }),"at the thought",
             0, 0, 
             1, 0, "." }),
          "sigh" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("sigh"), EVH_ADVERB }), "deeply", 
             0, 0, 
             1, 0, "." }), 
          "sing" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("sing"), "in Italian" }), 0, 
             0, 0, 
             1, 0, "." }), 
          "slap" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBD("slap"), EVH_TRG_OBJ }), 0, 
             1, 0, "!" }), 
          "smile" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("smile"), EVH_ADVERB }), "happily", 
             ({ EVH_ACT_SBJ, EVH_VERBE("smile"), EVH_ADVERB,"at", 
                EVH_TRG_OBJ }), "happily", 
             0, 0, "." }),
          "smirk" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("smirk"), EVH_ADVERB }), "", 
             ({ EVH_ACT_SBJ, EVH_VERB("smirk"), EVH_ADVERB, "at", 
                EVH_TRG_OBJ }), "", 
             0, 0, "." }), 
       ]);


static mapping verb_rec4 = 
       ([ 
          "snap" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("snap"), EVH_ACT_POS, "fingers" }), 0, 
             0, 0, 
             1, 0, "." }),
          "snarl" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("snarl"), EVH_ADVERB, "at", 
                EVH_TRG_OBJ }), "savagely", 
             1, 0, "!" }), 
          "sneer" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("sneer"), EVH_ADVERB }), 
               "contemptuously",
             ({ EVH_ACT_SBJ, EVH_VERB("sneer"), EVH_ADVERB, "at", 
                EVH_TRG_OBJ }), "contemptuously", 
             1, 0, "." }), 
          "sneeze" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("sneeze") }), 0, 
             0, 0, 
             1, 0, "." }), 
          "snicker" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("snicker") }), 0, 
             0, 0, 
             1, 0, "." }), 
          "sniff" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("sniff"), EVH_ADVERB }), "pitifully", 
             0, 0, 
             1, 0, "." }), 
          "snore" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("snore"), EVH_ADVERB }), "loudly", 
             0, 0, 
             1, 0, "." }), 
          "snuggle" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("snuggle"),"up to",EVH_TRG_OBJ }),0,
             1, 0, "!" }), 
          "sob" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("sob"), EVH_ADVERB }), "sadly", 
             0, 0, 
             1, 0, "." }), 
          "spank" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("spank"), EVH_TRG_OBJ, EVH_ADVERB }), 
             "playfully", 
             1, 1, "!" }), 
          "spit" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("spit"), "on the ground in disgust" }),
              0, 
             ({ EVH_ACT_SBJ, EVH_VERBD("spit"), "on", EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
          "squeeze" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("squeeze"),EVH_TRG_OBJ,EVH_ADVERB }),
             "fondly", 
             1, 1, "." }), 
          "squirm" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("squirm"), EVH_ADVERB }), 
             "uncomfortably", 
             0, 0, 
             1, 0, "." }), 
          "stare" : 
          ({ ({EVH_ACT_SBJ,EVH_VERBE("stare"), EVH_ADVERB }),"into space",
             ({EVH_ACT_SBJ,EVH_VERBE("stare"),EVH_ADVERB,"into", EVH_TRG_POS,
              "eyes" }), "dreamily", 
             0, 0, "." }), 
          "startle" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("startle"), EVH_TRG_OBJ }), 0, 
             1, 0, "!" }), 
          "steam" : 
          ({ ({ "steam comes boiling out of", EVH_ACT_POS, "ears" }), 0, 
             0, 0, 
             1, 0, "!" }), 
          "stick" : 
          ({ ({ EVH_ACT_SBJ, EVH_ADVERB, EVH_VERB("stick"), EVH_ACT_POS, 
                "tongue out" }), "", 
             ({ EVH_ACT_SBJ, EVH_ADVERB, EVH_VERB("stick"), EVH_ACT_POS, 
                "tongue out at", EVH_TRG_OBJ }), "", 
             0, 0, "!" }), 
          "stomp" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("stomp"), EVH_ACT_POS, 
              "feet angrily on the ground" }), 0, 
             0, 0, 
             1, 0, "." }), 
          "stretch" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBES("stretch"),EVH_ACT_POS,"tired body"}), 
             0, 
             0, 0, 
             0, 0, "." }), 
          "strut" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("strut"), EVH_ADVERB }), 
             "proudly", 
             0, 0, 
             0, 0, "." }), 
          "sulk" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("sulk"), "in the corner" }), 0, 
             0, 0, 
             0, 0, "." }), 
          "swallow" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("swallow"), EVH_ADVERB }), 
             "uncomfortably", 
             0, 0, 
             1, 0, "." }), 
          "swear" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("swear"), EVH_ADVERB }), "loudly", 
             0, 0, 
             1, 0, "." }), 
          "sweat" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("sweat"), EVH_ADVERB }), 
             "profusely", 
             0, 0, 
             0, 0, "." }), 
          "tackle" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("tackle"), EVH_TRG_OBJ }), 0, 
             1, 0, "!" }),
          "tap" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBD("tap"), EVH_ACT_POS, "foot", 
                EVH_ADVERB }), "impatiently", 
             0, 0, 
             1, 0, "." }), 
          "tease" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ,EVH_VERBE("tease"), EVH_TRG_OBJ,EVH_ADVERB }),
             "playfully", 
             1, 1, "!" }), 
          "thank" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("thank"), EVH_TRG_OBJ, EVH_ADVERB }), 
             "gratefully", 
             1, 1, "!" }), 
          "think" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBY("try"), "to look thoughtful but", 
             EVH_VERB("fail") }), 0, 
             0, 0, 
             0, 0, "." }), 
          "tingle" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("tingle"), "with excitement" }), 0, 
             0, 0, 
             0, 0, "." }),
          "tickle" : 
          ({ 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERBE("tickle"), EVH_TRG_OBJ, EVH_ADVERB }),
             "playfully", 
             1, 0, "." }), 
          "tremble" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("tremble"),"and", EVH_VERB("quiver"),
              "like a bowlful of jelly" }), 0, 
             0, 0, 
             1, 0, "." }), 
          "twiddle" : 
          ({ ({ EVH_ACT_SBJ,EVH_VERBE("twiddle"),EVH_ACT_POS,"thumbs"}), 0, 
             0, 0, 
             0, 0, "." }), 
          "twinkle" : 
          ({ ({EVH_ACT_SBJ,EVH_VERBE("twinkle"),EVH_ACT_POS,"eyes", 
              EVH_ADVERB }), "merrily", 
             0, 0, 
             0, 0, "." }), 
          "twitch" : 
          ({ ({ EVH_ACT_POS, "left eye twitches", EVH_ADVERB }), "nervously",
             0, 0, 
             0, 0, "." }), 
          "wail" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERB("wail"), "in agony" }), 0, 
             0, 0, 
             1, 0, "!" }),
          "wave" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("wave"), EVH_ADVERB }), "happily", 
             ({ EVH_ACT_SBJ, EVH_VERBE("wave"), EVH_ADVERB, "in", 
                EVH_TRG_POS, "direction" }), "happily", 
             0, 0, "." }), 
          "whine" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("whine"), EVH_ADVERB }), "petulantly", 
             ({ EVH_ACT_SBJ, EVH_VERBE("whine"), EVH_ADVERB, "to", 
                EVH_TRG_OBJ, }), "petulantly", 
             1, 0, "." }), 
          "whistle" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("whistle"), EVH_ADVERB }), 
               "appreciatively", 
             ({ EVH_ACT_SBJ, EVH_VERBE("whistle"), EVH_ADVERB, "at", 
                EVH_TRG_OBJ, }), "appreciatively", 
             1, 0, "!" }), 
          "wiggle" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("wiggle"), EVH_ACT_POS, "bottom", 
                EVH_ADVERB }), "", 
             0, 0, 
             0, 0, "." }), 
          "wince" : 
          ({ ({ EVH_ACT_SBJ, EVH_VERBE("wince"), "in pain" }), 0, 
             0, 0, 
             1, 0, "!" }), 
          "wink" : 
          ({ ({EVH_ACT_SBJ, EVH_VERB("wink"), EVH_ADVERB }),"suggestively", 
             ({EVH_ACT_SBJ,EVH_VERB("wink"),EVH_ADVERB,"at",EVH_TRG_OBJ, }), 
             "suggestively", 
             0, 0, "." }), 
          "worry" : 
          ({ ({"a worried look spreads across", EVH_ACT_POS, "face" }), 0, 
             ({EVH_ACT_SBJ, EVH_VERB("look"), "worried about",EVH_TRG_OBJ }), 
             0, 
             0, 0, "." }), 
          "worship" : 
          ({ 0, 0, 
             ({EVH_ACT_SBJ, EVH_VERB("fall"), "to", EVH_ACT_POS, 
              "knees before", EVH_TRG_OBJ, "in worship" }), 0, 
             0, 0, "." }), 
          "yawn" : 
          ({ ({EVH_ACT_SBJ, EVH_VERB("yawn") }), 0, 
             ({EVH_ACT_SBJ, EVH_VERB("yawn"), "at", EVH_TRG_OBJ }), 0, 
             1, 0, "." }), 
          "yodel" : 
          ({ ({EVH_ACT_SBJ, EVH_VERBD("yodel"), "a merry tune" }), 0, 
             0, 0, 
             1, 0, "." }), 
      ]); 


/* 
 * SPECIAL EMOTE CASES 
 */ 

varargs mixed 
pat(string str, object trg)
{
    object *oblist;
    string *zones;
    string one, two;

    zones = ({ "back", "head", "shoulder" });

    notify_fail("Whom are you trying to pat?\n");
    if (!stringp(str))
    {
        str = "head";
    }

    str = lower_case(str);
    if (member_array(str, zones) != -1)
    {
        return ({ this_player(), 0, 0, 1, 
                 ({ EVH_ACT_SBJ, EVH_VERBD("pat"), EVH_ACT_OBJ, "on", 
                    EVH_ACT_POS, str }), 
                 "pat", "." }); 
    } 

    if (sscanf(str, "%s %s", one, two) == 2) 
    { 
        if (member_array(two, zones) == -1) 
            return 0; 
        str = one; 
    } 

    if (!stringp(two)) 
        two = "head"; 

    if (objectp(trg) && (str == "it" || str == trg->query_objective())) 
        oblist = ({ trg }); 
    else 
        oblist = parse_this(str, "[the] %l"); 

    if (!sizeof(oblist))
    {
        return 0;
    }

    if (sizeof(oblist) > 1) 
    { 
         notify_fail("You can't pat more than one target with a double " + 
                     "emote.\n"); 
         return 0;
    } 

    return ({ this_player(), oblist[0], 0, 1, 
             ({ EVH_ACT_SBJ, EVH_VERBD("pat"), EVH_TRG_OBJ, "on", 
                EVH_TRG_POS, two }), 
             "pat", "." }); 
}


#define POKE_ZONES ({ "eye", "ear", "nose", "thorax", "abdomen", \
                      "shoulder", "ribs" })

varargs mixed
poke(string str, object trg)
{
    object *oblist;
    string  location;
    string *words;

    if (!stringp(str)) 
    { 
        notify_fail("Poke whom [where]?\n"); 
        return 0; 
    } 

    words = explode(str, " "); 
    if (objectp(trg) && (words[0] == "it" || words[0] == trg->query_objective())) 
        oblist = ({ trg }); 
    else 
    { 
    /* We have to add all zones here manually. See the remark with pinch. */
        oblist = parse_this(str, "[the] %l [eye] [ear] [nose] " +
                                 "[thorax] [abdomen] [shoulder] [ribs]");
    } 

    if (!sizeof(oblist))
    {
        notify_fail("Poke whom [where]?\n");
        return 0;
    }

    if (sizeof(oblist) > 1) 
    { 
         notify_fail("You can't poke more than one target with a double " + 
                     "emote.\n"); 
         return 0;
    } 

    if (member_array(words[sizeof(words) - 1], POKE_ZONES) != -1)
    {
        location = (words[sizeof(words) - 1]);
    }
    else
    {
        location = "ribs";
    }

    return ({ this_player(), oblist[0], 0, 1, 
             ({ EVH_ACT_SBJ, EVH_VERBE("poke"), EVH_TRG_OBJ, "in the", 
                location }), "poke", "." }); 
}


#define PINCH_ZONES ({ "cheek", "ear", "nose", "arm", "bottom" }) 

varargs mixed
pinch(string str, object trg)
{
    object *oblist;
    string  location;
    string *words;

    if (!stringp(str))
    {
        notify_fail("Pinch whom [where]?\n");
        return 0;
    }

    words = explode(str, " ");
    if (objectp(trg) && (words[0] == "it" || words[0] == trg->query_objective()))  
        oblist = ({ trg }); 
    else 
    { 
    /* We have to add all zones here manually. See the remark with pinch. */
        oblist = parse_this(str, "[the] %l [cheek] [ear] [nose] " +
                                 "[arm] [bottom]");
    } 

    if (!sizeof(oblist)) 
    { 
        notify_fail("Pinch whom [where]?\n"); 
        return 0; 
    } 

    if (sizeof(oblist) > 1) 
    { 
         notify_fail("You can't pinch more than one target with a double " + 
                     "emote.\n"); 
         return 0; 
    } 

    if (member_array(words[sizeof(words) - 1], PINCH_ZONES) != -1) 
    { 
        location = (words[sizeof(words) - 1]); 
    } 
    else 
    { 
        location = "cheek"; 
    } 

    return ({ this_player(), oblist[0], 0, 1, 
             ({ EVH_ACT_SBJ, EVH_VERBES("pinch"), EVH_TRG_POS, location }), 
             "pinch", "." }); 
}


#define POINT_DIRECTIONS ({ "up", "down", "north", "south", "west", "east", \
    "northwest", "southwest", "northeast", "southeast" })

varargs mixed
point(string str, object trg)
{
    object *oblist;
    string *tmp;

    notify_fail("Where do you want to point?\n");

    if (!stringp(str))
    {
        return ({ this_player(), 0, 0, 0, 
                 ({ EVH_ACT_SBJ, EVH_VERB("point"),"in a general direction"}), 
                 "point", "." }); 
    }

    str = lower_case(str);
    if (member_array(str, POINT_DIRECTIONS) >= 0)
    {
        return ({ this_player(), 0, 0, 0, 
                 ({ EVH_ACT_SBJ, EVH_VERB("point"), str }), 
                 "point", "." }); 
    }

    if ((str == (this_player()->query_real_name())) ||
        (str == "me") ||
        (str == "myself"))
    {
        return ({ this_player(), this_player(), 0, 0, 
                 ({ EVH_ACT_SBJ, EVH_VERB("point"), "at", EVH_ACT_OBJ }), 
                 "point", "." }); 
    }

    if (objectp(trg) && (str == "it" || str == trg->query_objective())) 
        oblist = ({ trg }); 
    else 
        oblist = parse_this(str, "[the] %l");

    if (!sizeof(oblist))
    {
        tmp = explode(str, " ");
        if (sizeof(tmp) > 1 && tmp[0] == "at")
            str = implode(tmp[1 .. sizeof(tmp) - 1], " ");
        oblist = FIND_STR_IN_OBJECT(str, environment(this_player()));

        if (!sizeof(oblist))
        {
            if (environment(this_player())->item_id(str))
            {
                return ({ this_player(), 0, 0, 0, 
                   ({ EVH_ACT_SBJ, EVH_VERB("point"),"at "+LANG_ADDART(str) }), 
                      "point", "." }); 
            }
            return 0;
        }
        return ({ this_player(), 0, 0, 0, 
                 ({ EVH_ACT_SBJ, EVH_VERB("point"), 
                    "at "+LANG_THESHORT(oblist[0]) }), 
                 "point", "." }); 
    }

    if (sizeof(oblist) > 1) 
    { 
         notify_fail("You can't point at more than one target with a double " + 
                     "emote.\n"); 
         return 0;
    } 

    return ({ this_player(), oblist[0], 0, 0, 
             ({ EVH_ACT_SBJ, EVH_VERB("point"), "at", EVH_TRG_OBJ }), 
             "point", "." }); 
}


varargs mixed
ponder(string str, object trg)
{
    object *oblist;

    if (!stringp(str))
        return ({ this_player(), 0, 0, 0, 
                  ({ EVH_ACT_SBJ, EVH_VERB("ponder"), "the situation" }), 
                  "ponder", "." }); 

    if ((strlen(str) > 60) && 
        (!(this_player()->query_wiz_level()))) 
        return ({ this_player(), 0, 0, 0, 
                  ({ EVH_ACT_SBJ, EVH_VERB("ponder"), "the situation" }), 
                  "ponder", "." }); 

    if (objectp(trg) && (str == "it" || str == trg->query_objective())) 
        oblist = ({ trg }); 
    else 
        oblist = parse_this(str, "[the] %l"); 

    if (!sizeof(oblist))
        return ({ this_player(), 0, 0, 0, 
                  ({ EVH_ACT_SBJ, EVH_VERB("ponder"), str }), 
                  "ponder", "." }); 

    if (sizeof(oblist) > 1) 
    { 
         notify_fail("You can't ponder about more than one target with " + 
                     "a double emote.\n"); 
         return 0; 
    } 

    return ({ this_player(), oblist[0], 0, 0, 
              ({ EVH_ACT_SBJ, EVH_VERB("ponder"), EVH_TRG_POS, "proposal" }), 
              "ponder", "." }); 
}


varargs mixed
scratch(string str, object trg)
{

    object *oblist;
    string *zones;
    string one, two;

    zones = ({ "head", "chin", "back", "behind", "nose" });

    if (!stringp(str))
    {
        str = "head";
    }

    notify_fail("Scratch [whom] where?\n");
    if (member_array(str, zones) != -1)
    {
        return ({ this_player(), 0, 0, 0, 
                  ({ EVH_ACT_SBJ, EVH_VERBES("scratch"), EVH_ACT_POS, str }), 
                  "scratch", "." }); 
    }

    if (sscanf(str, "%s %s", one, two) == 2)
    {
        if (member_array(two, zones) == -1)
        {
            return 0;
        }
    }
    else 
        one = str; 

    if (!stringp(two))
    {
        two = "head";
    }

    if (objectp(trg) && (one == "it" || one == trg->query_objective())) 
        oblist = ({ trg }); 
    else 
    { 
        oblist = parse_this(one, "[the] %l"); 

        if (!sizeof(oblist))
        {
            return 0;
        }
        else if (sizeof(oblist) > 1) 
        { 
            notify_fail("You can't scratch more than one target with " +
                        "a double emote.\n"); 
            return 0;
        } 
    } 

    return ({ this_player(), oblist[0], 0, 1, 
              ({ EVH_ACT_SBJ, EVH_VERBES("scratch"), EVH_TRG_POS, two }), 
              "scratch", "." }); 
}


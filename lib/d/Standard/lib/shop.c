/*
 * /d/Standard/lib/shop.c
 *
 * All shops should inherit this file. In fact any object trading with
 * objects should since we have plans to introduce a global market where
 * prices on items depends on demand and supply. You do not have to use
 * any other features from this object more than the price formulas.
 * You may change them too a little if you please, but the general idea
 * must be followed. Right now this object only inherits /lib/shop.c but
 * one day, one day.... and by inheriting this object we plan for the future.
 *
 * /Nick
 */

#pragma save_binary

inherit "/lib/shop";

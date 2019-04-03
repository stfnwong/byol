/*
 * DOGE
 * Doge language test
 */

#include "mpc.h"


/* 
 * Some notes about the Doge Language
 *
 * An ADJECTIVE is either "wow", "many", "so" or "such"
 * A NOUN is either "lisp", "language", "c", "build" or "book"
 * A PHRASE is an ADJECTIVE followed by a NOUN
 * A DOGE is zero or more PHRASES
 *
 */


//// ADJECTIVE parser
//mpc_parser_t* Adjective = mpc_or(4,
//        mpc_sym("wow"), mpc_sym("so"),
//        mpc_sym("many"), mpc_sym("much")
//);
//
//// NOUN parser
//mpc_parser_t* Noun = mpc_or(5,
//        mpc_sym("lisp"), mpc_sym("language"),
//        mpc_sym("book"), mpc_sym("build"),
//        mpc_sym("c")
//);
//
//// PHRASE parser
//// This is a combination of the ADJECTIVE and NOUN parsers
//// TODO: further explanation of the mpcf_strfold and free args
//mpc_parser_t* Phrase = mpc_and(2,
//        mpcf_strfold, 
//        Adjective, Noun, free
//);
//
//// DOGE Parser
//// NOTE: this parser accepts zero or more occurances of a PHRASE parser. Another way 
//// to state this is that the parser accepts inputs of any length.
//mpc_parser_t* Doge = mpc_many(mpcf_strfold, Phrase);

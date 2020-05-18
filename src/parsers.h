/*
 * PARSERS
 * Forward declaration of MPC parsers 
 */

#ifndef __BYOL_PARSERS_H
#define __BYOL_PARSERS_H

#include "mpc.h"

// Parsers for individual components
mpc_parser_t* Number;
mpc_parser_t* Decimal; 
mpc_parser_t* Symbol;  
mpc_parser_t* String ; 
mpc_parser_t* Comment ;
mpc_parser_t* Sexpr   ;
mpc_parser_t* Qexpr   ;
mpc_parser_t* Expr    ;
mpc_parser_t* Lispy   ;


#endif /*__BYOL_PARSERS_H*/

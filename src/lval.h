/*
 * LVAL
 * Lisp values 
 */

#ifndef __BYOL_LVAL_H
#define __BYOL_LVAL_H

#include "mpc.h"

// list of valid lval types
typedef enum 
{
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_SEXPR
} lval_type;

// lval errors
typedef enum
{
    LERR_NONE,
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
} lval_err_type;

// Lisp value 
typedef struct 
{
    int    type;
    long   num;
    // error and symbol types have some string data
    char*  err;
    char*  sym;
    // pointer to a list of lval 
    int    count;
    struct lval** cell;
} lval;


// TODO : factor out common stuff into something like __lval_builder()

// lval constructors
lval* lval_num(long x);
lval* lval_err(char* m);
lval* lval_sym(char* s);
lval* lval_sexpr(void);

void  lval_del(lval* val);

void  lval_print(lval* v);
void  lval_println(lval* v);

// Convert MPC expressions to lvals
lval* lval_read_num(mpc_ast_t* ast);
lval* lval_read(mpc_ast_t* ast);
lval* lval_add(lval* v, lval* x);

void  lval_sexpr_print(lval* v, char open, char close);

#endif /*__BYOL_LVAL_H*/

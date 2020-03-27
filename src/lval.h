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

typedef struct lval lval;

struct lval
{
    int    type;
    long   num;
    // error and symbol types have some string data
    char*  err;
    char*  sym;
    // pointer to a list of lval 
    int    count;
    lval** cell;
};


// lval constructors
lval* lval_num(long x);
lval* lval_err(char* m);
lval* lval_sym(char* s);
lval* lval_sexpr(void);

void  lval_del(lval* val);

void  lval_print(lval* v);
void  lval_println(lval* v);

/*
 * lval_add()
 * Append an lval to another lval
 */
lval* lval_add(lval* v, lval* x);

/*
 * lval_pop()
 * Pop from the list the item at idx
 */
lval* lval_pop(lval* val, int idx);
/*
 * lval_take()
 * Remove an item from a list, deleting all other items
 */
lval* lval_take(lval* val, int idx);

/*
 * lval_builtin_op()
 * Evaluate a builtin operator
 */
lval* lval_builtin_op(lval* val, char* op);
/*
 * lval_eval_sexpr()
 */
lval* lval_eval_sexpr(lval* val);
/*
 * lval_eval()
 */
lval* lval_eval(lval* val);

void  lval_sexpr_print(lval* v, char open, char close);

#endif /*__BYOL_LVAL_H*/
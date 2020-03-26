/*
 * LVAL
 * Lisp values 
 */

#ifndef __BYOL_LVAL_H
#define __BYOL_LVAL_H

// list of valid lval types
typedef enum 
{
    LVAL_NUM,
    LVAL_ERR
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
    int type;
    long num;
    int err;
} lval;


lval lval_num(long x);
lval lval_err(int e);
void lval_print(lval v);
void lval_println(lval v);

#endif /*__BYOL_LVAL_H*/

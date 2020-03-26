/*
 * LVAL
 * Lisp values 
 */

#include <stdio.h>
#include "lval.h"

/* 
 * lval_num()
 * Construct a numeric lval
 */
lval lval_num(long x)
{
    lval val;

    val.type = LVAL_NUM;
    val.num  = x;
    val.err  = LERR_NONE;

    return val;
}

/*
 * lval_err()
 * Construct an error lval
 */
lval lval_err(int e)
{
    lval val;

    val.type = LVAL_ERR;
    val.num  = 0;
    val.err  = e;

    return val;
}

/*
 * lval_print()
 */
void lval_print(lval v)
{
    switch(v.type)
    {
        case LVAL_NUM:
            fprintf(stdout, "%li", v.num);
            break;

        case LVAL_ERR:
            switch(v.err)
            {
                case LERR_DIV_ZERO:
                    fprintf(stdout, "ERROR: division by zero");
                    break;

                case LERR_BAD_OP:
                    fprintf(stdout, "ERROR: Invalid operator");
                    break;

                case LERR_BAD_NUM:
                    fprintf(stdout, "ERROR: Invalid number");
                    break;
            }
            break;
    }
}

/*
 * lval_println()
 */
void lval_println(lval v)
{
    lval_print(v);
    putchar('\n');
}

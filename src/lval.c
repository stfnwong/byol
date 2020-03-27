/*
 * LVAL
 * Lisp values 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lval.h"

/*
 * __lval_create()
 * Private lval constructor
 */
lval* __lval_create(long num, double decimal, char* m, char* s, int type)
{
    lval* val;

    val = malloc(sizeof(*val));
    if(!val)
    {
        fprintf(stdout, "[%s] failed to allocate %ld bytes for lval\n", 
                __func__, sizeof(*val)
        );
        return NULL;
    }
    val->type    = type;
    val->num     = num;
    val->decimal = decimal;
    if(m != NULL)
    {
        val->err = malloc(strlen(m) + 1);
        strcpy(val->err, m);
    }
    else
        val->err   = NULL;
    if(s != NULL)
    {
        val->sym = malloc(strlen(s) + 1);
        strcpy(val->sym, s);
    }
    else
        val->sym   = NULL;

    val->count = 0;
    val->cell  = NULL;

    return val;
}


/* 
 * lval_num()
 * Construct a numeric lval
 */
lval* lval_num(long x)
{
    return __lval_create(x, 0.0f, NULL, NULL, LVAL_NUM);
}

/*
 * lval_decimal()
 */
lval* lval_decimal(double x)
{
    return __lval_create(0, x, NULL, NULL, LVAL_DECIMAL);
}

/*
 * lval_err()
 * Construct an error lval
 */
lval* lval_err(char* m)
{
    return __lval_create(0, 0.0f, m, NULL, LVAL_ERR);
}

/*
 * lval_sym()
 */
lval* lval_sym(char* s)
{
    return __lval_create(0, 0.0f, NULL, s, LVAL_SYM);
}

/*
 * lval_sexpr()
 */
lval* lval_sexpr(void)
{
    return __lval_create(0, 0.0f, NULL, NULL, LVAL_SEXPR);
}

/*
 * lval_qexpr()
 */
lval* lval_qexpr(void)
{
    return __lval_create(0, 0.0f, NULL, NULL, LVAL_QEXPR);
}


/*
 * lval_del()
 */
void lval_del(lval* val)
{
    switch(val->type)
    {
        case LVAL_NUM:
            break;      // nothing extra to do
        case LVAL_ERR:
            free(val->err);
            break;
        case LVAL_SYM:
            free(val->sym);
            break;
        case LVAL_SEXPR:
        case LVAL_QEXPR:
            for(int i = 0; i < val->count; ++i)
            {
                lval_del(val->cell[i]);
            }
            free(val->cell);
            break;
    }
    free(val);
}

/*
 * lval_print()
 */
void lval_print(lval* v)
{
    switch(v->type)
    {
        case LVAL_NUM:
            fprintf(stdout, "%li", v->num);
            break;

        case LVAL_ERR:
            fprintf(stdout, "ERROR: %s", v->err);
            break;

        case LVAL_SYM:
            fprintf(stdout, "%s", v->sym);
            break;
            
        case LVAL_SEXPR:
            lval_sexpr_print(v, '(', ')');
            break;

        case LVAL_QEXPR:
            lval_sexpr_print(v, '{', '}');
            break;
    }
}


/*
 * lval_println()
 */
void lval_println(lval* v)
{
    lval_print(v);
    fprintf(stdout, "\n");
}

/*
 * lval_add()
 */
lval* lval_add(lval* v, lval* x)
{
    v->count++;
    v->cell = realloc(v->cell, sizeof(*v) * v->count);
    if(!v->cell)
    {
        fprintf(stdout, "[%s] failed to realloc memory for cell\n", __func__);
        return NULL;
    }
    v->cell[v->count-1] = x;

    return v;
}

/*
 * lval_pop()
 */
lval* lval_pop(lval* val, int idx)
{
    if(idx < 0 || idx > val->count)
        return NULL;

    // get the idx'th item
    lval* x = val->cell[idx];

    // shift memory after the item at idx 
    memmove(
            &val->cell[idx], 
            &val->cell[idx+1],
            sizeof(lval*) * (val->count - idx - 1)
    );
    val->count--;

    // realloc the used memory 
    val->cell = realloc(val->cell, sizeof(lval*) * val->count);

    return x;
}

/*
 * lval_take()
 */
lval* lval_take(lval* val, int idx)
{
    lval* x = lval_pop(val, idx);
    lval_del(val);

    return x;
}

/*
 * lval_builtin_op()
 */
lval* lval_builtin_op(lval* val, char* op)
{
    // enusre that all args are numbers 
    for(int i = 0; i < val->count; ++i)
    {
        if(val->cell[i]->type != LVAL_NUM)
        {
            lval_del(val);
            return lval_err("Cannot operate on non-number");
        }
    }

    // Pop the first element
    lval* x = lval_pop(val, 0);
    // If no arguments and we have '-' operator then perform
    // unary negation
    if((strncmp(op, "-", 1) == 0) && val->count == 0)
        x->num = -x->num;

    // for all the remaining elements 
    while(val->count > 0)
    {
        lval* y = lval_pop(val, 0);

        if(strncmp(op, "+", 1) == 0)
            x->num = x->num + y->num;
        if(strncmp(op, "-", 1) == 0)
            x->num = x->num - y->num;
        if(strncmp(op, "*", 1) == 0)
            x->num = x->num * y->num;
        if(strncmp(op, "/", 1) == 0)
        {
            if(y->num == 0)
            {
                lval_del(x);
                lval_del(y);
                x = lval_err("Division by zero");
                break;
            }
            x->num = x->num / y->num;
        }

        // Modulo division
        if(strncmp(op, "%", 1) == 0)
            x->num = x->num % y->num;
        // Power
        if(strncmp(op, "^", 1) == 0)
            x->num = pow(x->num, y->num);
        // min function
        if(strncmp(op, "min", 3) == 0)
            x->num = (x->num <= y->num) ? x->num : y->num;
        // max function
        if(strncmp(op, "max", 3) == 0)
            x->num = (x->num >= y->num) ? x->num : y->num;

        lval_del(y);
    }
    lval_del(val);

    return x;
}

// TODO : add assert macro later

/*
 * lval_builtin_head()
 */
lval* lval_builtin_head(lval* val)
{
    if(val->count != 1)
    {
        lval_del(val);
        return lval_err("[head] passed too many arguments");
    }

    if(val->cell[0]->type != LVAL_QEXPR)
    {
        lval_del(val);
        return lval_err("[head] passed incorrect type (must be qexpr)");
    }

    if(val->cell[0]->count == 0)
    {
        lval_del(val);
        return lval_err("[head] passed {}");
    }

    // now take first argument
    lval* v = lval_take(val, 0);
    // remove all non-head elements and return
    while(v->count > 1)
    {
        lval_del(lval_pop(v, 1));
    }

    return v;
}

/*
 * lval_builtin_tail()
 */
lval* lval_builtin_tail(lval* val)
{
    if(val->count != 1)
    {
        lval_del(val);
        return lval_err("[tail] passed too many arguments");
    }

    if(val->cell[0]->type != LVAL_QEXPR)
    {
        lval_del(val);
        return lval_err("[tail] passed incorrect type (must be qexpr)");
    }

    if(val->cell[0]->count == 0)
    {
        lval_del(val);
        return lval_err("[tail] passed {}");
    }

    lval* v = lval_take(val, 0);
    // delete first element and return 
    lval_del(lval_pop(v, 0));

    return v;
}

/*
 * lval_builtin_list()
 */
lval* lval_builtin_list(lval* val)
{
    val->type = LVAL_QEXPR;
    return val;
}

/*
 * lval_builtin_eval()
 */
lval* lval_builtin_eval(lval* val)
{
    if(val->count != 1)
    {
        lval_del(val);
        return lval_err("[list] passed too many arguments");
    }
    if(val->cell[0]->type != LVAL_QEXPR)
    {
        lval_del(val);
        return lval_err("[list] passed incorrect type (must be qexpr)");
    }

    lval* x = lval_take(val, 0);
    x->type = LVAL_SEXPR;

    return lval_eval(x);
}

/*
 * lval_join()
 */
lval* lval_join(lval* a, lval* b)
{
    while(b->count)
        a = lval_add(a, lval_pop(b, 0));

    lval_del(b);
    return a;
}

/*
 * lval_builtin_join()
 */
lval* lval_builtin_join(lval* val)
{
    for(int i = 0; i < val->count; ++i)
    {
        if(val->cell[0]->type != LVAL_QEXPR)
            return lval_err("[join] passed incorrect type (must be qexpr)");
    }

    lval* v = lval_pop(val, 0);
    // join up all the elements of val
    while(val->count)
        v = lval_join(v, lval_pop(val, 0));
    lval_del(val);

    return v;
}

/*
 * lval_builtin()
 */
lval* lval_builtin(lval* val, char* func)
{
    if(strncmp(func, "list", 4) == 0)
        return lval_builtin_list(val);
    if(strncmp(func, "head", 4) == 0)
        return lval_builtin_head(val);
    if(strncmp(func, "tail", 4) == 0)
        return lval_builtin_tail(val);
    if(strncmp(func, "join", 4) == 0)
        return lval_builtin_join(val);
    if(strncmp(func, "eval", 4) == 0)
        return lval_builtin_eval(val);
    if(strstr("+-/*", func))
        return lval_builtin_op(val, func);
    lval_del(val);

    return lval_err("Unknown function");
}

/*
 * lval_eval_sexpr()
 */
lval* lval_eval_sexpr(lval* val)
{
    // eval children of this lval
    for(int i = 0; i < val->count; ++i)
        val->cell[i] = lval_eval(val->cell[i]);

    // Check errors
    for(int i = 0; i < val->count; ++i)
    {
        if(val->cell[i]->type == LVAL_ERR)
            return lval_take(val, i);
    }

    // we have an empty expression
    if(val->count == 0)
        return val;

    // single expression
    if(val->count == 1)
        return lval_take(val, 0);

    // ensure first element is a symbol
    lval* f = lval_pop(val, 0);
    if(f->type != LVAL_SYM)
    {
        lval_del(f);
        lval_del(val);
        return lval_err("S-Expression does not start with symbol");
    }

    // call builtin with operator
    lval* result = lval_builtin(val, f->sym);
    lval_del(f);
    
    return result;
}

/*
 * lval_eval()
 */
lval* lval_eval(lval* val)
{
    if(val->type == LVAL_SEXPR)
        return lval_eval_sexpr(val);

    return val;
}

/*
 * lval_sexpr_print()
 */
void lval_sexpr_print(lval* val, char open, char close)
{
    fprintf(stdout, "%c", open);

    for(int i = 0; i < val->count; ++i)
    {
        lval_print(val->cell[i]);
        // remove trailing space if this is the last element
        if(i != (val->count-1))
            fprintf(stdout, " ");
    }

    fprintf(stdout, "%c", close);
}

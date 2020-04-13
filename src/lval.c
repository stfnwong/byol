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
        fprintf(stderr, "[%s] failed to allocate %ld bytes for lval\n", 
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

    val->builtin = NULL;     
    val->env     = NULL;
    val->formals = NULL;     
    val->body    = NULL;     
    val->cell    = NULL;
    val->count   = 0;

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
lval* lval_err(char* fmt, ...)
{
    lval* verr = __lval_create(0, 0.0f, NULL, NULL, LVAL_ERR);

    // create a new va_list 
    va_list va;
    va_start(va, fmt);

    // limit messages to 512 bytes
    verr->err = malloc(sizeof(char) * 512);
    vsnprintf(verr->err, 511, fmt, va);
    // resize to the real size of the string
    verr->err = realloc(verr->err, strlen(verr->err) + 1);
    va_end(va);

    return verr;
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
 * lval_func()
 */
lval* lval_func(lbuiltin func)
{
    lval* val = __lval_create(0, 0.0f, NULL, NULL, LVAL_FUNC);
    val->builtin = func;

    return val;
}

/*
 * lval_lambda()
 */
lval* lval_lambda(lval* formals, lval* body)
{
    lval* val = __lval_create(0, 0.0f, NULL, NULL, LVAL_FUNC);

    val->builtin = NULL;
    val->env     = lenv_new();
    val->formals = formals;
    val->body    = body;

    return val;
}

/*
 * lval_del()
 */
void lval_del(lval* val)
{
    switch(val->type)
    {
        case LVAL_ERR:
            free(val->err); // TODO : double free here when div by zero
            break;
        case LVAL_FUNC:
            if(!val->builtin)
            {
                lenv_del(val->env);
                lval_del(val->formals);
                lval_del(val->body);
            }
            break;
        case LVAL_NUM:
        case LVAL_DECIMAL:
            break;      // nothing extra to do
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
 * lval_copy()
 */
lval* lval_copy(lval* val)
{
    lval* out = malloc(sizeof(*out));  
    if(!out)
    {
        fprintf(stderr, "[%s] failed to allocate %ld bytes for new lval\n", __func__, sizeof(*out));
        return NULL;
    }
    out->type = val->type;

    switch(val->type)
    {
        // numbers and functions can be copied directly
        case LVAL_FUNC:
            if(val->builtin != NULL)        // TODO : segfault from here
                out->builtin = val->builtin;
            else
            {
                val->builtin = NULL;
                val->env     = lenv_copy(val->env);
                val->formals = lval_copy(val->formals);
                val->body    = lval_copy(val->body);
            }
            break;

        case LVAL_NUM:
        case LVAL_DECIMAL:
            out->num = val->num;
            break;

        case LVAL_ERR:
            out->err = malloc(strlen(val->err) + 1);
            strcpy(out->err, val->err);
            break;

        case LVAL_SYM:
            out->sym = malloc(strlen(val->sym) + 1);
            strcpy(out->sym, val->sym);
            break;

        case LVAL_SEXPR:
        case LVAL_QEXPR:
            out->count = val->count;
            out->cell = malloc(sizeof(lval*) * val->count);
            for(int i = 0; i < val->count; ++i)
                out->cell[i] = lval_copy(val->cell[i]);
            break;
    }

    return out;
}

/*
 * lval_print()
 */
void lval_print(lval* v)
{
    switch(v->type)
    {
        case LVAL_ERR:
            fprintf(stdout, "ERROR: %s", v->err);
            break;
        case LVAL_DECIMAL:
        case LVAL_NUM:
            fprintf(stdout, "%li", v->num);
            break;
        case LVAL_FUNC:
            if(v->builtin)
                fprintf(stdout, "<builtin>");
            else
            {
                fprintf(stdout, "(\\");
                lval_print(v->formals);
                fprintf(stdout, " ");
                lval_print(v->body);
                fprintf(stdout, ")");
            }
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
 * lval_type_str()
 */
char* lval_type_str(lval_type t)
{
    switch(t)
    {
        case LVAL_FUNC:
            return "Function";
        case LVAL_NUM:
            return "Number";
        case LVAL_DECIMAL:
            return "Decimal";
        case LVAL_ERR:
            return "Error";
        case LVAL_SYM:
            return "Symbol";
        case LVAL_SEXPR:
            return "S-Expression";
        case LVAL_QEXPR:
            return "Q-Expression";
        default:
            return "Unkown type\0";
    }
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
        fprintf(stderr, "[%s] failed to realloc memory for cell\n", __func__);
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

// ======== MATHEMATICAL OPERATORS ======== //

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
            lval* err = lval_err("[%s] operator '%s' expected %s, got %s", 
                    __func__, op, lval_type_str(LVAL_NUM), lval_type_str(val->cell[i]->type));
            lval_del(val);
            return err;
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
                x = lval_err("[%s] Division by zero", __func__);
                lval_del(y);
                goto LVAL_BUILTIN_OP_END;
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

LVAL_BUILTIN_OP_END:
    lval_del(val);

    return x;
}

/*
 * lval_builtin_head()
 */
lval* lval_builtin_head(lval* val)
{
    LVAL_ASSERT_NUM(__func__, val, 1);
    LVAL_ASSERT_TYPE(__func__, val, 0, LVAL_QEXPR);
    LVAL_ASSERT_NOT_EMPTY(__func__, val, 0);

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
    LVAL_ASSERT_NUM(__func__, val, 1);
    LVAL_ASSERT_TYPE(__func__, val, 0, LVAL_QEXPR);
    LVAL_ASSERT_NOT_EMPTY(__func__, val, 0);

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
            LVAL_ASSERT_TYPE(__func__, val, 0, LVAL_QEXPR);
    }

    lval* v = lval_pop(val, 0);
    // join up all the elements of val
    while(val->count)
        v = lval_join(v, lval_pop(val, 0));
    lval_del(val);

    return v;
}


/*
 * lval_eval_sexpr()
 */
lval* lval_eval_sexpr(lenv* env, lval* val)
{
    // eval children of this lval
    for(int i = 0; i < val->count; ++i)
        val->cell[i] = lval_eval(env, val->cell[i]);

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

    // ensure first element is a function after evaluation
    lval* f = lval_pop(val, 0);
    if(f->type != LVAL_FUNC)
    {
        lval* err =  lval_err("[%s] S-Expression starts with incorrect type. Got %s, expected %s",
                __func__,
                lval_type_str(f->type),
                lval_type_str(LVAL_FUNC)
        );
        lval_del(f);
        lval_del(val);
        return err;
    }

    // call builtin with operator
    lval* result = lval_call(env, f, val);
    lval_del(f);
    
    return result;
}

/*
 * lval_eval()
 */
lval* lval_eval(lenv* env, lval* val)
{
    // lookup syms in the envrionment
    if(val->type == LVAL_SYM)
    {
        lval* v = lenv_get(env, val);
        lval_del(val);
        return v;
    }

    // evaluate s-exprs
    if(val->type == LVAL_SEXPR)
        return lval_eval_sexpr(env, val);

    return val;
}

/*
 * lval_builtin_eval()
 */
lval* lval_builtin_eval(lenv* env, lval* val)
{
    LVAL_ASSERT_NUM(__func__, val, 1);
    LVAL_ASSERT_TYPE(__func__, val, 0, LVAL_QEXPR);

    lval* x = lval_take(val, 0);
    x->type = LVAL_SEXPR;

    return lval_eval(env, x);
}

/*
 * lval_call()
 */
lval* lval_call(lenv* env, lval* func, lval* val)
{
    // if we have a builtin then just run that 
    if(func->builtin != NULL)
        return func->builtin(env, val);

    int given = val->count;
    int total = func->formals->count;

    // Process all the arguments to exhaustion
    while(val->count > 0)
    {
        // return an error if we run out of formal arguments to bind
        if(func->formals->count == 0)
        {
            lval_del(val);
            return lval_err("[%s] Function passed to many arguments. Got %i, expected %i", 
                    __func__,
                    given, 
                    total
            );
        }

        // pop the first symbol from the formals
        lval* sym  = lval_pop(func->formals, 0);

        // TODO : add special case for & here
        // Get symbol and argument, and bind to enviroment
        //lval* nsym = lval_pop(val, 0); 
        //lenv_put(func->env, nsym, builtin_list(env, val));
        //lval_del(sym);
        //lval_del(val);

        // pop the first symbol from the formals 
        lval* nval = lval_pop(val, 0);
        // bind a copy into the functions env 
        lenv_put(func->env, sym, nval);
        // clean up
        lval_del(sym);
        lval_del(nval);
    }
    lval_del(val);

    // Now that all formals are bound we can evaluate 
    if(func->formals->count == 0)
    {
        func->env->parent = env;
        return builtin_eval(
                func->env, 
                lval_add(lval_sexpr(), lval_copy(func->body))
        );
    }
        return lval_copy(func);
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


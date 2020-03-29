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
    val->func  = 0;     // what is the 'default' value for builtin?

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
    lval* val = malloc(sizeof(lval));

    val->type    = LVAL_FUNC;
    val->num     = 0;
    val->decimal = 0.0f;
    val->err     = NULL;
    val->sym     = NULL;
    val->func    = func;
    val->count   = 0;
    val->cell    = NULL;

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
        fprintf(stdout, "[%s] failed to allocate %ld bytes for new lval\n", __func__, sizeof(*out));
        return NULL;
    }
    out->type = val->type;

    switch(val->type)
    {
        // numbers and functions can be copied directly
        case LVAL_FUNC:
            out->func = val->func;
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
            fprintf(stdout, "<function>");
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
            return "Symol";
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
                x = lval_err("Division by zero");
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
        lval_del(f);
        lval_del(val);
        return lval_err("First element is not a function");
    }

    // call builtin with operator
    lval* result = f->func(env, val);
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

/*
 * ENVIRONMENT
 */

/*
 * lenv_new()
 */
lenv* lenv_new(void)
{
    lenv* env = malloc(sizeof(*env));
    if(!env)
    {
        fprintf(stdout, "[%s] failed to allocate %ld bytes for env\n",
                __func__, sizeof(*env)
        );
        return NULL;
    }

    env->count = 0;
    env->syms  = NULL;
    env->vals  = NULL;

    return env;
}

/*
 * lenv_del()
 */
void lenv_del(lenv* env)
{
    for(int i = 0; i < env->count; ++i)
    {
        free(env->syms[i]);
        lval_del(env->vals[i]);
    }

    free(env->syms);
    free(env->vals);
    free(env);
}

// ======== ENVIRONMENT BUILTINS ======== //
// List operators
lval* builtin_head(lenv* env, lval* val)
{
    return lval_builtin_head(val);
}
lval* builtin_tail(lenv* env, lval* val)
{
    return lval_builtin_tail(val);
}
lval* builtin_list(lenv* env, lval* val)
{
    return lval_builtin_list(val);
}
lval* builtin_eval(lenv* env, lval* val)
{
    return lval_builtin_eval(env, val);
}
lval* builtin_join(lenv* env, lval* val)
{
    return lval_builtin_join(val);
}

// Mathematical operators 
lval* builtin_add(lenv* env, lval* val)
{
    return lval_builtin_op(val, "+");
}
lval* builtin_sub(lenv* env, lval* val)
{
    return lval_builtin_op(val, "-");
}
lval* builtin_mul(lenv* env, lval* val)
{
    return lval_builtin_op(val, "*");
}
lval* builtin_div(lenv* env, lval* val)
{
    return lval_builtin_op(val, "/");
}
lval* builtin_mod(lenv* env, lval* val)
{
    return lval_builtin_op(val, "%");
}
lval* builtin_pow(lenv* env, lval* val)
{
    return lval_builtin_op(val, "^");
}
lval* builtin_min(lenv* env, lval* val)
{
    return lval_builtin_op(val, "min");
}
lval* builtin_max(lenv* env, lval* val)
{
    return lval_builtin_op(val, "max");
}

/*
 * builtin_def()
 * Define a new variable
 */
lval* builtin_def(lenv* env, lval* val)
{
    LVAL_ASSERT(val, val->cell[0]->type == LVAL_QEXPR,
            "Function 'def' passed incorrect type"
    );

    lval* syms = val->cell[0];     // first arg is symbol list
    // ensure that all elements in the first list are symbols
    for(int i = 0; i < syms->count; ++i)
    {
        LVAL_ASSERT(val, syms->cell[i]->type == LVAL_SYM,
                "Function 'def' cannot define non-symbols"
        );
    }

    // check correct number of symbols and values 
    LVAL_ASSERT(val, syms->count == val->count - 1,
            "Function 'def' number of syms (%ld) does not match number of vals (%ld)", syms->count, val->count - 1
    );

    // Assign copies of each value mapped to each symbol
    for(int i = 0; i < syms->count; ++i)
        lenv_put(env, syms->cell[i], val->cell[i+1]);
    lval_del(val);

    return lval_sexpr();
}




// TODO : some quick wins here later might be to use a data
// structure for the varibles that allows faster lookup
/*
 * lenv_get()
 */
lval* lenv_get(lenv* env, lval* val)
{
    // check if the variable exists
    for(int i = 0; i < env->count; ++i)
    {
        if(strcmp(env->syms[i], val->sym) == 0)
            return lval_copy(env->vals[i]);
    }

    return lval_err("Unbound symbol %s", val->sym);
}

/*
 * lenv_put()
*/
void lenv_put(lenv* env, lval* sym, lval* func)
{
    // check if the variable exists
    for(int i = 0; i < env->count; ++i)
    {
        // if we find 
        if(strcmp(env->syms[i], sym->sym) == 0)
        {
            lval_del(env->vals[i]);
            env->vals[i] = lval_copy(func);
            return;
        }
    }

    // Add a new variable
    env->count++;
    env->vals = realloc(env->vals, sizeof(lval*) * env->count);
    env->syms = realloc(env->syms, sizeof(char*) * env->count);

    env->vals[env->count - 1] = lval_copy(func);
    env->syms[env->count - 1] = malloc(strlen(sym->sym) + 1);
    strcpy(env->syms[env->count - 1], sym->sym);
}


/*
 * lenv_add_builtin()
 */
void lenv_add_builtin(lenv* env, char* name, lbuiltin func)
{
    lval* sym_name = lval_sym(name);
    lval* function = lval_func(func);
    lenv_put(env, sym_name, function);
    lval_del(sym_name);
    lval_del(function);
}

/*
 * lenv_init_builtins()
 */
void lenv_init_builtins(lenv* env)
{
    // list functions 
    lenv_add_builtin(env, "list", builtin_list);
    lenv_add_builtin(env, "head", builtin_head);
    lenv_add_builtin(env, "tail", builtin_tail);
    lenv_add_builtin(env, "eval", builtin_eval);
    lenv_add_builtin(env, "join", builtin_join);
    lenv_add_builtin(env, "def",  builtin_def);

    // operators
    lenv_add_builtin(env, "+",   builtin_add);
    lenv_add_builtin(env, "-",   builtin_sub);
    lenv_add_builtin(env, "*",   builtin_mul);
    lenv_add_builtin(env, "/",   builtin_div);
    lenv_add_builtin(env, "%",   builtin_mod);
    lenv_add_builtin(env, "^",   builtin_pow);
    lenv_add_builtin(env, "min", builtin_min);
    lenv_add_builtin(env, "max", builtin_max);
}

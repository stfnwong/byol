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

        // Get symbol and argument, and bind to enviroment
        lval* sym = lval_pop(func->formals, 0);
        lval* val = lval_pop(val, 0); // TODO : memory issue is here...
        lenv_put(env, sym, val);
        lval_del(sym);
        lval_del(val);
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

    env->count  = 0;
    env->syms   = NULL;
    env->vals   = NULL;
    env->parent = NULL;

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

/*
 * lenv_copy()
 */
lenv* lenv_copy(lenv* env)
{
    lenv* e = malloc(sizeof(*e));
    if(!e)
    {
        fprintf(stderr, "[%s] failed to allocate %ld bytes for new lenv\n",
                __func__, sizeof(*e)
        );
        return NULL;
    }

    e->parent = env->parent;
    e->count  = env->count;
    e->syms   = malloc(sizeof(char*) * env->count);
    e->vals   = malloc(sizeof(lval*) * env->count);

    for(int i = 0; i < env->count; ++i)
    {
        e->vals[i] = lval_copy(env->vals[i]);
        e->syms[i] = malloc(strlen(env->syms[i]) + 1);
        strcpy(e->syms[i], env->syms[i]);
    }

    return e;
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

    // Check if the symbol is in a parent environment
    if(env->parent)
        return lenv_get(env->parent, val);

    return lval_err("[%s] Unbound symbol %s", __func__, val->sym);
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
 * lenv_def()
 */
void lenv_def(lenv* env, lval* sym, lval* func)
{
    while(env->parent != NULL)
        env = env->parent;

    lenv_put(env, sym, func);
}


// ======== ENVIRONMENT BUILTINS ======== //
lval* builtin_var(lenv* env, lval* val, char* func)
{
    LVAL_ASSERT_TYPE(func, val, 0, LVAL_QEXPR);

    lval* syms = val->cell[0];
    for(int i = 0; i < syms->count; ++i)
    {
        LVAL_ASSERT(val, (syms->cell[i]->type == LVAL_SYM),
                "[%s] Function '%s' cannot def non-symbol. Got %s, expected %s",
                __func__, func, lval_type_str(syms->cell[i]->type), lval_type_str(LVAL_SYM)
        );
    }

    LVAL_ASSERT(val, (syms->count == val->count - 1), 
            "[%s] Function '%s' passed too many args for symbols. Got %i, expected %i", 
            __func__, func, syms->count, val->count - 1
    );

    for(int i = 0; i < syms->count; ++i)
    {
        // defs go in global scope, puts go in local scope 
        if(strncmp(func, "def", 3) == 0)
            lenv_def(env, syms->cell[i], val->cell[i+1]);
        if(strncmp(func, "=", 1) == 0)
            lenv_put(env, syms->cell[i], val->cell[i+1]);
    }
    lval_del(val);

    return lval_sexpr();

}
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
/*
 * builtin_lambda()
 */
lval* builtin_lambda(lenv* env, lval* val)
{
    LVAL_ASSERT_NUM("\\", val, 2);
    LVAL_ASSERT_TYPE("\\", val, 0, LVAL_QEXPR);
    LVAL_ASSERT_TYPE("\\", val, 1, LVAL_QEXPR);

    // ensure that the first Q-Expression only contains symbols
    for(int i = 0; i < val->cell[0]->count; ++i)
    {
        LVAL_ASSERT(val, (val->cell[0]->cell[i]->type == LVAL_SYM),
                "[%s] Cannot def non-symbol. Got %s, expected %s",
                __func__, 
                lval_type_str(val->cell[0]->cell[i]->type),
                lval_type_str(LVAL_SYM)
        );
    }

    // pop the first two args and pass them to lval_lambda()
    lval* formals = lval_pop(val, 0);
    lval* body    = lval_pop(val, 0);
    lval_del(val);

    return lval_lambda(formals, body);
}

/*
 * builtin_def()
 */
lval* builtin_def(lenv* env, lval* val)
{
    return builtin_var(env, val, "def");
}
/*
 * builtin_put()
 */
lval* builtin_put(lenv* env, lval* val)
{
    return builtin_var(env, val, "=");
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
    lenv_add_builtin(env, "\\",   builtin_lambda);
    lenv_add_builtin(env, "def",  builtin_def);
    lenv_add_builtin(env, "=",    builtin_put);

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

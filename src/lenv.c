/*
 * LENV
 * Lisp Environment 
 *
 */

#include "lenv.h"


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
    // variable functions
    lenv_add_builtin(env, "\\",   builtin_lambda);
    lenv_add_builtin(env, "def",  builtin_def);
    lenv_add_builtin(env, "=",    builtin_put);
    // list functions 
    lenv_add_builtin(env, "list", builtin_list);
    lenv_add_builtin(env, "head", builtin_head);
    lenv_add_builtin(env, "tail", builtin_tail);
    lenv_add_builtin(env, "eval", builtin_eval);
    lenv_add_builtin(env, "join", builtin_join);
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

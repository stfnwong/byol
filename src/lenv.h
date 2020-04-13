/*
 * LENV
 * Lisp Environment 
 *
 */

#ifndef __BYOL_LENV_H
#define __BYOL_LENV_H


#include "lval.h"

/*
 * ENVIRONMENT
 * Holds the names and values for all 'variables' in the
 * current scope.
 * NOTE: I don't know if scope is quite the right word here
 */
struct lenv
{
    int count;
    char** syms;
    lval** vals;
    lenv*  parent;
};


lenv* lenv_new(void);
void  lenv_del(lenv* env);
lenv* lenv_copy(lenv* env);

/*
 * lenv_get()
 * Get a variable from the environment. Check all the values 
 * currently in the environment and see if any match the 
 * lval given by val.
 */
lval* lenv_get(lenv* env, lval* val);
/*
 * lenv_put()
 * Puts a new variable into the environment. If the variable already 
 * exists then replace its existing value with the new value.
 */
void lenv_put(lenv* env, lval* sym, lval* func);
/*
 * lenv_def()
 * Perform def in the top-most parent of the env
 */
void lenv_def(lenv* env, lval* sym, lval* func);

/*
 * ENVIRONMENT BUILTINS
 */
lval* builtin_var(lenv* env, lval* val, char* func);
lval* builtin_list(lenv* env, lval* val);
lval* builtin_head(lenv* env, lval* val);
lval* builtin_tail(lenv* env, lval* val);
lval* builtin_eval(lenv* env, lval* val);
lval* builtin_join(lenv* env, lval* val);
lval* builtin_lambda(lenv* env, lval* val);
lval* builtin_def(lenv* env, lval* val);
lval* builtin_put(lenv* env, lval* val);

// operations
lval* builtin_add(lenv* env, lval* val);
lval* builtin_sub(lenv* env, lval* val);
lval* builtin_mul(lenv* env, lval* val);
lval* builtin_div(lenv* env, lval* val);
lval* builtin_mod(lenv* env, lval* val);
lval* builtin_pow(lenv* env, lval* val);
lval* builtin_min(lenv* env, lval* val);
lval* builtin_max(lenv* env, lval* val);

/*
 * lenv_add_builtin()
 */
void lenv_add_builtin(lenv* env, char* name, lbuiltin func);
/*
 * lenv_init_builtins()
 */
void lenv_init_builtins(lenv* env);  



#endif /*__BYOL_LENV_H*/

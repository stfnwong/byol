/*
 * LVAL
 * Lisp values 
 */

#ifndef __BYOL_LVAL_H
#define __BYOL_LVAL_H

#include "mpc.h"

// note : do I need the ol' do{....} while(0) trick here?
#define LVAL_ASSERT(args, cond, fmt, ...) \
    if (!(cond)) {\
        lval* err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del(args); \
        return err; \
    } \

// Assert on types 
#define LVAL_ASSERT_TYPE(func, args, idx, expected) \
    LVAL_ASSERT(args, args->cell[idx]->type == expected, \
            "[%s] Function '%s': incorrect type for argument %i. Got %s, expected %s.", \
            __func__, func, idx, lval_type_str(args->cell[idx]->type), lval_type_str(expected))

// Assert on numbers 
#define LVAL_ASSERT_NUM(func, args, expected) \
    LVAL_ASSERT(args, args->count == expected, \
            "[%s] Function '%s': incorrect number of args. Got %i, expected %i.", \
            __func__, func, args->count, expected)

// Assert non empty 
#define LVAL_ASSERT_NOT_EMPTY(func, args, idx) \
    LVAL_ASSERT(args, args->cell[idx]->count != 0, \
            "[%s] Function '%s': passed {} for argument %i.", \
            __func__, func, idx)


// list of valid lval types
typedef enum 
{
    LVAL_ERR,
    LVAL_NUM,
    LVAL_DECIMAL,
    LVAL_FUNC,
    LVAL_SYM,
    LVAL_SEXPR,
    LVAL_QEXPR
} lval_type;

// lval errors
typedef enum
{
    LERR_NONE,
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM
} lval_err_type;

// Forward declarations of values, environments
typedef struct lval lval;
typedef struct lenv lenv;

// Lisp builtin function
typedef lval* (*lbuiltin)(lenv*, lval*);

/*
 * VALUE
 */
struct lval
{
    lval_type type;
    long      num;
    double    decimal;
    // error and symbol types have some string data
    char*     err;
    char*     sym;
    // Functions
    lbuiltin  builtin;
    lenv*     env;
    lval*     formals;
    lval*     body;
    // Expressions 
    int       count;
    lval**    cell;
};

// lval constructors
lval* lval_num(long x);
lval* lval_decimal(double x);
lval* lval_err(char* fmt, ...);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
lval* lval_qexpr(void);
lval* lval_func(lbuiltin func);
lval* lval_lambda(lval* formals, lval* body);

/*
 * lval_del()
 * Cleanup memory allocated to an LVAL
 */
void  lval_del(lval* val);
lval* lval_copy(lval* val);

// Display
void  lval_print(lval* v);
void  lval_println(lval* v);
char* lval_type_str(lval_type t);

// NOTE: We only need a lenv pointer as the first 
// argument in order to match the lbuiltin type 
// signature. In practice we don't actually do 
// anything with the pointer that we pass.

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
 * lval_builtin_head()
 * Take a QExpr and return its first element as a QExpr
 */
lval* lval_builtin_head(lval* val);
/*
 * lval_builtin_tail()
 * Take a QExpr and return its last element as a QExpr
 */
lval* lval_builtin_tail(lval* val);
/*
 * lval_builtin_list()
 * Take an SExpr and return a QExpr
 */
lval* lval_builtin_list(lval* val);
/*
 * lval_builtin_join()
 * Take a QExpr and return its last element as a QExpr
 */
lval* lval_builtin_join(lval* val);
/*
 * lval_builtin_cons()
 * Takes a value and a QExpr and appends the value to the front
 */
lval* lval_builtin_cons(lval* val);
/*
 * lval_join()  
 * Inner function for join
 */
lval* lval_join(lval* a, lval* b);
/*
 * lval_eval_sexpr()
 */
lval* lval_eval_sexpr(lenv* env, lval* val);
/*
 * lval_eval()
 */
lval* lval_eval(lenv* env, lval* val);
/*
 * lval_builtin_eval()
 */
lval* lval_builtin_eval(lenv* env, lval* val);
/*
 * lval_call()
 */
lval* lval_call(lenv* env, lval* func, lval* val);



/*
 * lval_sexpr_print()
 */
void  lval_sexpr_print(lval* v, char open, char close);


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


#endif /*__BYOL_LVAL_H*/

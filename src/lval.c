/*
 * LVAL
 * Lisp values 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lval.h"

// TODO; factor out common part of implementation

/* 
 * lval_num()
 * Construct a numeric lval
 */
lval* lval_num(long x)
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
    val->type  = LVAL_NUM;
    val->num   = x;
    val->err   = NULL;
    val->sym   = NULL;
    val->count = 0;
    val->cell  = NULL;

    return val;
}

/*
 * lval_err()
 * Construct an error lval
 */
lval* lval_err(char* m)
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

    val->type  = LVAL_ERR;
    val->num   = 0;
    val->err   = malloc(strlen(m) + 1);
    val->sym   = NULL;
    val->count = 0;
    val->cell  = NULL;
    strcpy(val->err, m);

    return val;
}

/*
 * lval_sym()
 */
lval* lval_sym(char* s)
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

    val->type  = LVAL_SYM;
    val->num   = 0;
    val->err   = NULL;
    val->sym   = malloc(strlen(s) + 1);
    val->count = 0;
    val->cell  = NULL;
    strcpy(val->sym, s);

    return val;
}

/*
 * lval_sexpr()
 */
lval* lval_sexpr(void)
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

    val->type = LVAL_SEXPR;
    val->num   = 0;
    val->err   = NULL;
    val->sym   = NULL;
    val->count = 0;
    val->cell  = NULL;
    val->count =  0;
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
 * lval_read_num()
 */
lval* lval_read_num(mpc_ast_t* ast)
{
    errno = 0;
    long x = strtol(ast->contents, NULL, 10);
    if(errno == ERANGE)
        return lval_err("Invalid number");
    else
        return lval_num(x);
}

/*
 * lval_read()
 */
lval* lval_read(mpc_ast_t* ast)
{
    // If input is a symbol or a number then return a conversion to that type
    if(strstr(ast->tag, "number"))
        return lval_read_num(ast);
    if(strstr(ast->tag, "symbol"))
        return lval_sym(ast->contents);

    // If this is the root (>) or an S-expr then create an empty list 
    lval* val = NULL;
    if(strncmp(ast->tag, ">", 1) == 0)
        val = lval_sexpr();
    if(strstr(ast->tag, "sexpr"))
        val = lval_sexpr();

    // Fill the list with valid expressions in the sexpr
    for(int i = 0; i < ast->children_num; ++i)
    {
        // unpack parens
        if(strncmp(ast->children[i]->contents, "(", 1) == 0)
            continue;
        if(strncmp(ast->children[i]->contents, ")", 1) == 0)
            continue;
        if(strncmp(ast->children[i]->contents, "{", 1) == 0)
            continue;
        if(strncmp(ast->children[i]->contents, "}", 1) == 0)
            continue;
        if(strncmp(ast->children[i]->tag, "regex", 5) == 0)
            continue;

        val = lval_add(val, lval_read(ast->children[i]));
    }

    return val;
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

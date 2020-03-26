/*
 * REPL 
 * The main loop for the Lisp interpreter
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// editline
#include <editline/readline.h>
//#include <editline/history.h>
// MPC library 
#include "mpc.h"

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


// TODO : min(), max()


/*
 * eval_op()
 */
lval eval_op(lval x, char* op, lval y)
{
    // handle errors 
    if(x.type == LVAL_ERR)
        return x;
    if(y.type == LVAL_ERR)
        return y;

    // DEBUG 
    //fprintf(stdout, "[%s] evaluating operator %c for inputs (%li, %li)\n",
    //       __func__, *op, x.num, y.num
    //);
    
    // handle operators 
    if(strncmp(op, "+", 1) == 0)
        return lval_num(x.num + y.num);
    if(strncmp(op, "-", 1) == 0)
        return lval_num(x.num - y.num);
    if(strncmp(op, "*", 1) == 0)
        return lval_num(x.num * y.num);
    if(strncmp(op, "/", 1) == 0)
    {
        if(y.num == 0)
            return lval_err(LERR_DIV_ZERO);
        else
            return lval_num(x.num / y.num);
    }

    // Modulo division
    if(strncmp(op, "%", 1) == 0)
        return lval_num(x.num % y.num);
    if(strncmp(op, "^", 1) == 0)
        return lval_num(pow(x.num, y.num));

    return lval_err(LERR_BAD_OP);
}


/*
 * eval()
 */
lval eval(mpc_ast_t* ast)
{
    // If tagged as a number, return directly 
    if(strstr(ast->tag, "number"))
    {
        errno = 0;
        long x = strtol(ast->contents, NULL, 10);
        if(errno == ERANGE)
            return lval_err(LERR_BAD_NUM);
        else
            return lval_num(x);
    }

    // operator is always the second child
    // NOTE: is this really true?
    char* op = ast->children[1]->contents;
    lval  x  = eval(ast->children[2]);  

    // iterate over the remaining children and combine
    int i = 3;
    while(strstr(ast->children[i]->tag, "expr"))
    {
        // TODO : add negative operator (unary '-')
        x = eval_op(x, op, eval(ast->children[i]));
        i++;
    }

    return x;
}


int main(int argc, char *argv[])
{
    //Print version information
    fprintf(stdout, "Lispy Version 0.0.0.0.1\n");
    fprintf(stdout, "Press Ctrl+C to exit\n");

    // I suppose that the polish notation grammar goes here for now... since the Lispy symbol needs
    // to be visible in this translation unit

    // Polish notation parsers

    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
      "                                                     \
        number   : /-?[0-9]+/ ;                             \
        operator : '+' | '-' | '*' | '/' | '^' | '%';                  \
        expr     : <number> | '(' <operator> <expr>+ ')' ;  \
        lispy    : /^/ <operator> <expr>+ /$/ ;             \
      ",
      Number, Operator, Expr, Lispy
    );


    // main loop
    while(1)
    {
        char* input = readline("lispy> ");
        add_history(input);

        // attempt to parse the input 
        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Lispy, &r))
        {
            // on success we eval the AST
            //mpc_ast_print(r.output);
            lval result = eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);
        }
        else
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    // cleanup parsers 
    mpc_cleanup(4, Number, Operator, Expr, Lispy);

    return 0;
}

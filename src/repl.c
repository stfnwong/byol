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

// TODO : min(), max()

// eval functions 
long eval_op(long x, char* op, long y)
{
    if(strncmp(op, "+", 1) == 0)
        return x + y;
    if(strncmp(op, "-", 1) == 0)
        return x - y;
    if(strncmp(op, "*", 1) == 0)
        return x * y;
    if(strncmp(op, "/", 1) == 0)
        return x / y;

    // Modulo division
    if(strncmp(op, "%", 1) == 0)
        return x % y;
    if(strncmp(op, "^", 1) == 0)
        return pow(x, y);

    return 0;
}


long eval(mpc_ast_t* ast)
{
    // If tagged as a number, return directly 
    if(strstr(ast->tag, "number"))
        return atoi(ast->contents);

    // operator is always the second child
    // NOTE: is this really true?
    char* op = ast->children[1]->contents;
    long  x  = eval(ast->children[2]);  

    // iterate over the remaining children and combine
    int i = 3;
    while(strstr(ast->children[i]->tag, "expr"))
    {
        // TODO : add negative operator (unary '-')
        // TODO: recursively break down min and max into binary ops...
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
            long result = eval(r.output);
            fprintf(stdout, "%li\n", result);
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

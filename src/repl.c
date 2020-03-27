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

// Lisp value definitions
#include "lval.h"


// TODO : min(), max()


int main(int argc, char *argv[])
{
    //Print version information
    fprintf(stdout, "Lispy Version 0.0.0.0.1\n");
    fprintf(stdout, "Press Ctrl+C to exit\n");

    // I suppose that the polish notation grammar goes here for now... since the Lispy symbol needs
    // to be visible in this translation unit

    // Polish notation parsers

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr  = mpc_new("sexpr");
    mpc_parser_t* Expr   = mpc_new("expr");
    mpc_parser_t* Lispy  = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
      "                                               \
        number   : /-?[0-9]+/ ;                       \
        symbol   : '+' | '-' | '*' | '/' | '^' | '%'; \
        sexpr    : '(' <expr>* ')';                   \
        expr     : <number> | <symbol> | <sexpr> ;    \
        lispy    : /^/ <expr>* /$/ ;       \
      ",
      Number, Symbol, Sexpr, Expr, Lispy
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
            // debug print the expressions
            lval* x = lval_read(r.output);
            x = lval_eval(x);
            lval_println(x);
            lval_del(x);

        }
        else
        {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    // cleanup parsers 
    mpc_cleanup(5, Number, Symbol, Expr, Sexpr, Lispy);

    return 0;
}

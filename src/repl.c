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

#include "lval.h"


// TODO : min(), max()


/*
 * eval_op()
 */
//lval eval_op(lval x, char* op, lval y)
//{
//    // handle errors 
//    if(x.type == LVAL_ERR)
//        return x;
//    if(y.type == LVAL_ERR)
//        return y;
//
//    // DEBUG 
//    //fprintf(stdout, "[%s] evaluating operator %c for inputs (%li, %li)\n",
//    //       __func__, *op, x.num, y.num
//    //);
//    
//    // handle operators 
//    if(strncmp(op, "+", 1) == 0)
//        return lval_num(x.num + y.num);
//    if(strncmp(op, "-", 1) == 0)
//        return lval_num(x.num - y.num);
//    if(strncmp(op, "*", 1) == 0)
//        return lval_num(x.num * y.num);
//    if(strncmp(op, "/", 1) == 0)
//    {
//        if(y.num == 0)
//            return lval_err(LERR_DIV_ZERO);
//        else
//            return lval_num(x.num / y.num);
//    }
//
//    // Modulo division
//    if(strncmp(op, "%", 1) == 0)
//        return lval_num(x.num % y.num);
//    if(strncmp(op, "^", 1) == 0)
//        return lval_num(pow(x.num, y.num));
//
//    return lval_err(LERR_BAD_OP);
//}


/*
 * eval()
 */
//lval eval(mpc_ast_t* ast)
//{
//    // If tagged as a number, return directly 
//    if(strstr(ast->tag, "number"))
//    {
//        errno = 0;
//        long x = strtol(ast->contents, NULL, 10);
//        if(errno == ERANGE)
//            return lval_err(LERR_BAD_NUM);
//        else
//            return lval_num(x);
//    }
//
//    // operator is always the second child
//    // NOTE: is this really true?
//    char* op = ast->children[1]->contents;
//    lval  x  = eval(ast->children[2]);  
//
//    // iterate over the remaining children and combine
//    int i = 3;
//    while(strstr(ast->children[i]->tag, "expr"))
//    {
//        // TODO : add negative operator (unary '-')
//        x = eval_op(x, op, eval(ast->children[i]));
//        i++;
//    }
//
//    return x;
//}


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
            lval_println(x);
            lval_del(x);

            // on success we eval the AST
            //lval result = eval(r.output);
            //lval_println(result);
            //mpc_ast_delete(r.output);
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

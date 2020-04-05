/*
 * REPL 
 * The main loop for the Lisp interpreter
 */

#include <math.h>
#include <stdio.h>          // GNU has getline here, but may not work generally
#include <stdlib.h>
#include <string.h>
#include <unistd.h>         // for getopt
// editline
#include <editline/readline.h>
//#include <editline/history.h>
// MPC library 
#include "repl.h"


// =============== REPL OPTS 
/*
 * create_repl_opts()
 */
ReplOpts* repl_opts_create(void)
{
    ReplOpts* opts;


    opts = malloc(sizeof(*opts));
    if(!opts)
    {
        fprintf(stderr, "[%s] failed to alloc %ld bytes for repl options\n", __func__, sizeof(*opts));
        return NULL;
    }

    // set defaulfs
    opts->filename = NULL;

    return opts;
}


/*
 * destroy_repl_opts()
 */
void repl_opts_destroy(ReplOpts* opts)
{
    free(opts->filename);
    free(opts);
}

/*
 * repl_opts_add_filename()
 */
void repl_opts_add_filename(ReplOpts* opts, char* filename)
{
    if(opts->filename != NULL)
        opts->filename = realloc(opts->filename, sizeof(char) * strlen(filename) + 1);
    else
        opts->filename = malloc(sizeof(char) * strlen(filename) + 1);

    strcpy(opts->filename, filename);
}


/*
 * lval_read_num()
 */
lval* lval_read_num(mpc_ast_t* ast)
{
    long x;

    errno = 0;
    x = strtol(ast->contents, NULL, 10);
    if(errno == ERANGE)
        return lval_err("Invalid number");
    else
        return lval_num(x);
}

/*
 * lval_read_decimal()
 */
lval* lval_read_decimal(mpc_ast_t* ast)
{
    char* end;
    double dec; 

    errno = 0;
    dec = strtod(ast->contents, &end);
    if(errno == ERANGE)
        return lval_err("Invalid number");
    else
        return lval_decimal(dec);
}

/*
 * lval_read()
 */
lval* lval_read(mpc_ast_t* ast)
{
    // If input is a symbol or a number then return a conversion to that type
    if(strstr(ast->tag, "number"))
        return lval_read_num(ast);
    if(strstr(ast->tag, "decimal"))
        return lval_read_decimal(ast);
    if(strstr(ast->tag, "symbol"))
        return lval_sym(ast->contents);

    // If this is the root (>) or an S-expr then create an empty list 
    lval* val = NULL;
    if(strncmp(ast->tag, ">", 1) == 0)
        val = lval_sexpr();
    if(strstr(ast->tag, "sexpr"))
        val = lval_sexpr();
    if(strstr(ast->tag, "qexpr"))
        val = lval_qexpr();

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

int main(int argc, char *argv[])
{
    // Deal with args
    int opt;
    extern int optind;  // for checking filename

    ReplOpts* repl_opts = repl_opts_create();

    do
    {
        opt = getopt(argc, argv, "");

    } while(opt != -1);

    if(optind < argc)
    {
        char* filename = argv[optind];
        // Move to options 
        repl_opts_add_filename(repl_opts, filename);
    }

    // Parsers for individual components
    mpc_parser_t* Number  = mpc_new("number");
    mpc_parser_t* Decimal = mpc_new("decimal");
    mpc_parser_t* Symbol  = mpc_new("symbol");
    mpc_parser_t* Sexpr   = mpc_new("sexpr");
    mpc_parser_t* Qexpr   = mpc_new("qexpr");
    mpc_parser_t* Expr    = mpc_new("expr");
    mpc_parser_t* Lispy   = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
      "                                                       \
        number   : /-?[0-9]+/  ;                              \
        decimal  : /-?([0-9]*[.])?[0-9]+/  ;                  \
        symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/;          \
        sexpr    : '(' <expr>* ')';                           \
        qexpr    : '{' <expr>* '}';                           \
        expr     : <number> | <symbol> | <sexpr> | <qexpr> ;  \
        lispy    : /^/ <expr>* /$/ ;                          \
      ",
      Number, Decimal, Symbol, Sexpr, Qexpr, Expr, Lispy
    );

    // get a new lisp environment
    lenv* env = lenv_new();
    lenv_init_builtins(env);

    if(repl_opts->filename != NULL)
    {
        // Non interactive mode 
        fprintf(stdout, "[%s] non-interactive mode goes here\n", __func__);
        FILE*  fp;
        size_t len = 0;
        char*  line = NULL;

        fp = fopen(repl_opts->filename, "r");
        if(!fp)
        {
            fprintf(stderr, "[%s] failed to open file [%s]\n",
                    __func__, repl_opts->filename);
            goto CLEANUP;
        }

        while(1)
        {
            ssize_t read = getline(&line, &len, fp);
            if(read == -1)
                break;

            mpc_result_t r;
            if(mpc_parse("<stdin>", line, Lispy, &r))
            {
                lval* x = lval_eval(env, lval_read(r.output));
                // TODO : need to print only the result of eval (or have print function later...)
                lval_println(x);
                lval_del(x);
                mpc_ast_delete(r.output);
            }
            else
            {
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
            }
        }
    }
    else
    {
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
                lval* x = lval_eval(env, lval_read(r.output));
                lval_println(x);
                lval_del(x);
                mpc_ast_delete(r.output);
            }
            else
            {
                mpc_err_print(r.error);
                mpc_err_delete(r.error);
            }
            free(input);
        }
    }

CLEANUP:
    // Since we quit with sigterm, we are actually letting the OS 
    // clean up after us. 
    lenv_del(env);
    repl_opts_destroy(repl_opts);

    // cleanup parsers 
    mpc_cleanup(6, Number, Decimal, Symbol, Sexpr, Qexpr, Expr, Lispy);

    return 0;
}

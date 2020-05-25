/*
 * REPL 
 * The main loop for the Lisp interpreter
 */

#define _GNU_SOURCE     // for getline. TODO : find an alternative

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
#include "parsers.h"


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
 * lval_read_str()
 */
lval* lval_read_str(mpc_ast_t* ast)
{
    // remove the final quote character 
    ast->contents[strlen(ast->contents)-1] = '\0';
    // copy the string without the first quote character
    char* unescaped = malloc(strlen(ast->contents + 1) + 1);
    strcpy(unescaped, ast->contents+1);     
    unescaped = mpcf_unescape(unescaped);
    // construct a new lval containing the string 
    lval* str = lval_str(unescaped);
    free(unescaped);

    return str;
}

/*
 * builtin_load()
 */
lval* builtin_load(lenv* env, lval* a)
{
    LVAL_ASSERT_NUM("load", a, 1);
    LVAL_ASSERT_TYPE("load", a, 0, LVAL_STR);

    // parse file given by string name 
    mpc_result_t result;
    if(mpc_parse_contents(a->cell[0]->str, Lispy, &result))
    {
        lval* expr = lval_read(result.output);
        mpc_ast_delete(result.output);

        // evaluate each expression 
        while(expr->count)
        {
            fprintf(stdout, "[%s] evaluating expr %d\n", __func__, expr->count + 1);
            lval* x = lval_eval(env, lval_pop(expr, 0));
            if(x->type == LVAL_ERR)
                lval_println(x);
            lval_del(x);
        }
        // clean up expressions and arguments 
        lval_del(expr);
        lval_del(a);

        return lval_sexpr();        // return empty list
    }
    else
    {
        // get the parse error as a string 
        char* err_msg = mpc_err_string(result.error);
        mpc_err_delete(result.error);

        lval* err = lval_err("Could not load library %s", err_msg);
        free(err_msg);
        lval_del(a);

        return err;
    }
}

/*
 * lval_read()
 */
lval* lval_read(mpc_ast_t* ast)
{
    // If input is a symbol or a number then return a conversion to that type
    if(strstr(ast->tag, "number"))
        return lval_read_num(ast);
    //if(strstr(ast->tag, "decimal"))
    //    return lval_read_decimal(ast);
    if(strstr(ast->tag, "symbol"))
        return lval_sym(ast->contents);
    if(strstr(ast->tag, "string"))
        return lval_read_str(ast);

    // If this is the root (>) or an S-expr then create an empty list 
    lval* val = NULL;
    if(strcmp(ast->tag, ">") == 0)  // TODO :never evaluates true?
    {
        val = lval_sexpr();
    }
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
        if(strstr(ast->children[i]->tag, "comment"))
            continue;
        val = lval_add(val, lval_read(ast->children[i]));
    }

    return val;
}


// ================ ENTRY POINT ============== //
int main(int argc, char *argv[])
{
    // Deal with args
    extern int optind;  // for checking filename
    const char* version_string = "1.002";

    // get a new lisp environment
    lenv* env = lenv_new();
    lenv_init_builtins(env);

    ReplOpts* repl_opts = repl_opts_create();

    Number  = mpc_new("number");
    Decimal = mpc_new("decimal");
    Symbol  = mpc_new("symbol");
    String  = mpc_new("string");
    Comment = mpc_new("comment");
    Sexpr   = mpc_new("sexpr");
    Qexpr   = mpc_new("qexpr");
    Expr    = mpc_new("expr");
    Lispy   = mpc_new("lispy");


    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
      "                                                       \
        number   : /-?[0-9]+/  ;                              \
        decimal  : /-?([0-9]*[.])?[0-9]+/  ;                  \
        symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/;          \
        string   : /\"(\\\\.|[^\"])*\"/ ;                     \
        comment  : /;[^\\r\\n]*/ ;                            \
        sexpr    : '(' <expr>* ')';                           \
        qexpr    : '{' <expr>* '}';                           \
        expr     : <number> | <symbol> | <sexpr> | <qexpr>    \
                 | <string> | <comment> ;                     \
        lispy    : /^/ <expr>* /$/ ;                          \
      ",
      Number, Decimal, Symbol, String, Comment, Sexpr, Qexpr, Expr, Lispy
    );

    // interactive prompt
    if(argc == 1)
    {
        fprintf(stdout, "Lispy version %s\n", version_string);
        fprintf(stdout, "Press Ctrl+C to exit\n");

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

    // list of files 
    if(argc >= 2)
    {
        for(int i = 1; i < argc; ++i)
        {
            lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));
            lval* x    = builtin_load(env, args);
            if(x->type == LVAL_ERR)
                lval_println(x);

            lval_del(x);
        }
    }

    // Since we quit with sigterm, we are actually letting the OS 
    // clean up after us. 
    lenv_del(env);
    repl_opts_destroy(repl_opts);

    // cleanup parsers 
    mpc_cleanup(8, 
            Number, 
            Decimal, 
            Symbol, 
            String, 
            Comment, 
            Sexpr, 
            Qexpr, 
            Expr, 
            Lispy
    );

    return 0;
}

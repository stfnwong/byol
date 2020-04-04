/*
 * REPL
 * Stuff for the Lisp repl
 *
 * Stefan Wong 2020
 */


#ifndef __BYOL_REPL_H
#define __BYOL_REPL_H

#include "lval.h"
#include "mpc.h"

const char* LISPY_VERSION = "0.0.0.2";

// Convert MPC expressions to lvals
lval* lval_read_num(mpc_ast_t* ast);
lval* lval_read(mpc_ast_t* ast);

/*
 * repl options 
 */
typedef struct 
{
    char* filename;
} ReplOpts;


ReplOpts* repl_opts_create(void);
void      repl_opts_destroy(ReplOpts* opts);
void      repl_opts_add_filename(ReplOpts* opts, char* filename);


#endif /*__BYOL_REPL_H*/

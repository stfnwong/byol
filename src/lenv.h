/*
 * LENV
 * Lisp Environment
 */

#ifndef __BYOL_LENV_H
#define __BYOL_LENV_H

#include "lval.h"


typedef struct lenv lenv;

typedef lval* (*lbuiltin)(lenv*, lval*);






#endif /*__BYOL_LENV_H*/

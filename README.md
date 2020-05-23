# LISP INTERPRETER

![*An artists impression of a typical SCIP reader*](doc/scip-lord.jpeg)
##### *An artists impression of a typical SCIP reader in the wild*

This is me following the [Build Your Own Lisp](http://www.buildyourownlisp.com/) book and building a LISP. I did this because I wanted to go through 
[SCIP](https://mitpress.mit.edu/sites/default/files/sicp/full-text/book/book.html) and figured why not start from scratch?


## Requirements 
- For now using the provided MPC parser combinator library.
- libedit
- Makefile specifies gcc. I'm using GNU `getline()` at the moment.

## What works 
- Lambda's
- Strings

## TODO :
- Loading from disk 
- Loading from inside repl

## Quick Guide 
To build 

`make all`

This puts the repl binary in `bin/repl`. To start the interpreter 

`./bin/repl`

which should give the prompt 

`lispy>` 

At which you may type. For example, a simple add and multiply function (from the book) :

``` lisp
def {add-mul} (\ {x y} {+ x (* x y)})
add-mul 10 20
> 30
```

There are currently no unit tests. 

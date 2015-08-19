#ifndef LISPY_FWD_H
#define LISPY_FWD_H

#include <stdlib.h>
#include <stdio.h>

struct lenv;
struct lval;
struct lparser;
typedef struct lenv lenv;
typedef struct lval lval;
typedef struct lparser lparser;

typedef lval*(*lbuiltin)(lenv*, lval*);

#endif//LISPY_FWD_H

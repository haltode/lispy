#ifndef PARSER_H
#define PARSER_H

#include "mpc.h"

void init_parsers( mpc_parser_t **Number, mpc_parser_t **Symbol,
                   mpc_parser_t **Sexpr, mpc_parser_t **Expr,
                   mpc_parser_t **Lispy);

#endif

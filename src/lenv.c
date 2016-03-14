#include <stdlib.h>
#include <string.h>

#include "lval.h"
#include "eval.h"

lenv *lenv_new(void)
{
   lenv *env   = malloc(sizeof(lenv));
   env->parent = NULL;
   env->count  = 0;
   env->sym    = NULL;
   env->val    = NULL;
   return env;
}

void lenv_del(lenv *env)
{
   int iVar;

   for(iVar = 0; iVar < env->count; ++iVar) {
      free(env->sym[iVar]);
      lval_del(env->val[iVar]);
   }

   free(env->sym);
   free(env->val);
   free(env);
}

lval *lenv_get(lenv *env, lval *var)
{
   int iVar;

   for(iVar = 0; iVar < env->count; ++iVar)
      if(!strcmp(env->sym[iVar], var->sym))
         return lval_copy(env->val[iVar]);

   // Check if the variable is in an other parent
   if(env->parent)
      return lenv_get(env->parent, var);
   else
      return lval_err("Unbound symbol '%s'", var->sym);
}

void lenv_put(lenv *env, lval *var, lval *val)
{
   int iVar;

   // If the variable already exists, replace it's values
   for(iVar = 0; iVar < env->count; ++iVar) {
      if(!strcmp(env->sym[iVar], var->sym)) {
         lval_del(env->val[iVar]);
         env->val[iVar] = lval_copy(val);
         return;
      }
   }

   // Otherwise, create a new one
   ++(env->count);
   env->val = realloc(env->val, sizeof(lval*) * env->count);
   env->sym = realloc(env->sym, sizeof(char*) * env->count);

   env->val[env->count - 1] = lval_copy(val);
   env->sym[env->count - 1] = malloc(strlen(var->sym) + 1);
   strcpy(env->sym[env->count - 1], var->sym);
}

void lenv_def(lenv *env, lval *var, lval *val)
{
   while(env->parent)
      env = env->parent;

   lenv_put(env, var, val);
}

lenv *lenv_copy(lenv *env)
{
   lenv *new   = malloc(sizeof(lenv));

   new->parent = env->parent;
   new->count  = env->count;
   new->sym    = malloc(sizeof(char*) * new->count);
   new->val    = malloc(sizeof(lval*) * new->count);

   int iVar;
   for(iVar = 0; iVar < new->count; ++iVar) {
      new->sym[iVar] = malloc(strlen(env->sym[iVar]) + 1);
      strcpy(new->sym[iVar], env->sym[iVar]);
      new->val[iVar] = lval_copy(env->val[iVar]);
   }

   return new;
}

void lenv_add_builtin(lenv *env, char *name, lbuiltin func)
{
   lval *var = lval_sym(name);
   lval *val = lval_fun(func);
   lenv_put(env, var, val);
   lval_del(var);
   lval_del(val);
}

void lenv_add_builtins(lenv *env)
{
   // List functions
   lenv_add_builtin(env, "list", builtin_list);
   lenv_add_builtin(env, "head", builtin_head);
   lenv_add_builtin(env, "tail", builtin_tail);
   lenv_add_builtin(env, "eval", builtin_eval);
   lenv_add_builtin(env, "join", builtin_join);

   // Mathematical Functions
   lenv_add_builtin(env, "+", builtin_add);
   lenv_add_builtin(env, "-", builtin_sub);
   lenv_add_builtin(env, "*", builtin_mul);
   lenv_add_builtin(env, "/", builtin_div);
   lenv_add_builtin(env, "%", builtin_mod);
   lenv_add_builtin(env, "^", builtin_pow);
   lenv_add_builtin(env, "add", builtin_add);
   lenv_add_builtin(env, "sub", builtin_sub);
   lenv_add_builtin(env, "mul", builtin_mul);
   lenv_add_builtin(env, "div", builtin_div);
   lenv_add_builtin(env, "mod", builtin_mod);
   lenv_add_builtin(env, "pow", builtin_pow);
   lenv_add_builtin(env, "min", builtin_min);
   lenv_add_builtin(env, "max", builtin_max);

   // Variable functions
   lenv_add_builtin(env, "def", builtin_def);
   lenv_add_builtin(env, "\\", builtin_lambda);
   lenv_add_builtin(env, "=", builtin_put);
}

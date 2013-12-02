/*          
 * src/var.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _var_h
#define _var_h

int var_set(int user, char *name, char *val);
int var_get(int user, char *name, char *val, int len);
int var_get_def(int user, char *name, char *val, int len, char *def);
int var_set_unique(int user, char *val);
void var_setconfig(const char *filename);
int var_deluser(int user);

#endif


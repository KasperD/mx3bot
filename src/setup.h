/*
 * src/setup.h
 *
 * Copyright 2002 Colin O'Leary
 *
 */

#ifndef _setup_h
#define _setup_h

int setup_all(void);
int setup_database(const char *dir, const char *host, const char *user,
   const char *pass, const char *name);
int setup_add_owner(void);

#endif


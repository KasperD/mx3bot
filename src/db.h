/*
 * src/db.h
 *
 * Copyright 2002 Colin O'Leary
 *
 */

#ifndef _db_h
#define _db_h

#define DBTYPE_TEXT	0
#define DBTYPE_MYSQL	1

int db_init(int type, const char *path, const char *host,
   const char *user, const char *pass, const char *name,
   void (*error_func)(const char *msg));
int db_set_type(int type);

#endif

/*
 * src/sql.h
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 */

#ifndef _sql_h
#define _sql_h

#ifdef HAVE_MYSQL

#include <mysql.h>

#define DB_BOT		0
#define DB_FXSITE	1
#define DB_PHPNUKE	2

/* dbconnect */
#define DB_ERR_INIT	1
#define DB_ERR_CONNECT	2
#define DB_ERR_SELECT	3

typedef MYSQL_RES* DB_RESULT;
typedef MYSQL_ROW  DB_ROW;

int db_connect(int i, const char *host, const char *user, const char *pass,
   const char *name, void (*error_handler)(const char *msg));
int db_select(int i, const char *name);
void db_disconnect(int i);
MYSQL_RES *db_query(int i, const char *query, ...);
MYSQL_ROW db_fetch(MYSQL_RES *result);
int db_rows(MYSQL_RES *result);
int db_affected(int i);
void db_free(MYSQL_RES *result);
int db_int(const char *s);
int db_escape(char *to, const char *from, int max_size);
int db_id(int i, const char *table, const char *field);
char *db_error(int i);
void db_stats(int *query, int *fetch);

#endif /* HAVE_MYSQL */

#endif /* _db_h */

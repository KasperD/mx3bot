/*
 * src/config.h
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * If you came here looking for an autoconf-generated config.h,
 * there isn't one.  Try src/mx3bot.h.
 *
 */

#ifndef _config_h
#define _config_h

#define CONFIG_ERROR	-1
#define CONFIG_OK	0
#define CONFIG_DONE	1
#define CONFIG_NOTFOUND	2

int config_get_str(const char *filename, const char *section,
   const char *varname, char *buffer, int maxlen);
int config_get_int(const char *filename, const char *section,
   const char *varname, int *val);
int config_get_bool(const char *filename, const char *section,
   const char *varname, int *val);
const char *config_bool_str(int val);
int config_get_row(const char *filename, const char *section, char *row,
   int len);
int config_set_str(const char *filename, const char *section,
   const char *name, const char *val);
int config_set_row(const char *filename, int lineno, const char *row);
int config_add_row(const char *filename, const char *section,
   const char *row);
int config_get_line();

#endif


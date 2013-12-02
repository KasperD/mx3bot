/*
 * src/mx3bot.h
 *
 * Copyright 2001-2002 Colin O'Leary
 *
 * Declares globals and includes configure-generated autoconf.h.
 *
 */

#ifndef _mx3bot_h
#define _mx3bot_h

#ifdef HAVE_CONFIG_H
#include <autoconf.h>
#endif

/* global variables: defined in src/main.c */
extern char command_prefix;
extern int purge_delay;
extern char default_username[32], default_realname[80];

#endif


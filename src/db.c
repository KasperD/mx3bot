/*
 * src/db.c
 *
 * Copyright 2002 Colin O'Leary
 *
 * Database backend drivers.  Mostly generated from misc/db_template.c.
 *
 */

#include "mx3bot.h"

#include <stdio.h>

#include "db.h"
#include "log.h"
#include "sql.h"

#define DBTYPE_TEXT	0
#define DBTYPE_MYSQL	1

struct db_driver
{
   int (*init)(const char *path, const char *host, const char *user,
      const char *pass, const char *name,
      void (*error_func)(const char *msg));
   char* (*ban_get_list)(char *network, char *chan, char *ident);
   int (*ban_get_id)(char *network, char *chan, char *ident);
   int (*ban_get_id_bymask)(char *network, char *chan, char *mask);
   int (*ban_add)(char *network, char *chan, char *mask, int uid, char *note);
   int (*ban_del)(int bid);
   int (*ban_get)(int bid, int *uid, char *mask, int mask_len, char *note, int note_len);
   void (*ban_setconfig)(const char *filename);
   int (*ban_deluser)(int uid);
   void (*mode_set)(int user, char *net, char *chan, char *mode, char *p);
   void (*mode_get)(int user, char *net, char *chan, char *mode);
   int (*mode_check)(int user, char *net, char *chan, int mode, char *param, int len);
   char* (*mode_get_chan_list)(char *net, int mode);
   void (*mode_setconfig)(const char *filename);
   int (*mode_deluser)(int user);
   char* (*note_get_list)(int to, int from);
   int (*note_get)(int nid, int *to, int *from, int *sent, char *note, int len);
   int (*note_del)(int nid);
   int (*note_add)(int to, int from, int sent, char *note);
   void (*note_setconfig)(const char *filename);
   int (*note_deluser)(int uid);
   int (*setup_database)(const char *dir, const char *host, const char *user, const char *pass, const char *name);
   int (*userdb_add)(char *name, char *mask);
   int (*userdb_del)(int uid);
   int (*userdb_find_byname)(char *name);
   int (*userdb_find_bymask)(char *ident);
   int (*userdb_find_bymask3)(char *nick, char *user, char *host);
   int (*userdb_set_name)(int uid, char *name);
   char* (*userdb_get_name)(int uid);
   int (*userdb_set_password)(int uid, char *pass);
   int (*userdb_check_password)(int uid, char *pass);
   int (*userdb_has_password)(int uid);
   int (*userdb_addmask)(int uid, char *mask);
   int (*userdb_delmask)(int uid, char *mask);
   char* (*userdb_getmasklist)(int uid);
   char* (*userdb_getmask)(int mid);
   int (*userdb_count)(void);
   void (*userdb_setconfig)(const char *filename);
   int (*var_set)(int user, char *name, char *val);
   int (*var_get)(int user, char *name, char *val, int len);
   int (*var_get_def)(int user, char *name, char *val, int len, char *def);
   int (*var_set_unique)(int user, char *val);
   void (*var_setconfig)(const char *filename);
   int (*var_deluser)(int user);
};


static int db_text_init(const char *path, const char *host,
   const char *user, const char *pass, const char *name,
   void (*error_func)(const char *msg));

#ifdef HAVE_MYSQL
static int db_mysql_init(const char *path, const char *host,
   const char *user, const char *pass, const char *name,
   void (*error_func)(const char *msg));
#endif


/* text prototypes: functions located in src/db/text */

char* text_ban_get_list(char *network, char *chan, char *ident);
int text_ban_get_id(char *network, char *chan, char *ident);
int text_ban_get_id_bymask(char *network, char *chan, char *mask);
int text_ban_add(char *network, char *chan, char *mask, int uid, char *note);
int text_ban_del(int bid);
int text_ban_get(int bid, int *uid, char *mask, int mask_len, char *note, int note_len);
void text_ban_setconfig(const char *filename);
int text_ban_deluser(int uid);

void text_mode_set(int user, char *net, char *chan, char *mode, char *p);
void text_mode_get(int user, char *net, char *chan, char *mode);
int text_mode_check(int user, char *net, char *chan, int mode, char *param, int len);
char* text_mode_get_chan_list(char *net, int mode);
void text_mode_setconfig(const char *filename);
int text_mode_deluser(int user);

char* text_note_get_list(int to, int from);
int text_note_get(int nid, int *to, int *from, int *sent, char *note, int len);
int text_note_del(int nid);
int text_note_add(int to, int from, int sent, char *note);
void text_note_setconfig(const char *filename);
int text_note_deluser(int uid);

int text_setup_database(const char *dir, const char *host, const char *user, const char *pass, const char *name);

int text_userdb_add(char *name, char *mask);
int text_userdb_del(int uid);
int text_userdb_find_byname(char *name);
int text_userdb_find_bymask(char *ident);
int text_userdb_find_bymask3(char *nick, char *user, char *host);
int text_userdb_set_name(int uid, char *name);
char* text_userdb_get_name(int uid);
int text_userdb_set_password(int uid, char *pass);
int text_userdb_check_password(int uid, char *pass);
int text_userdb_has_password(int uid);
int text_userdb_addmask(int uid, char *mask);
int text_userdb_delmask(int uid, char *mask);
char* text_userdb_getmasklist(int uid);
char* text_userdb_getmask(int mid);
int text_userdb_count(void);
void text_userdb_setconfig(const char *filename);

int text_var_set(int user, char *name, char *val);
int text_var_get(int user, char *name, char *val, int len);
int text_var_get_def(int user, char *name, char *val, int len, char *def);
int text_var_set_unique(int user, char *val);
void text_var_setconfig(const char *filename);
int text_var_deluser(int user);


/* sql prototypes: functions located int src/db/sql */

char* sql_ban_get_list(char *network, char *chan, char *ident);
int sql_ban_get_id(char *network, char *chan, char *ident);
int sql_ban_get_id_bymask(char *network, char *chan, char *mask);
int sql_ban_add(char *network, char *chan, char *mask, int uid, char *note);
int sql_ban_del(int bid);
int sql_ban_get(int bid, int *uid, char *mask, int mask_len, char *note, int note_len);
void sql_ban_setconfig(const char *filename);
int sql_ban_deluser(int uid);

void sql_mode_set(int user, char *net, char *chan, char *mode, char *p);
void sql_mode_get(int user, char *net, char *chan, char *mode);
int sql_mode_check(int user, char *net, char *chan, int mode, char *param, int len);
char* sql_mode_get_chan_list(char *net, int mode);
void sql_mode_setconfig(const char *filename);
int sql_mode_deluser(int user);

char* sql_note_get_list(int to, int from);
int sql_note_get(int nid, int *to, int *from, int *sent, char *note, int len);
int sql_note_del(int nid);
int sql_note_add(int to, int from, int sent, char *note);
void sql_note_setconfig(const char *filename);
int sql_note_deluser(int uid);

int sql_setup_database(const char *dir, const char *host, const char *user, const char *pass, const char *name);

int sql_userdb_add(char *name, char *mask);
int sql_userdb_del(int uid);
int sql_userdb_find_byname(char *name);
int sql_userdb_find_bymask(char *ident);
int sql_userdb_find_bymask3(char *nick, char *user, char *host);
int sql_userdb_set_name(int uid, char *name);
char* sql_userdb_get_name(int uid);
int sql_userdb_set_password(int uid, char *pass);
int sql_userdb_check_password(int uid, char *pass);
int sql_userdb_has_password(int uid);
int sql_userdb_addmask(int uid, char *mask);
int sql_userdb_delmask(int uid, char *mask);
char* sql_userdb_getmasklist(int uid);
char* sql_userdb_getmask(int mid);
int sql_userdb_count(void);
void sql_userdb_setconfig(const char *filename);

int sql_var_set(int user, char *name, char *val);
int sql_var_get(int user, char *name, char *val, int len);
int sql_var_get_def(int user, char *name, char *val, int len, char *def);
int sql_var_set_unique(int user, char *val);
void sql_var_setconfig(const char *filename);
int sql_var_deluser(int user);

/* text driver */

static struct db_driver text =
{
   db_text_init,

   text_ban_get_list,
   text_ban_get_id,
   text_ban_get_id_bymask,
   text_ban_add,
   text_ban_del,
   text_ban_get,
   text_ban_setconfig,
   text_ban_deluser,

   text_mode_set,
   text_mode_get,
   text_mode_check,
   text_mode_get_chan_list,
   text_mode_setconfig,
   text_mode_deluser,

   text_note_get_list,
   text_note_get,
   text_note_del,
   text_note_add,
   text_note_setconfig,
   text_note_deluser,

   text_setup_database,

   text_userdb_add,
   text_userdb_del,
   text_userdb_find_byname,
   text_userdb_find_bymask,
   text_userdb_find_bymask3,
   text_userdb_set_name,
   text_userdb_get_name,
   text_userdb_set_password,
   text_userdb_check_password,
   text_userdb_has_password,
   text_userdb_addmask,
   text_userdb_delmask,
   text_userdb_getmasklist,
   text_userdb_getmask,
   text_userdb_count,
   text_userdb_setconfig,

   text_var_set,
   text_var_get,
   text_var_get_def,
   text_var_set_unique,
   text_var_setconfig,
   text_var_deluser
};

/* mysql driver */

#ifdef HAVE_MYSQL
static struct db_driver mysql =
{
   db_mysql_init,

   sql_ban_get_list,
   sql_ban_get_id,
   sql_ban_get_id_bymask,
   sql_ban_add,
   sql_ban_del,
   sql_ban_get,
   NULL,
   sql_ban_deluser,

   sql_mode_set,
   sql_mode_get,
   sql_mode_check,
   sql_mode_get_chan_list,
   NULL,
   sql_mode_deluser,

   sql_note_get_list,
   sql_note_get,
   sql_note_del,
   sql_note_add,
   NULL,
   sql_note_deluser,

   sql_setup_database,

   sql_userdb_add,
   sql_userdb_del,
   sql_userdb_find_byname,
   sql_userdb_find_bymask,
   sql_userdb_find_bymask3,
   sql_userdb_set_name,
   sql_userdb_get_name,
   sql_userdb_set_password,
   sql_userdb_check_password,
   sql_userdb_has_password,
   sql_userdb_addmask,
   sql_userdb_delmask,
   sql_userdb_getmasklist,
   sql_userdb_getmask,
   sql_userdb_count,
   NULL,

   sql_var_set,
   sql_var_get,
   sql_var_get_def,
   sql_var_set_unique,
   NULL,
   sql_var_deluser
};
#endif

static struct db_driver *driver;

int db_set_type(int type)
{
   switch(type)
   {
      case DBTYPE_TEXT:
         driver = &text;
         break;

#ifdef HAVE_MYSQL
      case DBTYPE_MYSQL:
         driver = &mysql;
         break;
#endif

      default:
         return -1;
   }

   return 0;
}

int db_init(int type, const char *path, const char *host, const char *user,
   const char *pass, const char *name, void (*error_func)(const char *msg))
{
   if(db_set_type(type) != 0)
   {
      if(error_func)
         error_func("Invalid database type.");
      return -1;
   }

   return driver->init(path, host, user, pass, name, error_func);
}

static int db_text_init(const char *path, const char *host,
   const char *user, const char *pass, const char *name,
   void (*error_func)(const char *msg))
{
   char filename[1024];

   snprintf(filename, 1024, "%s/bans.conf", path);
   text_ban_setconfig(filename);
   snprintf(filename, 1024, "%s/modes.conf", path);
   text_mode_setconfig(filename);
   snprintf(filename, 1024, "%s/notes.conf", path);
   text_note_setconfig(filename);
   snprintf(filename, 1024, "%s/users.conf", path);
   text_userdb_setconfig(filename);
   snprintf(filename, 1024, "%s/vars.conf", path);
   text_var_setconfig(filename);

   return 0;
}

#ifdef HAVE_MYSQL
static int db_mysql_init(const char *path, const char *host,
   const char *user, const char *pass, const char *name,
   void (*error_func)(const char *msg))
{
   if(db_connect(0, host, user, pass, name, error_func) != 0)
   {
      log_error("Cannot connect to `%s' on `%s'.", name, host);
      return -1;
   }

   return 0;
}
#endif


/* functions: the rest of this file is (mostly) generated by a perl script
   from misc/dbfunc */

char *ban_get_list(char *network, char *chan, char *ident)
{
   return driver->ban_get_list(network, chan, ident);
}

int ban_get_id(char *network, char *chan, char *ident)
{
   return driver->ban_get_id(network, chan, ident);
}

int ban_get_id_bymask(char *network, char *chan, char *mask)
{
   return driver->ban_get_id_bymask(network, chan, mask);
}

int ban_add(char *network, char *chan, char *mask, int uid, char *note)
{
   return driver->ban_add(network, chan, mask, uid, note);
}

int ban_del(int bid)
{
   return driver->ban_del(bid);
}

int ban_get(int bid, int *uid, char *mask, int mask_len, char *note, int note_len)
{
   return driver->ban_get(bid, uid, mask, mask_len, note, note_len);
}

void ban_setconfig(const char *filename)
{
   if(driver->ban_setconfig)
      driver->ban_setconfig(filename);
}

int ban_deluser(int uid)
{
   return driver->ban_deluser(uid);
}

void mode_set(int user, char *net, char *chan, char *mode, char *p)
{
   driver->mode_set(user, net, chan, mode, p);
}

void mode_get(int user, char *net, char *chan, char *mode)
{
   driver->mode_get(user, net, chan, mode);
}

int mode_check(int user, char *net, char *chan, int mode, char *param, int len)
{
   return driver->mode_check(user, net, chan, mode, param, len);
}

char* mode_get_chan_list(char *net, int mode)
{
   return driver->mode_get_chan_list(net, mode);
}

void mode_setconfig(const char *filename)
{
   if(driver->mode_setconfig)
      driver->mode_setconfig(filename);
}

int mode_deluser(int user)
{
   return driver->mode_deluser(user);
}

char* note_get_list(int to, int from)
{
   return driver->note_get_list(to, from);
}

int note_get(int nid, int *to, int *from, int *sent, char *note, int len)
{
   return driver->note_get(nid, to, from, sent, note, len);
}

int note_del(int nid)
{
   return driver->note_del(nid);
}

int note_add(int to, int from, int sent, char *note)
{
   return driver->note_add(to, from, sent, note);
}

void note_setconfig(const char *filename)
{
   if(driver->note_setconfig)
      driver->note_setconfig(filename);
}

int note_deluser(int uid)
{
   return driver->note_deluser(uid);
}

int setup_database(const char *dir, const char *host, const char *user,
   const char *pass, const char *name)
{
   return driver->setup_database(dir, host, user, pass, name);
}

int userdb_add(char *name, char *mask)
{
   return driver->userdb_add(name, mask);
}

int userdb_del(int uid)
{
   return driver->userdb_del(uid);
}

int userdb_find_byname(char *name)
{
   return driver->userdb_find_byname(name);
}

int userdb_find_bymask(char *ident)
{
   return driver->userdb_find_bymask(ident);
}

int userdb_find_bymask3(char *nick, char *user, char *host)
{
   return driver->userdb_find_bymask3(nick, user, host);
}

int userdb_set_name(int uid, char *name)
{
   return driver->userdb_set_name(uid, name);
}

char* userdb_get_name(int uid)
{
   return driver->userdb_get_name(uid);
}

int userdb_set_password(int uid, char *pass)
{
   return driver->userdb_set_password(uid, pass);
}

int userdb_check_password(int uid, char *pass)
{
   return driver->userdb_check_password(uid, pass);
}

int userdb_has_password(int uid)
{
   return driver->userdb_has_password(uid);
}

int userdb_addmask(int uid, char *mask)
{
   return driver->userdb_addmask(uid, mask);
}

int userdb_delmask(int uid, char *mask)
{
   return driver->userdb_delmask(uid, mask);
}

char* userdb_getmasklist(int uid)
{
   return driver->userdb_getmasklist(uid);
}

char* userdb_getmask(int mid)
{
   return driver->userdb_getmask(mid);
}

int userdb_count(void)
{
   return driver->userdb_count();
}

void userdb_setconfig(const char *filename)
{
   if(driver->userdb_setconfig)
      driver->userdb_setconfig(filename);
}

int var_set(int user, char *name, char *val)
{
   return driver->var_set(user, name, val);
}

int var_get(int user, char *name, char *val, int len)
{
   return driver->var_get(user, name, val, len);
}

int var_get_def(int user, char *name, char *val, int len, char *def)
{
   return driver->var_get_def(user, name, val, len, def);
}

int var_set_unique(int user, char *val)
{
   return driver->var_set_unique(user, val);
}

void var_setconfig(const char *filename)
{
   if(driver->var_setconfig)
      driver->var_setconfig(filename);
}

int var_deluser(int user)
{
   return driver->var_deluser(user);
}

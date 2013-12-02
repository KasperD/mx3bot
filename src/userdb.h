/*          
 * src/userdb.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _userdb_h
#define _userdb_h

int userdb_add(char *name, char *mask);
int userdb_del(int uid);
int userdb_find_byname(char *name);
int userdb_find_bymask(char *ident);
int userdb_find_bymask3(char *nick, char *user, char *host);
int userdb_set_name(int uid, char *name);
char *userdb_get_name(int uid);
int userdb_set_password(int uid, char *pass);
int userdb_check_password(int uid, char *pass);
int userdb_has_password(int uid);
int userdb_addmask(int uid, char *mask);
int userdb_delmask(int uid, char *mask);
char *userdb_getmasklist(int uid);
char *userdb_getmask(int mid);
int userdb_count(void);
void userdb_setconfig(const char *filename);

#endif


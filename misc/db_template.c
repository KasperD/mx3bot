/* DBTYPE prototypes */

char* DBTYPE_ban_get_list(char *network, char *chan, char *ident);
int DBTYPE_ban_get_id(char *network, char *chan, char *ident);
int DBTYPE_ban_get_id_bymask(char *network, char *chan, char *mask);
int DBTYPE_ban_add(char *network, char *chan, char *mask, int uid, char *note);
int DBTYPE_ban_del(int bid);
int DBTYPE_ban_get(int bid, int *uid, char *mask, int mask_len, char *note, int note_len);
void DBTYPE_ban_setconfig(const char *filename);
int DBTYPE_ban_deluser(int uid);

void DBTYPE_mode_set(int user, char *net, char *chan, char *mode, char *p);
void DBTYPE_mode_get(int user, char *net, char *chan, char *mode);
int DBTYPE_mode_check(int user, char *net, char *chan, int mode, char *param, int len);
char* DBTYPE_mode_get_chan_list(char *net, int mode);
void DBTYPE_mode_setconfig(const char *filename);
int DBTYPE_mode_deluser(int user);

char* DBTYPE_note_get_list(int to, int from);
int DBTYPE_note_get(int nid, int *to, int *from, int *sent, char *note, int len);
int DBTYPE_note_del(int nid);
int DBTYPE_note_add(int to, int from, int sent, char *note);
void DBTYPE_note_setconfig(const char *filename);
int DBTYPE_note_deluser(int uid);

int DBTYPE_setup_database(const char *dir, const char *host, const char *user, const char *pass, const char *name);

int DBTYPE_userdb_add(char *name, char *mask, int level);
int DBTYPE_userdb_del(int uid);
int DBTYPE_userdb_find_byname(char *name);
int DBTYPE_userdb_find_bymask(char *ident);
int DBTYPE_userdb_find_bymask3(char *nick, char *user, char *host);
int DBTYPE_userdb_set_name(int uid, char *name);
char* DBTYPE_userdb_get_name(int uid);
int DBTYPE_userdb_set_level(int uid, int level);
int DBTYPE_userdb_get_level(int uid);
int DBTYPE_userdb_set_password(int uid, char *pass);
int DBTYPE_userdb_check_password(int uid, char *pass);
int DBTYPE_userdb_has_password(int uid);
int DBTYPE_userdb_addmask(int uid, char *mask);
int DBTYPE_userdb_delmask(int uid, char *mask);
char* DBTYPE_userdb_getmasklist(int uid);
char* DBTYPE_userdb_getmask(int mid);
int DBTYPE_userdb_count(int level);
void DBTYPE_userdb_setconfig(const char *filename);

int DBTYPE_var_set(int user, char *name, char *val);
int DBTYPE_var_get(int user, char *name, char *val, int len);
int DBTYPE_var_get_def(int user, char *name, char *val, int len, char *def);
int DBTYPE_var_set_unique(int user, char *val);
void DBTYPE_var_setconfig(const char *filename);
int DBTYPE_var_deluser(int user);

/* DBTYPE driver */

static struct db_driver DBTYPE =
{
   DBTYPE_ban_get_list,
   DBTYPE_ban_get_id,
   DBTYPE_ban_get_id_bymask,
   DBTYPE_ban_add,
   DBTYPE_ban_del,
   DBTYPE_ban_get,
   DBTYPE_ban_setconfig,
   DBTYPE_ban_deluser,

   DBTYPE_mode_set,
   DBTYPE_mode_get,
   DBTYPE_mode_check,
   DBTYPE_mode_get_chan_list,
   DBTYPE_mode_setconfig,
   DBTYPE_mode_deluser,

   DBTYPE_note_get_list,
   DBTYPE_note_get,
   DBTYPE_note_del,
   DBTYPE_note_add,
   DBTYPE_note_setconfig,
   DBTYPE_note_deluser,

   DBTYPE_setup_database,

   DBTYPE_userdb_add,
   DBTYPE_userdb_del,
   DBTYPE_userdb_find_byname,
   DBTYPE_userdb_find_bymask,
   DBTYPE_userdb_find_bymask3,
   DBTYPE_userdb_set_name,
   DBTYPE_userdb_get_name,
   DBTYPE_userdb_set_level,
   DBTYPE_userdb_get_level,
   DBTYPE_userdb_set_password,
   DBTYPE_userdb_check_password,
   DBTYPE_userdb_has_password,
   DBTYPE_userdb_addmask,
   DBTYPE_userdb_delmask,
   DBTYPE_userdb_getmasklist,
   DBTYPE_userdb_getmask,
   DBTYPE_userdb_count,
   DBTYPE_userdb_setconfig,

   DBTYPE_var_set,
   DBTYPE_var_get,
   DBTYPE_var_get_def,
   DBTYPE_var_set_unique,
   DBTYPE_var_setconfig,
   DBTYPE_var_deluser
};

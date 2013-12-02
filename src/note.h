/*          
 * src/note.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _note_h
#define _note_h

char *note_get_list(int to, int from);
int note_get(int nid, int *to, int *from, int *sent, char *note, int len);
int note_del(int nid);
int note_add(int to, int from, int sent, char *note);
void note_setconfig(const char *filename);
int note_deluser(int uid);

#endif


/*          
 * src/script.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _script_h
#define _script_h

int script_init(char *path);
int script_exec(int server, char *name, char *reply, ...);

#endif


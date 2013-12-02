/*          
 * src/irc/irc_queue.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _irc_queue_h
#define _irc_queue_h

int _irc_queue_init(int queue_size);
int _irc_queue_update(void);
int _irc_queue_insert(int priority, char *format, ...);
void _irc_queue_purge(void);

#endif

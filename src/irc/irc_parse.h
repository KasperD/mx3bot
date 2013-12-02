/*          
 * src/irc/irc_parse.h
 *          
 * Copyright 2001 Colin O'Leary
 *          
 */         
            
#ifndef _irc_parse_h
#define _irc_parse_h

struct irc_line
{
   char server[128];
   char nick[32];
   char user[32];
   char host[128];
   char ident[128];
   char command[128];
   char target[32];
   char param[512];
   char param2[512];
};

int _irc_parseline(char *line, struct irc_line *info);

#endif

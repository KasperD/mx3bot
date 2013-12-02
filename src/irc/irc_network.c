/*          
 * src/irc/irc_network.c
 *          
 * Copyright 2001 Colin O'Leary
 *          
 * Routines to handle socket network access.
 *
 */         
            
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "irc_network.h"

#define IRC_SERVER_CONNECTED	1
#define IRC_SERVER_DISCONNECT	2

#define BUFFER_SIZE		16384

static int sock, status;

int _irc_net_send(char *text)
{
   int sent;

   if(status == IRC_SERVER_DISCONNECT)
      return -1;

   sent = send(sock, text, strlen(text), 0);

   if(sent == -1)
      return -1;

   sent = send(sock, "\r\n", 2, 0);

   if(sent == -1)
      return -1;

   return 0;
}

int _irc_net_connect(char *addr, int port)
{
   struct sockaddr_in ircd;
   struct hostent *host;

   if(status == IRC_SERVER_CONNECTED)
      _irc_net_disconnect();

   sock = socket(AF_INET, SOCK_STREAM, 0);
   if(sock < 0)
      return -1;

   host = gethostbyname(addr);
   if(host == NULL)
      return -1;

   memset(&ircd, 0, sizeof(struct sockaddr_in));

   ircd.sin_family = AF_INET;
   ircd.sin_port = htons(port);
   ircd.sin_addr = *((struct in_addr *)host->h_addr);

   if(connect(sock, (struct sockaddr *)&ircd, 
      sizeof(struct sockaddr_in)) == -1)
      return -1;

   fcntl(sock, F_SETFL, O_NONBLOCK);

   return 0;
}

int _irc_net_disconnect(void)
{
   shutdown(sock, SHUT_RDWR);
   close(sock);

   return 0;
}

int _irc_net_readstatus(int delay)
{
   struct timeval tv;
   fd_set fds;
   int r;

   tv.tv_sec = delay / 1000;
   tv.tv_usec = (delay % 1000) * 1000;

   FD_ZERO(&fds);
   FD_SET(sock, &fds);

   do
   {
      r = select(sock + 1, &fds, NULL, NULL, &tv);
   }
   while((r == -1) && (errno == EINTR));

   if(FD_ISSET(sock, &fds))
      return 1;

   return 0;
}

int _irc_net_readline(char *buf, int maxlen, int delay)
{
   char buffer[1024];
   int len, i, ch;

   if(_irc_net_readstatus(delay) == 0)
      return 0;

   i = 0;
   ch = 0;

   while(ch != -1)
   {
      len = recv(sock, &ch, 1, 0);

      if(len < 0)
      {
         if(errno == EAGAIN)
            while(_irc_net_readstatus(delay) == 0)
               ;
         else
            return -1;
      }

      if(ch == '\r')
         continue;

      buffer[i] = ch;

      if(ch == '\n')
      {
         buffer[i] = 0;
         ch = -1;
      }
      else
         i++;
   }

   strncpy(buf, buffer, maxlen);

   return i;
}

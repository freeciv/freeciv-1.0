/********************************************************************** 
 Freeciv - Copyright (C) 1996 - A Kjeldberg, L Gregersen, P Unold
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/uio.h>
#include <sys/time.h>

#include <pwd.h>
#include <string.h>
#include <errno.h>

#ifdef AIX
#include <sys/select.h>
#endif

#include <signal.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "civclient.h"
#include "clinet.h"
#include "log.h"
#include "chatline.h"
#include "packets.h"
#include "xmain.h"
#include "chatline.h"
#include "game.h"

struct connection aconnection;
extern int errno;

/**************************************************************************
...
**************************************************************************/
int connect_to_server(char *hostname, int port)
{
  /* use name to find TCPIP address of server */
  struct sockaddr_in src;
  struct hostent *ph;
  long address;

  if(port==0)
    port=DEFAULT_SOCK_PORT;

  if(isdigit(*hostname)) {
    if((address = inet_addr(hostname)) == -1) {
      log(LOG_FATAL, "invalid host name");
      return -1;
    }
    src.sin_addr.s_addr = address;
    src.sin_family = AF_INET;
  }
  else if ((ph = gethostbyname(hostname)) == NULL) {
    log(LOG_FATAL, "invalid host name");
    return -1;
  }
  else {
    src.sin_family = ph->h_addrtype;
    memcpy((char *) &src.sin_addr, ph->h_addr, ph->h_length);
  }
  
  src.sin_port = htons(port);
  
  /* ignore broken pipes */
  signal (SIGPIPE, SIG_IGN);
  
  if((aconnection.sock = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
    log(LOG_FATAL, "socket failed: %s", strerror(errno));
    return -1;
  }
  
  if(connect(aconnection.sock, (struct sockaddr *) &src, sizeof (src)) < 0) {
    log(LOG_FATAL, "connect failed: %s", strerror(errno));
    return -1;
  }

  aconnection.buffer.ndata=0;

  return 0;

}



/**************************************************************************
...
**************************************************************************/
void get_net_input(XtPointer client_data, int *fid, XtInputId *id)
{
  if(read_socket_data(*fid, &aconnection.buffer)>0) {
    int type;
    char *packet;

    while((packet=get_packet_from_connection(&aconnection, &type))) {
      handle_packet_input(packet, type);
    }
  }
  else {
    char buf[512];
    
    sprintf(buf, "Lost connection to server! Quit and reconnect with\n\
the option \"-n %s\", when ready..", game.player_ptr->name);
    
    append_output_window(buf);
    
    log(LOG_NORMAL, "lost connection to server");
    close(*fid);
    remove_net_input();
  }
}





/**************************************************************************
...
**************************************************************************/
void close_server_connection(void)
{
  close(aconnection.sock);
}


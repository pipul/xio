/*
  Copyright (c) 2013-2014 Dong Fang. All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#ifndef _HPIO_XSOCK_
#define _HPIO_XSOCK_

#include <inttypes.h>
#include <xio/cplusplus_define.h>

char *xallocmsg(int size);
void xfreemsg(char *xmsg);
int xmsglen(char *xmsg);

#define XMSG_CMSGNUM      0
#define XMSG_GETCMSG      1
#define XMSG_SETCMSG      2
#define XMSG_CLONE        3
#define XMSG_COPYCMSG     4
#define XMSG_SWITCHCMSG   5


struct xcmsg {
    uint8_t idx;
    char *outofband;
};    
    
int xmsgctl(char *xmsg, int opt, void *optval);

/* Following address family are provided */
#define XPF_TCP        1
#define XPF_IPC        2
#define XPF_INPROC     4

/* Following scalability address family are provided */
#define XPF_SP         8

/* Following socktype are provided */
#define XLISTENER       1
#define XCONNECTOR      2

/* Following scalability socktype are provided */
#define XSP_LISTENER    3
#define XSP_ENDPOINT    4

int xsocket(int pf, int socktype);
int xbind(int fd, const char *addr);


#define XSOCKADDRLEN 128
int xlisten(const char *addr);
int xaccept(int fd);
int xconnect(const char *peer);

int xrecv(int fd, char **xmsg);
int xsend(int fd, char *xmsg);
int xclose(int fd);

/* Following sockopt-level are provided */
#define XL_SOCKET          1

/* Following sockopt-field are provided */
#define XNOBLOCK           0
#define XSNDWIN            1
#define XRCVWIN            2
#define XSNDBUF            3
#define XRCVBUF            4
#define XLINGER            5
#define XSNDTIMEO          6
#define XRCVTIMEO          7
#define XRECONNECT         8
#define XSOCKTYPE          9
#define XSOCKPROTO         10
#define XTRACEDEBUG        11
    
int xsetopt(int fd, int level, int opt, void *optval, int optlen);
int xgetopt(int fd, int level, int opt, void *optval, int *optlen);

#include <xio/cplusplus_endif.h>
#endif

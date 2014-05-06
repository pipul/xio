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

#ifndef _HPIO_POLL_
#define _HPIO_POLL_

#include <xio/cplusplus_define.h>

#define XPOLLIN   1
#define XPOLLOUT  2
#define XPOLLERR  4

int xselect(int events, int nin, int *in_set, int nout, int *out_set);

struct xpoll_event {
    int xd;
    void *self;
    /* What events i care about ... */
    int care;

    /* What events happened now */
    int happened;
};

struct xpoll_t;

struct xpoll_t *xpoll_create();
void xpoll_close(struct xpoll_t *po);

#define XPOLL_ADD 1
#define XPOLL_DEL 2
#define XPOLL_MOD 3
int xpoll_ctl(struct xpoll_t *xp, int op, struct xpoll_event *ue);

int xpoll_wait(struct xpoll_t *xp, struct xpoll_event *events, int n,
	       int timeout);

#include <xio/cplusplus_endif.h>
#endif

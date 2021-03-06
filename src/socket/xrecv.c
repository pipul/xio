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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <utils/waitgroup.h>
#include <utils/taskpool.h>
#include "global.h"

struct msgbuf *recvq_rm (struct sockbase *sb) {
	int rc;
	struct msgbuf *msg = 0;
	struct sockbase_vfptr *vfptr = sb->vfptr;
	u32 events = 0;

	mutex_lock (&sb->lock);
	while (!sb->fepipe && msgbuf_head_empty (&sb->rcv) && !sb->fasync) {
		sb->rcv.waiters++;
		condition_wait (&sb->cond, &sb->lock);
		sb->rcv.waiters--;
	}
	if ((rc = msgbuf_head_out_msg (&sb->rcv, &msg)) == 0) {
		events |= XMQ_POP;

		/* the first time when msgbuf_head is non-full */
		if (sb->rcv.wnd - sb->rcv.size <= msgbuf_len (msg))
			events |= XMQ_NONFULL;
		if (msgbuf_head_empty (&sb->rcv)) {
			BUG_ON (sb->rcv.size);
			events |= XMQ_EMPTY;
		}
	}

	if (events && vfptr->notify)
		vfptr->notify (sb, RECV_Q, events);

	__emit_pollevents (sb);
	mutex_unlock (&sb->lock);
	return msg;
}

int recvq_add (struct sockbase *sb, struct msgbuf *msg)
{
	struct sockbase_vfptr *vfptr = sb->vfptr;
	u32 events = 0;

	mutex_lock (&sb->lock);
	events |= XMQ_PUSH;

	/* the first time when msgbuf_head is non-empty */
	if (msgbuf_head_empty (&sb->rcv))
		events |= XMQ_NONEMPTY;
	msgbuf_head_in_msg (&sb->rcv, msg);
	if (!msgbuf_can_in (&sb->rcv))
		events |= XMQ_FULL;

	/* Wakeup the blocking waiters. */
	if (sb->rcv.waiters > 0)
		condition_broadcast (&sb->cond);

	if (events && vfptr->notify)
		vfptr->notify (sb, RECV_Q, events);

	__emit_pollevents (sb);
	mutex_unlock (&sb->lock);
	return 0;
}

int xrecv (int fd, char **ubuf)
{
	int rc = 0;
	struct sockbase *sb;
	struct msgbuf *msg = 0;

	if (!ubuf) {
		errno = EINVAL;
		return -1;
	}
	if (! (sb = xget (fd) ) ) {
		errno = EBADF;
		return -1;
	}
	if (! (msg = recvq_rm (sb) ) ) {
		errno = sb->fepipe ? EPIPE : EAGAIN;
		rc = -1;
	} else {
		*ubuf = msg->chunk.ubuf_base;
	}
	xput (fd);
	return rc;
}




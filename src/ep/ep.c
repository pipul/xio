#include <os/alloc.h>
#include <x/xsock.h>
#include <os/timesz.h>
#include <hash/crc.h>
#include "pxy.h"
#include "ep.h"

#define DEFAULT_GROUP "default"


struct ep *ep_new(int ty) {
    struct ep *ep = (struct ep *)mem_zalloc(sizeof(*ep));
    if (ep) {
	pxy_init(&ep->y);
	ep->type = ty;
    }
    return ep;
}

void ep_close(struct ep *ep) {
    pxy_destroy(&ep->y);
    mem_free(ep, sizeof(*ep));
}

extern int __pxy_connect(struct pxy *y, int ty, u32 ev, const char *url);

int ep_connect(struct ep *ep, const char *url) {
    return __pxy_connect(&ep->y, ep->type, XPOLLERR|XPOLLIN, url);
}


/* Comsumer endpoint api : recv_req and send_resp */
int ep_recv_req(struct ep *ep, char **req, char **r) {
    int n;
    struct pxy *y = &ep->y;
    struct fd *f;
    struct ep_hdr *h;
    struct xpoll_event ev = {};

    if ((n = xpoll_wait(y->po, &ev, 1, 0x7fff)) < 0)
	return -1;
    DEBUG_ON("%s", xpoll_str[ev.happened]);
    BUG_ON(ev.happened & XPOLLOUT);
    if (!(ev.happened & XPOLLIN))
	goto AGAIN;

    f = (struct fd *)ev.self;
    if (ep_recv(f->xd, &h) == 0) {
	/* Drop the timeout message */
	if (ep_hdr_timeout(h) < 0) {
	    DEBUG_ON("message is timeout");
	    ep_freehdr(h);
	    goto AGAIN;
	}
	/* If message has invalid header checkusm. return error */
	if (ep_hdr_validate(h) < 0) {
	    DEBUG_ON("invalid message's checksum");
	    ep_freehdr(h);
	    f->fok = false;
	    goto AGAIN;
	}
	if (!(*req = xallocmsg(h->size))) {
	    ep_freehdr(h);
	    goto AGAIN;
	}
	if (!(*r = xallocmsg(hdr_size(h)))) {
	    ep_freehdr(h);
	    xfreemsg(*req);
	    goto AGAIN;
	}
	/* Copy req into user-space */
	memcpy(*req, (char *)h + hdr_size(h), h->size);
	memcpy(*r, h, hdr_size(h));

	/* Payload was copy into user-space. */
	xfreemsg((char *)h);

	DEBUG_ON("channel %d recv req from network", f->xd);
	return 0;
    } else if (errno != EAGAIN) {
	DEBUG_ON("channel %d on bad status", f->xd);
	f->fok = false;
    }
    /* TODO: cleanup the bad status fd here */
 AGAIN:
    errno = EAGAIN;
    return -1;
}

int ep_send_resp(struct ep *ep, char *resp, char *r) {
    int rc;
    struct pxy *y = &ep->y;
    struct fd *f;
    struct ep_hdr *h;
    struct ep_rt *cr;

    if (!(h = ep_mergehdr(r, resp)))
	return -1;

    /* Copy header */
    h->end_ttl = h->ttl;
    h->go = false;
    h->size = xmsglen(resp);

    xfreemsg(r);
    xfreemsg(resp);

    /* Update header checksum */
    ep_hdr_gensum(h);
    cr = rt_cur(h);
    BUG_ON(!(f = rtb_route_back(&y->tb, cr->uuid)));
    DEBUG_OFF("channel %d send resp into network", f->xd);
    rc = ep_send(f->xd, h);
    return rc;
}


/* Producer endpoint api : send_req and recv_resp */
int ep_send_req(struct ep *ep, char *req) {
    int rc;
    struct pxy *y = &ep->y;
    struct ep_hdr *h;
    struct ep_rt *cr;
    struct fd *f;
    u32 hdr_sz = sizeof(*h) + sizeof(*cr);

    if (!(h = ep_allochdr(xmsglen(req) + hdr_sz)))
	return -1;

    /* Append ep_hdr and route. The proxy package frame header */
    h->version = 0;
    h->ttl = 1;
    h->end_ttl = 0;
    h->go = true;
    h->size = xmsglen(req);
    h->timeout = 0;
    h->checksum = 0;
    h->sendstamp = rt_mstime();

    memcpy((char *)h + hdr_size(h), req, xmsglen(req));
    xfreemsg(req);

    /* Update header checksum */
    ep_hdr_gensum(h);

    /* RoundRobin algo select a struct fd */
    BUG_ON(!(f = rtb_rrbin_go(&y->tb)));
    cr = rt_cur(h);
    uuid_copy(cr->uuid, f->st.ud);
    rc = ep_send(f->xd, h);
    DEBUG_OFF("channel %d send req into network", f->xd);
    return rc;
}


int ep_recv_resp(struct ep *ep, char **resp) {
    int n;
    struct pxy *y = &ep->y;
    struct fd *f;
    struct ep_hdr *h;
    struct xpoll_event ev = {};

    if ((n = xpoll_wait(y->po, &ev, 1, 0x7fff)) < 0)
	return -1;
    f = (struct fd *)ev.self;

    DEBUG_ON("%s", xpoll_str[ev.happened]);
    if (!(ev.happened & XPOLLIN)) {
	errno = EAGAIN;
	return -1;
    }
    if (ep_recv(f->xd, &h) == 0) {
	if (!(*resp = xallocmsg(h->size))) {
	    xfreemsg((char *)h);
	    errno = EAGAIN;
	    return -1;
	}

	/* Copy response into user-space */
	memcpy(*resp, (char *)h + hdr_size(h), h->size);

	/* Payload was copy into user-space. */
	xfreemsg((char *)h);
	return 0;
    } else if (errno != EAGAIN) {
	DEBUG_ON("channel %d on bad status", f->xd);
	f->fok = false;
    }
    return -1;
}


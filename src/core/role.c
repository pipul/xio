#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "role.h"
#include "grp.h"
#include "accepter.h"

static int r_rgs(struct role *r, uint32_t happened);
static int r_recv(struct role *r);
static int r_send(struct role *r);
static int r_error(struct role *r);

int r_event_handler(epoll_t *el, epollevent_t *et, uint32_t happened) {
    struct role *r = container_of(et, struct role, et);
    
    if (!(r->status & ST_REGISTED))
	r_rgs(r, happened);
    else {
	if ((r->status & ST_OK) && (happened & EPOLLIN))
	    r_recv(r);
	if ((r->status & ST_OK) && (happened & EPOLLOUT))
	    r_send(r);
    }
    if (!(r->status & ST_OK))
	r_error(r);
    return 0;
}

static int r_rgs(struct role *r, uint32_t happened) {
    int ret;
    struct pio_rgh *h = &r->pp.rgh;
    grp_t *grp;
    acp_t *acp = container_of(r->el, struct accepter, el);

    if ((ret = (r->type & ST_REGISTER) ? pp_recv_rgh(&r->pp, &r->conn_ops) :
	 pp_send_rgh_async(&r->pp, &r->conn_ops)) < 0 && errno == EAGAIN)
	return -1;
    if (ret < 0 || !(grp = acp_find(acp, h->grpname))) {
	r->status &= ~ST_OK;
	return -1;
    }
    r->status |= ST_REGISTED;
    r->type = h->type;
    uuid_copy(r->uuid, h->id);
    r->grp = grp;
    grp_add(grp, r);
    return 0;
}

static int r_receiver_recv(struct role *r) {
    int ret;
    int64_t now = rt_mstime();
    pio_msg_t *msg = NULL;
    struct role *dest;
    struct pio_rt *crt;

    if ((ret = pp_recv(&r->pp, &r->conn_ops, &msg, PIORTLEN)) == 0) {
	crt = pio_msg_currt(msg);
	crt->cost = (uint16_t)(now - msg->hdr.sendstamp - crt->begin);
	if ((dest = grp_loadbalance_dispatch(r->grp)))
	    r_push(dest, msg);
	else
	    pio_msg_free(msg);
    } else if (ret < 0 && errno != EAGAIN)
	r->status &= ~ST_OK;
    return 0;
}

static int r_dispatcher_recv(struct role *r) {
    int ret;
    pio_msg_t *msg = NULL;
    struct role *src;
    struct pio_rt *crt;

    if ((ret = pp_recv(&r->pp, &r->conn_ops, &msg, 0)) == 0) {
	pio_msg_shrinkrt(msg);
	crt = pio_msg_currt(msg);
	if ((src = grp_find_at(r->grp, crt->uuid)))
	    r_push(src, msg);
	else
	    pio_msg_free(msg);
    } else if (ret < 0 && errno != EAGAIN)
	r->status &= ~ST_OK;
    return 0;
}

static int r_recv(struct role *r) {
    if (IS_RCVER(r))
	return r_receiver_recv(r);
    return r_dispatcher_recv(r);
}

static int r_receiver_send(struct role *r) {
    pio_msg_t *msg;

    if (!(msg = r_pop(r)))
	return -1;
    if (pp_send(&r->pp, &r->conn_ops, msg) < 0)
	r->status &= ~ST_OK;
    pio_msg_free(msg);
    return 0;
}

static int r_dispatcher_send(struct role *r) {
    pio_msg_t *msg;
    int64_t now = rt_mstime();
    struct pio_rt rt = {}, *crt;
    
    if (!(msg = r_pop(r)))
	return -1;
    crt = pio_msg_currt(msg);
    uuid_copy(rt.uuid, r->uuid);
    rt.begin = (uint32_t)(now - msg->hdr.sendstamp);
    crt->stay = (uint16_t)(rt.begin - crt->begin - crt->cost);
    pio_msg_appendrt(msg, &rt);
    if (pp_send(&r->pp, &r->conn_ops, msg) < 0)
	r->status &= ~ST_OK;
    pio_msg_free(msg);
    return 0;
}

static int r_send(struct role *r) {
    if (IS_RCVER(r))
	return r_receiver_send(r);
    return r_dispatcher_send(r);
}


static int r_error(struct role *r) {
    epoll_del(r->el, &r->et);
    close(r->et.fd);
    grp_del(r->grp, r);
    r_put(r);
    return 0;
}

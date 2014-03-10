#ifndef _HPIO_ACCEPTER_
#define _HPIO_ACCEPTER_

#include "core.h"
#include "net/accepter.h"
#include "runner/taskpool.h"

int acp_event_handler(epoll_t *el, epollevent_t *et, uint32_t happened);


#define acp_init(acp, w) do {		\
	epollevent_t *__et = &(acp)->et;	\
	spin_init(&(acp)->lock);		\
	epoll_init(&(acp)->el, 1024, 100, 1);	\
	taskpool_init(&(acp)->tp, w);		\
	__et->f = acp_event_handler;	\
	__et->data = (acp);			\
	INIT_LIST_HEAD(&(acp)->grp_head);	\
    } while (0)

#define acp_destroy(acp) do {		\
	epoll_destroy(&(acp)->el);		\
	spin_destroy(&(acp)->lock);		\
	taskpool_destroy(&(acp)->tp);		\
    } while (0)

static inline grp_t *acp_find(acp_t *acp, char grpname[GRPNAME_MAX]) {
    grp_t *cur, *match = NULL;

    list_for_each_entry(cur, &acp->grp_head, grp_t, acp_link)
	if (memcmp(grpname, cur->grpname, GRPNAME_MAX) == 0) {
	    match = cur;
	    break;
	}
    return match;
}

static inline int acp_worker(void *args) {
    acp_t *acp = (acp_t *)args;
    return epoll_startloop(&acp->el);
}

static inline void acp_start(acp_t *acp) {
    taskpool_start(&acp->tp);
    taskpool_run(&acp->tp, acp_worker, acp);
}

static inline void acp_stop(acp_t *acp) {
    epoll_stoploop(&acp->el);
    taskpool_stop(&acp->tp);
}

static inline int acp_listen(acp_t *acp, const char *addr) {
    if (acp->et.fd > 0 || (acp->et.fd = act_listen("tcp", addr, 1024)) < 0)
	return -1;
    acp->et.events = EPOLLIN;
    return epoll_add(&acp->el, &acp->et);
}

static inline int acp_proxyto(acp_t *acp, char *grpname,
				   const char *addr) {
    int nfd;
    pio_rgh_t *h;
    struct role *r = r_new();
    
    if (!r || (nfd = sk_connect("tcp", "", addr)) < 0)
	return -1;
    r_init(r);
    h = &r->pp.rgh;
    h->type = PIO_SNDER;
    uuid_generate(h->id);
    strcpy(h->grpname, grpname);
    r->el = &acp->el;
    r->et.fd = nfd;
    r->et.events = EPOLLOUT;
    return epoll_add(&acp->el, &r->et);
}


#endif
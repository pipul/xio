#include <stdio.h>
#include <uuid/uuid.h>
#include <unistd.h>
#include "errno.h"
#include "proxyio.h"
#include "net/socket.h"

comsumer_t *comsumer_new(const char *addr, const char py[PROXYNAME_MAX]) {
    proxyio_t *io = proxyio_new();

    proxyio_init(io);
    if ((io->sockfd = sk_connect("tcp", "", addr)) < 0)
	goto RGS_ERROR;
    sk_setopt(io->sockfd, SK_NONBLOCK, true);
    io->rgh.type = PIO_SNDER;
    uuid_generate(io->rgh.id);
    memcpy(io->rgh.proxyname, py, sizeof(py));
    if (proxyio_at_rgs(io) < 0 && errno != EAGAIN)
	goto RGS_ERROR;
    while (!bio_empty(&io->out))
	if (bio_flush(&io->out, &io->sock_ops) < 0)
	    goto RGS_ERROR;
    return &io->sockfd;
 RGS_ERROR:
    proxyio_destroy(io);
    mem_free(io, sizeof(*io));
    return NULL;
}


void comsumer_destroy(comsumer_t *pp) {
    proxyio_t *io = container_of(pp, proxyio_t, sockfd);
    close(io->sockfd);
    proxyio_destroy(io);
    mem_free(io, sizeof(*io));
}



int comsumer_send_response(comsumer_t *pp, const char *data, int64_t size,
			   const char *urt, int64_t rt_size) {
    proxyio_t *io = container_of(pp, proxyio_t, sockfd);
    struct pio_hdr *h = (struct pio_hdr *)urt;

    h->go = 0;
    h->size = size;
    h->ttl = h->end_ttl = (rt_size - sizeof(*h)) / PIORTLEN;
    ph_makechksum(h);
    proxyio_bwrite(io, h, data, urt + sizeof(*h));
    if (proxyio_flush(io) < 0 && errno != EAGAIN)
	return -1;
    return 0;
}


int comsumer_psend_response(comsumer_t *pp, const char *data, int64_t size,
			    const char *urt, int64_t rt_size) {
    proxyio_t *io = container_of(pp, proxyio_t, sockfd);
    if (comsumer_send_response(pp, data, size, urt, rt_size) < 0)
	return -1;
    while (proxyio_flush(io) < 0)
	if (errno != EAGAIN)
	    return -1;
    return 0;
}




int comsumer_recv_request(comsumer_t *pp, char **data, int64_t *size,
			  char **rt, int64_t *rt_size) {
    char *urt;
    struct pio_hdr h = {};
    proxyio_t *io = container_of(pp, proxyio_t, sockfd);

    if ((proxyio_prefetch(io) < 0 && errno != EAGAIN)
	|| (proxyio_bread(io, &h, data, rt) < 0))
	return -1;
    if (!ph_validate(&h)) {
	mem_free(data, h.size);
	mem_free(rt, pio_rt_size(&h));
	errno = PIO_ECHKSUM;
	return -1;
    }
    if (!(urt = mem_realloc(*rt, pio_rt_size(&h) + sizeof(h)))) {
	mem_free(*data, h.size);
	mem_free(*rt, pio_rt_size(&h));
	return -1;
    }
    *size = h.size;
    *rt = urt;
    *rt_size = pio_rt_size(&h) + sizeof(h);
    memmove((*rt) + sizeof(h), *rt, pio_rt_size(&h));
    memcpy(*rt, (char *)&h, sizeof(h));
    return 0;
}

int comsumer_precv_request(comsumer_t *pp, char **data, int64_t *size,
			   char **rt, int64_t *rt_size) {
    int ret;
    while ((ret = comsumer_recv_request(pp, data, size, rt, rt_size)) < 0
	   && errno == EAGAIN) {
    }
    return ret;
}


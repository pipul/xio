#include <stdio.h>
#include <uuid/uuid.h>
#include <unistd.h>
#include "io.h"
#include "core/proto_parser.h"
#include "net/socket.h"

int comsumer_send_response(pio_t *io, const char *data, uint32_t size,
			   const char *urt, uint32_t rt_size) {
    int64_t now = rt_mstime();
    proto_parser_t *pp = container_of(io, proto_parser_t, sockfd);
    struct pio_rt *crt;
    struct pio_hdr *h = (struct pio_hdr *)urt;

    if (!ph_validate(h))
	return -1;
    h->go = 0;
    h->size = size;
    h->end_ttl = h->ttl;
    crt = &((struct pio_rt *)(urt + PIOHDRLEN))[h->ttl - 1];
    crt->begin[1] = (uint16_t)(now - h->sendstamp);
    crt->stay[0] = (uint16_t)(now - h->sendstamp - crt->begin[0] - crt->cost[0]);
    ph_makechksum(h);
    proto_parser_bwrite(pp, h, data, urt + PIOHDRLEN);
    modstat_update_timestamp(proto_parser_stat(pp), now);
    if (proto_parser_flush(pp) < 0 && errno != EAGAIN)
	return -1;
    return 0;
}


int comsumer_psend_response(pio_t *io, const char *data, uint32_t size,
			    const char *urt, uint32_t rt_size) {
    proto_parser_t *pp = container_of(io, proto_parser_t, sockfd);
    if (comsumer_send_response(io, data, size, urt, rt_size) < 0)
	return -1;
    while (proto_parser_flush(pp) < 0)
	if (errno != EAGAIN)
	    return -1;
    return 0;
}




int comsumer_recv_request(pio_t *io, char **data, uint32_t *size,
			  char **rt, uint32_t *rt_size) {
    int64_t now = rt_mstime();
    char *urt;
    struct pio_rt *crt;
    struct pio_hdr h = {};
    proto_parser_t *pp = container_of(io, proto_parser_t, sockfd);

    if ((proto_parser_prefetch(pp) < 0 && errno != EAGAIN) || (proto_parser_bread(pp, &h, data, rt) < 0))
	return -1;
    if (!ph_validate(&h)) {
	mem_free(*data, h.size);
	mem_free(*rt, pio_rt_size(&h));
	return -1;
    }
    if (!(urt = mem_realloc(*rt, pio_rt_size(&h) + PIOHDRLEN))) {
	mem_free(*data, h.size);
	mem_free(*rt, pio_rt_size(&h));
	return -1;
    }
    crt = &((struct pio_rt *)urt)[h.ttl - 1];
    crt->cost[0] = (uint16_t)(now - h.sendstamp - crt->begin[0]);
    *size = h.size;
    *rt = urt;
    *rt_size = pio_rt_size(&h) + PIOHDRLEN;
    memmove((*rt) + PIOHDRLEN, *rt, pio_rt_size(&h));
    memcpy(*rt, (char *)&h, PIOHDRLEN);
    modstat_incrskey(proto_parser_stat(pp), PP_RTT, now - h.sendstamp);
    modstat_update_timestamp(proto_parser_stat(pp), now);
    return 0;
}

int comsumer_precv_request(pio_t *pp, char **data, uint32_t *size,
			   char **rt, uint32_t *rt_size) {
    int ret;
    while ((ret = comsumer_recv_request(pp, data, size, rt, rt_size)) < 0
	   && errno == EAGAIN) {
    }
    return ret;
}


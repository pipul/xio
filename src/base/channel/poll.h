#ifndef _HPIO_CHANNEL_POLL_
#define _HPIO_CHANNEL_POLL_

#include <sync/mutex.h>
#include <sync/spin.h>
#include <sync/condition.h>

struct upoll_entry;
struct upoll_notify {
    void (*event) (struct upoll_notify *un, struct upoll_entry *ent);
};

#define UPOLLSTATE_NEW    0x01
#define UPOLLSTATE_DEL    0x02
#define UPOLLSTATE_CLOSED 0x04

struct upoll_entry {
    /* List item for linked other upoll_entry of the same channel */
    struct list_head channel_link;

    /* List item for linked other upoll_entry of the same poll_table */
    struct list_head lru_link;

    spin_t lock;

    /* This entry is normal Only when eflags == 0 && ref >= 2 */
    uint32_t eflags;
    /* Reference hold by channel/upoll_tb */
    int ref;

    /* Reference backtrace for debuging */
    int i_idx;
    int incr_tid[10];
    int d_idx;
    int desc_tid[10];
    
    /* struct upoll_event contain the care events and what happened */
    struct upoll_event event;
    struct upoll_notify *notify;
};

#define list_for_each_upoll_ent(pos, nx, head)				\
    list_for_each_entry_safe(pos, nx, head, struct upoll_entry, lru_link)

#define list_for_each_channel_ent(pos, nx, head)			\
    list_for_each_entry_safe(pos, nx, head, struct upoll_entry, channel_link)

struct upoll_entry *entry_new();
int entry_get(struct upoll_entry *ent);
int entry_put(struct upoll_entry *ent);

struct upoll_tb {
    struct upoll_notify notify;
    mutex_t lock;
    condition_t cond;

    /* Reference hold by upoll_entry */
    int ref;
    int uwaiters;
    int size;
    struct list_head lru_head;
};

struct upoll_tb *tb_new();
int tb_get(struct upoll_tb *ut);
int tb_put(struct upoll_tb *ut);
struct upoll_entry *tb_find(struct upoll_tb *tb, int cd);
struct upoll_entry *tb_popent(struct upoll_tb *tb);
struct upoll_entry *tb_getent(struct upoll_tb *tb, int cd);
struct upoll_entry *tb_putent(struct upoll_tb *tb, int cd);

void entry_attach_to_channel(struct upoll_entry *ent, int cd);
void entry_detach_from_channel(struct upoll_entry *ent, int cd);


#endif
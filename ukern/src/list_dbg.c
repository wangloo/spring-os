#include <list.h>
#include <kernel.h>


#ifdef CONFIG_DEBUG_LIST


void __list_add(struct list_head *new,
    struct list_head *prev,
    struct list_head *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void __list_del_entry(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
}

void list_del(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
    entry->next = LIST_POISON1;
    entry->prev = LIST_POISON2;
}


#endif
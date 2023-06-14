#ifndef __LIBMINOS_LIST_H__
#define __LIBMINOS_LIST_H__

struct list_head;

struct list_head{
	struct list_head *next, *pre;
};

#define LIST_HEAD(list)	\
struct list_head list = {	\
	.next = &list,	\
	.pre  = &list,	\
}

#define INIT_LIST_HEAD(list)		\
	do {				\
		list.next = &list;	\
		list.pre = &list;	\
	} while (0)

static void inline init_list(struct list_head *list)
{
	list->next = list;
	list->pre = list;
}

static void inline list_add(struct list_head *head,
			    struct list_head *new)
{
	head->next->pre = new;
	new->next = head->next;
	new->pre = head;
	head->next = new;
}

static void inline list_add_tail(struct list_head *head,
		struct list_head *new)
{
	head->pre->next = new;
	new->next = head;
	new->pre = head->pre;
	head->pre = new;
}

static void inline list_insert_before(struct list_head *head,
		struct list_head *new)
{
	new->pre = head->pre;
	new->next = head;
	head->pre->next = new;
	head->pre = new;
}

static void inline list_del(struct list_head *list)
{
	list->next->pre = list->pre;
	list->pre->next = list->next;
	list->next = (void *)0x0;
	list->pre = (void *)0x0;
}

static void inline list_del_tail(struct list_head *head)
{
	head->pre->pre->next = head;
	head->pre = head->pre->pre;
}

static int inline is_list_empty(struct list_head *head)
{
	return (head->next == head);
}

static int inline is_list_last(struct list_head *head,
		               struct list_head *list)
{
	return list->next == head;
}

static inline struct list_head *list_next(struct list_head *list)
{
	return list->next;
}

static inline struct list_head *list_prve(struct list_head *list)
{
	return list->pre;
}

#define list_entry(ptr, type, member)	\
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, __typeof__(*(pos)), member)

#define list_for_each(head, list)	\
	for(list = (head)->next; list != (head); list = list->next)

#define list_for_each_entry(pos, head, member)	\
	for (pos = list_entry((head)->next, __typeof__(*pos), member); \
	     &pos->member != (head); \
	     pos = list_entry(pos->member.next, __typeof__(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_first_entry(head, __typeof__(*pos), member),	\
		n = list_next_entry(pos, member);			\
	     &pos->member != (head); 					\
	     pos = n, n = list_next_entry(n, member))

#endif

#ifndef S3_LIB_LIST_H_
#define S3_LIB_LIST_H_

# include <stdlib.h>

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */
typedef struct S3ListHead S3List;
typedef struct S3ListHead S3ListHead;
struct S3ListHead {
  struct S3ListHead *next, *prev;
};

#define s3_list_inited(list) ((list) != NULL && (list)->next != NULL && (list)->prev != NULL)
#define S3_LIST_HEAD_INIT(name) { .next = &(name), .prev = &(name) }
#define s3_list_head_null {.next = NULL, .prev = NULL }

#define S3LISTHEAD(name) \
  struct S3ListHead name = S3_LIST_HEAD_INIT(name)

#define s3_list_head_init init_s3_list_head
#define s3_list_init init_s3_list_head
static inline void init_s3_list_head(struct S3ListHead *list) {
  list->next = list;
  list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
#ifndef CONFIG_DEBUG_LIST
static inline void __s3_list_add(struct S3ListHead *new,
                              struct S3ListHead *prev,
                              struct S3ListHead *next) {
  next->prev = new;
  new->next = next;
  new->prev = prev;
  prev->next = new;
}
#else
extern void __s3_list_add(struct S3ListHead *new,
                       struct S3ListHead *prev,
                       struct S3ListHead *next);
#endif

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
#define s3_list_add_head s3_list_add
static inline void s3_list_add(struct S3ListHead *new, struct S3ListHead *head) {
  __s3_list_add(new, head, head->next);
}


/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void s3_list_add_tail(struct S3ListHead *new, struct S3ListHead *head) {
  __s3_list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __s3_list_del(struct S3ListHead *prev, struct S3ListHead *next) {
  next->prev = prev;
  prev->next = next;
}

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
#ifndef CONFIG_DEBUG_LIST
static inline void s3_list_del(struct S3ListHead *entry) {
  __s3_list_del(entry->prev, entry->next);
  s3_list_init(entry);
}
#else
extern void s3_list_del(struct S3ListHead *entry);
#endif

/**
 * list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void s3_list_replace(struct S3ListHead *old,
                                struct S3ListHead *new) {
  new->next = old->next;
  new->next->prev = new;
  new->prev = old->prev;
  new->prev->next = new;
}

static inline void s3_list_replace_init(struct S3ListHead *old,
                                     struct S3ListHead *new) {
  s3_list_replace(old, new);
  init_s3_list_head(old);
}

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void s3_list_del_init(struct S3ListHead *entry) {
  __s3_list_del(entry->prev, entry->next);
  init_s3_list_head(entry);
}

/**
 * list_move - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void s3_list_move(struct S3ListHead *list, struct S3ListHead *head) {
  __s3_list_del(list->prev, list->next);
  s3_list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void s3_list_move_tail(struct S3ListHead *list,
                                  struct S3ListHead *head) {
  __s3_list_del(list->prev, list->next);
  s3_list_add_tail(list, head);
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int s3_list_is_last(const struct S3ListHead *list,
                               const struct S3ListHead *head) {
  return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int s3_list_empty(const struct S3ListHead *head) {
  return head->next == head;
}


/**
 * list_rotate_left - rotate the list to the left
 * @head: the head of the list
 */
static inline void s3_list_rotate_left(struct S3ListHead *head) {
  struct S3ListHead *first;

  if (!s3_list_empty(head)) {
    first = head->next;
    s3_list_move_tail(first, head);
  }
}

/**
 * list_is_singular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int s3_list_is_singular(const struct S3ListHead *head) {
  return !s3_list_empty(head) && (head->next == head->prev);
}

static inline void __s3_list_splice(const struct S3ListHead *list,
                                 struct S3ListHead *prev,
                                 struct S3ListHead *next) {
  struct S3ListHead *first = list->next;
  struct S3ListHead *last = list->prev;

  first->prev = prev;
  prev->next = first;

  last->next = next;
  next->prev = last;
}

/**
 * list_splice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void s3_list_splice(const struct S3ListHead *list,
                               struct S3ListHead *head) {
  if (!s3_list_empty(list)) {
    __s3_list_splice(list, head, head->next);
  }
}

/**
 * list_splice_tail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void s3_list_splice_tail(struct S3ListHead *list,
                                    struct S3ListHead *head) {
  if (!s3_list_empty(list)) {
    __s3_list_splice(list, head->prev, head);
  }
}

/**
 * list_splice_init - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void s3_list_splice_init(struct S3ListHead *list,
                                    struct S3ListHead *head) {
  if (!s3_list_empty(list)) {
    __s3_list_splice(list, head, head->next);
    init_s3_list_head(list);
  }
}

/**
 * list_splice_tail_init - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void s3_list_splice_tail_init(struct S3ListHead *list,
    struct S3ListHead *head) {
  if (!s3_list_empty(list)) {
    __s3_list_splice(list, head->prev, head);
    init_s3_list_head(list);
  }
}


// move list to new_list
static inline void s3_list_movelist(S3ListHead *list, S3ListHead *new_list) {
  if (!s3_list_empty(list)) {
    new_list->prev = list->prev;
    new_list->next = list->next;
    new_list->prev->next = new_list;
    new_list->next->prev = new_list;
    init_s3_list_head(list);
  } else {
    init_s3_list_head(new_list);
  }
}

// join list to head
static inline void s3_list_join(S3ListHead *list, S3ListHead *head) {
  if (!s3_list_empty(list)) {

    S3ListHead *first  = list->next;
    S3ListHead *last   = list->prev;
    S3ListHead *at     = head->prev;

    first->prev = at;
    at->next = first;
    last->next = head;
    head->prev = last;
  }
}

static inline int64_t s3_list_length(const S3ListHead *list) {
  int64_t ret = 0;
  if (NULL != list) {
    for (S3ListHead *pos = list->next; pos != list; pos = pos->next) {
      ++ret;
    }
  }
  return ret;
}

#define S3ListOffSetOf(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define S3ListContainerOf(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - S3ListOffSetOf(type,member));})

// get last
#define s3_list_get_last(ptr, type, member)                              \
    s3_list_empty(ptr) ? NULL : s3_list_entry((ptr)->prev, type, member)

// get first
#define s3_list_get_first(ptr, type, member)                             \
    s3_list_empty(ptr) ? NULL : s3_list_entry((ptr)->next, type, member)
/**
 * list_entry - get the struct for this entry
 * @ptr:  the &struct S3ListHead pointer.
 * @type: the type of the struct this is embedded in.
 * @member: the name of the list_struct within the struct.
 */
#define s3_list_entry(ptr, type, member) \
  S3ListContainerOf(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define s3_list_first_entry(ptr, type, member) \
	s3_list_entry((ptr)->next, type, member)

/**
 * list_first_entry_or_null - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define s3_list_first_entry_with_head(ptr, type, member) \
  (!s3_list_empty(ptr) ? s3_list_first_entry(ptr, type, member) : NULL)

/**
 * list_for_each  - iterate over a list
 * @pos:  the &struct S3ListHead to use as a loop cursor.
 * @head: the head for your list.
 */
#define s3_list_for_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop cursor.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define s3_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_prev - iterate over a list backwards
 * @pos:  the &struct S3ListHead to use as a loop cursor.
 * @head: the head for your list.
 */
#define s3_list_for_each_prev(pos, head) \
  for (pos = (head)->prev; pos != (head); pos = pos->prev)


/**
 * list_for_each_entry  - iterate over list of given type
 * @pos:  the type * to use as a loop cursor.
 * @head: the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define s3_list_for_each_entry(pos, head, member)        \
  for (pos = s3_list_entry((head)->next, typeof(*pos), member);  \
       &pos->member != (head);   \
       pos = s3_list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:  the type * to use as a loop cursor.
 * @head: the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define s3_list_for_each_entry_reverse(pos, head, member)      \
  for (pos = s3_list_entry((head)->prev, typeof(*pos), member);  \
       &pos->member != (head);   \
       pos = s3_list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:  the type * to use as a loop cursor.
 * @head: the head for your list.
 * @member: the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define s3_list_for_each_entry_from(pos, head, member)       \
  for (; &pos->member != (head);  \
       pos = s3_list_entry(pos->member.next, typeof(*pos), member))


#define s3_list_for_each_entry_safe(pos, n, head, member)                 \
    for (pos = s3_list_entry((head)->next, typeof(*pos), member),         \
            n = s3_list_entry(pos->member.next, typeof(*pos), member);    \
            &pos->member != (head);                                         \
            pos = n, n = s3_list_entry(n->member.next, typeof(*n), member))

#define s3_list_for_each_entry_safe_reverse(pos, n, head, member)         \
    for (pos = s3_list_entry((head)->prev, typeof(*pos), member),         \
            n = s3_list_entry(pos->member.prev, typeof(*pos), member);    \
            &pos->member != (head);                                         \
            pos = n, n = s3_list_entry(n->member.prev, typeof(*n), member))

#endif

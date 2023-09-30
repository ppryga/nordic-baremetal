#ifndef __TOOLS_SLIST_H__
#define __TOOLS_SLIST_H__

/** @file This is a simple implementation os sigle linked list.
 *
 * All operations done on head or tail of the list are O(1). In case of access to node that is
 * between head and tail iteration is required to find right element. That operation is of O(n)
 * complexity.
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief The structure is an internal type used to form a single linked list.
 *
 * This structure may be a member of other structure that stored actual list node data.
 * That approach allows to create multiple different lists that all use the same list
 * management code.
 */
typedef struct _slist_node {
	struct _slist_node *next;
} slist_node_t;

/** @brief The stucture holds a list*/
typedef struct _slist {
	slist_node_t *head;
	slist_node_t *tail;
} slist_t;

void slist_init(slist_t *list);

slist_node_t *slist_head_peek(slist_t *list);
slist_node_t *slist_head_get(slist_t *list);
void slist_head_remove(slist_t *list);
void slist_head_put(slist_t *list, slist_node_t *new_node);

slist_node_t *slist_tail_peek(slist_t *list);
void slist_tail_put(slist_t *list, slist_node_t *new_node);

slist_node_t *slist_next_peek(slist_node_t *node);
slist_node_t *slist_next_get(slist_t *list, slist_node_t *node);
void slist_next_remove(slist_t *list, slist_node_t *node);
void slist_next_put(slist_t *list, slist_node_t *node, slist_node_t *new_node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TOOLS_SLIST_H__ */

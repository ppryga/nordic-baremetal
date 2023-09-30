
#include <assert.h>
#include <stddef.h>
#include "slist.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void slist_init(slist_t *list)
{
	assert(list);

	list->head = NULL;
	list->tail = NULL;
}

slist_node_t *slist_head_peek(slist_t *list)
{
	assert(list);

	return list->head;
}

slist_node_t *slist_head_get(slist_t *list)
{
	assert(list);

	if (list->head != NULL) {
		slist_node_t *head = list->head;

		/* If list has only one element, then tail has to be updated also.
		 * Get operation removes head then both has to be changed to NULL.
		 */
		if (head == list->tail) {
			list->head = NULL;
			list->tail = NULL;
		} else {
			list->head = head->next;
		}

		return head;
	}

	/* Head was NULL, so list was empty*/
	return NULL;
}

void slist_head_remove(slist_t *list)
{
	assert(list);

	if (list->head != NULL) {
		/* If list has only one element, then tail has to be updated also.
		 * Get operation removes head then both has to be changed to NULL.
		 */
		if (list->head == list->tail) {
			list->head = NULL;
			list->tail = NULL;
		} else {
			list->head = list->head->next;
		}
	}
	/* Head was NULL, so list was empty*/
}

void slist_head_put(slist_t *list, slist_node_t *new_node)
{
	assert(list);
	assert(new_node);

	if (list->head == NULL) {
		/* Tail must be NULL so just store new node there also. */
		list->head = new_node;
		list->tail = new_node;
	} else {
		new_node->next = list->head;
		list->head = new_node;
	}
}

slist_node_t *slist_tail_peek(slist_t *list)
{
	assert(list);

	return list->tail;
}

void slist_tail_put(slist_t *list, slist_node_t *new_node)
{
	assert(list);
	assert(new_node);

	if (list->head == NULL) {
		/* Tail must be NULL so just store new node there also. */
		list->head = new_node;
		list->tail = new_node;
	} else {
		assert(list->tail != NULL);

		list->tail->next = new_node;
		list->tail = new_node;
	}
}

slist_node_t *slist_next_peek(slist_node_t *node)
{
	assert(node);

	return node->next;
}

slist_node_t *slist_next_get(slist_t *list, slist_node_t *node)
{
	assert(list);
	assert(node);

	slist_node_t *next_node = node->next;

	if (next_node != NULL) {
		if (next_node == list->tail) {
			/* next_node can't be list->head, it can be tail of the list. */
			list->tail = node;
			node->next = NULL;
		} else {
			/* node_next can be in between head and tail */
			node->next = next_node->next;
		}
		return next_node;
	}

	/* node is list->tail */
	assert(node == list->tail);

	return NULL;
}

void slist_next_remove(slist_t *list, slist_node_t *node)
{
	assert(list);
	assert(node);

	slist_node_t *next_node = node->next;

	if (next_node != NULL) {
		if (next_node == list->tail) {
			/* next_node can't be list->head, it can be tail of the list. */
			list->tail = node;
			node->next = NULL;
		} else {
			/* node_next can be in between head and tail */
			node->next = next_node->next;
		}
	} else {
		/* node is list->tail */
		assert(node == list->tail);
	}
}

void slist_next_put(slist_t *list, slist_node_t *node, slist_node_t *new_node)
{
	assert(list);
	assert(node);
	assert(new_node);

	if (list->tail == node) {
		slist_tail_put(list, new_node);
	} else {
		if (node->next != NULL) {
			slist_node_t *current_next = node->next;

			node->next = new_node;
			new_node->next = current_next;
		} else {
			node->next = new_node;
		}
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

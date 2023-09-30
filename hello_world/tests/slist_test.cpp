#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include <CppUTest/TestHarness.h>

#include "slist.h"
#include "tools/misc.h"

//#define TEST_DEBUG_OUTPUT 0
#define TEST_GROUP_NAME_PREPARE(testGroup) TEST_GROUP_##CppUTestGroup##testGroup

TEST_GROUP(slist_creation_base)
{
public:
	static const int NODES_NUMBER = 5;
	slist_t m_list;
	slist_node_t m_node[NODES_NUMBER];

	void test_list_print_content(slist_t * m_list)
	{
		assert(m_list);

		slist_node_t *m_node = slist_head_peek(m_list);
		do {
			printf("node: %p\r\n", m_node);
			m_node = slist_next_peek(m_node);
		} while (m_node != NULL);
	}
};
/* Tests */
TEST_GROUP_BASE(slist_creation_tests, TEST_GROUP_NAME_PREPARE(slist_creation_base))
{

};

TEST(slist_creation_tests, slist_create_by_head_put_test)
{
	slist_init(&m_list);

	for (int idx = 0; idx < NODES_NUMBER; idx++) {
		slist_head_put(&m_list, &m_node[idx]);
	}

	/* NOTE when add new nodes by put to head, index 0 is added firts
	 * hence becomes tail of the lists. 
	 */
	CHECK_EQUAL(m_list.head, &m_node[NODES_NUMBER - 1]);
	CHECK_EQUAL(m_list.tail, &m_node[0]);

#ifdef TEST_DEBUG_OUTPUT
	printf("List after create:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */
}

TEST(slist_creation_tests, slist_create_by_tail_put_test)
{
	slist_init(&m_list);

	for (int idx = 0; idx < NODES_NUMBER; idx++) {
		slist_tail_put(&m_list, &m_node[idx]);
	}

	CHECK_EQUAL(m_list.head, &m_node[0]);
	CHECK_EQUAL(m_list.tail, &m_node[NODES_NUMBER-1]);

#ifdef TEST_DEBUG_OUTPUT
	printf("List after create:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */
}

TEST_GROUP_BASE(slist_peek_node_tests,
		TEST_GROUP_NAME_PREPARE(slist_creation_tests))
{
	void setup()
	{
		slist_init(&m_list);

		for (int idx = 0; idx < ARRAY_SIZE(m_node); idx++) {
			slist_tail_put(&m_list, &m_node[idx]);
		}
	}

	void tear_down()
	{
		slist_init(&m_list);

		for (int idx = 0; idx < ARRAY_SIZE(m_node); idx++) {
			m_node[idx].next = NULL;
		}
	}
};

TEST(slist_peek_node_tests, slist_peek_head_test)
{
	slist_node_t *test_node;

	test_node = slist_head_peek(&m_list);
	CHECK_TRUE(test_node == &m_node[0]);
}

TEST(slist_peek_node_tests, slist_peek_tail_test)
{
	slist_node_t *test_node;

	test_node = slist_tail_peek(&m_list);
	CHECK_TRUE(test_node == &m_node[NODES_NUMBER-1]);
}

TEST(slist_peek_node_tests, slist_peek_node_next_test)
{
	slist_node_t *test_node;
	int test_idx = 2;

	test_node = slist_next_peek(&m_node[test_idx]);
	CHECK_TRUE(test_node == &m_node[test_idx + 1]);
}

TEST_GROUP_BASE(slist_get_node_tests, TEST_GROUP_NAME_PREPARE(slist_peek_node_tests))
{

};

TEST(slist_get_node_tests, test_slist_head_get)
{
	slist_node_t *test_node;

#ifdef TEST_DEBUG_OUTPUT
	printf("List after create:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */

	test_node = slist_head_get(&m_list);
	CHECK_TRUE(test_node == &m_node[0]);
	CHECK_TRUE(m_list.head == &m_node[1]);

#ifdef TEST_DEBUG_OUTPUT
	printf("List after get:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */
}

TEST(slist_get_node_tests, test_slist_next_get_for_node_in_mid)
{
	slist_node_t *test_node;
	int test_idx = 2;

#ifdef TEST_DEBUG_OUTPUT
	printf("List after create:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */

	test_node = slist_next_get(&m_list, &m_node[test_idx]);
	CHECK_TRUE(test_node == &m_node[test_idx + 1]);
	CHECK_TRUE(m_node[test_idx].next == &m_node[test_idx + 2]);

#ifdef TEST_DEBUG_OUTPUT
	printf("List after get:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */
}

TEST(slist_get_node_tests, test_slist_next_get_for_head)
{
	slist_node_t *test_node;
	int test_idx = 1;

#ifdef TEST_DEBUG_OUTPUT
	printf("List after create:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */

	test_node = slist_next_get(&m_list, &m_node[test_idx]);
	CHECK_TRUE(test_node == &m_node[test_idx + 1]);
	CHECK_TRUE(m_node[test_idx].next == &m_node[test_idx + 2]);

#ifdef TEST_DEBUG_OUTPUT
	printf("List after get:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */
}

TEST(slist_get_node_tests, test_slist_next_get_for_tail)
{
	slist_node_t *test_node;
	int test_idx = NODES_NUMBER-1;

#ifdef TEST_DEBUG_OUTPUT
	printf("List after create:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */

	test_node = slist_next_get(&m_list, &m_node[test_idx]);
	CHECK_TRUE(test_node == NULL);

#ifdef TEST_DEBUG_OUTPUT
	printf("List after get:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */
}

TEST(slist_get_node_tests, test_slist_next_get_for_one_before_tail)
{
	slist_node_t *test_node;
	int test_idx = NODES_NUMBER-2;

#ifdef TEST_DEBUG_OUTPUT
	printf("List after create:\r\n");
	test_list_print_content(&m_list);

	printf("Get from list node: %p\r\n", &m_node[test_idx].next);
#endif /* TEST_DEBUG_OUTPUT */

	test_node = slist_next_get(&m_list, &m_node[test_idx]);
	CHECK_TRUE(test_node == &m_node[test_idx + 1]);
	CHECK_TRUE(m_list.tail == &m_node[test_idx]);

#ifdef TEST_DEBUG_OUTPUT
	printf("List after get:\r\n");
	test_list_print_content(&m_list);
#endif /* TEST_DEBUG_OUTPUT */
}

TEST_GROUP_BASE(slist_remove_node_tests, TEST_GROUP_NAME_PREPARE(slist_get_node_tests))
{

};

TEST(slist_remove_node_tests, test_slist_remove_head_once)
{
	slist_node_t *new_head;

	slist_head_remove(&m_list);

	new_head = slist_head_peek(&m_list);
	CHECK_TRUE(new_head == &m_node[1]);
}

TEST(slist_remove_node_tests, test_slist_remove_head_all)
{
	slist_node_t *node;

	for (int idx = 0; idx < ARRAY_SIZE(m_node); idx++)
	{
		slist_head_remove(&m_list);
	}
	
	node = slist_head_peek(&m_list);
	CHECK_TRUE(node == NULL);
	node = slist_tail_peek(&m_list);
	CHECK_TRUE(node == NULL);
}

TEST(slist_remove_node_tests, test_slist_remove_next_once)
{
	slist_node_t *node;
	int test_idx = 2; /* Middle of the list nodes */

	slist_next_remove(&m_list, &m_node[test_idx]);

	node = slist_next_peek(&m_node[test_idx]);
	CHECK_TRUE(node == &m_node[test_idx+2]);
}

TEST(slist_remove_node_tests, test_slist_remove_next_all)
{
	slist_node_t *node;
	int test_idx_start = 2;

	for (int idx = test_idx_start; idx < NAME_MAX; idx++) {
		/* Remove next to the provided node, it should be always updated to point to node->next->next until
		 * tail of the list is reached.
		 */
		slist_next_remove(&m_list, &m_node[test_idx_start]);
	}
	node = slist_next_peek(&m_node[test_idx_start]);
	CHECK_TRUE(node == NULL);
}

TEST(slist_remove_node_tests, test_slist_remove_next_that_is_tail)
{
	slist_node_t *node;
	int test_idx = 3; /* Idx of node that is one before tail */

	slist_next_remove(&m_list, &m_node[test_idx]);

	node = slist_next_peek(&m_node[test_idx]);
	CHECK_TRUE(node == NULL);
}

TEST(slist_remove_node_tests, test_slist_remove_next_that_is_one_after_tail)
{
	slist_node_t *node;
	int test_idx = 4; /* Idx of node that is one before tail */

	slist_next_remove(&m_list, &m_node[test_idx]);

	
	node = slist_next_peek(&m_node[test_idx]);
	CHECK_TRUE(node == NULL);
}
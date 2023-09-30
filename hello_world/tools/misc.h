/*
 * Copyright (c) 2023 Piotr Pryga
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __TOOLS_MICS_H__
#define __TOOLS_MICS_H__

/* @brief Returns number of elements in an array */
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

/* @brief Macro allows to obtain an address to an instnace of a struct from pointer to one of members of the struct
 * 
 * Example 
 * 
 * //Structure that is a container we want to obtain from pointer to a member
 * struct container {
 *      int foo;
 *      int bar;
 * };
 * 
 * // Instantiation that happens somewhere in a code
 * struct container instance;
 * 
 * // Pointer to member that we have
 * int *bar_ptr = &instance.bar;
 * 
 * // Pointer to container instance that we obtain by use of pointer to member
 * struct container *contains_bar;
 * 
 * contains = CONTAINER_OF(bar_ptr, struct container, bar);
 * 
 * @param ptr pointer to a member of an object, which address we want to get
 * @param type name of a type of the object
 * @param field name of a filed (member) that ptr points to
 */
#define CONTAINER_OF(ptr, type, field) ((type *)(((uint8_t *)ptr) - offsetof(type, field)))

/* @brief Macro to generate an assembler inline with global constnat symbol that holds given value */
#define GEN_ASM_ABSOLUTE_SYM(sym_name, sym_value)                                                  \
	asm(".globl\t" #sym_name "\n\t"                                                            \
	    ".equ\t" #sym_name ",%c[value]\n\t"                                                    \
	    ".type\t" #sym_name ",%%object"                                                        \
	    :                                                                                      \
	    : [value] "n"(sym_value))

/* @brief Macro to generate an assembled accessible symbol that holds an offset of a member M from beginning of
 * struct S
 *
 * @param S name of a struct that holds member M
 * @param M name of a member of struct provided in S
 */
#define GEN_ASM_OFFSET_SYM(S, M) GEN_ASM_ABSOLUTE_SYM(__##S##_##M##_##OFFSET, offsetof(S, M))

/* @brief Macro to generate an assembled accessible symbol that holds an offset of a nested member M.N of  stcutc S.
 *
 * Example:
 * 
 * struct nested {
 *      int bar;
 * };
 * 
 * struct outer {
 *      int foo;
 *      struct nested bar_nested;
 * };
 * 
 * GEN_ASM_OFFSET_NESTED_SYM(struct outer, bar_nested, bar)
 *
 * @param S name of a struct that holds member M
 * @param M name of a member of struct provided in S
 * @param N name of a member of a struct that is type of member M
 */
#define GEN_ASM_OFFSET_NESTED_SYM(S, M, N)                                                         \
	GEN_ASM_ABSOLUTE_SYM(__##S##_##M##_##N##_##OFFSET, offsetof(S, M.N))
#define BIT(n) (1UL << (n))

#endif /* __TOOLS_MICS_H__ */

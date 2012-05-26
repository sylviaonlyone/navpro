/* This file is part of Into.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * IMPORTANT LICENSING INFORMATION
 *
 * Into is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Into is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _PIIPREPROCESSOR_H
#define _PIIPREPROCESSOR_H

/**
 * @file
 *
 * Useful preprocessor tricks.
 *
 * @ingroup Core
 */

/// @internal
#define PII_DO_STRINGIZE(arg) #arg
/**
 * Converts the argument to a string.
 *
 * @code
 * const char* name = PII_STRINGIZE(ClassName);
 * // const char* name == "ClassName";
 * @endcode
 */
#define PII_STRINGIZE(arg) PII_DO_STRINGIZE(arg)

/// @internal
#define PII_DO_JOIN(a, b) a ## b
/**
 * Joins arguments a and b.
 *
 * @code
 * struct PII_JOIN(Class, Name);
 * // struct ClassName.
 * @endcode
 */
#define PII_JOIN(a, b) PII_DO_JOIN(a, b)

/// @internal
#define PII_DO_JOIN3(a, b, c) a ## b ## c
/**
 * Joins arguments a, b and c.
 *
 * @see PII_JOIN
 */
#define PII_JOIN3(a, b, c) PII_DO_JOIN3(a, b, c)

/// @internal
#define PII_IF_SELECTOR_0(A,B) B
/// @internal
#define PII_IF_SELECTOR_1(A,B) A

/**
 * Expands to @a A if @a CONDITION is 0, and to @a B otherwise.
 */
#define PII_IF(CONDITION, A, B) PII_JOIN(PII_IF_SELECTOR_, CONDITION) (A,B)

/**
 * Expands to a comma (,).
 */
#define PII_COMMA ,
/**
 * Expands to a left parenthesis.
 */
#define PII_LPAREN (
/**
 * Expands to a right parenthesis.
 */
#define PII_RPAREN )

/**
 * Expands to @a PARAM. This macro is useful if you need to format the
 * arguments of a macro using another macro.
 *
 * @code
 * #define PRINTF(FORMAT, ARG1, ARG2) printf(FORMAT, ARG1, ARG2);
 *
 * #define ARGS1 "%s%d", "abc"
 * "define ARGS2 1
 *
 * #define CALL(MACRO) PII_EXPAND(MACRO PII_LPAREN ARGS1 PII_COMMA ARGS2 PII_RPAREN)
 *
 * CALL(PRINTF);
 * // expands to
 * // printf ("%s%d", "abc", 1);
 * @endcode
 */
#define PII_EXPAND(PARAM) PARAM

/**
 * Expands to the uppercase version of @a LETTER.
 *
 * @code
 * PII_UPPERCASE(a)
 * // Expands to A
 * @endcode
 */
#define PII_UPPERCASE(LETTER) PII_UPCASE_ ## LETTER
/**
 * Expands to the uppercase version of @a LETTER as a character string
 * constant.
 *
 * @code
 * PII_UPPERCASE_STR(a)
 * // Expands to "A"
 * @endcode
 */
#define PII_UPPERCASE_STR(LETTER) PII_STRINGIZE(PII_UPPERCASE(LETTER))

/**
 * Expands to the lowercase version of @a LETTER.
 *
 * @code
 * PII_UPPERCASE(B)
 * // Expands to b
 * @endcode
 */
#define PII_LOWERCASE(LETTER) PII_LOCASE_ ## LETTER
/**
 * Expands to the lowercase version of @a LETTER as a character string
 * constant.
 *
 * @code
 * PII_UPPERCASE_STR(B)
 * // Expands to "b"
 * @endcode
 */
#define PII_LOWERCASE_STR(LETTER) PII_STRINGIZE(PII_LOWERCASE(LETTER))

/// @cond null
#define PII_UPCASE_a A
#define PII_UPCASE_b B
#define PII_UPCASE_c C
#define PII_UPCASE_d D
#define PII_UPCASE_e E
#define PII_UPCASE_f F
#define PII_UPCASE_g G
#define PII_UPCASE_h H
#define PII_UPCASE_i I
#define PII_UPCASE_j J
#define PII_UPCASE_k K
#define PII_UPCASE_l L
#define PII_UPCASE_m M
#define PII_UPCASE_n N
#define PII_UPCASE_o O
#define PII_UPCASE_p P
#define PII_UPCASE_q Q
#define PII_UPCASE_r R
#define PII_UPCASE_s S
#define PII_UPCASE_t T
#define PII_UPCASE_u U
#define PII_UPCASE_v V
#define PII_UPCASE_w W
#define PII_UPCASE_x X
#define PII_UPCASE_y Y
#define PII_UPCASE_z Z

#define PII_LOCASE_A a
#define PII_LOCASE_B b
#define PII_LOCASE_C c
#define PII_LOCASE_D d
#define PII_LOCASE_E e
#define PII_LOCASE_F f
#define PII_LOCASE_G g
#define PII_LOCASE_H h
#define PII_LOCASE_I i
#define PII_LOCASE_J j
#define PII_LOCASE_K k
#define PII_LOCASE_L l
#define PII_LOCASE_M m
#define PII_LOCASE_N n
#define PII_LOCASE_O o
#define PII_LOCASE_P p
#define PII_LOCASE_Q q
#define PII_LOCASE_R r
#define PII_LOCASE_S s
#define PII_LOCASE_T t
#define PII_LOCASE_U u
#define PII_LOCASE_V v
#define PII_LOCASE_W w
#define PII_LOCASE_X x
#define PII_LOCASE_Y y
#define PII_LOCASE_Z z
/// @cond null

/**
 * Remove parentheses around @a PARAMS. The number of comma-separated
 * elements in @a PARAMS must be @a N.
 *
 * @code
 * PII_REMOVE_PARENS(3, (a, b, c))
 * // Expands to a, b, c
 * @endcode
 */
#define PII_REMOVE_PARENS(N, PARAMS) PII_JOIN(PII_REMOVE_PARENS_, N)PARAMS

/**
 * Expands to 0 if @a PARAM is a non-empty list of the form (first,
 * rest), and to 1 otherwise.
 */
#define PII_IS_NIL(PARAM) PII_JOIN(PII_TEST_NIL_, PII_TEST_NIL PARAM))


/// @cond null
#define PII_TEST_NIL(P1,P2) PII_NON_NIL
#define PII_TEST_NIL_PII_NON_NIL PII_NIL_FALSE(
#define PII_TEST_NIL_PII_TEST_NIL PII_NIL_TRUE(
#define PII_NIL_TRUE(X) 1
#define PII_NIL_FALSE() 0

// No, I'm not talking about automobiles. Go learn some Lisp.
#define PII_CAR_2(CAR, CDR) CAR
#define PII_CDR_2(CAR, CDR) CDR
#define PII_CAR(LIST) PII_CAR_2 LIST
#define PII_CDR(LIST) PII_CDR_2 LIST
/// @endcond


#ifndef _MSC_VER
/**
 * Repeats @a MACRO for each of the @a N elements in @a ELEMENTS. @a
 * MACRO will be called with two parameters: the (zero-based) index of
 * the current element and the element.
 *
 * @code
 * #define PRINTF(N, X) printf(#X);
 *
 * PII_FOR_N(PRINTF, 3, (a, b, c))
 * // Expands to
 * // printf("a"); printf("b"); printf("c");
 * @endcode
 */
#  define PII_FOR_N(MACRO, N, ELEMENTS) \
  PII_EXPAND(PII_JOIN(PII_FOR_, N) PII_LPAREN MACRO PII_COMMA PII_NULL_SEP PII_COMMA PII_REMOVE_PARENS(N, ELEMENTS) PII_RPAREN)
/**
 * Repeats @a MACRO for each of the @a N elements in @a ELEMENTS,
 * placing @a SEPARATOR between each element. @a SEPARATOR will be
 * called with one parameter: the (zero-based) index of the current
 * element and the element. Use @p PII_COMMA_SEP to place a comma
 * between elements.
 *
 * @code
 * #define MYMACRO(N, X) i##X = N
 *
 * int PII_FOR_N_SEP(MYMACRO, PII_COMMA_SEP, 3, (a, b, c));
 * // Expands to
 * // int ia = 0 , ib = 1 , ic = 2;
 * @endcode
 */
#  define PII_FOR_N_SEP(MACRO, SEPARATOR, N, ELEMENTS) \
  PII_EXPAND(PII_JOIN(PII_FOR_, N) PII_LPAREN MACRO PII_COMMA SEPARATOR PII_COMMA PII_REMOVE_PARENS(N, ELEMENTS) PII_RPAREN)
#else
#  define PII_FOR_N(MACRO, N, ELEMENTS) \
  PII_JOIN(PII_FOR_, N) PII_LPAREN MACRO PII_COMMA PII_NULL_SEP PII_COMMA PII_EXPAND(PII_JOIN(PII_REMOVE_PARENS_, N)ELEMENTS) PII_RPAREN
#  define PII_FOR_N_SEP(MACRO, SEPARATOR, N, ELEMENTS) \
  PII_JOIN(PII_FOR_, N) PII_LPAREN MACRO PII_COMMA SEPARATOR PII_COMMA PII_EXPAND(PII_JOIN(PII_REMOVE_PARENS_, N)ELEMENTS) PII_RPAREN
#endif


#define PII_NULL_SEP(N)
#define PII_COMMA_SEP(N) ,

/**
 * Repeat @a MACRO for each element in @a LIST.
 *
 * @code
 * #define LIST (1, (2, (3, (4, (5, PII_NIL)))))
 * #define MACRO(PARAM) + PARAM
 *
 * int i = PII_FOREACH(MACRO, LIST);
 * // expands to + 1 + 2 + 3 + 4 + 5
 * @endcode
 */
#define PII_FOREACH(MACRO, LIST) \
  PII_NEXT_LOOP(0, LIST)(MACRO, PII_CAR(LIST), PII_CDR(LIST))

#include "PiiPreprocessorFor.h"

#endif //_PIIPREPROCESSOR_H

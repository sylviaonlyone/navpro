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

#ifndef _PIITEMPLATEEXPORT_H
#define _PIITEMPLATEEXPORT_H

/**
 * @file
 *
 * Contains macros for exporting explicit template instances from
 * shared libraries. Different compilers need different (non-standard)
 * tricks to export template instantiations. The standard-compliant @p
 * export keyword, which was intended approximately for the purpose
 * falls miles short of its target and is actually not supported by
 * most of the compilers.
 *
 * @li GCC, MinGW: The @p DECLARE macros declare an @p extern explicit
 * instance of a template as per C++0x. The @p DEFINE macros create an
 * explicit instance.
 *
 * @li MSVC: The @p DECLARE macros expand to nothing if building a
 * library. If the library is not being build, the @p DECLARE macros
 * declare an explicit instance with @p __declspec(dllimport) storage
 * class. The @p DEFINE macros create an explicit instance with @p
 * __declspec(dllexport) storage class. This trick is needed because
 * there is no way to just declare an exported explicit instance in
 * MSVC. Even if the storage class is @p dllexport, the compiler
 * instantiates the template. In some cases all definitions for the
 * required template instance may not be available yet, and the
 * template must be instantiated later.
 *
 * @code
 * // In MyStruct.h
 * #include <PiiTemplateExport.h>
 *
 * #ifdef BUILD_MYLIB
 * #  define BUILDING_MYLIB 1
 * #else
 * #  define BUILDING_MYLIB 0
 * #endif
 *
 * template <class T> struct MyStruct
 * {
 * };
 *
 * template <class T> T identityFunc(T value) { return value; }
 *
 * // Declare an exported explicit instance of the class
 * PII_DECLARE_EXPORTED_CLASS_TEMPLATE(struct, MyStruct<double>, BUILDING_MYLIB);
 * // Declare an exported explicit instance of a template function.
 * PII_DECLARE_EXPORTED_FUNCTION_TEMPLATE(void, identityFunc<int>, (int), BUILDING_MYLIB);
 * @endcode
 *
 * @code
 * // In MyStruct.cc, create explicit instances:
 * #include "MyStruct.h"
 * PII_DEFINE_EXPORTED_CLASS_TEMPLATE(struct, MyStruct<double>);
 * PII_DEFINE_EXPORTED_FUNCTION_TEMPLATE(void, identityFunc<int>, (int));
 * @endcode
 *
 * @see PiiGlobal.h
 */

#include "PiiPreprocessor.h"

#if defined(_MSC_VER)
#  define PII_DECLARE_EXPORTED_CLASS_TEMPLATE(KEYWORD, TYPENAME, BUILDING_LIB) \
  PII_IF(BUILDING_LIB, namespace Dummy {}, template KEYWORD __declspec(dllimport) TYPENAME)
#  define PII_DEFINE_EXPORTED_CLASS_TEMPLATE(KEYWORD, TYPENAME) template KEYWORD __declspec(dllexport) TYPENAME
#  define PII_DECLARE_EXPORTED_CLASS_TEMPLATE2(KEYWORD, TYPENAME1, TYPENAME2, BUILDING_LIB) \
  PII_IF(BUILDING_LIB, namespace Dummy {}, template KEYWORD __declspec(dllimport) TYPENAME1 PII_COMMA TYPENAME2)
#  define PII_DEFINE_EXPORTED_CLASS_TEMPLATE2(KEYWORD, TYPENAME1, TYPENAME2) template KEYWORD __declspec(dllexport) TYPENAME1, TYPENAME2
#  define PII_DECLARE_EXPORTED_FUNCTION_TEMPLATE(RETURN, NAME, PARAMS, BUILDING_LIB) \
  PII_IF(BUILDING_LIB, namespace Dummy {}, template __declspec(dllimport) RETURN NAME PARAMS)
#  define PII_DEFINE_EXPORTED_FUNCTION_TEMPLATE(RETURN, NAME, PARAMS) template _declspec(dllexport) RETURN NAME PARAMS
#else
#  define PII_DECLARE_EXPORTED_CLASS_TEMPLATE(KEYWORD, TYPENAME, BUILDING_LIB) extern template KEYWORD TYPENAME
#  define PII_DEFINE_EXPORTED_CLASS_TEMPLATE(KEYWORD, TYPENAME) template KEYWORD TYPENAME
#  define PII_DECLARE_EXPORTED_CLASS_TEMPLATE2(KEYWORD, TYPENAME1, TYPENAME2, BUILDING_LIB) extern template KEYWORD TYPENAME1, TYPENAME2
#  define PII_DEFINE_EXPORTED_CLASS_TEMPLATE2(KEYWORD, TYPENAME1, TYPENAME2) template KEYWORD TYPENAME1, TYPENAME2
#  define PII_DECLARE_EXPORTED_FUNCTION_TEMPLATE(RETURN, NAME, PARAMS, BUILDING_LIB) extern template RETURN NAME PARAMS
#  define PII_DEFINE_EXPORTED_FUNCTION_TEMPLATE(RETURN, NAME, PARAMS) template RETURN NAME PARAMS
#endif

#endif //_PIITEMPLATEEXPORT_H

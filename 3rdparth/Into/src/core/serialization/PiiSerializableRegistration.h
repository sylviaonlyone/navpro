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

/**
 * @file
 *
 * Serializable objects are most conveniently registered by including
 * this file. The most straightforward way of making your class
 * serializable is as follows:
 *
 * @code
 * class MyClass
 * {
 *   friend class Piiserialization::Accessor;
 *   template <class Archive> void serialize(Archive& ar, const unsigned int version)
 *   {
 *      ar & i;
 *   }
 *  int i;
 * };
 *
 * #define PII_SERIALIZABLE_CLASS MyClass
 * #include <PiiSerializableRegistration.h>
 * @endcode
 *
 * The file recognizes the following macros (everything but @p
 * PII_SERIALIZABLE_CLASS are optional):
 *
 * @lip PII_SERIALIZABLE_CLASS - the name of the serializable type.
 *
 * @lip PII_SERIALIZABLE_CLASS_NAME - a stringified version of the
 * name. This is needed if you want a custom name for your type.
 *
 * @lip PII_TEMPLATE_TYPEDEF - a custom "class name" for a template
 * class. Use this if your class is a template instance with more than
 * one template parameter. It is a necessary work-around for the
 * preprocessor.
 *
 * @lip PII_SERIALIZABLE_CLASS_VERSION - the version number of your
 * class. The default is 0.
 *
 * @lip PII_NO_TRACKING - disable object tracking for this type. If
 * object tracking is disabled, references to the same memory location
 * may be serialized many times.
 *
 * @lip PII_VIRTUAL_METAOBJECT - tell the serialization system that
 * your class implements a virtual piiMetaObject() function. This is
 * necessary if the class is going to be serialized via a base class
 * pointer/reference. Actually, it is not strictly necessary to repeat
 * this macro for each derived class if the application never sees
 * them. But make sure you use it for the base class. Furthermore, you
 * need to place @ref PII_VIRTUAL_METAOBJECT_FUNCTION into the class
 * declaration of the base type and all derived types.
 *
 * @lip PII_SERIALIZABLE_IS_ABSTRACT - another way of disabling
 * default factory creation. Since abstract classes cannot be
 * instantiated, defining this value makes the library not try "new"
 * for the type. It also sets the corresponding type trait.
 *
 * @lip PII_SERIALIZABLE_SHARED - the serializable object is shared by
 * many translation units. This macro effectively suppresses the
 * creation of factory and serializer objects for the type. Just the
 * necessary declarations will be present. This is needed to avoid
 * multiple definitions. One must use the @ref PII_SERIALIZABLE_EXPORT
 * macro in a .cc file to create the necessary definitions. If your
 * serializable type is used only within one translation unit, you
 * don't need to define this value.
 *
 * @lip PII_BUILDING_LIBRARY - 1 if you are building a shared library
 * from which you want to export a serializable type, 0 otherwise.
 * This is needed on Windows to properly handle
 * <tt>__declspec(dllexport)</tt> and <tt>__declspec(dllimport></tt>.
 * This macro must be defined if @p PII_SERIALIZABLE_SHARED is
 * defined.
 *
 * @lip PII_ARCHIVE_TYPE - the archive type the object's serializer is
 * registered to. The default is 0, which registers the serializers to
 * generic archive types (PiiGenericInputArchive,
 * PiiGenericOutputArchive). This makes it possible to use the
 * serializable class with any archive implementation, but imposes a
 * performance penalty. Use 1 for text archives and 2 for binary
 * archives. Note that this macro has no effect if @p
 * PII_SERIALIZABLE_SHARED is defined, because it suppresses
 * serializer registrations.
 *
 * @lip PII_CUSTOM_FACTORY - if this macro is defined, a factory
 * object will be registered not only to the default serialization
 * factory but also to the archive-specific factory determined by @p
 * PII_ARCHIVE_TYPE.
 *
 * The following example shows how to register an abstract class that
 * will be linked to an application from a dynamic library.
 *
 * @code
 * // In PiiOperation.h
 * #define PII_SERIALIZABLE_CLASS PiiOperation
 * #define PII_SERIALIZABLE_IS_ABSTRACT
 * #define PII_SERIALIZABLE_SHARED
 * #define PII_VIRTUAL_METAOBJECT
 * #define PII_BUILDING_LIBRARY PII_BUILDING_YDIN
 *
 * #include <PiiSerializableRegistration.h>
 *
 * // In PiiOperation.cc
 * PII_DEFINE_VIRTUAL_METAOBJECT_FUNCTION(PiiOperation);
 * PII_SERIALIZABLE_EXPORT(PiiOperation);
 * @endcode
 *
 * @b NOTE! It is @b not possible to register class templates. 
 * Template instantiations are OK, but generic templates cannot be
 * registered without instantiating them. Furthermore, template
 * instantiations with more than one template parameter cannot be
 * registered directly because the preprocessor doesn't tolerate
 * commas in macro parameters. Use the @p PII_TEMPLATE_TYPEDEF macro,
 * preferably with @p PII_SERIALIZABLE_CLASS_NAME in these cases.
 *
 * An example of registering a template instance:
 *
 * @code
 * template <class T1, class T2> class MyClass
 * {
 *   friend class Piiserialization::Accessor;
 *   template <class Archive> void serialize(Archive& ar, const unsigned int version)
 *   {
 *      ar & t1;
 *      ar & t2;
 *   }
 *   T1 t1;
 *   T2 t2;
 * };
 *
 * // Explicit instantiation
 * template MyClass<int,int>;
 *
 * #define PII_SERIALIZABLE_CLASS MyClass<int,int>
 * #define PII_TEMPLATE_TYPEDEF MyClass_int_int
 * #define PII_SERIALIZABLE_CLASS_NAME "MyClass<int,int>"
 * #include <PiiSerializableRegistration.h>
 * @endcode
 *
 * @ingroup Serialization
 */

#include "PiiSerializationTraits.h"
#include "PiiSerializationFactory.h"
#include "PiiVirtualMetaObject.h"

#ifndef PII_SERIALIZABLE_CLASS
#  error "PII_SERIALIZABLE_CLASS must be defined before including PiiSerializableRegistration.h"
#endif

// Replace template class name with a typedef
#ifdef PII_TEMPLATE_TYPEDEF
typedef PII_SERIALIZABLE_CLASS PII_TEMPLATE_TYPEDEF;
#  undef PII_SERIALIZABLE_CLASS
#  define PII_SERIALIZABLE_CLASS PII_TEMPLATE_TYPEDEF
#endif

#ifndef PII_SERIALIZABLE_CLASS_NAME
#  define PII_SERIALIZABLE_CLASS_NAME PII_STRINGIZE(PII_SERIALIZABLE_CLASS)
#endif

PII_SERIALIZATION_NAME_CUSTOM(PII_SERIALIZABLE_CLASS, PII_SERIALIZABLE_CLASS_NAME);

// Store version trait
#ifdef PII_SERIALIZABLE_CLASS_VERSION
PII_SERIALIZATION_VERSION(PII_SERIALIZABLE_CLASS, PII_SERIALIZABLE_CLASS_VERSION);
#endif

// Store tracking trait
#ifdef PII_NO_TRACKING
PII_SERIALIZATION_TRACKING(PII_SERIALIZABLE_CLASS, false);
#endif

// Virtual metaobject implementation requested (otherwise, use
// default)
#ifdef PII_VIRTUAL_METAOBJECT
PII_SERIALIZATION_VIRTUAL_METAOBJECT(PII_SERIALIZABLE_CLASS);
#endif

#ifdef PII_SERIALIZABLE_IS_ABSTRACT
PII_SERIALIZATION_ABSTRACT(PII_SERIALIZABLE_CLASS);
#endif

// The factory and serializer of serializable objects in library
// cannot be instantiated here because that would cause multiple
// definitions.
#ifndef PII_SERIALIZABLE_SHARED
#  if !defined(PII_ARCHIVE_TYPE) || PII_ARCHIVE_TYPE == 0
#    include "PiiGenericInputArchive.h"
#    include "PiiGenericOutputArchive.h"
#    define PII_INPUT_ARCHIVE_NAME PiiGenericInputArchive
#    define PII_OUTPUT_ARCHIVE_NAME PiiGenericOutputArchive
#  elif PII_ARCHIVE_TYPE == 1
#    include "PiiTextInputArchive.h"
#    include "PiiTextOutputArchive.h"
#    define PII_INPUT_ARCHIVE_NAME PiiTextInputArchive
#    define PII_OUTPUT_ARCHIVE_NAME PiiTextOutputArchive
#  else
#    include "PiiBinaryInputArchive.h"
#    include "PiiBinaryOutputArchive.h"
#    define PII_INPUT_ARCHIVE_NAME PiiBinaryInputArchive
#    define PII_OUTPUT_ARCHIVE_NAME PiiBinaryOutputArchive
#  endif
#  include "PiiSerializationUtil.h"
PII_INSTANTIATE_SERIALIZER(PII_INPUT_ARCHIVE_NAME, PII_SERIALIZABLE_CLASS);
PII_INSTANTIATE_SERIALIZER(PII_OUTPUT_ARCHIVE_NAME, PII_SERIALIZABLE_CLASS);
PII_INSTANTIATE_FACTORY(PII_SERIALIZABLE_CLASS);
#  ifdef PII_CUSTOM_FACTORY
PII_INSTANTIATE_ARCHIVE_FACTORY(PII_SERIALIZABLE_CLASS, PII_INPUT_ARCHIVE_NAME);
#  endif
#else
#  ifndef PII_BUILDING_LIBRARY
#    error "PII_BUILDING_LIBRARY must be defined if PII_SERIALIZABLE_SHARED is defined."
#  endif
PII_DECLARE_FACTORY(PII_SERIALIZABLE_CLASS, PII_BUILDING_LIBRARY);
#endif // ifndef PII_SERIALIZABLE_SHARED

#undef PII_SERIALIZABLE_CLASS
#undef PII_TEMPLATE_TYPEDEF
#undef PII_SERIALIZABLE_CLASS_NAME
#undef PII_SERIALIZABLE_CLASS_VERSION
#undef PII_VIRTUAL_METAOBJECT
#undef PII_NO_TRACKING
#undef PII_SERIALIZABLE_SHARED
#undef PII_BUILDING_LIBRARY
#undef PII_SERIALIZABLE_IS_ABSTRACT
#undef PII_ARCHIVE_TYPE
#undef PII_INPUT_ARCHIVE_NAME
#undef PII_OUTPUT_ARCHIVE_NAME

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

#ifndef _PIIQVARIANTWRAPPER_H
#define _PIIQVARIANTWRAPPER_H

#include <QVariant>
#include "PiiSerialization.h"
#include "PiiVirtualMetaObject.h"

/**
 * A serialization wrapper for user types in @ref QVariant
 * "QVariants". To enable serialization of a user type as a QVariant,
 * one needs to subclass %PiiQVariantWrapper. The subclass must be
 * registered as a serializable object with the standard template
 * serializer naming syntax, e.g. "PiiQVariantWrapper<MyType>". The
 * serializer needs to store and retrieve "MyType" and store it as the
 * value of the internal #_variant member.
 *
 * @code
 * // In MyClass.h first declare the type as a Qt meta type:
 * extern int iMyClassTypeId;
 * Q_DECLARE_METATYPE(MyClass);
 *
 * // In MyClass.cc register the meta type:
 * int iMyClassTypeId = qRegisterMetaType<MyClass>("MyClass");
 *
 * // Then register the wrapper as a serializable object:
 * #include <PiiQVariantWrapper.h>
 * #define PII_SERIALIZABLE_CLASS PiiQVariantWrapper::Template<MyClass>
 * #define PII_SERIALIZABLE_CLASS_NAME "PiiQVariantWrapper<MyClass>"
 *
 * #include <PiiSerializableRegistration.h>
 * @endcode
 *
 * @ingroup Serialization
 */
class PII_SERIALIZATION_EXPORT PiiQVariantWrapper
{
  friend struct PiiSerialization::Accessor;
  PII_DECLARE_VIRTUAL_METAOBJECT_FUNCTION = 0;
  template <class Archive> void serialize(Archive&, const unsigned int) {}
  
public:
  /**
   * Get a reference to the variant. Subclasses store the actual user
   * type into the protected #_variant member variable. Use this
   * function to retrieve the restored value.
   */
  QVariant variant() const { return _variant; }
  /**
   * Set the variant to @p v, which stores a user type. After calling
   * this function, subclass can store the actual user type.
   */
  void setVariant(QVariant v) { _variant = v; }

  template <class T> class Template;

  virtual ~PiiQVariantWrapper();
  
protected:
  QVariant _variant;
};

/**
 * A template that is instantiated for all user types intended to be
 * serialized as QVariants. For each such user type, an instance of
 * this template must be compiled, and its serializer and factory must
 * be registered. To make @p MyType serializable as a QVariant, do
 * this in a .cc file:
 *
 * @code
 * #include <PiiQVariantWrapper.h>
 * #define PII_SERIALIZABLE_CLASS PiiQVariantWrapper::Template<MyType>
 * #define PII_SERIALIZABLE_CLASS_NAME "PiiQVariantWrapper<MyType>"
 * #include <PiiSerializableRegistration.h>
 * @endcode
 *
 * @ingroup Serialization
 */
template <class T> class PiiQVariantWrapper::Template : public PiiQVariantWrapper
{
  friend struct PiiSerialization::Accessor;
  PII_VIRTUAL_METAOBJECT_FUNCTION
  PII_SEPARATE_SAVE_LOAD_MEMBERS

  template <class Archive> void save(Archive& archive, const unsigned int)
  {
    archive << _variant.value<T>();
  }

  template <class Archive> void load(Archive& archive, const unsigned int)
  {
    T value;
    archive >> value;
    _variant = QVariant::fromValue(value);
  }
};

// Tracking is unnecessary because we only use the wrappers as
// temporary objects in storing and restoring user types.
PII_SERIALIZATION_TRACKING(PiiQVariantWrapper, false);
PII_SERIALIZATION_VIRTUAL_METAOBJECT(PiiQVariantWrapper);
PII_SERIALIZATION_ABSTRACT(PiiQVariantWrapper);

#endif //_PIIQVARIANTWRAPPER_H

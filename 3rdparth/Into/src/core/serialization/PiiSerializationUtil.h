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

#ifndef _PIISERIALIZATIONUTIL_H
#define _PIISERIALIZATIONUTIL_H

/**
 * @file
 *
 * Convenience macros and utility functions for serializing many Qt
 * types.
 *
 * @ingroup Serialization
 */

#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>
#include <QString>
#include <QPair>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QColor>
#include <QFont>
#include <QBrush>
#include <QPixmap>
#include <QGradient>
#include <QStringList>
#include <QByteArray>
#include <QVarLengthArray>
#include <QBuffer>
#include <QImage>
#include <QDateTime>
#include <complex>
#include "PiiSerialization.h"
#include "PiiSerializationFactory.h"
#include "PiiVirtualMetaObject.h"

/**
 * Prepare a class declaration for serialization. This puts the
 * necessary stuff into the class declaration. You may continue the
 * class declaration either by providing the function body or by
 * defining the function elsewhere.
 *
 * @code
 * class MyClass
 * {
 *   PII_DECLARE_SERIALIZATION_FUNCTION;
 * public:
 *   ...
 * };
 *
 * template <class Archive> void MyClass::serialize(Archive& archive, const unsigned int version)
 * {
 *   //Serialize members here
 * }
 * @endcode
 */
#define PII_DECLARE_SERIALIZATION_FUNCTION \
private: \
  friend struct PiiSerialization::Accessor; \
  template <class Archive> void serialize(Archive& archive, const unsigned int version)

/**
 * A default implementation of serialization functions. The default
 * function just passes serialization to the base object denoted by @a
 * BASE.
 *
 * @code
 * class MyDerived : public MyClass
 * {
 *   PII_DEFAULT_SERIALIZATION_FUNCTION(MyClass)
 * };
 * @endcode
 */
#define PII_DEFAULT_SERIALIZATION_FUNCTION(BASE) \
  PII_DECLARE_SERIALIZATION_FUNCTION { Q_UNUSED(version); PII_SERIALIZE_BASE(archive, BASE); }

/**
 * An implementation of serialization functions that stores and
 * retrieves object properties. The object must be derived from
 * QObject. This implementation first serializes the base class
 * denoted by @a BASE. Then, it serializes object properties. This
 * macro is used like @ref PII_DEFAULT_SERIALIZATION_FUNCTION. If the
 * state of your class can be fully stored/retrieved with its
 * properties, this is the macro you need. Note that all properties,
 * including those defined in subclasses will be serialized.
 *
 * @code
 * class MyClass : public QObject
 * {
 *   Q_OBJECT
 *   Q_PROPERTY(int member READ member WRITE setMember);
 *
 *   PII_PROPERTY_SERIALIZATION_FUNCTION(QObject)
 * public:
 *   void setMember(int m) { _iMember = m; }
 *   int member() const { return _iMember; }
 *
 * private:
 *   int _iMember;
 * };
 * @endcode
 */
#define PII_PROPERTY_SERIALIZATION_FUNCTION(base) \
  PII_DECLARE_SERIALIZATION_FUNCTION \
  { \
    Q_UNUSED(version); \
    PII_SERIALIZE_BASE(archive, base); \
    PiiSerialization::serializeProperties(archive, *this); \
  }

PII_SERIALIZATION_NAME(QObject);
class QWidget;
PII_SERIALIZATION_NAME(QWidget);

namespace PiiSerialization
{
  /**
   * Default implementation of meta-object getter for QObject-derived
   * objects. This function uses QMetaObject to construct an instance
   * of PiiMetaObject that will be kept in a static map.
   *
   * The className() field of the returned PiiMetaObject will be equal
   * to obj->metaObject()->className(). The version will be 0 by
   * default. This can be changed by adding a "version" tag to the
   * class:
   *
   * @code
   * class MyClass : public QObject
   * {
   *   Q_OBJECT
   *   Q_CLASSINFO("version", "1");
   * };
   * @endcode
   *
   * @see QMetaClassInfo
   */
  PII_SERIALIZATION_EXPORT const PiiMetaObject* metaObjectPointer(const QObject* obj);
  
  /**
   * Serializes a QObject. Does nothing but makes it possible to use
   * QObject as a base class when serializing objects.
   */
  template <class Archive> inline void serialize(Archive& /*ar*/, QObject& /*obj*/, const unsigned int /*version*/) {}
  
  /**
   * Save the properties of a QObject. This function stores properties
   * so that they can be retrieved in any order. It serializes all
   * read/write properties whose @p stored flag is @p true, and all
   * dynamic properties.
   *
   * @param archive an output archive
   *
   * @param obj the object whose properties are to be stored
   *
   * @param propertyOffset start from this property. 0 means all
   * properties, 1 means all but the @p objectName property.
   */
  template <class Archive> void saveProperties(Archive& archive, const QObject& obj, int propertyOffset=0);

  /**
   * Load properties stored by saveProperties(). Since properties are
   * saved as name-value pairs, one can restore them even if new
   * properties had been added or the order of properties has been
   * altered after saving.
   *
   * @param archive an input archive
   *
   * @param obj the object whose properties are to be restored
   */
  template <class Archive> void loadProperties(Archive& archive, QObject& obj);

  /**
   * A template function that either saves or load properties
   * depending on the type of the archive.
   */
  template <class Archive> inline void serializeProperties(Archive& archive, QObject& obj, int propertyOffset=0);

  /**
   * Serializes a QVariant.
   */
  template <class Archive> void save(Archive& archive, QVariant& variant, const unsigned int version);
  /**
   * Deserializes a QVariant.
   */
  template <class Archive> void load(Archive& archive, QVariant& variant, const unsigned int version);

  /**
   * Serializes a QSize.
   */
  template <class Archive> inline void save(Archive& archive, QSize& size, const unsigned int version);
  /**
   * Deserializes a QSize.
   */
  template <class Archive> inline void load(Archive& archive, QSize& size, const unsigned int version);
  /**
   * Serializes a QSizeF.
   */
  template <class Archive> inline void save(Archive& archive, QSizeF& size, const unsigned int version);
  /**
   * Deserializes a QSizeF.
   */
  template <class Archive> inline void load(Archive& archive, QSizeF& size, const unsigned int version);

  /**
   * Serializes a QPoint.
   */
  template <class Archive> inline void save(Archive& archive, QPoint& point, const unsigned int version);
  /**
   * Deserializes a QPoint.
   */
  template <class Archive> inline void load(Archive& archive, QPoint& point, const unsigned int version);
  /**
   * Serializes a QPointF.
   */
  template <class Archive> inline void save(Archive& archive, QPointF& point, const unsigned int version);
  /**
   * Deserializes a QPointF.
   */
  template <class Archive> inline void load(Archive& archive, QPointF& point, const unsigned int version);

  /**
   * Serializes a QRect.
   */
  template <class Archive> inline void save(Archive& archive, QRect& rect, const unsigned int version);
  /**
   * Deserializes a QRect.
   */
  template <class Archive> inline void load(Archive& archive, QRect& rect, const unsigned int version);
  /**
   * Serializes a QRectF.
   */
  template <class Archive> inline void save(Archive& archive, QRectF& rect, const unsigned int version);
  /**
   * Deserializes a QRectF.
   */
  template <class Archive> inline void load(Archive& archive, QRectF& rect, const unsigned int version);

  /**
   * Serializes any random-access collection (QVector/QList). A
   * random-access collection is serialized by storing the size
   * followed by the items. A serializer for the type @p T must be
   * defined.
   */
  template <class T, class Archive, class Collection> void save(Archive& archive, Collection& lst, const unsigned int version);

  /**
   * Deserializes any random-access collection (QVector/QList).
   */
  template <class T, class Archive, class Collection> void load(Archive& archive, Collection& lst, const unsigned int version);

  template <class Archive, class Collection, class T> inline void serialize(Archive& archive, Collection& lst, const unsigned int version);
  
  /**
   * Serializes a QList.
   */
  template <class Archive, class T> inline void serialize(Archive & ar, QList<T> & lst, const unsigned int version)
  {
    serialize<Archive, QList<T>, T>(ar, lst, version);
  }

  /**
   * Serializes a QVector.
   */
  template <class Archive, class T> inline void serialize(Archive & ar, QVector<T> & lst, const unsigned int version)
  {
    serialize<Archive, QVector<T>, T>(ar, lst, version);
  }

  /**
   * Serializes a QStringList.
   */
  template <class Archive> inline void serialize(Archive & ar, QStringList & lst, const unsigned int version)
  {
    serialize(ar, (QList<QString>&)lst, version);
  }

  /**
   * Serializes a QVarLenthArray.
   */
  template <class Archive, class T, int N> inline void serialize(Archive & ar, QVarLengthArray<T,N> & lst, const unsigned int version)
  {
    serialize<Archive, QVarLengthArray<T,N>, T>(ar, lst, version);
  }

  /**
   * Serializes a QPair.
   */
  template <class Archive, class T, class U> inline void serialize(Archive & ar, QPair<T,U> & pair, const unsigned int version);

  /**
   * Serializes a QMap.
   */
  template <class Archive, class Key, class Val> void save(Archive& archive, QMap<Key,Val>& map, const unsigned int version);
  /**
   * Deserializes a QMap.
   */
  template <class Archive, class Key, class Val> void load(Archive& archive, QMap<Key,Val>& map, const unsigned int version);

  template <class Archive, class Key, class Val> inline void serialize(Archive& archive, QMap<Key,Val>& map, const unsigned int version)
  {
    separateFunctions(archive, map, version);
  }

  /**
   * Serializes a QColor.
   */
  template <class Archive> inline void save(Archive& archive, QColor& color, const unsigned int version);
  /**
   * Deserializes a QColor.
   */
  template <class Archive> inline void load(Archive& archive, QColor& color, const unsigned int version);

  /**
   * Serializes a QByteArray.
   */
  template <class Archive> inline void save(Archive& archive, QByteArray& array, const unsigned int version);

  /**
   * Deserializes a QByteArray.
   */
  template <class Archive> inline void load(Archive& archive, QByteArray& array, const unsigned int version);

  /**
   * Serializes a QPixmap.
   */
  template <class Archive> inline void save(Archive& archive, QPixmap& pixmap, const unsigned int version);

  /**
   * Deserializes a QPixmap.
   */
  template <class Archive> inline void load(Archive& archive, QPixmap& pixmap, const unsigned int version);

  /**
   * Serializes a QGradient.
   */
  template <class Archive> inline void save(Archive& /*archive*/, QGradient& /*gradient*/, const unsigned int version);

  /**
   * Deserializes a QGradient.
   */
  template <class Archive> inline void load(Archive& /*archive*/, QGradient& /*gradient*/, const unsigned int version);
  
  /**
   * Serializes a QBrush.
   */
  template <class Archive> inline void save(Archive& archive, QBrush& brush, const unsigned int version);
  /**
   * Deserializes a QBrush.
   */
  template <class Archive> inline void load(Archive& archive, QBrush& brush, const unsigned int version);

  /**
   * Serializes a QFont.
   */
  template <class Archive> inline void save(Archive& archive, QFont& font, const unsigned int version);
  
  /**
   * Deserializes a QFont.
   */
  template <class Archive> inline void load(Archive& archive, QFont& font, const unsigned int version);

  /**
   * Serializes a QTime.
   */
  template <class Archive> inline void save(Archive& archive, QTime& time, const unsigned int version);
  
  /**
   * Deserializes a QTime.
   */
  template <class Archive> inline void load(Archive& archive, QTime& time, const unsigned int version);

  /**
   * Serializes a QDate.
   */
  template <class Archive> inline void save(Archive& archive, QDate& time, const unsigned int version);
  
  /**
   * Deserializes a QDate.
   */
  template <class Archive> inline void load(Archive& archive, QDate& time, const unsigned int version);

  /**
   * Serializes a QDateTime.
   */
  template <class Archive> inline void save(Archive& archive, QDateTime& time, const unsigned int version);
  
  /**
   * Deserializes a QDateTime.
   */
  template <class Archive> inline void load(Archive& archive, QDateTime& time, const unsigned int version);

  /**
   * Serializes a QImage.
   * PENDING
   */
  template <class Archive> inline void serialize(Archive&, QImage&, const unsigned int) {}

  /**
   * Serializes an object to a byte array. This function is useful if
   * objects need to be transferred through the clipboard, for
   * example.
   *
   * @code
   * MyClass c;
   * QByteArray marshalled = PiiSerialization::toByteArray<PiiTextOutputArchive>(c);
   * @endcode
   *
   * @see fromByteArray()
   */
  template <class Archive, class T> QByteArray toByteArray(const T& object);

  /**
   * Deserializes an object stored into a byte array.
   *
   * @code
   * MyClass c;
   * PiiSerialization::fromByteArray<PiiTextInputArchive>(marshalled, c);
   * @endcode
   *
   * @see toByteArray()
   */
  template <class Archive, class T> void fromByteArray(const QByteArray& array, T& object);

} // namespace PiiSerialization

#include <PiiMetaTemplate.h>
#include "PiiSerialization.h"
#include "PiiSerializationTraits.h"

PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QVariant);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QSize);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QPoint);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QRect);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QSizeF);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QPointF);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QRectF);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QColor);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QFont);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QTime);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QDate);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QDateTime);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QBrush);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QPixmap);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QGradient);
PII_SEPARATE_SAVE_LOAD_FUNCTIONS(QByteArray);

// Disable tracking and class info on all QPairs and tracking on QMaps, QLists and QVectors
namespace PiiSerializationTraits
{
  template <class T, class U> struct Tracking<QPair<T,U> > : Pii::False {};
  template <class T, class U> struct ClassInfo<QPair<T,U> > : Pii::False {};

  template <class T, class U> struct Tracking<QMap<T,U> > : Pii::False {};
  template <class T> struct Tracking<QVector<T> > : Pii::False {};
  template <class T> struct Tracking<QList<T> > : Pii::False {};
  template <class T, int N> struct Tracking<QVarLengthArray<T,N> > : Pii::False {};

  template <class T> struct ClassInfo<std::complex<T> > : Pii::False {};
  template <class T> struct Tracking<std::complex<T> > : Pii::False {};
}

PII_DECLARE_FACTORY(QObject, PII_BUILDING_SERIALIZATION);
PII_SERIALIZATION_DYNAMIC(QObject);

PII_SERIALIZATION_TRACKING(QStringList, false);
PII_SERIALIZATION_TRACKING(QGradient, false);

PII_SERIALIZATION_CLASSINFO(QVariant, false);
PII_SERIALIZATION_TRACKING(QVariant, false);

PII_SERIALIZATION_CLASSINFO(QSize, false);
PII_SERIALIZATION_TRACKING(QSize, false);

PII_SERIALIZATION_CLASSINFO(QPoint, false);
PII_SERIALIZATION_TRACKING(QPoint, false);

PII_SERIALIZATION_CLASSINFO(QRect, false);
PII_SERIALIZATION_TRACKING(QRect, false);

PII_SERIALIZATION_CLASSINFO(QSizeF, false);
PII_SERIALIZATION_TRACKING(QSizeF, false);

PII_SERIALIZATION_CLASSINFO(QPointF, false);
PII_SERIALIZATION_TRACKING(QPointF, false);

PII_SERIALIZATION_CLASSINFO(QRectF, false);
PII_SERIALIZATION_TRACKING(QRectF, false);

PII_SERIALIZATION_CLASSINFO(QBrush, false);
PII_SERIALIZATION_TRACKING(QBrush, false);

PII_SERIALIZATION_CLASSINFO(QColor, false);
PII_SERIALIZATION_TRACKING(QColor, false);

PII_SERIALIZATION_CLASSINFO(QFont, false);
PII_SERIALIZATION_TRACKING(QFont, false);

PII_SERIALIZATION_CLASSINFO(QTime, false);
PII_SERIALIZATION_TRACKING(QTime, false);

PII_SERIALIZATION_CLASSINFO(QDate, false);
PII_SERIALIZATION_TRACKING(QDate, false);

PII_SERIALIZATION_CLASSINFO(QDateTime, false);
PII_SERIALIZATION_TRACKING(QDateTime, false);

//PII_SERIALIZATION_VIRTUAL(QPixmap, false);
//PII_SERIALIZATION_VIRTUAL(QByteArray, false);

#include "PiiSerializationUtil-templates.h"

#endif //_PIISERIALIZATIONUTIL_H

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

#ifndef _PIIUTIL_H
#define _PIIUTIL_H

#include <cstdlib>
#include <iostream>
#include <cstdarg>

#include <QList>
#include <QVariant>
#include <QDateTime>
#include <QList>
#include <QStringList>
#include <QDir>
#include <QtGui/QPolygon>
#include <QPoint>
#include <QSettings>
#include <QVariantMap>
#include <QPair>

#include "PiiGlobal.h"
#include "PiiTypeTraits.h"
#include "PiiMathDefs.h"
#include "PiiAlgorithm.h"
#include "PiiTemplateExport.h"

/**
 * @file
 *
 * Miscellaneous useful functions, mainly for dealing with Qt classes.
 *
 * @ingroup Core
 */


namespace Pii
{
  /**
   * Write a QString to an @p std output stream.
   */
  inline std::ostream& operator<< (std::ostream& out, const QString& str)
  {
    out << str.toAscii().constData();
    return out;
  }
  
  /**
   * Find the intersection of two lists. The result contains the
   * elements that are present in both lists, or an empty list if the
   * intersection is empty. Any collection defining @p size(), @p
   * contains(), and @p append() functions can be used as a parameter.
   */
  template <class Collection> Collection intersect(const Collection& list1, const Collection& list2)
  {
    Collection result;
    for (int i=0; i<list1.size(); i++)
      if (list2.contains(list1[i]))
        result.append(list1[i]);
    return result;
  }

  /**
   * Find the union of two lists. The result contains the elements
   * that are present in either list, ignoring duplicates. Any
   * collection defining @p size(), @p contains(), and @p append()
   * functions can be used as a parameter.
   */
  template <class Collection> Collection join(const Collection& list1, const Collection& list2)
  {
    Collection result(list1);
    for (int i=0; i<list2.size(); i++)
      if (!result.contains(list2[i]))
        result.append(list2[i]);
    return result;
  }

  /**
   * Create a list that consists of the elements of @p list1 that are
   * not present in @p list2. Any collection defining @p size(), @p
   * contains(), and @p append() functions can be used as a parameter.
   */
  template <class Collection> Collection subtract(const Collection& list1, const Collection& list2)
  {
    Collection result;
    for (int i=0; i<list1.size(); i++)
      if (!list2.contains(list1[i]))
        result.append(list1[i]);
    return result;
  }

  /**
   * Property types for properties().
   */
  enum PropertyFlag
    {
      ReadableProperties = 1,
      WritableProperties = 2,
      StoredProperties   = 4,
      ScriptableProperties = 8,
      DesignableProperties = 16,
      DynamicProperties = 32
    };
  Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag);
  
  /**
   * Fetch the properties of a QObject as a map of name-value pairs.
   *
   * @param obj the object whose properties are queried
   *
   * @param propertyOffset skip this many properties in the beginning. 
   * This value can be used to skip the properties of superclasses. 
   * For example, using one as the @a propertyOffset skips the @ref
   * QObject::objectName property.
   *
   * @param propertyTypes a logical or of property types to be
   * included in the query (see @ref Pii::Properties
   * "Properties"). The function only returns properties that match
   * match all of the flags. For example, to return only stored
   * properties, use @p StoredProperties as the flag. "Readable" is,
   * of course, always a requirement, so one does not need to
   * explicitly specify it. @p DynamicProperties is an exception: if
   * it is set, dynamic properties will be returned, otherwise not. 
   * The default value includes all readable properties, including
   * dynamic ones.
   */
  PII_CORE_EXPORT QVariantMap properties(const QObject& obj, int propertyOffset = 0,
                                         PropertyFlags flags = DynamicProperties);

  /**
   * Returns the properties of @a obj as a list. This function retains
   * the order of properties.
   *
   * @see properties()
   */
  PII_CORE_EXPORT QList<QPair<QString, QVariant> > propertyList(const QObject& obj,
                                                                int propertyOffset = 0,
                                                                PropertyFlags flags = DynamicProperties);

  /**
   * Sets many properties in a bunch.
   *
   * @param obj the object to be modified
   *
   * @param properties a map of properties to set (name-value pairs)
   */
  PII_CORE_EXPORT void setProperties(QObject& obj, const QVariantMap& lstProperties);
  PII_CORE_EXPORT void setPropertyList(QObject& obj, const QList<QPair<QString, QVariant> >& lstProperties);

  /**
   * Converts C-style argument list to a QStringList.
   */
  PII_CORE_EXPORT QStringList argsToList(int argc, char* argv[]);
  
  /**
   * Converts a list of QVariant objects into a QList of type @p T.
   *
   * @code
   * QVariantList lst;
   * lst << 1.0 << 2.3;
   * QList<double> dLst = Pii::variantsToList<double>(lst);
   * @endcode
   */
  template <class T> QList<T> variantsToList(const QVariantList& variants)
  {
    QList<T> result;
    for (int i=0; i<variants.size(); i++)
      result << variants[i].value<T>();
    return result;
  }

  /**
   * Convert any collection to a QVariantList that contains the same
   * values as QVariant objects.
   */
  template <class T, class Collection> QVariantList collectionToVariants(const Collection& lst)
  {
    QVariantList result;
    for (int i=0; i<lst.size(); i++)
      result << lst[i];
    return result;
  }

  /**
   * Convert a QList to a QVariantList that contains the same values
   * as QVariant objects.
   */
  template <class T> inline QVariantList listToVariants(const QList<T>& lst)
  {
    return collectionToVariants<T>(lst);
  }

  /**
   * Convert a QVector to a QVariantList that contains the same values
   * as QVariant objects.
   */
  template <class T> inline QVariantList vectorToVariants(const QVector<T>& lst)
  {
    return collectionToVariants<T>(lst);
  }

  /**
   * Convert a list of QVariant objects into a QVector of type @p T.
   *
   * @code
   * QVariantList lst;
   * lst << 1.0 << 2.3;
   * QVector<double> dLst = Pii::variantsToVector<double>(lst);
   * @endcode
   */
  template <class T> QVector<T> variantsToVector(const QVariantList& variants)
  {
    QVector<T> result(variants.size());
    for (int i=variants.size(); i--; )
      result[i] = variants[i].value<T>();
    return result;
  }

  /**
   * Randomize the order of elements in a collection.
   *
   * @code
   * QList<int> lst;
   * lst << 1 << 2 << 3;
   * Pii::shuffle(lst);
   * @endcode
   */
  template <class Collection> inline void shuffle(Collection& lst)
  {
    shuffle(lst.begin(), lst.end());
  }

  
  /**
   * Select randomly @a n distinct integers that are smaller than @a
   * max. This function can be used to take a random sample of a
   * collection.
   *
   * @code
   * QStringList lstNames;
   * lstNames << "foo" << "bar" << "etc";
   * QVector<int> indices = Pii::selectRandomly(2, lstNames.size());
   * for (int i=0; i<indices.size(); ++i)
   *   qDebug(qPrintable(lstNames[indices[i]]));
   * @endcode
   *
   * @return randomly selected indices. If @a n is larger than @a max,
   * @a max indices will returned.
   *
   * @see selectRandomly(Collection&, const Collection&, int)
   */
  QVector<int> PII_CORE_EXPORT selectRandomly(int n, int max);

  /**
   * @overload
   *
   * Randomly selects @a n distinct integers in [0, max-1] and stores
   * them to @a indices. This version is better suited for selecting a
   * small number of indices in tight loops.
   */
  void PII_CORE_EXPORT selectRandomly(QVector<int>& indices, int n, int max);

  /**
   * Select randomly @a n elements from @a source and insert them to
   * @a target. The @p Collection type must have size() and append()
   * member functions defined.
   *
   * @code
   * QStringList lstNames;
   * lstNames << "foo" << "bar" << "etc";
   * QStringList selected;
   * Pii::selectRandomly(selected, lstNames, 2);
   * @endcode
   */
  template <class Collection>
  void selectRandomly(Collection& target, const Collection& source, int n)
  {
   QVector<int> indices = selectRandomly(n, source.size());
   for (int i=0; i<indices.size(); ++i)
     target.append(source[indices[i]]);
  }
  
  /**
   * Find all parent objects of @p obj up to @p maxParents parent
   * objects. By default, all parents are returned. If a template
   * parameter is specified, only parents matching the given type are
   * returned.
   *
   * @code
   * QList<MyObj*> parents = Pii::findAllParents<MyObj*>(obj);
   * // Returns all parents of obj whose type is MyObj*
   * @endcode
   */
  template <class T> QList<T> findAllParents(const QObject* obj,
                                             unsigned int maxParents = Pii::Numeric<unsigned int>::maxValue())
  {
    QList<T> result;
    QObject* parentObj = obj->parent();
    while (parentObj != 0 && maxParents--)
      {
        T obj = qobject_cast<T>(parentObj);
        if (obj != 0)
          result << obj;
        parentObj = parentObj->parent();
      }
    return result;
  }

  /**
   * Find all parents independent of their type.
   */
  PII_CORE_EXPORT QList<QObject*> findAllParents(const QObject* obj, unsigned int maxParents = Pii::Numeric<unsigned int>::maxValue());

  /**
   * Find the first parent of @p obj whose type matches @p T.
   * Equivalent to findAllParents(obj,1), but faster.
   */
  template <class T> T findFirstParent(const QObject* obj)
  {
    QObject* parentObj = obj->parent();
    while (parentObj != 0)
      {
        T obj = qobject_cast<T>(parentObj);
        if (obj != 0)
          return obj;
        parentObj = parentObj->parent();
      }
    return 0;
  }

  /**
   * Returns @p true if @p parent is a parent of @p child.
   */
  PII_CORE_EXPORT bool isParent(const QObject* parent, const QObject* child);

  /**
   * Find the object that is the closest common ancestor of @p obj1
   * and @p obj2 in the object hierarchy.
   *
   * @param obj1 first object
   *
   * @param obj1 second object
   *
   * @param parentIndex if non-zero, the index of the closest parent
   * object in @p obj1's parent list is stored here.
   *
   * @return a pointer to the parent found, or zero if the objects
   * don't have a common parent.
   */
  PII_CORE_EXPORT QObject* findCommonParent(const QObject* obj1, const QObject* obj2, int* parentIndex = 0);
  
  /**
   * Delete all members of a collection. This function works for any
   * stl or Qt collection. It goes through all members of the
   * collection and deletes each.
   *
   * @param c the collection. Any collection (e.g. QList, QMap, or
   * std::list) containing pointers.
   */
  template <class Collection> void deleteAll(Collection& c)
  {
    for (typename Collection::iterator i = c.begin(); i != c.end(); i++)
      delete i.operator*();
  }
  
  /**
   * Hash function for null-terminated C strings.
   */
  PII_CORE_EXPORT uint qHash(const char* key);

  /**
   * Match a list of crontab-like strings against the given time
   * stamp. Each string in @p list represents a rule with a syntax
   * similar but not equivalent to crontab (man crontab). Each rule
   * contains six fields separated by spaces. The fields are (in this
   * order): minute, hour, day, month, day of week, and week number. 
   * Each field may contain a comma-separated list of values or value
   * ranges, or an asterisk (*) that denotes "all". A rule matches if
   * all of its fields match. A set of rules matches if any of its
   * rules matches.
   *
   * @code
   * // Returns true during the first minute after midnight every day and
   * // 4:00-6:00 on the first day of every month
   * matchCrontab(QStringList() <<
   *              "0 0 * * * *" <<
   *              "* 4-5 1 * * *");
   *
   * // Returns true between 8:30 and 8:31 on Monday, Wednesday, and
   * // Friday on weeks 1, 2, 3, 5, 6, 7, 8, and 9
   * matchCrontab(QStringList() <<
   *              "30 8 * * 1,3,5 1-3,5-9);
   * @endcode
   *
   * @param list a list of crontab entries.
   *
   * @param timeStamp the time to match the entries against
   *
   * @return @p true if any of the entries match the time stamp.
   */
  PII_CORE_EXPORT bool matchCrontab(QStringList list, QDateTime timeStamp = QDateTime::currentDateTime());

  /**
   * Fetch a list of files matching a wildcard.
   *
   * @param pattern a wildcard pattern, e.g. "<code>/usr/share/icons/</code><code>*.xpm</code>"
   *
   * @param filters a filter for file attributes
   *
   * @param sort sort order
   *
   * @return all matching file names with full paths, sorted according
   * to @p sort.
   */
  PII_CORE_EXPORT QStringList fileList(const QString& pattern,
                                       QDir::Filters filters = QDir::Files | QDir::Readable | QDir::CaseSensitive,
                                       QDir::SortFlags sort = QDir::Name);

  /**
   * Flags for controlling property decoding.
   *
   * @lip NoDecodingFlag - no options apply
   *
   * @lip TrimPropertyName - remove white space in the beginning and
   * end of property name.
   *
   * @lip TrimPropertyValue - remove white space in the beginning and
   * end of property value.
   *
   * @lip RemoveQuotes - remove double quotes around property
   * names/values. Implies @p TrimPropertyName and @p
   * TrimPropertyValue.
   *
   * @lip DowncasePropertyName - convert all property names to lower
   * case.
   */
  enum PropertyDecodingFlag
    {
      NoDecodingFlag = 0,
      TrimPropertyName = 1,
      TrimPropertyValue = 2,
      RemoveQuotes = 4,
      DowncasePropertyName = 8
    };

  Q_DECLARE_FLAGS(PropertyDecodingFlags, PropertyDecodingFlag)
  Q_DECLARE_OPERATORS_FOR_FLAGS(PropertyDecodingFlags)

  /**
   * Returns the index of the first occurrence of @a separator in @p
   * str, starting at @a startIndex. This function ignores occurrences
   * that are preceded by an odd number of @a escape characters. If
   * the separator is not found, returns -1.
   *
   * @code
   * Pii::findSeparator("\"Test \\"string\"", '"', 1); // returns 14
   * @endcode
   */
  PII_CORE_EXPORT int findSeparator(const QString& str, QChar separator, int startIndex = 0, QChar escape = '\\');

  /**
   * Splits a string in which each part may be quoted.
   *
   * @code
   * Pii::splitQuoted("\"a,b,c\",d,e"); // returns ("a,b,c", "d", "e")
   * @endcode
   */
  PII_CORE_EXPORT QStringList splitQuoted(const QString& str, QChar separator = ',', QChar quote = '"',
                                          QString::SplitBehavior behavior = QString::KeepEmptyParts);
  
  /**
   * Decode string-encoded properties into a variant map. This
   * function can be used to parse ini files, css-style properties or
   * anything similar to a map of name-value pairs. All values will be
   * QStrings, but they are stored as QVariants for easy cooperation
   * with #setProperties().
   *
   * @param encodedProperties the string to decode
   *
   * @param propertySeparator a character that separates name-value
   * pairs
   *
   * @param valueSeparator a character that separates name from value
   *
   * @param escape escape character
   *
   * @param flags flags for controlling the decoding
   *
   * @return a map of name-value pairs
   *
   * @code
   * QVariantMap props = Pii::decodeProperties("color: #fff;font-size: 5pt", ';', ':');
   * // props now has two values:
   * // "color" -> "#fff"
   * // "font-size" -> "5pt"
   * @endcode
   */
  PII_CORE_EXPORT QVariantMap decodeProperties(const QString& encodedProperties,
                                               QChar propertySeparator = '\n',
                                               QChar valueSeparator = '=',
                                               QChar escape = '\\',
                                               PropertyDecodingFlags flags = TrimPropertyName | TrimPropertyValue | RemoveQuotes);

  /**
   * Replaces variables in @a string and returns a new string.
   *
   * @param string the input string with variables. Variables are
   * prefixed with a dollar sign and optionally delimited by curly
   * braces. ($variable or ${variable}).
   *
   * @param variables a map of variable values. This can be any type
   * that defines <tt>QString operator[] (const QString&)</tt>.
   *
   * @code
   * QMap<QString,QString> mapVariables;
   * mapVariables["foo"] = "bar";
   * mapVariables["bar"] = "foo";
   * QString strResult = Pii::replaceVariables("$foo ${bar}", mapVariables);
   * // strResult == "bar foo"
   * @endcode
   */
  template <class VariableMap> QString replaceVariables(const QString& string, const VariableMap& variables)
  {
    QString strResult(string);
    QRegExp reVariable("\\$((\\w+)|\\{(\\w+)\\})");
    int index = 0;
    while ((index = reVariable.indexIn(strResult, index)) != -1)
      {
        QString strReplacement;
        if (!reVariable.cap(2).isEmpty()) // no curly braces
          strReplacement = variables[reVariable.cap(2)];
        else if (!reVariable.cap(3).isEmpty()) // curly braces
          strReplacement = variables[reVariable.cap(3)];
        strResult.replace(index, reVariable.matchedLength(), strReplacement);
        index += strReplacement.size();
      }
    return strResult;
  }


  /**
   * Finds the name of a property in @p obj that matches @p name in a
   * case-insensitive manner.
   *
   * @return the real name of the property or 0 if no such property exists
   *
   * @code
   * QObject obj;
   * const char* objName = Pii::propertyName(obj, "objectname");
   * // returns "objectName"
   * @endcode
   */
  PII_CORE_EXPORT const char* propertyName(const QObject& obj, const QString& name);

  /**
   * Sets properties to an object. For convenient use with
   * configuration files, the function ignores values starting with a
   * comment marker ('#' by default).
   *
   * @param properties a list of property names to set
   *
   * @param valueMap a map that stores the actual property values
   *
   * @param sensitivity if @p Qt::CaseSensitive, the property names
   * must match exactly. If @p Qt::CaseInsensitive, case-insensitive
   * matching will be performed (@ref propertyName()).
   *
   * @param commentMark ignore values starting with this string
   *
   * @code
   * // Read properties from an ini file (case-sensitive)
   * QSettings settings(configFile, QSettings::IniFormat);
   * Pii::setProperties(obj, settings.childKeys(), settings);
   * @endcode
   *
   * @code
   * // Read string-encoded properties (case-insensitive)
   * QVariantMap properties = Pii::decodeProperties("objectname=foobar\n"
   *                                                "//comment, ignored");
   * Pii::setProperties(obj, properties.keys(), properties, Qt::CaseInsensitive, "//");
   * @endcode
   */
  template <class Map> void setProperties(QObject& obj, const QStringList& properties, const Map& valueMap,
                                          Qt::CaseSensitivity sensitivity = Qt::CaseSensitive,
                                          const QString& commentMark = "#");


  // Declare two exported explicit instantiations.
  PII_DECLARE_EXPORTED_FUNCTION_TEMPLATE(void, setProperties<QSettings>,
                                         (QObject& obj,
                                          const QStringList& properties,
                                          const QSettings& valueMap,
                                          Qt::CaseSensitivity sensitivity,
                                          const QString& commentMark),
                                         PII_BUILDING_CORE);
  PII_DECLARE_EXPORTED_FUNCTION_TEMPLATE(void, setProperties<QVariantMap>,
                                         (QObject& obj,
                                          const QStringList& properties,
                                          const QVariantMap& valueMap,
                                          Qt::CaseSensitivity sensitivity,
                                          const QString& commentMark),
                                         PII_BUILDING_CORE);

  /**
   * Performs array copy of non-overlapping arrays. If used with
   * primitive type, uses memcpy, else uses operator= one by one.
   *
   * @param to Pointer to destination array.
   * @param from Pointer to source array.
   * @param itemCount Number of items of type T to copy.
   *
   * @note Behavior is undefined if arrays @p to and @p from overlap.
   */
  template<class T, class size_type>
  inline void arrayCopy(T* to, const T* from, const size_type itemCount)
  {
    // Using simple if instead of defining further template implementation.
    if (Pii::IsPrimitive<T>::boolValue)
      memcpy(to, from, sizeof(T)*itemCount);
    else
      {
        for(size_type i = 0; i<itemCount; ++i)
          to[i] = from[i];
      }
  }

  /**
   * Converts a string into a number. This function differs from
   * QString::toDouble() and friends in that it recognizes magnitude
   * suffices.
   *
   * @code
   * Pii::toDouble("10k"); // returns 10000
   * Pii::toDouble("-2M"); // returns -2000000
   * Pii::toDouble("1.2m"); // returns 0.0012
   * @endcode
   *
   * The following suffices are recognized:
   *
   * <table style="border: 0">
   * <tr><th>Name</th><th>Symbol</th><th>Magnitude</th></tr>
   * <tr><td>yotta</td><td>Y</td><td>@f$10^{24}@f$</td></tr>
   * <tr><td>zetta</td><td>Z</td><td>@f$10^{21}@f$</td></tr>
   * <tr><td>exa</td><td>E</td><td>@f$10^{18}@f$</td></tr>
   * <tr><td>peta</td><td>P</td><td>@f$10^{15}@f$</td></tr>
   * <tr><td>tera</td><td>T</td><td>@f$10^{12}@f$</td></tr>
   * <tr><td>giga</td><td>G</td><td>@f$10^{9}@f$</td></tr>
   * <tr><td>mega</td><td>M</td><td>@f$10^{6}@f$</td></tr>
   * <tr><td>kilo</td><td>k</td><td>@f$10^{3}@f$</td></tr>
   * <tr><td>hecto</td><td>h</td><td> @f$10^{2}@f$</td></tr>
   * <tr><td>deka</td><td>e</td><td> @f$10^{1}@f$</td></tr>
   * <tr><td>deci</td><td>d</td><td>@f$10^{-1}@f$</td></tr>
   * <tr><td>centi</td><td>c</td><td> @f$10^{-2}@f$</td></tr>
   * <tr><td>milli</td><td>m</td><td> @f$10^{-3}@f$</td></tr>
   * <tr><td>micro</td><td>u</td><td> @f$10^{-6}@f$</td></tr>
   * <tr><td>nano</td><td>n</td><td>@f$10^{-9}@f$</td></tr>
   * <tr><td>pico</td><td>p</td><td>@f$10^{-12}@f$</td></tr>
   * <tr><td>femto</td><td>f</td><td> @f$10^{-15}@f$</td></tr>
   * <tr><td>atto</td><td>a</td><td>@f$10^{-18}@f$</td></tr>
   * <tr><td>zepto</td><td>z</td><td> @f$10^{-21}@f$</td></tr>
   * <tr><td>yocto</td><td>y</td><td> @f$10^{-24}@f$</td></tr>
   * </table>
   *
   * @param number the string to convert
   *
   * @param ok an optional pointer to a @p bool that indicates if the
   * conversion was successful.
   */
  PII_CORE_EXPORT double toDouble(const QString& number, bool* ok = 0);

  /**
   * Convert a string to any type.
   */
  template <class T> inline T stringTo(const QString& number, bool* ok = 0) { return T(number.toInt(ok)); }
  template <> inline short stringTo<short>(const QString& number, bool* ok) { return number.toShort(ok); }
  template <> inline unsigned short stringTo<unsigned short>(const QString& number, bool* ok) { return number.toUShort(ok); }
  template <> inline int stringTo<int>(const QString& number, bool* ok) { return number.toInt(ok); }
  template <> inline unsigned int stringTo<unsigned int>(const QString& number, bool* ok) { return number.toUInt(ok); }
  template <> inline long stringTo<long>(const QString& number, bool* ok) { return number.toLong(ok); }
  template <> inline unsigned long stringTo<unsigned long>(const QString& number, bool* ok) { return number.toULong(ok); }
  template <> inline long long stringTo<long long>(const QString& number, bool* ok) { return number.toLongLong(ok); }
  template <> inline unsigned long long stringTo<unsigned long long>(const QString& number, bool* ok) { return number.toULongLong(ok); }
  template <> inline float stringTo<float>(const QString& number, bool* ok) { return number.toFloat(ok); }
  template <> inline double stringTo<double>(const QString& number, bool* ok) { return number.toDouble(ok); }

  /**
   * Recursively connects neighbors given as a list of pairs. The
   * input contains a list of index pairs, each indicating a
   * "neighbor" relationship. The function scans through the list and
   * forms chains of neighbors. Finally, it returns a list in which
   * each entry contains a list of indices that were chained. Each
   * list is sorted in ascending order.
   *
   * @code
   * QLinkedList<QPair<int,int> > lstPairs;
   * lstPairs << qMakePair(0,1) << qMakePair(0,3) << qMakePair(0,4) << qMakePair(3,4) << qMakePair(3,5);
   * lstPairs << qMakePair(2,6) << qMakePair(6,8) << qMakePair(7,8);
   *
   * QList<QList<int> > lstNeighbors(Pii::findNeighbors(lstPairs));
   * // ((0, 1, 3, 4, 5), (2, 6, 7, 8))
   * @endcode
   */
  PII_CORE_EXPORT QList<QList<int> > findNeighbors(QLinkedList<QPair<int,int> >& pairs);
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Pii::PropertyFlags);

template <class T> QList<T> operator&& (const QList<T>& list1, const QList<T>& list2)
{
  return Pii::intersect(list1, list2);
}

inline QStringList operator&& (const QStringList& list1, const QStringList& list2)
{
  return Pii::intersect(list1, list2);
}

template <class T> QList<T> operator|| (const QList<T>& list1, const QList<T>& list2)
{
  return Pii::join(list1, list2);
}

inline QStringList operator|| (const QStringList& list1, const QStringList& list2)
{
  return Pii::join(list1, list2);
}

template <class T> QList<T> operator- (const QList<T>& list1, const QList<T>& list2)
{
  return Pii::subtract(list1, list2);
}

inline QStringList operator- (const QStringList& list1, const QStringList& list2)
{
  return Pii::subtract(list1, list2);
}

#endif //_PIIUTIL_H

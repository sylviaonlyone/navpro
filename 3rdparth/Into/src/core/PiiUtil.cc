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

#include "PiiUtil.h"

#include "PiiFunctional.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <QtDebug>
#include <QLinkedList>
#include <cmath>

namespace Pii
{
  QStringList argsToList(int argc, char* argv[])
  {
    QStringList lstResult;
    lstResult.reserve(argc);
    for (int i=0; i<argc; ++i)
      lstResult << argv[i];
    return lstResult;
  }
  
  QVariantMap properties(const QObject& obj, int propertyOffset, PropertyFlags flags)
  {
    QList<QPair<QString, QVariant> > lstProperties = propertyList(obj, propertyOffset, flags);

    QVariantMap data;
    for (int i=0; i<lstProperties.size(); i++)
      data[lstProperties[i].first] = lstProperties[i].second;

    return data;
  }

  QList<QPair<QString, QVariant> > propertyList(const QObject& obj, int propertyOffset, PropertyFlags flags)
  {
    QList<QPair<QString, QVariant> > result;

    const QMetaObject* pMetaObject = obj.metaObject();
    // Loop through the properties 
    for (int i=propertyOffset; i<pMetaObject->propertyCount(); ++i)
      {
        // Store only properties that match our flags
        QMetaProperty prop = pMetaObject->property(i);
        if (prop.isReadable() &&
            (!(flags & WritableProperties) || prop.isWritable()) &&
            (!(flags & StoredProperties) || prop.isStored()) &&
            (!(flags & ScriptableProperties) || prop.isScriptable()) &&
            (!(flags & DesignableProperties) || prop.isDesignable()))
          result.append(qMakePair(QString(prop.name()), obj.property(prop.name())));
      }

    if (flags & DynamicProperties)
      {
        // Dynamic properties
        QList<QByteArray> lstDynamicPropertyNames = obj.dynamicPropertyNames();
        for (int i=0; i<lstDynamicPropertyNames.size(); i++)
          {
            const char* pName = lstDynamicPropertyNames[i];
            result.append(qMakePair(QString(pName), obj.property(pName)));
          }
      }
    
    return result;
  }
  
  void setProperties(QObject& obj, const QVariantMap& properties)
  {
    QMapIterator<QString, QVariant> i(properties);
    while (i.hasNext())
      {
        i.next();
        obj.setProperty(i.key().toAscii().constData(), i.value());
      }
  }

  void setPropertyList(QObject& obj, const QList<QPair<QString, QVariant> >& lstProperties)
  {
    for (int i=0; i<lstProperties.size(); i++)
      obj.setProperty(lstProperties[i].first.toAscii().constData(), lstProperties[i].second);
  }

  bool isParent(const QObject* parent, const QObject* child)
  {
    QObject* parentObj = child->parent();
    while (parentObj != 0)
      {
        if (parentObj == parent)
          return true;
        parentObj = parentObj->parent();
      }
    return false;
  }

  QList<QObject*> findAllParents(const QObject* obj, unsigned int maxParents)
  {
    QList<QObject*> result;
    QObject* parentObj = obj->parent();
    while (parentObj != 0 && maxParents--)
      {
        result << parentObj;
        parentObj = parentObj->parent();
      }
    return result;
  }

  QObject* findCommonParent(const QObject* obj1, const QObject* obj2, int* parentIndex)
  {
    QList<QObject*> parents2 = findAllParents(obj2);
    QList<QObject*> parents1 = findAllParents(obj1);
    for (int i=0; i<parents1.size(); i++)
      {
        // this is a common parent
        if (parents2.contains(parents1[i]))
          {
            if (parentIndex)
              *parentIndex = i;
            return parents1[i];
          }
      }
    return 0;
  }

  uint qHash(const char* key)
  {
    if (key == 0)
      return 0;

    uint h = 0;
    uint g;

    while (*key) {
        h = (h << 4) + *key++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 23;
        h &= ~g;
    }
    return h;
  }


  static bool checkList(int temp, const QString& string )
  {
    QList<int> list;
    if ( string == "*" )
      {
        return true;
      }
    else
      {
        QStringList list2 = string.split(",");
        for ( int k=0; k<list2.size(); k++ )
          {
            QStringList list3 = list2[k].split("-");
            if ( list3.size() == 1 )
              list << list3[0].toInt();
            else
              {
                for ( int g=list3[0].toInt(); g<=list3[1].toInt(); g++ )
                  list << g;
              }
          }
      
      }
    if ( list.contains(temp) )
      return true;
    else
      return false;
  }

  bool matchCrontab(QStringList list, QDateTime timeStamp)
  {
    int table[6] = {timeStamp.time().minute(),
                    timeStamp.time().hour(),
                    timeStamp.date().day(),
                    timeStamp.date().month(),
                    timeStamp.date().dayOfWeek(),
                    timeStamp.date().weekNumber()};
    
    //qDebug("timeStamp : %i %i %i %i %i %i", table[0], table[1], table[2], table[3], table[4], table[5]);
    
    bool ret = false;
    for ( int i=0; i<list.size(); i++ )
      {
        QStringList list1 = list[i].split(" ");
        if ( list1.size() != 6 )
          {
            ret = false;
            break;
          }
        else
          {
            bool line = true;
            for ( int k=0; k<list1.size(); k++ )
              {
                if ( !checkList(table[k],list1[k]) )
                  {
                    line = false;
                    break;
                  }
                
              }
            ret = line;
            
          }
        
        if ( ret )
          return ret;
      }
    
    return ret;
  }

  QStringList fileList(const QString& pattern, QDir::Filters filters, QDir::SortFlags sort)
  {
    QStringList result;
    
    QDir directory = QFileInfo(pattern).dir();
    QString glob = QFileInfo(pattern).fileName();
    
    // Take the names of matching file names relative to "directory"
    QStringList names = directory.entryList(QStringList() << glob, //take all matching entries
                                            filters,
                                            sort); //sort by name
    // Add path to each name
    foreach (QString name, names)
      result << directory.path() + "/" + name;
    return result;
  }

  int findSeparator(const QString& str, QChar separator, int startIndex, QChar escape)
  {
    int index;
    // Loop until a real separator has been found
    for (;;startIndex = index+1)
      {
        index = str.indexOf(separator, startIndex);
        if (index == -1)
          break;
        int escapeIndex = index-1;
        while (escapeIndex > 0 && str[escapeIndex] == escape) --escapeIndex;
        if (((index - escapeIndex) & 1) != 0) // even number of escapes -> this is it
          break;
      }
    return index;
  }

  QStringList splitQuoted(const QString& str, QChar separator, QChar quote,
                          QString::SplitBehavior behavior)
  {
    QStringList lstResult;
    int iPos = 0;
    while (iPos < str.size())
      {
        // Part starts with quote -> find its pair
        if (str[iPos] == quote)
          {
            int iNextQuotePos = findSeparator(str, quote, iPos+1);
            // Pair was found -> strip quotes
            if (iNextQuotePos != -1)
              {
                lstResult << str.mid(iPos+1, iNextQuotePos-iPos-1);
                iPos = iNextQuotePos + 1;
                // Find the next separator
                while (iPos < str.size() && str[iPos++] != separator) ;
              }
            // No matching quote found -> use the rest of the string
            else
              {
                lstResult << str.mid(iPos);
                break;
              }
          }
        // Empty part
        else if (str[iPos] == separator)
          {
            ++iPos;
            if (behavior == QString::KeepEmptyParts)
              lstResult << "";
          }
        // Unquoted part?
        else
          {
            // Strip white space at the beginning of a quoted string.
            int iTmpPos = iPos;
            while (iTmpPos < str.size() - 1 && str[iTmpPos] == ' ') ++iTmpPos;
            if (str[iTmpPos] == quote)
              iPos = iTmpPos;
            // The string didn't start with a white space - quote sequence.
            else
              {
                iTmpPos = iPos;
                while (iTmpPos < str.size() && str[iTmpPos] != separator) ++iTmpPos;
                if (iTmpPos == str.size())
                  {
                    lstResult << str.mid(iPos);
                    break;
                  }
                else
                  {
                    lstResult << str.mid(iPos, iTmpPos-iPos);
                    iPos = iTmpPos + 1;
                  }
              }
          }
      }
    return lstResult;
  }
  
  QVariantMap decodeProperties(const QString& encodedProperties,
                               QChar propertySeparator,
                               QChar valueSeparator,
                               QChar escape,
                               PropertyDecodingFlags flags)
  {
    QVariantMap result;
    int previousIndex = 0, propertySeparatorIndex = 0;

    // Loop until all property separators have been handled
    for (; propertySeparatorIndex < encodedProperties.size(); previousIndex = propertySeparatorIndex + 1)
      {
        propertySeparatorIndex = findSeparator(encodedProperties, propertySeparator, previousIndex, escape);
        if (propertySeparatorIndex == -1)
          propertySeparatorIndex = encodedProperties.size();

        // Now separate property name and value
        QString pair = encodedProperties.mid(previousIndex, propertySeparatorIndex - previousIndex); // may be empty
        int valueSeparatorIndex = findSeparator(pair, valueSeparator, 0, escape);
        // No value separator -> ignore this value
        if (valueSeparatorIndex == -1)
          continue;

        // Take property name and unescape
        QRegExp unescape(escape == '\\' ? QString("\\\\(.)") : QString("\\%1(.)").arg(escape));
        QString name = pair.left(valueSeparatorIndex).replace(unescape, "\\1");
        if (flags & TrimPropertyName || flags & RemoveQuotes)
          name = name.trimmed();
        if (name.isEmpty())
          continue;
        if (flags & DowncasePropertyName)
          name = name.toLower();

        if (flags & RemoveQuotes && name.size() > 1 && name[0] == '"' && name[name.size()-1] == '"')
          name = name.mid(1, name.size()-2);
        if (name.isEmpty())
          continue;

        // Take property value and unescape
        QString value = pair.mid(valueSeparatorIndex+1).replace(unescape, "\\1");;
        if (flags & TrimPropertyValue || flags & RemoveQuotes)
          value = value.trimmed();
        if (flags & RemoveQuotes && value.size() > 1 && value[0] == '"' && value[value.size()-1] == '"')
          value = value.mid(1, value.size()-2);

        result[name] = value;
      }

    return result;
  }

  const char* propertyName(const QObject& obj, const QString& name)
  {
    QString lowerCaseProp = name.toLower();
    const QMetaObject* metaObj = obj.metaObject();
    for (int i=metaObj->propertyCount(); i--;)
      {
        QMetaProperty prop = metaObj->property(i);
        if (QString(prop.name()).toLower() == lowerCaseProp)
          return prop.name();
      }
    return 0;
  }

  template <class Map> void setProperties(QObject& obj, const QStringList& properties, const Map& valueMap,
                                          Qt::CaseSensitivity sensitivity,
                                          const QString& commentMark)
  {
    for (int i=0; i<properties.size(); ++i)
      {
        QString key = properties[i];
        if (key.startsWith(commentMark)) // ignore comment lines
          continue;
        if (sensitivity == Qt::CaseInsensitive)
          {
            const char* name = propertyName(obj, key);
            obj.setProperty(name, valueMap.value(key));
          }
        else
          obj.setProperty(key.toAscii(), valueMap.value(key));
      }
  }

  // Explicit instantiations.
  PII_DEFINE_EXPORTED_FUNCTION_TEMPLATE(void, setProperties<QSettings>,
                                        (QObject& obj,
                                         const QStringList& properties,
                                         const QSettings& valueMap,
                                         Qt::CaseSensitivity sensitivity,
                                         const QString& commentMark));
  
  PII_DEFINE_EXPORTED_FUNCTION_TEMPLATE(void, setProperties<QVariantMap>,
                                        (QObject& obj,
                                         const QStringList& properties,
                                         const QVariantMap& valueMap,
                                         Qt::CaseSensitivity sensitivity,
                                         const QString& commentMark));

  struct Magnitude
  {
    char symbol;
    int exponent;
  };

  double toDouble(const QString& number, bool *ok)
  {
    static const Magnitude magnitudes[] =
      { { 'Y', 24 },
        { 'Z', 21 },
        { 'E', 18 },
        { 'P', 15 },
        { 'T', 12 },
        { 'G', 9 },
        { 'M', 6 },
        { 'k', 3 },
        { 'h', 2 },
        { 'e', 1 },
        { 'd', -1 },
        { 'c', -2 },
        { 'm', -3 },
        { 'u', -6 },
        { 'n', -9 },
        { 'p', -12 },
        { 'f', -15 },
        { 'a', -18 },
        { 'z', -21 },
        { 'y', -24 }
      };

    if (number.size() > 0)
      {
        QChar cLast = number[number.size()-1];
        for (unsigned int i=0; i<sizeof(magnitudes)/sizeof(Magnitude); ++i)
          if (cLast == magnitudes[i].symbol)
            return number.left(number.size()-1).toDouble(ok) * ::pow(10.0, double(magnitudes[i].exponent));
      }
    
    return number.toDouble(ok);
  }

  void selectRandomly(QVector<int>& indices, int n, int max)
  {
    indices.clear();
    if (n <= 0)
      return;
    else if (n < max/2)
      {
        indices.reserve(n);
        int iRandom = std::rand() % max;
        // The first one cannot already be there
        indices.append(iRandom);
        // The rest can. Generate n-1 distinct indices.
        while (--n)
          {
            QVector<int>::iterator i;
            do
              {
                iRandom = std::rand() % max;
                // Binary search
                i = qLowerBound(indices.begin(),
                                indices.end(),
                                iRandom);
              }
            while (*i == iRandom);
            indices.insert(i, iRandom);
          }
      }
    else
      {
        indices.resize(max);
        Pii::generateN(indices.begin(), max, Pii::CountFunction<int>());
        if (n < max)
          {
            shuffle(indices);
            indices.resize(n);
          }
      }
  }

  QVector<int> selectRandomly(int n, int max)
  {
    QVector<int> vecResult;
    selectRandomly(vecResult, n, max);
    return vecResult;
  }

  void joinNeighbors(int index, QLinkedList<QPair<int,int> >& pairs, QList<int>& indices)
  {
    QList<int> newIndices;
    QLinkedList<QPair<int,int> >::iterator i = pairs.begin();
    while (i != pairs.end())
      {
        // If a match was found in the pair, add the other index in the
        // pair to the index list. Then remove the found match from the
        // list.
        if (i->first == index)
          {
            newIndices << i->second;
            i = pairs.erase(i);
          }
        else if (i->second == index)
          {
            newIndices << i->first;
            i = pairs.erase(i);
          }
        else
          ++i;
      }
    // Now we have found all direct neighbors of <index>. Let's find
    // (recursively) the neighbors of its neighbors.
    for (int i=0; i<newIndices.size(); ++i)
      if (newIndices[i] != index) //no need to check myself again...
        joinNeighbors(newIndices[i], pairs, indices);

    // Store all indices to a single list
    indices << newIndices;
  }

  QList<QList<int> > findNeighbors(QLinkedList<QPair<int,int> >& pairs)
  {
    QList<QList<int> > lstResult;
    
    while (!pairs.isEmpty())
      {
        QList<int> lstIndices;
        int iFirst = pairs.first().first;
        lstIndices << iFirst;
        
        // This removes all neighbors of the first item from <pairs>.
        joinNeighbors(iFirst, pairs, lstIndices);
        qSort(lstIndices);
        for (int i=lstIndices.size()-2; i>=0; --i)
          if (lstIndices[i+1] == lstIndices[i])
            lstIndices.removeAt(i);

        lstResult << lstIndices;
      }
    
    return lstResult;
  }

  QList<int> parseTypes(const QByteArray& types)
  {
    QList<int> lstResult;
    if (types.isEmpty())
      return lstResult;
    QList<QByteArray> lstTypes(types.split(','));
    for (int i=0; i<lstTypes.size(); ++i)
      {
        int type = QMetaType::type(lstTypes[i]);
        if (type == 0)
          return QList<int>();
        lstResult << type;
      }
    return lstResult;
  }
}

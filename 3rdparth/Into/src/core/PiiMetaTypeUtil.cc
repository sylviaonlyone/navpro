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

#include "PiiMetaTypeUtil.h"

#define PII_COPY_METATYPE_CASE(NAME, TYPE) \
  case QMetaType::NAME: *reinterpret_cast<TYPE*>(target) = *reinterpret_cast<const TYPE*>(source); break;

#define PII_COPY_METATYPE_CASE_N(N, PARAMS) PII_COPY_METATYPE_CASE PARAMS

#include <QStringList>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QRegExp>
#include <QBitArray>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QLine>
#include <QLineF>
#include <QPoint>
#include <QPointF>
#include <QEasingCurve>
#include <QUrl>

namespace Pii
{
  void copyMetaType(QVariant::Type type, const void* source, void* target)
  {
    switch (type)
      {
        PII_FOR_N(PII_COPY_METATYPE_CASE_N, 37,
                  ((Long,long),
                   (Int,int),
                   (Short,short),
                   (Char,char),
                   (ULong,ulong),
                   (UInt,uint),
                   (LongLong,qlonglong),
                   (ULongLong,qulonglong),
                   (UShort,ushort),
                   (UChar,uchar),
                   (Bool,bool),
                   (Float,float),
                   (Double,double),
                   (QChar,QChar),
                   (QVariantMap,QVariantMap),
                   (QVariantHash,QVariantHash),
                   (QVariantList,QVariantList),
                   (QVariant,QVariant),
                   (QByteArray,QByteArray),
                   (QString,QString),
                   (QStringList,QStringList),
                   (QBitArray,QBitArray),
                   (QDate,QDate),
                   (QTime,QTime),
                   (QDateTime,QDateTime),
                   (QUrl,QUrl),
                   (QLocale,QLocale),
                   (QRect,QRect),
                   (QRectF,QRectF),
                   (QSize,QSize),
                   (QSizeF,QSizeF),
                   (QLine,QLine),
                   (QLineF,QLineF),
                   (QPoint,QPoint),
                   (QPointF,QPointF),
                   (QRegExp,QRegExp),
                   (QEasingCurve,QEasingCurve)));
      default:
        break;
      }
  }

  bool copyMetaType(const QVariant& source, int expectedType, void** args)
  {
    // HACK See QMetaProperty::read()/write() for details
    if (int(source.type()) == expectedType)
      {
        copyMetaType(QVariant::Type(expectedType), source.constData(), args[0]);
        return true;
      }
    else if (source.type() == QVariant::UserType &&
        source.userType() == expectedType)
      {
        *reinterpret_cast<QVariant*>(args[1]) = source;
        *reinterpret_cast<int*>(args[2]) = 1;
        return true;
      }
    else if (expectedType < int(QVariant::UserType) && source.canConvert(QVariant::Type(expectedType)))
      {
        QVariant varConverted(source);
        varConverted.convert(QVariant::Type(expectedType));
        copyMetaType(QVariant::Type(expectedType), varConverted.constData(), args[0]);
        return true;
      }
    return false;
  }
  
  int scoreOverload(const QVariantList& params, const QList<int>& types)
  {
    // Overload resolution 1: parameter count must match
    if (params.size() != types.size())
      return -1;
    
    // Overload resolution 2: count matches on parameter types
    int iMatchCount = 0;
    for (int j=0; j<params.size(); ++j)
      {
        if (int(params[j].type()) == types[j] ||
            (params[j].type() == QVariant::UserType && params[j].userType() == types[j]))
          ++iMatchCount;
        else if (types[j] >= int(QVariant::UserType) ||
                 !params[j].canConvert(QVariant::Type(types[j])))
          return -1;
      }
    return iMatchCount;
  }

  QVariantList argsToList(const QList<int>& types, void** args)
  {
    QVariantList lstResult;
    for (int i=0; i<types.size(); ++i)
      lstResult << QVariant(types[i], args[i]);
    return lstResult;
  }
}

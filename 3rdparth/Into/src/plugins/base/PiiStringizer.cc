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

#include "PiiStringizer.h"

#include <PiiYdinTypes.h>
#include <PiiMatrixUtil.h>
#include <QTextStream>

PiiStringizer::Data::Data() :
  strColumnSeparator(" "),
  strRowSeparator("\n"),
  strStartDelimiter(""),
  strEndDelimiter(""),
  iPrecision(2)
{
}

PiiStringizer::PiiStringizer() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

void PiiStringizer::process()
{
  PII_D;
  PiiVariant obj = readInput();

  QString str;
  switch (obj.type())
    {
    case PiiYdin::QStringType:
      str = obj.valueAs<QString>();
      break;
      PII_INTEGER_CASES(str = intToString, obj);
      PII_FLOAT_CASES(str = floatToString, obj);
      PII_PRIMITIVE_MATRIX_CASES(str = matrixToString, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }
  emitObject(QString("%1%2%3").arg(d->strStartDelimiter).arg(str).arg(d->strEndDelimiter));
}

template <class T> QString PiiStringizer::floatToString(const PiiVariant& obj)
{
  PII_D;
  return QString("%1").arg(obj.valueAs<T>(), 0, 'f', d->iPrecision);
}

template <class T> QString PiiStringizer::intToString(const PiiVariant& obj)
{
  return QString::number(obj.valueAs<T>());
}

template <class T> QString PiiStringizer::matrixToString(const PiiVariant& obj)
{
  PII_D;
  const PiiMatrix<T> mat = obj.valueAs<PiiMatrix<T> >();
  QString strResult;
  QTextStream stream(&strResult, QIODevice::WriteOnly);
  Pii::printMatrix(stream, mat, d->strColumnSeparator, d->strRowSeparator);
  return strResult;
}

void PiiStringizer::setColumnSeparator(const QString& columnSeparator) { _d()->strColumnSeparator = columnSeparator; }
QString PiiStringizer::columnSeparator() const { return _d()->strColumnSeparator; }
void PiiStringizer::setRowSeparator(const QString& rowSeparator) { _d()->strRowSeparator = rowSeparator; }
QString PiiStringizer::rowSeparator() const { return _d()->strRowSeparator; }
void PiiStringizer::setPrecision(int precision) { _d()->iPrecision = precision; }
int PiiStringizer::precision() const { return _d()->iPrecision; }
void PiiStringizer::setStartDelimiter(const QString& startDelimiter) { _d()->strStartDelimiter = startDelimiter; }
QString PiiStringizer::startDelimiter() const { return _d()->strStartDelimiter; }
void PiiStringizer::setEndDelimiter(const QString& endDelimiter) { _d()->strEndDelimiter = endDelimiter; }
QString PiiStringizer::endDelimiter() const { return _d()->strEndDelimiter; }

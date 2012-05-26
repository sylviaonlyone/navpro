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

#include "PiiDebugOperation.h"
#include <PiiYdinTypes.h>
#include <PiiMatrixUtil.h>
#include <QDebug>
#include <PiiMath.h>

PiiDebugOperation::Data::Data() :
  iCnt(0),
  strFormat("$objectName: 0x$type received ($count since reset)\n"),
  outputStream(StdOut),
  bShowControlObjects(false)
{
}

PiiDebugOperation::PiiDebugOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output"));
}

void PiiDebugOperation::check(bool reset)
{
  PII_D;
  if (reset)
    d->iCnt = 0;
  PiiDefaultOperation::check(reset);
  d->mapVariables["objectName"] = objectName();
}

PiiFlowController* PiiDebugOperation::createFlowController()
{
  return new Controller(this);
}

void PiiDebugOperation::process()
{
  PII_D;
  PiiVariant obj = readInput();
  QString strValue;
  switch (obj.type())
    {
      PII_PRIMITIVE_MATRIX_CASES(strValue = printMatrix, obj);
      PII_PRIMITIVE_CASES(strValue = PiiYdin::numberToQString, obj);
    case PiiYdin::QStringType:
      strValue = obj.valueAs<QString>();
      break;
    }

  d->mapVariables["symbol"] = ".";
  d->mapVariables["value"] = strValue;
  d->mapVariables["count"] = QString::number(++d->iCnt);
  d->mapVariables["type"] = QString::number(obj.type(), 16);

  print();
  
  emitObject(obj);
}

void PiiDebugOperation::print()
{
  PII_D;
  QString strMessage = Pii::replaceVariables(d->strFormat, d->mapVariables);
  
  using std::cout;
  using std::cerr;
  
  switch (d->outputStream)
    {
    case StdOut: cout << piiPrintable(strMessage); cout.flush(); break;
    case StdErr: cerr << piiPrintable(strMessage); cerr.flush(); break;
    case Debug: piiDebug(strMessage);
    }
}

template <class T> QString PiiDebugOperation::printMatrix(const PiiVariant& obj)
{
  const PiiMatrix<T> mat = obj.valueAs<PiiMatrix<T> >();
  QString strMatrix;
  for (int r=0; r<qMin(mat.rows(), 20); ++r)
    {
      const T* pRow = mat[r];
      if (!strMatrix.isEmpty())
        strMatrix += '\n';
      for (int c=0; c<qMin(mat.columns(), 20); ++c)
        {
          if (c != 0)
            strMatrix += ' ';
          strMatrix += QString::number(pRow[c]);
        }
      if (mat.columns() > 20)
        strMatrix += " ...";
    }
  if (mat.rows() > 20)
    strMatrix += "\n...";

  return strMatrix;
}

void PiiDebugOperation::printControlObject(const PiiVariant& obj)
{
  PII_D;
  if (!d->bShowControlObjects)
    return;

  d->mapVariables["type"] = QString::number(obj.type(), 16);
  switch (obj.type())
    {
    case PiiYdin::SynchronizationTagType:
      if (obj.valueAs<int>() == 1)
        {
          d->mapVariables["value"] = "start tag";
          d->mapVariables["symbol"] = "<";
        }
      else
        {
          d->mapVariables["value"] = "end tag";
          d->mapVariables["symbol"] = ">";
        }
      break;
    case PiiYdin::StopTagType:
      d->mapVariables["value"] = "stop tag";
      d->mapVariables["symbol"] = "S";
      break;
    case PiiYdin::PauseTagType:
      d->mapVariables["value"] = "pause tag";
      d->mapVariables["symbol"] = "P";
      break;
    case PiiYdin::ResumeTagType:
      d->mapVariables["value"] = "resume tag";
      d->mapVariables["symbol"] = "R";
      break;
    default:
      d->mapVariables["value"] = "unidentified tag";
      d->mapVariables["symbol"] = "?";
    }
  print();
}

void PiiDebugOperation::setOutputStream(OutputStream outputStream) { _d()->outputStream = outputStream; }
PiiDebugOperation::OutputStream PiiDebugOperation::outputStream() const { return _d()->outputStream; }
void PiiDebugOperation::setFormat(const QString& format) { _d()->strFormat = format; }
QString PiiDebugOperation::format() const { return _d()->strFormat; }

void PiiDebugOperation::setShowControlObjects(bool showControlObjects) { _d()->bShowControlObjects = showControlObjects; }
bool PiiDebugOperation::showControlObjects() const { return _d()->bShowControlObjects; }

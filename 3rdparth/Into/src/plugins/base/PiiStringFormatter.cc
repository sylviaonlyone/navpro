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

#include "PiiStringFormatter.h"
#include <PiiYdinTypes.h>


PiiStringFormatter::PiiStringFormatter() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input0"));
  addSocket(new PiiOutputSocket("output"));
  setFormat("%0");
}

void PiiStringFormatter::process()
{
  QString strResult = _d()->strFormat;
  for (int i=0; i<inputCount(); ++i)
    strResult = strResult.arg(PiiYdin::convertToQString(inputAt(i)));
  emitObject(strResult);
}

void PiiStringFormatter::setFormat(const QString& format)
{
  _d()->strFormat = format;
  QRegExp reArgument("%[0-9]+");
  QList<int> lstArguments;
  int index = 0;
  // Find distinct argument numbers
  while ((index = reArgument.indexIn(format, index)) != -1)
    {
      int iArgument = reArgument.cap(0).mid(1).toInt();
      if (!lstArguments.contains(iArgument))
        lstArguments << iArgument;
      index += reArgument.matchedLength();
    }
  setNumberedInputs(qMax(lstArguments.size(), 1));
}

QString PiiStringFormatter::format() const
{
  return _d()->strFormat;
}

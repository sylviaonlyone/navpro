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

#include "PiiRegExpMatcher.h"

#include <PiiYdinTypes.h>

PiiRegExpMatcher::PiiRegExpMatcher() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("input"));
  addSocket(new PiiOutputSocket("output0"));
}

void PiiRegExpMatcher::setPattern(const QString& pattern)
{
  PII_D;
  d->re.setPattern(pattern);
  // One output for each subexpression plus one for the whole match.
  setNumberedOutputs(d->re.numCaptures()+1);
}

void PiiRegExpMatcher::process()
{
  PII_D;
  QString strInput = PiiYdin::convertToQString(inputAt(0));
  if (d->re.indexIn(strInput) > -1)
    {
      for (int i=outputCount(); i--; )
        emitObject(d->re.cap(i), i);
    }
  else
    {
      for (int i=outputCount(); i--; )
        emitObject(QString(""), i);
    }
}

QString PiiRegExpMatcher::pattern() const { return _d()->re.pattern(); }

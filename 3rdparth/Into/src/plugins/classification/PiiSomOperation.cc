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

#include "PiiSomOperation.h"
#include <PiiSerializationUtil.h>

template class PiiSomOperation::Template<PiiMatrix<float> >;
template class PiiSomOperation::Template<PiiMatrix<double> >;

PiiSomOperation::Data::Data() :
  PiiVectorQuantizerOperation::Data(PiiClassification::NonSupervisedLearner |
                                    PiiClassification::OnlineLearner),
  size(10, 10)
{
}

PiiSomOperation::PiiSomOperation(Data* dat) :
  PiiVectorQuantizerOperation(dat)
{
  PII_D;
  addSocket(d->pXOutput = new PiiOutputSocket("x"));
  //d->pXOutput->setDisplayName(tr("X coordinate of closest cell"));
  addSocket(d->pYOutput = new PiiOutputSocket("y"));
  //d->pYOutput->setDisplayName(tr("Y coordinate of closest cell"));
  protectProps();
}

void PiiSomOperation::protectProps()
{
  static const char* protectedProps[] =
    {
      "size",
      "width",
      "height",
      "topology",
      "rateFunction",
      "iterationNumber",
      "learningLength",
      "initialRadius",
      "initialLearningRate",
      "initMode",
      "learningAlgorithm"
    };
  for (unsigned i=0; i<sizeof(protectedProps)/sizeof(protectedProps[0]); ++i)
    setProtectionLevel(protectedProps[i], WriteWhenStoppedOrPaused);
}

QVariant PiiSomOperation::socketProperty(PiiAbstractSocket* socket, const char* name) const
{
  const PII_D;

  if (!PiiYdin::isNameProperty(name))
    {
      QString strPropertyName(name);
      if (socket == d->pXOutput || socket == d->pYOutput)
        {
          if (strPropertyName == "min")
            return 0;
          else if (strPropertyName == "resolution")
            return 1;
          else if (strPropertyName == "max")
            return socket == d->pXOutput ? d->size.width() : d->size.height();
          else if (strPropertyName == "displayName")
            return socket == d->pXOutput ? tr("x") : tr("y");
        }
    }

  return PiiVectorQuantizerOperation::socketProperty(socket,name);
}

void PiiSomOperation::setSize(QSize size) { _d()->size = size; }
QSize PiiSomOperation::size() const { return _d()->size; }
int PiiSomOperation::width() const { return _d()->size.width(); }
void PiiSomOperation::setWidth(int width) { _d()->size.setWidth(width); }
int PiiSomOperation::height() const { return _d()->size.height(); }
void PiiSomOperation::setHeight(int height) { _d()->size.setHeight(height); }

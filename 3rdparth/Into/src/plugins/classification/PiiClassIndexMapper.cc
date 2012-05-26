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

#include "PiiClassIndexMapper.h"
#include <PiiYdinTypes.h>
#include <PiiUtil.h>
#include <QtDebug>

PiiClassIndexMapper::PiiClassIndexMapper() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  addSocket(new PiiInputSocket("class index"));
  addSocket(new PiiOutputSocket("class index"));
  addSocket(new PiiOutputSocket("class name"));
  addSocket(new PiiOutputSocket("list index"));
}

void PiiClassIndexMapper::process()
{
  PII_D;
  PiiVariant obj = readInput();

  int index;
  switch (obj.type())
    {
      PII_NUMERIC_CASES(index = (int)PiiYdin::primitiveAs, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(inputAt(0));
    }

  int classIndex = -1, listIndex = -1;
  QString className("Unknown");
  
  if (index >= 0)
    {
      if (index < d->lstClassIndices.size())
        classIndex = d->lstClassIndices[index];
      if (index < d->lstClassNames.size())
        {
          className = d->lstClassNames[index];
          listIndex = d->lstListIndices[index];
        }
    }

  emitObject(classIndex);
  outputAt(1)->emitObject(className);
  outputAt(2)->emitObject(listIndex);
}

void PiiClassIndexMapper::setClassIndexMap(const QVariantList& classIndexMap)
{
  PII_D;
  d->lstClassIndices = Pii::variantsToList<int>(classIndexMap);
  setClassInfoMap(d->classInfoMap);
}

QVariantList PiiClassIndexMapper::classIndexMap() const
{
  const PII_D;
  return Pii::listToVariants(d->lstClassIndices);
}

void PiiClassIndexMapper::setClassInfoMap(const QVariantMap& classInfoMap)
{
  PII_D;
  d->classInfoMap = classInfoMap;
  d->lstClassNames.clear();
  d->lstListIndices.clear();
  
  QList<int> indices;
  bool bHaveIndices = false;
  if (d->classInfoMap.contains("classIndices"))
    {
      indices = Pii::variantsToList<int>(d->classInfoMap["classIndices"].toList());
      bHaveIndices = true;
    }
  QStringList names;
  if (d->classInfoMap.contains("classNames"))
    names = d->classInfoMap["classNames"].toStringList();
  
  for ( int i=0; i<d->lstClassIndices.size(); i++ )
    {
      int value = d->lstClassIndices[i];
      int index = i;
      if (bHaveIndices)
        index = indices.indexOf(value);
      
      if ( index > -1 && names.size() > index)
        d->lstClassNames.append(names[index]);
      else
        d->lstClassNames.append("Unknown");

      d->lstListIndices.append(index);
    }
}

QVariantMap PiiClassIndexMapper::classInfoMap() const { return _d()->classInfoMap; }

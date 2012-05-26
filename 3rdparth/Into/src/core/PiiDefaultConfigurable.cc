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

#include "PiiDefaultConfigurable.h"
#include "PiiMetaConfigurationValue.h"

PiiDefaultConfigurable::PiiDefaultConfigurable() :
  d(new Data)
{
}

PiiDefaultConfigurable::~PiiDefaultConfigurable()
{
  delete d;
}

void PiiDefaultConfigurable::addEnumeratedItem(const char* name,
                                               const char* description,
                                               QVariant::Type type,
                                               const QVariantList& possibleValues,
                                               const QStringList& descriptions,
                                               PiiMetaConfigurationValue::TypeFlags flags)
{
  d->lstMetaInformation << PiiMetaConfigurationValue(name,
                                                     description,
                                                     type,
                                                     possibleValues,
                                                     descriptions,
                                                     flags);
}

void PiiDefaultConfigurable::addNonEnumeratedItem(const char* name,
                                                  const char* description,
                                                  QVariant::Type type,
                                                  const QVariant& minValue,
                                                  const QVariant& maxValue,
                                                  PiiMetaConfigurationValue::TypeFlags flags)
{
  d->lstMetaInformation << PiiMetaConfigurationValue(name,
                                                     description,
                                                     type,
                                                     minValue,
                                                     maxValue,
                                                     flags);
}

int PiiDefaultConfigurable::configurationValueCount() const
{
  return d->lstMetaInformation.size();
}

PiiMetaConfigurationValue PiiDefaultConfigurable::metaInformation(int valueIndex) const
{
  if (valueIndex >=0 || valueIndex < d->lstMetaInformation.count())
    return d->lstMetaInformation[valueIndex];
  else
    return PiiMetaConfigurationValue();
}

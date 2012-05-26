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

#include "PiiConfigurable.h"
#include "PiiMetaConfigurationValue.h"

PiiConfigurable::~PiiConfigurable()
{}

QList<PiiMetaConfigurationValue> PiiConfigurable::metaInformations() const
{
  const int iCnt = configurationValueCount();
  QList<PiiMetaConfigurationValue> lstResult;
  for (int i=0; i<iCnt; ++i)
    lstResult << metaInformation(i);
  return lstResult;
}

QList<QPair<QString, QVariant> > PiiConfigurable::configurationValueList(PiiMetaConfigurationValue::TypeFlags flags) const
{
  QList<QPair<QString, QVariant> > lstResult;
  const int iCnt = configurationValueCount();
  for (int i=0; i<iCnt; ++i)
    {
      PiiMetaConfigurationValue meta(metaInformation(i));
      if (meta.isReadable() &&
          (!(flags & PiiMetaConfigurationValue::Writable) || meta.isWritable()) &&
          (!(flags & PiiMetaConfigurationValue::Stored) || meta.isStored()))
        lstResult.append(qMakePair(QString(meta.name()), configurationValue(meta.name())));
    }
  return lstResult;
}


QVariantMap PiiConfigurable::configurationValues(PiiMetaConfigurationValue::TypeFlags flags) const
{
  const int iCnt = configurationValueCount();
  QVariantMap mapResult;
  for (int i=0; i<iCnt; ++i)
    {
      PiiMetaConfigurationValue meta(metaInformation(i));
      if (meta.isReadable() &&
          (!(flags & PiiMetaConfigurationValue::Writable) || meta.isWritable()) &&
          (!(flags & PiiMetaConfigurationValue::Stored) || meta.isStored()))
        mapResult[meta.name()] = configurationValue(meta.name());
    }
  return mapResult;
}

void PiiConfigurable::setConfigurationValues(const QVariantMap& values)
{
  for (QVariantMap::const_iterator i=values.begin(); i != values.end(); ++i)
    setConfigurationValue(piiPrintable(i.key()), i.value());
}

void PiiConfigurable::setConfigurationValues(const QList<QPair<QString, QVariant> >& values)
{
  for (QList<QPair<QString, QVariant> >::const_iterator i=values.begin(); i != values.end(); ++i)
    setConfigurationValue(piiPrintable(i->first), i->second);
}

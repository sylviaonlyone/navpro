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

#ifndef _PIIDEFAULTCONFIGURABLE_H
#define _PIIDEFAULTCONFIGURABLE_H

#include "PiiConfigurable.h"
#include "PiiMetaConfigurationValue.h"
#include <QVector>
#include <QVariant>


/**
 * This class adds to the base class @p PiiConfigurable some
 * functionality related to handling the meta configuration
 * information. It contains a couple of protected functions for adding
 * new meta configuration value information for the object.
 * The protected functions are @p addEnumeratedItem() and @p
 * addNonEnumeratedItem(). Those function should be called as soon as
 * the configuration values are known for the class, e.g. in the
 * constructor of the derived class,
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiDefaultConfigurable : public PiiConfigurable
{
public:
  PiiDefaultConfigurable();
  ~PiiDefaultConfigurable();
  
  int configurationValueCount() const;
  PiiMetaConfigurationValue metaInformation(int valueIndex) const;
  
protected:
  /**
   * Add new enumerated meta configuration value to the object.
   */
  void addEnumeratedItem(const char* name,
                         const char* description,
                         QVariant::Type type,
                         const QVariantList& possibleValues,
                         const QStringList& descriptions,
                         PiiMetaConfigurationValue::TypeFlags flags = PiiMetaConfigurationValue::Default);
  
  /**
   * Add new non-enumerated meta configuration value to the object. 
   */
  void addNonEnumeratedItem(const char* name,
                            const char* description,
                            QVariant::Type type,
                            const QVariant& minValue,
                            const QVariant& maxValue,
                            PiiMetaConfigurationValue::TypeFlags flags = PiiMetaConfigurationValue::Default);

private:
  class Data
  {
  public:
    QVector<PiiMetaConfigurationValue> lstMetaInformation;
  } *d;
};

#endif //_PIIDEFAULTCONFIGURABLE_H

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

#ifndef _PIICONFIGURABLE_H
#define _PIICONFIGURABLE_H

#include <QString>
#include <QVariant>

#include "PiiGlobal.h"
#include "PiiMetaConfigurationValue.h"

/**
 * This is an interface class, which can be used for setting and
 * getting  the configuration values for the class inherited from this
 * interface.
 *
 * The configuration value can be either @p enumerated or @p
 * non-enumerated. That can be checked by the function
 * #PiiMetaConfigurationValue::isEnumerated(). The
 * PiiMetaConfigurationValue associated for the configuration value
 * can be retrieved by the function #metaInformation(). The type
 * being @p enumerated means that the value must be included in the
 * set returned by the function
 * #PiiMetaConfigurationValue::possibleValues(). The type of the
 * enumerated value can be any type supported by the class @p
 * QVariant. For example if there were configuration value for item
 * "frameSize", and the possible values for the frame size where
 * QSize(200, 320), QSize(400, 640) and QSize(760, 840), the function
 * #PiiMetaConfigurationValue::possibleValues() should return those
 * sizes, and the value given by the function setConfigurationValue()
 * must be one of those predefined sizes.
 *
 * If the configuration value is non-enumerated. Its value can be
 * enything between the minimum and maximum values determined by the
 * functions #PiiMetaConfigurationValue::minValue() and
 * #PiiMetaConfigurationValue::maxValue().
 * 
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiConfigurable
{
public: 
  virtual ~PiiConfigurable();
  
  /**
   * Sets the configuration value for the given configuration item.
   * Returns @p true if the configuration value has been succesfully set.
   * Else @p false is returned. This is an abstract function, which
   * must be implemented in the derived class.
   */
  virtual bool setConfigurationValue(const char* name, const QVariant& value) = 0;
  
  /**
   * Returns the configuration value for the given configuration item.
   * This is an abstract function, which must be implemented in
   * the derived class.
   */
  virtual QVariant configurationValue(const char* name) const = 0;

  /**
   * Returns the number of configuration values.
   */
  virtual int configurationValueCount() const = 0;

  /**
   * Returns the PiiMetaConfigurationValue object for the
   * configuration value having the index given as a parameter. The
   * parameter @p valueIndex must be within 0 and @p configurationValueCount()-1.
   */
  virtual PiiMetaConfigurationValue metaInformation(int valueIndex) const = 0;

  /**
   * Returns all configuration values as a map. The name of a
   * configuration value works as the key.
   */
  QVariantMap configurationValues(PiiMetaConfigurationValue::TypeFlags flags = PiiMetaConfigurationValue::None) const;

  QList<QPair<QString, QVariant> >
  configurationValueList(PiiMetaConfigurationValue::TypeFlags flags = PiiMetaConfigurationValue::None) const;

  void setConfigurationValues(const QVariantMap& values);
  void setConfigurationValues(const QList<QPair<QString, QVariant> >& values);
  
  /**
   * Returns meta informations for all configuration values as a list.
   */
  QList<PiiMetaConfigurationValue> metaInformations() const;
};

Q_DECLARE_INTERFACE(PiiConfigurable, "com.intopii.into.piiconfigurable/1.0");


#endif //_PIICONFIGURABLE_H

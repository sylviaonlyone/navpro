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

#ifndef _PIIMETACONFIGURATIONVALUE_H
#define _PIIMETACONFIGURATIONVALUE_H

#include <QVariant>
#include <QList>
#include <QStringList>
#include "PiiGlobal.h"

/**
 * This class provides meta information about a configuration value in
 * the class derived from the interface class #PiiConfigurable.
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiMetaConfigurationValue
{
public:
  enum TypeFlag
    {
      None = 0,
      Writable = 1,
      Readable = 2,
      Stored = 4,
      Default = -1
    };
  Q_DECLARE_FLAGS(TypeFlags, TypeFlag);
  
  /**
   * Default constructor.
   */
  PiiMetaConfigurationValue();

  /**
   * Constructor for enumerated type.
   *
   * @param name the name of the configuration value
   *
   * @param description the description of the configuration value
   *
   * @param type the type of the configuration value.
   *
   * @param possibleValues the list of possible values, which the
   * configuration value can hold.
   *
   * @param descriptions the user readable, and translatable,
   * descriptions of each possible value.
   *
   * @param flags a logical OR composition of type flags.
   */
  PiiMetaConfigurationValue(const char* name,
                            const char* description,
                            QVariant::Type type,
                            const QVariantList& possibleValues,
                            const QStringList& descriptions,
                            TypeFlags flags = Default);


  /**
   * Constructor for non-enumerated type
   *
   * @param name the name of the configuration value
   *
   * @param description the description of the configuration value
   *
   * @param type the type of the configuration value.
   *
   * @param minValue the maximum value, which the configuration value
   * can have. The type is assumed be same than the paramater @type
   * defines. The null value (QVariant()) means that no minimum value
   * exists.
   *
   * @param maxValue the minimum value, which the configuration value
   * can have. The type is assumed be same than the paramater @type
   * defines. The null value (QVariant()) means, that no maximum value
   * exists.
   *
   * @param writable tells, if the configuration value can be
   * written. The default value is @p true.
   *
   * @param readable tells, if the configuration value can be read.
   * The default value is @p true.
   *
   */ 
  PiiMetaConfigurationValue(const char* name,
                            const char* description,
                            QVariant::Type type,
                            const QVariant& minValue,
                            const QVariant& maxValue,
                            TypeFlags flags = Default);
                          
  PiiMetaConfigurationValue(const PiiMetaConfigurationValue& other);

  ~PiiMetaConfigurationValue();

  PiiMetaConfigurationValue& operator= (const PiiMetaConfigurationValue& other);

  /**
   * Returns @p true if this PiiMetaConfigurationValue represents a
   * valid configuration value, and @p false otherwise.
   */
  bool isValid() const;
  
  /**
   * Returns @p true if the type of the configuration value is
   * enumerated. Otherwise returns @p false. If the value is
   * enumerated, all possible values can be retrieved with the
   * function #possibleValues(), and the function #minValue() and
   * #maxValue() has no meaning. If the value is not enumerated, the
   * valid configuration values are between #minValue() and
   * #maxValue(), and the function @p possibleValues() has no meaning.
   */
  bool isEnumerated() const;

  /**
   * Returns @p true if this configuration value is readable, @p false
   * otherwise.
   */
  bool isReadable() const;

  /**
   * Returns @p true if this configuration value is writable, @p false
   * otherwise.
   */
  bool isWritable() const;

  /**
   * Returns @p true if this configuration value needs to be stored,
   * @p false otherwise.
   */
  bool isStored() const;

  /**
   * Returns the type of this configuration value. The return value is
   * one of the values of the QVariant::Type enumeration.
   */
  QVariant::Type type() const;

  /**
   * Returns the maximum value of the configuration value. The value
   * is returned as the same type, which is returned by the function #type().
   */
  QVariant maxValue() const;

  /**
   * Returns the minimum value of the configuration value. If the
   * configuration value is enumerated, this function has undefined
   * meaning. The value is returned as the same type, which is
   * returned by the function #type().
   */
  QVariant minValue() const;

  /**
   * Returns the name associated with the configuration value.
   */
  const char * name() const;

  /**
   * Returns the description associated with the configuration value.
   */
  const char * description() const;
  
  /**
   * If the value type is enumerated, returns the list of all possible
   * values. If the type is non-enumerated, returns an empty list.
   */
  QVariantList possibleValues() const;
  
  /**
   * Returns a list of human-representable names for enumerated
   * values.
   */
  QStringList descriptions() const;

private:
  /// @internal
  class Data : public PiiSharedD<Data>
  {
  public:
    Data();
    Data(const char* name,
         const char* description,
         QVariant::Type type,
         const QVariantList& lstPossibleValues,
         const QStringList& lstDescriptions,
         TypeFlags flags);
    Data(const char* name,
         const char* description,
         QVariant::Type type,
         const QVariant& minValue,
         const QVariant& maxValue,
         TypeFlags flags);

    static Data* sharedNull();

    const char* pName;
    const char* pDescription;
    QVariant::Type type;
    QVariantList lstPossibleValues;
    QStringList lstDescriptions;
    QVariant minValue;
    QVariant maxValue;
    TypeFlags typeFlags;
  } *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PiiMetaConfigurationValue::TypeFlags)

#endif //_PIIMETACONFIGURATIONVALUE_H

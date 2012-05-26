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

#ifndef _PIISTRINGCONVERTER_H
#define _PIISTRINGCONVERTER_H

#include <PiiDefaultOperation.h>

/**
 * Converts strings to numbers.
 *
 * @inputs
 *
 * @in input - a string to be converted (QString)
 *
 * @outputs
 *
 * @out output - converted value. Either a @p double or an @p int.
 *
 * @ingroup PiiBasePlugin
 */
class PiiStringConverter : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Conversion mode. This property controls the type of the output.
   */
  Q_PROPERTY(ConversionMode conversionMode READ conversionMode WRITE setConversionMode);
  Q_ENUMS(ConversionMode);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Conversion modes.
   *
   * @lip ConvertToInt - the input string will be converted to an @p
   * int.
   * @lip ConvertToDouble - the input string will be converted to a @p
   * double.
   */
  enum ConversionMode
  {
    ConvertToInt,
    ConvertToDouble
  };
  
  PiiStringConverter();

  void setConversionMode(ConversionMode conversionMode);
  ConversionMode conversionMode() const;

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    ConversionMode conversionMode;
  };
  PII_D_FUNC;
};

#endif //_PIISTRINGCONVERTER_H

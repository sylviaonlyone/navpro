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

#ifndef _PIIUNITCONVERTER_H
#define _PIIUNITCONVERTER_H

#include <QString>
#include "PiiGlobal.h"

class PII_CORE_EXPORT PiiUnitConverter
{
  friend class TestPiiUnitConverter;
public:
  
  /**
   * An enumeration used in determinining the unit system. The
   * following values are determined: 
   *
   * @lip Metric - metric system (m, mm)
   *
   * @lip Imperial - imperial system (inches, feet)
   */
  enum UnitSystem { Metric = 0, Imperial };

  /**
   * An enumeration used for determining the physical magnitude. The
   * following values are determined: 
   *
   * @lip Length
   *
   * @lip Weight
   *
   * @lip Volume
   */
  enum PhysicalMagnitude { Length = 0, Weight, Volume };

  /**
   * An enumeration used for determining the format of the unit, when
   * it is represented as string. The following values are determined: 
   *
   * @lip AutomaticFormat - Unit format is automatic. If this format
   * is used and @p metric system is used, the following rules apply:
   * 1) If the length is less than meter, the units are presented in
   * millimeters (e.g. 0.1mm, 469 mm, 0.001mm)  2) If the length is
   * between kilometer and meter, the length is presented in meters (
   * e.g. 890.2m, 1.234m)  3) If the length is over kilometer, it is
   * presented as in kilomteres (e.g 1202.345km). In each case the
   * precision is three decimals in the units used. If the @p imperial
   * system is used, the following rules are used: 1) If the length is
   * less than foot, it is presented in inches in the 1/16th
   * of inches (e.g. "3 1/2 inches" "3/16 inches", "10 1/8 inches". 2)
   * If the length is between mile and foot, the length is presented
   * in feet and inches (e.g. "32 feet 4 inches", "21 feet"). 3) If the
   * length is over mile, is presented in miles and in feet (e.g "3
   * miles 345 feet").
   *
   * @lip BaseUnitFormat - If this format is used, all the lengths are
   * presented in the base unit of the selected unit system. If the
   * metric system is used, the length is presented in meters in the
   * precision of three decimals (e.g. "234.453 m"). If the imperial
   * system is used, all the lenghts are presented in feet and inches
   * (e.g. "345 feet 4 inches").
   */  
  enum StringFormat { AutomaticFormat = 0, BaseUnitFormat };

  /**
   * The consturctor. The parameters:
   *
   * @param value the value in base units (e.g. feet or meter)
   *
   * @param magnitude physical magnitude (e.g. Length)
   *
   * @param system the unit system (metric, imperial)
   */
  PiiUnitConverter(double value, PhysicalMagnitude magnitude, UnitSystem system);

  /**
   * Sets the new value for the conversion. The value replaces the old
   * value set either with this function or constuctor.
   */
  void setValue(double value) { _dValue = value; }

  /**
   * Returns the current value used in conversion.
   */
  double value() const { return _dValue; }

  /**
   * Sets the precision of the inches. The precision of the inches
   * will be displayed in the precision of the inverse of the the
   * value given as the parameter. The precision of the inches has
   * only affect, when the string conversion, imperial units and the
   * physical magnitude @p Length are used. The legal values are 1 and
   * any power of two. The default value is 16. It is checked in the
   * function, whether the new value is the power of
   * two. If it is not, the value is set to default value 16.
   */
  void setInchPrecision(int precision);

  /**
   * Returns the current precision of inches.
   */
  int inchPrecision() { return _iInchPrecision; }
  
  /**
   * Makes the conversion for the value set by the constructor or by
   * the function @p setValue() from the unit system defined in the
   * constructor to the system given as a parameter.
   */
  double convert(UnitSystem to) const;

  /**
   * Works in a similar way than the function @p convert(), exept the
   * result is presented as a string. The parameter @p StringFormat
   * defines the format of the string returned. Refer to the
   * documentation of the enum @p StringFormat to get more information
   * on that.
   */  
  QString toString(StringFormat format, UnitSystem to) const;

private:
  // The minimum fraction of inch. Should be power of two (e.g. 16).
  int _iInchPrecision;
  // The value in the base units (meter, feet).
  double _dValue;
  PhysicalMagnitude _magnitude;
  UnitSystem _system;

  double fromImperialToMetric() const;
  double fromMetricToImperial() const;

  QString metricToString(StringFormat format, double value) const;
  QString imperialToString(StringFormat format, double value) const;
  QString metricAsAutomaticFormat(double value) const;
  QString metricAsBaseUnitFormat(double value) const;
  QString imperialAsAutomaticFormat(double value) const;
  QString imperialAsBaseUnitFormat(double value) const;
  QString imperialAsInches(double dInches) const;
  QString imperialAsFeetAndInches(double dFeet) const;
  QString imperialAsMilesAndFeet(double dMiles) const;
  static QString toHierarcicalUnits(double biggerUnitValue,
                                    int maxSmallerUnits,
                                    int maxBiggerUnits,
                                    bool useFractions,
                                    const QString& biggerUnitNameZero,
                                    const QString& biggerUnitNameOne,
                                    const QString& biggerUnitNamePlural,
                                    const QString& smallerUnitNameZero,            
                                    const QString& smallerUnitNameOne,
                                    const QString& smallerUnitNamePlural,
                                    const QString& biggestUnitNameOne );
  static void reduce(int& nominator, int& denominator);
};

#endif //_PIIUNITCONVERTER_H

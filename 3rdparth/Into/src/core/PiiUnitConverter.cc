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

#include "PiiUnitConverter.h"
#include "PiiMath.h" // round()
#include <QtDebug>
#include <QCoreApplication> // For translator

#define PII_FOOT_TO_METERS_RATIO 0.3048
#define PII_FOOT_TO_INCHES_RATIO 12
#define PII_MILE_TO_FEET_RATIO 5280

PiiUnitConverter::PiiUnitConverter(double value, PhysicalMagnitude magnitude, UnitSystem system) :
  _iInchPrecision(16), _dValue(value), _magnitude(magnitude), _system(system)
{
  //qDebug("PiiUnitConverter::PiiUnitConverter() BEGIN value = %f", value);
  //qDebug("PiiUnitConverter::PiiUnitConverter() END value = %f", value);
}

void PiiUnitConverter::setInchPrecision(int precision)
{
  //qDebug("PiiUnitConverter::setInchPrecision() BEGIN precision = %d", precision);
  if (precision != 1 && precision % 2 != 0)
    precision = 16;
  _iInchPrecision = precision;
  //qDebug("PiiUnitConverter::setInchPrecision() END precision = %d", precision);
}

double PiiUnitConverter::convert(UnitSystem to) const
{
  //qDebug("PiiUnitConverter::convert(), _dValue = %f", _dValue);
  switch(_system)
    {
    case Metric:
      switch (to)
        {
        case Metric:
          // No conversion needed
          return _dValue;
          //break;
        case Imperial:
          return fromMetricToImperial();
          break;
        }
      break;
    case Imperial:
      switch (to)
        {
        case Metric:
          return fromImperialToMetric();
          break;
        case Imperial:
          // No conversion needed
          return _dValue;
          //break;
        }
      break;
    }
  return _dValue;
}

//TODO: Implement conversions also for magnitudes Weight and Volume.
/* This is a private helper function, which converts the unit value
 * stored in member variable _dValue to the metric value. Currently
 * conversion only for physical magnitude Length is impelemented.
 * For other magnitudes (Weight, Volume) the member variable _dValue
 * is returned direclty.
 */
double PiiUnitConverter::fromImperialToMetric() const
{
  //qDebug("PiiUnitConverter::fromImperialToMetric(), _dValue = %f", _dValue);
  switch (_magnitude)
    {
    case Length:
      return _dValue*double(PII_FOOT_TO_METERS_RATIO);
      break;
    case Weight:
      // TODO: implement this
      return _dValue;
      break;
    case Volume:
      // TODO: implement this
      return _dValue;
      break;
    }
  return _dValue;
}

//TODO: Implement conversions also for magnitudes Weight and Volume.
/* This is a private helper function, which converts the unit value
 * stored in member variable _dValue to the imperial value. Currently
 * conversion only for physical magnitude Length is impelemented.
 * For other magnitudes (Weight, Volume) the member variable _dValue
 * is returned direclty.
 */
double PiiUnitConverter::fromMetricToImperial() const
{
  //qDebug("PiiUnitConverter::fromMetricToImperial()");
  switch (_magnitude)
    {
    case Length:
      return _dValue/PII_FOOT_TO_METERS_RATIO;
      break;
    case Weight:
      // TODO: implement this
      return _dValue;
      break;
    case Volume:
      // TODO: implement this
      return _dValue;
      break;
    default:
      return _dValue;
    }
  return _dValue;
}

QString PiiUnitConverter::toString(StringFormat format, UnitSystem to) const
{
  //qDebug("PiiUnitConverter::toString(), _dValue = %f", _dValue);  
  switch (to)
    {
    case Metric:
      return metricToString(format, convert(to));
      break;
    case Imperial:
      return imperialToString(format, convert(to));
      break;
    }
  return QString("");
}

/* This is a private helper function, which converts the value in
 * metric system to the string. The parameter format defines the
 * format of the string returned (automatic, baseUnit). */
QString PiiUnitConverter::metricToString(StringFormat format, double value) const
{
  //qDebug("PiiUnitConverter::metricToString()");
  switch (format)
    {
    case AutomaticFormat:
      return metricAsAutomaticFormat(value);
      break;
    case BaseUnitFormat:
      return metricAsBaseUnitFormat(value);
      break;
    }
  return QString("");
}

/* This is a private helper function, which converts the value in
 * imperial system to the string. The parameter format defines the
 * format of the string returned (automatic, baseUnit). */
QString PiiUnitConverter::imperialToString(StringFormat format, double value) const
{
  //qDebug("PiiUnitConverter::imperialToString()");
  switch (format)
    {
    case AutomaticFormat:
      return imperialAsAutomaticFormat(value);
      break;
    case BaseUnitFormat:
      return imperialAsBaseUnitFormat(value);
      break;
    } 
  return QString("");
}

/* This is a private helper function, which converts the value in
 * metric system to the string in the automatic format. */
QString PiiUnitConverter::metricAsAutomaticFormat(double value) const
{
  //qDebug("PiiUnitConverter::metricAsAutomaticFormat()");
  if (value < 1.0) 
    {
      // Less than one meter
      return QCoreApplication::translate("PiiUnitConverter", "%1 mm", "millimeter(s)").arg(value*1000.0, 0, 'f', 1);
    }
  else if (value < 1000.0)
    {
      // Between kilometer and one meter.
      return QCoreApplication::translate("PiiUnitConverter", "%1 m", "meter(s)").arg(value, 0, 'f', 1);
    }
  else 
    { // value >= 1000.0, over kilometer
      return QCoreApplication::translate("PiiUnitConverter", "%1 km", "kilometer(s)").arg(value/1000.0, 0, 'f', 2);
    }
}

/* This is a private helper function, which converts the value in
 * metric system to the string in the base unit format. */
QString PiiUnitConverter::metricAsBaseUnitFormat(double value) const
{
  //qDebug("PiiUnitConverter::metricAsBaseUnitFormat()");
  return QString("%1 m").arg(value, 0, 'f', 1);
}

/* This is a private helper function, which converts the value in
 * imperial system to the string in the automatic format. */
QString PiiUnitConverter::imperialAsAutomaticFormat(double value) const
{
  //qDebug("PiiUnitConverter::imperialAsAutomaticFormat()");
   if (value < 1.0)
    {
      // get the result as inches and fractions of inches.
      return imperialAsInches(value*double(PII_FOOT_TO_INCHES_RATIO));
    }
   else if (value < double(PII_MILE_TO_FEET_RATIO))
    {
      // get the result as feet and iches
      return imperialAsFeetAndInches(value);
    }
  else // value >= double(PII_MILE_TO_FEET_RATIO)
    {
      // get the result as miles and feet.
      return imperialAsMilesAndFeet(value/(double(PII_MILE_TO_FEET_RATIO)));
    } 
}

/* This is a private helper function, which converts the value in
 * imperial system to the string in the base unit format. */
QString PiiUnitConverter::imperialAsBaseUnitFormat(double value) const
{
  //qDebug("PiiUnitConverter::imperialAsBaseUnitFormat()");
  return imperialAsFeetAndInches(value);
}

/* This is a private helper function, which converts the value in
 * feet given as a parameter to inches and fractions of inches as
 * string. The examples of conversions:
 *
 *     value            conversion
 *     -----            ----------
 *     1.0              1 inch
 *     5.0              5 inches
 *     0.00001          0 inches
 *     3.0/16.0         3/16 inches
 *     4.0 + 3.0/16.0   4 3/16 inches
 */
QString PiiUnitConverter::imperialAsInches(double dInches) const
{

  //qDebug("PiiUnitConverter::imperialAsInches() BEGIN, dInches = %f", dInches);

  return toHierarcicalUnits(dInches,
                            _iInchPrecision,
                            PII_FOOT_TO_INCHES_RATIO,
                            true,
                            QCoreApplication::translate("PiiUnitConverter", "inches", "zero inches"),
                            QCoreApplication::translate("PiiUnitConverter", "inch", "one inch"),
                            QCoreApplication::translate("PiiUnitConverter", "inches", "more than one inches"),
                            "",
                            "",
                            "",
                            QCoreApplication::translate("PiiUnitConverter", "foot", "one foot"));

  // REMOVE >>
//   int iInches = int(dInches);
//   qDebug("PiiUnitConverter::imperialAsInches() iInches = %d", iInches);
//   double decimals;
//   decimals = fabs(dInches - double(iInches));
//   qDebug("PiiUnitConverter::imperialAsInches() decimals = %f", decimals);
//   QString strInches = QString("");
//   QString strFractions = QString("");
//   QString strUnit;
//   int numerator = int(round(decimals*double(_iInchPrecision)));
//   qDebug("PiiUnitConverter::imperialAsInches() numerator = %d", numerator);
//   if (numerator > _iInchPrecision-1)
//     {
//       iInches++;
//     }
//   else if (numerator > 0)
//     {
//       int denominator = _iInchPrecision;
//       qDebug("PiiUnitConverter::imperialAsInches() denominator = %d", denominator);
//       reduce(numerator, denominator);
//       qDebug("PiiUnitConverter::imperialAsInches(), after deducing numerator = %d, denominator = %d", numerator, denominator);
//       strFractions = QString("%1/%2").arg(numerator).arg(denominator);
//     }

//   QString retString;
//   if (iInches == 1)
//     {
//       strInches = QString("1");
//       strUnit = QString(" inch");
//     }
//   else
//     {
//       strUnit = QString(" inches");
//       if (iInches >0)
//         strInches = QString("%1").arg(iInches);
//       else if (iInches == 0 && numerator == 0)
//         strInches = QString("0");
//     }

//   if (!strInches.isEmpty() && !strFractions.isEmpty())
//     strInches += ' ';
  
//   qDebug() << "PiiUnitConverter::imperialAsInches() RETURNING" << strInches + strFractions + strUnit;
//   return strInches + strFractions + strUnit;
  // REMOVE <<
}

/* This is a private helper function, which converts the value in
 * feet given as a parameter to feet and inches as a string. The
 * examples of conversions:
 *
 *     value                             conversion
 *     -----                             ----------
 *     1.23                              1 foot 3 inches
 *     1.0/12.0                          1 inch
 *     5.0*1.0/12.0                      5 inches
 *     3.0+5.0*1.0/12.0                  3 feet 5 inches
 */
QString PiiUnitConverter::imperialAsFeetAndInches(double dFeet) const
{
  //qDebug("PiiUnitConverter::imperialAsFeetAndInches() BEGIN");
  return toHierarcicalUnits(dFeet,
                            PII_FOOT_TO_INCHES_RATIO,
                            PII_MILE_TO_FEET_RATIO,
                            false,
                            QCoreApplication::translate("PiiUnitConverter", "feet", "zero feet"),
                            QCoreApplication::translate("PiiUnitConverter", "foot", "one foot"),
                            QCoreApplication::translate("PiiUnitConverter", "feet", "more than one feet"),
                            QCoreApplication::translate("PiiUnitConverter", "inches", "zero inches"),
                            QCoreApplication::translate("PiiUnitConverter", "inch", "one inch"),
                            QCoreApplication::translate("PiiUnitConverter", "inches", "more than one inches"),
                            QCoreApplication::translate("PiiUnitConverter", "mile", "one mile"));
                            
  // REMOVE >>                
//   int iFeet = int(dFeet);
//   double decimals;
//   decimals = fabs(dFeet - double(iFeet));
//   QString strInches = QString("");
//   QString strFeet = QString("");
//   int inches = int(round(decimals*double(PII_FOOT_TO_INCHES_RATIO)));
//   if (inches > PII_FOOT_TO_INCHES_RATIO-1)
//     {
//       iFeet++;
//     }
//   else if (inches == 1)
//     {
//       strInches = QString("1 inch");
//     }
//   else if (inches >0)
//     {
//       strInches = QString("%1 inches").arg(inches);
//     }

//   if (iFeet > 1)
//     {
//       strFeet = QString("%1 feet").arg(iFeet);
//     }
//   else if (iFeet == 1)
//     {
//       strFeet = QString("1 foot");
//     }

//   // Add the space between the feet and inches, if needed.
//   if (!strFeet.isEmpty() && !strInches.isEmpty())
//     strFeet += ' ';

//   qDebug() << "PiiUnitConverter::imperialAsFeetAndInches() RETURNING" << strFeet + strInches;
//   return strFeet + strInches;
  // REMOVE
}

QString PiiUnitConverter::imperialAsMilesAndFeet(double dMiles) const
{
  //qDebug("PiiUnitConverter::imperialAsMilesAndFeet()");
  return toHierarcicalUnits(dMiles,
                            PII_MILE_TO_FEET_RATIO,
                            0,
                            false,
                            QCoreApplication::translate("PiiUnitConverter", "miles", "zero miles"),
                            QCoreApplication::translate("PiiUnitConverter", "mile", "one mile"),
                            QCoreApplication::translate("PiiUnitConverter", "miles", "more than one miles"),
                            QCoreApplication::translate("PiiUnitConverter", "feet", "zero feet"),
                            QCoreApplication::translate("PiiUnitConverter", "foot", "one foot"),
                            QCoreApplication::translate("PiiUnitConverter", "feet", "more than one feet"),
                            ""); 

  // REMOVE >>
//   int iMiles = int(dMiles);
//   double decimals;
//   decimals = fabs(dMiles - double(iMiles));
//   QString strFeet = QString("");
//   QString strMiles = QString("");
//   int feet = int(round(decimals*double(PII_MILE_TO_FEET_RATIO)));
//   if (feet > PII_MILE_TO_FEET_RATIO-1)
//     {
//       iMiles++;
//     }
//   else if (feet == 1)
//     {
//       strFeet = QString("1 foot");
//     }
//   else if (feet > 0)
//     {
//       strFeet = QString("%1 feet").arg(feet);
//     }
  
//   if (iMiles > 1)
//     {
//       strMiles = QString("%1 miles").arg(iMiles);
//     }
//   else if (iMiles == 1)
//     {
//       strMiles = QString("1 mile");
//     }

//   if (!strMiles.isEmpty() && !strFeet.isEmpty())
//     strMiles += ' ';
//   qDebug() << "PiiUnitConverter::imperialAsMilesAndFeet() RETURNING" << strMiles + strFeet;
//   return strMiles + strFeet;
  // REMOVE <<
}

/*
 * This is a private utility function, which converts the numerical
 * unit value to some hierarcical units as a string. For example 3.1667
 * feet is converted to "3 feet 2 inches". The function handles the
 * bigger and
 * smaller units. In the previous example feets are bigger units and
 * the inches is smaller units. The function works in two different
 * modes: in a "normal mode" and in a "fractions" mode. In the previous
 * example, normal mode was used. In the "fractions" mode the smaller
 * units can be shown as a fractions of the bigger units, e.g. 4.1875
 * inches is converted to "4 3/16 inches". In the "fractions" only the
 * bigger units are shown, at the end of the string, like in the
 * previous example, where inches were the bigger units.
 *
 * The parameters:
 *
 * @lip biggerUnitValue (double) - value of the (bigger) unit to be converted.
 *
 * @lip maxSmallerUnits (int) - maximum amount of the smaller units (e.g. 12
 * for inches).
 *
 * @lip maxBiggerUnits (int) - miximum amount of the bigger untis. If the
 * maximum amount of bigger units is reached, the @p biggestUnitName
 * is shown with one unit (e.g. "1 mile", if biggest unit name is
 * "mile", bigger unit is "feet", @p maxBiggerUnits is 5280, and the
 * amount of feets exceeds 5280). If amount of maxBiggerUnits is zero
 * or smaller, it is abandoned.
 *
 * @lip useFractions (bool) - determines the mode of the function.
 * With the @p true value, the function is in "fractions" mode. Else
 * the "normal mode" is used. In fractions mode the smaller units are
 * shown with the fractions, and only the bigger units are shown at the
 * end of the string (e.g. "3 1/8 inches"). The fractions are shown in
 * the precision of @p maxSmallerUnits reduced to the smallest possible
 * denominator. If "fractions" mode is used, the value of the
 * parameter maxSmallerUnits should be power of two. In the "normal
 * mode" the bigger unit name is shown right after the bigger unit
 * value and the smaller unit name is shown right after the smaller
 * unit value (e.g 12 feet 3 inches).
 *
 * @lip biggerUnitNameZero (QString) - the name of the  bigger unit,
 * when the amount of the bigger units is zero (e.g. "feet").
 *
 * @lip biggerUnitNameOne (QString) - the singular format of the bigger
 * unit name (e.g. "foot"). Is used, when the amount of the bigger
 * units is exactly 1.
 *
 * @lip biggerUnitNamePlural (QString) - the plural format of the
 * bigger unit name (e.g. "feet"). Is used, when the amount of bigger
 * unit is greater than 1.). This format is also used, when the
 * "fractions" mode is in use, and the amount of smaller units is not
 * equal to zero, regradless of what is the amount of bigger units
 * (e.g. "1 3/8 feet").
 *
 * @lip smallerUnitNameZero (QString) - the name of the smaller unit,
 * when the amount of the smaller unit is zero (e.g. "inches").
 *
 * @lip smallerUnitNameOne (QString) - the singular format of the smaller
 * unit name (e.g. "inch"). I used, when the amount of smaller units
 * is exactly 1.
 *
 * @lip smallerUnitNamePlural (QString) - the plural format of the
 * smaller unit name (e.g. "inches"). This is used, when the amount of
 * the smaller units is greater than one.
 *
 * @lip biggestUnitNameOne (QString) - the biggest unit name. This is
 * used, when the amount of bigger units exeeds @p maxBiggerUnits. If
 * value of  @p maxBiggerUnits is zero, this parameter is not used in
 * the function. (e.g. "1 mile" is returned, whent the smaller units
 * in feet exeeds 5280).
 * 
 */
QString PiiUnitConverter::toHierarcicalUnits(double biggerUnitValue,
                                             int maxSmallerUnits,
                                             int maxBiggerUnits,
                                             bool useFractions,
                                             const QString& biggerUnitNameZero,
                                             const QString& biggerUnitNameOne,
                                             const QString& biggerUnitNamePlural,
                                             const QString& smallerUnitNameZero,
                                             const QString& smallerUnitNameOne,
                                             const QString& smallerUnitNamePlural,
                                             const QString& biggestUnitNameOne )
{
  //qDebug("PiiUnitConverter::toHierarcicalUnits() BEGIN");
  int iBiggerUnitValue = int(biggerUnitValue);
  double decimals;
  decimals = fabs(biggerUnitValue - double(iBiggerUnitValue));
  QString strSmallUnit = QString("");
  QString strBigUnit = QString("");
  QString strFractions = QString("");
  int iSmallerUnitValue = int(Pii::round(decimals*double(maxSmallerUnits)));
  if (iSmallerUnitValue >= maxSmallerUnits)
    {
      iBiggerUnitValue++;
      if (maxBiggerUnits > 0)
        {
          // The "overflow" of the bigger units. If the bigger unit
          // were foot, the result will be "1 mile".
          if (iBiggerUnitValue >= maxBiggerUnits)
            return QString("1 ") + biggestUnitNameOne;
        }
    }
  else if (iSmallerUnitValue > 0)
    {
      if (useFractions)
        {
          int numerator = iSmallerUnitValue;
          int denominator = maxSmallerUnits;
          reduce(numerator, denominator);
          strFractions = QString("%1/%2").arg(numerator).arg(denominator);
          // the content of strSmallUnits remain empty
        }
      else
        {
          strSmallUnit = QString("%1 %2").arg(iSmallerUnitValue).arg(iSmallerUnitValue == 1 ? smallerUnitNameOne : smallerUnitNamePlural);
          // the content of strFractions remain empty
        }
     }
  // The case iSmallerUnitValue == 0 will be handled below

  //qDebug("PiiUnitConverter::toHierarcicalUnits(), iBiggerUnitValue = %d", iBiggerUnitValue);
  //qDebug("PiiUnitConverter::toHierarcicalUnits(), iSmallerUnitValue = %d", iSmallerUnitValue);
  //qDebug() << "PiiUnitConverter::toHierarcicalUnits(), strFractions = " << strFractions;

  // If we go neither in if or else branch, no bigger units are shown.
  // (E.g 3 inches).
  if (iBiggerUnitValue == 0)
    {
      if (iSmallerUnitValue == 0)
        {
          // The zero must be returned.
          if (useFractions)
            return QString("0 ") + biggerUnitNameZero; // E.g. 0 inches.
          //return QString("0 ") + biggerUnitNamePlural;
          else
            return QString("0 ") + smallerUnitNameZero;  // E.g. 0 feet
          //return QString("0 ") + smallerUnitName;
        }
      if (!strFractions.isEmpty())
        // This is the case where the fractions are used, and there
        // are no integer part in the unit (e.g. "3/12 feet".
        return QString("%1 %2").arg(strFractions).arg(biggerUnitNamePlural);
    }
  else
    {
      if (!strFractions.isEmpty())
        strFractions.prepend(' ');
      strBigUnit = QString("%1%2 %3").arg(iBiggerUnitValue).arg(strFractions).arg(iBiggerUnitValue == 1 && strFractions.isEmpty() ? biggerUnitNameOne : biggerUnitNamePlural);
      // If for example small units were inches and big units feet,
      // content of strBigUnit is now either 1 feet or 3 1/4 feet.
    }

  if (useFractions)
    // The smaller units are not needed, when fractions are used.
    return strBigUnit;
  
  if (!strBigUnit.isEmpty() && !strSmallUnit.isEmpty())
    strBigUnit += ' ';
  //qDebug() << "PiiUnitConverter::toHierarcicalUnit() RETURNING" << strBigUnit + strSmallUnit;
  return strBigUnit + strSmallUnit;
}

/* This private function reduces the fractional numer having
   denominator power of two. The reduced numerator and denominator are
   returned via the parameters.*/
void PiiUnitConverter::reduce(int& numerator, int& denominator)
{
  //qDebug("PiiUnitConverter::reduce() BEGIN numerator == %d, denominator == %d", numerator, denominator);

  while (numerator % 2 == 0)
    {
      // Reduce
      numerator /= 2;
      denominator /= 2;
    }

  //qDebug("PiiUnitConverter::reduce() END numerator == %d, denominator == %d", numerator, denominator);
}


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

#ifndef _PIISTRINGIZER_H
#define _PIISTRINGIZER_H

#include <PiiDefaultOperation.h>

/**
 * Convert (almost) any type to a string.
 *
 * @inputs
 *
 * @in input - any number or a matrix containing numbers
 * 
 * @outputs
 *
 * @out output - the input value as a string. Matrices are represented
 * as shown in the example below.
 *
@verbatim
0 1 2
3 4 5
@endverbatim
 *
 * @ingroup PiiBasePlugin
 */
class PiiStringizer : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Column separator for matrices. The default value is " ".
   */
  Q_PROPERTY(QString columnSeparator READ columnSeparator WRITE setColumnSeparator);

  /**
   * Row separator for matrices. Default value is "\n".
   */
  Q_PROPERTY(QString rowSeparator READ rowSeparator WRITE setRowSeparator);

  /**
   * This will be prepended to the output string. The default value is
   * an empty string.
   */
  Q_PROPERTY(QString startDelimiter READ startDelimiter WRITE setStartDelimiter);

  /**
   * This will be appended to the end of the string. The default value
   * is an empty string.
   */
  Q_PROPERTY(QString endDelimiter READ endDelimiter WRITE setEndDelimiter);
  
  /**
   * The maximum number of digits after the decimal point shown in a
   * floating-point number. The default value is 2.
   */
  Q_PROPERTY(int precision READ precision WRITE setPrecision);  

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiStringizer();

  void setColumnSeparator(const QString& columnSeparator);
  QString columnSeparator() const;
  
  void setRowSeparator(const QString& rowSeparator);
  QString rowSeparator() const;
  
  void setPrecision(int precision);
  int precision() const;
  
  void setStartDelimiter(const QString& startDelimiter);
  QString startDelimiter() const;
  
  void setEndDelimiter(const QString& endDelimiter);
  QString endDelimiter() const;
  
protected:
  void process();

private:
  template <class T> QString intToString(const PiiVariant& obj);
  template <class T> QString floatToString(const PiiVariant& obj);
  template <class T> QString matrixToString(const PiiVariant& obj);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    QString strColumnSeparator;
    QString strRowSeparator;
    QString strStartDelimiter;
    QString strEndDelimiter;
    int iPrecision;
  };
  PII_D_FUNC;
};


#endif //_PIISTRINGIZER_H

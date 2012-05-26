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

#ifndef _PIIAVERAGEOPERATION_H
#define _PIIAVERAGEOPERATION_H

#include <PiiDefaultOperation.h>

/**
 * Calculate the average of all values in a matrix.
 *
 * @inputs
 *
 * @in matrix - any numeric or complex matrix
 * 
 * @outputs
 *
 * @out average - the mean value, either @p double or @p
 * complex<double>. If an average type is meanRows or meanColumns,
 * then output type is PiiMatrix<double> or PiiMatrix<complex<double> >.
 *
 * @ingroup PiiBasePlugin
 */
class PiiAverageOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * CalculationMode is a type of the average calculation. Default is
   * MeanAll.
   */
  Q_PROPERTY(CalculationMode calculationMode READ calculationMode WRITE setCalculationMode);
  Q_ENUMS(CalculationMode);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiAverageOperation();

  /**
   * Calculation modes.
   *
   * @lip MeanAll - output the mean of all values (double or double
   * complex)
   *
   * @lip MeanRows - output the means of all rows (column matrix with
   * double or double complex data)
   *
   * @lip MeanColumns - output the means of all columns (row matrix
   * with double or double complex data)
   */
  enum CalculationMode
    {
      MeanAll,
      MeanRows,
      MeanColumns
    };
  
  void setCalculationMode(const CalculationMode& calculationMode);
  CalculationMode calculationMode() const;

protected:
  void process();

private:
  template <class T> void average(const PiiVariant& obj);
  template <class T> void complexAverage(const PiiVariant& obj);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    CalculationMode calculationMode;
  };
  PII_D_FUNC;
};


#endif //_PIIAVERAGEOPERATION_H

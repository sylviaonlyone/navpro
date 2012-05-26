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

#ifndef _PIIMATRIXNORMALIZER_H
#define _PIIMATRIXNORMALIZER_H

#include <PiiDefaultOperation.h>
#include <PiiMatrix.h>

/**
 * Scale the values in a matrix so that its values are limited to a
 * predefined range.
 *
 * @inputs
 *
 * @in input - the input matrix. Any numeric matrix.
 * 
 * @outputs
 *
 * @out output - the output matrix. The output type is determined by
 * the #outputType property.
 *
 * @ingroup PiiBasePlugin
 */
class PiiMatrixNormalizer : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The minimum value in the output matrix. Used in @p
   * NormalizeMinMax mode. Default is zero.
   */
  Q_PROPERTY(double min READ min WRITE setMin);
  /**
   * The minimum value in the output matrix. Used in @p
   * NormalizeMinMax mode. Default is one.
   */
  Q_PROPERTY(double max READ max WRITE setMax);
  /**
   * Mean of the output matrix. Used in @p NormalizeMeanVar mode.
   * Default is zero.
   */
  Q_PROPERTY(double mean READ mean WRITE setMean);
  /**
   * Variance of the output matrix. Used in @p NormalizeMeanVar mode
   * if non-zero. If @p variance is set to zero, only the mean value
   * will be changed. Default is one.
   */
  Q_PROPERTY(double variance READ variance WRITE setVariance);
  /**
   * The output type. See @ref PiiYdin::MatrixTypeId for valid values. 
   * Only numeric matrix types are allowed. The default is @p
   * PiiYdin::DoubleMatrixType.
   */
  Q_PROPERTY(int outputType READ outputType WRITE setOutputType);
  
  /**
   * Normalization mode. Default is @p NormalizeMinMax.
   */
  Q_PROPERTY(NormalizationMode normalizationMode READ normalizationMode WRITE setNormalizationMode);
  Q_ENUMS(NormalizationMode);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Normalization modes.
   *
   * @lip NormalizeMinMax - scale the input matrix to fixed minimum
   * and maximum values.
   *
   * @lip NormalizeMeanVar - scale the input matrix to fixed mean and
   * variance.
   */
  enum NormalizationMode { NormalizeMinMax, NormalizeMeanVar };
  
  PiiMatrixNormalizer();

  void check(bool reset);
  
  void setMin(double min);
  double min() const;
  void setMax(double max);
  double max() const;
  void setOutputType(int outputType);
  int outputType() const;
  void setMean(double mean);
  double mean() const;
  void setVariance(double variance);
  double variance() const;
  void setNormalizationMode(NormalizationMode normalizationMode);
  NormalizationMode normalizationMode() const;

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dMin;
    double dMax;
    double dMean;
    double dVariance;
    int iOutputType;
    NormalizationMode normalizationMode;
  };
  PII_D_FUNC;

  template <class T> void normalize(const PiiVariant& obj);
  template <class T> PiiMatrix<double> normalizeAs(const PiiMatrix<T>& matrix,
                                                   double preShift,
                                                   double scale,
                                                   double postShift);

  template <class T> inline void emitMatrix(PiiMatrix<double>& matrix)
  {
    emitObject(PiiMatrix<T>(matrix));
  }
  
  void throwOutputTypeError()
  {
    PII_THROW(PiiExecutionException, tr("Only numeric matrices are allowed as the output type."));
  }

};

#endif //_PIIMATRIXNORMALIZER_H

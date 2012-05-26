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

#ifndef _PIIGRAYFEATUREOPERATION_H
#define _PIIGRAYFEATUREOPERATION_H

#include <PiiDefaultOperation.h>

#define PII_GRAY_FEATURE_OPERATION_FEATURECNT 9

/**
 * Extracts a configurable set of simple gray-level features from
 * images.
 *
 * @inputs
 *
 * @in image - input image. Any gray-level image.
 * 
 * @outputs
 *
 * @out features - extracted features. (1xN PiiMatrix<float>)
 *
 * @ingroup PiiFeaturesPlugin
 */
class PiiGrayFeatureOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Enabled features. List here all features you want to enable. @p
   * Average, @p Minimum, and @p Maximum are enabled by default. If
   * the list is empty, @p Minimum will be used.
   *
   * @lip Minimum - minimum gray level
   * @lip Maximum - maximum gray level
   * @lip Average - average gray level
   * @lip Contrast - difference between maximum and minimum
   * @lip MaxDiff - maximum absolute difference to the reference value
   * (#reference).
   * @lip Variance - variance
   * @lip Deviation - standard deviation
   * @lip HighAverage - average of gray levels over the mean
   * @lip LowAverage - average of gray levels below the mean
   *
   * @code
   * pGrayFeatures->setProperty("features", QStringList() << "Average" << "Contrast");
   * @endcode
   *
   * Independent of the order of feature names in this list, the order
   * of features in the feature vector will be that listed above. If
   * the list contains "MaxDiff" and "Minimum", in this order, the
   * order of features in the resulting feature vector will be just
   * the opposite.
   */
  Q_PROPERTY(QStringList features READ features WRITE setFeatures);

  /**
   * Reference gray value. Maximum absolute difference is calculated
   * by subtracting this value from each pixel. The default value is
   * 127.
   */
  Q_PROPERTY(double reference READ reference WRITE setReference);
  
  PII_OPERATION_SERIALIZATION_FUNCTION

public:
  PiiGrayFeatureOperation();
  ~PiiGrayFeatureOperation();
  
  void setFeatures(const QStringList& features);
  QStringList features() const;

  void setReference(double reference);
  double reference() const;

  QList<int> enabledFeatures() const;
  
  void check(bool reset);

protected:
  void process();

private:
  class FeatureExtractor;
  class MinimumExtractor;
  class MaximumExtractor;
  class MinMaxExtractor;
  class AverageExtractor;
  class MinAvgExtractor;
  class MaxAvgExtractor;
  class MinMaxAvgExtractor;
  class DefaultExtractor;
  friend class DefaultExtractor;
  
  void setExtractor(FeatureExtractor* extractor);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();

    double dReference;
    QList<int> lstEnabledFeatures;
    int iFeatureMask;
    FeatureExtractor* pExtractor;
  };
  PII_D_FUNC;

  static const char* _pFeatureNames[PII_GRAY_FEATURE_OPERATION_FEATURECNT];
};

#endif //_PIIGRAYFEATUREOPERATION_H

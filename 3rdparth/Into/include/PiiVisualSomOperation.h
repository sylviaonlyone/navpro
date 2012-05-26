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

#ifndef _PIIVISUALSOMOPERATION_H
#define _PIIVISUALSOMOPERATION_H

#include <PiiSomOperation.h>
#include <PiiFrequencyLimiter.h>
#include <PiiQImage.h>

/**
 * PiiVisualSomOperation is a special som classifier. Here is also
 * an image input. So user can control training of the classifier. In
 * the first situation this classifier collects features to the
 * internal buffer. User can train this classifier when there is
 * enough images on the map.
 *
 *
 * @par Inputs
 *
 * @in image - an image (any image type)
 *
 */
class PII_CLASSIFICATION_EXPORT PiiVisualSomOperation : public PiiSomOperation::Template<PiiMatrix<double> >
{
  Q_OBJECT

  /**
   * How many features per second will be emitted to the ui-component.
   * Still all features will be classified and emit forward.
   */
  Q_PROPERTY(double maxFrequency READ maxFrequency WRITE setMaxFrequency);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiVisualSomOperation();
  ~PiiVisualSomOperation();

  void setMaxFrequency(double maxFrequency);
  double maxFrequency();

public slots:
  void removeImage(QImagePtr ptr);
  
signals:
  void removeSubImage(QImagePtr ptr);  
  void removeSubImages(int start);
  void addSubImage(QImagePtr ptr, int classIndex);
  void allSubImagesAdded();

protected:
  void collectSample(double label, double weight);
  double classify();
  bool learnBatch();
  void replaceClassifier();

private:
  typedef PiiSomOperation::Template<PiiMatrix<double> > SuperType;
  void storeImage(const PiiVariant& image, int classification);
  
  /// @internal
  class Data : public PiiSomOperation::Template<PiiMatrix<double> >::Data
  {
  public:
    Data();
    
    PiiInputSocket *pImageInput;

    QList<QImagePtr> lstImages, lstNewImages;
    PiiFrequencyLimiter limiter;
    QMutex storeMutex;
    double dClassification;
    int iFakedClassification;
  };
  PII_D_FUNC;
};


#endif //_PIIVISUALSOMOPERATION_H

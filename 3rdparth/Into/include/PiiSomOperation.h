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


#ifndef _PIISOMOPERATION_H
#define _PIISOMOPERATION_H

#include "PiiSom.h"
#include "PiiVectorQuantizerOperation.h"
#include <QSize>

/**
 * An Ydin-compatible SOM classifier operation. SOM is a
 * non-supervised learning machine. Thus, it does not need a class
 * label input. See the description of input and output sockets in
 * PiiClassifierOperation. As most classifiers, the SOM comes in a
 * couple of different forms specialized for a certain data type. SOM
 * supports float (often somewhat faster due to better data locality)
 * and double (more accurate). Int is not supported because the code
 * vectors need to be stored in floating point accuracy. To
 * instantiate the operation from an object registry, one must specify
 * the data type, e.g. "PiiSomOperation<double>".
 *
 * @outputs
 *
 * @out x - the x coordinate of the closest node on the SOM map
 *
 * @out y - the y coordinate of the closest node on the SOM map
 *
 * @ingroup PiiClassificationPlugin
 */
class PII_CLASSIFICATION_EXPORT PiiSomOperation : public PiiVectorQuantizerOperation
{
  Q_OBJECT

  /**
   * The size of the SOM map. The code book will be deleted if the
   * size changes. To build a new one, you need to set @p training to
   * @p true. The default map size is 10-by-10.
   */
  Q_PROPERTY(QSize size READ size WRITE setSize);
  
  /**
   * The number of nodes in horizontal direction.
   */
  Q_PROPERTY(int width READ width WRITE setWidth STORED false);

  /**
   * The number of nodes in vertical direction.
   */
  Q_PROPERTY(int height READ height WRITE setHeight STORED false);

  /**
   * SOM topology.
   */
  Q_PROPERTY(PiiClassification::SomTopology topology READ topology WRITE setTopology);

  /**
   * Learning rate function.
   */
  Q_PROPERTY(PiiClassification::SomRateFunction rateFunction READ rateFunction WRITE setRateFunction);
  
  /**
   * The (zero-based) index of the current training iteration. This
   * property is useful when one needs to retrain a SOM map. Setting
   * the @p iterationNumber property to zero (together with the @p
   * training property set to @p true) makes the map strongly adapt to
   * incoming data. The effect is that the map is trained again, but
   * the training is not started from a random state. If only
   * fine-tuning is needed, one can set the value to anything between
   * 0 and @p learningLength.
   */
  Q_PROPERTY(int iterationNumber READ iterationNumber WRITE setIterationNumber);

  /**
   * The maximum number of vectors used for training. Once this many
   * vectors have been used in training, the map stops adapting.
   */
  Q_PROPERTY(int learningLength READ learningLength WRITE setLearningLength);

  /**
   * The initial radius of the neighborhood affected in training.
   */
  Q_PROPERTY(double initialRadius READ initialRadius WRITE setInitialRadius);

  /**
   * The initial learning rate. This value determines the "strength"
   * of the training. Larger value means that larger changes to the
   * map are allowed by a single incoming vector.
   */
  Q_PROPERTY(double initialLearningRate READ initialLearningRate WRITE setInitialLearningRate);

  /**
   * The current learning rate (read only).
   */
  Q_PROPERTY(double currentLearningRate READ currentLearningRate);

  /**
   * The current radius (read only).
   */
  Q_PROPERTY(double currentRadius READ currentRadius);

  /**
   * Initialization mode. The default value is @p
   * PiiClassification::SomSampleInit.
   */
  Q_PROPERTY(PiiClassification::SomInitMode initMode READ initMode WRITE setInitMode);

  /**
   * The learning algorithm. The default value is @p
   * PiiClassification::SomSequentialAlgorithm.
   */
  Q_PROPERTY(PiiClassification::SomLearningAlgorithm learningAlgorithm READ learningAlgorithm WRITE setLearningAlgorithm);

public:
  template <class SampleSet> class Template;

  QVariant socketProperty(PiiAbstractSocket* socket, const char* name) const;

protected:
  /// @internal
  class Data : public PiiVectorQuantizerOperation::Data
  {
  public:
    Data();
    PiiOutputSocket* pXOutput, *pYOutput;
    QSize size;
  };
  PII_D_FUNC;

  /// @internal
  PiiSomOperation(Data* d);

  void setSize(QSize size);
  QSize size() const;

  int width() const;
  void setWidth(int width);

  int height() const;
  void setHeight(int height);

  virtual PiiClassification::SomTopology topology() const = 0;
  virtual void setTopology(PiiClassification::SomTopology topology) = 0;
  virtual PiiClassification::SomRateFunction rateFunction() const = 0;
  virtual void setRateFunction(PiiClassification::SomRateFunction rateFunction) = 0;
  virtual int iterationNumber() const = 0;
  virtual void setIterationNumber(int iterationNumber) = 0;
  virtual int learningLength() const = 0;
  virtual void setLearningLength(int learningLength) = 0;
  virtual double initialRadius() const = 0;
  virtual void setInitialRadius(double initialRadius) = 0;
  virtual double initialLearningRate() const = 0;
  virtual void setInitialLearningRate(double initialLearningRate) = 0;
  virtual double currentLearningRate() const = 0;
  virtual double currentRadius() const = 0;
  virtual PiiClassification::SomInitMode initMode() const = 0;
  virtual void setInitMode(PiiClassification::SomInitMode mode) = 0;
  virtual PiiClassification::SomLearningAlgorithm learningAlgorithm() const = 0;
  virtual void setLearningAlgorithm(PiiClassification::SomLearningAlgorithm algorithm) = 0;

private:
  void protectProps();
};

#include "PiiSomOperation-templates.h"

PII_DECLARE_SPECIALIZED_VIRTUAL_METAOBJECT_FUNCTION(PiiSomOperation, PiiMatrix<float>, PII_CLASSIFICATION_EXPORT);
PII_DECLARE_SPECIALIZED_VIRTUAL_METAOBJECT_FUNCTION(PiiSomOperation, PiiMatrix<double>, PII_CLASSIFICATION_EXPORT);

PII_DECLARE_EXPORTED_CLASS_TEMPLATE(class, PiiSomOperation::Template<PiiMatrix<float> >, PII_BUILDING_CLASSIFICATION);
PII_DECLARE_EXPORTED_CLASS_TEMPLATE(class, PiiSomOperation::Template<PiiMatrix<double> >, PII_BUILDING_CLASSIFICATION);

#endif //_PIISOMOPERATION_H

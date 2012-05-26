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

#ifndef _PIIPOINTMATCHINGOPERATION_H
#define _PIIPOINTMATCHINGOPERATION_H

#include <PiiClassifierOperation.h>

#include "PiiFeaturePointMatcher.h"

/**
 * An abstract superclass for feature point matching operations. This
 * operation maintains a database of object models as feature points
 * and quickly retrieves matching models for unknown objects. Each
 * feature point is described by point coordinates and a feature
 * vector (a.k.a "descriptor"). The point coordinates specify the
 * location of the feature point with respect to the model. The
 * operation can handle N-dimensional point coordinates, but when it
 * comes to images, two-dimensional points are used.
 *
 * There can be an unlimited number of models, each containing an
 * unlimited number of feature points. Furthermore, each model may
 * have an associated label. This makes it possible to have multiple
 * models of the same object class.
 *
 * In training, the operation collects all incoming data to a model
 * database. Each incoming model may optionally be associated with a
 * label and a model location. In classification, incoming descriptors
 * are matched against the collected database, and the most likely
 * matching models will be found.
 *
 * @inputs
 *
 * @in features - a N-by-M matrix in which each row stores a feature
 * point descriptor. All numeric matrix types will be accepted, but
 * the operation internally uses @p float for calculations. Inherited
 * from PiiClassifierOperation. Note that %PiiFeaturePointMatcher uses
 * matrices instead of row vectors as the feature type.
 *
 * @in label - an optional input that gives a label to the model
 * described by @p points and @p features. This makes it possible to
 * have many models for a single class. Any numeric type will be
 * accepted. This input is used only in when collecting the model
 * samples. Inherited from PiiClassifierOperation.
 *
 * @in points - the locations of feature points in image coordinates. 
 * A N-by-D matrix in which each row contains D-dimensional point
 * coordinates. All numeric matrix types will be accepted, but the
 * operation internally uses @p float for calculations. Each point
 * must have a corresponding feature vector. Otherwise, a run-time
 * error will be generated.
 *
 * @in location - the location of the model (or any interesting part)
 * with respect to the points. The location is described with 2*D
 * parameters as a hypercube. In the most typical two-dimensional
 * case, the hypercube is a rectangle whose parameters are stored into
 * a 1-by-4 as (x,y,width,height). In three-dimensional case, the
 * matrix must be 1-by-6 (x,y,z,width,height,depth) etc. Any numeric
 * matrix type will be accepted, but @p float is used internally. This
 * input is optional. If it is not connected, the operation will use
 * the minimum bounding hypercube of the feature points as the model
 * location. Note that it is not necessary that the feature points are
 * inside the location. The location can frame any area of the point
 * space with respect to the feature points. This input is used only
 * when collecting model samples.
 *
 * @outputs
 *
 * @note The number of objects emitted through the outputs depends on
 * #matchingMode. In @p MatchOneModel mode, only the best matching
 * model will be emitted. If no model matches, -1 will be emitted as
 * the model index. In @p MatchAllModels and @p MatchDifferentModels
 * modes, 0-N matching models will be emitted for each input.
 *
 * @out classification - the label associated with the matched model
 * (@p double). If no model matches in @p MatchOneModel mode, @p NaN
 * will be emitted. Inherited from PiiClassifierOperation.
 *
 * @out model index - the index of the model that matches the query
 * (@p int). If no model matches in @p MatchOneModel mode, -1 will be
 * emitted. In training, this output will always emit the current size
 * of the database minus one; the first training sample has a model
 * index of 0 etc.
 *
 * @out location - the location of the best matching model in model
 * coordinates, represented as a hypercube. The matrix emitted from
 * the @p transform output can be used to map the corners of the
 * hypercube back to the input space. In training, the received or
 * automatically created location will be passed here. If no model
 * matches in @p MatchOneModel mode, a 1-by-2*D zero matrix will be
 * emitted. PiiMatrix<float>.
 *
 * @out transform - the transform that relates the matched model
 * location to the input space. Usually, the input space is
 * two-dimensional image coordinates, and the transform is represented
 * as a 3-by-3 matrix A that maps homogeneous model coordinates m to
 * image coordinates x so that x = Am. The corresponding point for a
 * point m in the model can be found in the image by applying the
 * transform and reading the pixel at x. If the input space is
 * D-dimensional, the size of the transformation matrix will be D+1 by
 * D+1. In training, and if no model matches in @p MatchOneModel mode,
 * this output will emit a D+1 by D+1 identity matrix. 
 * PiiMatrix<double>.
 *
 * @out query points - the subset of the input points that was matched
 * against a model. An N-by-D PiiMatrix<float>. May be an empty
 * matrix.
 *
 * @out model points - the corresponding model points. This output
 * together with <tt>query points</tt> specifies the pairs of points
 * that were successfully matched. Applying the @p transform matrix to
 * these points should place them pretty close to the query points.
 *
 * @see PiiImageCropper
 *
 * @ingroup PiiMatchingPlugin
 */
class PII_MATCHING_EXPORT PiiPointMatchingOperation : public PiiClassifierOperation
{
  Q_OBJECT

  /**
   * Matching mode. The default is @p MatchAllModels: every query may
   * result in 0-N matching results, which may include multiple
   * matches to the same model. This mode is suitable if objects may
   * overlap and many similar objects may be present. If the operation
   * is used for database retrieval rather than object detection, @p
   * MatchDifferentModels is usually used. In this mode, only one
   * match is allowed for each model. In @p MatchOneModel mode each
   * query will be matched to exactly one model.
   */
  Q_PROPERTY(PiiMatching::ModelMatchingMode matchingMode READ matchingMode WRITE setMatchingMode);

  friend struct PiiSerialization::Accessor;
  template <class Archive> void serialize(Archive& archive, const unsigned int)
  {
    PII_D;
    PII_SERIALIZE_BASE(archive, PiiClassifierOperation);
    if (Archive::InputArchive)
      delete d->pMatcher;
    archive & PII_NVP("matcher", d->pMatcher);
    archive & PII_NVP("locations", d->matLocations);
    archive & PII_NVP("labels", d->lstLabels);
  }
public:
  ~PiiPointMatchingOperation();

  void check(bool reset);

  PiiPointMatchingOperation* clone() const;

  int bufferedSampleCount() const;
  int featureCount() const;

protected:
  typedef PiiFeaturePointMatcher<float, PiiMatrix<float> > Matcher;

  /// @internal
  class Data : public PiiClassifierOperation::Data
  {
  public:
    Data(int pointDimensions);
    ~Data();
    
    PiiInputSocket *pPointsInput, *pLocationInput;
    PiiOutputSocket *pModelIndexOutput, *pLocationOutput, *pTransformOutput,
      *pModelPointsOutput, *pQueryPointsOutput;
    PiiMatrix<double> matIdentity;
    PiiMatrix<float> matEmptyLocation, matEmptyPoint;
    PiiMatrix<float> matLocations, matNewPoints, matNewLocations, matNewFeatures;
    QList<double> lstLabels, lstNewLabels;
    QVector<int> vecNewModelIndices;
    Matcher *pMatcher, *pNewMatcher;
    int iModelCount;
    int iPointDimensions;
    PiiMatching::ModelMatchingMode matchingMode;
    bool bMustSendPoints;
  };
  PII_D_FUNC;

  /// @internal
  PiiPointMatchingOperation(Data* data);

  /**
   * Constructs a new %PiiPointMatchingOperation.
   *
   * @param pointDimensions the number of dimensions in the feature
   * point locations. In images, the number of dimensions is two.
   */
  PiiPointMatchingOperation(int pointDimensions);
  
  void setMatchingMode(PiiMatching::ModelMatchingMode matchingMode);
  PiiMatching::ModelMatchingMode matchingMode() const;

  bool learnBatch();
  void replaceClassifier();
  void resizeBatch(int newSize);
  void resetClassifier();
  double classify();
  void collectSample(double label, double weight);
  
  /**
   * Matches the given @a points to the database stored in @a matcher
   * using @a features as the feature descriptors. Subclasses override
   * this function to implement the actual matching strategy.
   *
   * @return a (possibly empty) list of matching models
   */
  virtual PiiMatching::MatchList match(Matcher& matcher,
                                       const PiiMatrix<float>& points,
                                       const PiiMatrix<float>& features) = 0;

  /**
   * Converts the parameters of the transformation model into a square
   * transformation matrix for homogeneous coordinates. Subclasses
   * override this function to convert the model parameters returned
   * by PiiFeaturePointMatcher to a transformation matrix.
   */
  virtual PiiMatrix<double> toTransformMatrix(const PiiMatrix<double>& transformParams) = 0;

  /**
   * Removes duplicate matches to the same model from @a
   * matchedModels. This function is called if there are more than one
   * matched model. Subclasses may implement any strategy for pruning
   * duplicates. The default implementation does nothing.
   */
  virtual void removeDuplicates(PiiMatching::MatchList& matchedModels);

private:
  void init();
  int checkDescriptor(const PiiMatrix<float>& points,
                      const PiiMatrix<float>& features) const;
  void emitMatch(const PiiMatching::Match& match, const PiiMatrix<float>& queryPoints);
};

#endif //_PIIPOINTMATCHINGOPERATION_H

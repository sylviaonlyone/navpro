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

#ifndef _PIIRIGIDPLANERANSAC_H
#define _PIIRIGIDPLANERANSAC_H

#include "PiiRansac.h"
#include "PiiOptimization.h"

#include <PiiVector.h>

/**
 * A RANSAC estimator that finds an in-plane transform between two
 * point sets representing a rigid object. It is assumed that the two
 * points sets can be aligned with 2D rotation, scaling, and
 * translation so that for each inlying point the following equation
 * holds (at least approximately):
 *
 * @f[
 * \left(\begin{array}{c}
 * x_2 \\
 * y_2
 * \end{array}\right) =
 * s\left(\begin{array}{cc}
 * \cos \theta & -\sin \theta \\
 * \sin \theta & \cos \theta
 * \right) \left(\begin{array}{c}
 * x_1 \\
 * y_1
 * \end{array}\right) +
 * \left(\begin{array}{c}
 * t_x \\
 * t_y
 * \end{array}\right)
 * @f]
 *
 * The mathematical model is parametrized by four unknowns: @f$(s,
 * \theta, t_x, t_y)@f$, which stand for scaling factor, rotation
 * angle, horizontal translation, and vertical translation,
 * respectively.
 *
 * This estimator uses squared geometric distance as the goodness of
 * fit.
 *
 * @ingroup PiiOptimization
 */
template <class T> class PiiRigidPlaneRansac :
  public PiiRansac,
  private PiiOptimization::ResidualFunction<double>
{
public:
  /**
   * Constructs a new RANSAC estimator with no points to match. You
   * need to set the points to match with #setPoints() before calling
   * @ref PiiRansac::findBestModel() "findBestModel()" or use the
   * #findBestModel(const PiiMatrix<T>&, const PiiMatrix<T>&)
   * function.
   */
  PiiRigidPlaneRansac();
  
  /**
   * Constructs a new RANSAC estimator that matches @a points1 to @a
   * points2 with an in-plane rotation-scaling-traslation transform. 
   * Both matrices should be N-by-2 and arranged so that matched
   * points are at the same indices.
   *
   * It is assumed that the matching of @a points1 and @a points2 is
   * putative. A percentage of matches is expected to be wrong.
   *
   * Call the @ref PiiRansac::findBestModel() "findBestModel()"
   * function to find the transformation that maps @a points1 to @a
   * points2.
   */
  PiiRigidPlaneRansac(const PiiMatrix<T>& points1,
                      const PiiMatrix<T>& points2);

  /**
   * Sets the point sets for which the transformation needs to be
   * found.
   */
  void setPoints(const PiiMatrix<T>& points1,
                 const PiiMatrix<T>& points2);

  /// @internal
  inline bool findBestModel() { return PiiRansac::findBestModel(); }
  
  /**
   * Runs the RANSAC algorithm to find a transformation that maps @a
   * points1 to @a points2.
   *
   * @see PiiRansac::findBestModel()
   */
  bool findBestModel(const PiiMatrix<T>& points1,
                     const PiiMatrix<T>& points2);

  /**
   * Returns either the model estimated by RANSAC or a geometrically
   * refined, presumably better estimate, depending on the value of
   * the #autoRefine() flag.
   */
  PiiMatrix<double> bestModel() const;
  
  /**
   * Refines the rough model estimate given by the standard RANSAC
   * algorithm. This function uses a iterative non-linear optimization
   * technique (Levenberg-Marquardt) to find the model that best
   * matches the found inlying points in geometrical sense. The
   * function returns refined model parameters as a 1-by-4 matrix, or
   * an empty matrix if findBestModel() has not found a suitable
   * model.
   */
  PiiMatrix<double> refineModel() const;

  /**
   * Enables or disables automatic geometric refinimenet of the best
   * model. If automatic refinement is enabled, the #bestModel()
   * function returns #refineModel() instead of the probably slightly
   * more inaccurate default estimate. By default, automatic
   * refinement is disabled.
   */
  void setAutoRefine(bool autoRefine);
  /**
   * Returns @p true if automatic geometric refinement of the best
   * model is enabled, @p false otherwise.
   */
  bool autoRefine() const;
  /**
   * Sets the maximum allowed absolute rotation angle in radians. By
   * default, this value is 2*pi, which accepts all possible
   * rotations. Setting the maximum allowed angle to a small value
   * makes it possible to limit the search to only small rotations. 
   * The algorithm then discards too large rotations right away
   * without wasting time in inspecting them further.
   */
  void setMaxRotationAngle(double maxRotationAngle);
  /**
   * Returns the maximum allowed rotation angle.
   */
  double maxRotationAngle() const;
  /**
   * Sets the minimum accepted scaling factor. Works in a similar
   * manner than #setMaxRotationAngle(), but imposes a limit to the
   * scaling factor. The default value is 0.5.
   */
  void setMinScale(double minScale);
  /**
   * Returns the minimum accepted scale factor.
   */
  double minScale() const;
  /**
   * Sets the maximum accepted scaling factor. Works analogously to
   * #setMinScale(). The default value is 2.
   */
  void setMaxScale(double maxScale);
  /**
   * Returns the maximum accepted scale factor.
   */
  double maxScale() const;

  /**
   * Transforms @a points to a new coordinate system using the given
   * @a model parameters.
   */
  static PiiMatrix<double> transform(const PiiMatrix<T>& points, const double* model);
  /**
   * @overload
   *
   * This function uses the first row of @a model as the model
   * parameter vector.
   */
  static PiiMatrix<double> transform(const PiiMatrix<T>& points, const PiiMatrix<double>& model)
  {
    return transform(points, model[0]);
  }

  /**
   * Converts the given @a model parameters to a 3-by-3 transformation
   * matrix for homogeneous coordinates.
   */
  static PiiMatrix<double> toTransformMatrix(const double* model);
  /// @overload
  static PiiMatrix<double> toTransformMatrix(const PiiMatrix<double>& model)
  {
    return toTransformMatrix(model[0]);
  }

protected:
  /**
   * Returns the minimum number of rows in the given two point sets.
   */
  int totalSampleCount() const;
  
  /**
   * Returns two. The transformation equation has four unknowns, which
   * can be uniquely solved with two 2D points.
   */
  int minSamples() const;

  /**
   * Always returns a 1-by-4 matrix representing the parameters of a
   * single matching model. If the two points are the same, an empty
   * matrix will be returned.
   */
  PiiMatrix<double> findPossibleModels(const int* dataIndices);

  /**
   * Transforms the point at @a dataIndices in the fist point set
   * using the given @a model parameters. Returns the squared
   * geometric distance between the transformed point and the
   * corresponding point in the second point set.
   */
  double fitToModel(int dataIndex, const double* model);

private:
  class Data : public PiiRansac::Data
  {
  public:
    Data() :
      bAutoRefine(false),
      dMaxRotationAngle(2*M_PI),
      dMinScale(0.5),
      dMaxScale(2),
      iInlierCount(0),
      piInliers(0)
    {}
    
    Data(const PiiMatrix<T>& points1,
         const PiiMatrix<T>& points2) :
      matPoints1(points1), matPoints2(points2),
      bAutoRefine(false),
      dMaxRotationAngle(2*M_PI),
      dMinScale(0.5),
      dMaxScale(2),
      iInlierCount(0),
      piInliers(0)
    {}
    
    PiiMatrix<T> matPoints1, matPoints2;
    bool bAutoRefine;
    double dMaxRotationAngle;
    double dMinScale;
    double dMaxScale;

    // Temporaries for LM minimization
    mutable int iInlierCount;
    mutable const int* piInliers;
  };
  PII_D_FUNC;

  static PiiVector<double,2> transform(const T* point, const double* model);

  // Interface for LM minimization
  int functionCount() const;
  void residualValues(const double* params, double* residuals) const;
};

namespace PiiOptimization
{
  /**
   * Converts rigid plane (scale, rotation, translation) model
   * parameters to a 3-by-3 transformation matrix. The returned
   * transformation matrix can be used as a premultiplier to transform
   * homogeneous model coordinates to the current image coordinates.
   *
   * @param model a 1-by-4 matrix (scale, rotation, tx, ty)
   *
   * @return a 3-by-3 transformation matrix for homogeneous
   * coordinates
   *
   * @relates PiiRigidPlaneRansac
   */
  PII_OPTIMIZATION_EXPORT PiiMatrix<float> rigidPlaneModelToTransform(const PiiMatrix<double>& model);
}

template <class T> PiiRigidPlaneRansac<T>::PiiRigidPlaneRansac() :
  PiiRansac(new Data)
{}

template <class T> PiiRigidPlaneRansac<T>::PiiRigidPlaneRansac(const PiiMatrix<T>& points1,
                                                               const PiiMatrix<T>& points2) :
  PiiRansac(new Data(points1, points2))
{}

template <class T> void PiiRigidPlaneRansac<T>::setPoints(const PiiMatrix<T>& points1,
                                                          const PiiMatrix<T>& points2)
{
  PII_D;
  d->matPoints1 = points1;
  d->matPoints2 = points2;
}

template <class T> bool PiiRigidPlaneRansac<T>::findBestModel(const PiiMatrix<T>& points1,
                                                              const PiiMatrix<T>& points2)
{
  setPoints(points1, points2);
  return PiiRansac::findBestModel();
}

template <class T> PiiMatrix<double> PiiRigidPlaneRansac<T>::bestModel() const
{
  return _d()->bAutoRefine ? refineModel() : PiiRansac::bestModel();
}

template <class T> PiiMatrix<double> PiiRigidPlaneRansac<T>::refineModel() const
{
  const PII_D;
  PiiMatrix<double> matBestModel(PiiRansac::bestModel());
  if (matBestModel.isEmpty())
    return PiiMatrix<double>();
  d->piInliers = inlyingPoints().constData();
  d->iInlierCount = inlierCount();
  return PiiOptimization::lmMinimize(this, matBestModel);
}

template <class T> bool PiiRigidPlaneRansac<T>::autoRefine() const { return _d()->bAutoRefine; }
template <class T> void PiiRigidPlaneRansac<T>::setAutoRefine(bool autoRefine) { _d()->bAutoRefine = autoRefine; }

template <class T> int PiiRigidPlaneRansac<T>::totalSampleCount() const
{
  const PII_D;
  return qMin(d->matPoints1.rows(), d->matPoints2.rows());
}

template <class T> int PiiRigidPlaneRansac<T>::minSamples() const
{
  return 2;
}

template <class T> PiiMatrix<double> PiiRigidPlaneRansac<T>::findPossibleModels(const int* dataIndices)
{
  PII_D;
  // The selected pair of points in the first point set, and the
  // vector between them.
  PiiVector<T,2>
    pt11 = d->matPoints1.template rowAs<PiiVector<T,2> >(dataIndices[0]),
    pt12 = d->matPoints1.template rowAs<PiiVector<T,2> >(dataIndices[1]),
    vec1 = pt12 - pt11;
  double dLength1 = vec1.length();
  // Degenerate case
  if (dLength1 == 0)
    return PiiMatrix<double>();

  // Same for the second point set
  PiiVector<T,2>
    pt21 = d->matPoints2.template rowAs<PiiVector<T,2> >(dataIndices[0]),
    pt22 = d->matPoints2.template rowAs<PiiVector<T,2> >(dataIndices[1]),
    vec2 = pt22 - pt21;
  double dLength2 = vec2.length();
  if (dLength2 == 0)
    return PiiMatrix<double>();

  // Length ratio is the scaling factor.
  double dScale = dLength2 / dLength1;
  // Is the scaling factor within limits?
  if (dScale < d->dMinScale || dScale > d->dMaxScale)
    return PiiMatrix<double>();

  // Rotation angle between the two vectors.
  double dTheta = ::atan2(double(vec2[1]), double(vec2[0])) - ::atan2(double(vec1[1]), double(vec1[0]));
  if (dTheta < 0)
    dTheta += 2*M_PI;
  // Is this angle within the allowed limits?
  if (dTheta > d->dMaxRotationAngle)
    return PiiMatrix<double>();

  // Return scale and rotation (traslation not known yet)
  PiiMatrix<double> matModel(1,4, dScale, dTheta, 0.0, 0.0);
  
  // Now that we know rotation and scale, we need to transform one of
  // the points in point set 1 to the new coordinate system to find
  // the translation.
  PiiVector<double,2> ptNew = transform(pt11.begin(), matModel[0]);

  matModel(0,2) = double(pt21[0]) - ptNew[0];
  matModel(0,3) = double(pt21[1]) - ptNew[1];

  return matModel;
}

template <class T> PiiVector<double,2> PiiRigidPlaneRansac<T>::transform(const T* point, const double* model)
{
  double dCos = model[0] * Pii::cos(model[1]), dSin = model[0] * Pii::sin(model[1]);
  return PiiVector<double,2>(dCos * point[0] - dSin * point[1] + model[2],
                             dSin * point[0] + dCos * point[1] + model[3]);
}

template <class T> PiiMatrix<double> PiiRigidPlaneRansac<T>::toTransformMatrix(const double* model)
{
  double dCos = model[0] * Pii::cos(model[1]), dSin = model[0] * Pii::sin(model[1]);
  return PiiMatrix<double>(3, 3,
                           dCos, -dSin, model[2],
                           dSin,  dCos, model[3],
                           0.0 ,  0.0 , 1.0);
}


template <class T> double PiiRigidPlaneRansac<T>::fitToModel(int dataIndex, const double* model)
{
  PII_D;
  PiiVector<double,2> ptTransformed = transform(d->matPoints1[dataIndex], model);
  return Pii::squaredDistanceN(ptTransformed.begin(), 2,
                               d->matPoints2[dataIndex],
                               0.0);
}

template <class T> PiiMatrix<double> PiiRigidPlaneRansac<T>::transform(const PiiMatrix<T>& points, const double* model)
{
  PiiMatrix<double> matResult(points.rows(), 2);
  for (int r=0; r<points.rows(); ++r)
    matResult.rowAs<PiiVector<double,2> >(r) = transform(points[r], model);
  return matResult;
}

template <class T> int PiiRigidPlaneRansac<T>::functionCount() const
{
  return _d()->iInlierCount;
}


template <class T> void PiiRigidPlaneRansac<T>::residualValues(const double* params, double* residuals) const
{
  const PII_D;
  for (int i=0; i<d->iInlierCount; ++i)
    {
      int iPoint = d->piInliers[i];
      // "Ground truth" is in point set 2
      PiiVector<double,2> vecPt2(double(d->matPoints2(iPoint,0)), double(d->matPoints2(iPoint,1)));
      // Calculate squared distance to the transformation result.
      residuals[i] = vecPt2.squaredDistance(transform(d->matPoints1[iPoint], params));
    }
}

template <class T> void PiiRigidPlaneRansac<T>::setMaxRotationAngle(double maxRotationAngle) { _d()->dMaxRotationAngle = maxRotationAngle; }
template <class T> double PiiRigidPlaneRansac<T>::maxRotationAngle() const { return _d()->dMaxRotationAngle; }
template <class T> void PiiRigidPlaneRansac<T>::setMinScale(double minScale) { _d()->dMinScale = minScale; }
template <class T> double PiiRigidPlaneRansac<T>::minScale() const { return _d()->dMinScale; }
template <class T> void PiiRigidPlaneRansac<T>::setMaxScale(double maxScale) { _d()->dMaxScale = maxScale; }
template <class T> double PiiRigidPlaneRansac<T>::maxScale() const { return _d()->dMaxScale; }


#endif //_PIIRIGIDPLANERANSAC_H

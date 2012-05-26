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

#ifndef _PIIEXTENDEDCOORDINATETRACKER_H
#define _PIIEXTENDEDCOORDINATETRACKER_H

#include "PiiCoordinateTracker.h"
#include "PiiTracking.h"

/**
 * A tracker that follows trajectories in @p D dimensions. It extends
 * PiiCoordinateTracker by adding more sophisticated trajectory
 * management facilities. %PiiExtendedCoordinateTracker has the
 * following properties:
 *
 * @li Predicts constant movement for all objects based on two latest
 * measurements.
 *
 * @li New trajectories will be created for measurements that are
 * either not connected to a trajectory or connected with a bad
 * fitness value (see #setGoodFitnessThreshold()). The fitness of such
 * a new trajectory will be 1.0 - the maximum fitness compared to any
 * candidate trajectory.
 *
 * @li If a trajectory cannot be reliably continued (see
 * #setGoodFitnessThreshold()), the trajectory will be retained in
 * candidate trajectories without extending it. This enhancement is
 * needed to cope with missing measurements. Such trajectories are
 * extended by cloning the last trajectory node.
 *
 * @li If a trajectory has not been extended with new measurements for
 * N (#setMaxStopTime()) iterations, it will be killed
 * (#endTrajectories()).
 *
 * @ingroup PiiTrackingPlugin
 */
template <class T, int D> class PiiExtendedCoordinateTracker : public PiiCoordinateTracker<T,D>
{
public:
  typedef PiiVector<T,D> MeasurementType;
  typedef PiiCoordinateTrackerNode<T,D> TrajectoryType;
  typedef PiiCoordinateTracker<T,D> ParentType;

  PiiExtendedCoordinateTracker() :
    _dGoodFitnessThreshold(0.0),
    _pMaximumMeasurementFitness(0),
    _pMaximumTrajectoryFitness(0),
    _iMaximumStopTime(1),
    _iMaximumPredictionLength(1)
  {}
  
  /**
   * Set the threshold for good fitness. "Good fitness" is a level of
   * fitness that indicates a measurement is pretty close to the
   * predicted position. Fitness values are within [0,1].
   *
   * The "good fitness" threshold is used in deciding whether a
   * measurement can form a new candidate trajectory or not. If the
   * maximum fitness of a sample compared to any of the current set of
   * trajectories is less than or equal to this threshold, a new
   * candidate will be formed. The fitness of the new trajectory will
   * be 1.0 - the maximum fitness.
   *
   * If a trajectory cannot be extended with a measurement whose
   * fitness greater than this value, a new candidate will be created
   * by cloning the current end point of the trajectory. The default
   * value, 0.0, causes the tracker to create a new trajectory only if
   * the measurement cannot be connected to any of the current
   * candidate trajectories.
   */
  void setGoodFitnessThreshold(double goodFitnessThreshold) { _dGoodFitnessThreshold = goodFitnessThreshold; }
  /**
   * Get the current good fitness threshold.
   */
  double goodFitnessThreshold() const { return _dGoodFitnessThreshold; }
  /**
   * The maximum time a trajectory will stay alive without any
   * measurements connected to it. If this much time passes and a
   * trajectory has not been extended, it needs to be ended. The
   * tracker collects all such trajectories into a list it passes to
   * #endTrajectories().
   *
   * @param maximumStopTime the maximum amount of time a trajectory
   * stays alive without measurements connected to it. The time is
   * expressed in the units given as the @p t parameter to @p
   * addMeasurements(). The default value is 1.
   */
  void setMaximumStopTime(int maximumStopTime) { _iMaximumStopTime = maximumStopTime; }
  /**
   * Get the maximum stop time.
   */
  int maximumStopTime() const { return _iMaximumStopTime; }

  /**
   * The maximum number of trajectory nodes between samples used for
   * movement prediction. The default value is 1, which means that the
   * difference between two previous samples will always be used.
   * Setting the maximum prediction length to a larger value makes the
   * estimate skip more samples in between, which should make the
   * estimate more robust.
   *
   * @param maximumPredictionLength the maximum distance between nodes
   * used for velocity estimation.
   */
  void setMaximumPredictionLength(int maximumPredictionLength) { _iMaximumPredictionLength = maximumPredictionLength; }
  /**
   * Get the current maximum prediction length.
   */
  int maximumPredictionLength() const { return _iMaximumPredictionLength; }
  
  void addMeasurements(const QList<MeasurementType>& measurements, int t);

protected:
  MeasurementType* predict(TrajectoryType* trajectory, int t);
  double measureFit(TrajectoryType** trajectory, const MeasurementType& measurement, int t) const;
  double measureFit(const MeasurementType& measurement, int t) const;
  
  /**
   * Called when the tracker decides to get rid of a set of
   * trajectories. Subclasses may perform any functionality needed to
   * close the trajectory in their overridden implementation. The
   * default implementation deletes all trajectories. If you don't
   * delete the trajectories yourself, make sure you remember to call
   * this function in your overridden function.
   *
   * @param trajectories the trajectories that are just about to pass
   * away. The ended trajectories will appear in this list in the same
   * order they appeared in trajectories.
   *
   * @param t the current time instant
   */
  virtual void endTrajectories(QList<TrajectoryType*> trajectories, int t)
  {
    Q_UNUSED(t);
    qDeleteAll(trajectories);
  }
  
private:
  double _dGoodFitnessThreshold;
  double* _pMaximumMeasurementFitness, *_pMaximumTrajectoryFitness;
  int _iMaximumStopTime;
  int _iMaximumPredictionLength;
};

template <class T, int D>
void PiiExtendedCoordinateTracker<T,D>::addMeasurements(const QList<MeasurementType>& measurements, int t)
{
  // Initialize an array for storing the maximum score of each
  // measurement
  _pMaximumMeasurementFitness = new double[measurements.size()];
  for (int i=measurements.size(); i--; )
    _pMaximumMeasurementFitness[i] = 0;

  // Same for trajectories
  _pMaximumTrajectoryFitness = new double[this->count()];
  for (int i=this->count(); i--; )
    _pMaximumTrajectoryFitness[i] = 0;

  // Store old trajectories
  QList<TrajectoryType*> oldTrajectories = *this;
  
  // Run the tracking algorithm
  ParentType::addMeasurements(measurements, t);

  // Inspect trajectories that don't have a sufficiently good match
  // among the new measurements. The old end point of such trajectory
  // must be retained to the next iteration.
  for (int i=oldTrajectories.size(); i--; )
    {
      //qDebug("Inspecting trajectory %d. Best score: %lf", i, _pMaximumTrajectoryFitness[i]);
      if (_pMaximumTrajectoryFitness[i] <= _dGoodFitnessThreshold)
        {
          // The trajectory was branched -> we need to extend the
          // trajectory by a clone of its last node.
          if (oldTrajectories[i]->branches() > 0)
            append(new TrajectoryType(*oldTrajectories[i]));
          // The trajectory was not extended -> just move it to the
          // current trajectory set.
          else
            append(oldTrajectories[i]);
        }
    }
  
  // If the end point of a trajectory is old enough, we discard of the
  // trajectory.
  QList<TrajectoryType*> deletedTrajectories;
  for (int i=this->count(); i--;)
    if (t - this->at(i)->time() > _iMaximumStopTime)
      deletedTrajectories << this->takeAt(i);

  endTrajectories(deletedTrajectories, t);

  // Destroy the maximum scores
  delete[] _pMaximumMeasurementFitness;
  _pMaximumMeasurementFitness = 0;
  delete[] _pMaximumTrajectoryFitness;
  _pMaximumTrajectoryFitness = 0;
}

template <class T, int D> typename PiiExtendedCoordinateTracker<T,D>::MeasurementType*
PiiExtendedCoordinateTracker<T,D>::predict(TrajectoryType* trajectory, int t)
{
  // Assume constant velocity
  return ::PiiTracking::predictConstantVelocity(trajectory, t, _iMaximumPredictionLength);
}

template <class T, int D>
double PiiExtendedCoordinateTracker<T,D>::measureFit(TrajectoryType** trajectory,
                                                     const MeasurementType& measurement,
                                                     int t) const
{
  double score = ParentType::measureFit(trajectory, measurement, t);
  if (trajectory)
    {
      // Store the maximum fitness for the current measurement
      _pMaximumMeasurementFitness[this->currentMeasurementIndex()] = qMax(score, _pMaximumMeasurementFitness[this->currentMeasurementIndex()]);
      
      // Store the maximum fitness for the current trajectory
      _pMaximumTrajectoryFitness[this->currentTrajectoryIndex()] = qMax(score, _pMaximumTrajectoryFitness[this->currentTrajectoryIndex()]);
    }
  return score;
}

template <class T, int D>
double PiiExtendedCoordinateTracker<T,D>::measureFit(const MeasurementType& /*measurement*/, int /*t*/) const
{
  // If the measurement was not reliable connected to a trajectory, it
  // can form a new trajectory.
  if (_pMaximumMeasurementFitness[this->currentMeasurementIndex()] <= _dGoodFitnessThreshold)
    return 1.0 - _pMaximumMeasurementFitness[this->currentMeasurementIndex()];

  // If the measurement is close to the boundary, it may be a new one
  /*if (measurement.values[0] < _area.left() + _dBoundaryThreshold ||
      measurement.values[0] > _area.right() - _dBoundaryThreshold ||
      measurement.values[1] < _area.top() + _dBoundaryThreshold ||
      measurement.values[1] > _area.bottom() - _dBoundaryThreshold)
      return 1;*/

  // It is either already connected to a trajectory or too far from
  // area boundaries -> not a new candidate
  return 0;
}


#endif //_PIIEXTENDEDCOORDINATETRACKER_H

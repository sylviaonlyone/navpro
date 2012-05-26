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

#ifndef _PIIMULTIHYPOTHESISTRACKER_H
#define _PIIMULTIHYPOTHESISTRACKER_H

#include <QList>

/**
 * The tracking algorithm uses a greedy breadth-first search algorithm
 * to find the most likely measurement sequence, given a measurement
 * model. The algorithm works as follows:
 *
 * @li Initialize an empty set of trajectories.
 *
 * @li At each time step read in a set of N candidate measurements.
 *
 * @li Evaluate how well each of the N measurements fits into the
 * current set of M candidate trajectories (N x M evaluations). 
 * (#measureFit())
 *
 * @li Generate a new set of candidate trajectories by extending the
 * old ones with the measurements with non-zero probabilities. This
 * may create many branches for a single trajectory. 
 * (#createTrajectory()) Note that trajectories will be cleared
 * and the new trajectories will be added to the empty list.
 *
 * @li Evaluate how likely it is that a measurement is a starting
 * point for a new trajectory (N evaluations). (#measureFit())
 *
 * @li For each measurement with a non-zero likelihood of being part
 * of a new trajectory, create a new trajectory and add it to the set
 * of candidate trajectories. (#createTrajectory())
 *
 * The tracker is a template class that works with any type of
 * measurements (e.g. 2-D or 3-D points) and trajectories (e.g. lists
 * of points). Measurements and trajectories can even be implemented
 * as indices to external storage.
 *
 * @ingroup PiiTrackingPlugin
 */
template <class Measurement, class Trajectory> class PiiMultiHypothesisTracker : public QList<Trajectory>
{
public:
  typedef Measurement MeasurementType;
  typedef Trajectory TrajectoryType;
  
  /**
   * Add a new set of candidate measurements to the tracker. This will
   * run one cycle of the algorithm.
   *
   * @param measurements a list of candidate measurements
   *
   * @param t the current time instant
   */
  virtual void addMeasurements(const QList<MeasurementType>& measurements, int t);

protected:
  virtual ~PiiMultiHypothesisTracker() {}
  
  /**
   * Create a new trajectory by extending an old one.
   *
   * @param trajectory the old trajectory to be extended/branched. If
   * this parameter is 0, a new trajectory should be created.
   *
   * @param measurement the measurement to add to the trajectory.
   *
   * @param fitness fit of the measurement to the trajectory. Measured
   * by the #measureFit() function.
   *
   * @param t the current time instant
   *
   * @return a new trajectory
   */
  virtual TrajectoryType createTrajectory(TrajectoryType* trajectory, const MeasurementType& measurement, double fitness, int t) = 0;

  /**
   * Measure how well @p measurement fits into @p trajectory.
   *
   * @param trajectory the trajectory @p measurement is evaluated
   * against. If this parameter is 0, the function should tell how
   * likely the measurement is to create a new trajectory. This
   * function implements the measurement model.
   *
   * @param measurement the measurement to evaluate.
   *
   * @param t the current time instant.
   *
   * @return an evaluation. Typically a value between zero (certainly
   * not) and one (absolutely yes).
   */
  virtual double measureFit(TrajectoryType* trajectory, const MeasurementType& measurement, int t) const = 0;

  /**
   * Get the current index of the trajectory.
   */
  int currentTrajectoryIndex() const
  {
    return _iTrajectoryIndex;
  }

  /**
   * Get the current index of the measurement.
   */
  int currentMeasurementIndex() const
  {
    return _iMeasurementIndex;
  }
private:
  /**
   * The index of the measurement currently being inspected by the
   * tracking algorithm. This index can be used by subclasses to store
   * information specific to a certain sample. The index refers to the
   * @p measurements list given as a parameter to @p addMeasurements.
   */
  int _iMeasurementIndex;
  /**
   * The index of the trajectory currently being inspected by the
   * tracking algorithm. Works analogously to @p _iMeasurementIndex.
   * The index refers to trajectories.
   */
  int _iTrajectoryIndex;
};


template <class Measurement, class Trajectory>
void PiiMultiHypothesisTracker<Measurement,Trajectory>::addMeasurements(const QList<MeasurementType>& measurements, int t)
{
  QList<TrajectoryType> oldTrajectories = *this;
  this->clear();

  for (_iTrajectoryIndex=oldTrajectories.size(); _iTrajectoryIndex--; )
    {
      for (_iMeasurementIndex=measurements.size(); _iMeasurementIndex--; )
        {
          // See how well this measurement would fit into the current
          // trajectory.
          double score = measureFit(&oldTrajectories[_iTrajectoryIndex], measurements[_iMeasurementIndex], t);
          // If it fits even in principle, create a new trajectory
          if (score > 0)
            append(createTrajectory(&oldTrajectories[_iTrajectoryIndex], measurements[_iMeasurementIndex], score, t));
        }
    }

  for (_iMeasurementIndex=measurements.size(); _iMeasurementIndex--; )
    {
      // Is this measurement likely to create a new trajectory?
      double score = measureFit(0, measurements[_iMeasurementIndex], t);
      // If this measurement can work as a starting point with a
      // non-zero probability, create a new trajectory.
      if (score > 0)
        append(createTrajectory(0, measurements[_iMeasurementIndex], score, t));
    }
}

#endif //_PIIMULTIHYPOTHESISTRACKER_H

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

#ifndef _PIITRACKERTRAJECTORYNODE_H
#define _PIITRACKERTRAJECTORYNODE_H

/**
 * A utility class that can be used as the @p Trajectory type with
 * PiiMultiHypothesisTracker. With this structure, trajectories are
 * built as (singly) linked lists that support branching. This class
 * is intended to be derived with the CRTP (curiously recurring
 * template pattern):
 *
 * @code
 * class MyNode : public PiiTrackerTrajectoryNode<int,MyNode>
 * {
 *   // ... your stuff here ...
 * };
 * @endcode
 *
 * @ingroup PiiTrackingPlugin
 */
template <class Measurement, class Node> class PiiTrackerTrajectoryNode
{
public:
  typedef Measurement MeasurementType;
  typedef Node NodeType;

  /**
   * Get the number of branches originating from (that is, references
   * to) this node. If this is the head of a list, the value will be
   * 0. If there is just one link, the value will be one and so on. 
   * A node can only be deleted when its branch count reaches zero.
   */
  int branches() const { return _iRefCount; }
  /**
   * Get the next node in sequence.
   */
  Node* next() const { return _pNext; }
  /**
   * Get the measurement time.
   */
  int time() const { return _iTime; }
  /**
   * Get the actual measurement.
   */
  const Measurement& measurement() const { return _measurement; }

  /**
   * Get the length of the linked list from this node to the very end.
   */
  int length() const { return _iLength; }

  /**
   * Get the length of the linked list from this point to the first
   * branch. If the list is not branched, the return value will be
   * equal to #length().
   */
  int lengthToBranch() const;

  /**
   * Compare two nodes. Nodes are equal if their time and measurement
   * are equal.
   */
  bool operator== (const PiiTrackerTrajectoryNode& other)
  {
    return _iTime == other._iTime && _measurement == other._measurement;
  }
  
protected:
  /**
   * Create an empty trajectory node.
   */
  PiiTrackerTrajectoryNode() :
    _iTime(0), _pNext(0), _iRefCount(0), _iLength(1)
  { }

  /**
   * Copy constructor.
   */
  PiiTrackerTrajectoryNode(const PiiTrackerTrajectoryNode& other) :
    _measurement(other._measurement), _iTime(other._iTime), _pNext(other._pNext), _iRefCount(0), _iLength(other._iLength)
  {
    if (_pNext) ++_pNext->_iRefCount;
  }
  
  /**
   * Create a new node of a trajectory.
   *
   * @param measurement the measurement through which the trajectory
   * goes.
   *
   * @param t the current time instant
   *
   * @param next the next node in chain
   *
   * The constructor will increase the reference count on @p next.
   */
  PiiTrackerTrajectoryNode(const MeasurementType& measurement, int t, Node* next) :
    _measurement(measurement), _iTime(t), _pNext(next), _iRefCount(0),
    // If next node exists, length is previous plus one
    _iLength(next ? next->_iLength + 1 : 1) 
  {
    if (_pNext) ++_pNext->_iRefCount;
  }

  /**
   * Destroy a node. This destroys the whole linked list up to the
   * first branch or to the very end, whichever comes first. This is
   * done by decrementing the reference count of @p next and by
   * deleting it when the count hits zero.
   */
  virtual ~PiiTrackerTrajectoryNode()
  {
    if (_pNext && --_pNext->_iRefCount == 0)
      delete _pNext;
  }

protected:
  /**
   * The actual measurement.
   */
  MeasurementType _measurement;

  /**
   * The current time instant.
   */
  int _iTime;
  
  /**
   * A pointer to the next node in chain.
   */
  NodeType* _pNext;

private:
  /**
   * The number of references to this node.
   */
  int _iRefCount;

  /**
   * The length of the linked list up to the first node.
   */
  int _iLength;
};

template <class Measurement, class Node> int PiiTrackerTrajectoryNode<Measurement,Node>::lengthToBranch() const
{
  // Iterative solution may be less elegant than a recursive one, but
  // it is a lot faster too.
  int cnt = 1;
  for (NodeType* next = _pNext; next && next->_iRefCount == 1; next = next->next(), cnt++) ;
  return cnt;
}

#endif //_PIITRACKERTRAJECTORYNODE_H

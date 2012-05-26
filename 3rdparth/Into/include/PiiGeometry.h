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

#ifndef _PIIGEOMETRY_H
#define _PIIGEOMETRY_H

#include <PiiMath.h>
#include <PiiGeometricObjects.h>


/**
 * Definitions for geometrical calculations and different algorithms
 * for handling geometric objects.
 *
 * @ingroup PiiGeometryPlugin
 */
namespace PiiGeometry
{
  /**
   * Douglas-Peucker recursive polyline simplification algorithm.
   *
   * @param vertices the input vertices forming a polyline. Each row
   * in the matrix represents a point in N-dimensional coordinates.
   *
   * @param tolerance the maximum allowed deviation between an
   * original data point and a reduced edge. This distance must be
   * squared. If you allow a deviation of 5 (pixels or anything), give
   * 25 as the tolerance.
   *
   * @param start the index of the first vertex in the optimization
   * range.
   *
   * @param end the index of the last vertex in the optimization
   * range.
   *
   * @param result a matrix to which the retained vertices will be
   * stored. The start and end point will not be stored.
   */
  template <class T> void reduceVertices(const PiiMatrix<T>& vertices,
                                         double tolerance,
                                         int start, int end,
                                         PiiMatrix<T>& result)
  {
    // Break recursion
    if (start >= end-1)
      return;

    const int iColumns = vertices.columns();
    PiiMatrix<T> startPoint(vertices(start,0,1,-1));
    PiiMatrix<T> endPoint(vertices(end,0,1,-1));
    // A vector spanning the current segment (start -> end)
    PiiMatrix<double> direction(endPoint - startPoint);
    // Squared length of the segment
    double segmentLength = Pii::innerProductN(direction.row(0), iColumns, direction.row(0), 0.0);

    PiiMatrix<T> currentVector, currentPoint;
    double distance = 0, maxDistance = 0;
    // The index of the point that deviates most from straight line.
    int maxIndex = start;

    // Find the point that deviates most from the current segment.
    for (int i=start+1; i<end; i++)
      {
        currentPoint = vertices(i,0,1,-1);
        // Vector from start to current point
        currentVector = currentPoint - startPoint;
        // Projection to direction vector
        double projectionLength = Pii::innerProductN(currentVector.row(0), iColumns,
                                                     direction.row(0), 0.0);
        // Negative or zero -> shortest distance is to start point
        if (projectionLength <= 0)
          distance = Pii::squaredDistanceN(currentPoint.row(0), iColumns, startPoint.row(0), 0.0);
        // Farther than the end point
        else if (segmentLength <= projectionLength)
          distance = Pii::squaredDistanceN(currentPoint.row(0), iColumns, endPoint.row(0), 0.0);
        // In between
        else
          distance = Pii::squaredDistanceN(currentPoint.row(0), iColumns,
                                           (startPoint + direction * (projectionLength / segmentLength)).rowBegin(0),
                                           0.0);

        // Exceeded the maximum distance...
        if (distance > maxDistance)
          {
            maxIndex = i;
            maxDistance = distance;
          }
      }
    if (maxDistance > tolerance) // Too much deviation -> must split
      {
        // Take the left side
        reduceVertices(vertices, tolerance, start, maxIndex, result);
        // Add the middle point to the result
        result.insertRow(-1,vertices.row(maxIndex));
        // Take the right side
        reduceVertices(vertices, tolerance, maxIndex, end, result);
      }
  }


  /**
   * Simplify a polyline. This function uses a two-step algorithm that
   * first prunes all vertices that are within @p tolerance from each
   * other and then uses the Douglas-Peucker recursive algorithm to
   * further optimize the polyline.
   *
   * @param vertices the input vertices forming a polyline. Each row
   * in the matrix represents a point in N-dimensional coordinates.
   *
   * @param tolerance the maximum allowed deviation between an
   * original data point and a reduced edge. This distance must be
   * squared. If you allow a deviation of 5 (pixels or anything else),
   * give 25 as the tolerance.
   *
   * @return a matrix containing the retained vertices. The first and
   * last point in @p vertices will always be the last and first
   * points in the result matrix.
   */
  template <class T> PiiMatrix<T> reduceVertices(const PiiMatrix<T>& vertices, double tolerance)
  {
    int count = vertices.rows();
    PiiMatrix<T> pruned(0, vertices.columns());
    pruned.reserve(32);

    // Prune vertices that are already within tolerance from each
    // other. Store the start point first.
    pruned.appendRow(vertices[0]);
    if (vertices.rows() == 1)
      return pruned;
    int previous = 0;
    for (int i=1; i<count; ++i)
      {
        // Find the first vertex that is not within the tolerance
        if (Pii::squaredDistanceN(vertices[i], vertices.columns(), vertices[previous], 0.0) < tolerance)
          continue;
        // Add to pruned buffer
        pruned.appendRow(vertices[i]);
        previous = i;
      }
    // The last point must always be stored.
    if (previous < vertices.rows()-1)
      pruned.appendRow(vertices[vertices.rows()-1]);

    PiiMatrix<T> result(0, vertices.columns());
    result.reserve(32);

    result.appendRow(pruned[0]);
    reduceVertices(pruned, tolerance, 0, pruned.rows()-1, result);
    result.appendRow(pruned[pruned.rows()-1]);

    return result;
  }

  /**
   * Calculate the shortest distance between two 3D lines.
   *
   * @code
   * // Two 3D lines.
   * PiiMatrix<double> first(2, 3,
   *                         0.0, 0.0, 0.0,
   *                         0.0, 2.0, 0.0);
   * PiiMatrix<double> second(1, 6,
   *                          4.0, 0.0, 0.0,
   *                          4.0, 0.0, 4.0);
   *
   * double distance = PiiGeometry::lineToLineDistance(first, second);
   *
   * // distance is now 4.0
   * @endcode 
   *
   * @param first a 2-by-3 matrix whose first row is the direction of
   * the line and the second row any point on the line.
   *
   * @param second a 2-by-3 matrix whose first row is the direction of
   * the line and the second row any point on the line.
   * 
   * @return the shortest distance between the two lines
   */
  template <class T> T lineToLineDistance(const PiiMatrix<T>& first, const PiiMatrix<T>& second)
  {
    PiiMatrix<T> w(1,first.columns());
    
    T a = 0.0, b = 0.0, c = 0.0, d = 0.0, e = 0.0;
    
    for (int i = 0; i < first.columns(); ++i)
      {
        a += first(0,i)*first(0,i);
        b += first(0,i)*second(0,i);
        c += second(0,i)*second(0,i);
        w(0,i) = first(1,i) - second(1,i);
        d += first(0,i)*w(0,i);
        e += second(0,i)*w(0,i);
      }

    T discriminant = a*c - b*b;
    T sc = 0, tc = 0;
    
    if (Pii::almostEqualRel(discriminant,0.0))
      {
        // the lines are almost parallel
        // use the largest demoninator
        tc = (b>c ? d/b : e/c);
      }
    else
      {
        sc = (b*e - c*d) / discriminant;
        tc = (a*e - b*d) / discriminant;
      }

 
    // get the difference of the two closest points
    // = L1(sc) - L2(tc)
    
    PiiMatrix<double> p(w + (sc * PiiMatrix<T>(1,3,first.row(0))) -
                        (tc * PiiMatrix<T>(1,3,second.row(0))));  

    // return the closest distance
    return Pii::sqrt<T>(Pii::innerProduct(p,p)); 
  }

  /**
   * Point distance from a line segment. 
   *
   * @param line a 2-by-3 (or 2-by-2) matrix which first row contains
   * line segment begin point and second row end point.  
   *
   * @param point is a 1x3 or 1x2 matrix that represents point which
   * distance from line segment we want to calculate.
   *
   * @return point distance from a line segment, if there is anykind problem
   * NAN value will be returned.
   */
  template <class T> T pointToLineSegmentDistance(const PiiMatrix<T> &line, const PiiMatrix<T> &point) 
  {
    /**
     * d = \frac{ |x_0 - x_1| \times | x_0 - x_2 |}{ | x_2 - x_1 |}
     */

    int iCols = line.columns();
    T distance = 0;
    
    if ( iCols == point.columns() )
      {
        PiiMatrix<T> x1(1,iCols, line.row(0));
        PiiMatrix<T> x2(1,iCols, line.row(1));
        
        PiiMatrix<T> diffX1(point - x1);
        PiiMatrix<T> diffX2(point - x2);
        
        PiiMatrix<T> down(x1 - x2);
        
        if ( iCols == 2 )
          {
            diffX1.insertColumn(-1, T(0));
            diffX2.insertColumn(-1, T(0));
            ++iCols;
          }
        
        PiiMatrix<T> up = Pii::crossProduct(diffX1,
                                                diffX2);
                                
        double dUp = 0, dDown = 0;

        for ( int i = 0; i < iCols; ++i)
          {
            dUp += double(up(0,i)*up(0,i));
            dDown += double(down(0,i)*down(0,i));
          }

        distance = static_cast<T>(Pii::sqrt(dUp)/Pii::sqrt(dDown));
        
      }
    else
      distance = NAN;
      

    return distance;
  }

  /// @internal
  template <class T> inline void checkEdge(const PiiPoint<T>& currentVertex,
                                           const PiiPoint<T>& nextVertex,
                                           int& windingNumber,
                                           int x, int y)
  {
#define PII_PROJECTION_SIGN                                     \
    (nextVertex.x - currentVertex.x) * (y - currentVertex.y) -  \
    (nextVertex.y - currentVertex.y) * (x - currentVertex.x)

    if (currentVertex.y <= y)
      {
        if (nextVertex.y > y) // an upward crossing
          if (PII_PROJECTION_SIGN > 0) // point left of edge
            ++windingNumber; // have a valid up intersect
      }
    else
      {
        if (nextVertex.y <= y) // a downward crossing
          if (PII_PROJECTION_SIGN < 0) // point right of edge
            --windingNumber; // have a valid down intersect
      }
#undef PII_PROJECTION_SIGN
  }

  /**
   * Checks if a polygon contains a point.
   *
   * @param polygon a N-by-2 matrix in which each row represents a
   * point (x,y).
   *
   * @param x the x coordinate of the point to check
   *
   * @param y the y coordinate of the point to check
   *
   * @return @p true if @a polygon contains (@a x, @a y), @p false
   * otherwise
   */
  template <class T> bool contains(const PiiMatrix<T>& polygon, int x, int y)
  {

    int iWindingNumber = 0;
    const int iRows = polygon.rows();
  
    // Loop through all edges of the polygon.
    for (int i=0; i<iRows-1; i++)
      checkEdge(polygon.template rowAs<PiiPoint<T> >(i),
                polygon.template rowAs<PiiPoint<T> >(i+1),
                iWindingNumber,
                x, y);
    
    // Optimization: handle end points separately
    checkEdge(polygon.template rowAs<PiiPoint<T> >(iRows-1),
              polygon.template rowAs<PiiPoint<T> >(0),
              iWindingNumber,
              x, y);
    
    return iWindingNumber != 0;
  }
}

#endif //_PIIGEOMETRY_H

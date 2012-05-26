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

#include "PiiImageGlobal.h"
#include <PiiMatrix.h>

#ifndef _PIIBOUNDARYFINDER_H
#define _PIIBOUNDARYFINDER_H

/**
 * An algorithm that traverses object boundaries. The typical use of
 * this class is to detect the boundaries of objects in binary images. 
 * The algorithm extacts both outer and inner boundaries in the order
 * they are first found in the image. Each boundary is represented as
 * a matrix in which each row stores the (x,y) coordinates of a pixel
 * on the boundary.
 *
 * @ingroup PiiImagePlugin
 */
class PII_IMAGE_EXPORT PiiBoundaryFinder
{
public:
  /**
   * Constructs a new boundary finder.
   *
   * @param objects find boundaries in this image. The image is only
   * used for initializing the parameters of the algorithm and will
   * not be stored.
   *
   * @param boundaryMask draw boundaries to this mask as they are
   * traversed. Right and bottom edges will be marked with ones, left
   * and top edges with twos and double edges with three.
   */
  PiiBoundaryFinder(const PiiTypelessMatrix& objects,
                    PiiMatrix<unsigned char>* boundaryMask = 0);

  ~PiiBoundaryFinder();

  /**
   * Finds the next unhandled boundary and returns its coordinates as
   * a N-by-2 matrix. Boundaries are searched from bottom to top.
   *
   * @param objects the detected objects. Each pixel to which @p rule
   * returns @p true when compared to @p value will be treated as an
   * object. Other pixels are treated as non-objects.
   *
   * @param rule the decision rule to match object pixels.
   *
   * @return The coordinates of the object's boundary pixels. Each row
   * of the matrix contains the x and y coordinates of a boundary
   * point, in this order. The last point will be equal to the first
   * one. Thus, the points can be turned into vectors by calculating
   * the difference between successive rows. If no more boundaries can
   * be found, an empty matrix will be returned.
   */
  template <class T, class UnaryOp>
  PiiMatrix<int> findNextBoundary(const PiiMatrix<T>& objects, UnaryOp rule);

  /**
   * Finds the next unhandled boundary and stores its coordinates to
   * @a points. Returns the number of boundary points appended to @a
   * points, or zero if no more boundaries can be found.
   */
  template <class T, class UnaryOp>
  int findNextBoundary(const PiiMatrix<T>& objects, UnaryOp rule, PiiMatrix<int>& points);

  /**
   * Returns the boundary mask. After each iteration
   * (#findNextBoundary()), all detected boundaries are marked into
   * this mask. Right and bottom edges are be marked with ones, left
   * and top edges with twos and double edges with three.
   */
  PiiMatrix<unsigned char> boundaryMask() const;

  /**
   * Extracts the boundary of an object. The boundary is traversed
   * clockwise, and the coordinates of found boundary points will be
   * stored in the returned matrix.
   *
   * @param objects the detected objects. Each pixel to which @p rule
   * returns @p true when compared to @p value will be treated as an
   * object. Other pixels are treated as non-objects.
   *
   * @param rule the decision rule to match object pixels.
   *
   * @param startR the row index of the starting point at the object's
   * boundary.
   *
   * @param startC the column index of the starting point at the
   * object's boundary.
   *
   * @param points the coordinates of the object's boundary pixels
   * will be appended to this matrix. Each row of the matrix will
   * contain the x and y coordinates of a boundary point, in this
   * order. The last point will be equal to the first one. Thus, the
   * points can be turned into vectors by calculating the difference
   * between successive rows.
   *
   * @return the number of boundary points found
   */
  template <class T, class UnaryOp> int findBoundary(const PiiMatrix<T>& objects,
                                                     UnaryOp rule,
                                                     int startR, int startC,
                                                     PiiMatrix<int>& points);

  /**
   * A convenience function that returns the outer boundary of a
   * single labeled object.
   *
   * @param objects a labeled image
   *
   * @param label the object label to look for
   *
   * @param boundaryMask mark the outer edge to this matrix, if
   * non-zero.
   *
   * @return the boundary coordinates, or an empty matrix if the label
   * is not found.
   */
  template <class T> static PiiMatrix<int> findBoundary(const PiiMatrix<T>& objects, T label,
                                                        PiiMatrix<unsigned char>* boundaryMask = 0);


  /**
   * Extracts all outer and inner boundaries of connected pixels that
   * match @p Rule with @p value.
   *
   * @param objects the detected objects. Each pixel to which @p Rule
   * returns @p true when compared to @p value will be treated as an
   * object. Other pixels are treated as non-objects.
   *
   * @param rule the decision rule to match object pixels.
   *
   * @param value use this value as the second argument to @p Rule.
   *
   * @param boundaryMask draw boundaries to this mask as they are
   * traversed. The size of the mask matrix will be adjusted to match
   * @p objects.
   *
   * @return a list of boundary coordinate matrices. One matrix will
   * be returned for each outer or inner boundary. Each row of a
   * coordinate matrix will contain the x and y coordinates of a
   * boundary point, in this order. The last point will be equal to
   * the first one.
   *
   * @code
   * PiiMatrix<int> objects(8,8,
   *                        0,1,2,0,1,1,0,0,
   *                        0,2,2,0,1,1,0,0,
   *                        0,2,2,0,0,0,0,0,
   *                        0,2,2,1,1,1,1,0,
   *                        0,2,2,0,0,0,1,0,
   *                        0,2,2,2,2,3,1,0,
   *                        0,0,0,0,0,0,0,0,
   *                        0,0,0,0,0,0,0,0);
   *
   * // Everything other than zero is an object
   * QList<PiiMatrix<int> > coordinates(findBoundaries(objects, std::bind2nd(std::not_equal_to<int>(), 0)));
   *
   * // The list now has 3 entries, one for the large L-shaped object
   * // with a hole, another for the hole, and one for the small
   * // square of ones.
   * @endcode
   */
  template <class T, class UnaryOp>
  static QList<PiiMatrix<int> > findBoundaries(const PiiMatrix<T>& objects,
                                               UnaryOp rule,
                                               PiiMatrix<unsigned char>* boundaryMask = 0);
  
private:
  /// @internal
  class Data
  {
  public:
    Data(const PiiTypelessMatrix& objects, PiiMatrix<unsigned char>* mask);
    PiiMatrix<unsigned char> matBoundaryMask;
    PiiMatrix<unsigned char>* pmatBoundaryMask;
    int iRow, iColumn, iRightEdge;
  } *d;

  /**
   * Find the last object boundary that has not been processed yet.
   * The boundary of an object is detected when two neighboring
   * pixels return different truth values when @a rule is applied.
   *
   * @param objects an image that contains the objects to be detected.
   *
   * @param rule the decision rule to match object pixels.
   */
  template <class T, class UnaryOp> void findNextUnhandledPoint(const PiiMatrix<T>& objects,
                                                                UnaryOp rule);  
};

#include "PiiBoundaryFinder-templates.h"

#endif //_PIIBOUNDARYFINDER_H

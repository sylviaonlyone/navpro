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

#ifndef _PIIBOUNDARYFINDER_H
# error "Never use <PiiBoundaryFinder-templates.h> directly; include <PiiBoundaryFinder.h> instead."
#endif

#include <PiiMatrixUtil.h>

template <class T, class UnaryOp>
QList<PiiMatrix<int> > PiiBoundaryFinder::findBoundaries(const PiiMatrix<T>& objects,
                                                         UnaryOp rule,
                                                         PiiMatrix<unsigned char>* boundaryMask)
{
  PiiBoundaryFinder finder(objects, boundaryMask);
    
  QList<PiiMatrix<int> > result;
  for (;;)
    {
      // Find last boundary that has not been handled yet
      PiiMatrix<int> boundary = finder.findNextBoundary(objects, rule);
      // No more boundariesx
      if (boundary.isEmpty())
        break;
      result << boundary;
    }
  
  return result;
}

template <class T> PiiMatrix<int> PiiBoundaryFinder::findBoundary(const PiiMatrix<T>& objects, T label,
                                                                  PiiMatrix<unsigned char>* boundaryMask)
{
  int row = 0, col = 0;
  Pii::findLast(objects, label, row, col);
  if (row != -1)
    {
      PiiBoundaryFinder finder(objects, boundaryMask);
      // Coordinate matrix
      PiiMatrix<int> matResult(0,2);
      matResult.reserve(256);
      finder.findBoundary(objects, std::bind2nd(std::equal_to<int>(), label), row, col, matResult);
      return matResult;
    }
  // No boundaries found
  return PiiMatrix<int>(0,2);
}

template <class T, class UnaryOp> PiiMatrix<int> PiiBoundaryFinder::findNextBoundary(const PiiMatrix<T>& objects,
                                                                                     UnaryOp rule)
{
  findNextUnhandledPoint(objects, rule);

  if (d->iRow >= 0)
    {
      // Coordinate matrix
      PiiMatrix<int> matResult(0,2);
      matResult.reserve(256);
      findBoundary(objects, rule, d->iRow, d->iRightEdge, matResult);
      return matResult;
    }

  return PiiMatrix<int>(0,2);
}

template <class T, class UnaryOp> int PiiBoundaryFinder::findNextBoundary(const PiiMatrix<T>& objects,
                                                                          UnaryOp rule,
                                                                          PiiMatrix<int>& points)
{
  findNextUnhandledPoint(objects, rule);

  if (d->iRow >= 0)
    return findBoundary(objects, rule, d->iRow, d->iRightEdge, points);

  return 0;
}


template <class T, class UnaryOp> void PiiBoundaryFinder::findNextUnhandledPoint(const PiiMatrix<T>& objects,
                                                                                 UnaryOp rule)
{
  // If we reached the beginning of a row, next iteration must start
  // at the end of the previous one.
  if (d->iColumn == -1)
    {
      d->iColumn = objects.columns()-1;
      --d->iRow;
    }
  // After the first row we always start at the end.
  for ( ;d->iRow >= 0; --d->iRow, d->iColumn = objects.columns()-1)
    {
      const T* objectsRow = objects.row(d->iRow);
      const unsigned char* maskRow = d->pmatBoundaryMask->row(d->iRow);

      while (d->iColumn >= 0)
        {
          // Find right edge
          while (!rule(objectsRow[d->iColumn]))
            {
              // The beginning of the row was empty
              if (d->iColumn == 0)
                goto nextRow;
              --d->iColumn;
            }
          
          
          // Store right edge position
          d->iRightEdge = d->iColumn;
                     
          // Find the left edge
          while (d->iColumn >= 0 && rule(objectsRow[d->iColumn])) --d->iColumn;

          if ((maskRow[d->iRightEdge] & 1) == 0)
            return;
        }
    nextRow:;
    }
}
 
template <class T, class UnaryOp> int PiiBoundaryFinder::findBoundary(const PiiMatrix<T>& objects,
                                                                      UnaryOp rule,
                                                                      int startR, int startC,
                                                                      PiiMatrix<int>& points)
{
  // Directions to go along the boundary. The last number is the
  // minimum number of turns (clockwise) at a given direction that
  // causes the traversed boundary to be a double boundary. The number
  // includes +2 for even directions and +3 for odd directions,
  // because that's the number of counter-clockwise directions the
  // algorithm starts looking at.
  static const int directions[21][3] = {
    { 1,  0, 8}, //E
    { 1,  1, 6}, //SE
    { 0,  1, 4}, //S
    {-1,  1, 4}, //SW
    {-1,  0, 8}, //W
    {-1, -1, 6}, //NW
    { 0, -1, 4}, //N
    { 1, -1, 4}, //NE
    // The data is repeated for optimization purposes
    { 1,  0, 8}, //E
    { 1,  1, 6}, //SE
    { 0,  1, 4}, //S
    {-1,  1, 4}, //SW
    {-1,  0, 8}, //W
    {-1, -1, 6}, //NW
    { 0, -1, 4}, //N
    { 1, -1, 4}, //NE
    { 1,  0, 8}, //E
    { 1,  1, 6}, //SE
    { 0,  1, 4}, //S
    {-1,  1, 4}, //SW
    {-1,  0, 8}  //W
  };
  
  int r = startR, c = startC;
  int testR, testC;
  int iPoints = 0;

  // Start looking down
  int currentDir = 2;
  // Test directions clockwise. Go to the first boundary point
  // found. The magic equation defines the first possible location
  // of the next boundary point. For even angles it is one turn
  // counter-clockwise (+7). For odd angles, it is two turns
  // counter-clockwise (+6).
  int firstPossibleDir = (currentDir | 1) + 6;
  do
    {
      //qDebug("At (%d, %d)", r, c);
      for (int dirIndex = firstPossibleDir; dirIndex < firstPossibleDir + 8; dirIndex++)
        {
          //qDebug("Testing dir %d", dirIndex & 0x7);
          testC = c + directions[dirIndex][0];
          testR = r + directions[dirIndex][1];
          if (testC >= 0 && testR >= 0 &&
              testC < objects.columns() && testR < objects.rows() &&
              rule(objects(testR, testC)))
            {
              // Add to the list of boundary points
              points.appendRow(c, r);
              ++iPoints;

              int turns = dirIndex - firstPossibleDir;
              
              /* Add this edge to the boundary mask.
               *
               * If the direction turned too many steps, the pixel is
               * a double outer edge and must be marked twice. In the
               * examples, X's and M's represent object pixels. M is a
               * pixel that needs to be specially marked:
               *
               * M M   M   M  M M  X  X XX XX XXX  X
               * X XX XXX XX X   X  M M  M M   M  M
               * 6 6   7  7  7  5  1  2  2 3   3  3  currentDir
               * 4 3   2  3  4  4  4  4  3 3   2  4  turns from currentDir
               */

              unsigned char& mark = (*d->pmatBoundaryMask)(r,c);
              
              // These are the double edges
              if (turns >= directions[currentDir][2])
                {
                  // If the edge is not marked, mark it. If it has
                  // been marked, do nothing.
                  if (mark == 0)
                    (*d->pmatBoundaryMask)(r,c) = 3;
                }
              // This blocks out double edges pointing inwards. If the
              // current direction is either SE or NW, the first
              // possible boundary point (turns == 0) means an inwards
              // pointing double edge (SE->NE or NW->SW).
              else if (turns != 0 || (currentDir != 1 && currentDir != 5))
                // 1 = right edge, 2 = left edge
                mark |= (currentDir >> 2) + 1;

              /*qDebug("Turning to %d at (%d,%d). Turns: %d, maxTurns: %d, Boundary mask: %d",
                     dirIndex & 0x7, r,c,
                     dirIndex - firstPossibleDir, directions[dirIndex][2],
                     int(mark));*/
              
              // Update directions
              currentDir = dirIndex & 0x7;
              firstPossibleDir = (currentDir | 1) + 6;

              c = testC; r = testR;
                
              // Found boundary. Go to next step.
              break;
            }
        }
    } while (r != startR || c != startC);

  points.appendRow(startC, startR);
  ++iPoints;

  // Special case: only one pixel
  if (iPoints == 1)
    (*d->pmatBoundaryMask)(r,c) = 3;
  // Special case: boundary start point is a double edge.
  // Starts SE, ends NE
  else if (currentDir == 7)
    {
      int iFirstRow = points.rows() - iPoints;
      if (points(iFirstRow+1,0) - points(iFirstRow,0) == 1 &&
          points(iFirstRow+1,1) - points(iFirstRow,1) == 1)
        (*d->pmatBoundaryMask)(startR, startC) = 3;
    }
  
  return iPoints;
}

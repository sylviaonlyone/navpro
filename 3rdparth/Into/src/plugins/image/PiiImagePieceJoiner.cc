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

#include "PiiImagePieceJoiner.h"
#include <PiiYdinTypes.h>
#include <PiiColor.h>
#include <complex>

using namespace Pii;
using namespace PiiYdin;

PiiImagePieceJoiner::Data::Data() :
  bTransparent(false), clrBackground(Qt::black),
  largeImage(0), bDiscardDefault(true),
  iLeftX(0), iTopY(0)
{
}

PiiImagePieceJoiner::PiiImagePieceJoiner() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  d->pImageInput = new PiiInputSocket("image");
  d->pLocationInput = new PiiInputSocket("location");
  d->pLocationInput->setOptional(true);

  d->pRectangleInput = new PiiInputSocket("area");
  d->pLabelInput = new PiiInputSocket("label");
  d->pRectangleInput->setGroupId(1);
  d->pLabelInput->setGroupId(1);
  d->pLabelInput->setOptional(true);

  d->pPieceOutput = new PiiOutputSocket("image");
  d->pRectangleOutput = new PiiOutputSocket("location");
  d->pLabelOutput = new PiiOutputSocket("label");

  addSocket(d->pImageInput);
  addSocket(d->pLocationInput);
  addSocket(d->pRectangleInput);
  addSocket(d->pLabelInput);

  addSocket(d->pPieceOutput);
  addSocket(d->pRectangleOutput);
  addSocket(d->pLabelOutput);
}

void PiiImagePieceJoiner::check(bool reset)
{
  PII_D;
  
  PiiDefaultOperation::check(reset);

  if (reset)
    {
      d->iLeftX = d->iTopY = 0;
    }
}

void PiiImagePieceJoiner::process()
{
  PII_D;
  if (activeInputGroup() == 1)
    {
      PiiVariant obj = d->pRectangleInput->firstObject();
      if (obj.type() == IntMatrixType)
        {
          const PiiMatrix<int> mat = obj.valueAs<PiiMatrix<int> >();
          if (mat.rows() < 1 || mat.columns() != 4)
            PII_THROW_WRONG_SIZE(d->pRectangleInput, mat, 1, 4);

          // Take the location and build up a QRect out of it.
          d->rectList << QRect(mat(0,0), mat(0,1), mat(0,2), mat(0,3));
        }
      else
        PII_THROW_UNKNOWN_TYPE(d->pRectangleInput);

      obj = d->pLabelInput->firstObject();
      int label = -1;
      if (obj.isValid())
        {
          switch (obj.type())
            {
              PII_PRIMITIVE_CASES(label = (int)PiiYdin::primitiveAs, obj);
            default:
              PII_THROW_UNKNOWN_TYPE(d->pLabelInput);
            }
        }
      d->labelList << label;
    }
  else
    {
      d->largeImage = d->pImageInput->firstObject();
      if (d->pLocationInput->isConnected())
        readLocation();
        
      //qDebug("PiiImagePieceJoiner: read main image (type 0x%x).", d->largeImage.type());
    }
}

void PiiImagePieceJoiner::syncEvent(SyncEvent* event)
{
  PII_D;
  if (event->type() == SyncEvent::EndInput &&
      event->groupId() == d->pImageInput->groupId())
    {
      //qDebug("PiiImagePieceJoiner: inputs in group %d synchronized.", event->groupId());
      d->pPieceOutput->startMany();
      d->pRectangleOutput->startMany();
      d->pLabelOutput->startMany();

      //qDebug("PiiImagePieceJoiner: joining");
      joinPieces();

      // Finalize the emission of an object bundle
      d->pPieceOutput->endMany();
      d->pRectangleOutput->endMany();
      d->pLabelOutput->endMany();
    }
}

void PiiImagePieceJoiner::readLocation()
{
  PII_D;
  PiiVariant locationObj = d->pLocationInput->firstObject();
  if (locationObj.type() != PiiYdin::IntMatrixType)
    PII_THROW_UNKNOWN_TYPE(d->pLocationInput);
  const PiiMatrix<int> mat = locationObj.valueAs<PiiMatrix<int> >();
  if (mat.columns() < 2 && mat.rows() != 1)
    PII_THROW_WRONG_SIZE(d->pLocationInput, mat, 1, "2-N");
  d->iLeftX = mat(0,0);
  d->iTopY = mat(0,1);
}

void PiiImagePieceJoiner::joinPieces()
{
  PII_D;
  if (d->rectList.size() == 0)
    return;
  
  QLinkedList<QPair<int,int> > pairs;

  /*QRect r1(0,0,10,10), r2(10,0,10,10), r3(20,0,10,10),
    r4(0,10,10,10), r5(10,10,10,10), r6(20,10,10,10),
    r7(0,20,10,10), r8(10,20,10,10), r9(20,20,10,10);

  qDebug("Neighbors:");
  qDebug("r1,r5: %s", isNeighbor(r1,r5) ? "yes" : "no");
  qDebug("r2,r5: %s", isNeighbor(r2,r5) ? "yes" : "no");
  qDebug("r3,r5: %s", isNeighbor(r3,r5) ? "yes" : "no");
  qDebug("r4,r5: %s", isNeighbor(r4,r5) ? "yes" : "no");
  qDebug("r5,r5: %s", isNeighbor(r5,r5) ? "yes" : "no");
  qDebug("r6,r5: %s", isNeighbor(r6,r5) ? "yes" : "no");
  qDebug("r7,r5: %s", isNeighbor(r7,r5) ? "yes" : "no");
  qDebug("r8,r5: %s", isNeighbor(r8,r5) ? "yes" : "no");
  qDebug("r9,r5: %s", isNeighbor(r9,r5) ? "yes" : "no");*/

  //qDebug("PiiImagePieceJoiner: Joining %d pieces", d->rectList.size());
  
  // First find all pairs of neighboring pieces (a piece is a neighbor
  // to itself)
  for (int i=d->rectList.size(); i--; )
    {
      if (//d->labelList[i] >= 0 && // negative values are unknown and always discarded
          (!d->bDiscardDefault || d->labelList[i] != 0)) // we need to consider also label 0
        {
          for (int j=i; j--; )
            if (d->labelList[i] == d->labelList[j] && //if the pieces are in the same class
                isNeighbor(d->rectList[i], d->rectList[j])) //... and they are neighbors
              pairs << qMakePair(i,j);
        }
    }

  // Then recursively join all neighbors
  while (!pairs.isEmpty())
    {
      // Stores the indices (in d->rectList) of all neighbors
      QList<int> indices;
      indices << pairs.first().first;
      // This removes all neighbors of the first item from <pairs>
      joinNeighbors(pairs.first().first, pairs, indices);

      //qDebug("PiiImagePieceJoiner: Joined %d neighbors. %d pairs left.", indices.size(), pairs.size());
      // Build up a compound that encloses all the neighbors
      if (d->bTransparent)
        {
          // If transparency is used, we need to collect all the
          // joined rectangles and copy each to a new image.
          QList<QRect*> subAreas;
          QRect area = d->rectList[indices[0]];
          subAreas << &d->rectList[indices[0]];
          for (int i=1; i<indices.size(); i++)
            {
              area |= d->rectList[indices[i]];
              subAreas << &d->rectList[indices[i]];
            }
          emitCompound(area, subAreas);
        }
      else
        {
          // If transparency is not used, it suffices to frame the
          // rectangles and send that as a shared copy.
          QRect area = d->rectList[indices[0]];
          for (int i=1; i<indices.size(); i++)
            area |= d->rectList[indices[i]];
          emitCompound(area);
        }
      
      // Send its label
      d->pLabelOutput->emitObject(d->labelList[indices[0]]);
      //qDebug("PiiImagePieceJoiner: finished this compound");
    }
  
  // Initialize the lists of rectangles and labels
  d->rectList.clear();
  d->labelList.clear();
}

// For transparency
void PiiImagePieceJoiner::emitCompound(QRect area, QList<QRect*>& subAreas)
{
  PII_D;
  QPair<QRect,QList<QRect*>* > pair(area, &subAreas);
  //qDebug("PiiImagePieceJoiner: emitting compound (%d x %d)", area.width(), area.height());
  switch (d->largeImage.type())
    {
      PII_ALL_MATRIX_CASES(emitSubImage, pair);
      PII_COLOR_IMAGE_CASES(emitSubImage, pair);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }
  //qDebug("PiiImagePieceJoiner: sent to sub-image output.");
  d->pRectangleOutput->emitObject(PiiMatrix<int>(1, 4, area.x(), area.y(), area.width(), area.height()));
  //qDebug("PiiImagePieceJoiner: sent to location output.");
}

template <class T> struct ColorConverter { static T value(QColor c) { return T(c.red()); } };
template <class T> struct ColorConverter<std::complex<T> > { static T value(QColor /*c*/) { return 0; } };
template <class T> struct ColorConverter<PiiColor<T> > { static PiiColor<T> value(QColor c) { return PiiColor<T>(c.red(), c.green(), c.blue()); } };
template <class T> struct ColorConverter<PiiColor4<T> > { static PiiColor4<T> value(QColor c) { return PiiColor4<T>(c.red(), c.green(), c.blue(), c.alpha()); } };

template <class T> void PiiImagePieceJoiner::emitSubImage(QPair<QRect,QList<QRect*>* >& pair)
{
  PII_D;
  QRect area = pair.first;
  int row = area.y(), column = area.x(), rows = area.height(), columns = area.width();
  QList<QRect*>* subAreas = pair.second;
  const PiiMatrix<T> largeImage = d->largeImage.valueAs<PiiMatrix<T> >();

  // Create an empty matrix to which we copy the original image data
  PiiMatrix<T> matPiece(PiiMatrix<T>::uninitialized(rows, columns));
  // Fill the image with the selected background color
  matPiece = ColorConverter<T>::value(d->clrBackground);
    
  for (int i=subAreas->size(); i--; )
    {
      QRect *subArea = subAreas->at(i);
      // Copy this image piece
      matPiece(subArea->y() - row, subArea->x() - column, subArea->height(), subArea->width()) <<
        largeImage(subArea->y() - d->iTopY, subArea->x() - d->iLeftX, subArea->height(), subArea->width());
    }
  d->pPieceOutput->emitObject(matPiece);
}

void PiiImagePieceJoiner::emitCompound(QRect area)
{
  PII_D;
  //qDebug("PiiImagePieceJoiner: emitting compound (%d x %d)", area.width(), area.height());
  switch (d->largeImage.type())
    {
      PII_ALL_MATRIX_CASES(emitSubImage, area);
      PII_COLOR_IMAGE_CASES(emitSubImage, area);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }
  //qDebug("PiiImagePieceJoiner: sent to sub-image output.");
  d->pRectangleOutput->emitObject(PiiMatrix<int>(1, 4, area.x(), area.y(), area.width(), area.height()));
  //qDebug("PiiImagePieceJoiner: sent to location output.");
}

template <class T> void PiiImagePieceJoiner::emitSubImage(QRect area)
{
  PII_D;
  //PiiMatrix<T>* piece = new PiiMatrix<T>(d->largeImage.valueAs<PiiMatrix<T> >(), area.y(), area.x(), area.height(), area.width());
  //qDebug("PiiImagePieceJoiner: created sub-image (%d,%d,%d,%d).", area.y(), area.x(), area.height(), area.width());
  d->pPieceOutput->emitObject(d->largeImage.valueAs<PiiMatrix<T> >()(area.y() - d->iTopY, area.x() - d->iLeftX, area.height(), area.width()));
}

bool PiiImagePieceJoiner::isNeighbor(QRect r1, QRect r2)
{
  // We check for 4-neighbors (rectangles are neighbors if they
  // intersect or they are side-by side, but not diagonally)
  return !((r2.right() < (r1.left()-1) ||
            r2.left() > (r1.right()+1) ||
            r2.bottom() < r1.top() ||
            r2.top() > r1.bottom()) &&
           (r2.right() < r1.left() ||
            r2.left() > r1.right() ||
            r2.bottom() < (r1.top()-1) ||
            r2.top() > (r1.bottom())+1));
}

void PiiImagePieceJoiner::joinNeighbors(int index, QLinkedList<QPair<int,int> >& pairs, QList<int>& indices)
{
  //qDebug("Finding neighbors for %d", index);
  QList<int> newIndices;
  QLinkedList<QPair<int,int> >::iterator i = pairs.begin();
  while (i != pairs.end())
    {
      // If a match was found in the pair, add the other index in the
      // pair to the index list. Then remove the found match from the
      // list.
      if (i->first == index)
        {
          //qDebug("  %d matches", i->second);
          newIndices << i->second;
          i = pairs.erase(i);
        }
      else if (i->second == index)
        {
          //qDebug("  %d matches", i->first);
          newIndices << i->first;
          i = pairs.erase(i);
        }
      else
        i++;
    }
  // Now we have found all direct neighbors of <index>. Let's find
  // (recursively) the neighbors of its neighbors.
  for (int i=newIndices.size(); i--; )
    if (newIndices[i] != index) //no need to check myself again...
      joinNeighbors(newIndices[i], pairs, indices);

  // Store all indices to a single list
  indices << newIndices;
}

bool PiiImagePieceJoiner::isTransparent() const { return _d()->bTransparent; }
void PiiImagePieceJoiner::setTransparent(bool transparent) { _d()->bTransparent = transparent; }
QColor PiiImagePieceJoiner::backgroundColor() const { return _d()->clrBackground; }
void PiiImagePieceJoiner::setBackgroundColor(QColor clr) { _d()->clrBackground = clr; }
bool PiiImagePieceJoiner::discardDefault() const { return _d()->bDiscardDefault; }
void PiiImagePieceJoiner::setDiscardDefault(bool discard) { _d()->bDiscardDefault = discard; }

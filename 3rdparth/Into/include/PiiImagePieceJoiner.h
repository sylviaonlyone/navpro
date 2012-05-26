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

#ifndef _PIIIMAGEPIECEJOINER_H
#define _PIIIMAGEPIECEJOINER_H

#include <PiiDefaultOperation.h>
#include <QList>
#include <QLinkedList>
#include <QPair>
#include <QColor>
#include <QRect>

/**
 * An operation that combines adjacent sub-images to larger compounds. 
 * This operation is used in situations where multiple sub-images are
 * extracted from a single large image and selected sub-images need to
 * be combined to form larger sub-images. The main use is in
 * recognizing large defects in defect detection. Each sub-image has a
 * label that tells the class of the sub-image. Adjacent sub-images
 * with the same class label are joined together to form continuous
 * regions.
 *
 * @inputs
 *
 * @in image - the large image the pieces are extracted from (any
 * matrix or image type)
 *
 * @in location - an optional input specifying the location of the
 * input area within a larger image. If this input is connected,
 * emitted locations will be subtracted by the upper left corner of
 * the input location. The location is expected to be a PiiMatrix<int>
 * with one row and at least two columns. The first two elements of
 * the matrix are treated as the (x,y) coordinates of the upper left
 * corner. This input is always coupled with the @a image input.
 *
 * @in area - a rectangle bounding a sub-image within the large
 * image. Multiple rectangles may be read for each large image. 
 * (1-by-4 PiiMatrix<int>, interpreted as a PiiRectangle). This inputs
 * works in sync with @p label and assumes objects at a higher flow
 * level than @p image. Suitable input can be obtained for example
 * from PiiImageSplitter.
 *
 * @in label - an integer that labels the incoming area. This input is
 * optional. If it is not connected, -1 will be used. (any primitive
 * type, converted to an @p int)
 *
 * @outputs
 *
 * @out image - any number of sub-images extracted from the main
 * image. The sub-images are created by finding the bounding box of
 * all incoming rectanges that are neighbors to each other. (any
 * matrix or image type)
 *
 * @out location - the location of the compound sub-image
 * with respect to the large image. (1-by-4 PiiMatrix<int>,
 * interpreted as a PiiRectangle)
 *
 * @out label - the label of the compound sub-image.
 *
 * @ingroup PiiImagePlugin
 */
class PiiImagePieceJoiner : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * A flag that indicates whether the piece joiner uses transparency
   * or not. Transparency might be needed if the joined images don't
   * form a rectangular area. For example, if three pieces are joined
   * as shown below, background color is used for the piece marked
   * with a question mark. If transparency is not in use, the missing
   * slot is filled with original image data.
   *
@verbatim
+---+---+
|   |   |
+---+---+
  ? |   |
    +---+
@endverbatim
   *
   * Note that transparent mode is not as efficient as the
   * non-transparent one because original image data cannot be shared.
   */
  Q_PROPERTY(bool transparent READ isTransparent WRITE setTransparent);

  /**
   * The color for transparent parts of the image. You need to use
   * RGBA for real transparency. The default value is black, which
   * isn't really transparent, but works also for images with only
   * three color channels. The background color has no effect on
   * complex-valued matrices, for which zero is always used as the
   * background. For gray-level images, the red channel is used.
   */
  Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor);

  /**
   * A flag that indicates whether the default label (0) is discarded
   * or not. By default, all sub-images labeled with zero are not
   * considered in joining the components. Set to @p false to make the
   * default label also form connected components.
   */
  Q_PROPERTY(bool discardDefault READ discardDefault WRITE setDiscardDefault);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiImagePieceJoiner();

  bool isTransparent() const;
  void setTransparent(bool transparent);

  QColor backgroundColor() const;
  void setBackgroundColor(QColor clr);

  bool discardDefault() const;
  void setDiscardDefault(bool discard);

  void check(bool reset);

protected:
  void process();
  void syncEvent(SyncEvent* event);
  
private:
  void readLocation();
  void joinPieces();
  void emitCompound(QRect area);
  void emitCompound(QRect area, QList<QRect*>& subAreas);
  inline bool isNeighbor(QRect r1, QRect r2);
  void joinNeighbors(int index, QLinkedList<QPair<int,int> >& pairs, QList<int>& indices);
  template <class T> void emitSubImage(QRect area);
  template <class T> void emitSubImage(QPair<QRect,QList<QRect*>* >& pair);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    bool bTransparent;
    QColor clrBackground;
    
    PiiVariant largeImage;
    
    bool bDiscardDefault;
    
    QList<QRect> rectList;
    QList<int> labelList;
    
    PiiInputSocket* pImageInput, *pLocationInput, *pRectangleInput, *pLabelInput;
    PiiOutputSocket* pPieceOutput, *pRectangleOutput, *pLabelOutput;

    int iLeftX, iTopY;
  };
  PII_D_FUNC;
};

#endif //_PIIIMAGEPIECEJOINER_H

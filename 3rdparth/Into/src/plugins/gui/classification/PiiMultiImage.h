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

#ifndef _PIIMULTIIMAGE_H
#define _PIIMULTIIMAGE_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QtGui>
#include <QListWidget>
#include <stdlib.h> // rand()

#include "PiiMultiImageDisplay.h"
#include "PiiVisualSelector.h"
#include "PiiIconListWidget.h"
#include "PiiMultiImageDisplay.h"

class PiiMultiImageDisplay;
/**
 * Class for storing pointers multiple images.
 * 
 **/
class PiiMultiImage : public QObject {
  Q_OBJECT
    
 public:
 
  PiiMultiImage();
  ~PiiMultiImage();
  
  void addImage(QImage* im);

  /**
   * removes given image. if im=0, current image is removed.
   **/
  void removeImage(QImage* im=0);
  
  QImage* currentImage() const;
  
  int imageCount()const ;
  
  int currentIndex() const ;

  int indexOf(QImage* im) const;

  QImage* image(int index) const;
  
  /*
   * Show widget that contains thumbnails of this PiiMultiImage
   */
  void showIconListWidget(QWidget *pEditorWidget, const QString& title);
  void updateIconListWidget(QWidget *pEditorWidget);
  bool hasIconListWidget();
  
  /**
   * If param is 0, current image is used.
   * If contains no images, returns 0
   **/
  void showImageDisplay(QImage* im = 0);
  void deleteAllImageDisplays();
  void notifyAllImageDisplays(QImage *image);

  QList<QImage*> images() const;

  void setUnitScale(const QSizeF& unitScale) { _unitScale = unitScale; }
  
  /**
   * Returns true if component is selected
   */
  bool isSelected() const;
  
  /**
   * Returns true if any image in this component contain given point.
   * images boundary is defined using offset and width and height.
   */
  bool imagesContain(QPoint p);

  /**
   * Returns true if any of the images in this component intersect the
   * given rectangle.
   */
  bool imagesIntersect(QRect r);
  
  bool contains(QImage* image);
  
protected:
  
  QList<QImage*> _images;    // List of images on this MultiImage
  int            _current;   // Index of current image
  QSizeF _unitScale;
  bool _selected;                 // Is multiImage selectedor not
  
  QPointer<PiiIconListWidget> _pIconListWidget;
  QList<QPointer<PiiMultiImageDisplay> > _lstMultiImageDisplays;
  
  
  
public slots:
    
    /**
     * Do the selection, if selection changes emits imagesSelected signal
     * if silent (default false) is true, no emit is done.
     **/
  void select(bool val);
  
 void nextImage();
 void prevImage();
 
 void removeAt(int index);
 void removeImages();
 void setCurrentIndex(int index);
 
 void showContentOfItem(QListWidgetItem *item);
 
 //signals:
  
  // Sends a signal when this multiimage has been selected or deselected
  // The pointer of current image is also send
  // These are emitted on mouse clicks
 //  void imageSelected(QImage* img, bool selected);
 //  void componentSelected(PiiMultiImage* w, bool selected);

};

#endif //_PIIMULTIIMAGE_H

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

#ifndef _PIIMULTIIMAGEDISPLAY_H
#define _PIIMULTIIMAGEDISPLAY_H

#include "PiiVisualSelector.h"
#include "PiiMultiImage.h"

#include <QtGui>
#include <QImage>
#include <QSizeF>

class PiiMultiImage;
class PiiImageViewport;
class PiiImageDisplay;

class PiiMultiImageDisplay : public QWidget
{
  Q_OBJECT

public:

  enum Direction { Forward, Backward };
  
  PiiMultiImageDisplay(PiiMultiImage* multiimage, QImage* im=0, QWidget* editorWidget = 0);
  ~PiiMultiImageDisplay();
  
  void setPixelSize(const QSizeF& pixelSize);
  
  /**
   * This function must be called when the image 'image' will be
   * destroyed soon.
   */
  void imageDestroyed(const QImage *image);

protected:
  void keyPressEvent(QKeyEvent *e);
  
  PiiMultiImage* _pMultiimage;
  PiiImageViewport* _pImageViewport;
  PiiImageDisplay* _pImageDisplay;
  int _index;

public slots:
  void nextImage();
  void prevImage();

private:
  void changeImage(Direction direction);
  QImage* readFromFile(QImage *image);
  void setImage(QImage *image);
  
  QImage *_pCopyImage, *_pBigImage, *_pCurrentImage;
};

#endif //_PIIMULTIIMAGEDISPLAY_H

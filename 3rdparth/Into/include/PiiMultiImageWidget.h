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

#ifndef _PIIMULTIIMAGEWIDGET_H
#define _PIIMULTIIMAGEWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QtGui>
#include <stdlib.h> // rand()

#include "PiiLabel.h"

class PiiMultiImageWidget : public QWidget 
{  /*public QTableWidgetItem,*/
  Q_OBJECT
    
public:
  PiiMultiImageWidget(QWidget* parent=0);
 
  void paintEvent(QPaintEvent* e);
  
  void addImage(QImage* im); // ADD IMAGE
  void removeImage(QImage* im); // REMOVE GIVEN IMAGE
  void removeAt(int index);  // REMOVE IMAGE FROM INDEX
  void removeImages();       // REMOVE ALL IMAGES
  
  int imageCount(){return _lstImages.size();}
  
  int currentIndex(){ return current;}
  
  void setSelectedIndex(int index){
    if(index<0 || index>_lstImages.size())
      return;
    sel = index; update();
  }
  
  /*
   * Returns ListWidget that containg thumbnails of this PiiMultiImageWidget 
   */
  QListWidget* createIconListWidget();
  
  PiiLabel* label() const { 
    return _label;
  }
  void setLabel(PiiLabel* label=0);
  
  QList<QImage*> images() const { return _lstImages;}
  
  /**
   * Returns true if component is selected
   **/
  bool selected(){return sel;}
  
  /**
   * Returns true if any image in this component contain given point.
   * images boundary is defined using offset and width and height.
   **/
  bool imagesContain(QPoint p);
  
public slots:
  
  void removeLabel(){
    _label = 0;
    emit componentSelected(this,sel); 
    update();
  }
  
  void select(bool val) {
    sel = val;
    emit componentSelected(this,sel);
    update();
  }
  
 protected:
  
  void mousePressEvent  ( QMouseEvent *e);
  void mouseReleaseEvent( QMouseEvent *e);
  void mouseMoveEvent   ( QMouseEvent *e);
  
  QList<QImage*> _lstImages;
  int current;
  
  //REMOVE:
  //QColor groupcolor;
  
  //PiiLabel* label;
  QPointer<PiiLabel> _label;
  
  bool sel;
  
 signals:
  
  // Sends a signal when this multiimage has been selected or deselected
  // The pointer of current image is also send
  // These are emitted on mouse clicks
  void imageSelected(QImage* img, bool selected);
  void componentSelected(PiiMultiImageWidget* w, bool selected);

};

#endif //_PIIMULTIIMAGEWIDGET_H

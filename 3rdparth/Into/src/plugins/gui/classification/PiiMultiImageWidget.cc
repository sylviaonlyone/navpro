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

#include "PiiMultiImageWidget.h"

PiiMultiImageWidget::PiiMultiImageWidget(QWidget* parent): QWidget(parent)
{
  _label = new PiiLabel("",QColor(0,0,0));
  sel = 0;
  current = 0;
}

void PiiMultiImageWidget::paintEvent(QPaintEvent* /*e*/){
  //  printf("MULTIIMAGE PAINTEVENT, current: %i\n ",current);
  
  //while(current>_lstImages.size() ) current--;
  
  QPainter p(this);
  p.setClipRect(QRect (0,0,width(),height()));
  
  if(_lstImages.size()>0){
    int w = width();
    int h = height();
    
    float aspect = (float)width() / (float)height();
    float imageaspect = (float)_lstImages[current]->width() / (float)_lstImages[current]->height();
    
    if(aspect > imageaspect)
      w = (int)( ((float)h / (float)_lstImages[current]->height()) * (float)_lstImages[current]->width());
    //((float)height() /(float)_lstImages[current]->height());
    else h = (int)( ((float)w / (float)_lstImages[current]->width()) * (float)_lstImages[current]->height());
    // ((float)width() /(float)_lstImages[current]->width());
    
    //p.drawPixmap(0,0, _lstImages[current]->scale(w,h));
    p.drawImage(0,0, _lstImages[current]->scaled(w,h));
    
    if(_label!=0){
      p.setPen(_label->color);
      
      //p.drawRect(0,0,_lstImages[current]->width()-1,_lstImages[current]->height()-1);
      p.drawRect(0,0,width()-1,height()-1);
    }
    QString s;
    s = QString("%1/%2").arg(current+1).arg(_lstImages.size());
    
    p.drawText(2,10,s);
  } else {
    if(_label!=0){	
      p.setPen(_label->color);
    }
    p.drawRect(0,0,width()-1,height()-1);
  }
  
  if(sel){
    if(_label!=0){	
		 p.setPen(_label->color);
		 p.setBrush(QColor(_label->color.red(),_label->color.green(),_label->color.blue(),60));
    } else {
      //p.setPen(QColor( ));
		 //p.setBrush(QColor(_label->color.red(),_label->color.green(),_label->color.blue(),60));
    }
    p.drawRect(0,0,width()-1,height()-1);
  }
}
//------------------------------------------------------------------

void PiiMultiImageWidget::addImage(QImage* im){
  _lstImages.append(im);
  current = _lstImages.lastIndexOf(im);
  update();
  // Notify only if this is selected...
  if(sel)emit componentSelected(this,sel);
}

void PiiMultiImageWidget::removeImage(QImage* im){
   //int index = _lstImages.indexOf(im);
   // UGLY HACK:
  if(sel) emit componentSelected(this,false);
  
  _lstImages.removeAll(im);
  current = _lstImages.size()-1;
  if(sel)emit componentSelected(this,sel);
  //  printf("CURRENT IS %i \n",current);
}

void PiiMultiImageWidget::removeAt(int index){
  if(sel) emit componentSelected(this,false);
   _lstImages.removeAt(index);
   current = _lstImages.size()-1;
   // printf("CURRENT IS %i \n",current);
   if(sel)emit componentSelected(this,sel);
   update();
}

void PiiMultiImageWidget::removeImages(){
  if(sel) emit componentSelected(this,false);
  _lstImages.clear();
  current = _lstImages.size()-1;
  if(sel) emit componentSelected(this,sel);
  update();
}

void PiiMultiImageWidget::setLabel(PiiLabel* label){
  _label = label;
  //printf("MIW LABEL %s \n",_label->name.ascii());
  emit componentSelected(this,sel);
  update();
}

bool PiiMultiImageWidget::imagesContain(QPoint p){
  if(_lstImages.size()>0)
    for (int j = 0; j < _lstImages.size(); ++j){
      QPoint offset = _lstImages.at(j)->offset();
      QRect rec(offset.x(),offset.y(),_lstImages.at(j)->width(),_lstImages.at(j)->height());
      if(rec.contains(p))return true;
    }
  return false;
}

#include <stdio.h>

void PiiMultiImageWidget::mousePressEvent  ( QMouseEvent *e){
  
  if(e->button()&Qt::RightButton){
    // printf("mouse pressed %i\n",current);
    current++;
    if(_lstImages.size()-1 < current)
      current = 0;
  } if(e->button()&Qt::LeftButton){
    sel = !sel; 
    
    if(_lstImages.size()>0)
      emit imageSelected(_lstImages[current],sel);
    else emit imageSelected(0,sel);
    
    // Notify the selection to slots
    emit componentSelected(this,sel);
  }
  //  if(_lstImages.size()>0)
  //	 setIcon(QIcon(QPixmap( *_lstImages[current] )));
  //  else setIcon(QIcon(QPixmap("images/test.bmp")));
  update();
}

void PiiMultiImageWidget::mouseReleaseEvent( QMouseEvent* /* e */){}
void PiiMultiImageWidget::mouseMoveEvent   ( QMouseEvent* /* e */){}

QListWidget* createIconListWidget(){
  QListWidget *listWidget = new QListWidget();
  
  (new QListWidgetItem("Oak", listWidget))->setIcon( QIcon(QPixmap("images/test.bmp")));
  new QListWidgetItem("Fir", listWidget);
  new QListWidgetItem("Pine", listWidget);
  // QListWidgetItem *newItem = new QListWidgetItem;
  // newItem->setText("testing");
   // listWidget->insertItem(0, newItem);
  listWidget->setViewMode(QListView::IconMode);
  return listWidget;
}

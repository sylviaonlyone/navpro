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

#include "PiiMultiImage.h"
#include "PiiMultiImageDisplay.h"

PiiMultiImage::PiiMultiImage()
{
  _selected = 0;
  _current = 0;
  _pIconListWidget = 0;
}

PiiMultiImage::~PiiMultiImage()
{
  deleteAllImageDisplays();
  delete _pIconListWidget;
}

void PiiMultiImage::deleteAllImageDisplays()
{
  while(_lstMultiImageDisplays.size() > 0)
    delete _lstMultiImageDisplays.takeFirst();
}

void PiiMultiImage::notifyAllImageDisplays(QImage *image)
{
  for (int i=0; i<_lstMultiImageDisplays.size(); i++)
    {
      if (_lstMultiImageDisplays[i] != 0)
        _lstMultiImageDisplays[i]->imageDestroyed(image);
    }
}

void PiiMultiImage::setCurrentIndex(int index){
  if(index<0 || index>_images.size())
    return;
  _current = index;
}

QImage* PiiMultiImage::currentImage() const {
  if(_images.size()==0) return 0;
  
  return _images[_current];
}

int PiiMultiImage::imageCount() const {return _images.size();}

int PiiMultiImage::currentIndex() const { return _current;}


int PiiMultiImage::indexOf(QImage* im) const {
  return _images.indexOf(im);
}

QImage* PiiMultiImage::image(int index) const{
  if(index<0 || index >= _images.size())
    return 0;
  return _images[index];
}

QList<QImage*> PiiMultiImage::images() const {
  return _images;
}


void PiiMultiImage::select(bool val){
  _selected = val;
}

bool PiiMultiImage::isSelected() const {
  return _selected;
}

void PiiMultiImage::nextImage(){
  _current++;
  if(_images.size()-1 < _current)
    _current = 0;
}


void PiiMultiImage::prevImage()
{
  _current--;
  if( _current < 0)
    _current = _images.size()-1;
}

void PiiMultiImage::addImage(QImage* im)
{
  _images.append(im);
  _current = _images.lastIndexOf(im);
}

void PiiMultiImage::removeImage(QImage* im)
{
  if (im == 0)
    {
      _images.removeAt(_current);
      _current = _images.size() - 1;
    }
  else if (_images.contains(im))
    {
      _images.removeAll(im);
      _current = _images.size() - 1;
    }

  if (_current < 0)
    _current = 0;

  notifyAllImageDisplays(im);
}

void PiiMultiImage::removeAt(int index)
{
  if (index >= 0 && index < _images.size())
    notifyAllImageDisplays(_images[index]);

  _images.removeAt(index);
  _current = _images.size()-1;
  if(_current < 0)
    _current = 0;
}

void PiiMultiImage::removeImages()
{
  _images.clear();
  _current = _images.size()-1;
  if(_current < 0)
    _current = 0;
  
  deleteAllImageDisplays();
}

bool PiiMultiImage::imagesContain(QPoint p)
{
  for (int j=_images.size(); j--; )
    {
      QPoint offset = _images.at(j)->offset();
      QRect rec(offset.x(),offset.y(),_images.at(j)->width(),_images.at(j)->height());
      if (rec.contains(p)) return true;
    }
  return false;
}

bool PiiMultiImage::imagesIntersect(QRect r)
{
  for (int j=_images.size(); j--; )
    {
      QPoint offset = _images.at(j)->offset();
      QRect rec(offset.x(),offset.y(),_images.at(j)->width(),_images.at(j)->height());
      if (rec.intersects(r)) return true;
    }
  return false;
}

bool PiiMultiImage::contains(QImage* image)
{
  return _images.contains(image);
}

bool PiiMultiImage::hasIconListWidget()
{
  return _pIconListWidget != 0;
}

void PiiMultiImage::updateIconListWidget(QWidget *pEditorWidget)
{
  _pIconListWidget->setImages(_images);
  _pIconListWidget->setEditorWidget(pEditorWidget);
}

void PiiMultiImage::showIconListWidget(QWidget *pEditorWidget, const QString& title)
{
  if(_images.size()==0)
    {
      delete pEditorWidget;
      return;
    }

  if (_pIconListWidget == 0)
    {
      _pIconListWidget = new PiiIconListWidget;

      QObject::connect(_pIconListWidget, SIGNAL(itemActivated(QListWidgetItem *)),
                       this, SLOT(showContentOfItem(QListWidgetItem *)));
      _pIconListWidget->setParent((QWidget*)parent(),Qt::Window);
      _pIconListWidget->setAttribute(Qt::WA_DeleteOnClose, true);
    }

  updateIconListWidget(pEditorWidget);
  _pIconListWidget->setWindowTitle(title);

  //show the widget on topmost
  _pIconListWidget->hide();
  _pIconListWidget->show();
}

void PiiMultiImage::showImageDisplay(QImage* im)
{
  if(im==0)
    im = currentImage();
  
  if(im!=0)
    {
      QPointer<PiiMultiImageDisplay> pmid = new PiiMultiImageDisplay((PiiMultiImage*)this,im);
      pmid->setPixelSize(_unitScale);
      pmid->setParent((QWidget*)parent(),Qt::Window);
      pmid->setAttribute(Qt::WA_DeleteOnClose, true);
      _lstMultiImageDisplays << pmid;
      pmid->show();
    }
}

void PiiMultiImage::showContentOfItem(QListWidgetItem *item)
{
  if ( item != 0)
    {
      QImage * im = 0;
      int index = item->data(Qt::UserRole).toInt();
      if (index < _images.size())
        im = _images.at(index);
      
      if (im!=0)
        showImageDisplay(im);
    }
}

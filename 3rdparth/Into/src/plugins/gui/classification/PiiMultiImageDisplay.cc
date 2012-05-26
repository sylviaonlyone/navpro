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


#include "PiiMultiImageDisplay.h"
#include "PiiImageViewport.h"
#include "PiiImageDisplay.h"
#include <QIcon>
#include <QToolButton>

PiiMultiImageDisplay::PiiMultiImageDisplay(PiiMultiImage* multiimage, QImage* im, QWidget* editorWidget) :
  _pCopyImage(0), _pBigImage(0), _pCurrentImage(0)
{
  _pMultiimage = multiimage;
  _pImageDisplay = new PiiImageDisplay;
  _pImageViewport = _pImageDisplay->imageViewport();
  
  if(im == 0)
    im = _pMultiimage->currentImage();
  _index = _pMultiimage->indexOf(im);

  _pImageDisplay->setMinimumSize(QSize(1,1));
  _pImageViewport->setFitMode(PiiImageViewport::FitToView);

  connect(_pImageViewport, SIGNAL(pageUpPressed()), SLOT(nextImage()));
  connect(_pImageViewport, SIGNAL(pageDownPressed()), SLOT(prevImage()));
  connect(_pImageViewport, SIGNAL(escPressed()), SLOT(close()));

  setWindowTitle( QString(QString("%1/%2 ").arg(_index+1)
        .arg(_pMultiimage->imageCount())+
        im->text("pii_filename")));

  _pImageDisplay->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  QHBoxLayout *pButtonLayout = new QHBoxLayout;

  QToolButton *pZoomIn = new QToolButton;
  QToolButton *pZoomOut = new QToolButton;
  QToolButton *pFitToView = new QToolButton;
  QToolButton *pActualSize = new QToolButton;

  pZoomIn->setText(tr("+"));
  pZoomOut->setText(tr("-"));
  pActualSize->setText(tr("1:1"));
  pFitToView->setIcon(QIcon(QPixmap(":/fitToView.png")));
  
  QToolButton *pPrevButton = new QToolButton;
  QToolButton *pNextButton = new QToolButton;
  QPushButton *pCloseButton = new QPushButton(tr("Close"));

  pPrevButton->setArrowType(Qt::LeftArrow);
  pNextButton->setArrowType(Qt::RightArrow);

  connect(pZoomIn, SIGNAL(clicked()), _pImageViewport, SLOT(zoomIn()));
  connect(pZoomOut, SIGNAL(clicked()), _pImageViewport, SLOT(zoomOut()));
  connect(pActualSize, SIGNAL(clicked()), _pImageViewport, SLOT(actualSize()));
  connect(pFitToView, SIGNAL(clicked()), _pImageViewport, SLOT(fitToView()));
  
  connect(pPrevButton, SIGNAL(clicked()), this, SLOT(prevImage()));
  connect(pNextButton, SIGNAL(clicked()), this, SLOT(nextImage()));
  connect(pCloseButton, SIGNAL(clicked()), this, SLOT(close()));

  pButtonLayout->addWidget(pZoomIn);
  pButtonLayout->addWidget(pZoomOut);
  pButtonLayout->addWidget(pActualSize);
  pButtonLayout->addWidget(pFitToView);
  pButtonLayout->addStretch(1);
  pButtonLayout->addWidget(pPrevButton);
  pButtonLayout->addWidget(pNextButton);
  pButtonLayout->addStretch(1);
  pButtonLayout->addWidget(pCloseButton);

  QVBoxLayout *pLayout = new QVBoxLayout;
  pLayout->addWidget(_pImageDisplay);
  pLayout->addLayout(pButtonLayout);

  setLayout(pLayout);

  _pImageViewport->setFocus();
  setImage(im);
  
  resize(500,500);
}

PiiMultiImageDisplay::~PiiMultiImageDisplay()
{
  delete _pCopyImage;
  delete _pBigImage;
}

void PiiMultiImageDisplay::imageDestroyed(const QImage *image)
{
  if (image != 0 && image == _pImageViewport->image())
    {
      delete _pCopyImage;
      _pCopyImage = new QImage(image->bits(), image->width(), image->height(), image->bytesPerLine(), image->format());
      if (image->format() == QImage::Format_Indexed8)
        _pCopyImage->setColorTable(image->colorTable());
      
      _pImageViewport->setImage(_pCopyImage);
    }
  else if (image == _pCurrentImage)
    _pCurrentImage = 0;
}

void PiiMultiImageDisplay::setPixelSize(const QSizeF& pixelSize)
{
  if ( _pImageViewport )
    _pImageViewport->setPixelSize(pixelSize);
}


void PiiMultiImageDisplay::nextImage()
{
  changeImage(Forward);
}

void PiiMultiImageDisplay::prevImage()
{
  changeImage(Backward);
}

void PiiMultiImageDisplay::changeImage(Direction direction)
{
  if(_pMultiimage->imageCount() == 0)
    return;
  
  QImage* im = 0;
    
  while(im == 0)
    {
      if (direction == Forward)
        _index++;
      else
        _index--;

      if(_index < 0 || _index >= _pMultiimage->imageCount())
        _index = direction == Forward ? 0 : _pMultiimage->imageCount()-1;

      im = _pMultiimage->image(_index);
      if(_pMultiimage->imageCount() == 0)
        break;
    }

  if(im != 0)
    { 
      setWindowTitle(QString(QString("%1/%2 ").arg(_index+1)
                             .arg(_pMultiimage->imageCount())+
                             im->text("pii_filename")));

      setImage(im);
      
      if (_pCopyImage)
        {
          delete _pCopyImage;
          _pCopyImage = 0;
        }
    }
}

QImage* PiiMultiImageDisplay::readFromFile(QImage *image)
{
  QString fileName = image->text("pii_filename");

  if (QFile::exists(fileName))
    return new QImage(fileName);

  return 0;
}

void PiiMultiImageDisplay::setImage(QImage *image)
{
  _pCurrentImage = image;
  if (image->text("pii_scaled") == "yes")
    {
      QImage *pImage = readFromFile(image);
      _pImageViewport->setImage(pImage);
      delete _pBigImage;
      _pBigImage = pImage;
    }
  else
    {
      _pImageViewport->setImage(_pCurrentImage);
      delete _pBigImage;
      _pBigImage = 0;
    }
}

void PiiMultiImageDisplay::keyPressEvent(QKeyEvent *e)
{
  if ( e->key() == Qt::Key_Escape )
    close();

  QWidget::keyPressEvent(e);
}


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

#include "PiiImageDisplay.h"
#include "PiiImageViewport.h"

#include <PiiYdinTypes.h>
#include <PiiMath.h>
#include <PiiQImage.h>
#include <PiiColor.h>

#include <QScrollBar>
#include <QtDebug>
#include <QSize>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPoint>

PiiImageDisplay::Data::Data() :
  pImageViewport(0),
  bCanMoveDisplay(true),
  bDragging(false),
  displayType(Normal)
{
  lstImages << 0;
}

PiiImageDisplay::Data::~Data()
{
  qDeleteAll(lstImages);
}

PiiImageDisplay::PiiImageDisplay(Data *dd, QImage* image, QWidget* parent) :
  QAbstractScrollArea(parent), d(dd)
{
  d->pImageViewport = new PiiImageViewport(image);
  initViewport();
}

PiiImageDisplay::PiiImageDisplay(Data *dd, const QImage& image, QWidget* parent) :
  QAbstractScrollArea(parent), d(dd)
{
  d->pImageViewport = new PiiImageViewport(image);
  initViewport();
}

PiiImageDisplay::PiiImageDisplay(Data *data, QWidget* parent) :
  QAbstractScrollArea(parent), d(data)
{
  d->pImageViewport = new PiiImageViewport;
  initViewport();
}

PiiImageDisplay::PiiImageDisplay(QImage* image, QWidget* parent) :
  QAbstractScrollArea(parent), d(new Data)
{
  d->pImageViewport = new PiiImageViewport(image);
  initViewport();
}

PiiImageDisplay::PiiImageDisplay(const QImage& image, QWidget* parent) :
  QAbstractScrollArea(parent), d(new Data)
{
  d->pImageViewport = new PiiImageViewport(image);
  initViewport();
}

PiiImageDisplay::PiiImageDisplay(QWidget* parent) :
  QAbstractScrollArea(parent), d(new Data)
{
  d->pImageViewport = new PiiImageViewport;
  initViewport();
}

PiiImageDisplay::~PiiImageDisplay()
{
  delete d;
}
void PiiImageDisplay::initViewport()
{
  d->bCanMoveDisplay = true;
  d->bDragging = false;
  
  d->pImageViewport->setParent(viewport());
  connect(d->pImageViewport, SIGNAL(visibleAreaChanged(int, int, int, int)), this, SLOT(visibleAreaChanged(int, int, int, int)));
  connect(d->pImageViewport, SIGNAL(areaSelected(const QRect&,int)), this, SIGNAL(areaSelected(const QRect&,int)));
  connect(d->pImageViewport, SIGNAL(clicked(const QPoint&,int)), this, SIGNAL(clicked(const QPoint&,int)));
}

PiiImageViewport* PiiImageDisplay::imageViewport() const
{
  return d->pImageViewport;
}

void PiiImageDisplay::mouseMoveEvent(QMouseEvent *event)
{
  QPoint newPoint = event->pos();
  
  // Middle button is for dragging the image
  if (d->bDragging && (event->buttons() & Qt::MidButton))
    {
      QPoint mousePrev = d->mouseCurrPoint;

      double zoomFactor = d->pImageViewport->zoom();
      int xmove = int(double(newPoint.x()-mousePrev.x())/zoomFactor);
      int ymove = int(double(newPoint.y()-mousePrev.y())/zoomFactor);

      // This if makes sure, that moving of the image works also when
      // the image has been zoomed out very much, a.
      if (xmove != 0 || ymove != 0)
        {
          horizontalScrollBar()->setValue(horizontalScrollBar()->value()-xmove);
          verticalScrollBar()->setValue(verticalScrollBar()->value()-ymove);
          d->mouseCurrPoint = newPoint;
        }
    }
}

void PiiImageDisplay::mousePressEvent(QMouseEvent *event)
{
  if (event->button() & Qt::MidButton)
    {
      d->mouseCurrPoint = event->pos();
      d->bDragging = true;
    }
}

void PiiImageDisplay::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() & Qt::MidButton)
    {
      d->bDragging = false;
    }
}

void PiiImageDisplay::resizeEvent (QResizeEvent* event)
{
  if (d->pImageViewport != 0)
    {
      // Disable painting temporarily, to avoid flickering.
      setUpdatesEnabled(false);
      d->pImageViewport->resize(viewport()->size());
      setUpdatesEnabled(true);
    }
  QAbstractScrollArea::resizeEvent(event);
}

void PiiImageDisplay::wheelEvent(QWheelEvent * event)
{
  if (0 == (Qt::ControlModifier & event->modifiers()))
    verticalScrollBar()->setValue(verticalScrollBar()->value() - event->delta());
}

void PiiImageDisplay::visibleAreaChanged(int x, int y, int width, int height)
{
  // It is assumed, that the image has already been moved to the
  // correct position in the image viewport. That's why we set the
  // value of this member variable temporary to false.
  d->bCanMoveDisplay = false;
  
  int xPageStep = qMin(width, d->pImageViewport->image()->width());
  int yPageStep = qMin(height, d->pImageViewport->image()->height());
  int xVal = qMax(0, x);
  int yVal = qMax(0, y);

  horizontalScrollBar()->setPageStep(xPageStep);
  verticalScrollBar()->setPageStep(yPageStep);

  horizontalScrollBar()->setRange(0, d->pImageViewport->image()->width() - xPageStep);
  verticalScrollBar()->setRange(0, d->pImageViewport->image()->height() - yPageStep);

  horizontalScrollBar()->setValue(xVal);
  verticalScrollBar()->setValue(yVal);
  
  d->bCanMoveDisplay = true;
}

void PiiImageDisplay::scrollContentsBy(int dx, int dy)
{
  if (d->bCanMoveDisplay)
    {
      // When x-scrollbar is moved right, the parameter dx will get
      // negative values. Also when the y-scrollbar is moved down, dy will
      // get negative values. That's why we must invert the values before
      // calling the corresponding functions of PiiImageViewport.
      if (dx != 0)
        d->pImageViewport->moveCurrX(-dx);
      if (dy != 0)
        d->pImageViewport->moveCurrY(-dy);
    }
}

void PiiImageDisplay::setImage(const PiiVariant& image, int layer)
{
  if (layer < 0 || layer > d->lstImages.size())
    return;

  // Ensure we have a pointer for each displayed layer.
  for (int i=d->lstImages.size(); i<d->pImageViewport->layerCount(); ++i)
    d->lstImages << 0;
  
  if (!image.isValid())
    {
      d->pImageViewport->setImage(0, layer);
      if (layer == 0)
        {
          delete d->lstImages[0];
          d->lstImages[0] = 0;
        }
      else
        delete d->lstImages.takeAt(layer);
    }
  else
    {
      switch (image.type())
        {
          PII_INTEGER_MATRIX_CASES_M(grayImage, (image, layer));
          PII_UNSIGNED_MATRIX_CASES_M(grayImage, (image, layer));
          PII_FLOAT_MATRIX_CASES_M(grayImage, (image, layer));
        case PiiYdin::UnsignedCharColorMatrixType:
          colorImage<PiiColor<unsigned char> >(image, layer);
          break;
        case PiiYdin::UnsignedCharColor4MatrixType:
          colorImage<PiiColor4<unsigned char> >(image, layer);
          break;
        case PiiYdin::BoolMatrixType: // booleans are treaded as unsigned chars
          grayImage<unsigned char>(image, layer);
          break;
        default:
          break;
        }
    }
}

template <class T> void PiiImageDisplay::scaledImage(const PiiMatrix<T>& image, int layer)
{
  PiiMatrix<unsigned char> result(PiiMatrix<unsigned char>::uninitialized(image.rows(), image.columns()));
  T minimum, maximum;
  Pii::minMax(image, &minimum, &maximum);
  if (minimum == maximum)
    {
      view(PiiMatrix<unsigned char>(image), layer);
      return;
    }
  float scale = 255.0f / (maximum - minimum);
  int iRows = image.rows(), iCols = image.columns();
  for (int r=0; r<iRows; ++r)
    {
      const T* sourceRow = image.row(r);
      unsigned char* resultRow = result.row(r);
      for (int c=0; c<iCols; ++c)
        resultRow[c] = (unsigned char)(scale * (sourceRow[c] - minimum));
    }
  view(result, layer);
}

template <class T> void PiiImageDisplay::grayImage(const PiiVariant& obj, int layer)
{
  if (d->displayType == AutoScale)
    scaledImage(obj.valueAs<PiiMatrix<T> >(), layer);
  else
    view(PiiMatrix<unsigned char>(obj.valueAs<PiiMatrix<T> >()), layer);
}

template <class T> void PiiImageDisplay::floatImage(const PiiVariant& obj, int layer)
{
  if (d->displayType == AutoScale)
    scaledImage<T>(obj, layer);
  else
    view(PiiMatrix<unsigned char>(obj.valueAs<PiiMatrix<T> >() * 255), layer);
}

template <class T> void PiiImageDisplay::colorImage(const PiiVariant& obj, int layer)
{
  view(PiiMatrix<PiiColor4<unsigned char> >(obj.valueAs<PiiMatrix<T> >()), layer);
}

template <class T> void PiiImageDisplay::view(const PiiMatrix<T>& image, int layer)
{
  if (!image.isEmpty())
    {
      QImage *pImage = Pii::createQImage(image);
      d->pImageViewport->setImage(pImage, layer);
      delete d->lstImages[layer];
      d->lstImages[layer] = pImage;
    }
}

void PiiImageDisplay::setDisplayType(DisplayType type) { d->displayType = type; }
PiiImageDisplay::DisplayType PiiImageDisplay::displayType() { return d->displayType; }

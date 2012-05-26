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

#include "PiiImageAnnotator.h"

#include <PiiYdinTypes.h>
#include <PiiSmartPtr.h>

#include <QPainter>

using namespace Pii;
using namespace PiiYdin;

PiiImageAnnotator::Data::Data() :
  annotationType(Auto),
  brush(Qt::NoBrush),
  pen(QColor(Qt::red)),
  bAnnotationConnected(false),
  bTypeConnected(false),
  bEnabled(true)
{
  pen.setCosmetic(true);
}

PiiImageAnnotator::PiiImageAnnotator() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  
  addSocket(d->pImageInput = new PiiInputSocket("image"));
  addSocket(d->pAnnotationInput = new PiiInputSocket("annotation"));
  d->pAnnotationInput->setOptional(true);
  addSocket(d->pTypeInput = new PiiInputSocket("type"));
  d->pTypeInput->setOptional(true);
  
  addSocket(d->pImageOutput = new PiiOutputSocket("image"));
}

PiiImageAnnotator::~PiiImageAnnotator()
{
}

PiiImageAnnotator::AnnotationType PiiImageAnnotator::annotationType() const { return _d()->annotationType; }
void PiiImageAnnotator::setAnnotationType(AnnotationType annotationType) { _d()->annotationType = annotationType; }

void PiiImageAnnotator::setFont(const QFont& font) { _d()->font = font; }
QFont PiiImageAnnotator::font() const { return _d()->font; }

QBrush PiiImageAnnotator::brush() const { return _d()->brush; }
void PiiImageAnnotator::setBrush(QBrush brush) { _d()->brush = brush; }

QPen PiiImageAnnotator::pen() const { return _d()->pen; }
void PiiImageAnnotator::setPen(QPen pen) { _d()->pen = pen; }

void PiiImageAnnotator::setTextPosition(const QPoint& textPosition) { _d()->textPosition = textPosition; }
QPoint PiiImageAnnotator::textPosition() const { return _d()->textPosition; }

void PiiImageAnnotator::setAnnotations(const QVariantList& annotations) { _d()->lstAnnotations = annotations; }
QVariantList PiiImageAnnotator::annotations() const { return _d()->lstAnnotations; }

void PiiImageAnnotator::setEnabled(bool enabled) { _d()->bEnabled = enabled; }
bool PiiImageAnnotator::enabled() const { return _d()->bEnabled; }

void PiiImageAnnotator::check(bool reset)
{
  PII_D;
  
  PiiDefaultOperation::check(reset);
  d->bAnnotationConnected = d->pAnnotationInput->isConnected();
  d->bTypeConnected = d->pTypeInput->isConnected();
  if (d->bTypeConnected && !d->bAnnotationConnected)
    PII_THROW(PiiExecutionException, tr("Type input cannot be connected if the annotation input is not connected."));
}
  
void PiiImageAnnotator::process()
{
  PII_D;
  
  PiiVariant obj = d->pImageInput->firstObject();

  if (!d->bEnabled)
    d->pImageOutput->emitObject(obj);
  else
    {
      switch (obj.type())
        {
        case UnsignedCharColorMatrixType:
          d->annotate<PiiColor<unsigned char> >(obj);
          break;        
        case UnsignedCharColor4MatrixType:
          d->annotate<PiiColor4<unsigned char> >(obj);
          break;
          PII_NUMERIC_MATRIX_CASES(d->annotate, obj);
        default:
          PII_THROW_UNKNOWN_TYPE(d->pImageInput);
        }
    }
}

template <class T> void PiiImageAnnotator::Data::annotate(const PiiVariant& obj)
{
  if (bAnnotationConnected || lstAnnotations.size() != 0)
    {
      const PiiMatrix<T> matrix = obj.valueAs<PiiMatrix<T> >();
      // Protect against exceptions
      PiiSmartPtr<PiiQImage<PiiColor4<unsigned char> > > qmatrix(PiiColorQImage::create(matrix));

      if (lstAnnotations.size() != 0)
        drawAnnotations(qmatrix, lstAnnotations);
      if (bAnnotationConnected)
        {
          PiiVariant pAnnotation = pAnnotationInput->firstObject();
          AnnotationType type = bTypeConnected ?
            static_cast<AnnotationType>(PiiYdin::primitiveAs<int>(pTypeInput)) :
            annotationType;

          if (type == Text)
            drawText(qmatrix, PiiYdin::convertToQString(pAnnotationInput));
          else
            {
              switch (pAnnotation.type())
                {
                  PII_NUMERIC_MATRIX_CASES_M(annotate, (qmatrix, pAnnotation, type));
                default:
                  PII_THROW_UNKNOWN_TYPE(pAnnotationInput);
                  break;
                }
            }
        }
      pImageOutput->emitObject(qmatrix.release()->toMatrix());
    }
  else
    pImageOutput->emitObject(obj);
}

void PiiImageAnnotator::Data::drawText(QImage* image, const QString& text)
{
  QPainter painter(image);
  painter.setBrush(Qt::NoBrush);
  painter.setFont(font);
  painter.setPen(pen);
  painter.drawText(textPosition.x(), textPosition.y(), text);
}


template <class T> struct DrawingTraits
{
  typedef QPoint Point;
  typedef QLine Line;
  typedef QRect Rect;
};

template <> struct DrawingTraits<float>
{
  typedef QPointF Point;
  typedef QLineF Line;
  typedef QRectF Rect;
};

template <> struct DrawingTraits<double> : DrawingTraits<float> {};

template <class T> void PiiImageAnnotator::Data::annotate(QImage* imageBuffer, const PiiVariant& annotation, AnnotationType type)
{
  PiiMatrix<T> matrix = annotation.valueAs<PiiMatrix<T> >();
  QPainter painter(imageBuffer);
  painter.setPen(pen);
  painter.setBrush(brush);

  const int iRows = matrix.rows();
  const int iColumns = matrix.columns();

  // Determine property type automatically based on the number of
  // columns in input.
  if (type == Auto)
    {
      switch (iColumns)
        {
        case 2: type = Point; break;
        case 3: type = Circle; break;
        case 4: type = Rectangle; break;
        }
    }

  switch (type)
    {
    case Point:
      {
        if (iColumns != 2)
          break;
        for ( int i=iRows; i--; )
          painter.drawPoint(typename DrawingTraits<T>::Point(matrix(i,0), matrix(i,1)));
        break;
      }
    case Line:
      {
        if (iColumns != 4)
          break;
        for ( int i=iRows; i--; )
          painter.drawLine(typename DrawingTraits<T>::Line(matrix(i,0), matrix(i,1), matrix(i,2), matrix(i,3)));
        break;
      }
    case Rectangle:
      {
        if (iColumns != 4)
          break;
        for ( int i=iRows; i--; )
          painter.drawRect(typename DrawingTraits<T>::Rect(matrix(i,0), matrix(i,1), matrix(i,2), matrix(i,3)));
        break;
      }
    case Ellipse:
      {
        if (iColumns != 4)
          break;
        for ( int i=iRows; i--; )
          painter.drawEllipse(typename DrawingTraits<T>::Rect(matrix(i,0), matrix(i,1), matrix(i,2), matrix(i,3)));
        break;
      }
    case Circle:
      {
        if (iColumns != 3)
          break;
        for ( int i=iRows; i--; )
          painter.drawEllipse(typename DrawingTraits<T>::Rect((matrix(i,0)-matrix(i,2)), (matrix(i,1)-matrix(i,2)), matrix(i,2)*2, matrix(i,2)*2));
        break;
      }
    case Auto:
    case Text:
    default:
      // These cases were already handled
      break;
    }
}

void PiiImageAnnotator::Data::drawAnnotations(QImage* image, const QVariantList& annotations)
{
  QPainter painter(image);
  for (int i=0; i<annotations.size(); ++i)
    {
      QVariantMap map = annotations[i].toMap();

      // All types must have x
      if (!map.contains("x"))
        continue;
      
      if (map.contains("pen"))
        painter.setPen(map["pen"].value<QPen>());
      else
        painter.setPen(Qt::red);

      if (map.contains("brush"))
        painter.setBrush(map["brush"].value<QBrush>());
      else
        painter.setBrush(Qt::NoBrush);
      
      if (map["x"].type() == QVariant::Double)
        drawAnnotation<float,double>(&painter, map);
      else
        drawAnnotation<int,int>(&painter, map);
    }
}

template <class T, class U> void PiiImageAnnotator::Data::drawAnnotation(QPainter* painter, const QVariantMap& annotation)
{
  AnnotationType type = static_cast<AnnotationType>(annotation["annotationType"].toInt());

  T x = annotation["x"].value<U>();
  T y = annotation["y"].value<U>();
  T width = annotation["width"].value<U>();
  T height = annotation["height"].value<U>();
  T radius = annotation["radius"].value<U>();
  T x2 = annotation["x2"].value<U>();
  T y2 = annotation["y2"].value<U>();
  QString text = annotation["text"].toString();
  
  switch (type)
    {
    case Text:      painter->drawText(x,y,text); break;
    case Line:      painter->drawLine(x,y,x2,y2); break;
    case Point:     painter->drawPoint(x,y); break;
    case Rectangle: painter->drawRect(x,y,width,height); break;
    case Ellipse:   painter->drawEllipse(x,y,width,height); break;
    case Circle:    painter->drawEllipse(x-radius, y-radius,radius*2, radius*2); break;
    case Auto:
    default:
      break;
    }
}

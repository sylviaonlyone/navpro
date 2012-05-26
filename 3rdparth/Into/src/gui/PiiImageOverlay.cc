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

#include "PiiImageOverlay.h"

/***** PiiImageOverlay *****/
PiiImageOverlay::Data::~Data()
{}

PiiImageOverlay::PiiImageOverlay()
{}

PiiImageOverlay::PiiImageOverlay(Data* data) : d(data)
{}

PiiImageOverlay::~PiiImageOverlay()
{
  delete d;
}

void PiiImageOverlay::setToolTipText(const QString& text) { d->strToolTipText = text; }
QString PiiImageOverlay::toolTipText() const { return d->strToolTipText; }
bool PiiImageOverlay::intersects(const QRect&) { return true; }
bool PiiImageOverlay::contains(const QPoint&) { return false; }


/***** PiiGeometricOverlay *****/
PiiGeometricOverlay::Data::Data(const QBrush& bg, const QPen& border) :
  brush(bg), pen(border)
{}

PiiGeometricOverlay::Data::Data() :
  brush(QColor(255,0,0,20))
{}

PiiGeometricOverlay::PiiGeometricOverlay() :
  PiiImageOverlay(new Data())
{}

PiiGeometricOverlay::PiiGeometricOverlay(const QBrush& bg, const QPen& border) :
  PiiImageOverlay(new Data(bg, border))
{}

PiiGeometricOverlay::PiiGeometricOverlay(Data* d) :
  PiiImageOverlay(d)
{}

PiiGeometricOverlay::~PiiGeometricOverlay()
{}

void PiiGeometricOverlay::setPen(const QPen& pen) { _d()->pen = pen; }
QPen PiiGeometricOverlay::pen() const { return _d()->pen; }

void PiiGeometricOverlay::setBrush(const QBrush& brush) { _d()->brush = brush; }
QBrush PiiGeometricOverlay::brush() const { return _d()->brush; }


/***** PiiRectangleOverlay *****/
PiiRectangleOverlay::Data::Data()
{}

PiiRectangleOverlay::Data::Data(const QRect& rect) :
  rectangle(rect)
{}

PiiRectangleOverlay::PiiRectangleOverlay() :
  PiiGeometricOverlay(new Data)
{}

PiiRectangleOverlay::PiiRectangleOverlay(const QRect& rectangle) :
  PiiGeometricOverlay(new Data(rectangle))
{}

PiiRectangleOverlay::~PiiRectangleOverlay()
{}

void PiiRectangleOverlay::paint(QPainter* p, bool filled)
{
  PII_D;
  p->setPen(d->pen);
  if (filled)
    p->setBrush(d->brush);
  p->drawRect(d->rectangle);
}

bool PiiRectangleOverlay::intersects(const QRect& r)
{
  return r.intersects(_d()->rectangle);
}

bool PiiRectangleOverlay::contains(const QPoint& p)
{
  return _d()->rectangle.contains(p);
}

void PiiRectangleOverlay::setRectangle(const QRect& rectangle) { _d()->rectangle = rectangle; }
QRect PiiRectangleOverlay::rectangle() const { return _d()->rectangle; }
QRect& PiiRectangleOverlay::rectangle() { return _d()->rectangle; }

/***** PiiPolygonOverlay *****/
PiiPolygonOverlay::Data::Data()
{}

PiiPolygonOverlay::Data::Data(const QPainterPath& s) :
  shape(s)
{}

PiiPolygonOverlay::PiiPolygonOverlay() :
  PiiGeometricOverlay(new Data)
{}

PiiPolygonOverlay::PiiPolygonOverlay(const QPainterPath& s) :
  PiiGeometricOverlay(new Data(s))
{}

PiiPolygonOverlay::~PiiPolygonOverlay()
{}

void PiiPolygonOverlay::setShape(const QPainterPath& shape) { _d()->shape = shape; }
QPainterPath PiiPolygonOverlay::shape() const { return _d()->shape; }

bool PiiPolygonOverlay::intersects(const QRect& r)
{
  return r.intersects(_d()->shape.boundingRect().toRect());
}

void PiiPolygonOverlay::paint(QPainter* p, bool filled)
{
  PII_D;
  p->setPen(d->pen);
  if(filled)
    p->setBrush(d->brush);
  p->drawPath(d->shape); 
}

bool PiiPolygonOverlay::contains(const QPoint& p)
{
  return _d()->shape.contains(p);
}


/***** PiiEllipseOverlay *****/
PiiEllipseOverlay::PiiEllipseOverlay()
{}

PiiEllipseOverlay::PiiEllipseOverlay(const QRect& rectangle) :
  PiiRectangleOverlay(rectangle)
{}

PiiEllipseOverlay::~PiiEllipseOverlay()
{}

void PiiEllipseOverlay::paint(QPainter* p, bool filled)
{
  PII_D;
  p->setPen(d->pen);
  if (filled)
    p->setBrush(d->brush);
  p->drawEllipse(d->rectangle);
}

bool PiiEllipseOverlay::intersects(const QRect& r)
{
  return r.intersects(_d()->rectangle);
}

bool PiiEllipseOverlay::contains(const QPoint& p)
{
  return _d()->rectangle.contains(p);
}


/***** PiiCrossOverlay *****/
PiiCrossOverlay::Data::Data()
{}

PiiCrossOverlay::Data::Data(const QPoint& p) :
  point(p)
{}

PiiCrossOverlay::PiiCrossOverlay() :
  PiiImageOverlay(new Data)
{}

PiiCrossOverlay::PiiCrossOverlay(const QPoint& point) :
  PiiImageOverlay(new Data(point))
{}

PiiCrossOverlay::~PiiCrossOverlay()
{}

void PiiCrossOverlay::setPen(const QPen& pen) { _d()->pen = pen; }
QPen PiiCrossOverlay::pen() const { return _d()->pen; }

void PiiCrossOverlay::paint(QPainter* p, bool /*filled*/)
{
  PII_D;
  p->setPen(d->pen);
  p->drawLine(d->point.x()-1, d->point.y()-1, d->point.x()+1, d->point.y()+1);
  p->drawLine(d->point.x()-1, d->point.y()+1, d->point.x()+1, d->point.y()-1);
  //p->drawPoint(_point);
}

bool PiiCrossOverlay::intersects(const QRect& r)
{
  return r.contains(_d()->point);
}

bool PiiCrossOverlay::contains(const QPoint& p)
{
  return _d()->point == p;
}

/***** PiiLineOverlay *****/
PiiLineOverlay::Data::Data()
{}

PiiLineOverlay::Data::Data(const QLine& l) :
  line(l)
{}

PiiLineOverlay::PiiLineOverlay() :
  PiiImageOverlay(new Data)
{}

PiiLineOverlay::PiiLineOverlay(const QLine& line) :
  PiiImageOverlay(new Data(line))
{}

void PiiLineOverlay::setPen(const QPen& pen) { _d()->pen = pen; }
QPen PiiLineOverlay::pen() const { return _d()->pen; }
QLine PiiLineOverlay::line() const { return _d()->line; }
void PiiLineOverlay::setLine(const QLine& line) { _d()->line = line; }

void PiiLineOverlay::paint(QPainter* p, bool /*filled*/)
{
  PII_D;
  p->setPen(d->pen);
  p->drawLine(d->line);
}

/***** PiiStringOverlay *****/
PiiStringOverlay::Data::Data() :
  bShowBorders(false),
  flags(Qt::AlignHCenter | Qt::AlignVCenter)
{}

PiiStringOverlay::Data::Data(const QRect& r, const QString& t) :
  rect(r),
  strText(t),
  bShowBorders(false),
  flags(Qt::AlignHCenter | Qt::AlignVCenter)
{}

PiiStringOverlay::PiiStringOverlay() :
  PiiImageOverlay(new Data)
{}

PiiStringOverlay::PiiStringOverlay(const QRect& rect, const QString& text) :
  PiiImageOverlay(new Data(rect, text))
{}

void PiiStringOverlay::setTextFlags(int flags) { _d()->flags = flags; }
void PiiStringOverlay::setRectangle(const QRect& rect) { _d()->rect = rect; }
void PiiStringOverlay::setText(const QString& text) { _d()->strText = text; }
void PiiStringOverlay::setFont(const QFont& font) { _d()->font = font; }
void PiiStringOverlay::setShowBorders(bool show) { _d()->bShowBorders = show; }
void PiiStringOverlay::setPen(const QPen& pen) { _d()->pen = pen; }
QPen PiiStringOverlay::pen() const { return _d()->pen; }

void PiiStringOverlay::paint(QPainter* p, bool /*filled*/)
{
  PII_D;
  p->setPen(d->pen);
  p->setFont(d->font);
  p->drawText(d->rect, d->flags, d->strText);

  if (d->bShowBorders)
    p->drawRect(d->rect);
}

bool PiiStringOverlay::intersects(const QRect& r)
{
  return r.intersects(_d()->rect);
}

bool PiiStringOverlay::contains(const QPoint& p)
{
  return _d()->rect.contains(p);
}

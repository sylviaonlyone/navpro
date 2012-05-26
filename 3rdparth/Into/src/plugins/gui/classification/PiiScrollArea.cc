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

#include "PiiScrollArea.h"

PiiScrollArea::PiiScrollArea(QWidget * parent,Qt::AspectRatioMode aspectMode):QScrollArea(parent){
  setAspectRatioMode(aspectMode);
}

void PiiScrollArea::setAspectRatioMode(Qt::AspectRatioMode aspectMode)
{
  _aspectMode = aspectMode;

  _fitToView = (_aspectMode == Qt::KeepAspectRatioByExpanding || _aspectMode == Qt::KeepAspectRatio);
  if ( _aspectMode != Qt::IgnoreAspectRatio )
    resizeWidget(size());
  update();
}

Qt::AspectRatioMode PiiScrollArea::aspectRatioMode() const
{
  return _aspectMode;
}

void PiiScrollArea::keyPressEvent(QKeyEvent *e)
{
  if ( e->key() == Qt::Key_PageUp )
    emit pageUpPressed();
  else if ( e->key() == Qt::Key_PageDown )
    emit pageDownPressed();
  else
    QScrollArea::keyPressEvent(e);
}

void PiiScrollArea::resizeEvent ( QResizeEvent * event )
{
  if (_fitToView)
    resizeWidget(event->size());
  QScrollArea::resizeEvent(event);
}

void PiiScrollArea::resizeWidget(QSize newSize)
{
  if (widget() == 0 ||
      newSize.width() == 0 ||
      newSize.height() == 0)
    return;
  
  QSize baseSize(newSize.width(),
                 widget()->heightForWidth(newSize.width()));
  baseSize.scale(newSize,_aspectMode);
    
  QSize m = maximumViewportSize();
  // NO scrollbars needed, no resizing, THiS PREVENTS THE oscillation
  if (_aspectMode != Qt::KeepAspectRatioByExpanding ||  m.expandedTo(baseSize) != m)
    widget()->resize(baseSize);
}

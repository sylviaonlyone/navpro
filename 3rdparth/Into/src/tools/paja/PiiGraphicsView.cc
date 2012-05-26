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

#include "PiiGraphicsView.h"

PiiGraphicsView::PiiGraphicsView(QWidget* parent) :
  QGraphicsView(parent)
{
  setTransformationAnchor(AnchorUnderMouse);
}

PiiGraphicsView::~PiiGraphicsView()
{
}


void PiiGraphicsView::wheelEvent(QWheelEvent* event)
{
  if(Qt::ControlModifier & event->modifiers())
    {
      int delta = event->delta();
      if ( delta > 0 )
        scale(1.1, 1.1);
      else
        scale(1.0/1.1, 1.0/1.1);
    }
  else
    QGraphicsView::wheelEvent(event);
}


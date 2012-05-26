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

#include "PushButton.h"
#include <QPainter>

PushButton::PushButton(QWidget *parent) :
  QPushButton(parent)
{
  setText("");
  
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _pixmapLeft = QPixmap(":left");
  _pixmapRight = QPixmap(":right");

  setIconMode(IconLeft);
}

void PushButton::paintEvent(QPaintEvent *event)
{
  QPushButton::paintEvent(event);
  
  QPainter p(this);
  int h = height();
  int w = width();

  int cap = 5;
  
  // Draw icon and text
  if (_mode == IconLeft)
    {
      p.drawPixmap(QRect(cap,cap,h-2*cap,h-2*cap), _pixmapLeft, _pixmapLeft.rect());
      p.drawText(QRect(0,0,w,h), Qt::AlignHCenter | Qt::AlignVCenter, _strName);
    }
  else
    {
      p.drawPixmap(QRect(w-h+cap,cap,h-2*cap,h-2*cap), _pixmapRight, _pixmapRight.rect());
      p.drawText(QRect(0,0,w,h), Qt::AlignHCenter | Qt::AlignVCenter, _strName);
    }
  p.end();
}

void PushButton::setIconMode(IconMode mode)
{
  setText("");
  
  _mode = mode;
  _strName = mode == IconLeft ? tr("Previous image") : tr("Next image");
  
  // Set minimum width
  QFontMetrics m(font());
  setMinimumWidth(m.width(_strName) + 2*height());

  repaint();
}
                            

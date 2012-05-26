/* This file is part of Into demo.
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
#include <QFontMetrics>

PushButton::PushButton(QWidget *parent) :
  QPushButton("", parent),
  _strName(""),
  _strDirname("")
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _pixmap = QPixmap(":/icons/folder.png");

  // Initialize height
  setFixedHeight(40);
}

void PushButton::paintEvent(QPaintEvent *event)
{
  QPushButton::paintEvent(event);
  
  QPainter p(this);
  int h = height();
  int w = width();

  // Draw icon and text
  p.drawPixmap(QRect(10,10,h-20,h-20), _pixmap, _pixmap.rect());
  p.drawText(QRect(h,0,w-h,h), Qt::AlignLeft | Qt::AlignVCenter, _strName);
  p.end();
}

void PushButton::setName(const QString& name)
{
  _strName = name;

  int iHeight = height();
  
  // Calculate font size
  QFont f = font();
  f.setPixelSize(iHeight/3*2);
  setFont(f);
  
  repaint();
}

void PushButton::setPixmap(const QPixmap& pixmap)
{
  _pixmap = pixmap;
  repaint();
}

void PushButton::setDirname(const QString& dirname)
{
  _strDirname = dirname;
}

QString PushButton::dirname() const
{
  return _strDirname;
}


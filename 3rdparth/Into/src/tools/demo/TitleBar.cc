/* This file is part of Into demo.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * Created: Wed Mar 17 2010 by Lasse Raiha
 *
 * $Revision:$
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

#include <TitleBar.h>

TitleBar::TitleBar(QWidget *parent)
{
  setupUi(this);
  
  // Use the style to set the button pixmaps
  _pCloseButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarCloseButton));
  _pMinimizeButton->setIcon(style()->standardPixmap(QStyle::SP_TitleBarMinButton));

  _pCloseButton->setForegroundRole(QPalette::NoRole);
  
  _maxPixmap = style()->standardPixmap(QStyle::SP_TitleBarMaxButton);
  _restorePixmap = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
  
  _pMaximizeButton->setIcon(_maxPixmap);

  _pTitleLabel->setText("Into demos");
  parent->setWindowTitle("Into demos");
        
  _bMaxNormal = false;
  
  connect(_pCloseButton, SIGNAL(clicked()), parent, SLOT(close()));
  connect(_pMinimizeButton, SIGNAL(clicked()), this, SLOT(showSmall()));
  connect(_pMaximizeButton, SIGNAL(clicked()), this, SLOT(showMaxRestore()));
}

void TitleBar::showSmall()
{
  parentWidget()->showMinimized();
}
    
void TitleBar::showMaxRestore()
{
  if (_bMaxNormal)
    {
      parentWidget()->showNormal();
      _bMaxNormal = !_bMaxNormal;
      _pMaximizeButton->setIcon(_maxPixmap);
    }
  else
    {
      parentWidget()->showMaximized();
      _bMaxNormal = !_bMaxNormal;
      _pMaximizeButton->setIcon(_restorePixmap);
    }
}

void TitleBar::mousePressEvent(QMouseEvent *me)
{
  _startPos = me->globalPos();
  _clickPos = mapToParent(me->pos());
}

void TitleBar::mouseMoveEvent(QMouseEvent *me)
{
  if (_bMaxNormal)
    return;
  parentWidget()->move(me->globalPos() - _clickPos);
}


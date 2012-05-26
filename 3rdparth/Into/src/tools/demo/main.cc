/* This file is part of Into demo.
 * Copyright (C) 2003-2011 Intopii.
 * All rights reserved.
 *
 * Created: Wed Mar 10 2010 by Lasse Raiha
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

#include <QApplication>
#include <Frame.h>
#include <QRect>
#include <QDesktopWidget>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  Frame box;
  
  //init view
  QRect screenRect = QApplication::desktop()->screenGeometry(QApplication::desktop()->primaryScreen());
  QRect windowRect(0, 0, 800, 600);
  if (screenRect.width() < 800)
    windowRect.setWidth(screenRect.width());
  if (screenRect.height() < 600)
    windowRect.setHeight(screenRect.height());
  windowRect.moveCenter(screenRect.center());
  box.setGeometry(windowRect);
  box.setMinimumSize(400, 300);
  box.setWindowTitle("Into demos");

  box.show();
  
  return app.exec();    
}


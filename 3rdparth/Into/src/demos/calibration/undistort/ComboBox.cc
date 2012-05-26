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

#include "ComboBox.h"
#include <QLineEdit>

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent)
{
}

void ComboBox::keyPressEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Return)
    {
      QString text = lineEdit()->text();
      if (findText(text) < 0)
        insertItem(0, text);
      setCurrentIndex(0);
    }

  QComboBox::keyPressEvent(event);
}


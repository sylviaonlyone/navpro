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

#ifndef _PIIICONLISTWIDGET_H
#define _PIIICONLISTWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QListWidget>
#include <QKeyEvent>

class PiiIconListWidget : public QWidget
{
  Q_OBJECT
  
public:
  PiiIconListWidget(QWidget *parent = 0, QWidget *editorWidget = 0);

  void setImages(const QList<QImage*>& lstImages);
  void setEditorWidget(QWidget *editorWidget);

protected:
  void keyPressEvent(QKeyEvent *e);

signals:
  void itemActivated(QListWidgetItem*);
  
private:
  QHBoxLayout *_pBottomLayout;
  QWidget *_pEditorWidget;
  QListWidget *_pView;
};


#endif //_PIIICONLISTWIDGET_H

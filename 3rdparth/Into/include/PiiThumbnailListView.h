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

#ifndef _PIITHUMBNAILLISTVIEW_H
#define _PIITHUMBNAILLISTVIEW_H

#include <QListView>
#include <QPoint>
#include <QMouseEvent>
#include "PiiImageListModel.h"

class PII_GUI_EXPORT PiiThumbnailListView : public QListView
{
  Q_OBJECT
  
public:
  PiiThumbnailListView(QWidget *parent = 0);
  ~PiiThumbnailListView();
  
  /**
   * Sets the model for the view present.
   */
  void setModel(PiiImageListModel *model);

  /**
   * Return the current selected fileName of the thumbnail. If there
   * is no any item selected, return empty string.
   */
  QString currentThumbnail() const;
  
signals:
  void removeThumbnail(const QModelIndex&);
  void thumbnailActivated(const QString&);
  
protected:
  void mousePressEvent(QMouseEvent *e);

private slots:
  void removeCurrent();
  void itemSelected(const QModelIndex& index);
  
private:
  void showMenu(const QPoint& point);

  class Data
  {
  public:
    PiiImageListModel *pModel;
  } *d;
};

#endif //_PIITHUMBNAILLISTVIEW_H

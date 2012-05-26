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

#include "PiiThumbnailListView.h"
#include <QMenu>

#include <QtDebug>

PiiThumbnailListView::PiiThumbnailListView(QWidget *parent) :
  QListView(parent),
  d(new Data)
{
  setGridSize(QSize(100,120));
  setIconSize(QSize(70,90));

  //setWrapping(true);
  setResizeMode(QListView::Adjust);
  setViewMode(QListView::IconMode);
  setFlow(QListView::TopToBottom);

  connect(this, SIGNAL(activated(const QModelIndex&)), this, SLOT(itemSelected(const QModelIndex&)));
  connect(this, SIGNAL(clicked(const QModelIndex&)), this, SLOT(itemSelected(const QModelIndex&)));
}

PiiThumbnailListView::~PiiThumbnailListView()
{
  delete d;
}

void PiiThumbnailListView::setModel(PiiImageListModel *model)
{
  d->pModel = model;
  QListView::setModel(model);
}

void PiiThumbnailListView::mousePressEvent(QMouseEvent *e)
{
  QListView::mousePressEvent(e);
  if ( e->button() == Qt::RightButton )
    showMenu(e->globalPos());
}

void PiiThumbnailListView::showMenu(const QPoint& point)
{
  QModelIndex ci = currentIndex();
  if (ci.isValid())
    {
      
      QMenu *menu = new QMenu;
      menu->addAction(tr("Remove this thumbnail"), this, SLOT(removeCurrent()));
      menu->addSeparator();
      menu->addAction(tr("Clear"), d->pModel, SLOT(clear()));
      menu->exec(point);
      delete menu;
    }
}

void PiiThumbnailListView::removeCurrent()
{
  QModelIndex ci = currentIndex();
  if (ci.isValid())
    {
      d->pModel->removeRow(ci.row());
      int rowCount = d->pModel->rowCount(QModelIndex());
      QModelIndex ind = ci.row() < rowCount ? ci : d->pModel->index(rowCount-1,0);
      setCurrentIndex(ind);
      itemSelected(ind);
    }
}

void PiiThumbnailListView::itemSelected(const QModelIndex& index)
{
  if (index.isValid())
    {
      QMap<int, QVariant> itemData = d->pModel->itemData(index);
      emit thumbnailActivated(itemData[Qt::UserRole].toMap()["fileName"].toString());
    }
  else
    emit thumbnailActivated("");
}

QString PiiThumbnailListView::currentThumbnail() const
{
  QModelIndex ci = currentIndex();
  if (ci.isValid())
    {
      QMap<int, QVariant> itemData = d->pModel->itemData(ci);
      return itemData[Qt::UserRole].toMap()["fileName"].toString();
    }
  else
    return "";
}


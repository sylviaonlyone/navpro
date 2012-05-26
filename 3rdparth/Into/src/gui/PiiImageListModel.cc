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

#include "PiiImageListModel.h"
#include <QCoreApplication>
#include <QtDebug>

PiiImageListModel::PiiImageListModel(QObject *parent) :
  QAbstractListModel(parent),
  d(new Data)
{
  connect(&d->thumbnailLoader, SIGNAL(thumbnailReady(const QString&, const QImage&)),
          this, SLOT(updateThumbnail(const QString&, const QImage&)));
}

PiiImageListModel::~PiiImageListModel()
{
  d->thumbnailLoader.stopLoading();
  d->thumbnailLoader.wait();
  delete d;
}

int PiiImageListModel::rowCount(const QModelIndex& /*parent*/) const
{
  return d->lstItems.count(); 
}

QVariant PiiImageListModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() >= d->lstItems.size() || index.row() < 0)
    return QVariant();

  switch (role)
    {
    case Qt::DisplayRole: return d->lstItems[index.row()].displayName();
    case Qt::ToolTipRole: return d->lstItems[index.row()].tooltipText();
    case Qt::DecorationRole: return d->lstItems[index.row()].icon();
    case Qt::BackgroundRole: return d->lstItems[index.row()].background();
    default: return d->lstItems[index.row()].data(role);
    }
}

void PiiImageListModel::setData(const QModelIndex& index, int role, const QVariant& data)
{
  if (!index.isValid() || index.row() >= d->lstItems.size() || index.row() < 0)
    return;

  switch (role)
    {
    case Qt::DisplayRole: return d->lstItems[index.row()].setDisplayName(data.toString());
    case Qt::ToolTipRole: return d->lstItems[index.row()].setTooltipText(data.toString());
    case Qt::DecorationRole: return d->lstItems[index.row()].setIcon(data.value<QIcon>());
    case Qt::BackgroundRole: return d->lstItems[index.row()].setBackground(data.value<QBrush>());
    default: d->lstItems[index.row()].setData(role, data);
    }
}

void PiiImageListModel::setData(const QString& fileName, int role, const QVariant& data)
{
  setData(index(fileName), role, data);
}

QMap<int, QVariant> PiiImageListModel::itemData(const QModelIndex& index) const
{
  QMap<int, QVariant> data = QAbstractItemModel::itemData(index);

  int row = index.row();
  if (row >= 0 && row < d->lstItems.count())
    data[Qt::UserRole] = d->lstItems[row].dataMap();

  return data;
}

bool PiiImageListModel::insertRows(int /*row*/, int /*count*/, const QModelIndex& /*parent*/)
{
  return false;
}

bool PiiImageListModel::removeRows(int row, int count, const QModelIndex& parent)
{
  if (row >= 0 && (row+count-1) < d->lstItems.count())
    {
      beginRemoveRows(parent, row, row+count-1);
      
      for (int i=0; i<count; i++)
        d->lstItems.removeAt(row);

      endRemoveRows();

      return true;
    }

  return false;
}

void PiiImageListModel::clear()
{
  d->lstItems.clear();
  emit layoutChanged();
}

void PiiImageListModel::setFileNames(const QStringList& fileNames)
{
  clear();

  if (fileNames.size() > 0)
    {
      d->lstItems.reserve(fileNames.size());
      beginInsertRows(QModelIndex(), 0, fileNames.size()-1);

      for (int i=0; i<fileNames.size(); i++)
        d->lstItems << PiiImageListModelItem(fileNames[i]);

      endInsertRows();
    }

  emit layoutChanged();
  
  for (int i=0; i<fileNames.size(); i++)
    d->thumbnailLoader.addFileName(fileNames[i]);
}

QStringList PiiImageListModel::fileNames() const
{
  QStringList lstResult;
  for (int i=0; i<d->lstItems.size(); ++i)
    lstResult << d->lstItems[i].fileName();
  return lstResult;
}

QVariantList PiiImageListModel::dataList(int role) const
{
  QVariantList lstResult;
  for (int i=0; i<d->lstItems.size(); ++i)
    lstResult << d->lstItems[i].data(role);
  return lstResult;
}

void PiiImageListModel::setItemList(const QList<PiiImageListModelItem>& itemList)
{
  d->lstItems = itemList;
  emit layoutChanged();
}

void PiiImageListModel::appendItem(const QString& fileName)
{
  beginInsertRows(QModelIndex(), d->lstItems.count(), d->lstItems.count());

  PiiImageListModelItem item(fileName);
  d->lstItems << item;
  
  endInsertRows();

  d->thumbnailLoader.addFileName(fileName);
}

void PiiImageListModel::updateThumbnail(const QString& fileName, const QImage& image)
{
  for (int i=0; i<d->lstItems.size(); i++)
    if (d->lstItems[i].fileName() == fileName)
      {
        d->lstItems[i].setIcon(QIcon(QPixmap::fromImage(image)));
        QModelIndex index = createIndex(i,0);
        emit layoutChanged();
      }
}

QModelIndex PiiImageListModel::index(const QString& fileName) const
{
  for (int i=0; i<d->lstItems.size(); i++)
    if (d->lstItems[i].fileName() == fileName)
      return QAbstractListModel::index(i);
  return QModelIndex();
}

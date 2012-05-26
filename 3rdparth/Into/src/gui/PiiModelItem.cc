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

#include "PiiModelItem.h"

#include <QVariant>

PiiModelItem::Data::Data() :
  _flags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled)
{
}

PiiModelItem::Data::~Data()
{
}

PiiModelItem::PiiModelItem(const QString& text) :
  d(new Data)
{
  setText(text);
}

PiiModelItem::PiiModelItem(Data* data, const QString& text) :
  d(data)
{
  setText(text);
}

PiiModelItem::~PiiModelItem()
{
  delete d;
}

void PiiModelItem::setText(const QString& text)
{
  d->_mapData[Qt::DisplayRole] = text;
}

QString PiiModelItem::text() const
{
  return d->_mapData[Qt::DisplayRole].toString();
}

void PiiModelItem::setIcon(const QIcon& icon)
{
  d->_mapData[Qt::DecorationRole] = icon;
}

QIcon PiiModelItem::icon() const
{
  return d->_mapData[Qt::DecorationRole].value<QIcon>();
}

void PiiModelItem::setFlags(const Qt::ItemFlags& flags)
{
  d->_flags = flags;
}

Qt::ItemFlags PiiModelItem::flags() const
{
  return d->_flags;
}

void PiiModelItem::setData(int role, const QVariant& value)
{
  d->_mapData[role] = value;
}

QVariant PiiModelItem::data(int role) const
{
  return d->_mapData[role];
}

QMap<int,QVariant> PiiModelItem::dataMap() const
{
  return d->_mapData;
}

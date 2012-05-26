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

#include "PiiTableModelDelegate.h"

#include "PiiTableModel.h"

PiiTableModelDelegate::PiiTableModelDelegate(PiiTableModel *parent) : QItemDelegate(parent)
{
}

QWidget *PiiTableModelDelegate::createEditor(QWidget *parentWidget,
                                           const QStyleOptionViewItem& /* option */,
                                           const QModelIndex &index) const
{
  const PiiTableModel* pModel = tableModel();
  return pModel->createEditor(parentWidget, index.row(), index.column());
}

void PiiTableModelDelegate::setEditorData(QWidget *editor,
                                          const QModelIndex &index) const
{
  const PiiTableModel* pModel = tableModel();
  pModel->setEditorData(editor, index.row(), index.column());
}

void PiiTableModelDelegate::setModelData(QWidget* editor,
                                         QAbstractItemModel* /*model*/,
                                         const QModelIndex &index) const
{
  PiiTableModel* pModel = tableModel();
  pModel->setModelData(editor, index.row(), index.column());
}


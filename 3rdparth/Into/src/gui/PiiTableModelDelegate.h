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

#ifndef _PIITABLEMODELDELEGATE_H
#define _PIITABLEMODELDELEGATE_H

#include <QItemDelegate>

#include "PiiTableModel.h"

/**
 * An implementation of the QItemDelegate interface that just passes
 * calls to a PiiTableModel.
 *
 * @internal
 *
 * @ingroup Gui
 */
class PiiTableModelDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  /**
   * Create a new %PiiTableModelDelegate and attach to the given
   * model.
   */
  PiiTableModelDelegate(PiiTableModel *parent);

  QWidget *createEditor(QWidget *parentWidget,
                        const QStyleOptionViewItem& option,
                        const QModelIndex &index) const;

  void setEditorData(QWidget *editor,
                     const QModelIndex &index) const;

  void setModelData(QWidget *editor,
                    QAbstractItemModel *model,
                    const QModelIndex &index) const;

private:
  PiiTableModel* tableModel() const { return static_cast<PiiTableModel*>(parent()); }
};

#endif //_PIITABLEMODELDELEGATE_H

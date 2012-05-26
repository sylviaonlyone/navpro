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

#ifndef _PIIMODELITEM_H
#define _PIIMODELITEM_H

#include "PiiGui.h"

#include <QObject>
#include <QMap>
#include <QIcon>

/**
 * A class used by PiiTableModel to store item data for table cells
 * and the header. %PiiModelItem uses a map of variants to store data
 * associated with different roles. See @ref Qt::ItemDataRole for
 * roles supported by most Qt view classes.
 *
 * @ingroup Gui
 */
class PII_GUI_EXPORT PiiModelItem : public QObject
{
  Q_OBJECT

  /**
   * The text of the item. Stored as @p Qt::DisplayRole data into the
   * data map.
   */
  Q_PROPERTY(QString text READ text WRITE setText);
  /**
   * The icon of the item. Stored as @p Qt::DecorationRole data into
   * the data map.
   */
  Q_PROPERTY(QIcon icon READ icon WRITE setIcon);
  /**
   * Item flags. The default value is <tt>Qt::ItemIsSelectable |
   * Qt::ItemIsEditable | Qt::ItemIsEnabled</tt>.
   */
  Q_PROPERTY(Qt::ItemFlags flags READ flags WRITE setFlags);

public:
  /**
   * Create a new model item with the given user-visible text.
   */
  PiiModelItem(const QString& text = "");
  /**
   * Destroy the model item.
   */
  ~PiiModelItem();

  /**
   * Set data associated with @p role to @p value.
   *
   * @code
   * // Show "10 %" to the user ...
   * PiiModelItem* pItem = new PiiModelItem("10 %");
   * // ... but store the percentage as a number
   * pItem->setData(Qt::UserRole, 10);
   * @endcode
   */
  void setData(int role, const QVariant& value);
  /**
   * Get data associated with @p role.
   */
  QVariant data(int role) const;

  /**
   * Get the full data map. The data map stores data associated with
   * all roles.
   */
  QMap<int,QVariant> dataMap() const;

  /**
   * A utility function for setting the data associated with the @p
   * Qt::DisplayRole.
   */
  void setText(const QString& text);
  /**
   * A utility function for getting the data associated with the @p
   * Qt::DisplayRole.
   */
  QString text() const;
  /**
   * A utility function for setting the data associated with the @p
   * Qt::DecorationRole.
   */
  void setIcon(const QIcon& icon);
  /**
   * A utility function for getting the data associated with the @p
   * Qt::DecorationRole.
   */
  QIcon icon() const;
  /**
   * Set the item's flags.
   */
  void setFlags(const Qt::ItemFlags& flags);
  /**
   * Get the item's flags.
   */
  Qt::ItemFlags flags() const;

protected:
  /// @internal
  struct Data
  {
    Data();
    virtual ~Data();
    Qt::ItemFlags _flags;
    QMap<int,QVariant> _mapData;
  } *d;

  /// @internal
  PiiModelItem(Data* data, const QString& text = "");
};

#endif //_PIIMODELITEM_H

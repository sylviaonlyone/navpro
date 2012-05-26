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

#ifndef _PIIITEM_H
#define _PIIITEM_H

#include <QAbstractGraphicsShapeItem>
#include <QFont>
#include <QObject>

class PiiItem : public QObject, public QAbstractGraphicsShapeItem
{
  Q_OBJECT
  
public:
  PiiItem(QGraphicsItem *parent);

  /**
   * A type of the item. 
   */
  enum Type { OperationItem = UserType + 1,
              OutputSocketItem,
              InputSocketItem,
              ProxySocketItem,
              LineItem };

  /**
   * Get a font.
   */
  QFont font() const;

  /**
   * Set a font.
   */
  void setFont(const QFont& font);
  
private:
  QFont _font;
};

#endif //_PIIITEM_H

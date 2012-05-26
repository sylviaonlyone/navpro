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

#ifndef _PIIIMAGELISTMODELITEM_H
#define _PIIIMAGELISTMODELITEM_H

#include <QIcon>
#include <QBrush>
#include <QVariantMap>
#include "PiiGui.h"

class PII_GUI_EXPORT PiiImageListModelItem
{
public:
  PiiImageListModelItem();
  PiiImageListModelItem(const QString& fileName);
  PiiImageListModelItem(const PiiImageListModelItem& other);
  ~PiiImageListModelItem();

  PiiImageListModelItem& operator= (const PiiImageListModelItem& other);
  
  QVariantMap dataMap() const;
  QString fileName() const;
  QString displayName() const;
  QString tooltipText() const;
  QIcon icon() const;
  QBrush background() const;

  void setDisplayName(const QString& displayName);
  void setTooltipText(const QString& text);
  void setIcon(const QIcon& icon);
  void setBackground(const QBrush& background);

  QVariant data(int role) const;
  void setData(int role, const QVariant& data);

private:
  class Data : public PiiSharedD<Data>
  {
  public:
    Data() {}
    Data(const QString& fileName);
    
    QString strDisplayName, strTooltipText, strFileName;
    QIcon icon;
    QBrush backgroundBrush;
    QList<QPair<int,QVariant> > lstData;
    
    static Data* sharedNull();
  } *d;

  int indexOf(int role) const;
};

#endif //_PIIIMAGELISTMODELITEM_H

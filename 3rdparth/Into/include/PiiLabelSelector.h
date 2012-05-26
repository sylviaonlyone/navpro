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


#ifndef _PIILABELSELECTOR_H
#define _PIILABELSELECTOR_H

#include <QtGui>
#include "PiiLabel.h"

class PiiLabelSelector : public QWidget 
{
  
  Q_OBJECT
    
public:
  
  /**
   * Parameter is a label list that this editor Changes
   **/
  
  PiiLabelSelector( QList<PiiLabel*>* _labels);
  
public slots:
  void applyChages();
  void currentChanged(QListWidgetItem * item,QListWidgetItem * previous);
 
protected:
  QList<PiiLabel*>* labels;
  
  QLineEdit *namevalue;
  QPushButton *colorvalue;
  
  QListWidget *list;
  
signals:
  
};

#endif //_PIILABELSELECTOR_H

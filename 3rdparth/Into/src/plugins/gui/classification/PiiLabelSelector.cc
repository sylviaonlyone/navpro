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

#include "PiiLabelSelector.h"

PiiLabelSelector::PiiLabelSelector( QList<PiiLabel*>* _labels){
   labels = _labels;
   
   QGridLayout *gridlayout = new QGridLayout( this );
   
   list = new QListWidget();
   gridlayout->addWidget(list,0,0);
   
   QGroupBox* group = new QGroupBox("Properties");
   
   QGridLayout *grid2 = new QGridLayout( group );
   for(int i=0;i<labels->size();i++){
	  new QListWidgetItem( labels->at(i)->name, list);
   }
   
   QObject::connect(list, SIGNAL(currentChanged(QListWidgetItem*,
						QListWidgetItem*)),
		    this, SLOT(currentChanged(QListWidgetItem*,
					      QListWidgetItem*)));
   
   QPixmap map(20,20);
   map.fill(labels->at(0)->color);
   
   namevalue = new QLineEdit(labels->at(0)->name);
   colorvalue = new QPushButton(QIcon(map),QString(""));
   
   grid2->addWidget(new QLabel("Name"),0,0);
   grid2->addWidget(namevalue,0,1);
   grid2->addWidget(new QLabel("Color"),1,0);
   grid2->addWidget(colorvalue,1,1);
   
   gridlayout->addWidget(group,0,1);
}

void PiiLabelSelector::applyChages(){   
}

void PiiLabelSelector::currentChanged(QListWidgetItem * item,
				      QListWidgetItem * /*previous*/){
   printf("recv currentchange\n");
   int row = list->row(item);
   if(row<0) return;
   printf("PiiLabelSelector Item sel %i\n",row);
   
   QColor c = labels->at(row)->color;
   QPixmap map(20,20);
   map.fill(c);
   colorvalue->setIcon(QIcon(map));
   namevalue->setText(item->text());
}

//PiiLabelSelector::

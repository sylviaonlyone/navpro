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

#ifndef PIILABEL
#define PIILABEL

#include <QString>
#include <QColor>
#include <stdlib.h> // rand()

class PiiLabel : public QObject {
   
  public:
   QString name;
   QColor  color;
   
   PiiLabel(QString _name, QColor _color){
	  name = _name;
	  color = _color;
   }
   
   PiiLabel(){
	  name = QString("undefined");
	  color = QColor( rand()&255, rand()&255, rand()&255 );
   }

    PiiLabel(QString _name){
	   name = _name;
	  color = QColor( rand()&255, rand()&255, rand()&255 );
   }
   
   QColor transparentColor(int transparency){
	  if(transparency > 256)transparency = 256;
	  if(transparency <0)transparency = 0;
	  return QColor(color.red(),color.green(),color.blue(),256 - transparency);
   }
};

#endif


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

#ifndef _PIIFLOATLAYERINFO_H
#define _PIIFLOATLAYERINFO_H

#include <QtGui>
#include "PiiSelectorLayerInfo.h"

/**
 * Class for float layer info.
 **/
class PiiFloatLayerInfo : public PiiSelectorLayerInfo
{

  Q_OBJECT
public:
  PiiFloatLayerInfo(QColor color1=Qt::red);
  
  PiiFloatLayerInfo(QColor color1, QColor color2);

  QWidget* editorWidget(int pointIndex, double current) const;
  QWidget* editor() const;
  QMenu* menu() const;

  /**
   * Returns 30x30 pixel size icon that represents the info type
   * NOTE: changes on color or other properties does NOT change the apperance of 
   * the icon directly. Therefore the icon must be regenerate.
   * TODO: Check if it is possible...
   **/
  QIcon icon() const ;
  
  /**
   * As all cell values are floats, this method simply calls brush(float)
   * and convert given int to float.
   **/
  QBrush brush(int value) const;
  
  /**
   * Returns a brush for painting assosiated to given value.
   * for minimum the first color is returned, for maximum second color is returned
   **/
  QBrush brush(double value) const;
  
  /**
   * Returns a pen (for borders and shapes) for painting assosiated to given value.
   * Gives out a pen with layer color and width 2.
   * NOTE! sets colors alpha to 1 so the pen color is not transparent.
   **/
  QPen pen(double value) const;
  
  void setEditorValue(double value);
  void showMenuWidget(double value);

public slots:
  void unselectEditor();
  
private slots:
  void assignEditorValue(double value);
  void menuActionPerformed();

protected:
  QColor _color1, _color2;

private:
  /**
   * Calculates gradient color from info's colors according the given value.
   **/
  QColor calculateGradientColor(double value) const;

};


#endif //_PIIFLOATLAYERINFO_H

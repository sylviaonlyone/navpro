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

#include "PiiSelectorLayer.h"
#include "PiiFloatLayerInfo.h"

PiiSelectorLayer::PiiSelectorLayer()
{
  _pInfo = new PiiFloatLayerInfo();
  _bEnabled = false;
}

PiiSelectorLayer::~PiiSelectorLayer(){
  delete _pInfo;
}

void PiiSelectorLayer::setInfo(PiiSelectorLayerInfo* info)
{
  delete _pInfo;
  _pInfo = info;
}

void PiiSelectorLayer::increaseValueAt(int x, int y)
{
  //qDebug("*********************");
  //qDebug("increaseValueAt(%i,%i)", x, y);
  //qDebug("*********************");
  setValueAt(x,y, valueAt(x,y) + _pInfo->resolution());
}
void PiiSelectorLayer::decreaseValueAt(int x, int y)
{
  //qDebug("*********************");
  //qDebug("decreaseValueAt(%i,%i)", x, y);
  //qDebug("*********************");
  setValueAt(x,y, valueAt(x,y) - _pInfo->resolution());
}

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

#include "PiiEditorWidgetCombo.h"

PiiEditorWidgetCombo::PiiEditorWidgetCombo(PiiComboBox* cb, int layer, int pointIndex) : PiiEditorWidget(layer,pointIndex)
{
  QVBoxLayout *mainlayout = new QVBoxLayout(this);
  mainlayout->setMargin(0);
  mainlayout->addWidget(cb);
  
  QObject::connect(cb, SIGNAL(activated(int)), SLOT(assignEditorValue(int)));
}

void PiiEditorWidgetCombo::assignEditorValue(int value)
{
  emit valueChanged((float)value, _iLayer, _iPointIndex);
}



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

#ifndef SELECTORLAYER_H
#define SELECTORLAYER_H

#include <QObject>
#include <QtGui>
/**
 * Simple class to expands the lacks of QCombobox class
 * that is: slots for setting current value.
 */
class PiiComboBox : public QComboBox
{
 Q_OBJECT

public:
 PiiComboBox();
 
public slots:
 
 void setCurrentValue(int val);
 void setCurrentValue(double val);
 void unselected();
};

#endif

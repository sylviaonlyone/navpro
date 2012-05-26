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

#include "PiiTriggerButton.h"
#include <QtGui>

PiiTriggerButton::Data::Data(PiiTriggerButton::Buttons b) :
  buttons(b)
{
}


PiiTriggerButton::PiiTriggerButton(Buttons buttons) :
  d(new Data(buttons))
{
  
  QHBoxLayout* pLayout = new QHBoxLayout(this);
  d->pLeft = new QToolButton;
  d->pLeft->setArrowType(Qt::LeftArrow);
  pLayout->addWidget(d->pLeft);
  d->pRight = new QToolButton;
  d->pRight->setArrowType(Qt::RightArrow);
  pLayout->addWidget(d->pRight);
  setLayout(pLayout);

  QSignalMapper* pMapper = new QSignalMapper(this);
  pMapper->setMapping(d->pLeft, -1);
  pMapper->setMapping(d->pRight, 1);
  connect(d->pLeft, SIGNAL(clicked()), pMapper, SLOT(map()));
  connect(d->pRight, SIGNAL(clicked()), pMapper, SLOT(map()));
  connect(pMapper, SIGNAL(mapped(int)), this, SIGNAL(triggered(int)));

  setButtons(buttons);
}

PiiTriggerButton::~PiiTriggerButton()
{
  delete d;
}

void PiiTriggerButton::setButtons(Buttons buttons)
{
  d->buttons = buttons;
  d->pLeft->setVisible(buttons & Left);
  d->pRight->setVisible(buttons & Right);
}

PiiTriggerButton::Buttons PiiTriggerButton::buttons() const
{
  return d->buttons;
}

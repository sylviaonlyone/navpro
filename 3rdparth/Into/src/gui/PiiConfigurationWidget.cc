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

#include "PiiConfigurationWidget.h"

PiiConfigurationWidget::Data::Data() :
  bAcceptable(true),
  bChanged(false)
{
}

PiiConfigurationWidget::PiiConfigurationWidget(QWidget* parent) :
  QWidget(parent),
  d(new Data)
{}

PiiConfigurationWidget::~PiiConfigurationWidget()
{
  delete d;
}


bool PiiConfigurationWidget::canAccept() const
{
  return d->bAcceptable;
}

void PiiConfigurationWidget::acceptChanges()
{ }

bool PiiConfigurationWidget::canReset() const
{
  return false;
}

void PiiConfigurationWidget::reset()
{ }

bool PiiConfigurationWidget::hasDefaults() const
{
  return false;
}

void PiiConfigurationWidget::setDefaults()
{ }
  
bool PiiConfigurationWidget::hasChanged() const
{
  return d->bChanged;
}

void PiiConfigurationWidget::setChanged(bool changed)
{
  d->bChanged = changed;
  emit contentsChanged(changed);
}

void PiiConfigurationWidget::setAcceptable(bool acceptable)
{
  d->bAcceptable = acceptable;
  emit acceptStateChanged(acceptable);
}

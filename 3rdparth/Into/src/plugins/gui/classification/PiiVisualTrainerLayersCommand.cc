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

#include "PiiVisualTrainerLayersCommand.h"


PiiVisualTrainerLayersCommand::PiiVisualTrainerLayersCommand(PiiVisualTrainerWidget *parentWidget,
                                                             const QVariantList& oldState,
                                                             const QVariantList& newState,
                                                             const QString& text,
                                                             QUndoCommand *parent) :QUndoCommand(parent),
                                                                                    _pParent(parentWidget),
                                                                                    _oldState(oldState),
                                                                                    _newState(newState),
                                                                                    _bFirstTime(true)
{
  setText(text);
}

void PiiVisualTrainerLayersCommand::undo()
{
  _pParent->setLayers(_oldState, true);
  if (!_previousLabels.empty())
    _pParent->setLabels(_previousLabels, true);
}

void PiiVisualTrainerLayersCommand::redo()
{
  if (_bFirstTime)
    _bFirstTime = false;
  else
    _pParent->setLayers(_newState, true);
}

void PiiVisualTrainerLayersCommand::setLabels(const QVariantList& labels)
{
  _previousLabels = labels;
}

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

#ifndef _PIIVISUALTRAINERLABELINGCOMMAND_H
#define _PIIVISUALTRAINERLABELINGCOMMAND_H

#include <QUndoCommand>
#include <PiiVisualTrainerWidget.h>
#include <QVector>

class PiiVisualTrainerLabelingCommand : public QUndoCommand
{
  
public:
  PiiVisualTrainerLabelingCommand(PiiVisualTrainerWidget *parentWidget,
                                  const QVariantList& oldState,
                                  const QVariantList& newState,
                                  const QString& text,
                                  QUndoCommand *parent = 0);
  ~PiiVisualTrainerLabelingCommand() {};

  /**
   * Applies a change to the document.
   */
  void redo();

  /**
   * Reverts a change to the document.
   */
  void undo();

private:
  PiiVisualTrainerWidget *_pParent;
  QVariantList _oldState, _newState;
  bool _bFirstTime;
};



#endif //_PIIVISUALTRAINERLABELINGCOMMAND_H

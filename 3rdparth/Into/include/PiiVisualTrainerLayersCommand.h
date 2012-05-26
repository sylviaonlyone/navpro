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

#ifndef _PIIVISUALTRAINERLAYERSCOMMAND_H
#define _PIIVISUALTRAINERLAYERSCOMMAND_H

#include <QUndoCommand>
#include <PiiVisualTrainerWidget.h>
#include <QVector>

class PiiVisualTrainerLayersCommand : public QUndoCommand
{
  
public:
  PiiVisualTrainerLayersCommand(PiiVisualTrainerWidget *parentWidget,
                                const QVariantList& oldState,
                                const QVariantList& newState,
                                const QString& text,
                                QUndoCommand *parent = 0);
  ~PiiVisualTrainerLayersCommand() {};

  /**
   * Applies a change to the document.
   */
  void redo();

  /**
   * Reverts a change to the document.
   */
  void undo();

  /**
   * Set previous labels. When we change layers, we must save the
   * previous state of the labels, because labels may be changed when
   * setting new layers. That's why we must restore labels when undo
   * layers.
   */
  void setLabels(const QVariantList& labels);
  
private:
  PiiVisualTrainerWidget *_pParent;
  QVariantList _oldState, _newState, _previousLabels;
  bool _bFirstTime;
};



#endif //_PIIVISUALTRAINERLAYERSCOMMAND_H

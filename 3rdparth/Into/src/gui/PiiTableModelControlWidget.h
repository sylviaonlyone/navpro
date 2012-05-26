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

#ifndef _PIITABLEMODELCONTROLWIDGET_H
#define _PIITABLEMODELCONTROLWIDGET_H

#include "PiiGui.h"
#include <QWidget>
#include <QBoxLayout>

class PiiTableModel;
class QPushButton;

/**
 * A widget with Add, Delete, Move up, and Move down buttons for
 * controlling a PiiTableModel.
 *
 *
 * @ingroup Gui
 */
class PII_GUI_EXPORT PiiTableModelControlWidget : public QWidget
{
  Q_OBJECT

  /**
   * Layout direction. By default, control buttons are laid out
   * vertically, from top to bottom.
   */
  Q_PROPERTY(QBoxLayout::Direction direction READ direction WRITE setDirection);

  /**
   * Show/hide move buttons. The buttons are show by default. If you
   * do not want the user to be able to move items in a PiiTableModel,
   * set this flag to @p false.
   */
  Q_PROPERTY(bool moveButtonsVisible READ moveButtonsVisible WRITE setMoveButtonsVisible);
   
public:
  /**
   * Create a new %PiiTableModelControlWidget with the given parent
   * widget.
   */
  PiiTableModelControlWidget(QWidget* parent = 0);
  /**
   * Destroy the widget.
   */
  ~PiiTableModelControlWidget();

  /**
   * Connect the control widget to the given @p model. The Delete and
   * Move buttons of a connected widget will be automatically enabled
   * and disabled. Pushing the buttons will modify the model.
   */
  void connect(PiiTableModel* model);

  void setDirection(QBoxLayout::Direction direction);
  QBoxLayout::Direction direction() const;

  void setMoveButtonsVisible(bool moveButtonsVisible);
  bool moveButtonsVisible() const;
  
private:
  struct Data
  {
    Data(PiiTableModelControlWidget* parent);
    static QString tr(const char* text);
    
    QPushButton* pbtnAdd;
    QPushButton* pbtnDelete;
    QPushButton* pbtnMoveUp;
    QPushButton* pbtnMoveDown;
  } *d;
};

#endif //_PIITABLEMODELCONTROLWIDGET_H

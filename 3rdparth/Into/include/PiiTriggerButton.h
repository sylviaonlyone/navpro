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

#ifndef _PIITRIGGERBUTTON_H
#define _PIITRIGGERBUTTON_H

#include <QWidget>
#include "PiiGui.h"

class QToolButton;

/**
 * A pair of arrow buttons. This widget can be used to trigger, for
 * example, PiiImageFileReader and PiiVideoReader.
 *
 * @ingroup Gui
 */
class PII_GUI_EXPORT PiiTriggerButton : public QWidget
{
  Q_OBJECT  

  /**
   * Determines the buttons to display. The default @p Left | @p
   * Right, which means that both arrow buttons will be shown.
   */
  Q_PROPERTY(Buttons buttons READ buttons WRITE setButtons);
  Q_FLAGS(Buttons);
  
public:
  /**
   * Buttons to show.
   *
   * @lip Left - show left arrow button
   * @lip Right - show right arrow button
   */
  enum Button { Left = 1, Right = 2 };
  Q_DECLARE_FLAGS(Buttons, Button);
  
  PiiTriggerButton(Buttons buttons = QFlags<Button>(Left) | Right);
  ~PiiTriggerButton();

  void setButtons(Buttons buttons);
  Buttons buttons() const;

signals:
  /**
   * Emitted when either of the buttons is pressed. The @a direction
   * parameter tells which one: -1 means back and 1 means forward.
   */
  void triggered(int direction);

private:
  class Data
  {
  public:
    Data(Buttons buttons);
    Buttons buttons;
    QToolButton *pLeft, *pRight;
  } *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PiiTriggerButton::Buttons);

#endif //_PIITRIGGERBUTTON_H

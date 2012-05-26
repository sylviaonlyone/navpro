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

#ifndef _PIIUNDOCOMMANDPRODUCER_H
#define _PIIUNDOCOMMANDPRODUCER_H

#include "PiiGlobal.h"
#include <QObject>

class QUndoCommand;

/**
 * An interface for UI components that support undo/redo functionality.
 * Using this interface makes it possible to combine undo/redo
 * functionality provided by diverse software components into a global
 * undo/redo function.
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiUndoCommandProducer
{
public:
  /**
   * A listener interface for applied undoable commands. This
   * interface is implemented by the application that controls a set
   * of UI components performing undoable (a.k.a impossible) actions.
   */
  class Listener
  {
  public:
    /**
     * Informs the listener that an undoable action has been
     * performed.
     *
     * @param sender the object that performed the action
     *
     * @param command the action that was performed. The receiver @b
     * must take the ownership of this pointer. Typically, it will be
     * placed on a QUndoStack.
     */
    virtual void undoCommandPerformed(PiiUndoCommandProducer* sender, QUndoCommand* command) = 0;
  };

  /**
   * Default constructor.
   */
  PiiUndoCommandProducer();
  
  /**
   * Virtual destructor. Does nothing.
   */
  virtual ~PiiUndoCommandProducer();

  /**
   * Set the listener.
   */
  void setUndoCommandListener(Listener* undoCommandListener);
  /**
   * Get the listener.
   */
  Listener* undoCommandListener() const;

  /**
   * Generate a unique id number for a class of undoable commands. 
   * Unique id numbers can be utilized by Qt's undo system in merging
   * successive commands. Using the technique described in the example
   * below ensures application-wide unique command identifiers and
   * ensures that no overlapping ids will be used by independent
   * components.
   *
   * @code
   * // MyCommand.h
   * class MyCommand : public QUndoCommand
   * {
   * public:
   *   //...
   *   int id() { return iCommandId; }
   * private:
   *   static int iCommandId;
   * };
   *
   * // MyCommand.cc
   * int MyCommand::iCommandId = PiiUndoCommandProducer::generateId();
   * @endcode
   */
  static int generateId();

protected:
  /**
   * Called by subclasses to indicate that an undoable command has
   * been performed. The caller must release the ownership of the
   * pointer. If there is a listener, the command will be passed to
   * its @p undoCommandPerformed() function. If there is no listener,
   * the command will be deleted.
   */
  void undoCommandPerformed(QUndoCommand* command);
  
private:
  Listener* _pUndoCommandListener;
};

Q_DECLARE_INTERFACE(PiiUndoCommandProducer, "com.intopii.into.piiundocommandproducer/1.0");

#endif //_PIIUNDOCOMMANDPRODUCER_H

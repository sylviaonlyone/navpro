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

#ifndef _PIIPROBEINPUT_H
#define _PIIPROBEINPUT_H

#include "PiiInputSocket.h"
#include "PiiInputController.h"

/**
 * An input socket that works without a controlling operation. The
 * socket works like a measurement probe that emits the
 * #objectReceived() signal whenever a new object is received. It also
 * saves the last received object.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiProbeInput :
  public PiiSocket,
  public PiiAbstractInputSocket,
  public PiiInputController
{
  Q_OBJECT

public:
  /**
   * Constructs a new probe input and sets its @p objectName property
   * to @a name.
   */
  PiiProbeInput(const QString& name = "probe");

  /**
   * Constructs a new probe input and connects it to @a output. 
   * Connects the #objectReceived() signal to @a slot in @a receiver.
   *
   * @code
   * PiiOperation* reader = engine.createOperation("PiiImageFileReader");
   * PiiImageDisplay* display = new PiiImageDisplay;
   * PiiProbeInput* probe = new PiiProbeInput(reader->output("image"),
   *                                          display, SLOT(setImage(PiiVariant)));
   * @endcode
   */
  PiiProbeInput(PiiAbstractOutputSocket* output, const QObject* receiver,
                const char* slot, Qt::ConnectionType = Qt::AutoConnection);

  /**
   * Returns @p Input.
   */
  PiiSocket::Type type() const;
  
  /**
   * Emits #objectReceived() and saves the received object.
   */
  bool tryToReceive(PiiAbstractInputSocket* sender, const PiiVariant& object) throw ();

  /**
   * Returns the last received object. If no object has been received,
   * returns an invalid variant.
   */
  PiiVariant savedObject() const;
  /**
   * Sets the saved object to @a obj.
   */
  void setSavedObject(const PiiVariant& obj);

  /**
   * Returns true if an object has been saved into this socket.
   */
  bool hasSavedObject() const;

  PiiInputController* controller() const;

  PiiProbeInput* socket();
  PiiAbstractInputSocket* asInput();
  PiiAbstractOutputSocket* asOutput();

signals:
  /**
   * Emitted whenever an object is received in this input socket. Note
   * that all objects, including control objects, will be emitted. The
   * slot receiving the objects can filter the objects based on their
   * type.
   *
   * @see PiiYdin::isControlType()
   */
  void objectReceived(const PiiVariant& obj);

private:
  class Data : public PiiAbstractInputSocket::Data
  {
  public:
    PiiVariant varSavedObject;
  };
  PII_UNSAFE_D_FUNC;
};

Q_DECLARE_METATYPE(PiiProbeInput*);

#endif //_PIIPROBEINPUT_H

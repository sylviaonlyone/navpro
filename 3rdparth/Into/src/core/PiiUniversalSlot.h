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

#include "PiiGlobal.h"

#include <QObject>
#include <QVariantList>

/**
 * An object that can be used as a slot for any signal. 
 * %PiiUniversalSlot has no static moc-generated slots, but itcan
 * create a matching a slot for any signal at run time. When the slot
 * is invoked, the call is passed to the protected #invokeSlot()
 * function that must be implemented in a subclass.
 *
 * @note Since Qt's meta-object system relies on a static number of
 * signals/slots in a class, classes derived from %PiiUniversalSlot
 * cannot have ordinary signals and slots.
 *
 * @ingroup Core
 */
class PII_CORE_EXPORT PiiUniversalSlot : public QObject
{
  // Omitting Q_OBJECT on purpose.
public:
  ~PiiUniversalSlot();
  
  /// @internal
  int qt_metacall(QMetaObject::Call call, int id, void** arguments);

  /**
   * Connects the given @a signal in the @a source object to this
   * object. If the @a slot already exists, the signal will be
   * connected to it. Otherwise, a new slot will be created and
   * assigned a unique id.
   *
   * @code
   * class MySlot : public PiiUniversalSlot { ... };
   * QObject obj;
   * MySlot mySlot;
   * mySlot.dynamicConnect(&obj, "destroyed()");
   * @endcode
   *
   * @param source the sender of the signal
   *
   * @param signal the signature of the signal to connect to. You can
   * use the SIGNAL macro to wrap the signal name, but it is not
   * necessary.
   *
   * @param slot the signature of the dynamic slot to create. If @a
   * slot is zero, @a signal will be used as the signature of the slot
   * as well. You can use the SLOT macro to wrap the slot name, but it
   * is not necessary.
   *
   * @param type the type of the connection
   *
   * @returns the id of the slot (either new or existing), or -1 on
   * failure.
   */
  int dynamicConnect(QObject* source, const char* signal, const char* slot = 0,
                     Qt::ConnectionType type = Qt::AutoConnection);

  /**
   * Disconnects a signal from a dynamic slot.
   *
   * @return @p true on success, @p false on failure
   *
   * @see dynamicConnect()
   */
  int dynamicDisconnect(QObject* source, const char* signal, const char* slot = 0);

protected:
  PiiUniversalSlot();

  /**
   * Invokes the universal slot.
   *
   * @param id the id of the slot that was invoked
   *
   * @param args typeless pointers to the signal's arguments. args[0]
   * is the return value, and the rest of the values point to function
   * call parameters, whose types match those listed in the signature.
   *
   * @return @p true if the call was successful and @p false otherwise.
   *
   * @code
   * bool MySlot::invokeSlot(int id, void** args)
   * {
   *   if (signatureOf(id) == "valueChanged(int)")
   *     {
   *       int iValue = *reinterpret_cast<int*>(args[1]);
   *       // ...
   *     }
   *   return true;
   * }
   * @endcode
   */
  virtual bool invokeSlot(int id, void** args) = 0;

  /**
   * Returns the normalized signature of the slot corresponding to
   * @a id.
   */
  QByteArray signatureOf(int id) const;

  /**
   * Converts the arguments of the slot identified by @a id to a
   * QVariantList.
   *
   * @code
   * bool MySlot::invokeSlot(int id, void** args)
   * {
   *   QVariantList lstArgs(argsToList(id, args));
   *   if (lstArgs.size() == 2)
   *     dealWithTwoArgs(); // ...
   *   return true;
   * }
   * @endcode
   */
  QVariantList argsToList(int id, void** args) const;

  /**
   * Returns the number of arguments for the slot identified by @a id.
   */
  int argumentCount(int id) const;

private:
  class Data
  {
  public:
    QList<QByteArray> lstSlots;
    QList<QList<int> > lstTypes;
  } *d;

  PII_DISABLE_COPY(PiiUniversalSlot);
};

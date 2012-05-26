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

#ifndef _PIITRIGGERSOURCE_H
#define _PIITRIGGERSOURCE_H

#include <PiiBasicOperation.h>
#include <QMutex>

/**
 * A source operation that can be triggered with Qt's signals. This
 * operation is useful when one needs to send values from a user
 * interface to the analysis engine.
 *
 * @outputs
 *
 * @out trigger - any object received into the #trigger() slot.
 *
 * @ingroup PiiBasePlugin
 */
class PiiTriggerSource : public PiiBasicOperation
{
  Q_OBJECT;
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiTriggerSource();

  /**
   * Stop the process.
   *
   * @see PiiOperation::stop().
   */
  void stop();

  /**
   * Pause the process.
   *
   * @see PiiOperation::pause().
   */
  void pause();

  /**
   * Start the process.
   *
   * @see PiiOperation::start().
   */
  void start();

  /**
   * Interrupt the process.
   *
   * @see PiiOperation::interrupt().
   */
  void interrupt();
  
public slots:
  /**
   * Send an @p int (1) to the output socket.
   */
  void trigger();
  
  /**
   * Send a PiiVariant to the output socket.
   */
  void trigger(const PiiVariant& value);
  /**
   * Send an @p int to the output socket.
   */
  void trigger(int value);
  /**
   * Send a @p double to the output socket.
   */
  void trigger(double value);
  /**
   * Send a QString to the output socket.
   */
  void trigger(const QString& value);

private:
  /**
   * Send a given value to the output socket.
   */
  template <class T> void emitValue(T value);

  /// @internal
  class Data : public PiiBasicOperation::Data
  {
  public:
    Data();
    
    QMutex stateMutex;
  };
  PII_D_FUNC;

};

#endif //_PIITRIGGERSOURCE_H

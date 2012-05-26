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

#ifndef _PIIOBJECTCAPTURER_H
#define _PIIOBJECTCAPTURER_H

#include <PiiDefaultOperation.h>
#include <QVariantList>

/**
 * Emits a received objects as a Qt signal.
 *
 * @inputs
 *
 * @in sync - sync input. Any type. If this input is not connected,
 * each incoming object (or a set of objects) will be sent once
 * received. If this input is connected, every object related to the
 * sync object will be sent once everything has been received.
 *
 * @in inputX - reads in objects of any type. X ranges from 1 to
 * #dynamicInputCount - 1. @p input0 can also be accessed as @p input.
 * 
 * @ingroup PiiBasePlugin
 */
class PiiObjectCapturer : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The number of dynamic inputs (in addition to @p sync). The
   * default is one.
   */
  Q_PROPERTY(int dynamicInputCount READ dynamicInputCount WRITE setDynamicInputCount);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiObjectCapturer();
  ~PiiObjectCapturer();
    
  /**
   * Aliases @p input to @p input0.
   */
  PiiInputSocket*	input(const QString &name) const;

  void check(bool reset);

signals:
  /**
   * Emitted for each incoming object if the @p sync input is not
   * connected and if #dynamicInputCount is one.
   */
  void objectCaptured(const PiiVariant& object);
  /**
   * Emitted for each incoming object if the @p sync input is not
   * connected and if #dynamicInputCount is greater than one. Each
   * element in the list will be a PiiVariant. The first object is the
   * one read from @p input0 and so on.
   */
  void objectsCaptured(const QVariantList& objects);
  /**
   * Emitted when everything related to the object in the @p sync
   * input has been received.
   *
   * @param syncObject the object received in the @p sync input
   *
   * @param objects all collected objects. The length of this list
   * equals #dynamicInputCount, and each element is a QVariantList
   * containing the collected objects as PiiVariants.
   */
  void objectsCaptured(const PiiVariant& syncObject, const QVariantList& objects);
  
protected:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiInputSocket *pSyncInput;
    PiiVariant syncObject;
    QList<QVariantList> lstObjects;
    int iDynamicInputCount;
    bool bSyncInputConnected;
  };
  PII_D_FUNC;

  void process();
  void syncEvent(SyncEvent* event);

  void setDynamicInputCount(int dynamicInputCount);
  int dynamicInputCount() const;

private:
  void initObjectList();
  void clearObjects();
};


#endif //_PIIOBJECTCAPTURER_H

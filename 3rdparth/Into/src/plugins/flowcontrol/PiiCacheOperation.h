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

#ifndef _PIICACHEOPERATION_H
#define _PIICACHEOPERATION_H

#include <PiiDefaultOperation.h>
#include <QLinkedList>
#include <QHash>

#include <PiiMatrix.h>

/**
 * An operation that caches processing results. %PiiCacheOperation can
 * be used to avoid recalculating values that need to be used more
 * than once. The most typical use is in caching feature vectors used
 * for training a classifier.
 *
 * @note The implementation of this operation is still incomplete. 
 * Don't use.
 *
 * The cache works by associating each cached object with a @e key. 
 * Whenever a key is received, the cache is searched for an
 * occurrence. If a hit is found, it will be sent to the @p data
 * output. If not, the key will be passed to the @p key output. The @p
 * found output will emit either 0 or 1 depending on the search
 * result (0 = not found, 1 = found).
 *
 * The cache must be connected so that if an object is not found, a
 * new object will always be received in the @p data input. This
 * usually means a loop-back connection wired to the @p key output;
 * whenever a key is emitted, it causes the calculation of a new
 * object that will be sent back to the cache to be associated with
 * the key.
 *
 * @inputs
 *
 * @in key - a cache key that uniquely identifies data in the cache. A
 * QString or a primitive type that is convertible to a QString.
 *
 * @in data - the data associated with key. Any type. Note that this
 * input is not synchronous to @p key. It must receive an object if
 * and only if the @p key output emits an object. Objects in this
 * input will be passed directly to the @p data output.
 *
 * @outputs
 *
 * @out found - 0 or 1 depending on whether the key was found in cache
 * or not (0 = not found, 1 = found). This output can be used as a
 * control signal to a PiiDemuxOperation.
 *
 * @out key - passes the object in the @p key input, if the key was
 * not found in cache.
 *
 * @out data - the data associated with the key input, if found in the
 * cache. In the case of a cache miss, an object received in the @p
 * data input will be passed to this output.
 *
 * @ingroup PiiFlowControlPlugin
 */
class PiiCacheOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The maximum number of bytes the cache is allowed to occupy. The
   * operation recognizes all primitive types, complex numbers, and
   * matrices composed of them. It also knows QStrings. The number of
   * bytes an object occupies is an approximation because memory
   * allocation techniques vary. The operation uses an estimate of 32
   * bytes per each object (const PiiVariant&). If the object is a
   * primitive type, additional 32 bytes will be assumed. If it is a
   * matrix, 32 bytes + sizeof(T) * rows * columns will be assumed. 
   * The size of a QString is assumed to be 32 + two times its length. 
   * Zero means no limit. The default is 2 Mb.
   */
  Q_PROPERTY(int maxBytes READ maxBytes WRITE setMaxBytes);

  /**
   * The maximum number of objects the cache is allowed to hold. Zero
   * (the default) means no limit.
   */
  Q_PROPERTY(int maxObjects READ maxObjects WRITE setMaxObjects);

  /**
   * Allow/disallow changing of object emission order. If an object is
   * found in the cache, it could be sent to the data output right
   * away. Normally, this is however not done until all data related
   * to previously received requests has been handled to avoid
   * synchronization problems. If it doesn't matter in which order the
   * objects are sent, setting this flag to @p true may increase
   * processing speed on multi-core machines.
   */
  Q_PROPERTY(bool allowOrderChanges READ allowOrderChanges WRITE setAllowOrderChanges);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiCacheOperation();
  
  void setMaxBytes(int maxBytes);
  int maxBytes() const;
  void setMaxObjects(int maxObjects);
  int maxObjects() const;
  void setAllowOrderChanges(bool allowOrderChanges);
  bool allowOrderChanges() const;

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiInputSocket* pKeyInput, *pDataInput;
    PiiOutputSocket* pFoundOutput, *pKeyOutput, *pDataOutput;
    int iMaxBytes;
    int iMaxObjects;
    bool bAllowOrderChanges;

    int iConsumedMemory;
    
    QHash<QString, PiiVariant> hashObjects;
    QLinkedList<QString> lstKeys;
  };
  PII_D_FUNC;

};


#endif //_PIICACHEOPERATION_H

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

#ifndef _PIIOBJECTREPLICATOR_H
#define _PIIOBJECTREPLICATOR_H

#include <PiiDefaultOperation.h>

/**
 * An operation that replicates an object received in its @p data
 * input. The operation useful when an object needs to be repeated
 * many times. A typical situation arises when an image is split into
 * smaller pieces, each of which is processed separately. Later, the
 * pieces need to be connected to the name of the original image file,
 * which is sent by PiiImageFileReader only once per each image. The
 * solution is to connect the file name to the @p data input of a
 * PiiObjectReplicator and trigger its output with the smaller image
 * pieces.
 *
 * @note This operation can lead to lock-ups if not used carefully. If
 * two branches of a processing pipeline are connected to the @p data
 * and @p trigger inputs, make sure that @p data is always received
 * first. Otherwise, if there are no threaded operations in the
 * pipelines, and if the input queue in @p trigger becomes full before
 * @p data is received, the whole configuration may hang. The output
 * that sends data to the two branches should be first connected to @p
 * data and then to the other branch.
 *
 * @inputs
 *
 * @in data - the data that needs to be copied. Any type.
 *
 * @in trigger - emit the last object in @p data each time an object
 * is received in this input. Any type.
 *
 * @outputs
 *
 * @out output - the object last received in @p data. This output will
 * emit the object whenever an object is received in @p trigger.
 *
 * @ingroup PiiFlowControlPlugin
 */
class PiiObjectReplicator : public PiiDefaultOperation
{
  Q_OBJECT

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiObjectReplicator();

protected:
  void process();
  PiiFlowController* createFlowController();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    PiiVariant pData;
  };
  PII_D_FUNC;
};


#endif //_PIIOBJECTREPLICATOR_H

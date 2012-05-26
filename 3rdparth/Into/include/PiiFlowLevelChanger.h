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

#ifndef _PIIFLOWLEVELCHANGER_H
#define _PIIFLOWLEVELCHANGER_H

#include <PiiDefaultOperation.h>

/**
 * An operation that changes the flow level of data passed through it. 
 * It is useful in situations where one needs to adjust different flow
 * levels before connecting them to further processing. A typical use
 * is to fake image pieces to be "original" images as shown in the
 * image below. The images emitted through @p output will be at the
 * flow level of PiiImageFileReader's @p image.
 *
 * @image html flowlevelchanger.png
 *
 * @inputs
 *
 * @in sync - Synchronization input. The output will always maintain
 * the flow level of this input.
 *
 * @in input - The objects whose flow level must match @p sync.
 * 
 * @outputs
 *
 * @out output - Objects read from @p input at the flow level of @p
 * sync.
 *
 * @ingroup PiiFlowControlPlugin
 */
class PiiFlowLevelChanger : public PiiDefaultOperation
{
  Q_OBJECT

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiFlowLevelChanger();

protected:
  void process();
};


#endif //_PIIFLOWLEVELCHANGER_H

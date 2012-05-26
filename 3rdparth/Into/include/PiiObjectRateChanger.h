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

#ifndef _PIIOBJECTRATECHANGER_H
#define _PIIOBJECTRATECHANGER_H

#include <PiiDefaultOperation.h>

/**
 * Replicate or sample input objects. Depending on the value of
 * #rateChange, this operation either replicates each input object N
 * times or passes just every Nth object.
 *
 * @inputs
 *
 * @in input - any object
 * 
 * @outputs
 *
 * @out output - input objects, replicated or sampled
 *
 * @ingroup PiiFlowControlPlugin
 */
class PiiObjectRateChanger : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * Change in object rate. The default value is 0 which causes every
   * input object to be passed as such. If this value is greater than
   * zero, every object will be sent @p rateChange times. If the value
   * is less than zero, @p abs(rateChange) objects will be skipped
   * after each passed object. For example, a rate change of -1 means
   * that every second object will be passed, starting from the second
   * one.
   */
  Q_PROPERTY(int rateChange READ rateChange WRITE setRateChange);

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiObjectRateChanger();

  void check(bool reset);

protected:
  void process();

  void setRateChange(int rateChange);
  int rateChange() const;

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    int iRateChange, iCounter;
  };
  PII_D_FUNC;
};


#endif //_PIIOBJECTRATECHANGER_H

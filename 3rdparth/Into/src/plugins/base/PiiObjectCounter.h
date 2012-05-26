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

#ifndef _PIIOBJECTCOUNTER_H
#define _PIIOBJECTCOUNTER_H

#include <PiiDefaultOperation.h>

/**
 * Counts received objects.
 *
 * @inputs
 *
 * @in input - any object
 * 
 * @outputs
 *
 * @out count - the number of objects received so far.
 *
 * @ingroup PiiBasePlugin
 */
class PiiObjectCounter : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The number of objects received. If you want zero-based indices
   * instead of object count, set the count to -1 before starting the
   * operation.
   */
  Q_PROPERTY(int count READ count WRITE setCount);

  /**
   * If this flag is @p true, the value of the counter will be reset
   * to zero when the operation is restarted after stopping. If the
   * flag is @p false, the previous count will be retained. The
   * default value is @p true.
   */
  Q_PROPERTY(bool autoReset READ autoReset WRITE setAutoReset);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiObjectCounter();

  void check(bool reset);

  void setCount(int count);
  int count() const;
  void setAutoReset(bool autoReset);
  bool autoReset() const;
  
protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    int iCount;
    bool bAutoReset;
  };
  PII_D_FUNC;
};


#endif //_PIIOBJECTCOUNTER_H

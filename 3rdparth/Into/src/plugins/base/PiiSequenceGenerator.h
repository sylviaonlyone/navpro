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

#ifndef _PIISEQUENCEGENERATOR_H
#define _PIISEQUENCEGENERATOR_H

#include <PiiDefaultOperation.h>

/**
 * An operation that generates sequences of numbers. The sequence goes
 * from #sequenceStart to #sequenceEnd, stepping #step units each time.
 *
 * @inputs
 *
 * @in trigger - an optional input that triggers the emission.
 * 
 * @outputs
 *
 * @out output - a number in the sequence.
 *
 * @ingroup PiiBasePlugin
 */
class PiiSequenceGenerator : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The first value in the sequence. The default value is 0. This
   * value must be less than or equal to #sequenceEnd.
   */
  Q_PROPERTY(double sequenceStart READ sequenceStart WRITE setSequenceStart);
  /**
   * The last value in the sequence. The default value is 1. This
   * value must be larger than or equal to #sequenceStart.
   */
  Q_PROPERTY(double sequenceEnd READ sequenceEnd WRITE setSequenceEnd);
  /**
   * The increment on each iteration. Use a negative value to go
   * backwards.
   */
  Q_PROPERTY(double step READ step WRITE setStep);
  /**
   * The sequence mode. The default mode is @p Repeat.
   */
  Q_PROPERTY(SequenceMode sequenceMode READ sequenceMode WRITE setSequenceMode);
  Q_ENUMS(SequenceMode);
  
  /**
   * The number of times the sequence will be repeated. Once this
   * count is reached, the operation will send a stop signal which
   * will stop the whole execution engine if properly connected. The
   * default value is zero, which means "eternally". This value has no
   * effect if @p trigger is connected.
   */
  Q_PROPERTY(int repeatCount READ repeatCount WRITE setRepeatCount);
  /**
   * The data type of the output. By default, this value is @p false
   * and @p doubles will be emitted. Setting this value to @p true
   * causes converts the output to @p ints.
   */
  Q_PROPERTY(bool integerOutput READ integerOutput WRITE setIntegerOutput);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Sequence modes.
   *
   * @lip Repeat - the sequence will start at the beginning once
   * #sequenceEnd is reached.
   *
   * @lip BackAndForth - the sequence goes back and forth between
   * #sequenceEnd and #sequenceStart.
   */
  enum SequenceMode { Repeat, BackAndForth };
  
  PiiSequenceGenerator();

  void setSequenceStart(double sequenceStart);
  double sequenceStart() const;
  void setSequenceEnd(double sequenceEnd);
  double sequenceEnd() const;
  void setStep(double step);
  double step() const;
  void setSequenceMode(const SequenceMode& sequenceMode);
  SequenceMode sequenceMode() const;
  void setRepeatCount(int repeatCount);
  int repeatCount() const;
  void setIntegerOutput(bool integerOutput);
  bool integerOutput() const;

  void check(bool reset);

protected:
  void process();

private:
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dSequenceStart;
    double dSequenceEnd;
    double dStep;
    SequenceMode sequenceMode;
    int iRepeatCount;
    bool bIntegerOutput;
    double dCurrentValue, dCurrentStep;
    int iCurrentRepeatCount;
    bool bAutoExit;
  };
  PII_D_FUNC;
};


#endif //_PIISEQUENCEGENERATOR_H

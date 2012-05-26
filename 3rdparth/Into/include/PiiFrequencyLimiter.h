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

#ifndef _PIIFREQUENCYLIMITER_H
#define _PIIFREQUENCYLIMITER_H

#include <QTime>

/**
 * This is the class for the limit input frequency to the wanted
 * frequency. Notice that the wanted frequency is the max frequency,
 * so the real output frequency is a little bit smaller than wanted max
 * frequency.
 *
 * @ingroup PiiSmartClassifierOperation.
 */

class PiiFrequencyLimiter
{
  
public:
  PiiFrequencyLimiter(double maxFrequency = 10);

  /**
   * This function calculate mean frequence and check if it is smaller
   * than max frequency.
   *
   */
  bool check();

  /**
   * Set and get the max frequency.
   */
  void setMaxFrequency(double maxFrequency);
  double maxFrequency() { return _dMaxFrequency; }
  
private:
  double _dMaxFrequency,_dMaxMeanInterval,_dFilterInterval,_dMeanInterval;
  QTime _filterTime;

};

#endif //_PIIFREQUENCYLIMITER_H

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

#include "PiiFrequencyLimiter.h"

PiiFrequencyLimiter::PiiFrequencyLimiter(double maxFrequency)
{
  setMaxFrequency(maxFrequency);
  _filterTime.start();
}

bool PiiFrequencyLimiter::check()
{
  bool bUpdateObject = true;
  if (_dFilterInterval > 0)
    {
      int elapsed = _filterTime.elapsed();
      _dMeanInterval = 0.05 * elapsed + 0.95 * _dMeanInterval;
      
      if (_dMeanInterval > _dMaxMeanInterval)
        _dMeanInterval = _dMaxMeanInterval;
      
      if ( (double)_dFilterInterval > _dMeanInterval )
        bUpdateObject = false;
    }

  if ( bUpdateObject )
    _filterTime.restart();

  return bUpdateObject;
}

void PiiFrequencyLimiter::setMaxFrequency(double frequency)
{
  _dMaxFrequency = frequency;
  if (frequency == 0)
    {
      _dFilterInterval = 0;
      _dMaxMeanInterval = 0;
    }
  else
    {
      _dFilterInterval = (int)(1.0/frequency*1000.0);
      _dMaxMeanInterval = _dFilterInterval + frequency*0.05;
      _dMeanInterval = _dMaxMeanInterval;
    }
}


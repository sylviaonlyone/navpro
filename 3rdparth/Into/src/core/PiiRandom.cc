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

#include "PiiRandom.h"
#include "PiiMath.h"
#include <QDateTime>

namespace Pii
{
  void seedRandom()
  {
    QTime now(QTime::currentTime());
    seedRandom(QDateTime::currentDateTime().toTime_t() ^ now.msec());
  }

  PiiMatrix<double> uniformRandomMatrix(int rows, int columns)
  {
    PiiMatrix<double> result(PiiMatrix<double>::uninitialized(rows, columns));
    for (PiiMatrix<double>::iterator i=result.begin(); i != result.end(); ++i)
      *i = uniformRandom();
    return result;
  }

  PiiMatrix<double> uniformRandomMatrix(int rows, int columns,
                                        double min, double max)
  {
    PiiMatrix<double> result(PiiMatrix<double>::uninitialized(rows, columns));
    for (PiiMatrix<double>::iterator i=result.begin(); i != result.end(); ++i)
      *i = uniformRandom(min,max);
    return result;
  }

  double normalRandom()
  {
    return erf(uniformRandom());
  }

  PiiMatrix<double> normalRandomMatrix(int rows, int columns)
  {
    PiiMatrix<double> result(PiiMatrix<double>::uninitialized(rows, columns));
    for (PiiMatrix<double>::iterator i=result.begin(); i != result.end(); ++i)
      *i = normalRandom();
    return result;
  }
}

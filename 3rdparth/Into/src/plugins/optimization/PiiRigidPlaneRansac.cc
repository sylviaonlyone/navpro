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

#include "PiiRigidPlaneRansac.h"
#include <PiiMath.h>

namespace PiiOptimization
{
  PiiMatrix<float> rigidPlaneModelToTransform(const PiiMatrix<double>& model)
  {
    double dCos = model(0) * Pii::cos(model(1)), dSin = model(0) * Pii::sin(model(1));
    return PiiMatrix<float>(3,3,
                            dCos, -dSin, model(2),
                            dSin,  dCos, model(3),
                            0.0,   0.0,  1.0);
  }
}


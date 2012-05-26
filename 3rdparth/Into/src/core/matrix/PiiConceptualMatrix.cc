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

#include "PiiConceptualMatrix.h"
#include <QCoreApplication>

namespace Pii
{
  QString notSquareErrorMessage()
  {
    static QString msg = QCoreApplication::translate("PiiMatrix", "Matrix must be square.");
    return msg;
  }

  QString sizeMismatchErrorMessage()
  {
    static QString msg = QCoreApplication::translate("PiiMatrix", "Matrix dimensions do not match.");
    return msg;
  }
}


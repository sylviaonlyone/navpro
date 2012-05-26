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

#include "PiiMatrixUtil.h"
#include <QRegExp>
#include <QStringList>

namespace Pii
{
  PiiMatrix<double> matlabParse(const QString& str)
  {
    QString strCopy(str);
    // Remove line break marks, commas, brackets, and tabulators
    strCopy.replace(QRegExp("[\\[\\],\\t]|\\.\\.\\.")," ");
    // Rows are separated by semicolons or newlines
    QStringList rowStrs = strCopy.split(QRegExp(" *; *\\r?\\n?|\\r?\\n"), QString::SkipEmptyParts);
    QList<QList<double> > rows;
    int maxCols = 0;
    for (int r=0; r<rowStrs.size(); r++)
      {
        // Elements are now separated with spaces (commas were taken
        // away)
        QStringList row = rowStrs[r].split(' ', QString::SkipEmptyParts);
        int colCnt = row.size();
        // Find maximum width
        if (colCnt > maxCols)
          maxCols = colCnt;
        QList<double> dRow;
        for (int c=0; c<row.size(); c++)
          dRow << row[c].toDouble();
        rows << dRow;
      }
    
    // Store values into the result matrix
    PiiMatrix<double> result(rows.size(), maxCols);
    for (int r=0; r<rows.size(); r++)
      for (int c=0; c<rows[r].size(); c++)
        result(r,c) = rows[r][c];
    return result;
  }
}

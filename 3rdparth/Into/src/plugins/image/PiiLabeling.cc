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

#include "PiiLabeling.h"

namespace PiiImage
{
  // Mark a sequence of detected object pixels into the label buffer
  void markToBuffer(LabelInfo& info, int rowIndex, int start, int end)
  {
    // Mark the run into the label buffer
    int* runRow = info.matLabels[rowIndex];
    for (int c = start; c < end + info.iConnectivityShift; ++c)
      runRow[c] = info.iLabelIndex;
  }

  // On row rowIndex, find all runs that overlap with the range
  // start-end.
  void connectRunsRecursively(LabelInfo& info, int rowIndex, int start, int end)
  {
    // Out of image boundaries...
    if (rowIndex < 0 || rowIndex >= info.lstRuns.size())
      return;

    // Go through all runs on this row and find the overlapping ones
    for (RunNode* pNode = info.lstRuns[rowIndex].first; pNode != 0;)
      {
        // No overlap
        if (start > pNode->end ||
            end < pNode->start)
          {
            pNode = pNode->next;
            continue;
          }

        // Invalidate the current run to prevent loops in recursion
        int end = pNode->end;
        pNode->end = -1;
        pNode->seed = false;
        // Mark and recurse
        markToBuffer(info, rowIndex, pNode->start, end);
        connectRunsRecursively(info, rowIndex - 1, pNode->start, end);
        connectRunsRecursively(info, rowIndex + 1, pNode->start, end);
        // Destroy the node
        info.lstRuns[rowIndex].remove(pNode);
        RunNode* pNodeToDelete = pNode;
        pNode = pNode->next;
        delete pNodeToDelete;
      }
  }
}

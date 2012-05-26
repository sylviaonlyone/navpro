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

#include <PiiPajaUtil.h>

QList<QList<PiiOperation*> > PiiPajaUtil::checkOrder(const QList<PiiOperation*>& operations)
{
  /*QList<QList<PiiOperation*> > lstOperations;
  for (int i=0; i<operations.size(); i++)
    {
      QList<PiiOperation*> list;
      list.append(operations[i]);

      lstOperations.append(list);
    }

  return lstOperations;
  */
  
  /**
   * 1. Collect pairList.
   *
   * If the operation A has no dependencies, then we create a pair
   * <0,A>
   *
   * If the operation B has dependencies C and D, we will create pairs
   * <C,B>
   * <D,B>
   */
  
  QList<QPair<PiiOperation*,PiiOperation*> > pairList;
  for (int i=0; i<operations.size(); i++ )
    {
      PiiOperation *pCurrentOperation = operations[i];
      QList<PiiOperation*> lstDependencies = findDependencies(operations,pCurrentOperation);
      if (lstDependencies.size() > 0)
        {
          for (int j=0; j<lstDependencies.size(); j++)
            {
              QPair<PiiOperation*, PiiOperation*> pair(lstDependencies[j],pCurrentOperation);
              pairList << pair;
            }
        }
      else
        {
          QPair<PiiOperation*, PiiOperation*> pair(0,pCurrentOperation);
          pairList << pair;
        }
    }
  
  /**
   * 2. Rearrange operations depends on collected pairs.
   *
   */
  QList<QList<PiiOperation*> > lstOperations;
  while (pairList.size() > 0)
    {
      // Find all nodes, where the first element is 0
      QList<int> lstReadyPairs;
      for (int i=pairList.size(); i--;)
        {
          // If the first element is a 0, we can handle it.
          if (pairList[i].first == 0)
            lstReadyPairs << i;
        }

      // If there is no any ready pairs, we must guess the start point
      if (lstReadyPairs.isEmpty())
        {
          pairList[0].first = 0;
        }
      else
        {
          // Take the ready operations
          QList<PiiOperation*> readyOperations;
          for (int i=0; i<lstReadyPairs.size(); i++)
            {
              if (pairList[lstReadyPairs[i]].second != 0)
                readyOperations << pairList[lstReadyPairs[i]].second;
            }
          
          // Remove pairs
          for (int i=0; i<lstReadyPairs.size(); i++)
            pairList.removeAt(lstReadyPairs[i]);
          
          // Mark first items to zero
          for (int i=0; i<pairList.size(); i++)
            {
              if (readyOperations.contains(pairList[i].first))
                pairList[i].first = 0;
            }
          
          lstOperations << readyOperations;
        }
    }

  return lstOperations;
}

QList<PiiOperation*> PiiPajaUtil::findDependencies(const QList<PiiOperation*>& operations, PiiOperation* operation)
{
  QList<PiiAbstractInputSocket*> lstInputs = operation->inputs();
  
  QList<PiiOperation*> lstDependencies;
  
  for (int i=0; i<lstInputs.size(); i++)
    {
      PiiAbstractOutputSocket *pOutput = lstInputs[i]->connectedOutput();
      if (pOutput)
        {
          for (int j=0; j<operations.size(); j++)
            {
              if (operations[j] != operation)
                {
                  QList<PiiAbstractOutputSocket*> lstOutputs = operations[j]->outputs();
                  if (lstOutputs.contains(pOutput))
                    {
                      lstDependencies << operations[j];
                      break;
                    }
                }
            }
          
        }
      
    }

  return lstDependencies;
}

QList<QList<QPointF> > PiiPajaUtil::checkPositions(const QList<QList<PiiOperation*> >& operations)
{
  QList<QList<QPointF> > lstPoints;
  for (int i=0; i<operations.size(); i++)
    {
      QList<PiiOperation*> lstOperations = operations[i];
      QList<QPointF> points;
      for (int j=0; j<lstOperations.size(); j++)
        points << QPointF(i,j);

      lstPoints.append(points);
    }

  return lstPoints;
}

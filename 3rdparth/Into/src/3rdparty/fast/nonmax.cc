/* This file is adapted from the original FAST source code
   distribution to work with Into data types.
 */

#include "nonmax.h"
#include <PiiGeometricObjects.h>

#define FAST_COMPARE(X, Y) ((X)>=(Y))

PiiMatrix<int> fast_suppress_nonmax(const PiiMatrix<int>& corners,
                                    const QVector<int>& scores)
{
	/*Point above points (roughly) to the pixel above the one of interest, if there
    is a feature there.*/
	int iPointAbove = 0;
	int iPointBelow = 0;

  PiiMatrix<int> matResult(0,2);
  const int iRows = corners.rows();
	if (iRows < 1)
    return matResult;

  matResult.reserve(iRows);

	/* Find where each row begins
	   (the corners are output in raster scan order). A beginning of -1 signifies
	   that there are no corners on that row. */
	int iLastRow = corners(iRows-1, 1);
  QVector<int> vecRowStart(iLastRow+1, -1);
	
  int iPrevRow = -1;
  for (int i=0; i<iRows; ++i)
    if (corners(i,1) != iPrevRow)
      {
        iPrevRow = corners(i,1);
        vecRowStart[iPrevRow] = i;
			}
	
	for (int i=0; i<iRows; ++i)
    {
      int score = scores[i];
      PiiPoint<int> pos = corners.rowAs<PiiPoint<int> >(i);
			
      /*Check left */
      if(i > 0)
        if(corners(i-1,0) == pos.x - 1 && corners(i-1,1) == pos.y && FAST_COMPARE(scores[i-1], score))
          continue;
      
      /*Check right*/
      if (i < (iRows - 1))
        if (corners(i+1,0) == pos.x + 1 && corners(i+1,1) == pos.y && FAST_COMPARE(scores[i+1], score))
          continue;
			
      /*Check above (if there is a valid row above)*/
      if (pos.y != 0 && vecRowStart[pos.y - 1] != -1)
        {
          /*Make sure that current iPointAbove is one
            row above.*/
          if (corners(iPointAbove,1) < pos.y - 1)
            iPointAbove = vecRowStart[pos.y-1];
          
          /*Make iPointAbove point to the first of the pixels above the current point,
            if it exists.*/
          for (; corners(iPointAbove,1) < pos.y && corners(iPointAbove,0) < pos.x - 1; iPointAbove++)
            {}
        
          
          for (int j=iPointAbove; corners(j,1) < pos.y && corners(j,0) <= pos.x + 1; ++j)
            {
              int x = corners(j,0);
              if (x >= pos.x-1 && x <= pos.x+1 && FAST_COMPARE(scores[j], score))
                goto cont;
            }
        }
			
      /*Check below (if there is anything below)*/
      if (pos.y != iLastRow && vecRowStart[pos.y + 1] != -1 && iPointBelow < iRows) /*Nothing below*/
        {
          if (corners(iPointBelow,1) < pos.y + 1)
            iPointBelow = vecRowStart[pos.y+1];
          
          /* Make point below point to one of the pixels belowthe current point, if it
             exists.*/
          for (; iPointBelow < iRows && corners(iPointBelow,1) == pos.y+1 && corners(iPointBelow,0) < pos.x - 1; iPointBelow++)
            {}
          
          for (int j=iPointBelow; j < iRows && corners(j,1) == pos.y+1 && corners(j,0) <= pos.x + 1; j++)
            {
              int x = corners(j,0);
              if (x >= pos.x - 1 && x <= pos.x + 1 && FAST_COMPARE(scores[j],score))
                goto cont;
            }
        }

      matResult.appendRow(corners[i]);
		cont:;
    }
  
  return matResult;
}

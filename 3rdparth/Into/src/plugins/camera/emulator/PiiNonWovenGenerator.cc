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

#include "PiiNonWovenGenerator.h"

#include <PiiMatrix.h>

PiiNonWovenGenerator::PiiNonWovenGenerator() :
  _dRoughness(0),
  _iMeanGrayLevel(127),
  _iBondingPatternSize(0),
  _iBondingPatternStartRow(0),
  _iBondingPatternStartCol(0),
  _productQuality(MediumQuality)
{
  _fractalGenerator.setTargetMean(_iMeanGrayLevel);
  setProductQuality(MediumQuality);
}

void PiiNonWovenGenerator::setMeanGrayLevel(int meanGrayLevel)
{
  _iMeanGrayLevel = meanGrayLevel;
  _fractalGenerator.setTargetMean(_iMeanGrayLevel);
}

void PiiNonWovenGenerator::setProductQuality(ProductQuality productQuality)
{
  _productQuality = productQuality;

  switch(_productQuality)
    {
    case HighQuality:
      _dRoughness = 0.2;
      _fractalGenerator.setRoughnessScale(0.5);
      setMeanGrayLevel(150);
      break;
    case MediumQuality:
      _dRoughness = 0.4;
      _fractalGenerator.setRoughnessScale(0.7);
      setMeanGrayLevel(130);
      break;
    case LowQuality:
      _dRoughness = 0.6;
      _fractalGenerator.setRoughnessScale(0.8);
      setMeanGrayLevel(120);
      break;
    }
}

void PiiNonWovenGenerator::generateTexture(PiiMatrix<unsigned char>& buffer,
                                           int row, int column,
                                           int rows, int columns,
                                           bool first)
{
  int firstRow = 0;

  /* PENDING
   * Insert comment here
   */
  
  PiiMatrix<unsigned char> newBuffer(rows+1,columns);
  if (!first)
    {
      row--; rows++;
      int iRow = row % buffer.rows();
      for (int i=0; i<buffer.columns(); i++)
        newBuffer(0,i) = buffer(iRow,i);
      firstRow = 1;
    }
  
  // If the texture needs to be continuous, take an additional line
  // just before the first generated line so that the fractal
  // generator can glue the new texture to it.
  /*if (!first)
    {
      row--; rows++;
      if (row < 0) row += buffer.rows();
      firstRow = 1;
    }
    unsigned char** scanLines = new unsigned char*[rows];

    // Create scan-lines pointers
    for (int r=0; r<rows; r++)
    scanLines[r] = buffer.row((row + r) % buffer.rows()) + column;
    
    PiiMatrix<unsigned char> newBuffer(new PiiMatrixData<unsigned char>(rows, columns, scanLines));
  */

  int roughnessX = int(_fractalGenerator.scaleRoughness(columns,100) * _dRoughness * 100 + 0.5);
  
  int c1, c2;
  // If we can seed the fractal by previous data, we'll do it.
  if (firstRow == 1)
    {
      c1 = newBuffer(0, column);
      c2 = newBuffer(0, column + columns-1);
    }
  else
    {
      c1 = _iMeanGrayLevel;
      c2 = _iMeanGrayLevel;
    }

  _fractalGenerator.generateFractal(newBuffer,
                                    firstRow, column, rows-firstRow, columns,
                                    c1, c2,
                                    (c1 + _iMeanGrayLevel) >> 1,
                                    (c2 + _iMeanGrayLevel) >> 1,
                                    roughnessX);

  // Generate bonding pattern if requested
  if (_iBondingPatternSize > 0)
    // Leave the last row out so that it doesn't affect the generation
    // of the next fractal.
    generateBondingPattern(newBuffer,
                           0, column,
                           rows-1, columns);
}

void PiiNonWovenGenerator::generateBondingPattern(PiiMatrix<unsigned char>& buffer,
                                                  int row, int column,
                                                  int rows, int columns)
{
  int halfSize = _iBondingPatternSize >> 1;
  int meanLevel3 = _iMeanGrayLevel * 3;
  for (int r = _iBondingPatternStartRow; r < rows; r += _iBondingPatternSize)
    {
      unsigned char* pRow = buffer.row(row+r);
      for (int c = _iBondingPatternStartCol*halfSize; c < columns; c += _iBondingPatternSize)
        {
          pRow[column+c] = (pRow[column+c] + meanLevel3) >> 2;
        // Alternate starting position
        _iBondingPatternStartCol ^= 1;
        }
    }
  _iBondingPatternStartRow = _iBondingPatternSize - (rows - _iBondingPatternStartRow) % _iBondingPatternSize;
}

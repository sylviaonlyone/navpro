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

#include "PiiFiberBundleGenerator.h"
#include <PiiMatrix.h>
#include <PiiMath.h>
#include <PiiRandom.h>

class PiiFiberBundleGenerator::FiberBundle
{
public:
  FiberBundle(PiiFiberBundleGenerator* parent, int startPos) :
    _pParent(parent),
    _iBundleWidth(_pParent->_iBundleWidth),
    _iStartPos(startPos),
    _dStartPos(0),
    _dPosChange(0),
    _dpFiberThickness(new double[_iBundleWidth]),
    _dpThicknessChange(new double[_iBundleWidth]),
    _iLineCount(0),
    _iNextUpdate(rand() % 50)
  {
    double mean = (_pParent->_iMinThickness + _pParent->_iMaxThickness) / 2;
    for (int i=_iBundleWidth; i--; )
      _dpFiberThickness[i] = mean;
    updateChanges();
  }

  ~FiberBundle()
  {
    delete[] _dpFiberThickness;
    delete[] _dpThicknessChange;
  }
  
  void draw(unsigned char* line, int lineWidth)
  {
    // Draw fibers
    for (int i=0; i<_iBundleWidth; ++i)
      {
        int bufferPos = i + _iStartPos + int(_dStartPos);
        if (bufferPos >= 0 && bufferPos < lineWidth)
          {
            // Thickness brings light intensity down
            int newValue = int(line[bufferPos]) - int(_dpFiberThickness[i]);
            if (newValue < 0)
              newValue = 0;
            else if (newValue > 255)
              newValue = 255;
            line[bufferPos] = static_cast<unsigned char>(newValue);
          }
      }
  }

  void updateChanges()
  {
    _dPosChange = Pii::uniformRandom() * _pParent->_dElasticity - _pParent->_dElasticity/2;
    double halfThickness = _pParent->_dThicknessFluctuation/2;
    for (int i=0; i<_iBundleWidth; ++i)
      _dpThicknessChange[i] = Pii::uniformRandom() * _pParent->_dThicknessFluctuation - halfThickness;
  }

  void update()
  {
    // Time to update fiber movement
    if (_iLineCount == _iNextUpdate)
      {
        _iLineCount = 0;
        _iNextUpdate = rand() % 50;
        updateChanges();
      }
    else
      ++_iLineCount;
    
    // Update horizontal position
    _dStartPos += _dPosChange;
    if (_dStartPos < -_pParent->_iMaxMovement)
      _dStartPos = -_pParent->_iMaxMovement;
    else if (_dStartPos > _pParent->_iMaxMovement)
      _dStartPos = _pParent->_iMaxMovement;

    // Update individual fiber thickness values
    for (int i=0; i<_iBundleWidth; ++i)
      {
        _dpFiberThickness[i] += _dpThicknessChange[i];
        if (_dpFiberThickness[i] < _pParent->_iMinThickness)
          _dpFiberThickness[i] = _pParent->_iMinThickness;
        else if (_dpFiberThickness[i] > _pParent->_iMaxThickness)
          _dpFiberThickness[i] = _pParent->_iMaxThickness;
      }
  }
  
private:
  PiiFiberBundleGenerator* _pParent;
  int _iBundleWidth;
  int _iStartPos;
  double _dStartPos;
  double _dPosChange;
  double* _dpFiberThickness;
  double* _dpThicknessChange;
  int _iLineCount, _iNextUpdate;
};


PiiFiberBundleGenerator::PiiFiberBundleGenerator() :
  _iBundleWidth(200),
  _iBundleCount(8),
  _dElasticity(0.05),
  _iMaxMovement(15),
  _dThicknessFluctuation(0.3),
  _iMinThickness(60),
  _iMaxThickness(90),
  _bSmooth(true)
{
}

PiiFiberBundleGenerator::~PiiFiberBundleGenerator()
{
  qDeleteAll(_lstBundles);
}


void PiiFiberBundleGenerator::generateTexture(PiiMatrix<unsigned char>& buffer,
                                              int row, int column,
                                              int rows, int columns,
                                              bool first)
{
  // On first round initialize fiber bundles
  if (first)
    {
      qDeleteAll(_lstBundles);
      _lstBundles.clear();
      for (int i=0; i<_iBundleCount; ++i)
        _lstBundles << new FiberBundle(this, i*columns/_iBundleCount);
    }

  unsigned char** ppScanLines = new unsigned char*[rows];

  // Fill the buffer line by line and store scan-line pointers at the
  // same time.
  for (int r=0; r<rows; ++r)
    {
      ppScanLines[r] = buffer[(row + r) % buffer.rows()] + column;
      generateLine(ppScanLines[r], columns);
    }

  if (_bSmooth)
    {
      PiiMatrix<unsigned char> newBuffer(rows, columns, *ppScanLines);
      newBuffer = Pii::movingAverage<int>(Pii::movingAverage<int>(newBuffer, 3,
                                                                  Pii::Vertically,
                                                                  Pii::ShrinkWindow),
                                          3,
                                          Pii::Horizontally,
                                          Pii::ShrinkWindow);          
    }
  else
    delete[] ppScanLines;
}


void PiiFiberBundleGenerator::generateLine(unsigned char* line, int width)
{
  // In back lighting, everything is white unless something gets in
  // between the camera and the light.
  memset(line, 255, width);
  // Draw bundles on top of each other
  for (int i=0; i<_iBundleCount; ++i)
    {
      _lstBundles[i]->update();
      _lstBundles[i]->draw(line, width);
    }
}

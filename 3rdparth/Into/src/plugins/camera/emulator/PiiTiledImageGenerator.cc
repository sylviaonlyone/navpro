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


#include "PiiTiledImageGenerator.h"

PiiTiledImageGenerator::PiiTiledImageGenerator() : _iRowCounter(0),
                                                   _iDirection(1),
                                                   _pImage(0),
                                                   _strTileFileName("")
                                                   
{
  qDebug("PiiTiledImageGenerator()");
}

PiiTiledImageGenerator::~PiiTiledImageGenerator()
{
  delete _pImage;
}

void PiiTiledImageGenerator::generateTexture(PiiMatrix<unsigned char>& buffer,
                                             int row, int column,
                                             int rows, int columns,
                                             bool first)
{
  if (_pImage != 0)
    {
      int iColumns = qMin(_pImage->columns(), buffer.columns());
      
      for (int r=0; r<rows; r++ )
        {
          for (int c=0; c<iColumns; c++)
            buffer(r+row,c+column) = (*_pImage)(_iRowCounter,c);

          _iRowCounter += _iDirection;
          if (_iRowCounter <= 0 || _iRowCounter >= (_pImage->rows()-1))
            _iDirection = -_iDirection;
        }
    }
  else
    {
      buffer(row,column,rows,columns) = 128;
    }
}

void PiiTiledImageGenerator::setTileFileName(const QString& tileFileName)
{
  _strTileFileName = tileFileName;
  initImage();
}

void PiiTiledImageGenerator::initImage()
{
  if (QFile::exists(_strTileFileName))
    {
      delete _pImage;
      _pImage = PiiImageFileReader::readGrayImage(_strTileFileName)->toMatrixPointer();
    }
  else
    piiWarning(tr("Image file '%1' doesn't exists.").arg(_strTileFileName));
}

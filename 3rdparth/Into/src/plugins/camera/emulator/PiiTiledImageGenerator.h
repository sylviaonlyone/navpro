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

#ifndef _PIITILEDIMAGEGENERATOR_H
#define _PIITILEDIMAGEGENERATOR_H

#include "PiiTextureGenerator.h"
#include <PiiMatrix.h>
#include <PiiImageFileReader.h>

/**
 * A texture generator that produces tiled image background
 * texture.
 *
 * @ingroup PiiCameraPlugin
 */
class PII_CAMERAEMULATOR_EXPORT PiiTiledImageGenerator : public PiiTextureGenerator
{
  Q_OBJECT

  /**
   * tileFileName description
   */
  Q_PROPERTY(QString tileFileName READ getTileFileName WRITE setTileFileName);
  
  
public:
  PiiTiledImageGenerator();
  ~PiiTiledImageGenerator();
  
  void generateTexture(PiiMatrix<unsigned char>& buffer,
                       int row, int column,
                       int rows, int columns,
                       bool first = false);

  void setTileFileName(const QString& tileFileName);
  QString getTileFileName() const { return _strTileFileName; }


private:
  void initImage();
  QImage convertColorToGray(QImage img);
  
  int _iRowCounter, _iDirection;
  PiiMatrix<unsigned char> *_pImage;
  QString _strTileFileName;
};


#endif //_PIITILEDIMAGEGENERATOR_H

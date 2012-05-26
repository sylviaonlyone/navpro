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

#ifndef _PIITEXTUREGENERATOR_H
#define _PIITEXTUREGENERATOR_H

#include <QObject>
#include <PiiMatrix.h>
#include <PiiCameraEmulatorGlobal.h>

/**
 * An interface for classes that produce background texture for
 * PiiLineScanEmulator. You may create your own generator by
 * subclassing this interface. Register the class to PiiEngine's
 * registry (@ref PiiEngine::getRegistry()) and set the @ref
 * PiiLineScanEmulator::textureGenerator "textureGenerator" property
 * in PiiLineScanEmulator to the name of your class.
 *
 * @ingroup PiiCameraPlugin
 */
class PII_CAMERAEMULATOR_EXPORT PiiTextureGenerator : public QObject
{
  Q_OBJECT

public:
  /**
   * Generate texture to the given frame buffer.
   *
   * @param buffer the image buffer
   *
   * @param row start generating at this row
   *
   * @param col start generating at this column
   *
   * @param rows the number of rows to generate
   *
   * @param columns the number of columns to generate
   *
   * @param first if @p false, the texture should form a contiuous
   * surface with previous data at @p row-1.
   *
   * @note Access the buffer in a circular manner. When the frame
   * buffer in PiiLinescanEmulator is full, it will be filled from the
   * beginning again. The emulator will not do the circular indexing
   * for you. Thus, it may happen that @p row points to the last row
   * in @p buffer while @p rows is larger than one.
   *
   * @code
   * for (int r=0; r<rows; ++r)
   *   generateLine(buffer[(row + r) % buffer.rows()] + column);
   * @endcode
   */
  virtual void generateTexture(PiiMatrix<unsigned char>& buffer,
                               int row, int column,
                               int rows, int columns,
                               bool first = false) = 0;
};

#endif //_PIITEXTUREGENERATOR_H

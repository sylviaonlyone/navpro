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

#ifndef _PIINONWOVENGENERATOR_H
#define _PIINONWOVENGENERATOR_H

#include "PiiTextureGenerator.h"
#include <PiiCloudFractalGenerator.h>

/**
 * A texture generator that produces non-woven-like background
 * texture.
 *
 * @ingroup PiiCameraPlugin
 */
class PII_CAMERAEMULATOR_EXPORT PiiNonWovenGenerator : public PiiTextureGenerator
{
  Q_OBJECT

  /**
   * The mean gray level the generator will try to produce. This value
   * is automatically set when #productQuality is changed, but you may
   * also set it manually.
   */
  Q_PROPERTY(int meanGrayLevel READ getMeanGrayLevel WRITE setMeanGrayLevel);

  /**
   * The spacing of bonding pattern grid in pixels. The default value
   * is 0, which disables the generation of a bonding pattern.
   */
  Q_PROPERTY(int bondingPatternSize READ getBondingPatternSize WRITE setBondingPatternSize);

  /**
   * The quality of the product. Use this property to set the overall
   * look of the generate texture.
   */
  Q_PROPERTY(ProductQuality productQuality READ getProductQuality WRITE setProductQuality);
  Q_ENUMS(ProductQuality);
  
public:
  /**
   * The quality of the non-woven product overall.
   *
   * @lip HighQuality - smooth, white surface
   *
   * @lip MediumQuality - somewhat cloudy surface
   *
   * @lip LowQuality - ragged, very cloudy surface
   */
  enum ProductQuality { HighQuality, MediumQuality, LowQuality };
  
  PiiNonWovenGenerator();
  
  void generateTexture(PiiMatrix<unsigned char>& buffer,
                       int row, int column,
                       int rows, int columns,
                       bool first = false);

  void setMeanGrayLevel(int meanGrayLevel);
  int getMeanGrayLevel() const { return _iMeanGrayLevel; }
  void setBondingPatternSize(int bondingPatternSize) { _iBondingPatternSize = bondingPatternSize; }
  int getBondingPatternSize() const { return _iBondingPatternSize; }
  void setProductQuality(ProductQuality productQuality);
  ProductQuality getProductQuality() const { return _productQuality; }

private:
  void generateBondingPattern(PiiMatrix<unsigned char>& buffer,
                              int row, int column,
                              int rows, int columns);
  double _dRoughness;
  int _iMeanGrayLevel;
  int _iBondingPatternSize;
  int _iBondingPatternStartRow, _iBondingPatternStartCol;
  ProductQuality _productQuality;
  PiiCloudFractalGenerator _fractalGenerator;
};

#endif //_PIINONWOVENGENERATOR_H

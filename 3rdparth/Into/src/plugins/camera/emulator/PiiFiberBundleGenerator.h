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

#ifndef _PIIFIBERBUNDLEGENERATOR_H
#define _PIIFIBERBUNDLEGENERATOR_H

#include "PiiTextureGenerator.h"

/**
 * A texture generator that produces a texture composed of bundles of
 * infinitely long fibers (such as nylon) seen in transmitted light
 * (back-lighting).
 *
 * @ingroup PiiCameraEmulatorPlugin
 */
class PII_CAMERAEMULATOR_EXPORT PiiFiberBundleGenerator : public PiiTextureGenerator
{
  Q_OBJECT

  /**
   * The width of an individual fiber bundle or "band", in pixels. The
   * default value is 200.
   */
  Q_PROPERTY(int bundleWidth READ getBundleWidth WRITE setBundleWidth);

  /**
   * The total number of fiber bundles. The default value is 8.
   */
  Q_PROPERTY(int bundleCount READ getBundleCount WRITE setBundleCount);

  /**
   * The maximum amount of pixels a fiber bundle can move horizontally
   * between two scan lines. The default value is 0.05. The smaller
   * the value the straighter the fibers.
   */
  Q_PROPERTY(double elasticity READ getElasticity WRITE setElasticity);

  /**
   * The maximum number of pixels a fiber bundle can deviate from its
   * original position either left or right. The default value is 15.
   */
  Q_PROPERTY(int maxMovement READ getMaxMovement WRITE setMaxMovement);

  /**
   * The maximum allowed change in fiber thickness between two scan
   * lines. The default value is 0.3 (gray levels).
   */
  Q_PROPERTY(double thicknessFluctuation READ getThicknessFluctuation WRITE setThicknessFluctuation);

  /**
   * The minimum thickness of a fiber bundle. The default value is 60.
   */
  Q_PROPERTY(int minThickness READ getMinThickness WRITE setMinThickness);

  /**
   * The maximum thickness of a fiber bundle. The default value is 90.
   */
  Q_PROPERTY(int maxThickness READ getMaxThickness WRITE setMaxThickness);
  
  /**
   * Smoothing flag. If set to @p true (the default), the generated
   * texture will be run through a low-pass filter.
   */
  Q_PROPERTY(bool smooth READ getSmooth WRITE setSmooth);

public:
  PiiFiberBundleGenerator();
  ~PiiFiberBundleGenerator();

  void setBundleWidth(int bundleWidth) { _iBundleWidth = bundleWidth; }
  int getBundleWidth() const { return _iBundleWidth; }
  void setBundleCount(int bundleCount) { _iBundleCount = bundleCount; }
  int getBundleCount() const { return _iBundleCount; }
  void setElasticity(double elasticity) { _dElasticity = elasticity; }
  double getElasticity() const { return _dElasticity; }
  void setMaxMovement(int maxMovement) { _iMaxMovement = maxMovement; }
  int getMaxMovement() const { return _iMaxMovement; }
  void setMinThickness(int minThickness) { _iMinThickness = minThickness; }
  int getMinThickness() const { return _iMinThickness; }
  void setMaxThickness(int maxThickness) { _iMaxThickness = maxThickness; }
  int getMaxThickness() const { return _iMaxThickness; }
  void setThicknessFluctuation(double thicknessFluctuation) { _dThicknessFluctuation = thicknessFluctuation; }
  double getThicknessFluctuation() const { return _dThicknessFluctuation; }
  void setSmooth(bool smooth) { _bSmooth = smooth; }
  bool getSmooth() const { return _bSmooth; }

  void generateTexture(PiiMatrix<unsigned char>& buffer,
                       int row, int column,
                       int rows, int columns,
                       bool first = false);
private:
  class FiberBundle;

  void generateLine(unsigned char* line, int width);
  
  int _iBundleWidth;
  int _iBundleCount;
  double _dElasticity;
  int _iMaxMovement;
  double _dThicknessFluctuation;
  int _iMinThickness;
  int _iMaxThickness;
  bool _bSmooth;

  QList<FiberBundle*> _lstBundles;
};



#endif //_PIIFIBERBUNDLEGENERATOR_H

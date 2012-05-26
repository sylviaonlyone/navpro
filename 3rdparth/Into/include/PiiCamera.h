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

#ifndef _PIICAMERA_H
#define _PIICAMERA_H

/**
 * Camera-related utility functions and definitions.
 *
 * @ingroup PiiCameraPlugin
 */
namespace PiiCamera
{
  /**
   * Supported image formats.
   *
   * @lip InvalidFormat - the image format is unrecognized or not yet
   * known.
   *
   * @lip MonoFormat - the image is monochrome (gray-scale)
   *
   * @lip BayerRGGBFormat - the image is Bayer-encoded in BGGR color
   * order.
   *
   * @lip BayerGBRGFormat - the image is Bayer-encoded in GBRG color
   * order.
   *
   * @lip BayerGRBGFormat - the image is Bayer-encoded in GRBG color
   * order.
   *
   * @lip RgbFormat - the image is in RGB format
   *
   * @lip BgrFormat - the image is in BGR format.
   */
  enum ImageFormat
    {
      InvalidFormat = 0,
      MonoFormat = 1,
      BayerRGGBFormat,
      BayerBGGRFormat,
      BayerGBRGFormat,
      BayerGRBGFormat,
      RgbFormat = 16,
      BgrFormat
    };

  /**
   * Supported camera types.
   *
   * @lip AreaScan - the camera type is area scan.
   *
   * @lip LineScan - the camera type is line scan.
   */
  enum CameraType
    {
      AreaScan = 0,
      LineScan
    };
}

#endif //_PIICAMERA_H

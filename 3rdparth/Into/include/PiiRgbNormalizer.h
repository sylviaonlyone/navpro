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

#ifndef _PIIRGBNORMALIZER_H
#define _PIIRGBNORMALIZER_H

#include <PiiDefaultOperation.h>

/**
 * An operation that converts images to normalized RGB. See @ref
 * PiiColors::normalizedRgb() for details.
 *
 * @inputs
 *
 * @in image - any color image
 * 
 * @outputs
 *
 * @out channel0 - first normalized color channel (red, by default)
 *
 * @out channel1 - second normalized color channel (green, by default)
 *
 * @ingroup PiiColorsPlugin
 */
class PiiRgbNormalizer : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The maximum value a normalized color channel can attain. The
   * default is 255. This allows simultaneous quantization and
   * normalization.
   */
  Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue);

  /**
   * The normalized color channels the operation will emit through @p
   * channel0 and @p channel1.
   */
  Q_PROPERTY(ChannelPair channels READ channels WRITE setChannels);
  Q_ENUMS(ChannelPair);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  /**
   * Allowed channel pairs. 
   *
   * @lip RedGreen - normalized RG. @p channel0 will emit the red
   * channel and @p channel1 the green channel.
   *
   * @lip RedBlue - normalized RB
   *
   * @lip GreenBlue - normalized GB
   */
  enum ChannelPair { RedGreen, RedBlue, GreenBlue };
  
  PiiRgbNormalizer();

  void setMaxValue(double maxValue);
  double maxValue() const;

  void setChannels(const ChannelPair& channels);
  ChannelPair channels() const;

protected:
  void process();

private:
  template <class T> void normalizedRgb(const PiiVariant& obj);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    double dMaxValue;
    ChannelPair channels;
  };
  PII_D_FUNC;
};


#endif //_PIIRGBNORMALIZER_H

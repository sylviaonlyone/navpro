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

#ifndef _PIICOLORCHANNELSPLITTER_H
#define _PIICOLORCHANNELSPLITTER_H

#include "PiiDefaultOperation.h"

/**
 * An operation that splits color images into channels. The operation
 * reads in any color image type and outputs three to four separated
 * channel images on different outputs.
 *
 * @inputs
 *
 * @in image - a color image
 *
 * @outputs
 *
 * @out channelX - color channel X as an intensity image. @p
 * channel0 is for the red color channel, @p channel1 for green, and
 * @p channel2 for blue. If the input image has an alpha channel, it
 * is emitted from @p channel3.
 *
 * @ingroup PiiColorsPlugin
 */
class PiiColorChannelSplitter : public PiiDefaultOperation
{
  Q_OBJECT

  PII_OPERATION_SERIALIZATION_FUNCTION;

public:
  PiiColorChannelSplitter();
  ~PiiColorChannelSplitter();
  
protected:
  void process();

private:
  template <class Color, int channels> void splitChannels(const PiiVariant& obj);
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiInputSocket* pImageInput;
  };
  PII_D_FUNC;

};

#endif //_PIICOLORCHANNELSPLITTER_H

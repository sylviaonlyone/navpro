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

#ifndef _PIIHISTOGRAMBACKPROJECTOR_H
#define _PIIHISTOGRAMBACKPROJECTOR_H

#include <PiiDefaultOperation.h>
#include <QPair>
#include <PiiMatrix.h>

/**
 * Perform histogram back-projection. In histogram back-projection,
 * the pixels in an input image are replaced with values in a @e model
 * histogram.
 *
 * @see PiiImage::backProject().
 *
 * @inputs
 *
 * @in image - input image. Any integer-valued one-channel (i.e. 
 * gray-level) image.
 *
 * @in channel0 - alias for @p image. More intuitive to the programmer
 * with two-dimensional backprojection.
 *
 * @in channel1 - optional input that is used only with
 * two-dimensional backprojection.
 *
 * @in model - optional input for a changing model histogram. 
 * Overrides #model if connected.
 * 
 * @outputs
 *
 * @out image - backprojected image. Type depends on the model.
 *
 * @ingroup PiiImagePlugin
 */
class PiiHistogramBackProjector : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * The model histogram. Any matrix will do. The output type is
   * determined by the type of the model histogram. Typically, the
   * model is a PiiMatrix<int> or a PiiMatrix<bool>.
   */
  Q_PROPERTY(PiiVariant model READ model WRITE setModel);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiHistogramBackProjector();

  /**
   * Maps @p channel0 to @p image.
   */
  PiiInputSocket* input(const QString& name) const;

  void setModel(const PiiVariant& model);
  PiiVariant model() const;

protected:
  void process();
  void check(bool reset);

private:
  template <class T> void backProject(const PiiVariant& obj1, const PiiVariant& obj2);
  template <class T, class U> void backProject(const PiiMatrix<U>& ch1, const PiiMatrix<U>& ch2);
  template <class T, class U> void backProject(const PiiMatrix<U>& image);

  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    PiiVariant varModel, varTmpModel;
    bool bChannel2Connected, bModelConnected;
  };
  PII_D_FUNC;
};


#endif //_PIIHISTOGRAMBACKPROJECTOR_H

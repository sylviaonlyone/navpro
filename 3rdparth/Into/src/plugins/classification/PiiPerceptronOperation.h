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

#ifndef _PIIPERCEPTRONOPERATION_H
#define _PIIPERCEPTRONOPERATION_H

#include "PiiPerceptron.h"
#include <PiiClassifierOperation.h>

/**
 * An Ydin-compatible Perceptron classifier operation. Perceptron
 * is a supervised learning machine. Thus,
 * it does have a class label input. See the
 * description of input and output sockets in PiiClassifierOperation.
 * 
 * To instantiate the operation from an object registry, one must
 * specify the data type, e.g "PiiPerceptronOperation<double>". 
 *
 * @see PiiClassifierOperation
 *
 * @ingroup PiiClassificationPlugin
 */
class PiiPerceptronOperation : public PiiClassifierOperation
{
  Q_OBJECT

public:
  template <class SampleSet> class Template;

  PiiPerceptronOperation();
};

#include "PiiPerceptronOperation-templates.h"

#endif // _PIIPERCEPTRONOPERATION_H 

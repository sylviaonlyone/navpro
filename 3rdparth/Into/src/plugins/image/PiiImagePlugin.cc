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

#include "PiiPlugin.h"

//Basic image handling
#include "PiiImageFileReader.h"
#include "PiiImageFileWriter.h"
#include "PiiImageSplitter.h"
#include "PiiImageCropper.h"
#include "PiiImagePieceJoiner.h"
#include "PiiEdgeDetector.h"
#include "PiiBackgroundExtractor.h"
#include "PiiImageAnnotator.h"
#include "PiiImageScaleOperation.h"
#include "PiiImageRotationOperation.h"
#include "PiiImageFilterOperation.h"
#include "PiiCornerDetector.h"

//Histograms 
#include "PiiHistogramOperation.h"
#include "PiiDefaultOperation.h"
#include "PiiQuantizerOperation.h"
#include "PiiHistogramBackProjector.h"
#include "PiiHistogramEqualizer.h"

//Binary
#include "PiiThresholdingOperation.h"
#include "PiiMorphologyOperation.h"
#include "PiiLabelingOperation.h"
#include "PiiObjectPropertyExtractor.h"
#include "PiiFeatureRangeLimiter.h"
#include "PiiMaskGenerator.h"
#include "PiiBoundaryFinderOperation.h"

// Other
#include "PiiImageUnwarpOperation.h"

PII_IMPLEMENT_PLUGIN(PiiImagePlugin);

//Basic image handling
PII_REGISTER_OPERATION(PiiImageFileReader);
PII_REGISTER_OPERATION(PiiImageFileWriter);
PII_REGISTER_OPERATION(PiiImageSplitter);
PII_REGISTER_OPERATION(PiiImageCropper);
PII_REGISTER_OPERATION(PiiImagePieceJoiner);
PII_REGISTER_OPERATION(PiiEdgeDetector);
PII_REGISTER_OPERATION(PiiBackgroundExtractor);
PII_REGISTER_OPERATION(PiiImageAnnotator);
PII_REGISTER_OPERATION(PiiImageScaleOperation);
PII_REGISTER_OPERATION(PiiImageRotationOperation);
PII_REGISTER_OPERATION(PiiImageFilterOperation);
PII_REGISTER_OPERATION(PiiCornerDetector);

//Histograms
PII_REGISTER_OPERATION(PiiHistogramOperation);
PII_REGISTER_OPERATION(PiiQuantizerOperation);
PII_REGISTER_OPERATION(PiiHistogramBackProjector);
PII_REGISTER_OPERATION(PiiHistogramEqualizer);

//Binary
PII_REGISTER_OPERATION(PiiThresholdingOperation);
PII_REGISTER_OPERATION(PiiMorphologyOperation);
PII_REGISTER_OPERATION(PiiLabelingOperation);
PII_REGISTER_OPERATION(PiiObjectPropertyExtractor);
PII_REGISTER_OPERATION(PiiFeatureRangeLimiter);
PII_REGISTER_OPERATION(PiiMaskGenerator);
PII_REGISTER_OPERATION(PiiBoundaryFinderOperation);

//Other
PII_REGISTER_OPERATION(PiiImageUnwarpOperation);

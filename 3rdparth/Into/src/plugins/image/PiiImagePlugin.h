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

/**
 * @defgroup PiiImagePlugin
 *
 * The image plug-in contains operations and functions image
 * processing and for reading and writing images.
 *
 * @par Region of Interest
 *
 * Region of interest (ROI for short) is a way of telling image
 * processing operations the parts of an image that need to be
 * processed. On the lowest level, ROIs are implemented as function
 * objects that return @p true for each pixel that needs to be
 * analyzed and @p false for the others. The actual implementation of
 * a ROI can vary; the only requirement is that the ROI class has the
 * () operator defined for two integer arguments (row and column
 * coordinates of a pixel, in this order). For example, PiiMatrix is a
 * valid ROI.
 *
 * Image processing operations use the ROI as a template. With
 * compiler optimizations turned on, using the default ROI
 * (PiiImage::DefaultRoi) imposes no computational overhead.
 *
 * ROIs are used with many image processing and analysis operations.
 * Some examples:
 *
 * @code
 * // Calculate histogram over an elliptical area that just fits into an image
 * PiiMatrix<int> histogram =
 *   PiiHistogram::histogram(image,
 *                           PiiBinary::createMask(PiiBinary::Elliptical,
 *                                                 image.rows(),
 *                                                 image.columns()));
 *
 * // Calculate LBP histogram of an arbitrary set of pixels (mask is a binary matrix)
 * PiiMatrix<int> lbpHistogram = PiiLbp::basicLbp<PiiLbp::Histogram>(image, mask);
 * @endcode
 *
 * Many analysis operations (PiiLbpOperation, PiiHistogramOperation)
 * have an optional @p roi input that accepts all integer-valued
 * matrices as input. Best performance is achieved with 8-bit data
 * types such as @p bool and @p (unsigned) @p char. The size of the
 * ROI mask read from this input must match the size of the image
 * currently in analysis.
 *
 * Additionally, the operations are able to handle rectangular ROIs. 
 * In a rectangular ROI, the interesting region is represented by a
 * set of rectangular areas. The rectangles are stored into an N-by-4
 * PiiMatrix<int> in which each row describes a rectangle as (x, y,
 * width, height). PiiImage::RoiType is used to select between these
 * representations of a ROI.
 *
 * Operations with a @p roi input support both mask and rectangle
 * ROIs. By default, the @p roiType property of such an operation is
 * set to @p AutoRoi. In this mode, the type of the ROI is determined
 * by the input: a N-by-4 PiiMatrix<int> is treated as a rectangular
 * ROI, and all others as a mask roi. If the @p roiType property is
 * set to @p MaskRoi, and the @p roi input is not connected, the alpha
 * channel of a four-channel color image will be used as a ROI mask.
 *
 * Note that the use of a ROI may change the behaviour of a feature
 * vector. For example, the sum of a histogram changes with different
 * regions of interest. This must be taken into account if the
 * features are used in classification.
 *
 * @dependencies PiiImagePlugin
 *
 * @runtime PiiDspPlugin
 * Provides basic signal processing operations
 * such as convolution, correlation and Fourier transform.
 */

/**
 * @namespace PiiImage
 * @ingroup PiiImagePlugin
 *
 * @brief Definitions and operations for handling images.
 *
 * This namespace contains functions and definitions for creating
 * digital image filters and for applying them to images,
 * thresholding, labeling binary images, connected component analysis,
 * edge detection, and histogram handling.
 */

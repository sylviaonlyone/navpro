/* This file is part of Into.
 * Copyright 2003-2010 Intopii.
 * All rights reserved.
 */

#ifndef _PIIDIGITEXTRACTOR_H
#define _PIIDIGITEXTRACTOR_H

#include <PiiDefaultOperation.h>
#include <PiiKnnClassifier.h>

/**
 * Recognizes hand-written digits in gray-level images.
 *
 * The operation first segments out individual digits and scales them
 * to a fixed size (20-by-20). Then, PCA is used to reduce the
 * original dimensionality of the image (400) to 50. The PCA
 * coefficients are used as the feature vector, which is classified
 * using a k-NN classifier.
 *
 * @inputs
 *
 * @in image - a gray-level input image. There can be multiple digits
 * in one image, but they are assumed to be on the same row. All
 * digits should be written in dark (i.e. black) pen on lighter
 * background (i.e. white paper).
 *
 * @outputs
 *
 * @out number - the recognized number as an integer
 *
 * @out digitstring - a concatenated string of found digits
 *
 * @out digits - a 1xN row matrix storing each found digit
 *
 * @out boundingboxes - an Nx4 PiiMatrix<int> storing locations of
 * each found digit in bounding box format (x, y, width, height)
 *
 * @ingroup PiiOcrPlugin
 */
class PiiDigitExtractor: public PiiDefaultOperation
{
  Q_OBJECT

  PII_OPERATION_SERIALIZATION_FUNCTION
public:
  PiiDigitExtractor();

  void check(bool reset);

protected:
  void process();

private:
  template <class T> void extractIntDigits(const PiiVariant& obj);
  template <class T> void extractDigits(const PiiMatrix<T>& image);
  void createBlob(PiiMatrix<int> &boundingBoxes);
  bool isIncorrectBlob(const PiiMatrix<int> &boundingBoxes, int index,int imageHeight);
  void initializeKnnClassifier();
  int classify(const PiiMatrix<int> &scaledImage);

  /// @internal
  class Data : public PiiDefaultOperation::Data
    {
    public:
      Data();
      PiiInputSocket *pImageInput;
      PiiOutputSocket *pNumberOutput, *pDigitsInStringOutput, *pDigitsOutput, *pBoundingBoxesOutput;
      int iCombineYThreshold;
      int iCombineXThreshold;
      double dSizeThreshold;
      double dLocationThreshold;
      PiiKnnClassifier<PiiMatrix<float> > digitClassifier;
      PiiMatrix<float> matMeanDigitVector; //(400,1);
      PiiMatrix<float> matBaseDigitVectors; //(50,400);
    };
  PII_D_FUNC;
};


#endif // _PIIDIGITEXTRACTOR_H

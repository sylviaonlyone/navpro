/* This file is part of Into.
 * Copyright 2003-2010 Intopii.
 * All rights reserved.
 */

#include "PiiDigitExtractor.h"
#include <PiiYdinTypes.h>
#include <PiiThresholding.h>
#include <PiiObjectProperty.h>
#include <PiiLabeling.h>
#include <PiiFunctional.h>
#include <PiiMorphology.h>
#include <QString>
#include <PiiSampleSet.h>
#include <PiiImage.h>
#include <functional>
#include <PiiBinaryInputArchive.h>

using namespace Pii;
using namespace PiiYdin;

PiiDigitExtractor::Data::Data() :
    iCombineYThreshold(6),iCombineXThreshold(2), dSizeThreshold(0.2), dLocationThreshold(0.6)
{
}

PiiDigitExtractor::PiiDigitExtractor() :
    PiiDefaultOperation(new Data, Threaded)
{
  PII_D;
  addSocket(d->pImageInput = new PiiInputSocket("image"));

  addSocket(d->pNumberOutput = new PiiOutputSocket("number"));
  addSocket(d->pDigitsInStringOutput = new PiiOutputSocket("digitstring"));
  addSocket(d->pDigitsOutput = new PiiOutputSocket("digits"));
  addSocket(d->pBoundingBoxesOutput = new PiiOutputSocket("boundingboxes"));
}

void PiiDigitExtractor::check(bool reset)
{
  PiiDefaultOperation::check(reset);

  try
    {
      initializeKnnClassifier();
    }
  catch (PiiSerializationException& ex)
    {
      PII_THROW(PiiExecutionException, ex.message());
    }
}

void PiiDigitExtractor::process()
{
  PII_D;
  PiiVariant obj = readInput();

  switch (obj.type())
    {
      PII_INT_GRAY_IMAGE_CASES(extractIntDigits, obj);
    default:
      PII_THROW_UNKNOWN_TYPE(d->pImageInput);
    }
}

template <class T> void PiiDigitExtractor::extractIntDigits(const PiiVariant& obj)
{
  extractDigits(obj.valueAs<PiiMatrix<T> >());
}

//This makes everything:
//Threshold image, segment into blobs, combines and prunes blobs into single digits,
//recenters and scales digit images, and finally classifies these
template <class T> void PiiDigitExtractor::extractDigits(const PiiMatrix<T>& image)
{
  PII_D;
  QString sDigitsString("");
  PiiMatrix<int> matDigits(1,0);
  PiiMatrix<int> matBoundingBoxes;
  PiiMatrix<int> matThresholded;
  PiiMatrix<int> matLabels;
  int iWindowSize = image.rows()/6;
  int iLabelCount = 0;

  if (image.rows() < 6 || image.columns() < 6)
    {
      d->pNumberOutput->emitObject(0);
      d->pDigitsInStringOutput->emitObject(sDigitsString);
      d->pDigitsOutput->emitObject(matDigits);
      d->pBoundingBoxesOutput->emitObject(PiiMatrix<int>(0,4));
      return;
    }
  // Threshold input grayscale image using adaptive thresholding
  matThresholded = PiiImage::adaptiveThreshold(image, PiiImage::InverseThresholdFunction<int>(), 0.95, 0.0, 65,iWindowSize);

  // Remove noise by morphological opening operation
  PiiMatrix<int> matOpenMask(2,2);
  PiiImage::createMask(PiiImage::RectangularMask, matOpenMask);
  matThresholded = PiiImage::open(matThresholded, matOpenMask);

  //Connected components analysis to detect single digits
  matLabels = PiiImage::labelImage(matThresholded,
                                   std::bind2nd(std::equal_to<int>(),1),
                                   std::bind2nd(std::equal_to<int>(),1),
                                   PiiImage::Connect8,
                                   1,
                                   &iLabelCount);

  // Find each connected component and calculate properties (bounding boxes etc.) for these
  PiiMatrix<int> matAreas;
  PiiMatrix<int> matCentroids;

  PiiImage::calculateProperties(matLabels, iLabelCount, matAreas,
                                matCentroids, matBoundingBoxes);


  //So far we have number of bounding boxes. Not all of these contain exactly one digit. Next we combine and prune these.
  QVector<bool> vecProcessedBlobs(iLabelCount);
  vecProcessedBlobs  = vecProcessedBlobs.fill(false);
  PiiMatrix<int> matBB(0,4);
  matBB.reserve(32);

  //This one combines closely located bounding boxes into blobs
  createBlob(matBoundingBoxes);
  PiiMatrix<int> matBB2(0,4);
  matBB2.reserve(32);

  //Here we prune the blobs that are fully inside another, or if they are very small or in incorrect location
  for (int i = 0; i < matBoundingBoxes.rows();i++)
    {
      if (!isIncorrectBlob(matBoundingBoxes,i,image.rows()))
        matBB2.appendRow(matBoundingBoxes[i]);
    }

  //now each row of matBB2 should contain bounding box for one digit
  Pii::sortRows(matBB2,0);

  PiiMatrix<int> matScaledDigit;

  int iNumber = 0;
  //Classify each digit
  for (int nbr=0; nbr<matBB2.rows(); nbr++)
    {
      //Extract digit from thresholded image in center of image "matDigit"
      int size = Pii::max(matBB2(nbr,2),matBB2(nbr,3));
      PiiMatrix<int> matDigit(size,size);
      matDigit=0;
      int xskip=(size-matBB2(nbr,2))/2;
      int yskip=(size-matBB2(nbr,3))/2;

      for (int y=0;y<matBB2(nbr,3);y++)
        for (int x=0;x<matBB2(nbr,2);x++)
          matDigit(y+yskip,x+xskip) = matThresholded(matBB2(nbr,1)+y,matBB2(nbr,0)+x)*255;

      //Scale this one into 20x20 image
      matScaledDigit = PiiImage::scale(matDigit, 20, 20);

      //Classify image matScaledDigit
      int label = classify(matScaledDigit);
      sDigitsString.append(QString::number(label));
      matDigits.appendColumn(PiiMatrix<int> (1,1,label));
      iNumber += Pii::pow(10, matBB2.rows()-nbr-1) * label;
    }

  d->pNumberOutput->emitObject(iNumber);
  d->pDigitsInStringOutput->emitObject(sDigitsString);
  d->pDigitsOutput->emitObject(matDigits);
  d->pBoundingBoxesOutput->emitObject(matBB2);
  //d->pImageOuput->emitObject(matThresholded*255);

  //for (int j=0;j<matDigits.columns();j++)
  //  qDebug() << matDigits(0,j);
  //qDebug() << sDigitsString;
}


//Indigates if the blob should be rejected
bool PiiDigitExtractor::isIncorrectBlob(const PiiMatrix<int> &boundingBoxes, int index,int imageHeight)
{
  PII_D;
  int iTop = boundingBoxes(index,1);
  int iLeft = boundingBoxes(index,0);
  int iRight = iLeft+boundingBoxes(index,2);
  int iBottom = iTop+boundingBoxes(index,3);

  //single blobs are discarded if they:
  //  a) locate too low or high in the image (max dLocationThreshold times image height)
  //  b) are too small (max dSizeThreshold times image height)
  //  c) are inside another blob

  //case a:
  if (iTop>imageHeight*d->dLocationThreshold || iBottom<imageHeight*(1.0-d->dLocationThreshold))
    return true;

  //case b:
  if (boundingBoxes(index,3)<imageHeight*d->dSizeThreshold)
    return true;

  for (int i=0; i<boundingBoxes.rows(); ++i)
    {
      if (i==index)
        continue;

      int iT = boundingBoxes(i,1);
      int iL = boundingBoxes(i,0);
      int iR = iL+boundingBoxes(i,2);
      int iB = iT+boundingBoxes(i,3);

      //case c:
      if (iTop>=iT && iBottom<=iB && iRight<=iR && iLeft>=iL)
        return true;
    }

  return false;
}


// Create blobs from bounding boxes by combining the ones that are close to each other
void PiiDigitExtractor::createBlob(PiiMatrix<int> &boundingBoxes)
{
  if (boundingBoxes.isEmpty())
    return;
  
  //Combines bounding boxes into blobs. Specially, boxes may be combined if
  //x locations are on most iCombineXThreshold pixels apart
  //y locations are on most iCombineYThreshold pixels apart

  PII_D;
  int iX,iY,iWidth,iHeight;

  bool changed=false;
  int b1 = 0,b2 = 0,bdist;
  do
    {
      changed=false;
      bdist=9999;

      for (int firstBox=0;firstBox<boundingBoxes.rows(); ++firstBox)
        {
          int iTop = boundingBoxes(firstBox,1);
          int iBottom = iTop+boundingBoxes(firstBox,3);
          int iLeft = boundingBoxes(firstBox,0);
          int iRight = iLeft+boundingBoxes(firstBox,2);

          //check whether to combine firstBox with secondBox
          for (int secondBox=firstBox+1;secondBox<boundingBoxes.rows(); ++secondBox)
            {
              int iT = boundingBoxes(secondBox,1);
              int iB = iT+boundingBoxes(secondBox,3);
              int iL=boundingBoxes(secondBox,0);
              int iR=iL+boundingBoxes(secondBox,2);

              //Overlap within given threshold amount (i.e. close enough)
              if (iR+d->iCombineXThreshold >= iLeft &&
                  iL <= iRight+d->iCombineXThreshold &&
                  (iB >= iTop-d->iCombineYThreshold) &&
                  (iT <= iBottom+d->iCombineYThreshold))
                {
                  //Do not combine, if new aspect ratio would be too distorted (clearly wider than high). This case it is more probably that
                  //two digits here...
                  int ww=Pii::max(iR,iRight)-Pii::min(iL,iLeft);
                  int hh=Pii::max(iB,iBottom)-Pii::min(iT,iTop);
                  if (ww<1.2*hh)
                    {
                      changed=true;

                      int thisdist=Pii::min(Pii::abs(iLeft-iR),Pii::abs(iL-iRight))-d->iCombineXThreshold;
                      thisdist=Pii::min(thisdist,Pii::abs(iB-iTop)-d->iCombineYThreshold);
                      thisdist=Pii::min(thisdist,Pii::abs(iBottom-iT)-d->iCombineYThreshold);

                      //there might be several possible combinations. Always take the best looking first
                      if (thisdist<bdist)
                        {
                          bdist=thisdist;
                          b1=firstBox;
                          b2=secondBox;
                        }
                    }
                }
            }
        }

      if (changed)
        {
          //qDebug() << " Combine blobs " << b1 << " and " << b2 ;
          //qDebug() << boundingBoxes(b1,0) << " "<< boundingBoxes(b1,1) << " "<< boundingBoxes(b1,2) << " "<< boundingBoxes(b1,3);
          //qDebug() << boundingBoxes(b2,0) << " "<< boundingBoxes(b2,1) << " "<< boundingBoxes(b2,2) << " "<< boundingBoxes(b2,3);
          iX = Pii::min(boundingBoxes(b1,0), boundingBoxes(b2,0));
          iY = Pii::min(boundingBoxes(b1,1), boundingBoxes(b2,1));

          iWidth = Pii::max(boundingBoxes(b2,0)+boundingBoxes(b2,2)-iX, boundingBoxes(b1,0)+boundingBoxes(b1,2)-iX);
          iHeight = Pii::max(boundingBoxes(b2,1)+boundingBoxes(b2,3)-iY, boundingBoxes(b1,1)+boundingBoxes(b1,3)-iY);

          boundingBoxes(b1,0)=iX;
          boundingBoxes(b1,1)=iY;
          boundingBoxes(b1,2)=iWidth;
          boundingBoxes(b1,3)=iHeight;
          boundingBoxes.removeRow(b2);
        }
    } while (changed);

}


/* classifier related stuff starts here */

//Read data for digit classification
void PiiDigitExtractor::initializeKnnClassifier()
{
  PII_D;
  //digitExtractor.bin contains data for classification in a predefined format
  QFile binFile(":/digitExtractor.bin");
  if (!binFile.open(QIODevice::ReadOnly))
    PII_THROW(PiiExecutionException, tr("Could not read classifier data."));

  PiiBinaryInputArchive ia(&binFile);
  d->digitClassifier.setK(1);

  //Feature vectors
  PiiMatrix<float> matFeatureVector;
  ia >> matFeatureVector;
  d->digitClassifier.setModels(matFeatureVector);

  //Labels
  QVector<double> vecLabels;
  for (int i=9; i>=0; --i)
    vecLabels.insert(0,1000,(double)i);
  d->digitClassifier.setClassLabels(vecLabels);

  //Average vector
  ia >> d->matMeanDigitVector;
      
  //Base vectors
  ia >> d->matBaseDigitVectors;
}

//Converts segmented image into (PCA-reduced) feature vector, and classifies it using kNN
int PiiDigitExtractor::classify(const PiiMatrix<int> &scaledImage)
{
  PII_D;

  //reshapes data, and makes PCA transform
  PiiMatrix<float> fFullVector(400,1);
  for (int i=0;i<20;i++)
    for (int j=0;j<20;j++)
      fFullVector(j*20+i,0)=scaledImage(i,j);

  fFullVector = (fFullVector-127.5)/127.5;
  fFullVector -= d->matMeanDigitVector;

  PiiMatrix<float> matFeatureVector=d->matBaseDigitVectors*fFullVector;

  return (int)d->digitClassifier.classify(matFeatureVector[0]);
}



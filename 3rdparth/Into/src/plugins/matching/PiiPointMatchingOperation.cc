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

#include "PiiPointMatchingOperation.h"

#include <PiiClassification.h>
#include <PiiSquaredGeometricDistance.h>
#include <PiiYdinTypes.h>
#include <PiiRigidPlaneRansac.h>

#include <PiiMatrixUtil.h>

PiiPointMatchingOperation::Data::Data(int pointDimensions) :
  PiiClassifierOperation::Data(PiiClassification::NonSupervisedLearner),
  matIdentity(PiiMatrix<float>::identity(pointDimensions+1)),
  matEmptyLocation(1, pointDimensions*2),
  matEmptyPoint(1, pointDimensions),
  pMatcher(new Matcher), pNewMatcher(0),
  iModelCount(0),
  iPointDimensions(pointDimensions),
  matchingMode(PiiMatching::MatchAllModels),
  bMustSendPoints(false)
{
}

PiiPointMatchingOperation::Data::~Data()
{
  delete pMatcher;
  delete pNewMatcher;
}

PiiPointMatchingOperation::PiiPointMatchingOperation(int pointDimensions) :
  PiiClassifierOperation(new Data(pointDimensions))
{
  init();
}

PiiPointMatchingOperation::PiiPointMatchingOperation(Data* data) :
  PiiClassifierOperation(data)
{
  init();
}

void PiiPointMatchingOperation::init()
{
  PII_D;
  addSocket(d->pPointsInput = new PiiInputSocket("points"));
  addSocket(d->pLocationInput = new PiiInputSocket("location"));
  d->pLocationInput->setOptional(true);

  addSocket(d->pModelIndexOutput = new PiiOutputSocket("model index"));
  addSocket(d->pLocationOutput = new PiiOutputSocket("location"));
  addSocket(d->pTransformOutput = new PiiOutputSocket("transform"));
  addSocket(d->pQueryPointsOutput = new PiiOutputSocket("query points"));
  addSocket(d->pModelPointsOutput = new PiiOutputSocket("model points"));
}


PiiPointMatchingOperation::~PiiPointMatchingOperation()
{}

void PiiPointMatchingOperation::check(bool reset)
{
  PII_D;

  PiiClassifierOperation::check(reset);

  if (!d->pMatcher->modelPoints().isEmpty())
    {
      if (d->matLocations.isEmpty())
        PII_THROW(PiiExecutionException, tr("Point matcher has at least one model but no model locations specified."));
      if (!d->lstLabels.isEmpty() && d->lstLabels.size() != d->matLocations.rows())
        PII_THROW(PiiExecutionException, tr("There must be an equal number of model locations and labels."));
    }
  
  d->pMatcher->setMatchingMode(d->matchingMode);
  d->bMustSendPoints = d->pModelPointsOutput->isConnected() ||
    d->pQueryPointsOutput->isConnected();
}

bool PiiPointMatchingOperation::learnBatch()
{
  PII_D;
  try
    {
      d->pNewMatcher = new Matcher;
      d->pNewMatcher->buildDatabase(d->matNewPoints,
                                    d->matNewFeatures,
                                    d->vecNewModelIndices,
                                    this);
    }
  catch (PiiClassificationException& ex)
    {
      delete d->pNewMatcher;
      d->pNewMatcher = 0;
      setLearningError(ex.message());
      return false;
    }
  return true;
}

void PiiPointMatchingOperation::replaceClassifier()
{
  PII_D;
  delete d->pMatcher;
  d->pMatcher = d->pNewMatcher;
  d->pNewMatcher = 0;
  d->lstLabels = d->lstNewLabels;
  d->matLocations = d->matNewLocations;

  resizeBatch(0);
}

void PiiPointMatchingOperation::resetClassifier()
{
  PII_D;
  delete d->pMatcher;
  d->pMatcher = new Matcher;
  d->lstLabels.clear();
  d->matLocations.clear();
}

void PiiPointMatchingOperation::resizeBatch(int newSize)
{
  PII_D;
  if (newSize == 0)
    {
      d->lstNewLabels.clear();
      d->matNewPoints.clear();
      d->matNewLocations.clear();
      d->matNewFeatures.clear();
      d->vecNewModelIndices.clear();
      d->iModelCount = 0;
    }
  else
    {
      // PENDING
    }
}

int PiiPointMatchingOperation::bufferedSampleCount() const
{
  return _d()->iModelCount;
}

int PiiPointMatchingOperation::featureCount() const
{
  return _d()->matNewFeatures.columns();
}


int PiiPointMatchingOperation::checkDescriptor(const PiiMatrix<float>& points,
                                               const PiiMatrix<float>& features) const
{
  const PII_D;
  const int iPoints = points.rows();
  if (iPoints < 1)
    PII_THROW(PiiExecutionException, tr("The received descriptor contains no feature points."));
  if (points.columns() != d->iPointDimensions)
    PII_THROW(PiiExecutionException,
              tr("Input points must be %1-dimensional. Received %2-dimensional points")
              .arg(d->iPointDimensions)
              .arg(points.columns()));

  if (features.rows() != iPoints)
    PII_THROW(PiiExecutionException,
              tr("Each feature point must have a descriptor. Got %1 points and %2 features.")
              .arg(points.rows()).arg(features.rows()));

  const int iFeatures = featureCount();
  if (iFeatures != 0 && iFeatures != features.columns())
    PII_THROW(PiiExecutionException,
              tr("All feature points must be described with the same number of features. "
                 "The model database has %1 features, the current feature matrix has %2.")
              .arg(iFeatures)
              .arg(features.columns()));

  return iPoints;
}

double PiiPointMatchingOperation::classify()
{
  PII_D;
  
  PiiMatrix<float> matFeatures = PiiYdin::convertMatrixTo<float>(featureInput());
  PiiMatrix<float> matPoints = PiiYdin::convertMatrixTo<float>(d->pPointsInput);

  checkDescriptor(matPoints, matFeatures);

  PiiMatching::MatchList lstMatches = match(*d->pMatcher, matPoints, matFeatures);
  if (d->matchingMode == PiiMatching::MatchAllModels)
    {
      startMany();
      if (lstMatches.size() > 1)
        removeDuplicates(lstMatches);
      for (int i=0; i<lstMatches.size(); ++i)
        emitMatch(lstMatches[i], matPoints);
      endMany();
    }
  else
    {
      if (lstMatches.size() > 0)
        emitMatch(lstMatches[0], matPoints);
      else
        {
          classificationOutput()->emitObject(NAN);
          d->pModelIndexOutput->emitObject(-1);
          d->pLocationOutput->emitObject(d->matEmptyLocation);
          d->pTransformOutput->emitObject(d->matIdentity);
          d->pQueryPointsOutput->emitObject(d->matEmptyPoint);
          d->pModelPointsOutput->emitObject(d->matEmptyPoint);
        }
    }
  return NAN;
}

void PiiPointMatchingOperation::removeDuplicates(PiiMatching::MatchList&)
{}

void PiiPointMatchingOperation::emitMatch(const PiiMatching::Match& match, const PiiMatrix<float>& queryPoints)
{
  PII_D;
  int iModelIndex = match.modelIndex();

  /*qDebug("PiiPointMatchingOperation::match()\n"
         "Match { index: %d, label: %lf }",
         iModelIndex,
         iModelIndex < d->lstLabels.size() ?
         d->lstLabels[iModelIndex] : iModelIndex);*/
  
  classificationOutput()->emitObject(iModelIndex < d->lstLabels.size() ?
                                     d->lstLabels[iModelIndex] : iModelIndex);
  d->pModelIndexOutput->emitObject(iModelIndex);
  d->pLocationOutput->emitObject(PiiMatrix<float>(1, d->iPointDimensions*2, d->matLocations[iModelIndex]));
  d->pTransformOutput->emitObject(toTransformMatrix(match.transformParams()));

  if (d->bMustSendPoints)
    {
      QList<QPair<int,int> > lstPairs = match.matchedPoints();
      PiiMatrix<float> matQueryPoints(0, d->iPointDimensions);
      matQueryPoints.reserve(lstPairs.size());
      PiiMatrix<float> matModelPoints(matQueryPoints);
      for (int i=0; i<lstPairs.size(); ++i)
        {
          matQueryPoints.appendRow(queryPoints[lstPairs[i].first]);
          matModelPoints.appendRow(d->pMatcher->modelPoints()[lstPairs[i].second]);
        }
      d->pQueryPointsOutput->emitObject(matQueryPoints);
      d->pModelPointsOutput->emitObject(matModelPoints);
    }
}

void PiiPointMatchingOperation::collectSample(double label, double /*weight*/)
{
  PII_D;
  
  PiiMatrix<float> matFeatures = PiiYdin::convertMatrixTo<float>(featureInput());
  PiiMatrix<float> matPoints = PiiYdin::convertMatrixTo<float>(d->pPointsInput);

  const int iPoints = checkDescriptor(matPoints, matFeatures);

  if (learningBatchSize() > 0 && d->iModelCount >= learningBatchSize())
    {
      int iRemovedIndex = 0;
      switch (fullBufferBehavior())
        {
        case PiiClassification::OverwriteRandomSample:
          iRemovedIndex = rand() % d->iModelCount;
          break;
        case PiiClassification::OverwriteOldestSample:
          break;
        case PiiClassification::DiscardNewSample:
        default:
          return;
        }
      // Find the first occurrence of the model to be removed.
      for (int i=0; i<d->vecNewModelIndices.size(); ++i)
        {
          if (d->vecNewModelIndices[i] == iRemovedIndex)
            {
              // Find the last occurrence
              int iFirstIndex = i;
              for (++i; i<d->vecNewModelIndices.size() && d->vecNewModelIndices[i] == iRemovedIndex; ++i) {}
              int iPointsInModel = i - iFirstIndex;
              // We deleted one model -> must decrease larger model indices by one
              for (; i<d->vecNewModelIndices.size(); ++i)
                --d->vecNewModelIndices[i];
              // Delete everything in between
              d->vecNewModelIndices.remove(iFirstIndex, iPointsInModel);
              d->matNewFeatures.removeRows(iFirstIndex, iPointsInModel);
              d->matNewPoints.removeRows(iFirstIndex, iPointsInModel);
              if (d->lstNewLabels.size() < iRemovedIndex)
                d->lstNewLabels.removeAt(iRemovedIndex);
              d->matNewLocations.removeRow(iRemovedIndex);
              --d->iModelCount;
              break;
            }
        }
    }
  
  // PENDING Should we reserve some room to further avoid
  // reallocation?
  
  d->matNewFeatures.appendRows(matFeatures);
  d->matNewPoints.appendRows(matPoints);
  d->vecNewModelIndices.reserve(d->vecNewModelIndices.size() + iPoints);
  for (int i=0; i<iPoints; ++i)
    d->vecNewModelIndices << d->iModelCount;
  if (labelInput()->isConnected())
    d->lstNewLabels << label;

  // If location input is connected, read and store object
  // location from there.
  if (d->pLocationInput->isConnected())
    {
      PiiMatrix<float> matLocation = PiiYdin::convertMatrixTo<float>(d->pLocationInput);
      if (matLocation.rows() != 1 || matLocation.columns() != d->iPointDimensions * 2)
        PII_THROW_WRONG_SIZE(d->pLocationInput, matLocation, 1, d->iPointDimensions * 2);
      d->matNewLocations.appendRow(matLocation);
      //Pii::transformRows(d->matPoints(0,-iPoints,-1,-1), matLocation[0], std::minus<int>());
    }
  // Otherwise find bounding box automatically.
  else
    {
      PiiMatrix<float> matMin(Pii::min(matPoints, Pii::Vertically));
      PiiMatrix<float> matMax(Pii::max(matPoints, Pii::Vertically));
      if (d->matNewLocations.isEmpty())
        d->matNewLocations.resize(0, d->iPointDimensions*2);
      float* pCube = d->matNewLocations.appendRow();
      for (int i=0; i<d->iPointDimensions; ++i)
        pCube[i+d->iPointDimensions] = matMax(0,i) - (pCube[i] = matMin(0,i));
    }
  ++d->iModelCount;
}

PiiPointMatchingOperation* PiiPointMatchingOperation::clone() const
{
  const PII_D;
  PiiPointMatchingOperation* pNewOperation = static_cast<PiiPointMatchingOperation*>(PiiClassifierOperation::clone());
  delete pNewOperation->_d()->pMatcher;
  Data* pNewData = pNewOperation->_d();
  pNewData->pMatcher = d->pMatcher != 0 ? new Matcher(*d->pMatcher) : 0;
  pNewData->matLocations = d->matLocations;
  pNewData->matNewPoints = d->matNewPoints;
  pNewData->matNewLocations = d->matNewLocations;
  pNewData->matNewFeatures = d->matNewFeatures;
  pNewData->lstLabels = d->lstLabels;
  pNewData->lstNewLabels = d->lstNewLabels;
  pNewData->vecNewModelIndices = d->vecNewModelIndices;
  pNewData->iModelCount = d->iModelCount;
  
  return pNewOperation;
}

void PiiPointMatchingOperation::setMatchingMode(PiiMatching::ModelMatchingMode matchingMode) { _d()->matchingMode = matchingMode; }
PiiMatching::ModelMatchingMode PiiPointMatchingOperation::matchingMode() const { return _d()->matchingMode; }

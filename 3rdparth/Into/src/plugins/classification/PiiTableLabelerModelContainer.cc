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

#include "PiiTableLabelerModelContainer.h"
#include "PiiTableLabelerModel.h"
#include "PiiTableLabelerOperation.h"

#include <QtDebug>

PiiTableLabelerModelContainer::PiiTableLabelerModelContainer() : //PiiTableLabelerOperation* operation) :
  _pOperation(0)
{

}


void PiiTableLabelerModelContainer::initialize(const QVariantList& layers,
                                               const QVariantList& featureProperties,
                                               const QList< PiiMatrix< PiiMinMax > >& ruleMatrices,
                                               const QList< QList<double> >& labelMatrix,
                                               const QList< QList<bool> >& ruleVectors,
                                               PiiTableLabelerOperation* operation)
{
  _pOperation = operation;
  for (int i = 0; i<layers.size(); ++i)
    {
      PiiTableLabelerModel* model = new PiiTableLabelerModel(layers[i],
                                                             featureProperties,
                                                             ruleMatrices[i],
                                                             labelMatrix[i],
                                                             ruleVectors[i],
                                                             this);

      _lstModels << model;
      connect(model, SIGNAL(changedStatusChanged(bool)), this, SIGNAL(changedStatusChanged(bool)));
      emit modelCreated(model,  PiiLayerParser::layerName(layers[i]));
    }
}


/* PENDING
 * If the amount of layers differs from the amount of models stored in
 * this class, the models should be removed or added. This must
 * be implemented later. Currently it is only assumed, that the amount
 * of layers matches with the amount of models.
 */
void PiiTableLabelerModelContainer::setLayers(const QVariantList& layers)
{
  for (int i = 0; i<_lstModels.size(); ++i)
    {
      _lstModels[i]->setLayer(layers[i]);
    }
  //emit changedStatusChanged(false);
}

void PiiTableLabelerModelContainer::setContent(const QList< PiiMatrix< PiiMinMax > >& ruleMatrices,
                                               const QList< QList<double> >& labelMatrix,
                                               const QList< QList<bool> >& ruleVectors)
{
  for (int i = 0; i<_lstModels.size(); ++i)
    {
      _lstModels[i]->setContent(ruleMatrices[i], labelMatrix[i], ruleVectors[i]);
    }
  emit changedStatusChanged(false);
}

void PiiTableLabelerModelContainer::setFeatureProperties(const QVariantList& featureProperties)
{
    for (int i = 0; i<_lstModels.size(); ++i)
    {
      _lstModels[i]->setFeatureProperties(featureProperties);
    }
}

bool PiiTableLabelerModelContainer::hasChanged() const
{
  QList< PiiMatrix< PiiMinMax > > ruleMatrices;
  QList< QList<double> > labelMatrix;
  QList< QList<bool> > ruleVectors;
  
  for (int i = 0; i<_lstModels.size(); ++i)
    {
      ruleMatrices << _lstModels[i]->ruleMatrix();
      labelMatrix << _lstModels[i]->labelVector();
      ruleVectors << _lstModels[i]->ruleVector();
    }

  // Check labels and rule types
  if (labelMatrix != _pOperation->rawLabels() ||
      ruleVectors != _pOperation->rawRuleVectors())
    return true;

  // Check size of the rule matrices
  QList<PiiMatrix<PiiMinMax> > operationRuleMatrices = _pOperation->rawRuleMatrices();
  if (ruleMatrices.size() != operationRuleMatrices.size())
    return true;

  // Check content of the rule matrices
  for (int i=0; i<ruleMatrices.size(); i++)
    {
      PiiMatrix<PiiMinMax> ruleMatrix = ruleMatrices[i];
      PiiMatrix<PiiMinMax> operationRuleMatrix = operationRuleMatrices[i];
      if (ruleMatrix.rows() != operationRuleMatrix.rows() ||
          ruleMatrix.columns() != operationRuleMatrix.columns())
        return true;

      for (int r=0; r<ruleMatrix.rows(); r++)
        for (int c=0; c<ruleMatrix.columns(); c++)
          if (ruleMatrix(r,c) != operationRuleMatrix(r,c))
            return true;
    }

  return false;
}

void PiiTableLabelerModelContainer::acceptChanges ()
{
  QList< PiiMatrix< PiiMinMax > > ruleMatrices;
  QList< QList<double> > labelMatrix;
  QList< QList<bool> > ruleVectors;
  
  for (int i = 0; i<_lstModels.size(); ++i)
    {
      ruleMatrices << _lstModels[i]->ruleMatrix();
      labelMatrix << _lstModels[i]->labelVector();
      ruleVectors << _lstModels[i]->ruleVector();
    } 

  _pOperation->setRawRuleMatrices(ruleMatrices);
  _pOperation->setRawLabels(labelMatrix);
  _pOperation->setRawRuleVectors(ruleVectors);
  
  emit changedStatusChanged(false);
}

void PiiTableLabelerModelContainer::reset ()
{
  QList< PiiMatrix< PiiMinMax > > ruleMatrices = _pOperation->rawRuleMatrices();
  QList< QList<double> > labelMatrix = _pOperation->rawLabels();
  QList< QList<bool> > ruleVectors = _pOperation->rawRuleVectors();
  
  for (int i = 0; i<_lstModels.size(); ++i)
    _lstModels[i]->setContent(ruleMatrices[i], labelMatrix[i], ruleVectors[i]);

  emit changedStatusChanged(false);
}

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

#include "PiiVisualSomClassifier.h"
#include <PiiVisualSomOperation.h>

PiiVisualSomClassifier::Data::Data() :
  pSom(0), pTrainer(0)
{
}

PiiVisualSomClassifier::PiiVisualSomClassifier() :
  PiiOperationCompound(new Data)
{
  initTrainer();
  initSom(false); //the default is normal som
  connectSignals();
  
  setMapSize(QSize(5,4));
}

PiiVisualSomClassifier::PiiVisualSomClassifier(PiiSerialization::Void) :
  PiiOperationCompound(new Data)
{
  // Contents will be read from an archive
}

void PiiVisualSomClassifier::initTrainer()
{
  PII_D;
  delete d->pTrainer;
  d->pTrainer = createOperation("PiiVisualTrainer", "trainer");
}

void PiiVisualSomClassifier::initSom(bool controlledTraining)
{
  PII_D;
  PiiOperation *som = 0;

  //get properties from the old som
  QList<QPair<QString, QVariant> > properties;
  if (d->pSom != 0)
    properties = Pii::propertyList(*d->pSom);

  if (controlledTraining)
    {
      som = createOperation("PiiVisualSomOperation","som");
      exposeInput(som->input("image"), "image", AliasConnection);
      unexposeInput("subimage");
      unexposeInput("location");
    }
  else
    {
      som = createOperation("PiiSomOperation<double>","som");
      exposeInput(d->pTrainer->input("image"), "image", AliasConnection);
      exposeInput(d->pTrainer->input("subimage"), "subimage", AliasConnection);
      exposeInput(d->pTrainer->input("location"), "location", AliasConnection);
    }
  
  som->connectOutput("classification", d->pTrainer, "class index");
  
  //set properties to the new one
  if ( !properties.isEmpty() )
    Pii::setPropertyList(*som,properties);
  else
    som->setProperty("learningAlgorithm", 1); //Balanced SOM algorithm is the default

  exposeInput(som->input("features"), "features", AliasConnection);
  exposeInput(som->input("boundaries"), "boundaries", AliasConnection);
  exposeInput(d->pTrainer->input("filename"), "filename", AliasConnection);
  
  delete d->pSom;
  d->pSom = som;
}


void PiiVisualSomClassifier::setControlledTraining(bool para)
{
  PII_D;
 //if it is possible and necessary, we change the controller of the training
  if (d->pSom && d->pTrainer && para != controlledTraining())
    {
      //change controller
      initSom(para);
      connectSignals();
    }
}

bool PiiVisualSomClassifier::controlledTraining() const
{
  return qobject_cast<PiiVisualSomOperation*>(_d()->pSom) != 0;
}

void PiiVisualSomClassifier::connectSignals()
{
  PII_D;
  if (controlledTraining())
    {
      connect(d->pSom, SIGNAL(addSubImage(QImagePtr, int)), d->pTrainer, SLOT(storeSubImage(QImagePtr, int)));
      //connect(d->pSom, SIGNAL(removeSubImages(int)), d->pTrainer, SLOT(removeFromBuffer(int)));
      connect(d->pTrainer, SIGNAL(subImageRemoved(QImagePtr)), d->pSom, SLOT(removeImage(QImagePtr)));
      connect(d->pSom, SIGNAL(allSubImagesAdded()), d->pTrainer, SIGNAL(allSubImagesArrived()));
    }
}

void PiiVisualSomClassifier::setMapSize(QSize mapSize)
{
  PII_D;
  d->pSom->setProperty("size", mapSize);
  int radius = mapSize.width() > mapSize.height() ? mapSize.width() : mapSize.height();
  radius /= 2;
  if (radius == 0) radius = 1;
  d->pSom->setProperty("initialRadius", radius);
  d->pSom->setProperty("learningLength", 150 * mapSize.height()*mapSize.width());
  d->pTrainer->setProperty("gridSize", mapSize);
}

QSize PiiVisualSomClassifier::mapSize() const
{
  return _d()->pSom->property("size").toSize();
}

void PiiVisualSomClassifier::setLayers(const QVariantList& layers)
{
  // This removes extra outputs from the trainer. Since the removal is
  // signaled to this operation, extra aliases will be removed as
  // well.
  //qDebug("calling d->pTrainer->setProperty(layers) (%d layers)", layers.size());
  _d()->pTrainer->setProperty("layers", layers);

  createOutputs();
}

void PiiVisualSomClassifier::createOutputs()
{
  // Expose all of the trainer's outputs that have not been exposed
  // yet
  QList<PiiAbstractOutputSocket*> outputs = _d()->pTrainer->outputs();
  for (int i=outputCount(); i<outputs.size(); i++)
    exposeOutput(outputs[i], QString("label%1").arg(i), AliasConnection);
}

QVariantList PiiVisualSomClassifier::layers() const
{
  return _d()->pTrainer->property("layers").toList();
}

PiiVisualSomClassifier* PiiVisualSomClassifier::clone() const
{
  PiiVisualSomClassifier *pResult = static_cast<PiiVisualSomClassifier*>(PiiOperationCompound::clone());

  if (pResult == 0)
    return 0;
  
  pResult->_d()->pSom = pResult->findChild<PiiOperation*>("som");
  pResult->_d()->pTrainer = pResult->findChild<PiiOperation*>("trainer");

  //get and set properties from the old to the new one
  QList<QPair<QString, QVariant> > properties = Pii::propertyList(*this,0,
                                                                  Pii::WritableProperties |
                                                                  Pii::StoredProperties);
  Pii::setPropertyList(*pResult,properties);
  
  pResult->connectSignals();

  return pResult;
}

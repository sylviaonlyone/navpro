/* This file is part of Into ClassificationGui plug-in.
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

#include "PiiVisualTrainerConnector.h"
#include <PiiOperation.h>
#include "PiiVisualTrainerWidget.h"

bool PiiVisualTrainerConnector::connectResources(void* resource1, void* resource2, const QString& role) const
{
  PiiOperation *pOperation = static_cast<PiiOperation*>(resource1);
  PiiVisualTrainerWidget *pWidget = static_cast<PiiVisualTrainerWidget*>(resource2);

  if (pOperation != 0 && pWidget != 0)
    {
      if (role == "pii:configurator")
        {
          QObject::connect(pOperation, SIGNAL(subImageAdded(QImagePtr,double,double)),
                           pWidget, SLOT(addSubImage(QImagePtr,double,double)));
          QObject::connect(pOperation, SIGNAL(subImageRemoved(QImagePtr)),pWidget, SLOT(removeSubImage(QImagePtr)));
          QObject::connect(pOperation, SIGNAL(mainImageChanged(QImagePtr)),pWidget, SLOT(setMainImage(QImagePtr)));
          QObject::connect(pOperation, SIGNAL(allSubImagesRemoved()),pWidget, SLOT(removeAllSubImages()));
          QObject::connect(pOperation, SIGNAL(allSubImagesArrived()),pWidget, SLOT(updateView()));
          QObject::connect(pOperation, SIGNAL(gridSizeChanged(const QSize&)),pWidget, SLOT(setGridSize(const QSize&)));
          QObject::connect(pOperation, SIGNAL(layersChanged(const QVariantList&)),pWidget, SLOT(setLayers(const QVariantList&)));
          QObject::connect(pOperation, SIGNAL(labelsChanged(const QVariantList&)),pWidget, SLOT(setLabels(const QVariantList&)));
          QObject::connect(pOperation, SIGNAL(stateChanged(int)),pWidget, SLOT(checkOperationStatus(int)));
        }
      pWidget->setOperation(pOperation);
    }
  else
    return false;

  return true;
}

bool PiiVisualTrainerConnector::disconnectResources(void* resource1, void* resource2, const QString& role) const
{
  PiiOperation *pOperation = static_cast<PiiOperation*>(resource1);
  PiiVisualTrainerWidget *pWidget = static_cast<PiiVisualTrainerWidget*>(resource2);
  
  if (pOperation != 0 && pWidget != 0)
    {
      if (role == "pii:configurator")
        {
          QObject::disconnect(pOperation, SIGNAL(subImageAdded(QImagePtr,double,double)),
                              pWidget, SLOT(addSubImage(QImagePtr,double,double)));
          QObject::disconnect(pOperation, SIGNAL(subImageRemoved(QImagePtr)),pWidget, SLOT(removeSubImage(QImagePtr)));
          QObject::disconnect(pOperation, SIGNAL(mainImageChanged(QImagePtr)),pWidget, SLOT(setMainImage(QImagePtr)));
          QObject::disconnect(pOperation, SIGNAL(allSubImagesRemoved()),pWidget, SLOT(removeAllSubImages()));
          QObject::disconnect(pOperation, SIGNAL(allSubImagesArrived()),pWidget, SLOT(updateView()));
          QObject::disconnect(pOperation, SIGNAL(gridSizeChanged(const QSize&)),pWidget, SLOT(setGridSize(const QSize&)));
          QObject::disconnect(pOperation, SIGNAL(layersChanged(const QVariantList&)),pWidget, SLOT(setLayers(const QVariantList&)));
          QObject::disconnect(pOperation, SIGNAL(labelsChanged(const QVariantList&)),pWidget, SLOT(setLabels(const QVariantList&)));
          QObject::disconnect(pOperation, SIGNAL(stateChanged(int)),pWidget, SLOT(checkOperationStatus(int)));
        }
      pWidget->setOperation(0);
    }
  else
    return false;

  return true;
}


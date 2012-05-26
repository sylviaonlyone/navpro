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

#ifndef _PIITABLELABELERMODELCONTAINER_H
#define _PIITABLELABELERMODELCONTAINER_H

#include <QObject>
#include <QStringList>
#include <QVariant>

#include <PiiMatrix.h>

class PiiTableLabelerOperation;
class PiiTableLabelerModel;
class PiiMinMax;

/**
 * This class is a container of all models in the table labeler
 * configurator. For each tab there is a model. When changing the
 * content of a single tab, it is possible to do that by changing the
 * content of signle model. When changing the content, which can
 * affect to all tabs (
 */
class PiiTableLabelerModelContainer : public QObject
{
  Q_OBJECT

public:
  /**
   * Constructor.
   */
  PiiTableLabelerModelContainer(); //PiiTableLabelerOperation* operation);

  /**
   * TODO: DESCRIBE PARAMETERS
   * Create a new models, adds its internally in the list of models.
   * For each created model the signal modelCreated() is emitted.
   */
  void initialize(const QVariantList& layers,
                  const QVariantList& featureProperties,
                  const QList< PiiMatrix< PiiMinMax > >& ruleMatrices,
                  const QList< QList<double> >& labelMatrix,
                  const QList< QList<bool> >& ruleVectors,
                  PiiTableLabelerOperation* operation);
 
  /**
   * This sets the layers for the model container. The layers can be set
   * also when calling the function initalize(), but if there is need for
   * setting the layers after initizing the model containter, for examle if
   * more classes are added in a class type layer, this function
   * should be called in order to changes to have effect also in the
   * configurator user interface associated with this model container.
   * @p PiiTableLabelerOperation::setLayers() calls this
   * function, so normally the programmer doesn't need to take care of
   * calling this function.
   *
   * If a type of the layer remain unchanged, the content of the
   * user interface is not cleared. Only the relevant parts of the
   * configuration ui are updated. For example the combo box
   * containing the class names is updated, if the list of class names
   * in the "class" type layer has been changed. If a layer type has
   * been changed, the corresponging content in the configurator user
   * interface (tab widget) is recreated.
   *
   * PENDING:
   * Add functionality for checking the changed "min", "max" and
   * "resolution" values in the "float" type layers.
   *
   */
  void setLayers(const QVariantList& layers);

  /**
   * Updates @p ruleMatrices and @p labelMatrix in the model container.
   * The new rule matrices and label vectors are updated in each model
   * in the containter. Also the content of the configurator user
   * interface corresponding to the model container is changed. This
   * function is normally called from @p PiiTableLabeler operation,
   * when the content of the data has been changed as a result of
   * internal action of the application.
   */
  void setContent(const QList< PiiMatrix< PiiMinMax > >& ruleMatrices,
                  const QList< QList<double> >& labelMatrix,
                  const QList< QList<bool> >& ruleVectors);


  /**
   * TODO: Complete this comment block.
   */
  void setFeatureProperties(const QVariantList& featureProperties);
  
  void acceptChanges ();

  void reset ();

  bool hasChanged() const;
  
signals:
  /**
   * This signal is emmited each time, a new model has been created.
   */
  void modelCreated(PiiTableLabelerModel* model, const QString& name);
  
  /**
   * This signal is emitted, when one of the model has been modified in a
   * way, that the chagned status of this container class has been
   * changed. Refer to the documentation of the function @p
   * setChanged() of the class @p PiiConfigurationWidget to get the
   * more information, what "changed status" means.
   */
  void changedStatusChanged(bool status);  
  
protected:
  QList<PiiTableLabelerModel*> _lstModels;
  PiiTableLabelerOperation* _pOperation;

};


#endif //_PIITABLELABELERMODELCONTAINER_H

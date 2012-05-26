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

#include "PiiTableLabelerOperation.h"
#include "PiiTableLabelerConfigurator.h"
#include "PiiTableLabelerModelContainer.h"
#include <PiiYdinTypes.h>
#include <PiiSocket.h>
//#include <PiiMath.h> //For infinity values
#include <PiiMatrixUtil.h>
//#include <iostream>

using namespace PiiYdin;
using namespace Pii;

PiiTableLabelerOperation::Data::Data() :
  defaultLabel(0.0),
  pConfigurator(0),
  pToolBar(0)

{
}

PiiTableLabelerOperation::PiiTableLabelerOperation() :
  PiiDefaultOperation(new Data, NonThreaded)
{
  PII_D;
  //_pFeatureInput = new PiiInputSocket("features");
  d->pFeatureInput = new PiiInputSocket("features");
  addSocket(d->pFeatureInput);

}

void PiiTableLabelerOperation::check(bool reset)
{
  PII_D;
  /**
   * Repair widths of the _lstRuleMatrices and _labelMatrix. This must
   * be done because there was a bug in initializing the widths of the
   * ruleMatrices and labelMatrix. And it is possible that there are
   * some old configurations where the _labelMatrix is too long.
   */
  for (int i=0; i<d->lstRuleMatrices.size(); i++)
    {
      PiiMatrix<PiiMinMax>& ruleMatrix = d->lstRuleMatrices[i];
      QList<double>& labelMatrix = d->labelMatrix[i];
      if (ruleMatrix.rows() > labelMatrix.size())
        ruleMatrix.resize(labelMatrix.size(), ruleMatrix.columns());
      else if (labelMatrix.size() > ruleMatrix.rows())
        {
          int rows = ruleMatrix.rows();
          while(labelMatrix.size() > rows)
            {
              labelMatrix.removeLast();
            }
        }
    }

  PiiDefaultOperation::check(reset);
}

void PiiTableLabelerOperation::process()
{
  PII_D;
  PiiVariant featureMatrixVariant = d->pFeatureInput->firstObject();
  if (featureMatrixVariant.isValid())
    {
      PiiMatrix<double> featureMatrix;
      switch (featureMatrixVariant.type())
        {
          PII_FLOAT_MATRIX_CASES(featureMatrix = PiiYdin::matrixAs, featureMatrixVariant);
          PII_INTEGER_MATRIX_CASES(featureMatrix = PiiYdin::matrixAs, featureMatrixVariant);
        default:
          PII_THROW_UNKNOWN_TYPE(d->pFeatureInput);
        }
      
      if (featureMatrix.rows() != 1)  
        PII_THROW_WRONG_SIZE(d->pFeatureInput, featureMatrix, 1, d->layerParser.enabledLayerCount());

      // Emit labels for each layer
      for (int i=0; i<d->lstOutputs.size(); i++)
        {
          outputAt(i)->emitObject(labelForVector(i, featureMatrix));
        }
      
    }
}

/* This private function returns the double valued label for the
   feature vector for the given layer. It is assumed that the vector is stored in a
   matrix of size <1xN> and the length of the the vector
   (N) equals to amount of features stored in the operation. It is
   assumed in the function that the rule matrices and the label vectors
   has been set to the all layers. Also the amount of rows in a rule
   matrix in a layer must match with the length of the label vector
   (_labelMatrix[i], where i is the layer index) for the layer. If the
   rule matrix exists for the layer, it is checked in the function that
   the the length of the feature vector don't exceed the width of the
   rule matrix.

   REVIEW THIS:It shoud be noted, that the length of the label vector
   can exceed the amount of the rows in a layer, but the length of the
   label vector cannot be smaller than the amount of the rows
   */
double PiiTableLabelerOperation::labelForVector(int layer, const PiiMatrix< double >& featureMatrix) const
{
  const PII_D;
  //This function assumes, that the amount of layers and size of the
  //rule matrices and label matrix are in sync. For example the amount
  //of rule matrices must be at least equal to the amount of layers.
  //Also the amount of labels in a row in a label matrix must be at
  //least equal to the amount of rows in the corresponding rule matrix
  //for a layer. The functions setRuleMatrices() and setLabelMatrix()
  //takes care of these synchronizations. If those functions have been
  //used for setting the rule matrices and laber matrix, caller of this
  //function doesn't have to care about the synchronization things.
  

  // Get the rule matrix for the given layer.
  PiiMatrix<PiiMinMax> ruleMatrix = d->lstRuleMatrices[layer];

  //get the row rule vector for the given layer
  QList<bool> lstRowRules = d->lstRuleVectors[layer];

  //The width variable determines, how many of the features are used
  //in comparson when going through the rule matrix. It is the minimum
  //value of the width of the ruleMatrix and the length of the
  //featureVector. Taking the minimum value of these two makes the
  //comparison possible also in the cases when there are not enough
  //features in the feature vector compared to the rule matrix and when
  //there are too many features in the feature vector.

  //int width = ruleMatrix.columns();
  int width = qMin(ruleMatrix.columns(), featureMatrix.columns());

  /*std::cout << "RuleMatrix:" << std::endl;
  for (int r=0; r<ruleMatrix.rows(); r++)
    {
      for (int c=0; c<ruleMatrix.columns(); c++)
        std::cout << " (" << ruleMatrix(r,c).dMin << "-" << ruleMatrix(r,c).dMax << ")";
      std::cout << std::endl;
    }

  std::cout << "FeatureMatrix:" << std::endl;
  for (int r=0; r<featureMatrix.rows(); r++)
    {
      for (int c=0; c<featureMatrix.columns(); c++)
        std::cout << " " << featureMatrix(r,c);
      std::cout << std::endl;
    }
  */
  
  // Go through the rows until there is match in the rule. 
  for (int r=0; r<ruleMatrix.rows(); ++r)
    {
      bool bAnd = lstRowRules[r];
      bool bAllTrue = true;
      PiiMinMax* row = ruleMatrix.row(r);
      
      for (int c=0; c<width; ++c)
        {
          // If the rule is not defined, we don't care this column
          bool bRuleDefined = row[c].dMin > -INFINITY || row[c].dMax < INFINITY;
          
          // Compare the c'th feature in the feature vector to the
          // minimum and maximum value of the curren item.
          bool bColumnTrue = featureMatrix(0,c) >= row[c].dMin && featureMatrix(0,c) <= row[c].dMax;
          
          //if the rule is AND and at least one column is false, we
          //can skip this row
          if (bAnd && !bColumnTrue)
            break;

          //if the rule is OR and at least one column is true, this
          //row is true and we can return the class label
          if (!bAnd && bColumnTrue && bRuleDefined)
            {
              //Match for the rule was found, return the label value of the
              //row
              PiiLayerParser::LayerType layerType = d->layerParser.layerType(layer);
              if (layerType == PiiLayerParser::LayerTypeClass)
                {
                  // Convert the internally stored global class indices
                  // to real indices, before returning the label.
                  return double(d->layerParser.realIndex(layer, int(d->labelMatrix[layer][r])));
                }
              else
                {
                  // Types Float and Binary.
                  return d->labelMatrix[layer][r];
                }
            }

          bAllTrue &= bColumnTrue;
          
          if (c == width-1 && bAllTrue)
            {
              //Match for the rule was found, return the label value of the
              //row
              PiiLayerParser::LayerType layerType = d->layerParser.layerType(layer);
              if (layerType == PiiLayerParser::LayerTypeClass)
                {
                  // Convert the internally stored global class indices
                  // to real indices, before returning the label.
                  return double(d->layerParser.realIndex(layer, int(d->labelMatrix[layer][r])));
                }
              else
                {
                  // Types Float and Binary.
                  return d->labelMatrix[layer][r];
                }
            }  
        }
    }

  //No match for the rule has been found. The default value, 0.0, is
  //returned.
  return d->defaultLabel;
}

void PiiTableLabelerOperation::setFeatureProperties(const QVariantList& featureProperties)
{
  PII_D;
  QStringList similarFeatureNames;
  QList< PiiMatrix< PiiMinMax > > lstRulesForSimilarFeatures;
  rulesForSimilarFeatures(featureProperties, similarFeatureNames, lstRulesForSimilarFeatures);

  d->lstFeatureProperties = featureProperties;

  // If there were no similar features in the old and new feature list,
  // the label matrix is cleared.
  if (similarFeatureNames.size() == 0)
    {
      for (int i = 0; i<d->labelMatrix.size(); i++)
        d->labelMatrix[i].clear();
    }
  
  // Clear extra rows from the rule matrices.
  removeRowsFromRuleMatrices();
  // Update the width of the rule matrix based on the length
  // of the feature name list.
  updateWidthsOfRuleMatrices();
  
  setRulesForFeatures(similarFeatureNames, lstRulesForSimilarFeatures);
  
  if (d->pConfigurator)
    {
      d->pConfigurator->modelContainer()->setFeatureProperties(featureProperties);
      // The following function picks the current rule matrices and
      // the label matrix from the operation.
      d->pConfigurator->reset();
    }
}

void PiiTableLabelerOperation::setLayers(const QVariantList& layers)
{
  PII_D;
  /* PENDING
   * It must be checked if the content of layers differs too much from
   * the old layers. For example, if a layer differ only in terms of
   * the class names (for class type layer), only the new class names
   * should be delivered to the user interfece, but the content of the
   * rule matrices must not be changed. If the type of the layer
   * changes, then the data (rule matrixes and label matrix) msut be
   * cleared first.
   */
  
  //d->lstRuleMatrices.clear(); // DO WE NEED THIS?? CHECK LATER!!
  
  d->layerParser.setLayers(layers);
  QStringList descriptions = d->layerParser.socketDescriptions();

  setNumberedOutputs (layers.size(), 0, "label");
  
  // Add new dummy rule matrices to avoid a segmentation fault, in
  // case the rule matrices will not be explicitely set later.
  while (d->lstRuleMatrices.size() < layers.size())
     {
       addEmptyMatrixToList<PiiMinMax>(d->lstRuleMatrices);
     }

  // If the amount of rule matrices is greated than the amount of the
  // layers to be set, extra matrices are removed from the list of the
  // rule matrices.
  while (d->lstRuleMatrices.size() > layers.size())
     {
       d->lstRuleMatrices.removeLast();
     }

  // Update the width of the rule matrix based on the length
  // of the feature name list. This must be done in case the rule
  // matrices are not initialized at all.
  updateWidthsOfRuleMatrices();  

  // Make the label matrix consistent with the rule matrices just set,
  // to avoid segmentation faults.
  makeDataConsistent();

  // Checks the consistence between the layers, rulematrices and
  // labels. If the layer is of type class, and the global class index
  // corresponding to the label for the row doesn't exist in the
  // layer, the row is remover from the rule matrix and the label from
  // the label vector. 
  if (true ==removeUnusedRows())
    {
      // Update the configurator
      d->pConfigurator->modelContainer()->setContent(d->lstRuleMatrices, d->labelMatrix, d->lstRuleVectors);
    }
  
  if (d->pConfigurator)
    {
      //TODO: ADD SOME CHECKS HERE.
      d->pConfigurator->modelContainer()->setLayers(layers);
      //d->pConfigurator->setLayers(layers);
    }
}

QWidget* PiiTableLabelerOperation::createUiComponent(const QString& name)
{
  PII_D;
  if (name == "configurator")
    {
      // This makes sure, that only one instance for the configurator
      // can exist.
      if (d->pConfigurator)
        return d->pConfigurator;

      //Create the UI-component. The model containter will be created
      //for the UI-component at the same time automatically.
      d->pConfigurator = new PiiTableLabelerConfigurator( 0 );
      
      PiiTableLabelerModelContainer* modelContainer = d->pConfigurator->modelContainer();
      
      modelContainer->initialize(d->layerParser.layers(),
                                 d->lstFeatureProperties,
                                 rawRuleMatrices(),
                                 rawLabels(),
                                 rawRuleVectors(),
                                 this);
      
      // This ensures that the configator widget really gets destroyed, when it
      // is destroyed in the application.      
      connect(d->pConfigurator, SIGNAL(destroyed(QObject*)), SLOT(uiComponentDestroyed(QObject*)));

      // If the toolbar widget exists, connect the signals between
      // it and the configurator object, which was just created.
      if (d->pToolBar)
        connectUiComponents();

      return d->pConfigurator;
    }
  else if (name == "toolbar")
    {
      // This makes sure, that only one instance for the toolbar
      // can exist.
      if (d->pToolBar)
        return d->pToolBar;
      
      d->pToolBar = new PiiTableLabelerToolBar;

      // This ensures that the toolbar really gets destroyed, when it
      // is destroyed in the application.
      connect(d->pToolBar, SIGNAL(destroyed(QObject*)), SLOT(uiComponentDestroyed(QObject*)));

      // If the configurator widget exists, connect the signals between
      // it and the toolbar object, which was just created.
      if (d->pConfigurator)
        connectUiComponents();
      
      return d->pToolBar;
    }
  return 0;
}

/* This is a private slot, which is called, when some of the ui
 *  components created by function createUiComponent is destroyed.
 * The slot sets the corresponding widget member variable to zero.
 */
void PiiTableLabelerOperation::uiComponentDestroyed(QObject* object)
{
  PII_D;
  QWidget* widget = qobject_cast<QWidget*>(object);
  if (widget == d->pConfigurator)
    d->pConfigurator = 0;
  else if (widget == d->pToolBar)
    d->pToolBar = 0;
}

/* This is a private helper function, which connects the signals
   between the "configurator" and "toolbar" widgets. If neither of the
   configurator has not been created, the function doesn nothing.
 */
void PiiTableLabelerOperation::connectUiComponents()
{
  PII_D;
  PiiTableLabelerToolBar* toolbar = qobject_cast<PiiTableLabelerToolBar*>(d->pToolBar);
  PiiTableLabelerConfigurator* configurator = qobject_cast<PiiTableLabelerConfigurator*>(d->pConfigurator);
  if (!toolbar || !configurator)
    return;
  
  connect(toolbar->buttonAddRow(),SIGNAL(clicked()),configurator,SLOT(addRow()));
  connect(toolbar->buttonRemoveRow(),SIGNAL(clicked()),configurator,SLOT(removeRow()));
  connect(toolbar->buttonMoveRowUp(),SIGNAL(clicked()),configurator,SLOT(moveRowUp()));
  connect(toolbar->buttonMoveRowDown(),SIGNAL(clicked()),configurator,SLOT(moveRowDown()));
  connect(toolbar->buttonChangeRuleType(),SIGNAL(clicked()),configurator,SLOT(changeRuleType()));

  connect(configurator,SIGNAL(addingRowsEnabled(bool)),toolbar->buttonAddRow(),SLOT(setEnabled(bool)));
  connect(configurator,SIGNAL(removingRowsEnabled(bool)),toolbar->buttonRemoveRow(),SLOT(setEnabled(bool)));
  connect(configurator,SIGNAL(movingRowsUpEnabled(bool)),toolbar->buttonMoveRowUp(),SLOT(setEnabled(bool)));
  connect(configurator,SIGNAL(movingRowsDownEnabled(bool)),toolbar->buttonMoveRowDown(),SLOT(setEnabled(bool)));
  connect(configurator,SIGNAL(changeRuleTypesEnabled(bool)),toolbar->buttonChangeRuleType(),SLOT(setEnabled(bool)));
  //Inform the configurator, that a new controller (toolbar) has been added.
  configurator->controllerAdded();
}

QStringList PiiTableLabelerOperation::supportedUiComponents() const
{
  return QStringList() << "configurator" << "toolbar";
}

/* This is a private function, which adds an empty matrix to the list
   of rule matrices member variable (d->lstRuleMatrices) */
template <class T> void PiiTableLabelerOperation::addEmptyMatrixToList(QList< PiiMatrix<T> >& lstMatrices) const
{
  
  PiiMatrix<T> matrix = PiiMatrix<T>();
  lstMatrices << matrix;

}

/* This is a private function, which fills missing labels in
 * d->labelMatrix for the given layer with the default value @p
 * d->defaultLabel. The label vector for the given
 * layer must exist before calling this function, but the size of the
 * vector can be zero. If the size of the label vector for the given
 * layer is equal or larger than the amount of the rows in the rule
 *  matrix, neither new items are added to the vector nor items are
 * removed from the vector.
 */
void PiiTableLabelerOperation::fillMissingLabelsInVector(int layer)
{
  PII_D;

  if (d->labelMatrix[layer].size() >= d->lstRuleMatrices[layer].rows())
    {
      //Nothing needs to be done.
      return;
    }

  //start from end of teh vector, and fill it with the default values.
  for (int i = d->labelMatrix[layer].size() ; i < d->lstRuleMatrices[layer].rows(); ++i)
    {
      d->labelMatrix[layer] << d->defaultLabel;
    }

}

void PiiTableLabelerOperation::fillMissingRuleVectorsInMatrix()
{
  PII_D;
  for (int i=0; i < d->lstRuleMatrices.size(); ++i)
    {
      // If rule vector for the layer doesn't yet exists, create a
      // new empty vector, and add it to the label matrix.
      if (d->lstRuleVectors.size() == i)
        {
          QList<bool> ruleVector;
          d->lstRuleVectors << ruleVector;
        }
      fillMissingRulesInVector(i);
    }
}

void PiiTableLabelerOperation::fillMissingRulesInVector(int layer)
{
  PII_D;
  if (d->lstRuleVectors[layer].size() >= d->lstRuleMatrices[layer].rows())
    {
      //Nothing needs to be done.
      return;
    }

  //start from end of teh vector, and fill it with the default values.
  for (int i=d->lstRuleVectors[layer].size(); i<d->lstRuleMatrices[layer].rows(); ++i)
    d->lstRuleVectors[layer] << true;
}

/* This is a private function, which fills missing labels in
   all label vectors in d->labelMatrix, and also adds new vectors in the
   matrix if needed. The function goes through the rule matrices for
   the all layers. If the label vector for a layer is missing, a new
   vector is added. If the amount of items in a vector is
   smaller, than the amount of rows in the same layer, new items are
   added in the vector, vith the default values.
*/
void PiiTableLabelerOperation::fillMissingLabelsInMatrix()
{
  PII_D;

  for (int i=0; i < d->lstRuleMatrices.size(); ++i)
    {
      // If label vector for the layer doesn't yet exists, create a
      // new empty vector, and add it to the label matrix.
      if (d->labelMatrix.size() == i)
        {
          QList<double> labelVector;
          d->labelMatrix << labelVector;
        }
      fillMissingLabelsInVector(i);
    }

}

void PiiTableLabelerOperation::setRuleMatrices(const QVariantList& ruleMatrices)
{
  PII_D;
  d->lstRuleMatrices = ruleMatricesToInternal(ruleMatrices);
  //setRawRuleMatrices(ruleMatricesToInternal(ruleMatrices));

  makeDataConsistent();
  
}

QList< PiiMatrix< PiiMinMax > > PiiTableLabelerOperation::rawRuleMatrices() const
{
  const PII_D;
  QList< PiiMatrix< PiiMinMax > > retMatrix;
  for (int i = 0; i<d->lstRuleMatrices.size(); ++i)
    retMatrix << d->lstRuleMatrices[i];
  return retMatrix;
}

void PiiTableLabelerOperation::setRawRuleMatrices(const QList< PiiMatrix< PiiMinMax > >& ruleMatrices)
{
  PII_D;

  //Assign the copy of ruleMatrices to the member variable d->lstRuleMatrices.
  d->lstRuleMatrices.clear();
  for (int i = 0; i<ruleMatrices.size(); ++i)
    d->lstRuleMatrices << ruleMatrices[i];
  
  makeDataConsistent();
                            
}

/* This is a private helper function, which makes the size of label
 * vectors in d->lableMatrix consistent with the size (amount of rows)
 * of the rule matrices in d->lstRuleMatrices, and the amount of
 * ruleMatrices consistent with the amount of layers. When calling
 * this function one can be sure that during running of operation
 * there will not be segmentation faults.
 */
void PiiTableLabelerOperation::makeDataConsistent()
{
  PII_D;
  // Fill the missing labels in the label matrix. The amount of rows in
  // the label matrix should be at least the amount of layers in the
  // operation, and the amount of column in a single row should be at
  // least amount of rows in the rule matrix in the layer.
  fillMissingLabelsInMatrix();
  fillMissingRuleVectorsInMatrix();

  // Make sure, that the amount of rule matrices will be at least equal
  // to the amount of layers.
  while (d->lstRuleMatrices.size() < d->layerParser.layers().size() )
    {
      addEmptyMatrixToList<PiiMinMax>( d->lstRuleMatrices);
    }
}

/* Sets the widths of the rule matrices same than the length of the ...at least as wide as
 * the length of list of feature names (d->lstFeatureProperties). If the
 * width of a rule matrix are allready wider, it is not set smalled. */
void PiiTableLabelerOperation::updateWidthsOfRuleMatrices()
{
  PII_D;
  // Loop the rule matrices.
  for (int i = 0; i<d->lstRuleMatrices.size(); i++)
    {
      int oldWidth = d->lstRuleMatrices[i].columns(); 
      if (oldWidth<d->lstFeatureProperties.size())
        {
          // The matrix was too narrow.

          // Reserve memory for the new columns.
          d->lstRuleMatrices[i].resize(d->lstRuleMatrices[i].rows(),d->lstFeatureProperties.size());
          // Loop the rows in a rule matrix.
          for (int j = 0; j<d->lstRuleMatrices[i].rows(); j++)
            {
              // Initialize the new columns in a row.
              for (int k = oldWidth; k<d->lstFeatureProperties.size(); k++)
                d->lstRuleMatrices[i](j,k).clear();
            }
        }
      else if (oldWidth>d->lstFeatureProperties.size())
        // The matrix were too wide.
        d->lstRuleMatrices[i].resize(d->lstRuleMatrices[i].rows(),d->lstFeatureProperties.size());
    }
}


/* Checks the consistence between the layers, rulematrices and
 * labels. If the layer is of type class, and the global class index
 * corresponding to the label for the row doesn't exist in the
 * layer, the row is removed from the rule matrix and the label from
 * the label vector. If any removals has been done, @p true is
 * returned from the function. Else @p false is returned. */
bool PiiTableLabelerOperation::removeUnusedRows()
{
  PII_D;
  bool changed = false;
  // Loop the vectors in label matrix. Each label vector corresponds
  // to a layer.
  for (int i=d->labelMatrix.size(); i--;)
    {
      PiiLayerParser::LayerType layerType = d->layerParser.layerType(i);
      if (layerType == PiiLayerParser::LayerTypeClass)
        {
          // Loop the items in the label matrix. Each label in the vector
          // corresponds to a row in the rule matrix.
          for (int j=d->labelMatrix[i].size(); j--;)
            {
              if (d->layerParser.checkIndexExistence(i, int(d->labelMatrix[i][j])) == false)
                {
                  // Remove the row.
                  d->lstRuleMatrices[i].removeRow(j);
                  d->labelMatrix[i].removeAt(j);
                  changed = true;
                }
            }
        }
    }

  return changed;
}

/* This function checks, which features in member variable
   d->lstFeatureProperties are in the parameter
   newFeatureProperties. The comparison is made based on the feature
   name. The feature unit and scale are not used in the comparison.
   The names of the similar features are returned in the list
   similarFeatureNames, and the rules corresponding each feature are
   returned in the parameter rulesForSimilarFeatures. The latter
   parameter contains a matrix for each layer. Each matrix has a column
   for a feature. The rules (minimum and maximum values) for a
   feature is stored in a column.
 */
void PiiTableLabelerOperation::rulesForSimilarFeatures(const QVariantList& newFeatureProperties,
                                                       QStringList& similarFeatureNames,
                                                       QList< PiiMatrix< PiiMinMax > >& rulesForSimilarFeatures) const
{
  const PII_D;
  similarFeatureNames.clear();
  rulesForSimilarFeatures.clear();

  QList<int> similarFeatureNewIndices;
  
  // Go through all new features, and store the names of similar
  // features compared to old features.
  for (int i = 0; i<newFeatureProperties.size(); i++)
    {
      QVariantMap map = newFeatureProperties[i].toMap();
      if (map.contains("name"))
        {
          QString featureName = map["name"].toString();
          // Check, if the feature exists in the old list of features.
          // If yes, add it in the list returned from this function.-
          if (featureExists(featureName))
            {
              similarFeatureNames << featureName;
              similarFeatureNewIndices << i;
            }
        }
    }

  // Create the matrices for storing temporarily the rules.
  while(rulesForSimilarFeatures.size() < d->lstRuleMatrices.size())
    {
      addEmptyMatrixToList<PiiMinMax>(rulesForSimilarFeatures);
      int index = rulesForSimilarFeatures.size()-1;
      // Add the required amount of columns in the matrix
      rulesForSimilarFeatures[index].resize(0,similarFeatureNames.size());
    }

  // Copy the rules.
  //similar features
  for (int i = 0; i<similarFeatureNames.size(); i++)
    {
      // Get the feature index in the d->lstFeatureProperties.
      int featureIndex = indexForFeature(similarFeatureNames[i]);
      if (featureIndex>=0)
        {
          //each layer
          for (int j = 0; j<d->lstRuleMatrices.size(); j++)
            {
              // Add new rows in the matrix, if needed
              if (rulesForSimilarFeatures[j].rows()<d->lstRuleMatrices[j].rows())
                rulesForSimilarFeatures[j].resize(d->lstRuleMatrices[j].rows(),rulesForSimilarFeatures[j].columns());

              // Copy a column from rule matrix
              for (int k = 0; k<d->lstRuleMatrices[j].rows(); k++)
                {
                  // Copy a single minmax-item.
                  rulesForSimilarFeatures[j](k, i) = d->lstRuleMatrices[j](k, featureIndex);
                  
                  // Fix the class name indices if possible
                  if (d->lstFeatureProperties[featureIndex].toMap().contains("classNames"))
                    {
                      QVariantMap oldMap = d->lstFeatureProperties[featureIndex].toMap();
                      QVariantMap newMap = newFeatureProperties[similarFeatureNewIndices[i]].toMap();
                      int oldIndex = int(d->lstRuleMatrices[j](k, featureIndex).dMin + 0.5);
                      int newIndex = -1;

                      if (oldIndex >= 0)
                        {
                          if (oldMap.contains("classIndices") && newMap.contains("classIndices"))
                            {
                              QVariantList oldIndices = oldMap.value("classIndices").toList();
                              QVariantList newIndices = newMap.value("classIndices").toList();
                              newIndex = newIndices.indexOf(oldIndices[oldIndex]);
                            }
                          else
                            {
                              QStringList oldNames = oldMap.value("classNames").toStringList();
                              QStringList newNames = newMap.value("classNames").toStringList();
                              newIndex = newNames.indexOf(oldNames[oldIndex]);
                            }
                        }
                      double dMin = (double)newIndex;
                      double dMax = (double)newIndex;
                      
                      if (newIndex < 0)
                        {
                          dMin = -INFINITY;
                          dMax = INFINITY;
                        }
                      rulesForSimilarFeatures[j](k,i) = PiiMinMax(dMin,dMax);
                    }
                }
            }
        }
    }
  
}

/* This function returns the index of the feature in the member
   variable d->lstFeatureProperties having the feature name given as parameter.
*/
int PiiTableLabelerOperation::indexForFeature(const QString& featureName) const
{
  const PII_D;
  for (int i = 0; i<d->lstFeatureProperties.size(); i++)
    {
      QVariantMap map = d->lstFeatureProperties[i].toMap();
      if (map.contains("name"))
        {
          if (featureName == map["name"].toString())
            return i;
        }
    }
  return -1;
}

/* Checks if the feature given as a parameter exists in the member
   variable d->lstFeatureProperties. */
bool PiiTableLabelerOperation::featureExists(const QString& featureName) const
{
  return indexForFeature(featureName)>=0;
}

/* This function copies the rules from the parameter rulesForFeatures
   to the member variable d->lstRuleMatrices for the features that are
   stored in the parameter featureNames. For each feature stored in the
   paremeter, corresponding index is searched from the member variable
   d->lstFeatureProperties, and the rules for the feature are copied
   from the parameter rulesForFeatures to the column corresponding to
   the index found. The function also clears rules for the features in
   the member variable d->lstRuleMatrices that don't exist in the parameter
   featureNames.  
 */
void PiiTableLabelerOperation::setRulesForFeatures(const QStringList& featureNames,
                                                          const QList< PiiMatrix< PiiMinMax > >& rulesForFeatures)
{
  PII_D;
  // Go through each feature.
  for (int i = 0; i<d->lstFeatureProperties.size(); i++)
    {
      // Go through the rule matrices for each layer.
      for (int j = 0; j<d->lstRuleMatrices.size(); j++)
        {
          if (j>=rulesForFeatures.size() || i >= d->lstRuleMatrices[j].columns())
            continue;
          // Add new rows in the rule matrix, if needed
          if (d->lstRuleMatrices[j].rows()<rulesForFeatures[j].rows())
            d->lstRuleMatrices[j].resize(rulesForFeatures[j].rows(),d->lstRuleMatrices[j].columns());

          QString featureName = d->lstFeatureProperties[i].toMap().value("name").toString();
          int featureIndex = featureNames.indexOf(featureName);
          
          // Copy a column to the rule matrix (or clear the column).
          for (int k = 0; k<rulesForFeatures[j].rows(); k++)
            {
              if (featureIndex>=0)
                // Copy a single minmax-item.
                d->lstRuleMatrices[j](k, i) = rulesForFeatures[j](k, featureIndex);
              else
                // Clear the item (sets the -INF and INF values for
                // min and max.
                d->lstRuleMatrices[j](k, i).clear();
            }
          
        }
    }
}

/* Remove the all the rows from the rule matrices. Preserves the
   amount of colmuns. */
void PiiTableLabelerOperation::removeRowsFromRuleMatrices()
{
  PII_D;
  for (int i = 0; i<d->lstRuleMatrices.size(); i++)
    {
      d->lstRuleMatrices[i].resize(0,d->lstRuleMatrices[i].columns());
    }
}

/* This private function converts the rule
   matrices from the external format to the internal format. The external
   format is given as a parameter and the internal format of the
   matrix is returned from the function.

   The difference between the external and internal format is how the
   minimum and maximum values are stored in the matrix. In the
   external format a single matrix is of type PiiMatrix<double>, where
   the minimum and maximum values are stored adjacent in places in the
   a rule matric. The internal format of matrix is
   PiiMatrix<PiiMinMax>, where the minimum and maximum values are
   stored in PiiMinMax structure. In the extrenal format the
   PiiMatrix<double> data structures are converted to QVariants
   through PiiVariant structures.*/
QList< PiiMatrix< PiiMinMax > > PiiTableLabelerOperation::ruleMatricesToInternal(const QVariantList& doubleMatrices) const
{
  QList< PiiMatrix< PiiMinMax > > retMatrices;

  //Go through all matrices
  for (int i = 0; i < doubleMatrices.size(); ++i )
    {
      if (doubleMatrices[i].canConvert<PiiVariant>())
        {
          PiiVariant variant = doubleMatrices[i].value<PiiVariant>();
          //Check that the matrix is of correct type
          if (variant.type() != PiiYdin::DoubleMatrixType)
            {
              //Error
              addEmptyMatrixToList<PiiMinMax>( retMatrices);
              continue;
            }
          
          const PiiMatrix<double>& doubleMat = variant.valueAs<PiiMatrix<double> >(); 
          
          //Check that the amount of columns in the matrix is even
          if (doubleMat.columns() % 2 != 0)
            {
              //Error
              addEmptyMatrixToList<PiiMinMax>( retMatrices);
              continue;
            }
           
          
          PiiMatrix<PiiMinMax> minMaxMat = PiiMatrix<PiiMinMax>(doubleMat.rows(), doubleMat.columns()/2);
          //Go through the rows of a single matrix
          for (int j = 0; j < doubleMat.rows(); ++j)
            {
              //Create a new rule matrix for minMax matrix.
              //PiiMatrix<PiiMinMax> minMaxRow = PiiMatrix<PiiMinMax>(1, minMaxMat.columns());

              //Go through the columns
              for (int k = 0; k < doubleMat.columns(); k += 2)
                {
                  minMaxMat(j,k/2) = PiiMinMax(doubleMat(j,k),doubleMat(j,k+1));
                }    
            }
          retMatrices << minMaxMat;
        }
      else
        {
          //Error
          addEmptyMatrixToList<PiiMinMax>( retMatrices);
        }

    }

  return retMatrices;
}

/* PENDING
   THIS FUNCTION HAS NOT BEEN TESTED.

   This private function converts the rule
   matrices from the external format to the internal format. The external
   format is given as a parameter and the internal format of the
   matrix is returned from the function.

   The difference between the external and internal format is how the
   minimum and maximum values are stored in the matrix. In
   the external format a single matrix is of type PiiMatrix<double>,
   where the minimum and maximum values are stored
   adjacent in places in the a rule matric. In the external format type
   of matrix is PiiMatrix<PiiMinMax>, where the minimum and maximum values are
   stored in PiiMinMax structure. In the extrenal format the
   PiiMatrix<double> data structures are converted to QVariants
   through PiiVariant structures.*/
QVariantList PiiTableLabelerOperation::ruleMatricesToExternal(const QList<PiiMatrix<PiiMinMax> >& minMaxMatrices) const
{
  QVariantList retMatrices;

  //Go through all matrices
  for (int i = 0; i < minMaxMatrices.size(); ++i)
    {
      PiiMatrix<double> doubleMat(minMaxMatrices[i].rows(), 2*minMaxMatrices[i].columns());
      const PiiMatrix< PiiMinMax>& minMaxMat = minMaxMatrices[i];
          
      //PiiMatrix<PiiMinMax> minMaxMat = PiiMatrix<PiiMinMax>(doubleMat.rows(), doubleMat.columns()/2);

      //Go through the rows of a single matrix
      for (int j = 0; j < minMaxMat.rows(); ++j)
        {
          //Create a new rule matrix for minMax matrix.
          //PiiMatrix<PiiMinMax> minMaxRow = PiiMatrix<PiiMinMax>(1, minMaxMat.columns());

          //Go through the columns
          for (int k = 0; k < minMaxMat.columns(); ++k)
            {
              doubleMat(j,k*2) = minMaxMat(j,k).dMin;
              doubleMat(j,k*2+1) = minMaxMat(j,k).dMax;
            }    
        }
      retMatrices << Pii::createQVariant(doubleMat);
    }

  return retMatrices;
}

void PiiTableLabelerOperation::setLabels(const QList< QVariant >& labelMatrix)
{
  setRawLabels(labelMatrixToInternal(labelMatrix));
}

void PiiTableLabelerOperation::setRawLabels(const QList< QList<double> >& labelMatrix)
{
  PII_D;
  d->labelMatrix = labelMatrix;

  //Complete the label matrix, if some items in vectors or some
  //vectors are missing.x
  fillMissingLabelsInMatrix();
}

void PiiTableLabelerOperation::setRules(const QList< QVariant >& ruleVectors)
{
  setRawRuleVectors(ruleVectorsToInternal(ruleVectors));
}

void PiiTableLabelerOperation::setRawRuleVectors(const QList< QList<bool> >& ruleVectors)
{
  PII_D;
  d->lstRuleVectors = ruleVectors;
  fillMissingRuleVectorsInMatrix();
}

/* This private function converts the label
   matrices from the external format to the internal format. The external
   format is given as a parameter and the internal format of the
   matrix is returned from the function.

   The difference between the external and internal format is in how the
   lists are stored in the memory. In the internal type the label
   matrix is stored in the format QList< QList<double> >. It is the
   list of lists, where the list of doubles are the items in another
   list. In the extrenal format the outer matrix is of type
   QList<QVariant>, where the items contain lists of type
   QList<QVariant>, where the items are doubles.  */
QList< QList<double> > PiiTableLabelerOperation::labelMatrixToInternal(const QList< QVariant >& labels) const
{
  const PII_D;
  QList< QList<double> > retMatrix;
  
  //Loop the layers (outer list)
  for (int i = 0; i<labels.size(); ++i)
    {
      //New list of doubles
      QList<double> labelList;

      //Temporary variant list
      QList<QVariant> variantList = labels[i].toList();

      //Loop the labels in a layer
      for (int j = 0; j < variantList.size(); ++j)
        {
          //Add the label (either given or default)
          //to the list of labels.
          if (variantList[j].canConvert(QVariant::Double))
            labelList << variantList[j].toDouble();
          else
            labelList << d->defaultLabel;
        }
      retMatrix << labelList;
    }

  return retMatrix;
}

QList< QList<bool> > PiiTableLabelerOperation::ruleVectorsToInternal(const QList< QVariant >& ruleVectors) const
{
  QList< QList<bool> > retMatrix;
  
  //Loop the layers (outer list)
  for (int i = 0; i<ruleVectors.size(); ++i)
    {
      //New list of booleans
      QList<bool> ruleVectorList;

      //Temporary variant list
      QList<QVariant> variantList = ruleVectors[i].toList();

      //Loop the ruleVectors in a layer
      for (int j = 0; j < variantList.size(); ++j)
        {
          //Add the label (either given or default)
          //to the list of labels.
          if (variantList[j].canConvert(QVariant::Bool))
            ruleVectorList << variantList[j].toBool();
          else
            ruleVectorList << true;
        }
      retMatrix << ruleVectorList;
    }

  return retMatrix;
}


/* PENDING
   THIS FUNCTION HAS NOT BEEN TESTED.

   This private function converts the label
   matrices from the internal format to the external format. The internal
   format is given as a parameter and the external format of the
   matrix is returned from the function.

   The difference between the external and internal format is in how the
   lists are stored in the memory. See the comments of function
   lableMatrixToInternal for a more information.  */
QList< QVariant > PiiTableLabelerOperation::labelMatrixToExternal(const QList< QList<double> >& labels) const
{
  QList< QVariant > retMatrix;
  
  //Loop the layers (outer list)
  for (int i = 0; i<labels.size(); ++i)
    {
      //New list of double, which are converted to QVariants
      QList< QVariant > variantList;

      //Temporary double list
      const QList<double>& doubleList = labels[i];

      //Loop the labels in a layer
      for (int j = 0; j < doubleList.size(); ++j)
        {
          variantList << QVariant(doubleList[j]);
        }
      retMatrix << QVariant(variantList);
    }

  return retMatrix;
}

QList< QVariant > PiiTableLabelerOperation::ruleVectorsToExternal(const QList< QList<bool> >& ruleVectors) const
{
  QList< QVariant > retMatrix;
  
  //Loop the layers (outer list)
  for (int i = 0; i<ruleVectors.size(); ++i)
    {
      //New list of bool, which are converted to QVariants
      QList< QVariant > variantList;

      //Temporary bool list
      const QList<bool>& boolList = ruleVectors[i];

      //Loop the ruleVectors in a layer
      for (int j = 0; j < boolList.size(); ++j)
        {
          variantList << QVariant(boolList[j]);
        }
      retMatrix << QVariant(variantList);
    }

  return retMatrix;
}

QVariantList PiiTableLabelerOperation::featureProperties() const { return _d()->lstFeatureProperties; }
QVariantList PiiTableLabelerOperation::layers() const { return _d()->layerParser.layers(); }
QVariantList PiiTableLabelerOperation::ruleMatrices() const { return ruleMatricesToExternal(_d()->lstRuleMatrices); }
QList< QVariant > PiiTableLabelerOperation::labels() const { return labelMatrixToExternal(_d()->labelMatrix); }
QList< QVariant > PiiTableLabelerOperation::rules() const { return ruleVectorsToExternal(_d()->lstRuleVectors); }
QList< QList<double> > PiiTableLabelerOperation::rawLabels() const { return _d()->labelMatrix; }
QList< QList<bool> > PiiTableLabelerOperation::rawRuleVectors() const { return _d()->lstRuleVectors; }

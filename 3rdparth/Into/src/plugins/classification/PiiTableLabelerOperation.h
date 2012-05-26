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

#ifndef _PIITABLELABELEROPERATION_H
#define _PIITABLELABELEROPERATION_H


#include <PiiDefaultOperation.h>
#include <PiiLayerParser.h>
#include <QList>
#include <QPair>
#include <PiiMatrix.h>
#include <QVariant>
#include <PiiMath.h> //for INFINITY values.
#include <QtDebug>

/**
 * This class is used for storing the minimum and maximum values of a
 * double. The reason for implementing a separate class for this
 * purpose was, that QPair cannot be used as an item of PiiMatrix. So, a
 * new class had to be implemented, which is accepted by PiiMatrix.
 * PiiMatrix doesn't accept items that doesn't have a constructor,
 * which has exactly one parameter.
 */
class PiiMinMax
{
public:
  //PiiMinMax() : dMin(0), dMax(0)
  //{ }
  /**
   * The reason for implementing this constructor is to get it to work
   * with PiiMatrix data strucuture. Using a class as an item of PiiMatrix
   * requires, that it has a constructor, which has exactly one parameter.
   */
  //PiiMinMax(double init) : dMin(init), dMax(init)
  //{ }  
  //PiiMinMax(double min = 0, double max = 0) : dMin(min), dMax(max)
  PiiMinMax(double min = -INFINITY, double max = INFINITY) : dMin(min), dMax(max)
  { }

  /**
   * This constructor is provided for a convinience. A new PiiMinMax
   * object is created from the parameters @p min and @p max by using
   * the rules described in the functions setMin() and setMax().
   */
  PiiMinMax(QString min, QString max)
  {
    setMin(min);
    setMax(max);
  }

  /**
   * This constructor is provided for convinience. It constructs a
   * new PiiMinMax object from the QVariant object given as a
   * paramater. It is assumed, that the QVariant object has been
   * previously created with the function @p toVariant().
   */  
  PiiMinMax(QVariant minMaxValues)
  {
    fromVariant(minMaxValues);
  }

  /**
   * Equality operator.
   */
  bool operator==(const PiiMinMax& other) const
  {
    return (dMin == other.dMin) && (dMax == other.dMax);
  }

  /**
   * Inequality operator.
   */
  bool operator!=(const PiiMinMax& other) const
  {
    return !operator==(other);
  }

  /**
   * Multiplication operator.
   */
  PiiMinMax operator*(double val) const
  {
    return PiiMinMax(this->dMin*val,this->dMax*val);
  }

  /**
   * Division operator.
   */
  PiiMinMax operator/(double val) const
  {
    return PiiMinMax(this->dMin/val,this->dMax/val);
  }

  /**
   * Operator, which multiplies both min and max values with the same value.
   */
  void operator*=(double val)
  {
    this->dMin*=val;
    this->dMax*=val;
  }

  /**
   * Operator, which divides both min and max values with the same value.
   */
  void operator/=(double val)
  {
    this->dMin/=val;
    this->dMax/=val;
  }
  
  /**
   * Return the content of the class in a QMap<QString, QVariant>
   * structure. The map contain the following keys:
   *
   * @li p min, the content of dMin.
   * 
   * @li p max, the content of dMax.
   *
   * The value in each item should be a double converted to QVariant.
   */
  QMap<QString, QVariant> toMap() const
  {
    QMap<QString, QVariant> map;
    map["min"] = dMin;
    map["max"] = dMax;
    return map;
  }
  
  /**
   * Assign the min and max values stored in the parameter @p map to
   * the member variables dMin and dMax. The paramater should contain
   * the QMap<QString, QVariant> data structure, which has the
   * following keys.
   *
   * @li p min, the content of dMin.
   * 
   * @li p max, the content of dMax.
   *
   * The value in each item should a double converted to QVariant.
   */  
  void fromMap(QMap<QString, QVariant> map)
  {
    if (map.contains("min"))
      {
        QVariant min = map["min"];
        if (min.canConvert(QVariant::Double))
          dMin = map["min"].toDouble();
      }
    if (map.contains("max"))
      {
        QVariant max = map["max"];
        if (max.canConvert(QVariant::Double))        
          dMax = map["max"].toDouble();
      }
  }

  /**
   * This function converts the PiiMinMax object to a QVariant object.
   * The function fromVariant can be used to assign the min and max values
   * stored in QVariant object back to a PiiMinMax object.
   */
  QVariant toVariant() const
  {
    return toMap();
  }


  /**
   * Assign the minimum and maximum values stored in the QVariant
   * object given as a parameter to the member variables of the
   * PiiMinMax object. It is assumed that the variant given as a
   * parameter has been created with the function @p toVariant(). If
   * the QVariant object given as a parameter is invalid, zeros are
   * assigned to the dMin and dMax.
   */  
  void fromVariant(QVariant variant)
  {
    if (variant.canConvert(QVariant::Map))
      fromMap(variant.toMap());
    else
      {
        dMin = 0;
        dMax = 0;
      }
  }

  /**
   * This function returns value of dMin as text. If the value of the
   * dMin == -INFINITY, an empty string is returned. Else the value of
   * dMin is returned as a string.  
   */
  QString minAsString() const
  {
    if ( dMin == -INFINITY )
     return "";
   else
     return QString("%1").arg(dMin);
  }

  /**
   * This function returns value of dMin as text. If the value of the
   * dMin == -INFINITY, an empty string is returned. Else the value of
   * dMin is returned as a string.  
   */
  QString maxAsString() const
  {
    if ( dMax == INFINITY )
     return "";
   else
     return QString("%1").arg(dMax);
  }
  
  /**
   * This function converts the @p min value given as a paremter to
   * double, and sets the result to the member variable @p dMin. If the
   * string given as a parameter is empty, the string is converted to
   * -INFINITY value before assigning to @p dMin. Else the string is
   * converted normally to double before assigning.
   */
  void setMin(QString min)
  {
    //The empty string is converted to -INFINITY double value.
    if ( min.isEmpty() )
      dMin = -INFINITY;
    else
      dMin = min.toDouble();
  }

  /**
   * This function converts the @p min value given as a paremter to
   * double, and sets the result to the member variable @p dMax. If the
   * string given as a parameter is empty, the string is converted to
   * INFINITY value before assigning to @p dMax. Else the string is
   * converted normally to double before assigning.
   */
  void setMax(QString max)
  {
    //The empty string is converted to INFINITY double value.
    if ( max.isEmpty() )
      dMax = INFINITY;
    else
      dMax = max.toDouble();
  }

  /**
   * Sets @p dMax to INFINITY and @p dMin to -INFINITY.
   */
  void clear()
  {
      dMax = INFINITY;
      dMin = -INFINITY;
  }

  // Member variables (public).
  double dMin;
  double dMax;
  
  //private:
};

class PiiInputSocket;
class PiiTableLabelerConfigurator;
class PiiTableLabelerToolBar;

/**
 * PiiTableLabelerOperation can be used for classification where the
 * classification rules are given in a table format. The classifier
 * has two ui componets, "configuration" and "toolbar". The
 * "configurator" ui-component is used for setting manually the rules
 * for the configurator and selecting labels for rules.
 *
 * The operation supports several layers.
 *
 * @inputs
 *
 * @in features - feature vector of samples (any rule matrix with a
 * primitive content type)
 *
 * @outputs
 *
 * @out labelX - a classification (double) for the incoming features. 
 * If the operation is not configured, zero (0) is emitted for each
 * incoming object. X denotes the zero-based index of the layer. For
 * example, if there is only one layer, @p label0 emits its
 * classification result. The label can be any floating point value,
 * but its value range and resolution are limited by the layer type. 
 * For class label layers, emitted values are cardinal numbers.
 *
 * @ingroup PiiClassificationPlugin
 */
class PiiTableLabelerOperation : public PiiDefaultOperation
{
  Q_OBJECT

  /**
   * A rather complex property which defines the rules, how feature inputs
   * are converted to the labels in the outputs of the operation. For
   * each entry in this list, a new labeling layer is created. The
   * configuration of the layer is stored as a map of name-value
   * pairs. That is, each QVariant in this list is actually a
   * QMap<QString, QVariant>. The configuration parameters depend on
   * the type of the layer. Refer to the documentation of @p
   * PiiLayerParser to get the more information of the exacts
   * structure of the item in the list.
   */
  Q_PROPERTY(QVariantList layers READ layers WRITE setLayers);

  /**
   * Determines the properties of the features. Contains the list of
   * QVariantMaps, where each map contains the following key-value
   * pairs:
   *
   * @li displayName - visible name of the feature (QString)
   *
   * @li name - id of the feature.
   *
   * @li unit - unit of the feature (QString)
   *
   * @li scale - scale of the feature unit compared to pixels (double)
   *
   * The item with the key "name" is mandatory. Other items are
   * optional. The name and unit are shown for the user in the ui
   * component. The scale is used for calculating the values as units
   * stored in the rule matrices as pixels.
   *
   * Setting this property affects to the property @p ruleMatrices.
   * The amount of columns (width) of each raws rule matrix is set
   * equal to the length of the feature properties list set by this property.
   *
   * Changing the amount of the rows (height) of the rule matrices
   * depens on the difference between the new features set by
   * this property and the old features before setting the property.
   * If by setting the property all the new features names are different than
   * the old feature names, the rows of each rule matrix are
   * removed. If by setting the property at least one new feature is
   * same than the old, the rows in the rule matrices are not removed,
   * but the column corresponding to the feature are copied from the
   * old rule matrices to the new rule matrices. The equality is
   * checked by comparing the name of the feature, not the unit nor
   * the scale. If for a feature the new unit scale is different than
   * the old, the visible values of the unit values changes according
   * to the new scale, but they are still stored internally exaclty
   * with the same values than before setting the property. The
   * columns corresponding to the new feature are cleared in the raw rule
   * matrices (by setting the min and max values to -INF and INF values).
   */
  Q_PROPERTY(QVariantList featureProperties READ featureProperties WRITE setFeatureProperties);
  
  /**
   * A property that defines the decision rules for the operation. The
   * ruleMatrices data structure contains a list of matrices, where
   * each matrix contains the decision rules for a layer. The matrix
   * is of type PiiMatrix<double>, which must be casted to QVariant
   * through PiiVariant before adding to ruleMatrices list. The rows
   * in a matrix is equal to the amount of rows of decision rules used
   * in the layer. The items in a row contains the minimum and maximum
   * values of the features to be used in the decision. The minimum
   * and maximum values must alternate so that the minimum values are
   * located in the even indexed locations and the maximum values in
   * the odd indexed location. The amount of columns in a row should
   * be twice the amount of the features in a feature vector coming in
   * the operatotion through the @p featureInput.
   */
  Q_PROPERTY( QVariantList ruleMatrices READ ruleMatrices WRITE setRuleMatrices);


  /**
   * This property defines the result labels for the decision rows for
   * the each layer. The property contain a list, where an item is
   * another list converted to QVariant. This inner list contains
   * doubles converted to QVariant. The inner list contains the label
   * values for a layer. The outer list contains these lists for each layer.
   * 
   */
  Q_PROPERTY(QVariantList labels READ labels WRITE setLabels);


  /**
   * This property defines the rules for the decision rows for
   * the each layer. The property contain a list, where an item is
   * another list converted to QVariant. This inner list contains
   * booleans converted to QVariant. The inner list contains the boolean
   * values for a layer (AND = true, OR = false). The outer list
   * contains these lists for each layer.
   * 
   */
  Q_PROPERTY(QVariantList rules READ rules WRITE setRules);
  
  PII_OPERATION_SERIALIZATION_FUNCTION
    
public:
  PiiTableLabelerOperation();

  /**
   * Creates the UI component for @p PiiTableLabelerOperation.
   * Currently the UI components "configurator" and "toolbar" are
   * supported. "configurator" is a widget of type @p
   * PiiTableLabelerConfigurator. "toolbar" is a toolbar, of type
   * PiiTableLabelerToolBar, for managing the @p configurator. The
   * order of creation of "configurator" and "toolbar" doesn't matter.
   */
  QWidget* createUiComponent(const QString& name);
  
  /**
   * Returns the names of the supported UI components. Currently @p configurator
   * and @p toolbar are supported.
   */
  QStringList supportedUiComponents() const;

  void setFeatureProperties(const QVariantList& featureProperties);
  QVariantList featureProperties() const;
  
  void setLayers(const QVariantList& layers);
  QVariantList layers() const;


  /* NEW The ruleMatrices paramater contains a list of rule matrices.
     Each item in the list is a PiiMatrix<double> data structure casted to
     PiiVariant. Each row in the matrix contains a row in a
     decision table. The items in a row contain minimum and maximum
     values of the features used in the deduction. The minimum and
     maximum values are located in the row consecutively so, that the
     minimum values are located in the even indexed locations and the
     maximum values in the odd indexed location.
  */
  void setRuleMatrices(const QVariantList& ruleMatrices);
  QVariantList ruleMatrices() const;
  
  void setLabels(const QList< QVariant >& labels);
  QList< QVariant > labels() const;

  void setRules(const QList< QVariant >& rules);
  QList< QVariant > rules() const;

  /**
   * This function sets the rule matrices in a "raw" format. The raw
   * in this context means the format, that the
   * PiiTableLabelerOperation itself uses for storing the matrices.
   * This function is recommended to be called  only from inside the @pclassification
   * plugin. If you want to set the rule matrices from somewhere else,
   * use the @p ruleMatrices @p property interface in stead.
   */
  void setRawRuleMatrices(const QList< PiiMatrix< PiiMinMax > >& ruleMatrices);

  /**
   * This function returns rule matrices in a "raw" format. The
   * matrices returned are deep copies of the orginal matrices. See the
   * function setRawRuleMatrices for the more information.
   */
  QList< PiiMatrix< PiiMinMax > > rawRuleMatrices() const;
  
  /**
   * This function sets the labels in a "raw" format. The raw in this
   * context means the format, that the PiiTableLabelerOperation
   * itself uses for storing the label information. This function is
   * recommended to be called only from inside the @p classification
   * plugin. If you want to set the labels from somewhere else, use
   * the @p labels @p property interface in stead.
   */
  void setRawLabels(const QList< QList<double> >& labelMatrix);
  //{ _labelMatrix = labelMatrix; }

  /**
   * This function returns the labels in a "raw" format. See the
   * function setRawLabels() for the more information.
   */
  QList< QList<double> > rawLabels() const;

  void setRawRuleVectors(const QList< QList<bool> >& ruleVectors);
  QList< QList<bool> > rawRuleVectors() const;
  
protected:
  void process();
  void check(bool reset);

private:
  
  /// @internal
  class Data : public PiiDefaultOperation::Data
  {
  public:
    Data();
    // The input that receives feature vectors.
    PiiInputSocket* pFeatureInput;
    PiiLayerParser layerParser;
    
    // Contains the feature properties as QVariantMap items.
    QVariantList lstFeatureProperties;
    QList< PiiMatrix< PiiMinMax > > lstRuleMatrices;
    QList< QList<double> > labelMatrix;
    QList< QList<bool> > lstRuleVectors;
    
    // The default label value. This vill be used, when the label value
    //  cannot be calculated.
    double defaultLabel;
    
    PiiTableLabelerConfigurator *pConfigurator;
    PiiTableLabelerToolBar *pToolBar;
  };
  PII_D_FUNC;

  template <class T> void addEmptyMatrixToList(QList< PiiMatrix<T> >& lstMatrices) const;
  void fillMissingLabelsInVector(int layer);
  void fillMissingLabelsInMatrix();
  void fillMissingRulesInVector(int layer);
  void fillMissingRuleVectorsInMatrix();
  
  void makeDataConsistent();
  void updateWidthsOfRuleMatrices();
  bool removeUnusedRows();
  void rulesForSimilarFeatures(const QVariantList& newFeatureProperties,
                               QStringList& similarFeatureNames,
                               QList< PiiMatrix< PiiMinMax > >& rulesForSimilarFeatures) const;
  int indexForFeature(const QString& featureName) const;
  bool featureExists(const QString& featureName) const;
  void setRulesForFeatures(const QStringList& featureNames,
                                  const QList< PiiMatrix< PiiMinMax > >& rulesForFeatures);
  void removeRowsFromRuleMatrices();
  void connectUiComponents();

  QVariantList ruleMatricesToExternal( const QList< PiiMatrix< PiiMinMax > >& minMaxMatrices) const;
  QVariantList labelMatrixToExternal(const QList< QList<double> >& labels) const;
  QVariantList ruleVectorsToExternal( const QList< QList<bool> >& ruleVectors) const;
  QList< PiiMatrix< PiiMinMax > > ruleMatricesToInternal(const QVariantList& doubleMatrices) const;
  QList< QList<double> > labelMatrixToInternal(const QList< QVariant >& labels) const;
  QList< QList<bool> > ruleVectorsToInternal(const QList< QVariant >& ruleVectors) const;
  
  double labelForVector(int layer, const PiiMatrix< double >& featureMatrix) const;

private slots:
  void uiComponentDestroyed(QObject* object);
  
};

inline QDebug operator<<(QDebug dbg, const PiiMinMax &minmax)
{
  dbg.nospace() << "PiiMinMax(" << minmax.dMin << ", " << minmax.dMax << ")";
  
  return dbg.space();
}

template <class T> QDebug operator<<(QDebug dbg, const PiiMatrix<T>& matrix)
{
  dbg.nospace() << " PiiMatrix(" << matrix.rows() << ", " << matrix.columns() << "): [ ";
  for (int i = 0; i < matrix.rows(); ++i)
    {
      dbg.nospace() << " [ ";
      
      for (int j = 0; j < matrix.columns(); ++j)
        {
          dbg.space() << matrix(i,j);
          if (j<matrix.columns()-1)
            dbg.nospace() << ", ";
        }
      dbg.nospace() << " ] ";
      if (i<matrix.rows()-1)
        dbg.nospace() << ", ";
    }
  dbg.nospace() << " ]";
  return dbg.space();
}

#endif //_PIITABLELABELEROPERATION_H

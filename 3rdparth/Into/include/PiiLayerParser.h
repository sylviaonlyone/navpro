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

#ifndef _PIILAYERPARSER_H
#define _PIILAYERPARSER_H

#include <QList>
#include <QVector>
#include <QVariant>
#include <QColor>
#include <PiiUtil.h>
#include "PiiYdin.h"

// The following definitions defines the indices where the "max" and
// "min" values of the float layer are store in _classColorsCache,
// when the float layer type is the active layer.
#define PII_LAYER_PARSER_START_COLOR_INDEX 0
#define PII_LAYER_PARSER_END_COLOR_INDEX 1

//class QColor;
class QString;

/**
 * This class provides an interface for handling a data structure
 * called a @layer.
 *
 * @ingroup Ydin
 */
class PII_YDIN_EXPORT PiiLayerParser
{
 
public:

  /**
   * The type of a layer  can assume following values:
   *
   * @lip LayerTypeUndefined - undefined layer type
   *   
   * @lip LayerTypeFloat - layer type "float"
   *
   * @lip LayerTypeClass - layer type "class"
   *
   */  
  enum LayerType
    {
      LayerTypeUndefined = 0x00,
      LayerTypeFloat = 0x01,
      LayerTypeClass = 0x02
      //PiiLayerTypePhysical = 0x04
    };

  PiiLayerParser();
  ~PiiLayerParser();

  /**
   * TODO: REVIEW THIS COMMENT BLOCK!!  
   * This function defines the selector layers. For each
   * entry in this list, a new labeling layer is created. The
   * configuration of the layer is stored as a map of name-value
   * pairs. That is, each QVariant in this list is actually a
   * QMap<QString, QVariant>. The configuration parameters depend on
   * the type of the layer:
   *
   * @par All layers
   *
   * @li @p type - layer type, currently either "float" or "class"
   * (QString)
   *
   * @li @p name - the user-visible name of the layer (QString)
   *
   * @li @p opacity - strength of overlay coloring (int, 0 =
   * transparent, 255 = opaque)
   *
   * @li @p disabled - is layer disabled? (bool, default: false)
   *
   *
   * @par Floating point layers
   *
   * @li @p min - minimum value for a label (double, default: 0)
   *
   * @li @p max - maximum value for a label (double, default: 1)
   *
   * @li @p resolution - value resolution (double, default: 0.1)
   *
   * @li @p startColor - gradient start color (QColor, default:
   * transparent red)
   *
   * @li @p endColor - gradient end color (QColor, default: saturated
   * red)
   *
   *
   * @par Class label layers
   *
   * @li @p classNames - class names (QStringList, default: "Good",
   * "Defect")
   *
   * @li @p classColors - color codes (QList<QVariant>, QVariants must
   * be QColors, default: arbitrary colors)
   *
   *
   * @code
   * QMap<QString, QVariant> firstLayer;
   * firstLayer["type"] = "float";
   * firstLayer["name"] = "Float layer";
   * firstLayer["max"] = 2;
   * firstLayer["endColor"] = QColor(Qt::blue); //Note: Qt::blue is an int
   *
   * QMap<QString, QVariant> secondLayer;
   * secondLayer["type"] = "class";
   * secondLayer["name"] = "Class name layer";
   * secondLayer["classNames"] = QStringList() << "Good" << "Fair" << "Poor";
   * secondLayer["classColors"] = QVariantList() << QColor(Qt::green) << QColor(Qt::yellow) << QColor(Qt::red);
   *
   * PiiLayerParser layerParser;
   * layerParser.setLayers( QList<QVariant>() << firstLayer << secondLayer));
   * @endcode
   */  
  void setLayers(const QList<QVariant>& lstLayers);
  QList<QVariant> layers() const { return _lstLayers; }

  /**
   * Sets the active layer. Some of the functions in PiiLayerParser
   * expects that the active layer has been set. The default value for
   * the active layer is 0.
   */
  void setActiveLayer(int activeLayer); // { _iActiveLayer = activeLayer; }

  /**
   * Returns the active layer.
   */
  int activeLayer() const { return _iActiveLayer; }  

  /** 
   * Sets the default color for the defects. The default color is used
   * in the situations where the color information is not available.
   * This is the case for example when the layer information has not
   * been set or if when with the float type layer the value of the
   * label  is not between the minimum and maximum value. If the
   * defaultDefectColor is not set, the default color is QColor(0,0,0),
   */
  void setDefaultDefectColor(const QColor &color) { _defaultDefectColor = color; }
  QColor defaultDefectColor() const { return _defaultDefectColor; }

  /** 
   * Sets the default layer name for the layer parser. The default
   * default value is used in the situations where the layer name has
   * not been set for the layer, or invalid layer index is used, when
   * attempting to get the layer name. See the function @p
   * layerName() for the more information. If the default layer
   * name has not been set, the default layer name is an empty string
   * by default.
   */
  void setDefaultLayerName(const QString &name) { _defaultLayerName = name; }
  QString defaultLayerName() const { return _defaultLayerName; }
  
  /**
   * Returns the info text for the defect, whose numeric labels are given in
   * the parameter @p measurements. The text is composed by looping
   * through @p measurements, calling the the function inforTextForLabel() for
   * each label in @p measurements, and combining the results of the
   * function calls by adding the line break character between them.
   * See the documentation of the function infoTextForLabel() for
   * more information.
   */
  QString infoTextForDefect(const QList<double> &measurements) const;

  /** Returns the info text corresponding the label for the @l layer
   * given as a parameter.  The rules for composing the info text
   * depend on the layer type. The rules are the following:
   *
   * @li for the the <i>floating point type layers</i> the format is:
   * <code>layerName + ": " + label</code>, where @p layerName is the
   * layer name corresponding the parameter @p layer and @p label is
   * the second parameter of the function. For example the result can
   * be "Formation index: 1.8".
   *
   * @li for the <i>class type layer</i> the format is:
   * <code>layerName + ": " + class<code>, where @p layerName is the
   * layer  name corresponding to the parameter @p layer and @p class
   * is the class corresponding to the parameter @p label. For example
   * the result can be "Severity: Good".
   *
   * See more information about the layers in the documentation for
   * the class PiiVisualTrainer.
   */
  QString infoTextForLabel(int layer, double label) const;

  /**
   * Returns the defect color corresponding the parameter @p label for
   * the the layer given as a parameter. This function recognizes the
   * layer type and based on the result of the recognition it calls
   * the function classColor() or getFloatColor(). If the color
   * information is  not availabe, the default color, set by function
   * setDefaultDefectColor(), is returned.
   */
  QColor defectColor(int layer, double label) const;

  /**
   * Similar to the previous function, except it uses the current
   * active layer. It is recommended to use this function in stead the
   * one introduced above, because this is faster.
   */
  QColor defectColor(double label) const;
  
  /**
   * Return the class color corresponding the numberic label value @p
   * label for the layer @p layer given as a parameter. The type of
   * the layer @p layer is expected to be "class". Otherwise the
   * behaviour of the function is undefined.  See more documentation
   * about the layers in the documentation of the class PiiVisualTrainer.
   */ 
  QColor classColor(int layer, float label) const;
  
  /**
   * Return the color corresponding the float type label value @p
   * label for the layer @p layer given as a parameter. The type of
   * the layer @p layer is expected to be "float". Otherwise the
   * behaviour of the function is undefined.  See more documentation
   * about the layers in the documentation of the class PiiVisualTrainer.
   */   
  QColor floatColor(int layer, float label) const;

  /**
   * Works like the previous function, exept the color is calculated
   * for the currently active layer. This function is also faster than
   * the other function, because the cache is used to obtain the
   * "min", "max", "startColor" and "endColor" values.*/
  QColor floatColor(float label) const;
  
  /**
   * Returns the layer name for the given layer. If the layer index is
   * invalid or the layer doesn't contain any valid name, a default
   * name is returned. The default name can be set with the function
   * setDefaultLayerName(). If the default name has not been set, it
   * is empty by default.
   */
  QString layerName(int layer) const;

  /**
   * Returns the layer type for the @p layer given as a parameter. If the
   * layer index is invalid, #LayerTypeUndefined is returned.
   */  
  LayerType layerType(int layer) const;
  
  /**
   * Returns true if the key given as a parameter exists in the given layers.
   */
  bool hasKey(int layer, const QString& key) const;
  
  /**
   * Return the amount of enabled layers
   * NOTE: Currently return the amount of all layers.
   */
  int enabledLayerCount() const;
  //int amountOfEnabledLayers() const;

  /**
   * Returns the current number of labeling layers.
   */  
  int layerCount() const { return _lstLayers.size(); }
  
  /**
   * Lists the user-visible names for layers.
   */
  QStringList layerNames() const;

  QStringList socketDescriptions() const;

  /**
   * Works exactly in the same way, than the corresponding static
   * function @p classNameStatic(), except the layer number is
   * given as a parameter in stead of the layer itself. If the layer
   * number is out of bounds, an empty string is returned.
   */  
  QString className(int layer, int classIndex) const;
  
  /**
   * Works exactly in the same way, than the corresponding static
   * function @p realIndexStatic(), except the layer number is
   * given as a parameter in stead of the layer itself. If the layer
   * number is out of bounds, -1 is returned.
   */  
  int realIndex(int layer, int classIndex) const;

  /**
   * Works exactly in the same way, than the corresponding static
   * function @p classIndexStatic(), except the layer number is
   * given as a parameter in stead of the layer itself. If the layer
   * number is out of bounds, -1 is returned.
   */  
  int classIndex(int layer, int index) const;

  /**
   * Works exactly in the same way, than the corresponding static
   * function, except the layer number is
   * given as a parameter in stead of the layer itself. If the layer
   * number is out of bounds, @p false is returned.
   */  
  bool checkIndexExistence(int layer, int index) const;
  
  /**
   * This static function returns the name of the given layer, or the
   * default layer name @p defaultName. The function does
   * all the necessary validity checks.
   */
  static QString layerName(const QVariant& layer, const QString& defaultName = "");

  /**
   * This static function returns the type of the given layer. In
   * case of error LayerTypeUndefined is returned.
   */  
  static LayerType layerType(const QVariant& layer);

  /**
   * Set the layer type. The possible parameter can be @p
   * LayerTypeFloat, @p LayerTypeClass, and @p LayerTypeUndefined.
   */
  static void setLayerType(QVariant& layer, PiiLayerParser::LayerType layerType);

  /**
   * Creates and returns the layer of type @p layerType
   */
  static QVariant createLayer(PiiLayerParser::LayerType layerType);

  /**
   * Copies the map item corresponding the key given as a parameter
   * from one layer to another. If no item with the given key doesn't
   * exist in the source layer, the item is not added in the
   * destionation layer.
   */
  static void copyMapItem(QVariant& toLayer, const QVariant& fromLayer, const QString& key);
  
  /**
   * Returns the the class names as a string list for the layer given
   * as a parameter. If the layer given as a parameter has not a valid
   * structure, an empty list is returned.
   */
  static QStringList classNames(const QVariant& layer);

  /**
   * Sets the class names as a string list to the layer given
   * as a parameter. If the layer given as a parameter contain not a valid
   * QVariantMap structure, list is not set.
   */
  static void setClassNames(QVariant& layer, const QStringList& items);

  /**
   * Returns the the item "classIndices" of the layer as an integer list.
   * The indices are stored intenally in the layer as
   * QVariantList, but the function makes the conversion from variant
   * list to integer list. If the layer given as a parameter has not a
   * valid QVariantMap structure, an empty list is returned. 
   */
  static QList<int> classIndices(const QVariant& layer);
  
  /**
   * Sets the global class indices as integers to the layer given
   * as a parameter. The function converts the integer list given as a
   * paramaeter to variant list, and stores the list in the layer as
   * list of variants. If the layer given as a parameter contain not a
   * valid QVariantMap structure, the list is not set.
   */
  static void setClassIndices(QVariant& layer, const QList<int>& items);

  // REMOVE >>
//   /**
//    * Returns the the global class indices as an integer list for the
//    * layer given as a parameter. The behavior of the function differs
//    * a little bit, depending wheter the layer contains the item
//    * "classIndices" or not. If the item "classIndices" exist, the
//    * return value of the function classIndices() is returned. Else
//    * the series of numbers, from zero to the amount of
//    * classes substracted by one, is returned. If for example the
//    * amount of classes in the layer were 5, and there are no
//    * "classIndices" item in the layer, the returned list will be [0,
//    * 1, 2, 3, 4].The indices are stored intenally in the layer as
//    * QVariantList, but the function makes the conversion from variant
//    * list to integer list. If the layer given as a parameter has not a
//    * valid QVariantMap structure, an empty list is returned. 
//    */
//   static QList<int> globalClassIndices(const QVariant& layer);
// REMOVE <<
  
  /**
   * The function checks existence of the global index of the index
   * given as a paramter for the given layer. It is assumed, that the
   * type of the layer is LayerTypeClass. Else the behavior of the
   * function is indeterminate.
   *
   * The behavior of the function differs a little bit, depending
   * wheter the layer contains the item "classIndices" or not. If the
   * item "classIndices" exist, the function checks, if the list of
   * class indices contains the index given as a parameter. Else the
   * function checks, if the value of the index is greater than zero
   * and less than amount of classes in the layer subtracted by one.
   * If any of the conditions are met, @p true is returned. Else @p
   * false is returned from the function.
   *
   * There is also corresponding non-static function.
   */
  static bool checkIndexExistence(const QVariant& layer, int index);
  
  /**
   * Returns the the class colors from the given layer as a list of
   * QColor objects. The class colors are stored internally in the
   * layer as a list of QVariants, but this function takes care of
   * making the conversion. If the layer given as a parameter has not a
   * valid QVariantMap structure, an empty list is returned. 
   */
  static QList<QColor> classColors(const QVariant& layer);
  
  /**
   * Sets the class colors to the given layer as a list of QColor objects. 
   * The function takes care of converting the list of QColors to the
   * list of QVariants, how the colors are internally stored in the
   * layer.  If the layer given as a parameter contain not a
   * valid QVariantMap structure, the list is not set.
   */
  static void setClassColors(QVariant& layer, const QList<QColor>& items);
  
  /**
   * Return the list of class colors for the layer. The way how the
   * list is formed, differs a little bit,
   * depending whether the layer contain a "classIndices" item or not.
   * If the layer doesn't contain a "classIndices" item, the list
   * returned contains the colors in the same order than they were
   * stored in the list. In the other case, the list returned
   * contains the colors in the order defined by the "classIndices"
   * list and also in the indices defined by the "classIndices" list.
   * The length of the list returned is equal to the highest index
   * stored in the "classInidices" list. The list returned can contain
   * also indices that are not in "classIndices" list. Those indices
   * will contain a default color given as a second parameter.
   */
  
  static QVector<QColor> globalClassColors(const QVariant& layer, const QColor& defaultColor);

  
  /**
   * Return the list of the items of type @p T stored with the key @p
   * itemName in the layer given as a parameter. It is assumed that
   * the items has been
   * stored in the layer as a QVariantList. The function makes the
   * conversion from the QVariants to the type T.
   */
  template <class T> static QList<T> list(const QVariant& layer, const QString& itemName)
  {
    QVariant itemList = mapItem(layer, itemName, QVariant::List);
    if (itemList.isValid())
      {
        return Pii::variantsToList<T>(itemList.toList());
      }
    else
      return QList<T>();
  }
  
  /**
   * Set the list containing the items of type @p T to the map item @p
   * itemName into the layer given as a paramter. The 
   * items will be stored in the layer as a QVariantList. The function makes the
   * conversion from the type T to QVariants.
   */
  template <class T> static void setList(QVariant& layer, const QList<T>& itemList, const QString& itemName)
  {
    if ( !layer.canConvert(QVariant::Map) )
      return;
    
    QMap<QString, QVariant> layerMap = layer.toMap();
    layerMap[itemName] = Pii::listToVariants(itemList);
    
    layer = QVariant(layerMap);     
  }

  /**
   * This function converts a list having "real" class indices to the vector
   * having the "global" class indices. The global indices are picked
   * from the "classIndices" field of the @layer given as a parameter.
   * The size of the vector is equal to the highest index in the
   * "classIndices" list. The indices that are missing in the
   * "classIndices" list, are filled with the default value given as a
   * parameter.
   *
   * If the "classInidices" field is missing, the return value
   * contains the items in the same order than in @sourceList just
   * converted to vector.
   */
  template <class T> static QVector<T> makeVectorUsingGlobalInidices(const QVariant& layer, const QList<T>& sourceList, const T& defaultValue)
  {
    //qDebug("PiiLayerParser::makeVectorUsingGlobalInidices() BEGIN");
    if (PiiLayerParser::hasKey(layer, "classIndices"))
      {
        QList<int> lstClassIndices = classIndices(layer);
        QVector<T> retVector;
        for (int i = 0; i<lstClassIndices.size(); i++)
          {
            // Fill the missing items itn the list to be returned with
            // the default values.
            while (retVector.size() <= lstClassIndices[i])
              retVector << defaultValue;
            // Make sure that the source list is big enough before
            // copying an item from it.
            if ( sourceList.size() > i )
              retVector[lstClassIndices[i]] = sourceList[i];
          }
        return retVector;
      }
    else
      {
        //qDebug("PiiLayerParser::makeVectorUsingGlobalInidices() RETURN source list converted to vector.");
        return sourceList.toVector();
      }
  }

  /**
   * This static function returns the real index for the global class
   * index given as a parameter, provided that the layer contains the
   * item "classIndices". If for example the item "classIndices"
   * contained the list {0,1,7,9}, and the value of the parameter @p
   * classIndex were 7, 2 is returned. If the list of classIndices
   * doesn't contain the number given as a parameter, -1 is returned.
   *
   * If the layer doesn't contain the item "classIndices", the number
   * given as a parameter is returned.
   *
   * It is assumed, that the type of the layer is "class". In other
   * case, the behavior of the function is undefined.
   *
   * There is also a corresponding non-static function @p realIndex().
   */  
  static int realIndexStatic(const QVariant& layer, int classIndex);

  /**
   * This static function returns the global index for the real class
   * index given as a parameter, provided that the layer contains the
   * item "classIndices". If for example the item "classIndices"
   * contained the list {0,1,7,9}, and the value of the parameter @p
   * classIndex were 2, 7 is returned. If the index given as a
   * parameter is out of the bounds, -1 is returned.
   *
   * If the layer doesn't contain the item "classIndices", the number
   * given as a parameter is returned.
   *
   * It is assumed, that the type of the layer is "class". In other
   * case, the behavior of the function is undefined.
   *
   * There is also the corresponding non-static function @p classIndex().
   */  
  static int classIndexStatic(const QVariant& layer, int index);
  
  /**
   * This static function returns the string to be shown in the user
   * interface for the label given as a parameter. The apperance of
   * the string depends on the type of the layer given as a parameter.
   * If the layer type is "float", the label value given as a parmeter
   * is just converted to string. If the layer type is "class" the
   * class name corresponding to the label value is returned. In case
   * of error an empty string is returned.
   */  
  static QString labelAsString(const QVariant& layer, double label);

  /**
   * The function returns the class name corresponding to the (real) index
   * given as a parameter. If a class with the index doesn't exist, an
   * empty string is returned. There is also corresponding non-static
   * function, className().
   */
  static QString classNameStatic(const QVariant& layer, int index);

  /**
   * The function returns the class name corresponding to the either
   * global or real index given as a parameter. If the item
   * "classIndices" exists in the layer given as a parameter, the
   * global index is used. Else the real index is used. If a class
   * with the given index doesn't exist, the value of the parameter @p
   * defaultName is returned.
   */
  static QString classNameStatic(const QVariant& layer, int globalIndex, const QString& defaultName);

  /**
   * The function returns the class color corresponding to the either
   * global or real index given as a parameter. If the item
   * "classIndices" exists in the layer given as a parameter, the
   * global index is used. Else the real index is used. If a class
   * with the given index doesn't exist, the value of the parameter @p
   * defaultColor is returned.
   */  
  static QColor classColorStatic(const QVariant& layer, int globalIndex, const QColor& defaultColor);
  
  /**
   * Returns the default label as double. If the layer is of type
   * "float", the result of function  @p minValue(), with the
   * default value 0.0  is returned.
   *
   * If the layer type is "class", the result depends whether the
   * layer contains the map item "classIndices". If the "classIndices"
   * item is missing, 0.0 is returned. Else the class item having the
   * smallest global index (== "classIndices") is returned.
   *
   * The function can be used e.g. for initializing the outputs values,
   * having labels as doubles, to some safe initial values.
   */
  static double defaultNumericLabel(const QVariant& layer);
  
  /**
   * This function return the minimum value for the label
   * in the given @p layer. If the @p layer data structure given as a
   * parameter has an invalid format (e.g. the minimum value has not
   * been specified), @p defaulVal is returned.
   */ 
  static double minValue(const QVariant& layer, double defaultVal = 0.0);

  /**
   * This function return the maximum value for the label
   * in the given @p layer. If the @p layer data structure given as a
   * parameter has an invalid format (e.g. the maximum value has not
   * been specified), @p defaulVal is returned.
   */   
  static double maxValue(const QVariant& layer, double defaultVal = 1.0);

  /**
   * This function return the resolution for the label value
   * in the given @p layer. If the @p layer data structure given as a
   * parameter has an invalid format (e.g. the resolution has not
   * been specified), @p defaulVal is returned.
   */ 
  static double resolution(const QVariant& layer, double defaultVal = 0.1);

  /** 
   * This a helper function, which returns the value for @p key as
   * QVariant. The function also checks, if the layer given as a
   * parameter is a valid @p QVarianMap, if the key given as a parameter
   * exists and if the value for the key is of @p type. In case of error an
   * invalid QVariant is returned.
   */
  static QVariant mapItem(const QVariant& layer, const QString& key, QVariant::Type type);

  /**
   * The function sets the map item for the layer given as a
   * parameter. The layer given as a paramter should be a QVariantMap
   * converted to the QVariant. The function checks that. In case of
   * incorrect type, no changes are made in the layer.
   */
  static void setMapItem(QVariant& layer, const QString& key, const QVariant& data);
  
  /**
   * Returns true if the key given as a parameter exists the given layer.
   */
  static bool hasKey(const QVariant& layer, const QString& key)
  {
    return layer.toMap().contains(key);
  }

  /**
   * Removes the key given as a parameter from the layer.
   */
  static void removeKey(QVariant& layer, const QString& key);
  
private:

  //NOTE: This memeber variable contains only enabled layers
  QList<QVariant> _lstLayers;

  QString _defaultLayerName;
  QColor _defaultDefectColor;

  // Contains the colors for each global class index. The list
  // contains the colors for the current active layer. If the type of
  // the current layer is LayerTypeFloat, the list contains minimum
  // and maximum values in idexec PII_LAYER_PARSER_MIN_INDEX and
  // PII_LAYER_PARSER_MAX_INDEX. If the layer type is LayerTypeClass
  QVector<QColor> _classColorsCache;
  int _iActiveLayer;
  LayerType _currLayerType; // Layer type for the active layer.
  // "min" and "max" values for the current layer. Makes only sense,
  // if the current layer is of type "float". 
  double _dCurrMin, _dCurrMax;
  
  void _initCache();

  static double _getDoubleItemValue(const QVariant& layer, const QString& key, double defaultVal)  ;

  /**
   * Set the layer type. The possible parameter can be @p
   * LayerTypeFloat, @p LayerTypeClass, and @p LayerTypeUndefined.
   */
  static void _setLayerType(QVariantMap& layerMap, PiiLayerParser::LayerType layerType);

  /**
   * Returns the smallest global index. If the map item "classIndices"
   * doesn't exist in the layer or the list of incides is empty, -1 is
   * returned.
   */
  static int smallestGlobalIndex(const QVariant& layer);
  
  /**
   * This function checks, if the layer given as a parameter contains
   * the map item "classIndices".
   */
  inline static bool hasGlobalIndices(const QVariant& layer)
  {
    // The name of the class must be in the function call in order to
    // distinct this function from the corresponding non-static,
    // function, which has the layer parameter as an int in stead of QVariant.
    return PiiLayerParser::hasKey(layer, "classIndices");
  }

  
};



#endif //_PIILAYERPARSER_H

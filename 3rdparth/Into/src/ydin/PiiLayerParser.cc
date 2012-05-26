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

#include "PiiLayerParser.h"
#include <QtDebug>
#include <QCoreApplication>

PiiLayerParser::PiiLayerParser() :
  _defaultLayerName(""), _defaultDefectColor(QColor(0,0,0)), _iActiveLayer(0),
  _currLayerType(LayerTypeUndefined), _dCurrMin(0), _dCurrMax(1)
{
  //_iActiveLayer = 0;
}

PiiLayerParser::~PiiLayerParser()
{

}

void PiiLayerParser::setLayers(const QList<QVariant>& lstLayers)
{
  //qDebug("PiiLayerParser::setLayers() BEGIN");
  _lstLayers = lstLayers;
  _initCache();
  //qDebug("PiiLayerParser::setLayers() END");
}

void PiiLayerParser::setActiveLayer(int activeLayer)
{
  //qDebug("PiiLayerParser::setActiveLayer() BEGIN");
  _iActiveLayer = activeLayer;
  _initCache();
  //qDebug("PiiLayerParser::setActiveLayer() END");
}

QString PiiLayerParser::infoTextForDefect(const QList<double> &measurements) const  
{
  QString retStr = "";
  for (int i=0; i<measurements.size(); i++)
    {
      retStr += infoTextForLabel(i,measurements[i]);
      if (i < measurements.size()-1)
        retStr += "\n";
    }
  return retStr;
}

QString PiiLayerParser::infoTextForLabel(int layer, double label) const
{
  //Check that the lable input index is within bounds.
  QString errText = QCoreApplication::translate("PiiLayerParser","Class info not available");
  if (layer >= _lstLayers.size() || layer <0)
    {
      //qDebug("PiiLayerParser::infoTextForLabel(): layer index out of bounds, layer = %d", layer);
      //qDebug("PiiLayerParser::infoTextForLabel(): _lstLayers.size() = %d", _lstLayers.size());
      //TODO: change this to exception
      return errText;
    }
  if (_lstLayers[layer].toMap()["type"] == "class")
    {
       //qDebug("PiiLayerParser::infoTextForLabel(): layer type == class");
      //Check for negative label value
      if (label<0)
        {
          //qDebug("PiiLayerParser::infoTextForLabel(), negative label value");
          return errText;
        }

      int classIndex = (int)label;

      // Make the vector of class names having the global class
      // indices filling the missing indices with null strings.
      QVector<QString> tempClassNames = makeVectorUsingGlobalInidices(_lstLayers[layer], classNames(_lstLayers[layer]), QString());

      // Check that the label has not exceeded the maximum class
      // index
      if (classIndex >= tempClassNames.size())
        return errText;

      if (tempClassNames[classIndex].isEmpty())
        {
          return errText;
        }
      else
        {
          QString tempStr = QCoreApplication::translate("PiiLayerParser","Class: ");
          tempStr += tempClassNames[classIndex];
          return tempStr;
        }
    }
  else if (_lstLayers[layer].toMap()["type"] == "float")
    {
      //qDebug("PiiLayerParser::infoTextForLabel(): layer type == float");
      QString tempStr = QString("%1 ").arg(_lstLayers[layer].toMap()["name"].toString());
      tempStr += QString(": %1 ").arg(label);
      return tempStr;
    }
  else
    {
      //qDebug("PiiLayerParser::infoTextForLabel(): layer type not recognized");
      //Layer type not recognized.
      return errText;
    }
}

// TODO: GET RID OF THIS FUNCTION!!!!!!
QColor PiiLayerParser::defectColor(int layer, double label) const  
{
  if ( layer<0 || layer>= _lstLayers.size() )
    {
      //qDebug("PiiLayerParser::defectColor(), layer<0 || layer>_lstLayers.size(), RETURN defaulDefectColor"); 
      return _defaultDefectColor;
    }

  if (_lstLayers[layer].toMap()["type"].toString().toLower() == "class")
    {
      
      //qDebug("PiiLayerParser::defectColor(): layer type ==
      //class");

      return classColor(layer, label);
    }
  else if (_lstLayers[layer].toMap()["type"].toString().toLower() == "float")
    {
      //qDebug("PiiLayerParser::defectColor(): layer type == float");
       return floatColor(layer, label);
    }
  else
    {
      //qDebug("PiiDefectMap::defectColor(): layer type unknown");
      //The class info  not available. The default color is returned.
      return _defaultDefectColor;
    }
  
}

QColor PiiLayerParser::defectColor(double label) const
{
  if (_currLayerType == LayerTypeClass)
    {
      int classIndex = (int)label;
      if (label < _classColorsCache.size())
        return _classColorsCache[classIndex];
      else
        return _defaultDefectColor;
    }
  else if (_currLayerType == LayerTypeFloat)
    {
      return floatColor(label);
    }
  else
    return _defaultDefectColor;
}
  
QColor PiiLayerParser::classColor(int layer, float label) const
{
  if (layer < 0 || layer > _lstLayers.size())
    return _defaultDefectColor;
  
  int classIndex = int(label);
  //const QVariantList varList = _lstLayers[_iActiveLayer].toMap()["classColors"].toList();
  const QVariantList varList = _lstLayers[layer].toMap()["classColors"].toList();
  //qDebug("PiiLayerParser::classColor(): classIndex == %d", classIndex);
  if (classIndex < 0 || classIndex >= varList.size())
    {
       //qDebug("PiiLayerParser::classColor(): class out of bounds");
      //The class index is out of the bounds. The default color
      //is returned.
      return _defaultDefectColor;
    }

  return varList[classIndex].value<QColor>();
  
}

/* Calcutates the color for the float-type layer. It is assumed in
   this function, that the fields "min", "max", "startColor" and
   "endColor" are correctly set.*/
QColor PiiLayerParser::floatColor(int layer, float label) const
{
  //qDebug("PiiLayerParser::floatColor(): value == %f", val);
  if (layer < 0 || layer > _lstLayers.size() )
    return _defaultDefectColor;
  
  double min = _lstLayers[layer].toMap()["min"].toDouble();
  double max = _lstLayers[layer].toMap()["max"].toDouble();

  if (label < min || label > max)
    return _defaultDefectColor;

  double factor = (label-min)/(max-min);

  //qDebug("PiiLayerParser::floatColor(): factor == %f", factor);

  QColor startColor = _lstLayers[layer].toMap()["startColor"].value<QColor>();
  QColor endColor = _lstLayers[layer].toMap()["endColor"].value<QColor>();

  //qDebug() << "PiiLayerParser::floatColor(): startColor = :" << startColor;
  //qDebug() << "PiiLayerParser::floatColor(): endColor = :" << endColor;
  
  int red = startColor.red()+int((endColor.red()-startColor.red())*factor);
  int green = startColor.green()+int((endColor.green()-startColor.green())*factor);
  int blue = startColor.blue()+int((endColor.blue()-startColor.blue())*factor);
  int alpha = startColor.alpha()+int((endColor.alpha()-startColor.alpha())*factor);

  QColor retColor = QColor ( red, green, blue,  alpha );
  //qDebug() << "PiiLayerParser::floatColor(): retColor = :" << retColor;
  
  return retColor;
}

/* Calcutates the color for the currently active float-type layer.
   The function uses the "min", "max", "startColor" and "endColor"
   that are stored in cache. The caller of this function must make
   sure, that the active layer has type "float". If the label given as
   a parmeter doesn't contain a value between min and max values in
   the cache, the default defect color is returned.*/
QColor PiiLayerParser::floatColor(float label) const
{
  
  if (label < _dCurrMin || label > _dCurrMax)
    return _defaultDefectColor;

  double factor = (label-_dCurrMin)/(_dCurrMax-_dCurrMin);

  //qDebug("PiiLayerParser::floatColor(): factor == %f", factor);

  QColor startColor = _classColorsCache[PII_LAYER_PARSER_START_COLOR_INDEX];
  QColor endColor = _classColorsCache[PII_LAYER_PARSER_END_COLOR_INDEX];

  //qDebug() << "PiiLayerParser::floatColor(): startColor = :" << startColor;
  //qDebug() << "PiiLayerParser::floatColor(): endColor = :" << endColor;
  
  int red = startColor.red()+int((endColor.red()-startColor.red())*factor);
  int green = startColor.green()+int((endColor.green()-startColor.green())*factor);
  int blue = startColor.blue()+int((endColor.blue()-startColor.blue())*factor);
  int alpha = startColor.alpha()+int((endColor.alpha()-startColor.alpha())*factor);

  QColor retColor = QColor ( red, green, blue,  alpha );
  //qDebug() << "PiiLayerParser::floatColor(): retColor = :" << retColor;
  
  return retColor;
}


QString PiiLayerParser::layerName(int layer) const
{
  if (layer < 0 || layer > _lstLayers.size())
    return _defaultLayerName;  
  else
    return layerName(_lstLayers[layer],_defaultLayerName);
}

PiiLayerParser::LayerType PiiLayerParser::layerType(int layer) const
{
  if (layer < 0 || layer > _lstLayers.size())
    return LayerTypeUndefined;  
  else
    return layerType(_lstLayers[layer]);

}

bool PiiLayerParser::hasKey(int layer, const QString& key) const
{
  if (layer < 0 || layer > _lstLayers.size())
    return false;  

  return hasKey(_lstLayers[layer],key);
}


/* NOTE: Currently in this function it is assumed, that only enabled
   layers are stored in _lstLayers. However, this can be changed in the future.*/
//int PiiLayerParser::amountOfEnabledLayers() const
int PiiLayerParser::enabledLayerCount() const
{
  return _lstLayers.size();
}

QStringList PiiLayerParser::layerNames() const
{
  QStringList result;
  for (int i=0; i<_lstLayers.size(); i++)
    result << layerName(i);
  //result << _lstLayers[i].toMap()["name"].toString();
  return result;
}

QStringList PiiLayerParser::socketDescriptions() const
{
  QStringList result;
  for (int i=0; i<_lstLayers.size(); i++)
    {
      QMap<QString,QVariant> map = _lstLayers[i].toMap();
      
      if ( !map["disabled"].toBool() )
        {
          if (map["type"] == "class")
            {
              //if ( !map["disabled"].toBool() )
              result << QString(QCoreApplication::translate("PiiLayerParser","%1, 0..%2").arg(map["name"].toString()).arg(map["classNames"].toStringList().size()-1));
            }
          else if ( map["type"] == "physical")
            {
              //if ( !map["disabled"].toBool() )
              //result << tr("%1").arg(map["name"].toString());
              result << QString("%1").arg(map["name"].toString());
            }
          else
            {
              //map["type"] = "float";
          
              //if ( !map["disabled"].toBool() )
              //result << tr("%1, %2-%3, %4 resolution ")
              result << QString(QCoreApplication::translate("PiiLayerParser","%1, %2-%3, %4 resolution ")
                                .arg(map["name"].toString())
                                .arg(map["min"].toDouble())
                                .arg(map["max"].toDouble())
                                .arg(map["resolution"].toDouble())); 
            }
        }
    }
  return result;
}

QString PiiLayerParser::className(int layer, int classIndex) const
{
  //qDebug("PiiLayerParser::className()");
  if (layer < 0 || layer > _lstLayers.size())
    return "";  
  else
    return classNameStatic(_lstLayers[layer], classIndex);
}

int PiiLayerParser::realIndex(int layer, int classIndex) const
{
  //qDebug("PiiLayerParser::realIndex(), layer == %d, _lstLayers.size() == %d, classIndex == %d", layer, _lstLayers.size(), classIndex);
  if (layer < 0 || layer > _lstLayers.size())
    {
      //qDebug("PiiLayerParser::realIndex(), returning -1");
      return -1;
    }
  else
    {
      //qDebug("PiiLayerParser::realIndex(), returning getRealIndexStatic(_lstLayers[layer], classIndex)");
      //qDebug() << "                           _lstLayers[i] = " << _lstLayers[layer];
      return realIndexStatic(_lstLayers[layer], classIndex);
    }
}

int PiiLayerParser::classIndex(int layer, int index) const
{
  //qDebug("PiiLayerParser::classIndex()");
  if (layer < 0 || layer > _lstLayers.size())
    return -1;  
  else
    return classIndexStatic(_lstLayers[layer], index);
}


bool PiiLayerParser::checkIndexExistence(int layer, int index) const
{
  if (layer < 0 || layer > _lstLayers.size())
    return false;  
  else
    return checkIndexExistence(_lstLayers[layer], index);
}

// QStringList PiiLayerParser::enabledLayerNames() const
// {
//   QStringList result;
//   for (int i=0; i<_lstLayers.size(); i++)
//     if ( !_lstLayers[i].toMap()["disabled"].toBool() )
//       result << _lstLayers[i].toMap()["name"].toString();
//   return result;
// }

//////////////// Implementation of the static functions ////////////////

QVariant PiiLayerParser::mapItem(const QVariant& layer, const QString& key, QVariant::Type type)
{
  if ( !layer.canConvert(QVariant::Map) )
    return QVariant();

  QVariantMap layerMap = layer.toMap();
  if ( !layerMap.contains(key))
    return QVariant();

  QVariant value = layerMap[key];
  if ( !value.canConvert(type) )
    return QVariant();
  else
    return value;
}

void PiiLayerParser::setMapItem(QVariant& layer, const QString& key, const QVariant& data)
{
  if ( !layer.canConvert(QVariant::Map) )
    return;

  QMap<QString, QVariant> layerMap = layer.toMap();

  layerMap[key] = data;

  // Make sure, that changing the data has an effect in the layer.
  layer = QVariant(layerMap);
}

void PiiLayerParser::removeKey(QVariant& layer, const QString& key)
{
  if ( !layer.canConvert(QVariant::Map) )
    return;

  QVariantMap layerMap = layer.toMap();
  if (layerMap.contains(key))
    {
      layerMap.remove(key);
      // Make sure that the change in the map also will be stored in
      // the layer variable given as a parameter.
      layer = QVariant(layerMap);
    }
}

QString PiiLayerParser::layerName(const QVariant& layer, const QString& defaultName)
{
  QVariant name = mapItem(layer, "name", QVariant::String);
  if (name.isValid())
    return name.toString();
  else
    return defaultName;
}

PiiLayerParser::LayerType PiiLayerParser::layerType(const QVariant& layer)
{

  QVariant layerType = mapItem(layer, "type", QVariant::String);
  if (!layerType.isValid())
    return LayerTypeUndefined;
  
  if (layerType == "float")
    return LayerTypeFloat;
  else if (layerType == "class")
    return LayerTypeClass;
  else
    return LayerTypeUndefined;
}

/* PENDING
 * It should be still checked that the layer doesn't contain the
 * information that the new layer type cannot contain. For example the
 * "class" type layer cannot contain the keys "min" and "max". Those
 * kind of keys should be removed, when changing the layer type.
 */
void PiiLayerParser::setLayerType(QVariant& layer, PiiLayerParser::LayerType layerType)
{
  if ( !layer.canConvert(QVariant::Map) )
    return;

  QMap<QString, QVariant> layerMap = layer.toMap();

  _setLayerType(layerMap, layerType);

  // Make sure, that changing the layer type has an effect in the layer.
  layer = QVariant(layerMap);
}

QVariant PiiLayerParser::createLayer(PiiLayerParser::LayerType layerType)
{
  QMap<QString, QVariant> layerMap;
  _setLayerType(layerMap, layerType);
  QVariant layer = QVariant(layerMap);
  return layer;
}

void PiiLayerParser::copyMapItem(QVariant& toLayer, const QVariant& fromLayer, const QString& key)
{
  if ( toLayer.canConvert(QVariant::Map) && fromLayer.canConvert(QVariant::Map))
    {
      if (hasKey(fromLayer, key))
        {
          // The assignment must be this complicated, because function
          // toMap() returns a copy of the map, not a reference to it.
          QVariantMap tempMap = toLayer.toMap();
          tempMap[key] = fromLayer.toMap()[key];
          toLayer = QVariant(tempMap);
        }
    }
}

QStringList PiiLayerParser::classNames(const QVariant& layer)
{
  QVariant classNames = mapItem(layer, "classNames", QVariant::StringList);
  if (classNames.isValid())
    return classNames.toStringList();
  else
    return QStringList();
}

void PiiLayerParser::setClassNames(QVariant& layer, const QStringList& items)
{
  if ( !layer.canConvert(QVariant::Map) )
    return;

  QMap<QString, QVariant> layerMap = layer.toMap();
  layerMap["classNames"] = items;

  layer = QVariant(layerMap); 
}

QList<int> PiiLayerParser::classIndices(const QVariant& layer)
{
  return list<int>(layer, "classIndices");
}

void PiiLayerParser::setClassIndices(QVariant& layer, const QList<int>& items)
{
  setList(layer, items, "classIndices");
}

QList<QColor> PiiLayerParser::classColors(const QVariant& layer)
{
  return list<QColor>(layer, "classColors");
}

void PiiLayerParser::setClassColors(QVariant& layer, const QList<QColor>& items)
{
  setList(layer, items, "classColors");
}

  
QVector<QColor> PiiLayerParser::globalClassColors(const QVariant& layer, const QColor& defaultColor)
{
  //qDebug("PiiLayerParser::globalClassColors() BEGIN");
  return makeVectorUsingGlobalInidices(layer, classColors(layer), defaultColor);
}

int PiiLayerParser::realIndexStatic(const QVariant& layer, int classIndex)
{
  //qDebug("PiiLayerParser::realIndexStatic() BEGIN, classIndex = %d", classIndex);
  //qDebug() << "PiiLayerParser::realIndexStatic(), layer = " << layer;
  if (hasGlobalIndices(layer))
    {
      // Case, where "classIndices" exist
      QList<int> lstClassIndices = classIndices(layer);
      //qDebug("PiiLayerParser::realIndexStatic() RETURN classIndices.indexOf(classIndex) == %d", classIndices.indexOf(classIndex));
      return lstClassIndices.indexOf(classIndex);
    }
  else
    {
      // Case, where "classIndices" don't exist.
      //qDebug("PiiLayerParser::realIndexStatic() RETURN classIndex == %d", classIndex);
      return classIndex;
    }
}

int PiiLayerParser::classIndexStatic(const QVariant& layer, int index)
{
  //qDebug("PiiLayerParser::classIndexStatic() BEGIN, index = %d", index);
  if (hasGlobalIndices(layer))
    {
      // Case, where "classIndices" exist
      QList<int> lstClassIndices = classIndices(layer);
      //qDebug("PiiLayerParser::classIndexStatic() classIndices.size() = %d", classIndices.size());
      //qDebug() << "PiiLayerParser::classIndexStatic() classIndices = " << classIndices;
      if (index >=0 && index < lstClassIndices.size())
        {
          return lstClassIndices[index];
          //qDebug("PiiLayerParser::classIndexStatic() RETURN classIndices[index] == %d", classIndices[index]);
        }
      else
        {
          //qDebug("PiiLayerParser::classIndexStatic() RETURN -1");
          return -1;
        }
    }
  else
    {
      // Case, where "classIndices" don't exist.
      //qDebug("PiiLayerParser::classIndexStatic() RETURN index");
      return index;
    }
}

bool PiiLayerParser::checkIndexExistence(const QVariant& layer, int index)
{ 
  //qDebug("PiiLayerParser::checkIndexEsistence() BEGIN, index = %d", index);
  if (hasGlobalIndices(layer))
    {
      // Case, where "classIndices" exist
      //qDebug() << "PiiLayerParser::checkIndexEsistence(), 'classIndices'
      //exists. RETURN " << classIndices.contains(index);
      QList<int> lstClassIndices = classIndices(layer);
      return lstClassIndices.contains(index);
    }
  else
    {
      // Case, where "classIndices" don't exist.
      //qDebug() << "PiiLayerParser::checkIndexEsistence(), 'classIndices'
      //doesn't exist. RETURN " << (index >=0 && index < classNames(layer).size());
      return (index >=0 && index < classNames(layer).size());
    }
}

QString PiiLayerParser::labelAsString(const QVariant& layer, double label)
{
  PiiLayerParser::LayerType type = layerType(layer);

  if (type == PiiLayerParser::LayerTypeFloat)
    return QString("%1").arg(label);
  else if (type == PiiLayerParser::LayerTypeClass)
    return classNameStatic(layer, (int)label);
  else
    return "";
}

QString PiiLayerParser::classNameStatic(const QVariant& layer, int index)
{
  QStringList lstClassNames = classNames(layer);
  if (index >=0 && index < lstClassNames.size())
    return lstClassNames[index];
  else
    return "";
}

QString PiiLayerParser::classNameStatic(const QVariant& layer, int globalIndex, const QString& defaultName)
{
  //qDebug("PiiDefectMap::classNameStatic(): BEGIN, globalIndex = %d", globalIndex);
  //qDebug() << "PiiDefectMap::classNameStatic(): BEGIN layer = " << layer;
  //qDebug() << "PiiDefectMap::classNameStatic(): BEGIN defaultName = " << defaultName;
  QStringList classNames = PiiLayerParser::classNames(layer);
  int realIndex = PiiLayerParser::realIndexStatic(layer, globalIndex);
  if (realIndex<0 || realIndex>=classNames.size())
    {
      //qDebug("PiiDefectMap::classNameStatic(): RETURN defaultName");
      return defaultName;
    }
  //qDebug("PiiDefectMap::classNameStatic(): RETURN classNames[realIndex]");
  return classNames[realIndex];
}

QColor PiiLayerParser::classColorStatic(const QVariant& layer, int globalIndex, const QColor& defaultColor)
{
  //qDebug("PiiDefectMap::classColorStatic(): BEGIN, globalIndex = %d", globalIndex);
  //qDebug() << "PiiDefectMap::classColorStatic(): BEGIN layer = " << layer;
  //qDebug() << "PiiDefectMap::classColorStatic(): BEGIN defaultColor = " << defaultColor;
  QList<QColor> classColors = PiiLayerParser::classColors(layer);
  int realIndex = PiiLayerParser::realIndexStatic(layer, globalIndex);
  if (realIndex<0 || realIndex>=classColors.size())
    {
      //qDebug("PiiDefectMap::classColorStatic(): RETURN defaultColor");
      return defaultColor;
    }
  //qDebug("PiiDefectMap::classColorStatic(): RETURN classColors[realIndex]");
   return classColors[realIndex];
}

/* This private function intializes the internal cache of the layer
   parser. The cache is used for non-static function. The layers must
   be set, before calling this function. */
void PiiLayerParser::_initCache()
{
  //qDebug("PiiLayerParser::_initCache() BEGIN");
  //qDebug("                _iActiveLayer == %d", _iActiveLayer);
  //qDebug("                _lstLayers.size() == %d", _lstLayers.size());
  //qDebug("                _classColorsCache.size() == %d", _classColorsCache.size());
  _classColorsCache.clear();

  if (_iActiveLayer < _lstLayers.size())
    {
      _currLayerType = layerType(_lstLayers[_iActiveLayer]);
      if (_currLayerType == LayerTypeClass)
        {
          //qDebug("PiiLayerParser::_initCache() _currLayerType == LayerTypeClass ");
          _classColorsCache = globalClassColors(_lstLayers[_iActiveLayer], _defaultDefectColor);
        }
      else if (_currLayerType == LayerTypeFloat)
        {
          // Now, the cache for storing the colors are used to store
          // the start color and end color of the float layer.
          
          //qDebug("PiiLayerParser::_initCache() _currLayerType == LayerTypeFloat ");

          _classColorsCache.resize( 1+qMax( PII_LAYER_PARSER_START_COLOR_INDEX, PII_LAYER_PARSER_END_COLOR_INDEX));
          _classColorsCache[PII_LAYER_PARSER_START_COLOR_INDEX] = mapItem(_lstLayers[_iActiveLayer], "startColor", QVariant::Color).value<QColor>();
          _classColorsCache[PII_LAYER_PARSER_END_COLOR_INDEX] = mapItem(_lstLayers[_iActiveLayer], "endColor", QVariant::Color).value<QColor>();
          _dCurrMin = minValue(_lstLayers[_iActiveLayer]);
          _dCurrMax = maxValue(_lstLayers[_iActiveLayer]);
        }
                                                  
    }
  //qDebug("PiiLayerParser::_initCache() END");
}

/* This is a private static function. The function returns the user
   readable class name for the label given as a paramater. The
   function assumes, that the layer type is "class". The class type is
   not checked in the function. If the class name cannot be formed for
   any reason, an empty string is returned.*/
// QString PiiLayerParser::_getClassForLabel(const QVariant& layer, double label)
// {
//   QStringList classNames = classNames(layer);
//   int classIndex = int(label);
//   if (classIndex >= classNames.size())
//     return "";
//   else
//     return classNames[classIndex];
// }

/* This is a private static function, which returns the double value
   for the @p key given as a parameter. Such keys are for example
   "min", "max" and "resolution". If the key given as a parameter does
   not exists, or the type of data in the map item is not double, @p
   defaultValue given as a parameter is returned.*/
double PiiLayerParser::_getDoubleItemValue(const QVariant& layer, const QString& key, double defaultVal)
{
  QVariant doubleValue = mapItem(layer, key, QVariant::Double);
  if (doubleValue.isValid())
    return doubleValue.toDouble();
  else
    return defaultVal;
}

double PiiLayerParser::defaultNumericLabel(const QVariant& layer)
{
  PiiLayerParser::LayerType type = layerType(layer);

  if (type == PiiLayerParser::LayerTypeFloat)
    return minValue(layer, 0.0);
  else
    {
      if (hasGlobalIndices(layer))
        {
          // The global indices existed. The minimum value of them is returned.
          return double(smallestGlobalIndex(layer));
        }
      else
        {
          return 0.0;
        }
    }
}

double PiiLayerParser::minValue(const QVariant& layer, double defaultVal)
{
  return _getDoubleItemValue(layer, "min", defaultVal);
}

double PiiLayerParser::maxValue(const QVariant& layer, double defaultVal)
{
  return _getDoubleItemValue(layer, "max", defaultVal);
}

double PiiLayerParser::resolution(const QVariant& layer, double defaultVal)
{
  return _getDoubleItemValue(layer, "resolution", defaultVal);
}

/* PENDING
 * It should be still checked that the layer doesn't contain the
 * information that the new layer type cannot contain. For example the
 * "class" type layer cannot contain the keys "min" and "max". Those
 * kind of keys should be removed, when changing the layer type.
 */
/* Private member function, which does exactly the same than the
 * corresponding public function, but the first parameter is of type
 * QVariantMap in stead of QVariant.*/
void PiiLayerParser::_setLayerType(QVariantMap& layerMap, PiiLayerParser::LayerType layerType)
{
  switch(layerType)
    {
    case LayerTypeFloat:
      layerMap["type"] = "float";
      break;
    case LayerTypeClass:
      layerMap["type"] = "class";
      break;
    default:
      layerMap["type"] = "";
    }
}

int PiiLayerParser::smallestGlobalIndex(const QVariant& layer)
{
  QList <int> indices = classIndices(layer);
  if (indices.size() == 0)
    return -1;
  int minIndex = indices[0];
  for (int i = 1; i<indices.size(); i++)
    {
      if (indices[i] < minIndex)
        minIndex = indices[i];
    }
  return minIndex;
}

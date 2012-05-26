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


#include "PiiTableLabelerModel.h"
#include "PiiTableLabelerOperation.h" //(for PiiMinMax)

PiiTableLabelerModel::PiiTableLabelerModel(const QVariant& layer,
                                           const QVariantList& featureProperties,
                                           PiiMatrix< PiiMinMax > ruleMatrix,
                                           QList<double> labels,
                                           QList<bool> rules,
                                           QObject* parent)
  : QAbstractTableModel(parent),
    _layer(layer),
    _ruleMatrix(ruleMatrix),
    _lstLabels(labels),
    _lstRules(rules)
{
  setFeatureProperties(featureProperties);
}

int PiiTableLabelerModel::rowCount( const QModelIndex& /*parent*/) const
{
  return _ruleMatrix.rows();
}

int PiiTableLabelerModel::columnCount( const QModelIndex& /*parent*/) const
{
  return _ruleMatrix.columns() + 1;
}

QVariant PiiTableLabelerModel::data( const QModelIndex & index, int role ) const
{
  if (!index.isValid())
    return QVariant();

  //We have to take in to account, that the amount of columns has to
  //be one more compared to the width of the rule matrix.
  if ( index.row() >= _ruleMatrix.rows() ||
       index.column() > _ruleMatrix.columns() )
    return QVariant();

  PiiTableLabelerModel::ItemDataType itemDataType = indexDataType(index);

  if (role == ItemDataTypeRole)
    return itemDataType;
    
  
  if (itemDataType == PiiTableLabelerModel::ItemTypeMinMax)
    {
      if (index.column() >= _lstFeatureProperties.size())
        return QVariant();
      
      // Before showing the min max values to user, the values must be
      // converted to the column specific unit.
      double dUnitScale = _lstFeatureProperties[index.column()].value("scale", 1.0).toDouble();
      const PiiMinMax& minMax = _ruleMatrix(index.row(),index.column()) * dUnitScale;

      if (role == Qt::DisplayRole)
        {
          return QString("%1 - %2").arg(minMax.minAsString() ).arg(minMax.maxAsString());
        }
      else if (role == Qt::EditRole)
        {
          return minMax.toMap();
        }
      else if (role == Qt::TextColorRole)
        {
          if (minMax.dMin > minMax.dMax)
            return QColor(200,0,0);
        }
      else if (role == Qt::ToolTipRole)
        {
          if (minMax.dMin > minMax.dMax)
            return "Warning: minimum value greater than maximum value";
        }
      else if (role == Qt::TextAlignmentRole)
        {
          return Qt::AlignCenter;
        }
    }
  else if (itemDataType == PiiTableLabelerModel::ItemTypeLabel)
    {
      if (index.column() >= _lstFeatureProperties.size())
        return QVariant();

      // Before showing the min max values to user, the values must be
      // converted to the column specific unit.
      const PiiMinMax& minMax = _ruleMatrix(index.row(),index.column());
      int value = int(minMax.dMin + 0.5);

      if (role == Qt::DisplayRole)
        {
          QStringList classNames = _lstFeatureProperties[index.column()].value("classNames").toStringList();
          
          //First label in combobox must be undefined.
          if (value < 0 || value >= classNames.size())
            return "-";

          return classNames[value];
        }
      else if (role == Qt::EditRole)
        {
          return value; //the index to the currently selected class
        }
      else if (role == Qt::TextAlignmentRole)
        {
          return Qt::AlignCenter;
        }
    }
  else if (itemDataType == PiiTableLabelerModel::ItemTypeClassLabel)
    {
      int realIndex = PiiLayerParser::realIndexStatic(_layer, int(_lstLabels[index.row()]));
      if (role == Qt::DisplayRole)
        {
          return PiiLayerParser::classNameStatic(_layer, realIndex);
        }
      else if (role == Qt::EditRole)
        {
          //return the index to the currently selected class.
          return realIndex;
        }
    }
  else if (itemDataType == PiiTableLabelerModel::ItemTypeFloatLabel)
    {
      if (role == Qt::DisplayRole)
        {
          return PiiLayerParser::labelAsString(_layer, _lstLabels[index.row()]);
        }
      else if (role == Qt::EditRole)
        {
          //return the label value itself
          return _lstLabels[index.row()];
        }
    }
   
  return QVariant();

}

bool PiiTableLabelerModel::setData(const QModelIndex &index,
                                   const QVariant &value,
                                   int role)
{
  if (index.isValid() && role == Qt::EditRole)
    {
      PiiTableLabelerModel::ItemDataType itemDataType = indexDataType(index);

      if ( itemDataType == PiiTableLabelerModel::ItemTypeMinMax )
        {
          double dUnitScale = 1.0;
          if (index.column() < _lstFeatureProperties.size())
            dUnitScale = _lstFeatureProperties[index.column()].value("scale", 1.0).toDouble();
          
          _ruleMatrix(index.row(), index.column()) = PiiMinMax(value) / dUnitScale;
          emit dataChanged(index, index);
          emit changedStatusChanged(true);
          emit layoutChanged();
          return true;
        }
      else if (itemDataType == PiiTableLabelerModel::ItemTypeFloatLabel )
        {
          if (index.row() < _lstLabels.size())
            {
              if (value.canConvert(QVariant::Double))
                {
                  _lstLabels[index.row()] = value.toDouble();
                  emit dataChanged(index, index);
                  emit changedStatusChanged(true);
                  return true;
                }
            }
        }
      else if (itemDataType == PiiTableLabelerModel::ItemTypeLabel)
        {
          //First label in combobox must be undefined. That's why we
          //must decrease the value, if it is > 0.
          if (value.toDouble() <= 0)
            _ruleMatrix(index.row(), index.column()) = PiiMinMax(-INFINITY, INFINITY);
          else
            _ruleMatrix(index.row(), index.column()) = PiiMinMax(value.toDouble()-1,value.toDouble()-1);
          
          emit dataChanged(index, index);
          emit changedStatusChanged(true);
          return true;
        } 
      else if (itemDataType == PiiTableLabelerModel::ItemTypeClassLabel )
        {
          //The class type index is assumed to be received as an int.
          //Here it must be converted to double.
          if (index.row() < _lstLabels.size())
            {
              if (value.canConvert(QVariant::Int))
                {
                  _lstLabels[index.row()] = double(PiiLayerParser::classIndexStatic(_layer, value.toInt()));
                  emit dataChanged(index, index);
                  emit changedStatusChanged(true);
                  return true;
                }
            }
        }
    }
  return false;
}

Qt::ItemFlags PiiTableLabelerModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant PiiTableLabelerModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal)
    {
      // If there are more features than there are columns in the rule
      // matrix, number of features shown is equal to the amount of
      // the columns. In the opposite case (amount of features less
      // than amount of columns) empty strings are used as a
      // feature names.
      if (section < _ruleMatrix.columns())
        {
          if (section < _lstFeatureProperties.size())
            return formattedHeaderText(section);
          else
            return "";
        }
      else if (section == _ruleMatrix.columns())
        return  PiiLayerParser::layerName(_layer);
      else
        return QVariant();
    }
  else
    return QString("%1 (%2)").arg(section + 1).arg(_lstRules[section] ? tr("All") : tr("Any"));
}

bool PiiTableLabelerModel::insertRows(int position, int rows, const QModelIndex & /*parent*/)
{
  beginInsertRows(QModelIndex(), position, position+rows-1);

  for (int row = 0; row < rows; ++row)
    {
      PiiMatrix<PiiMinMax> rowMatrix = PiiMatrix<PiiMinMax>(1,_ruleMatrix.columns());
      for (int i = 0; i<rowMatrix.columns(); i++)
        // Fill the minMax sructure with the default values
        // (-INFINITY and INFINITY)
        rowMatrix(0,i) = PiiMinMax();  
                                      
      _ruleMatrix.insertRow(position, rowMatrix);
      _lstLabels.insert(position, PiiLayerParser::defaultNumericLabel(_layer));
      _lstRules.insert(position, true);
    }

  emit changedStatusChanged(true);
  endInsertRows();
  return true;
}

bool PiiTableLabelerModel::removeRows(int position, int rows, const QModelIndex & /*parent*/)
{
  beginRemoveRows(QModelIndex(), position, position+rows-1);

  for (int row = 0; row < rows; ++row)
    {
      if (position < _ruleMatrix.rows())
          _ruleMatrix.removeRow(position);
      if (position < _lstLabels.size())
        _lstLabels.removeAt(position);
      if (position < _lstRules.size())
        _lstRules.removeAt(position);
    }

  emit changedStatusChanged(true);
  endRemoveRows();
  return true;
}

void PiiTableLabelerModel::moveRowsUp(int position, int rows, const QModelIndex & parent)
{
  //Move the rows up one at a time,starting from the top.  
  for (int i = position ; i < position+rows ; ++i)
    {
      if (i>0)
        {
          if ( i < rowCount())
              _ruleMatrix.swapRows	(i-1, i );
          if (i < _lstLabels.size())
            _lstLabels.swap(i-1,i);
          if (i < _lstRules.size())
            _lstRules.swap(i-1,i);
        }
    }

  //Inform the view about the changed data.
  QModelIndex topLeft = parent.child(position-1, 0);
  QModelIndex bottomRight = parent.child(position+rows-1, columnCount());
  emit dataChanged(topLeft, bottomRight);
  emit changedStatusChanged(true);
}


void PiiTableLabelerModel::moveRowsDown(int position, int rows, const QModelIndex & parent)
{
  //Move the rows down one at a time,starting from the bottom.
  for (int i = position+rows ; i > position ; --i)
    {
      if (i>0)
        {
          if ( i < rowCount())
              _ruleMatrix.swapRows	(i-1, i );
          if (i < _lstLabels.size())
            _lstLabels.swap(i-1,i);
          if (i < _lstRules.size())
            _lstRules.swap(i-1,i);
        }
    }

  //Inform the view about the changed data.
  QModelIndex topLeft = parent.child(position, 0);
  QModelIndex bottomRight = parent.child(position+rows, columnCount());
  emit dataChanged(topLeft, bottomRight);
  emit changedStatusChanged(true);
}

void PiiTableLabelerModel::changeRuleType(const QList<int>& rows)
{
  for (int i=0; i<rows.size(); i++)
    {
      int row = rows[i];
      if (row >= 0 && row < _lstRules.size())
        {
          QModelIndex index = createIndex(row,0);
          _lstRules[row] = !_lstRules[row];
          emit dataChanged(index, index);
          emit changedStatusChanged(true);
          emit layoutChanged();
        }
    }
}

PiiMatrix<PiiMinMax> PiiTableLabelerModel::ruleMatrix() const
{
  return _ruleMatrix;
}

void PiiTableLabelerModel::setLayer(const QVariant& layer)
{
  _layer = layer;
  emit layoutChanged();
}

void PiiTableLabelerModel::setContent(const PiiMatrix< PiiMinMax >& ruleMatrix,
                                      const QList<double>& labelVector,
                                      const QList<bool>& ruleVector)
{
  _ruleMatrix = ruleMatrix;
  _lstLabels = labelVector;
  _lstRules = ruleVector;
  emit layoutChanged();
}

void PiiTableLabelerModel::setFeatureProperties(const QVariantList& featureProperties)
{
  _lstFeatureProperties = Pii::variantsToList<QVariantMap>(featureProperties);
}

PiiTableLabelerModel::ItemDataType PiiTableLabelerModel::indexDataType( const QModelIndex &index ) const
{
  //Find out the layer
  if (index.column() == _ruleMatrix.columns())
    {
      //The index points to the label column. The item type is eihter
      //"class" or "float"

      if ( index.row() < _lstLabels.size() )
        {
          //a label to the row found

          //Check the layer type
          PiiLayerParser::LayerType layerType = PiiLayerParser::layerType(_layer);
          if (layerType == PiiLayerParser::LayerTypeFloat )
            return PiiTableLabelerModel::ItemTypeFloatLabel;
          else if ( layerType == PiiLayerParser::LayerTypeClass )
            return PiiTableLabelerModel::ItemTypeClassLabel;
          else
            return PiiTableLabelerModel::ItemTypeUndefined;
        }
      else
        {
          //Label index is out of the bounds
          return PiiTableLabelerModel::ItemTypeUndefined;
          //return QVariant();
        }
         
    }
  else
    {
      int c = index.column();

      if (c < _lstFeatureProperties.size())
        {
          if (_lstFeatureProperties[c].contains("classNames"))
            return ItemTypeLabel;
          else
            return ItemTypeMinMax;
        }
      else
        return ItemTypeUndefined;
    }
}

/* This is a protected helper function, which returns the formatted
 * header data. Formatted means, that in addition to the feature name,
 * there are also units in the header name.
 */
QString PiiTableLabelerModel::formattedHeaderText(int section) const
{
  QString units;
  
  if (_lstFeatureProperties[section].contains("unit"))
    units = QString(" (%1)").arg(_lstFeatureProperties[section].value("unit").toString());
    
  return _lstFeatureProperties[section].value("displayName", _lstFeatureProperties[section].value("name", "")).toString() + units;
}


QVariant PiiTableLabelerModel::featureProperty(int column, const QString& name) const
{
  if (column < _lstFeatureProperties.size() && _lstFeatureProperties[column].contains(name))
    return _lstFeatureProperties[column].value(name);

  return QVariant();
}


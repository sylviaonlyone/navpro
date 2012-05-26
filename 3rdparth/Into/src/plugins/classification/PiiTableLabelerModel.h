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

#ifndef _PIITABLELABELERMODEL_H
#define _PIITABLELABELERMODEL_H

#include <PiiMatrix.h>

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QStringList>
#include <QSizeF>
#include <QString>

class PiiMinMax;
class PiiTableLabelerOperation;

/**
 * The following constants are used with enum Qt::ItemDataRole. They
 * are custom roles that PiiTableLabelerModel uses
 *
 * @p ItemDataTypeRole defines the data type of the item in the role. It
 * contain the enum value PiiTableLabelerModel::ItemDataType.
 */
const int ItemDataTypeRole = Qt::UserRole;

/**
 * TODO: Add comment here!
 */
class PiiTableLabelerModel : public QAbstractTableModel
{
  Q_OBJECT

public:


  /**
   * This enum type defines the ItemDataType of the Table. It is used
   * as a value for the role ItemDataTypeRole in the model. ItemDataType
   * tells, what kind of data is stored in the item. Delegate class needs
   * this information, when editing the content of the item.
   *
   * @lip ItemTypeUndefined - undefined value
   *
   * @lip ItemTypeMinMax - minimum and maximum values
   *   
   * @lip ItemTypeClass - class type label
   *
   * @lip ItemTypeFloat - float type label
   *
   */  
  enum ItemDataType
    {
      ItemTypeUndefined = 0x00,
      ItemTypeMinMax = 0x08,
      ItemTypeLabel = 0x10,
      ItemTypeClassLabel = ItemTypeLabel + 0x01,
      ItemTypeFloatLabel = ItemTypeLabel + 0x02
    };
  
  /* PENDING
   * COMPLEMENT THIS COMMENT BLOCK!!
   */
  PiiTableLabelerModel( const QVariant& layer,
                        const QVariantList& featureProperties,
                        PiiMatrix< PiiMinMax > ruleMatrix,
                        QList<double> labels,
                        QList<bool> rules,
                        QObject* parent);

  int rowCount(const QModelIndex &parent = QModelIndex() ) const;

  int columnCount( const QModelIndex &parent = QModelIndex() ) const;

  /**
   * This function implements the corresponding pure virtual function
   * from the base class QAbstaractTableModel. The function
   * returns the mininum and maximum values from the model. The format
   * of the returned data depends on the value of @p role. With the
   * role value Qt:DisplayRole the minimum and maximum values are returned
   * in a single string "MIN - MAX", where MIN and MAX contain the
   * double values. With the role value Qt:EditRole the minimum and
   * maximum values are returned in a QMap<QString,QVariant> data
   * structure, where the field ["min"] contains the minimum value as
   * a double and the field ["max"] the maximum value as a double.
   */
  QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;

  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole);

  Qt::ItemFlags flags(const QModelIndex &index) const;
  
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;

  /**
   * The following function returns the layer of the model. The
   * returned value contains a QMap converted to QVariant. The
   * contents of the layer structure is explained more detailed in the
   * documentation of @p PiiLayerParser.
   */
  const QVariant& layer() const {return _layer;}

  /**
   * This function implements the pure virtual function
   * declared in the base class AbstractItemModel. The @p rows amount
   * of empty rows are added in the position @position.
   */  
  bool insertRows(int position, int rows, const QModelIndex &parent);

  /**
   * This function implements the pure virtual function
   * declared in the base class AbstractItemModel. The @p rows amount
   * of rows are deleted from the position @position.
   */  
  bool removeRows(int position, int rows, const QModelIndex &parent);  

  /**
   * This function moves the @rows amount of rows upward in the
   * position @p position.
   */ 
  void moveRowsUp(int position, int rows, const QModelIndex & parent = QModelIndex());

  /**
   * This function moves the @rows amount of rows downward in the
   * position @p position.
   */ 
  void moveRowsDown(int position, int rows, const QModelIndex & parent = QModelIndex());

  /**
   * Returns the rule matrix from the model. The function returns the
   * values in the rule matrix as pixel, in stead of units.
   */
  PiiMatrix<PiiMinMax> ruleMatrix() const; // {return _ruleMatrix;}

  /**
   * Returns the label vector from the model;
   */
  QList<double> labelVector() const {return _lstLabels;}

  /**
   * Returns the rule vector from the model;
   */
  QList<bool> ruleVector() const {return _lstRules;}

  /**
   * Sets a layer for the model.
   */
  void setLayer(const QVariant& layer);

  /**
   * Sets the rule matrix, the label vector and the rule vector for the model. The signal
   * @p layoutChanged emitted as a result of calling this function.
  */
  void setContent(const PiiMatrix< PiiMinMax >& ruleMatrix, const QList<double>& labelVector, const QList<bool>& ruleVector);

  /**
   * Sets the properties of the classification features, that are
   * stored in the model. The parameter @p featureProperties contains
   * the list of QVariants, where each item is a QVariantMap
   * containing the following key-value pairs:
   *
   * @li name - the name of the feature (QString).
   *
   * @li unit - the unit of the feature (QString).
   *
   * @li scale - the scale of the unit (double).
   *
   * @li classNames - the names of the class labels
   *
   * @li classIndices - the unique indices of the class labels
   *
   * TODO: COMPLEMENT THIS COMMENT BLOCK.
   */
  void setFeatureProperties(const QVariantList& featureProperties);

  /**
   * Get the property of the feature properties list by the given
   * index. If the index is outside from the property list, returns an
   * empty QVariant.
   */
  QVariant featureProperty(int colum, const QString& name) const;

  void changeRuleType(const QList<int>& rows);
  
signals:
  /**
   * This signal is emitted, when the model has been modified in a
   * way, that the chagned status of the model has been changed. Refer
   * to the documentation of the function @p setChanged() of the class
   * @p PiiConfigurationWidget to get the more information, what
   * "changed status" means.
   */
  void changedStatusChanged(bool status);   

protected:
  QString formattedHeaderText(int section) const;
  
private:
  QVariant _layer;
  QList<QVariantMap> _lstFeatureProperties;
  PiiMatrix< PiiMinMax > _ruleMatrix;
  QList<double> _lstLabels;
  QList<bool> _lstRules;
  
  ItemDataType indexDataType(const QModelIndex &index) const;  
};


#endif //_PIITABLELABELERMODEL_H

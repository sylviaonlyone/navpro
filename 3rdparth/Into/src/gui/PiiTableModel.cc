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

#include "PiiTableModel.h"

#include "PiiTableModelDelegate.h"
#include "PiiModelItem.h"

#include <PiiUtil.h>
#include "PiiGuiUtil.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>

#include <QRegExpValidator>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QItemSelectionModel>
#include <QAbstractItemView>

PiiTableModel::Data::Data(PiiTableModel* model) :
  _pDelegate(new PiiTableModelDelegate(model)),
  _bCanDeleteLast(true)
{
  // Create an empty header. First row in _lstItems is the header. 
  // Table data starts at row 1.
  _lstItems << QList<PiiModelItem*>();
}

PiiTableModel::Data::~Data()
{
  // Delete header
  qDeleteAll(_lstItems[0]);
}

PiiTableModel::PiiTableModel(QAbstractItemView *parent) :
  QAbstractTableModel(parent),
  d(new Data(this))
{
  initialize(parent);
}

PiiTableModel::PiiTableModel(Data* data, QAbstractItemView *parent) :
  QAbstractTableModel(parent),
  d(data)
{
  initialize(parent);
}

void PiiTableModel::initialize(QAbstractItemView* parent)
{
  parent->setModel(this);
  QItemSelectionModel* pSelectionModel = parent->selectionModel();
  connect(pSelectionModel, SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          SIGNAL(currentItemChanged()));
  connect(pSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          SLOT(enableControls()));
  parent->setItemDelegate(d->_pDelegate);
  parent->setSelectionBehavior(QAbstractItemView::SelectRows);
}

PiiTableModel::~PiiTableModel()
{
  // Delete data
  clear();
  delete d;
}

void PiiTableModel::clear()
{
  // Delete everything except the header
  for (int i=d->_lstItems.size()-1; i>=1; --i)
    qDeleteAll(d->_lstItems.takeAt(i));
  reset();
  enableControls();
}

void PiiTableModel::insertRow(const QList<PiiModelItem*>& items, int row)
{
  if (items.size() != d->columns())
    {
      qDeleteAll(items);
      return;
    }

  if (row < 0 || row > d->rows())
    row = d->rows();

  beginInsertRows(QModelIndex(), row, row);
  d->_lstItems.insert(row+1, items);
  endInsertRows();
}

QList<PiiModelItem*> PiiTableModel::takeRow(int row)
{
  if (row >= 0 && row < d->rows())
    {
      beginRemoveRows(QModelIndex(), row, row);
      QList<PiiModelItem*> lstRow = d->_lstItems.takeAt(row+1);
      endRemoveRows();
      return lstRow;
    }
  return QList<PiiModelItem*>();
}

int PiiTableModel::rowCount(const QModelIndex &parent) const
{
  return parent.isValid() ? 0 : d->rows();
}

int PiiTableModel::columnCount(const QModelIndex &parent) const
{
  return parent.isValid() ? 0 : d->columns();
}

QVariant PiiTableModel::data(const QModelIndex &index, int role) const
{
  const int iRow = index.row(), iCol = index.column();
  if (!index.isValid() ||
      iRow < 0 || iRow >= d->rows() ||
      iCol < 0 || iCol >= d->columns())
    return QVariant();
  return d->_lstItems[iRow+1][iCol]->data(role);
}

bool PiiTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  const int iRow = index.row(), iCol = index.column();
  if (!index.isValid() ||
      iRow < 0 || iRow >= d->rows() ||
      iCol < 0 || iCol >= d->columns())
    return false;
  d->_lstItems[iRow+1][iCol]->setData(role, value);
  emit dataChanged(index, index);
  return true;
}

PiiModelItem* PiiTableModel::itemAt(int row, int column) const
{
  if (row < 0 || row >= d->rows() ||
      column < 0 || column >= d->columns())
    return 0;
  return d->_lstItems[row+1][column];
}

QVariant PiiTableModel::data(int row, int column, int role) const
{
  PiiModelItem* pItem = itemAt(row, column);
  if (pItem != 0)
    return pItem->data(role);
  return QVariant();
}

void PiiTableModel::setData(int row, int column, const QVariant& value, int role)
{
  PiiModelItem* pItem = itemAt(row, column);
  if (pItem != 0)
    {
      pItem->setData(role, value);
      QModelIndex idx = index(row, column);
      emit dataChanged(idx, idx);
    }
}

void PiiTableModel::setValue(int row, int column, const QVariant& value, ValueChangeBehavior behavior)
{
  PiiModelItem* pItem = itemAt(row, column);
  if (pItem != 0)
    {
      pItem->setData(ColumnEditorValueRole, value);
      if (behavior == ChangeTextAutomatically)
        pItem->setText(textForValue(column, value));
      QModelIndex idx = index(row, column);
      emit dataChanged(idx, idx);
    }
}


QMap<int, QVariant> PiiTableModel::itemData(const QModelIndex &index) const
{
  const int iRow = index.row(), iCol = index.column();
  if (!index.isValid() ||
      iRow < 0 || iRow >= d->rows() ||
      iCol < 0 || iCol >= d->columns())
    return QMap<int, QVariant>();

  return d->_lstItems[iRow+1][iCol]->dataMap();
}

Qt::ItemFlags PiiTableModel::flags(const QModelIndex &index) const
{
  const int iRow = index.row(), iCol = index.column();
  if (!index.isValid() ||
      iRow < 0 || iRow >= d->rows() ||
      iCol < 0 || iCol >= d->columns())
    return Qt::ItemIsDropEnabled; // allow drops outside of the items
  return d->_lstItems[iRow+1][iCol]->flags();
}

QVariant PiiTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Vertical)
    {
      if (role == Qt::DisplayRole)
        return section + 1;
    }
  else
    {
      if (section >= 0 && section < d->columns())
        return d->_lstItems[0][section]->data(role);
    }
  return QVariant();
}

bool PiiTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
  if (orientation == Qt::Vertical)
    return false;

  if (section >= 0 && section < d->columns())
    {
      // Set data
      d->_lstItems[0][section]->setData(role, value);
      // Inform view
      emit headerDataChanged(orientation, section, section);
      return true;
    }
  return false;
}

void PiiTableModel::setHeaderTitles(const QStringList& headerTitles)
{
  for (int i=0; i<headerTitles.size(); ++i)
    setHeaderData(i, Qt::Horizontal, headerTitles[i], Qt::DisplayRole);
}

bool PiiTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
  if (count < 1 || row < 0 || row > d->rows() || parent.isValid())
    return false;

  beginInsertRows(QModelIndex(), row, row + count - 1);
  for (int r = row; r < row + count; ++r)
    d->_lstItems.insert(r+1, createRow(r));
  endInsertRows();
  return true;
}

QList<PiiModelItem*> PiiTableModel::createRow(int row)
{
  QList<PiiModelItem*> lstRow;
  if (row == -1)
    row = d->rows();
  for (int c=0; c < d->columns(); ++c)
    lstRow << createItem(row, c);
  return lstRow;
}

bool PiiTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
  if (count < 1 || row < 0 || (row + count) > d->rows() || parent.isValid())
    return false;

  beginRemoveRows(QModelIndex(), row, row + count - 1);
  for (int r = 0; r < count; ++r)
    qDeleteAll(d->_lstItems.takeAt(row+1));
  endRemoveRows();
  return true;
}

bool PiiTableModel::insertColumns(int column, int count, const QModelIndex &parent)
{
  if (count < 1 || column < 0 || column > d->columns() || parent.isValid())
    return false;

  beginInsertColumns(QModelIndex(), column, column + count - 1);
  for (int c = column; c < column + count; ++c)
    for (int r = 0; r < d->_lstItems.size(); ++r)
      d->_lstItems[r].insert(c, createItem(r-1, c));
  endInsertColumns();
  return true;
}

bool PiiTableModel::removeColumns(int column, int count, const QModelIndex &parent)
{
  if (count < 1 || column < 0 || (column + count) > d->columns() || parent.isValid())
    return false;

  beginRemoveColumns(QModelIndex(), column, column + count - 1);
  for (int c = 0; c < count; ++c)
    for (int r = 0; r < d->_lstItems.size(); ++r)
      delete d->_lstItems[r].takeAt(column);
  endRemoveColumns();
  return true;
}

QString PiiTableModel::textForValue(int column, const QVariant& value) const
{
  switch (columnEditorType(column))
    {
    case ComboBoxEditor:
      // If the value is an integer or does not exist, take default
      // text from the combo's item list.
      if (!value.isValid() || value.type() == QVariant::Int)
        {
          QStringList lstItems = columnEditorProperties(column).value("items").toStringList();
          // Invalid variant turns to zero here
          int iListIndex = value.toInt();
          if (iListIndex >= 0 && iListIndex < lstItems.size())
            return lstItems[iListIndex];
        }
      break;
    case IntegerSpinBoxEditor:
    case DoubleSpinBoxEditor:
      {
        QVariantMap mapProps = columnEditorProperties(column);
        if (mapProps.contains("specialValueText") &&
            mapProps.contains("minimum") &&
            value.toDouble() == mapProps["minimum"].toDouble())
          return mapProps["specialValueText"].toString();
        else
          {
            QString strNumber;
            if (mapProps.contains("decimals"))
              strNumber = QString::number(value.toDouble(), 'f', mapProps["decimals"].toInt());
            else
              strNumber = value.toString();
            return mapProps["prefix"].toString() + strNumber +
              mapProps["suffix"].toString();
          }
      }
      break;
    case LineEditor:
    default:
      break;
    }

  return value.toString();
}

PiiModelItem* PiiTableModel::createItem(int /*row*/, int column)
{
  PiiModelItem* pItem = new PiiModelItem();
  QVariant value = defaultValue(column);
  if (value.isValid())
    {
      pItem->setText(textForValue(column, value));
      pItem->setData(ColumnEditorValueRole, value);
    }
  return pItem;
}

QWidget* PiiTableModel::createEditor(QWidget* parent,
                                     int /*row*/, int column) const
{
  QVariantMap mapEditorProps = columnEditorProperties(column);
  QWidget* pWidget = 0;
  switch (columnEditorType(column))
    {
    case LineEditor:
      {
        QLineEdit* pEditor = new QLineEdit(parent);
        // If the editor's property map contains validator
        // configuration, auto-detect validator type.
        if (mapEditorProps.contains("validator"))
          {
            QVariantMap mapValidator = mapEditorProps.take("validator").toMap();
            // Regexp validator
            if (mapValidator.contains("regexp"))
              pEditor->setValidator(new QRegExpValidator(QRegExp(mapValidator["regexp"].toString()), pEditor));
            // Numeric validators, type detected by the variant type
            else if (mapValidator.contains("top") && mapValidator.contains("bottom"))
              {
                if (mapValidator["top"].type() == QVariant::Double)
                  {
                    QDoubleValidator* pValidator = new QDoubleValidator(pEditor);
                    // Default values
                    pValidator->setDecimals(2);
                    pValidator->setNotation(QDoubleValidator::StandardNotation);
                    Pii::setProperties(*pEditor, mapValidator);
                    pEditor->setValidator(pValidator);
                  }
                else
                  pEditor->setValidator(new QIntValidator(mapValidator["bottom"].toInt(),
                                                          mapValidator["top"].toInt(),
                                                          pEditor));
              }
          }
        pWidget = pEditor;
        break;
      }
    case IntegerSpinBoxEditor:
      pWidget = new QSpinBox(parent);
      break;
    case DoubleSpinBoxEditor:
      pWidget = new QDoubleSpinBox(parent);
      break;
    case ComboBoxEditor:
      {
        QComboBox* pCombo = new QComboBox(parent);
        // Combo items cannot be directly set as a property. Must
        // handle separately.
        if (mapEditorProps.contains("items"))
          pCombo->addItems(mapEditorProps.take("items").toStringList());
        if (mapEditorProps.contains("colors"))
          {
            QList<QColor> lstColors = Pii::variantsToList<QColor>(mapEditorProps.take("colors").toList());
            for (int i=0; i<qMin(lstColors.size(), pCombo->count()); ++i)
              pCombo->setItemIcon(i, PiiGui::createIcon(20, 20, lstColors[i]));
          }
        if (mapEditorProps.contains("icons"))
          {
            QList<QIcon> lstIcons = Pii::variantsToList<QIcon>(mapEditorProps.take("icons").toList());
            for (int i=0; i<qMin(lstIcons.size(), pCombo->count()); ++i)
              pCombo->setItemIcon(i, lstIcons[i]);
          }
        pWidget = pCombo;
      }
    }

  pWidget->setContentsMargins(0,0,0,0);
  // Set remaining properties to the widget
  Pii::setProperties(*pWidget, mapEditorProps);
  
  return pWidget;
}

void PiiTableModel::setEditorData(QWidget* editor, int row, int column) const
{
  PiiModelItem* pItem = itemAt(row, column);
  if (pItem == 0)
    return;
  
  switch (columnEditorType(column))
    {
    case LineEditor:
      editor->setProperty("text", pItem->text());
      break;
    case IntegerSpinBoxEditor:
    case DoubleSpinBoxEditor:
      editor->setProperty("value", pItem->data(ColumnEditorValueRole));
      break;
    case ComboBoxEditor:
      editor->setProperty("currentIndex", pItem->data(ColumnEditorValueRole));
      break;
    }
}

void PiiTableModel::setModelData(QWidget* editor, int row, int column)
{
  if (row < 0 || row >= d->rows() ||
      column < 0 || column >= d->columns())
    return;

  PiiModelItem* pItem = d->_lstItems[row+1][column];
  switch (columnEditorType(column))
    {
    case LineEditor:
      pItem->setData(Qt::DisplayRole, editor->property("text"));
      break;
    case IntegerSpinBoxEditor:
    case DoubleSpinBoxEditor:
      pItem->setData(Qt::DisplayRole, editor->property("text"));
      pItem->setData(ColumnEditorValueRole, editor->property("value"));
      break;
    case ComboBoxEditor:
      pItem->setData(Qt::DisplayRole, editor->property("currentText"));
      pItem->setData(ColumnEditorValueRole, editor->property("currentIndex"));
      break;
    }
  QModelIndex idx(index(row, column));
  emit dataChanged(idx, idx);
}


QVariantList PiiTableModel::columnValues(int column) const
{
  QVariantList result;
  for (int r=0; r<d->rows(); ++r)
    result << value(r, column);
  return result;
}

void PiiTableModel::setColumnValues(int column, const QVariantList& values)
{
  if (d->rows() > values.size())
    removeRows(values.size(), d->rows() - values.size());
  else if (d->rows() < values.size())
    insertRows(d->rows(), values.size() - d->rows());
  for (int r=0; r<values.size(); ++r)
    setValue(r, column, values[r]);
}

QStringList PiiTableModel::columnTexts(int column) const
{
  QStringList result;
  for (int r=0; r<d->rows(); ++r)
    result << text(r, column);
  return result;
}

void PiiTableModel::setColumnTexts(int column, const QStringList& texts)
{
  if (d->rows() > texts.size())
    removeRows(texts.size(), d->rows() - texts.size());
  else if (d->rows() < texts.size())
    insertRows(d->rows(), texts.size() - d->rows());
  for (int r=0; r<texts.size(); ++r)
    setText(r, column, texts[r]);
}


void PiiTableModel::enableControls()
{
  emit moveUpEnabled(canMoveUp());
  emit moveDownEnabled(canMoveDown());
  emit deleteEnabled(canDelete());
  emit selectionChanged();
}

bool PiiTableModel::canMoveUp() const
{
  QModelIndexList lstSelectedItems = static_cast<QAbstractItemView*>(QObject::parent())->selectionModel()->selectedIndexes();
  if (lstSelectedItems.size() == 0)
    return false;
  for (QModelIndexList::iterator i=lstSelectedItems.begin(); i != lstSelectedItems.end(); ++i)
    if (i->row() == 0)
      return false;
  return true;
}

bool PiiTableModel::canMoveDown() const
{
  QModelIndexList lstSelectedItems = static_cast<QAbstractItemView*>(QObject::parent())->selectionModel()->selectedIndexes();
  if (lstSelectedItems.size() == 0)
    return false;
  for (QModelIndexList::iterator i=lstSelectedItems.begin(); i != lstSelectedItems.end(); ++i)
    if (i->row() == d->rows()-1)
      return false;
  return true;
}

bool PiiTableModel::canDelete() const
{
  return static_cast<QAbstractItemView*>(QObject::parent())->selectionModel()->hasSelection() &&
    (d->_bCanDeleteLast || d->rows() > 1);
}

bool PiiTableModel::canDeleteLast() const
{
  return d->_bCanDeleteLast;
}

void PiiTableModel::setCanDeleteLast(bool canDeleteLast)
{
  d->_bCanDeleteLast = canDeleteLast;
}

QList<int> PiiTableModel::selectedRows() const
{
  QModelIndexList lstSelectedItems = selectionModel()->selectedIndexes();
  QList<int> lstSelectedRows;
  for (QModelIndexList::iterator i=lstSelectedItems.begin(); i != lstSelectedItems.end(); ++i)
    {
      int iRow = i->row();
      if (lstSelectedRows.indexOf(iRow) == -1)
        lstSelectedRows << iRow;
    }
  qSort(lstSelectedRows);
  return lstSelectedRows;
}

void PiiTableModel::moveSelectedRowsUp()
{
  if (!canMoveUp()) return;
  QList<int> lstRows = selectedRows();
  for (int i=0; i<lstRows.size(); ++i)
    {
      insertRow(takeRow(lstRows[i]), lstRows[i]-1);
      --lstRows[i];
    }
  selectRows(lstRows);
  enableControls();
}

void PiiTableModel::moveSelectedRowsDown()
{
  if (!canMoveDown()) return;
  QList<int> lstRows = selectedRows();
  for (int i=lstRows.size(); i--; )
    {
      insertRow(takeRow(lstRows[i]), lstRows[i]+1);
      ++lstRows[i];
    }
  selectRows(lstRows);
  enableControls();
}

void PiiTableModel::addRow()
{
  selectionModel()->clear();
  insertRow(createRow());
  enableControls();
  selectRow(d->rows()-1);
}

void PiiTableModel::deleteSelectedRows()
{
  QList<int> lstRows = selectedRows();
  selectionModel()->clear();
  for (int i=lstRows.size(); i--; )
    removeRows(lstRows[i]);
  selectRow(qMin(d->rows()-1, lstRows.last() + 1 - lstRows.size()));
  enableControls();
}

void PiiTableModel::selectRows(const QList<int>& rows)
{
  selectionModel()->clear();
  for (QList<int>::const_iterator i = rows.begin(); i != rows.end(); ++i)
    selectRow(*i);
}

void PiiTableModel::selectRow(int row)
{
  selectionModel()->select(index(row, 0),
                              QItemSelectionModel::Select |
                              QItemSelectionModel::Rows |
                              QItemSelectionModel::Current);
}

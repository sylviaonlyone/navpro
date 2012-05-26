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

#include "PiiLayerEditor.h"
#include <QtGui>
#include <QWidget>
#include <ui_type_info.h>

#include <QtDebug>

PiiLayerEditor::PiiLayerEditor(const QList<QVariant>& layerList, QWidget *parent) :
  QWidget(parent)
{
  setLayerList(layerList);
  init();
}

PiiLayerEditor::PiiLayerEditor(QWidget *parent) : QWidget(parent)
{
  init();
}

PiiLayerEditor::~PiiLayerEditor()
{
}

QList<QVariant> PiiLayerEditor::layerList() const
{
  return QVariantList() << _dataMap;
}

void PiiLayerEditor::setLayerList(const QList<QVariant>& layerList)
{
  if (layerList.isEmpty())
    _dataMap = QVariantMap();
  else
    _dataMap = layerList[0].toMap();
  
  initState();
}
  
void PiiLayerEditor::setVisible( bool visible )
{
  if ( visible )
    initState();
  QWidget::setVisible(visible);
}

void PiiLayerEditor::modeChanged()
{
  _stackedWidget->setCurrentIndex(_btnFloatMode->isChecked() ? 0 : 1);
  _dataMap["type"] = _btnFloatMode->isChecked() ? "float" : "class";
  checkOkButtonState();
}


void PiiLayerEditor::init()
{
  setupUi(this);

  _lstInitialColors << QColor(Qt::red)
                    << QColor(Qt::green)
                    << QColor(Qt::blue)
                    << QColor(Qt::cyan)
                    << QColor(Qt::magenta)
                    << QColor(Qt::yellow)
                    << QColor(Qt::gray)
                    << QColor(Qt::darkRed)
                    << QColor(Qt::darkGreen)
                    << QColor(Qt::darkBlue)
                    << QColor(Qt::darkCyan)
                    << QColor(Qt::darkMagenta)
                    << QColor(Qt::darkYellow)
                    << QColor(Qt::darkGray);
  _iMaxId = 0;
  _iMaxClassIndex = 0;
  
  connect(_txtMeasurementName, SIGNAL(textEdited(QString)), this, SLOT(nameChanged(QString)));
  connect(_spnMinimum, SIGNAL(editingFinished()), this, SLOT(checkMinMaxValues()));
  connect(_spnMaximum, SIGNAL(editingFinished()), this, SLOT(checkMinMaxValues()));
  connect(_pResolutionCombo, SIGNAL(currentIndexChanged(const QString&)),
          this, SLOT(checkResolution(const QString&)));
  
  connect(toolButton, SIGNAL(clicked()), this, SLOT(openStartColorDialog()));
  connect(toolButton_2, SIGNAL(clicked()), this, SLOT(openEndColorDialog()));
  connect(infoButton, SIGNAL(clicked()), this, SLOT(openInfoDialog()));
  
  _spnMinimum->setKeyboardTracking(false);
  _spnMaximum->setKeyboardTracking(false);
  
  connect(_btnFloatMode, SIGNAL(clicked()), this, SLOT(modeChanged()));
  connect(_btnClassMode, SIGNAL(clicked()), this, SLOT(modeChanged()));
  
  //for class mode
  connect(_btnNewClass, SIGNAL(clicked()), this, SLOT(newClass()));
  connect(_btnDeleteClass, SIGNAL(clicked()), this, SLOT(deleteClass()));
  connect(_btnClassUp, SIGNAL(clicked()), this, SLOT(upClass()));
  connect(_btnClassDown, SIGNAL(clicked()), this, SLOT(downClass()));
  connect(_txtClassName, SIGNAL(textEdited(QString)), this, SLOT(classNameChanged(QString)));
  connect(_pChangeColorButton, SIGNAL(clicked()), this, SLOT(openClassColorDialog()));
  
  _lstClasses->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(_lstClasses, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(classClicked(QListWidgetItem*)));
  connect(_lstClasses, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(openClassColorDialog(QListWidgetItem*)));
  
  setWindowTitle(tr("Edit output values"));
  
  initState();
 
}

void PiiLayerEditor::initState()
{
  if (_dataMap.isEmpty())
    {
      QVariantMap dataMap;
      dataMap["name"] = "Quality";
      dataMap["type"] = "float";
      dataMap["id"] = 0;
      dataMap["disabled"] = false;
      dataMap["min"] = 0.0;
      dataMap["max"] = 1.0;
      dataMap["resolution"] = 0.1;
      dataMap["startColor"] = QColor(QColor(170,255,255));
      dataMap["endColor"] = QColor(Qt::red);
      dataMap["classNames"] = QStringList() << "Unassigned" << "Good" << "Poor";
      dataMap["classColors"] = QList<QVariant>() << QColor(Qt::gray) << QColor(Qt::green) << QColor(Qt::red);
      dataMap["classIndices"] = QList<QVariant>() << 0 << 1 << 2;
      _dataMap = dataMap;
    }
      
  //update max id
  if (_dataMap["id"].toInt() > _iMaxId)
    _iMaxId = _dataMap["id"].toInt();
  
  QList<QVariant> lstClassIndices = _dataMap["classIndices"].toList();
  for (int k=0; k<lstClassIndices.size(); k++)
    if (lstClassIndices[k].toInt() > _iMaxClassIndex)
      _iMaxClassIndex = lstClassIndices[k].toInt();
  
  populateFields(_dataMap);
  QString type = _dataMap["type"].toString();
  if (type == "class")
    {
      _stackedWidget->setCurrentIndex(1);
      _btnClassMode->setChecked(true);
    }
  else
    {
      _stackedWidget->setCurrentIndex(0);
      _btnFloatMode->setChecked(true);
    }

  checkDeleteClassState();
  checkClassUpButtonState();
  checkClassDownButtonState();
}

QString PiiLayerEditor::translateType( const QString string )
{
  if ( string == "float" )
    return _btnFloatMode->text();
  else if ( string == "class" )
    return _btnClassMode->text();
  else
    return QString("");
}

void PiiLayerEditor::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
    {
      if (_txtClassName->hasFocus())
        _lstClasses->setFocus();
      else if (_txtMeasurementName->hasFocus())
        _btnFloatMode->setFocus();
      else
        QWidget::keyPressEvent(e);
    }
  else
    QWidget::keyPressEvent(e);
}

void PiiLayerEditor::openInfoDialog()
{
  QDialog window;
  Ui::TypeInfo ui;
  ui.setupUi(&window);
  window.setModal(true);
  window.setWindowFlags(window.windowFlags() & ~Qt::WindowContextHelpButtonHint);
  window.exec();
}

void PiiLayerEditor::nameChanged(QString name)
{
  _dataMap["name"] = name;
  checkOkButtonState();
}


/******************** FOR CLASS MODE ********************/
void PiiLayerEditor::newClass()
{
  QColor color = _lstInitialColors[_lstClasses->count() % _lstInitialColors.size()];
                                   
  QListWidgetItem *item = new QListWidgetItem;
  int row = _lstClasses->currentRow();
  _lstClasses->insertItem(row+1, item);

  QMap<QString,QVariant> dataMap;
  int i=1;
  while ( true )
    {
      QString name = QString("New class%1").arg(i);
      QList<QListWidgetItem*> tempList = _lstClasses->findItems(name, Qt::MatchContains);
      if ( tempList.isEmpty() )
        {
          item->setText(name);
          break;
        }
      i++;
    }
  int classIndex = _iMaxClassIndex + 1;

  dataMap["color"] = color;
  dataMap["index"] = classIndex;
  item->setData(Qt::UserRole, dataMap);
  item->setData(Qt::DecorationRole,icon(color));

  _lstClasses->setCurrentItem(item);
  populateClassFields(item);
  checkDeleteClassState();
  checkClassUpButtonState();
  checkClassDownButtonState();

  updateClassNames();
  updateClassColors();
  updateClassResolution();
  
  checkOkButtonState();
}

void PiiLayerEditor::deleteClass()
{
  int row = _lstClasses->currentRow();
  if ( row > -1 )
    {
      _lstClasses->takeItem(row);
      int size = _lstClasses->count();
      
      if ( row < size)
        {
          _lstClasses->setCurrentRow(row);
          populateClassFields(_lstClasses->item(row));
        }
      else
        {
          _lstClasses->setCurrentRow(row-1);
          populateClassFields(_lstClasses->item(row-1));
        }
    }
  checkDeleteClassState();
  checkClassUpButtonState();
  checkClassDownButtonState();

  updateClassNames();
  updateClassColors();
  updateClassResolution();
  
  checkOkButtonState();
}

void PiiLayerEditor::checkDeleteClassState()
{
  if ( _lstClasses->count() > 1 )
    _btnDeleteClass->setEnabled(true);
  else
    _btnDeleteClass->setEnabled(false);
}

void PiiLayerEditor::checkClassUpButtonState()
{
  if ( _lstClasses->currentRow() < 1 )
    _btnClassUp->setEnabled(false);
  else
    _btnClassUp->setEnabled(true);
}

void PiiLayerEditor::checkClassDownButtonState()
{
  if ( _lstClasses->currentRow() == (_lstClasses->count()-1) )
    _btnClassDown->setEnabled(false);
  else
    _btnClassDown->setEnabled(true);
}

void PiiLayerEditor::upClass()
{
  int row = _lstClasses->currentRow();
  QString currentName = _lstClasses->item(row)->text();
  QString previousName = _lstClasses->item(row-1)->text();
  QListWidgetItem *current = _lstClasses->item(row);
  QListWidgetItem *previous = _lstClasses->item(row-1);
  
  QListWidgetItem *temp = new QListWidgetItem;
  *temp = *previous;
  *previous = *current;
  *current = *temp;
  
  _lstClasses->setCurrentRow(row-1);
  _lstClasses->item(row-1)->setText(currentName);
  _lstClasses->item(row)->setText(previousName);
  
  populateClassFields(_lstClasses->item(row-1));
  checkClassUpButtonState();
  checkClassDownButtonState();

  updateClassNames();
  updateClassColors();
}

void PiiLayerEditor::downClass()
{
  int row = _lstClasses->currentRow();
  QString currentName = _lstClasses->item(row)->text();
  QString nextName = _lstClasses->item(row+1)->text();
  QListWidgetItem *current = _lstClasses->item(row);
  QListWidgetItem *next = _lstClasses->item(row+1);
  
  QListWidgetItem *temp = new QListWidgetItem;
  *temp = *next;
  *next = *current;
  *current = *temp;
  delete temp;
  
  _lstClasses->setCurrentRow(row+1);
  _lstClasses->item(row+1)->setText(currentName);
  _lstClasses->item(row)->setText(nextName);
  
  populateClassFields(_lstClasses->item(row+1));
  checkClassUpButtonState();
  checkClassDownButtonState();

  updateClassNames();
  updateClassColors();
}

void PiiLayerEditor::classClicked( QListWidgetItem *item )
{
  if ( item != 0 )
    populateClassFields(item);
  checkClassUpButtonState();
  checkClassDownButtonState();
}

void PiiLayerEditor::openClassColorDialog()
{
  openClassColorDialog(_lstClasses->currentItem());
}

void PiiLayerEditor::openClassColorDialog(QListWidgetItem *item)
{
  if (item)
    {
      QMap<QString, QVariant> map = item->data(Qt::UserRole).toMap();

      QColor color = QColorDialog::getColor(map["color"].value<QColor>(), this);
      if ( color.isValid() )
        {
          QMap<QString, QVariant> map = item->data(Qt::UserRole).toMap();
          map["color"] = color;
          item->setData(Qt::UserRole, map);
          item->setData(Qt::DecorationRole, icon(color));
          updateClassColors();
        }
    }
}

void PiiLayerEditor::populateClassFields(const QVariantMap& map)
{
  _lstClasses->clear();
  QStringList classes = map["classNames"].toStringList();
  if ( !classes.isEmpty() )
    _lstClasses->addItems(classes);
  else
    _lstClasses->addItem("New class");

  QList<QVariant> colors = map["classColors"].toList();
  QList<QVariant> indices = map["classIndices"].toList();
  
  for ( int i=0; i<_lstClasses->count(); i++ )
    {
      QMap<QString, QVariant> cmap;
      cmap["color"] = colors.at(i);
      cmap["index"] = indices.at(i);
      
      _lstClasses->item(i)->setData(Qt::UserRole, cmap);
      _lstClasses->item(i)->setData(Qt::DecorationRole, icon(colors.at(i).value<QColor>()));
    }

  populateClassFields(_lstClasses->item(0));
}


void PiiLayerEditor::populateClassFields(QListWidgetItem *item)
{
  QMap<QString, QVariant> map = item->data(Qt::UserRole).toMap();
  _txtClassName->setText(item->text());

  _txtClassName->setFocus();
  _txtClassName->selectAll();
}

void PiiLayerEditor::classNameChanged( QString name )
{
  int row = _lstClasses->currentRow();
  if ( row > -1 )
    {
      _lstClasses->item(row)->setText(name);
      updateClassNames();
    }
}

void PiiLayerEditor::updateClassNames()
{
  //update item map (classNames)
  QStringList classList;
  for ( int i=0; i<_lstClasses->count(); i++)
    classList << _lstClasses->item(i)->text();
  _dataMap["classNames"] = classList;

  checkOkButtonState();
}

void PiiLayerEditor::updateClassColors()
{
  //update item map (classColors)
  QList<QVariant> colorList;
  QList<QVariant> indexList;
  for ( int i=0; i<_lstClasses->count(); i++)
    {
      QMap<QString, QVariant> classMap = _lstClasses->item(i)->data(Qt::UserRole).toMap();
      colorList << classMap["color"].value<QColor>();
      indexList << classMap["index"].toInt();
    }
  _dataMap["classColors"] = colorList;
  _dataMap["classIndices"] = indexList;
}

void PiiLayerEditor::updateClassResolution()
{
  //update item map (min, max, resolution)
  _dataMap["min"] = 0;
  _dataMap["max"] = _lstClasses->count() - 1;
  _dataMap["resolution"] = 1;
}


void PiiLayerEditor::checkOkButtonState()
{
  bool value = true;

  QVariantMap map = _dataMap;
  
  // Check name
  QString name = map["name"].toString();
  if ( name.isEmpty() )
    value = false;
  
  // Check minimum and maximum values
  if (map["type"] == "float" && map.value("min",0.0).toDouble() >= map.value("max",1.0).toDouble() )
    value = false;

  // Check class names
  QStringList classNames = map["classNames"].toStringList();
  for (int k=0; k<classNames.size(); k++)
    {
      //class name is empty
      if (classNames[k].isEmpty())
        value = false;
      else
        {
          //is there duplicate names
          int index = classNames.lastIndexOf(classNames[k]);
          if (index != k)
            value = false;
        }

    }
  
  emit acceptStateChanged(value);
}
 
void PiiLayerEditor::setResolutionComboValue(double value)
{
  int index = 0;
  double dStartValue = 1.0;
  for (int i=1; i<4; i++)
    {
      if (value < dStartValue)
        {
          index = i;
          dStartValue /= 10;
        }
      else
        break;
    }

  _pResolutionCombo->setCurrentIndex(index);
}

void PiiLayerEditor::populateFields(const QVariantMap& map)
{
  _txtMeasurementName->setText(map.value("name", "Quality").toString());
  _spnMinimum->setValue(map.value("min",0.0).toDouble());
  _spnMaximum->setValue(map.value("max",1.0).toDouble());
  setResolutionComboValue(map.value("resolution",0.1).toDouble());

  _startColor = map["startColor"].value<QColor>();
  toolButton->setIcon(icon(_startColor));

  _endColor = map["endColor"].value<QColor>();
  toolButton_2->setIcon(icon(_endColor));

  populateClassFields(map);
  _lstClasses->setCurrentRow(0);
  _txtMeasurementName->setFocus();
  _txtMeasurementName->selectAll();
}

/****************************** FOR FLOAT MODE *******************************/
void PiiLayerEditor::checkMinMaxValues()
{
  double temp = 1 / _dResolution;
  int mintemp = (int)(temp * _spnMinimum->value() + 0.5);
  int maxtemp = (int)(temp * _spnMaximum->value() + 0.5);
  _spnMinimum->setValue(((double)mintemp) / ((double)temp));
  _spnMaximum->setValue(((double)maxtemp) / ((double)temp));
  spinValueChanged("min", _spnMinimum->value());
  spinValueChanged("max", _spnMaximum->value());
  checkOkButtonState();
}  
  
void PiiLayerEditor::checkResolution(const QString& text)
{
  double value = text.toDouble();
  
  _spnMaximum->setSingleStep(value);
  _spnMinimum->setSingleStep(value);

  _dResolution = value;
  checkMinMaxValues();
  
  spinValueChanged("resolution", value);
}

void PiiLayerEditor::openStartColorDialog()
{
  QColor color = QColorDialog::getColor(_startColor, this);
  if ( color.isValid() )
    {
      _startColor = color;
      colorChanged("startColor", color );
      toolButton->setIcon(icon(color));
    }
}

void PiiLayerEditor::openEndColorDialog()
{
  QColor color = QColorDialog::getColor(_endColor, this);
  if ( color.isValid() )
    {
      _endColor = color;
      colorChanged("endColor", color);
      toolButton_2->setIcon(icon(color));

    }
}

void PiiLayerEditor::spinValueChanged(QString name, double value)
{
  _dataMap[name] = value;
}

void PiiLayerEditor::colorChanged(QString name, QColor value)
{
  _dataMap[name] = value;
}


QIcon PiiLayerEditor::icon(const QColor& color) const
{
  QPixmap map(10,10);
  map.fill(color);
  return QIcon(map);
}

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

#include "PiiTableLabelerDelegate.h"
#include "PiiTableLabelerModel.h"
#include "PiiTableLabelerOperation.h" //for PiiMinMax
#include <PiiLayerParser.h>

#include <QLineEdit>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QFocusEvent> // TEST
#include <QApplication> // TEST

#include <QDoubleValidator>

PiiTableLabelerDelegate::PiiTableLabelerDelegate( QObject *parent )
  : QItemDelegate(parent)
{
}

QWidget *PiiTableLabelerDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &index ) const
{
  QVariant itemDataTypeVar = index.model()->data(index, ItemDataTypeRole);

  if ( !itemDataTypeVar.canConvert(QVariant::Int) )
    return 0;

  PiiTableLabelerModel::ItemDataType itemDataType = (PiiTableLabelerModel::ItemDataType)itemDataTypeVar.toInt();


  //Check the item data type and open the data editor specific to the
  //data type stored.
  if (itemDataType == PiiTableLabelerModel::ItemTypeMinMax )
    {
      PiiMinMaxEditor *editor = new PiiMinMaxEditor(parent);
      editor->installEventFilter(const_cast<PiiTableLabelerDelegate*>(this));
      return editor;
    }
  else if (itemDataType == PiiTableLabelerModel::ItemTypeLabel)
    {
      QComboBox *editor = new QComboBox(parent);
      const PiiTableLabelerModel* tlModel = qobject_cast<const PiiTableLabelerModel*>(index.model());
      if (tlModel)
        {      
          QStringList tempList = tlModel->featureProperty(index.column(), "classNames").toStringList();
          tempList.insert(0,"-");
          editor->addItems(tempList);
          editor->installEventFilter(const_cast<PiiTableLabelerDelegate*>(this));
        }
      return editor;
    }
  else if (itemDataType == PiiTableLabelerModel::ItemTypeFloatLabel )
    {
      QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
      const PiiTableLabelerModel* tlModel = qobject_cast<const PiiTableLabelerModel*>(index.model());
      if (tlModel)
        {      
          QStringList tempList;
          QVariant layer = tlModel->layer();
          editor->setRange(PiiLayerParser::minValue(layer),
                           PiiLayerParser::maxValue(layer));
          double resolution = PiiLayerParser::resolution(layer);
          editor->setSingleStep(resolution);
          //Calculate the amount of decimals show in the double spin box:
          int decimals = qMax(0,(int)ceil(log10(1/(resolution-floor(resolution)))));
          editor->setDecimals(decimals);
          editor->installEventFilter(const_cast<PiiTableLabelerDelegate*>(this));
        }      
      return editor;
    }
  else if (itemDataType == PiiTableLabelerModel::ItemTypeClassLabel )
    {
      QComboBox *editor = new QComboBox(parent);
      const PiiTableLabelerModel* tlModel = qobject_cast<const PiiTableLabelerModel*>(index.model());
      if (tlModel)
        {      
          QStringList tempList;
          tempList = PiiLayerParser::classNames(tlModel->layer());
          editor->addItems(tempList);
          editor->installEventFilter(const_cast<PiiTableLabelerDelegate*>(this));
        }
      return editor;
    }  
  else
    return 0;
  
}

void PiiTableLabelerDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
  
  PiiMinMaxEditor *minMaxEditor = qobject_cast<PiiMinMaxEditor*>(editor);
  if (minMaxEditor)
    {
      minMaxEditor->setValues( PiiMinMax(index.model()->data(index, Qt::EditRole)) );
      return;
    }

  QDoubleSpinBox *floatEditor = qobject_cast<QDoubleSpinBox*>(editor);
  if (floatEditor)
    {
      //Set the currently visible value of the spin box. 
      QVariant data = index.model()->data(index, Qt::EditRole);
      if (data.canConvert(QVariant::Double))
        floatEditor->setValue ( data.toDouble() );      
      return;
    }
  
  QComboBox *classEditor = qobject_cast<QComboBox*>(editor);
  if (classEditor)
    {
      //Select the visible item. 
      QVariant data = index.model()->data(index, Qt::EditRole);
      if (data.canConvert(QVariant::Int))
        {
          classEditor->setCurrentIndex(data.toInt());
        }
      return;
    }
        
}

void PiiTableLabelerDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  QAbstractItemDelegate::setModelData(editor, model, index);
  PiiMinMaxEditor *minMaxEditor = qobject_cast<PiiMinMaxEditor*>(editor);
  if (minMaxEditor)
    {
      // Check first, if either min or max value has really been
      // changed. If yes, the model is updated.
      if (PiiMinMax(model->data(index, Qt::EditRole)) != minMaxEditor->values())
        {
          model->setData(index, minMaxEditor->values().toVariant(), Qt::EditRole);
        }
      else
      return;
    }

  QDoubleSpinBox *floatEditor = qobject_cast<QDoubleSpinBox*>(editor);
  if (floatEditor)
    {
      // Check first, if the float value has really been changed. If
      // yes, the model is updated.
      if (model->data(index, Qt::EditRole).toDouble() != floatEditor->value())
        model->setData(index, floatEditor->value(), Qt::EditRole);
      return;        
    }
  
  QComboBox *classEditor = qobject_cast<QComboBox*>(editor);
  if (classEditor)
    {
      // Check first, if the float value has really been changed. If
      // yes, the model is updated.
      if (model->data(index, Qt::EditRole).toInt() != classEditor->currentIndex())
        model->setData(index, classEditor->currentIndex(), Qt::EditRole);
      return;        
    }
}

void PiiTableLabelerDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
  editor->setGeometry(option.rect);
}

////// PiiMinMaxEditor class implementation //////////

PiiMinMaxEditor::PiiMinMaxEditor( QWidget *parent )
  : QWidget(parent)
{
  _pMinEdit = new QLineEdit(this);
  _pMaxEdit = new QLineEdit(this);
  _pLabel = new QLabel(this); //for showing the dash
  _pLabel->setText("-");

  
  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(_pMinEdit);
  layout->addWidget(_pLabel);
  layout->addWidget(_pMaxEdit);

  _pMinEdit->setMaxLength(5);
  _pMinEdit->setMaximumWidth(50);

  _pMaxEdit->setMaxLength(5);
  _pMaxEdit->setMaximumWidth(50);

  QDoubleValidator* minValidator = new QDoubleValidator(this);
  QDoubleValidator* maxValidator = new QDoubleValidator(this);

  _pMinEdit->setValidator(minValidator);
  _pMaxEdit->setValidator(maxValidator);

  _pMinEdit->installEventFilter(this);
  _pMaxEdit->installEventFilter(this);

  layout->setMargin(2);

  
  setLayout(layout);

  // This prevents the old values shining through the editing widget.
  setAutoFillBackground(true);
  setFocusPolicy(Qt::StrongFocus);
//   setFocusPolicy ( Qt::ClickFocus );
  _pMinEdit->setFocus(Qt::MouseFocusReason);
  
}

void PiiMinMaxEditor::setValues(const PiiMinMax& minMax)
{
  _pMinEdit->setText(minMax.minAsString());
  _pMaxEdit->setText(minMax.maxAsString());
}

PiiMinMax PiiMinMaxEditor::values() 
{
  return PiiMinMax(_pMinEdit->text(), _pMaxEdit->text()).toVariant();
}

bool PiiMinMaxEditor::eventFilter(QObject * watched, QEvent * event)  
{
  // If either of the QLineEdits (_pMinEdit or _pMaxEidt) loses the
  // focus, their parent (=this) is also set to lost focus state. This
  // must be manually, others the status of this widget won't change.
  if (((watched == _pMinEdit) || (watched == _pMaxEdit)) && (event->type() == QEvent::FocusOut))
    {
      QFocusEvent event(QEvent::FocusOut, Qt::OtherFocusReason);
      QApplication::sendEvent(this, &event);
    }
  return false;
}

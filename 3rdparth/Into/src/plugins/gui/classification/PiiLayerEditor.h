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

#ifndef _PIILAYEREDITOR_H
#define _PIILAYEREDITOR_H

#include <QWidget>
#include "ui_layer_editor_stacked.h"

#include <QKeyEvent>

/**
 * An user interface component for editing "layers" in visual trainer.
 *
 * @ingroup PiiVisualizationPlugin
 */
class PiiLayerEditor : public QWidget, private Ui_LayerEditor
{
  Q_OBJECT
  
public:
  PiiLayerEditor(const QList<QVariant>& layerList, QWidget *parent = 0);
  PiiLayerEditor(QWidget *parent = 0);
  ~PiiLayerEditor();

  QList<QVariant> layerList() const;
  void setLayerList(const QList<QVariant>& layerList);

protected:
  void keyPressEvent(QKeyEvent *e);
  
public slots:
  void setVisible(bool visible);
  
private slots:
  void openInfoDialog();
  void nameChanged(QString name);
  void modeChanged();
  void openStartColorDialog();
  void openEndColorDialog();
  
  void newClass();
  void deleteClass();
  void upClass();
  void downClass();
  void classClicked( QListWidgetItem *item );
  void classNameChanged(QString name);
  void openClassColorDialog();
  void openClassColorDialog(QListWidgetItem *item);

  void checkMinMaxValues();
  void checkResolution(const QString& text);
  
signals:
  void updateLayers(QList<QVariant> layerList);
  void acceptStateChanged(bool);
  
private:
  QString translateType( const QString string );
  void init();
  void initState();

  void checkDeleteClassState();
  void checkClassUpButtonState();
  void checkClassDownButtonState();
  void populateClassFields(const QVariantMap& map);
  void populateClassFields(QListWidgetItem *item);
  void updateClassNames();
  void updateClassColors();
  void updateClassResolution();
  
  void checkOkButtonState();
  void setResolutionComboValue(double value);
  void populateFields(const QVariantMap& map);
  void spinValueChanged(QString name, double value);
  void colorChanged(QString name, QColor value);

  QIcon icon(const QColor& color) const;

  QVariantMap _dataMap;
  QColor _classColor, _startColor, _endColor;
  double _dResolution;
  int _iMaxId;
  int _iMaxClassIndex;
  QList<QColor> _lstInitialColors;
};


#endif //_PIILAYEREDITOR_H

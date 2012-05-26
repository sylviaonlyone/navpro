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

#ifndef _PIITABLELABELERDELEGATE_H
#define _PIITABLELABELERDELEGATE_H

#include <QWidget>
#include <QItemDelegate>

class QLineEdit; //for PiiMinMaxEditor
class QLabel; //for PiiMinMaxEditor
class PiiMinMax;

class PiiMinMaxEditor : public QWidget
{
  Q_OBJECT

public:
  PiiMinMaxEditor(QWidget *parent = 0);

  /**
   * Fills the line edit fields with the minimum and maximum values.
   * The parameter @p minMaxValues is assumed to contain a
   * QMap<QString, QVariant> data structure, where the keys are "min"
   * and "max" and the values doubles converted to QVariant. The
   * infinity values are shown with an empty string in the line edits.
   */
  void setValues(const PiiMinMax& minMax);

  /**
   * Returns the contents of the line edit fields. They are returned
   * in the QVariant. The content of the QVariant returned corresponds
   * to the parameter minMaxValues in the function setValues(). The
   * empty string in the minumum value line edit field is converted to double
   * value -INFINITY and and the empty string in the maximum value
   * field to double value INFINITY.
   */  
  PiiMinMax values();

  bool eventFilter ( QObject * watched, QEvent * event ); //TEST

protected:
  // TEST >>
  /**
   * This re-implements the corresponding function in the base class.
   * In the function, the tab key presses are checked.
   */
//   bool event ( QEvent * event );
// TEST <<
  
  // TEST >>
  /**
   * This function changes the focus state of the widget.
   */
  //void changeFocus(bool forward);
  // TEST
  
private:
  double _dMin, _dMax;

  QLineEdit *_pMinEdit, *_pMaxEdit;
  QLabel *_pLabel;

};

/**
 * TODO: COMPLEMENT THIS COMMENT BLOCK
 */ 
class PiiTableLabelerDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  PiiTableLabelerDelegate(QObject *parent);

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;

  void setEditorData(QWidget *editor, const QModelIndex &index) const;
  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const;

  void updateEditorGeometry(QWidget *editor,
                            const QStyleOptionViewItem &option, const QModelIndex &index) const;

  // REMOVE >>
//   bool editorEvent ( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index ) ;
  // REMOVE <<

  //bool eventFilter ( QObject * watched, QEvent * event ) ;
};

#endif //_PIITABLELABELERDELEGATE_H

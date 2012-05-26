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

#ifndef _PIITABLEMODEL_H
#define _PIITABLEMODEL_H

#include "PiiGui.h"

#include <QAbstractTableModel>
#include <QAbstractItemView>
#include <QList>

class PiiTableModelDelegate;
class PiiModelItem;

/**
 * A hybrid of a table model and an item "delegate". This model can be
 * used with Qt's view classes to create editable lists and tables. 
 * %PiiTableModel is useful with data such as database query results
 * where all items in a table column contain similar data. The same
 * editor is used for each row, but columns may have different
 * editors.
 *
 * This class uses PiiModelItem to store item and header data. Each
 * item in the model has two associated values with different data
 * roles:
 *
 * @lip Qt::DisplayRole - the text displayed in the table cell when
 * the editor is not active.
 *
 * @lip ColumnEditorValueRole - the actual value of the cell, which
 * may be different from the text. For example, the value of an
 * element in a combo box column is the index of the selected item,
 * but the cell displays the text associated with the index. See
 * #EditorType for data types associated with editors.
 *
 * @ingroup Gui
 */
class PII_GUI_EXPORT PiiTableModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /**
   * Custom data roles used by %PiiTableModel to store information
   * about column editors and their values.
   *
   * @lip ColumnDefaultValueRole - the default value for a cell in a
   * column. Any variant. Used only in header items.
   *
   * @lip ColumnEditorTypeRole - the type of the column editor. An
   * integer. See #EditorType for valid values. Used only in header
   * items.
   *
   * @lip ColumnEditorPropertiesRole - a QVariantMap that can be used
   * to customize the column editor's properties. Used only in header
   * items.
   *
   * @lip ColumnEditorValueRole - the current value of the item, in
   * the editor's internal representation. Any variant type. Used only
   * in table data items.
   */
  enum ColumnEditorDataRole
    {
      ColumnDefaultValueRole = Qt::UserRole + 1,
      ColumnEditorTypeRole,
      ColumnEditorPropertiesRole,
      ColumnEditorValueRole
    };
  
  /**
   * Supported editor types.
   *
   * @lip LineEditor - a QLineEdit. Suitable for text input with or
   * without validation. The value and text are the same. To configure
   * a validator for the editor, use the @p validator property
   * (QVariantMap).
   *
   * @lip IntegerSpinBoxEditor - a QSpinBox. Suitable for integer
   * input. The value is an integer. If the spin box has the @p prefix
   * a and @p suffix properties set, the text will be different from
   * the value.
   *
   * @lip DoubleSpinBoxEditor - a QDoubleSpinBox. Suitable for
   * double-precision value input. The value is a double. If the spin
   * box has the @p prefix a and @p suffix properties set, the text
   * will be different from the value.
   *
   * @lip ComboBoxEditor - a QComboBox. Suitable for multiple choices
   * type input. The value is an integer that stores the index of the
   * selected choice. The text is the corresponding element in the
   * combo box. To set the selectable items, use the @p items property
   * (StringList).
   *
   * @see #setColumnEditorType()
   * @see #setColumnEditorProperties()
   */
  enum EditorType
    {
      LineEditor,
      IntegerSpinBoxEditor,
      DoubleSpinBoxEditor,
      ComboBoxEditor
    };

  /**
   * A flag for #setValue() that can be used to enable/disable
   * automatic setting of the displayed text of the element.
   */
  enum ValueChangeBehavior
    {
      ChangeTextAutomatically,
      DoNotChangeText
    };

  /**
   * Creates a new %PiiTableModel and binds it to the @p parent view. 
   * Since %PiiTableModel is not a pure model, but works also as the
   * "delegate", it cannot be used with many views simultaneously.
   */
  PiiTableModel(QAbstractItemView *parent);

  /**
   * Destroys the model.
   */
  ~PiiTableModel();

  /**
   * Clears all data, excluding column header definitions.
   */
  void clear();

  /**
   * Returns the item in the given cell. If there is no such cell, 0
   * will be returned.
   */
  PiiModelItem* itemAt(int row, int column = 0) const;
  /**
   * Returns the data associated with @p role in the given cell. If
   * there is no such cell or no such data, an invalid variant will be
   * returned.
   */
  QVariant data(int row, int column, int role = Qt::DisplayRole) const;
  /**
   * Sets the data associated with @p role in the given cell.
   */
  void setData(int row, int column, const QVariant& value, int role);
  /**
   * Returns the displayed text in the given cell.
   */
  inline QString text(int row, int column) const { return data(row, column).toString(); }
  /**
   * Sets the displayed text in the given cell.
   */
  inline void setText(int row, int column, const QString& text) { setData(row, column, text, Qt::DisplayRole); }
  /**
   * Returns the data associated with @p ColumnEditorValueRole in the
   * given cell.
   */
  inline QVariant value(int row, int column) const { return data(row, column, ColumnEditorValueRole); }

  /**
   * Returns all data associated with @p ColumnEditorValueRole in the
   * given @p column as a variant list.
   */
  QVariantList columnValues(int column) const;
  /**
   * Sets all data associated with @p ColumnEditorValueRole in the
   * given @p column as a variant list. The number of rows in the
   * model will be adjusted to match the length of @p values.
   */
  void setColumnValues(int column, const QVariantList& values);

  /**
   * Returns all data associated with @p Qt::DisplayRole in the given
   * @p column as a string list.
   */
  QStringList columnTexts(int column) const;
  /**
   * Returns all data associated with @p Qt::DisplayRole in the given
   * @p column as a string list. The number of rows in the model will
   * be adjusted to match the length of @p texts.
   */
  void setColumnTexts(int column, const QStringList& texts);
  
  /**
   * Sets the value of the element at the given coordinates. The value
   * may be different from the displayed text. If @p behavior is @p
   * ChangeTextAutomatically, the text will be automatically set based
   * on column configuration. For example, setting the value of a
   * combo box column automatically changes the text based on the
   * combo's item list. To disable the default behavior, set @p
   * behavior @p DoNotChangeText.
   */
  void setValue(int row, int column, const QVariant& value, ValueChangeBehavior behavior = ChangeTextAutomatically);
  /**
   * Converts @p value into user-displayable text. The default
   * implementation returns different text depending on the column
   * editor.
   *
   * @li LineEditor - the value will be converted to a QString
   *
   * @li {Integer,Double}SpinBoxEditor - the value will be converted
   * to a QString and wrapped into the editor's @p prefix and @p
   * suffix, if set. The @p specialValueText property of the spin box
   * will be taken into account.
   *
   * @li ComboBoxEditor - the value is converted to an int, and the
   * corresponding text in the combo's item list will be returned.
   */
  virtual QString textForValue(int column, const QVariant& value) const;

  /**
   * Inserts the given items as a new row to the model.
   *
   * @param items a list of items. The length of this list must equal
   * the number of columns in the model. The model takes the ownership
   * of the pointers.
   *
   * @param row insert the row after this index. -1 means last.
   */
  void insertRow(const QList<PiiModelItem*>& items, int row = -1);
  /**
   * Removes @p row and returns its items as a list. The model no
   * longer owns the pointers, and they must be deleted by the caller.
   */
  QList<PiiModelItem*> takeRow(int row);

  /**
   * Returns the number of rows in the model.
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  /**
   * Returns the number of columns in the model.
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  bool setHeaderData(int section, Qt::Orientation, const QVariant& value, int role);

  /**
   * Sets the default @p value for cells in @p column.
   */
  inline void setDefaultValue(int column, const QVariant& value)
  {
    setHeaderData(column, Qt::Horizontal, value, ColumnDefaultValueRole);
  }
  /**
   * Returns the default value for cells in @p column.
   */
  inline QVariant defaultValue(int column) const
  {
    return headerData(column, Qt::Horizontal, ColumnDefaultValueRole);
  }
  /**
   * Sets the editor used to edit items in the specified column.
   */
  inline void setColumnEditorType(int column, EditorType type)
  {
    setHeaderData(column, Qt::Horizontal, static_cast<int>(type), ColumnEditorTypeRole);
  }
  /**
   * Returns the editor used to edit items in the specified column.
   */
  inline EditorType columnEditorType(int column) const
  {
    return static_cast<EditorType>(headerData(column, Qt::Horizontal, ColumnEditorTypeRole).toInt());
  }
  /**
   * Configures the column editor in the specified column. Except for
   * a few special properties recognized by %PiiTableModel, the given
   * properties will be directly transferred to the editor widget.
   *
   * @code
   * pModel->setHeaderTitles(QStringList() << "Combo" << "IntSpin"
   *                                       << "DoubleSpin" << "Text");
   * // Combo box for column 0
   * pModel->setColumnEditorType(0, PiiTableModel::ComboBoxEditor);
   * QVariantMap map;
   * map["items"] = QStringList() << tr("Jaa") << tr("Ei");
   * // Create icons automatically from the given colors
   * map["colors"] = QVariantList() << QColor(Qt::green) << QColor(Qt::red);
   * // Use custom icons
   * map["icons"] = QVariantList() << QIcon(":yes.svg") << QIcon(":no.svg");
   * pModel->setColumnEditorProperties(0, map);
   *
   * pModel->setColumnEditorType(1, PiiTableModel::IntegerSpinBoxEditor);
   * map.clear();
   * map["minimum"] = 2;
   * map["singleStep"] = 2;
   * map["maximum"] = 24;
   * pModel->setColumnEditorProperties(1, map);
   *
   * pModel->setColumnEditorType(2, PiiTableModel::DoubleSpinBoxEditor);
   * map.clear();
   * map["minimum"] = 1.0;
   * map["maximum"] = 100.0;
   * pModel->setColumnEditorProperties(2, map);
   *
   * pModel->setColumnEditorType(3, PiiTableModel::LineEditor);
   * map.clear();
   * QVariantMap mapValidator;
   * mapValidator["regexp"] = "[1-9][0-9]{3}-[0-9]{2}-[0-9]{2}";
   * map["validator"] = mapValidator;
   * pModel->setColumnEditorProperties(3, map);
   * @endcode
   *
   * @note @p LineEditor supports regexp and numeric validators. If
   * the editor properties contains a QVariantMap named "validator",
   * its properties will be used to configure a validator. The
   * validator's type is auto-detected based on the properties. If
   * "regexp" is present, a QRegExpValidator will be used. If "top"
   * and "bottom" are present, a QIntValidator or QDoubleValidator
   * will be used, based on the type (int or double) of the limits.
   */
  inline void setColumnEditorProperties(int column, const QVariantMap& properties)
  {
    setHeaderData(column, Qt::Horizontal, properties, ColumnEditorPropertiesRole);
  }
  /**
   * Returns the properties of the editor at @p column. This function
   * returns the properties set with #setColumnEditorProperties()
   * only. No other properties will be returned.
   */
  inline QVariantMap columnEditorProperties(int column) const
  {
    return headerData(column, Qt::Horizontal, ColumnEditorPropertiesRole).toMap();
  }

  /**
   * Sets the column header titles with one function call.
   *
   * @param headerTitles the titles. The length of this list must
   * equal the number of columns.
   */
  void setHeaderTitles(const QStringList& headerTitles);

  /**
   * Returns data associated with any role at the given model index. 
   * See @ref PiiModelItem::_d().
   */
  QMap<int, QVariant> itemData(const QModelIndex &index) const;

  bool insertRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());
  bool removeRows(int row, int count = 1, const QModelIndex &parent = QModelIndex());

  bool insertColumns(int column, int count = 1, const QModelIndex &parent = QModelIndex());
  bool removeColumns(int column, int count = 1, const QModelIndex &parent = QModelIndex());

  Qt::ItemFlags flags(const QModelIndex &index) const;

  /**
   * Returns the indices of all rows in which at least one cell is
   * selected. The list is sorted in ascending order.
   */
  QList<int> selectedRows() const;

  /**
   * Enables/disables deletion of the last row in the model. If this
   * flag is set to @p false, #canDelete() will return @p false if
   * there is only one row left.
   */
  void setCanDeleteLast(bool canDeleteLast);
  /**
   * Returns @p true if the last row can be deleted, @p false
   * otherwise.
   */
  bool canDeleteLast() const;  
  
  /**
   * Returns @p true if the selected rows can be moved up, @p false
   * otherwise. Moving rows up is possible if a) at least one row is
   * selected and b) the first row is not selected.
   */
  bool canMoveUp() const;
  /**
   * Returns @p true if the selected rows can be moved down, @p false
   * otherwise. Moving rows down is possible if a) at least one row is
   * selected and b) the last row is not selected.
   */
  bool canMoveDown() const;
  /**
   * Returns @p true if the selected rows can be deleted, @p false
   * otherwise. Deleting rows down is possible if a) at least one row
   * is selected and b) either the #canDeleteLast() flag is @p true or
   * there are more than one row left.
   */
  bool canDelete() const;

signals:
  /**
   * Signals that selection has changed on the attached view.
   */
  void selectionChanged();
  /**
   * Signals that the current item has changed on the attached view.
   */
  void currentItemChanged();
  /**
   * Emitted whenever the selection has changed. If the @p enabled
   * flag is @p true, the selected rows can be moved up.
   */
  void moveUpEnabled(bool enabled);
  /**
   * Emitted whenever the selection has changed. If the @p enabled
   * flag is @p true, the selected rows can be moved down.
   */
  void moveDownEnabled(bool enabled);
  /**
   * Emitted whenever the selection has changed. If the @p enabled
   * flag is @p true, there are selected rows.
   */
  void deleteEnabled(bool enabled);

public slots:
  /**
   * Inserts a new row at the end of the table and fill with it
   * default values.
   */
  void addRow();
  /**
   * Deletes selected rows.
   */
  void deleteSelectedRows();
  /**
   * Moves selected rows one step up. Does nothing if there are now
   * selected rows or the first row is selected.
   */
  void moveSelectedRowsUp();
  /**
   * Moves selected rows down step up. Does nothing if there are now
   * selected rows or the last row is selected.
   */
  void moveSelectedRowsDown();

private slots:
  void enableControls();

protected:
  /// @internal
  struct Data
  {
    Data(PiiTableModel* model);
    ~Data();

    int rows() const { return _lstItems.size()-1; }
    int columns() const { return _lstItems[0].size(); }
    
    PiiTableModelDelegate* _pDelegate;
    QList<QList<PiiModelItem*> > _lstItems;
    bool _bCanDeleteLast;
  } *d;
  
  /**
   * Creates a new item at the given coordinates. The default
   * implementation sets the item's value (@p ColumnEditorValueRole)
   * to the default value of the column, if given.
   */
  virtual PiiModelItem* createItem(int row, int column);
  /**
   * Creates an editor for editing the item at the given coordinates. 
   * The default implementation creates one of the supported editor
   * types, based on header data. If you override this function, you
   * probably need to override #setEditorData(), #setModelData(), and
   * #textForValue() as well.
   */
  virtual QWidget* createEditor(QWidget* parent, int row, int column) const;
  /**
   * Sets up the contents of @p editor based on the contents of the
   * cell at the given position.
   */
  virtual void setEditorData(QWidget* editor, int row, int column) const;
  /**
   * Modifies the cell at the given position based on the contents of
   * @p editor.
   */
  virtual void setModelData(QWidget* editor, int row, int column);

  /// @internal
  PiiTableModel(Data* data, QAbstractItemView *parent);
  
private:
  friend class PiiTableModelDelegate;

  QList<PiiModelItem*> createRow(int row = -1);
  inline void selectRows(const QList<int>& rows);
  void selectRow(int row);
  inline QItemSelectionModel* selectionModel() const { return static_cast<QAbstractItemView*>(QObject::parent())->selectionModel(); }
  void initialize(QAbstractItemView* parent);
};


#endif //_PIITABLEMODEL_H

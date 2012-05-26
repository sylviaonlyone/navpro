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

#ifndef _PIITABLELABELERCONFIGURATOR_H
#define _PIITABLELABELERCONFIGURATOR_H


#include <PiiMatrix.h>
#include <PiiLayerParser.h>

#include <QtGui>

class QStringList;
class PiiTableLabelerOperation;
class PiiTableLabelerModel;
class PiiTableLabelerModelContainer;

/**
 * User interface class, which is designed to be used with
 * PiiTableLabelerConfigurator. Provides tool buttons for adding and
 * removing the rows and for moving a row upwards and downwards. The
 * signals of the button must be connected to the corresponding slots
 * in PiiTableLabelerConfigurator.   
 */
class PiiTableLabelerToolBar : public QWidget
{
  Q_OBJECT

public:
  
  PiiTableLabelerToolBar(QWidget* parent = 0);

  /**
   * Returns instance to the Add Row button. This function is needed
   * for example when connecting the signals/slots from/to the button.
   */
  QPushButton* buttonAddRow() {return _pAddButton;}
  /**
   * Returns instance to the Remove Row button. This function is needed
   * for example when connecting the signals/slots from/to the button.
   */
  QPushButton* buttonRemoveRow() {return _pRemoveButton;}  
  /**
   * Returns instance to the Move Row Up button. This function is needed
   * for example when connecting the signals/slots from/to the button.
   */
  QPushButton* buttonMoveRowUp() {return _pMoveUpButton;}
  /**
   * Returns instance to the Move Row Down button. This function is needed
   * for example when connecting the signals/slots from/to the button.
   */
  QPushButton* buttonMoveRowDown() {return _pMoveDownButton;}
  /**
   * Returns instance to the Change rule type button. This function is needed
   * for example when connecting the signals/slots from/to the button.
   */
  QPushButton* buttonChangeRuleType() {return _pChangeRuleButton;}
  
  /**
   * This function returns the box layout of the widget.
   */
  //QBoxLayout* boxLayout()
  //{return qobject_cast<QBoxLayout*>(layout());}

  /**
   * This function adds the strecth to right had side of the toolbar.
   */
  //void addStretch();
  
 
private:
  QPushButton *_pAddButton, *_pRemoveButton, *_pMoveUpButton, *_pMoveDownButton, *_pChangeRuleButton;

  void createButtons();
  QPushButton* createButton();

};

/*
 * This class has been created in order to make the protected function
 * of QTabWidget tabBar() visible to the classes, that don't inherit
 * from QTabWidget.
 */
class PiiTableLabelerTabWidget : public QTabWidget
{
  Q_OBJECT

public:
  
  PiiTableLabelerTabWidget(QWidget* parent = 0) : QTabWidget(parent)
  {}

  using QTabWidget::tabBar;
};


#include <PiiConfigurationWidget.h>

/**
 * Complement this comment block.
 */
class PiiTableLabelerConfigurator : public PiiConfigurationWidget
{
  Q_OBJECT

public:

  /**
   * Constructor.
   */
  PiiTableLabelerConfigurator(QWidget* parent = 0);

  /**
   * Destructor.
   */
  ~PiiTableLabelerConfigurator();

  /**
   * Gets the Model Container associated with the configurator.
   */
  PiiTableLabelerModelContainer* modelContainer() {return _pModelContainer; }
  
  /**
   * This function implements the corresponding function inherited
   * from PiiConfigurationWidget. Returns the return value of
   * hasChanged() function.
   */
  bool 	canReset () const;

  bool hasChanged() const;
  
  /**
   * This function implements the corresponding function inherited
   * from PiiConfigurationWidget. Returns always @p false.
   */
  bool 	hasDefaults () const;
 	
  /**
   * This function informs the configurator that a new
   * ui controller (e.g. toolbar or pop menu) has been created and
   * connected to the configurator. This function should be called
   * allways after that kind of situation. 
   */
  void controllerAdded();
  
public slots:
  // PENDING: Implement corresponding removeTab slot.  
  /**
   * Adds a new tab and a new Table View associated to the @p model
   * given as a parameter.
   */
   void addTab(PiiTableLabelerModel* model, const QString& name);
    
  /**
   * This function implements the corresponding function inherited
   * from PiiConfigurationWidget.
   * PENDIND: COMPLEMENT THIS COMMENT BLOCK
   * Accept the UI component.
   */
  void acceptChanges ();


  /**
   * This function implements the corresponding function inherited
   * from PiiConfigurationWidget.
   * PENDIND: COMPLEMENT THIS COMMENT BLOCK
   * Reset the component to the state it was upon creation or after the last acceptChanges() call.
   */  
  void reset();

  /**
   * This function implements the corresponding function inherited
   * from PiiConfigurationWidget.
   * PENDIND: COMPLEMENT THIS COMMENT BLOCK
   * Set the default values.
   */  
  void setDefaults();

  /**
   * This slot adds a new row just below the currently selected row
   * in the visible tab.
   */  
  void addRow();

  /**
   * This slot removes the currently selected row from the visible tab.
   */  
  void removeRow();

  /**
   * This slot moves the currently selected row upward in the visible
   * tab, provided that it can be moved upward.
   */  
  void moveRowUp();

  /**
   * This slot moves the currently selected row downward in the visible
   * tab, provided that it can be moved downward.
   */  
  void moveRowDown();

  void changeRuleType();

  /**
   * This slot tells that the changed status should be changed. In
   * effect it calls the steChanged() function of the class.
   */
  void setChangedState(bool status);
  
signals:
  /* PENDING
   * Add the comment blocks in the following signals.
   */
  void addingRowsEnabled(bool status);  
  void removingRowsEnabled(bool status);  
  void movingRowsUpEnabled(bool status);
  void movingRowsDownEnabled(bool status);
  void changeRuleTypesEnabled(bool status);

protected:
  void contextMenuEvent(QContextMenuEvent *event);  
  
private:
  void createPopupMenu();
  QList<int> indivSelectedRows(QTableView* currView) const;
  QList< QPair<int, int> > selectedRows(QTableView* currView) const;
  void selectRows(QAbstractItemView* view, int row, int amount);
  void updateControllers();
  void setContentsChanged(bool status);
  
  PiiTableLabelerTabWidget* _pTabWidget;
  PiiTableLabelerModelContainer* _pModelContainer;

  /* PENDING
   * This popup menu and its actions should maybe by moved to the view
   * class!!
   */
  QMenu *_pPopMenu;     //Popup menu

private slots:
  void selectionChanged(const QItemSelection&, const QItemSelection& );
  void currentTabChanged(int index);
};


#endif //_PIITABLELABELERCONFIGURATOR_H

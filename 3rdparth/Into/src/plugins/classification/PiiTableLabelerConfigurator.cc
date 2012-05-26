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

#include "PiiTableLabelerConfigurator.h"
#include "PiiTableLabelerModelContainer.h"
#include "PiiTableLabelerModel.h"
#include "PiiTableLabelerDelegate.h"

#include <QStringList>
#include <QtDebug>
#include <QTableView>
#include <QModelIndex>

PiiTableLabelerConfigurator::PiiTableLabelerConfigurator(QWidget* parent)
  : PiiConfigurationWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  _pTabWidget = new PiiTableLabelerTabWidget(this);
  
  layout->addWidget(_pTabWidget);

  connect(_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

  _pModelContainer = new PiiTableLabelerModelContainer();
  connect(_pModelContainer, SIGNAL(modelCreated(PiiTableLabelerModel*, const QString&)  ), this, SLOT(addTab(PiiTableLabelerModel*, const QString&)));
  
  //Hide the tab bar, because the amount of tabs is still less than two.
  _pTabWidget->tabBar()->setVisible(false); 
  
  createPopupMenu();

  connect(_pModelContainer,SIGNAL(changedStatusChanged(bool)),this,SLOT(setChangedState(bool)));
}

PiiTableLabelerConfigurator::~PiiTableLabelerConfigurator()
{
  delete _pModelContainer;
}

void PiiTableLabelerConfigurator::addTab(PiiTableLabelerModel* model, const QString& name)
{
  QTableView* view = new QTableView( this);

  QHeaderView *horHeader = view->horizontalHeader();
  horHeader->setResizeMode(QHeaderView::ResizeToContents);
  horHeader->setMinimumSectionSize(60);
  view->setHorizontalHeader(horHeader);
  
  view->setModel(model);
  PiiTableLabelerDelegate* delegate = new PiiTableLabelerDelegate(this);
  view->setItemDelegate(delegate);
  view->setEditTriggers(QAbstractItemView::AllEditTriggers);
  connect(view->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),this,SLOT(selectionChanged(const QItemSelection&, const QItemSelection& )));    
      
  _pTabWidget->addTab ( view, name );
  
  //Show the tab bar if the amount of tabs is creater than one.
  if (_pTabWidget->count() > 1)
    _pTabWidget->tabBar()->setVisible(true);
}

/* This private function creates the popmenu
 */
void PiiTableLabelerConfigurator::createPopupMenu()
{
  _pPopMenu = new QMenu(this);

  QAction *pInsertRowAct = _pPopMenu->addAction(tr("Add new rule"), this, SLOT(addRow()));
  pInsertRowAct->setToolTip(tr("Add as many new rules as there are rows selected"));

  QAction *pRemoveRowAct = _pPopMenu->addAction(tr("Remove rules"), this, SLOT(removeRow()));
  pRemoveRowAct->setToolTip(tr("Remove selected rules"));

  _pPopMenu->addSeparator();

  QAction *pMoveRowUpAct = _pPopMenu->addAction(tr("Move up"), this, SLOT(moveRowUp()));
  pMoveRowUpAct->setToolTip(tr("Move selected rules up"));

  QAction *pMoveRowDownAct = _pPopMenu->addAction(tr("Move down"), this, SLOT(moveRowDown()));
  pMoveRowDownAct->setToolTip(tr("Move selected rules down"));
  
  _pPopMenu->addSeparator();

  QAction *pChangeRuleAct= _pPopMenu->addAction(tr("Switch rule type"), this, SLOT(changeRuleType()));
  pChangeRuleAct->setToolTip(tr("Change a rule of the current row"));
  
  connect(this,SIGNAL(addingRowsEnabled(bool)),pInsertRowAct,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(removingRowsEnabled(bool)),pRemoveRowAct,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(movingRowsUpEnabled(bool)),pMoveRowUpAct,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(movingRowsDownEnabled(bool)),pMoveRowDownAct,SLOT(setEnabled(bool)));
  connect(this,SIGNAL(changeRuleTypesEnabled(bool)),pChangeRuleAct,SLOT(setEnabled(bool)));
  
}

void PiiTableLabelerConfigurator::contextMenuEvent(QContextMenuEvent *event)
{
  _pPopMenu->exec(event->globalPos());
}

bool PiiTableLabelerConfigurator::hasChanged() const
{
  return _pModelContainer->hasChanged();
}

bool PiiTableLabelerConfigurator::canReset () const
{
  /* PENDING
   * Check this later.
   */    
  //return true;
  return hasChanged();
}
bool PiiTableLabelerConfigurator::hasDefaults () const
{
  /* PENDING
   * Check this later.
   */
  return false;
}

void PiiTableLabelerConfigurator::controllerAdded()
{
  updateControllers();
}

void PiiTableLabelerConfigurator::acceptChanges ()
{
  _pModelContainer->acceptChanges();
}

void PiiTableLabelerConfigurator::reset ()
{
  _pModelContainer->reset();
}

void PiiTableLabelerConfigurator::setDefaults ()
{
  /* PENDING
   * Implement this
   */
}

void PiiTableLabelerConfigurator::addRow()
{
  QTableView* currView = qobject_cast<QTableView*>(_pTabWidget->currentWidget());
  if (currView)
    {
      //QList<int> selectedRows = getIndivSelectedRows(currView);
      QList< QPair<int, int> > lstSelectedRows = selectedRows(currView);
      QAbstractItemModel* model = currView->model();

      if (lstSelectedRows.size() > 0)
        {
          //Add the row just before the selected items.
          
          //Reason for starting from the last selected row is that in that
          //way it is easier to keep track, which rows has been allready
          //removed. We have also take into account, that the selected
          //area is not necessary continous. That's wha we remove only one
          //row at a time.
          for(int i = lstSelectedRows.size()-1; i>=0; i--)
            {
              int row = lstSelectedRows[i].first;
              int amount = lstSelectedRows[i].second;

              //deselect the currently selected items (rows).
              currView->selectionModel()->clear();
              
              if (model->insertRows(row, amount))
                {
                  //If the rows were successfully added, the new rows
                  //are selected in the view.
                  selectRows(currView, row, amount);
                }
            }
        }
      else
        {
          //No rows selected, the new row is added at the end.
          model->insertRows(model->rowCount(), 1 ) ;
        }
      
      // NOTE: updateControllers() need not to be called here, because
      // it get called anyway, as a result of the changed selections in the
      // view.
      //
      // Update the status of buttons, etc.
      //updateControllers();
            
    }  
  /* PENDING
   * Implement this
   */
}



void PiiTableLabelerConfigurator::removeRow()
{
  QTableView* currView = qobject_cast<QTableView*>(_pTabWidget->currentWidget());
  if (currView)
    {
      //QList<int> selectedRows = getSelectedRows(currView);
      QList< QPair<int, int> > lstSelectedRows = selectedRows(currView);
      
      //Reason for starting from the last selected row is that in that
      //way it is easier to keep track, which rows has been allready
      //removed. We have also take into account, that the selected
      //area is not necessary continous. 
      for(int i = lstSelectedRows.size()-1; i>=0; i--)
        { 
          currView->model()->removeRows(lstSelectedRows[i].first, lstSelectedRows[i].second ) ;
        }

      // NOTE: updateControllers() must be called here, because
      // for some it won't get  called as a result of the changed
      // selections in the view.
      //
      // Update the status of buttons, etc.
      updateControllers();
    }
}

void PiiTableLabelerConfigurator::moveRowUp()
{
  QTableView* currView = qobject_cast<QTableView*>(_pTabWidget->currentWidget());
  if (currView)
    {
      QList< QPair<int, int> > lstSelectedRows = selectedRows(currView);

      PiiTableLabelerModel* tlModel = qobject_cast<PiiTableLabelerModel*>(currView->model());
      if (tlModel)
        {
          //Here actually the direction of the loop doesn't matter, if
          //we can be sure, that the rows in selectedRows, are
          //ordered. If we cannot be sure of that, then it is more
          //secure to start from the begining of the list.
          for(int i = 0; i<lstSelectedRows.size(); i++)
            {           
              int row = lstSelectedRows[i].first;
              int amount = lstSelectedRows[i].second;


              //deselect the lines before moving
              currView->selectionModel()->clear();
                            
              tlModel->moveRowsUp(row, amount ) ;

              //select the moved lines.
              selectRows(currView, row-1, amount);
            }
        }

      // NOTE: updateControllers() need not to be called here, because
      // it get called anyway, as a result of the changed selections in the
      // view.
      //
      // Update the status of buttons, etc.
      //updateControllers();
    }
}

void PiiTableLabelerConfigurator::moveRowDown()
{
  QTableView* currView = qobject_cast<QTableView*>(_pTabWidget->currentWidget());
  if (currView)
    {
      QList< QPair<int, int> > lstSelectedRows = selectedRows(currView);

      PiiTableLabelerModel* tlModel = qobject_cast<PiiTableLabelerModel*>(currView->model());
      if (tlModel)
        {
          //Here actually the direction of the loop doesn't matter, if
          //we can be sure, that the rows in selectedRows, are
          //ordered. If we cannot be sure of that, then it is more
          //secure to start from the end of the list.
          for(int i = lstSelectedRows.size()-1; i>=0; i--)
            {
              int row = lstSelectedRows[i].first;
              int amount = lstSelectedRows[i].second;

              //deselect the lines before moving.
              currView->selectionModel()->clear();
              
              tlModel->moveRowsDown(row, amount ) ;

              //select the moved lines.
              selectRows(currView, row+1, amount);
            }
        }

      // NOTE: updateControllers() need not to be called here, because
      // it get called anyway, as a result of the changed selections in the
      // view.
      //
      // Update the status of buttons, etc.
      //updateControllers();
    }
}

void PiiTableLabelerConfigurator::changeRuleType()
{
  QTableView* currView = qobject_cast<QTableView*>(_pTabWidget->currentWidget());
  if (currView)
    {
      //collect rows which rule type must change
      QModelIndexList selectedIndexes = currView->selectionModel()->selectedIndexes();
      QList<int> lstRows;
      for (int i=0; i<selectedIndexes.size(); i++)
        {
          if (!lstRows.contains(selectedIndexes[i].row()))
            lstRows << selectedIndexes[i].row();
        }

      //change rule type
      if (lstRows.size() > 0)
        qobject_cast<PiiTableLabelerModel*>(currView->model())->changeRuleType(lstRows);
    }
}




/* This is a private helper function, which returns the list of row
 * numbers that are selected in the view. The rows, which contain at
 * least one selected item, are included in the list
 */
QList<int> PiiTableLabelerConfigurator::indivSelectedRows(QTableView* currView) const
{
  QList<int> selectedRows;
  QModelIndexList selectedIndexes =  currView->selectionModel()->selectedIndexes();
  QModelIndex index;
  foreach (index, selectedIndexes)
    {
      int row = index.row();
      if (!selectedRows.contains(row))
        selectedRows.append(row);
    }

  //The list returned must be orderer before returning, because we
  //cannot assume that the selections are automatically in order.
  qSort(selectedRows.begin(), selectedRows.end());
  return selectedRows;
}

/* Private function that returns the selected rows in a list of
   QPairs. The first value of the each QPair item determines the row
   number, and the second value in the item the number of adjacent
   selected rows. If selected are is non-continuous, the size of the
   list is greater than one. */
QList< QPair<int, int> > PiiTableLabelerConfigurator::selectedRows(QTableView* currView) const
{
  QList< QPair<int, int> > retList;
  QList<int> indivRows = indivSelectedRows(currView);
  
  if (indivRows.size() == 0)
    {
      return retList;
    }

  QPair<int,int> currItem = QPair<int,int>(indivRows[0],1);
  
  for (int i = 1; i<indivRows.size(); i++)
    {
      if (indivRows[i]-1 == currItem.first)
        currItem.second++;
      else
        {
          retList << currItem;
          currItem.first = indivRows[i];
          currItem.second = 1;
        }
    }
  retList << currItem;
  return retList;
}

/* Private helper function, which selects in the view the @p amount amount of rows
 * starting from the row @p.
 */
void PiiTableLabelerConfigurator::selectRows(QAbstractItemView* view, int row, int amount)
{
  QAbstractItemModel* model = view->model();
  QModelIndex topLeft = model->index(row,0,QModelIndex());
  QModelIndex bottomRight = model->index(row+amount-1,model->columnCount()-1,QModelIndex());
  view->selectionModel()->select( QItemSelection(topLeft, bottomRight), QItemSelectionModel::Select); 
}

/* This private function update the status of controllers, like
 * toolbuttons and pop-up menus. The function checks the status of
 * selections, and enables/disables adding/remowing/moving up/moving
 * down the rows, when needed.
 */
void PiiTableLabelerConfigurator::updateControllers()
{
  QTableView* currView = qobject_cast<QTableView*>(_pTabWidget->currentWidget());
  QList< QPair<int, int> > lstSelectedRows;

  if (currView)
    {
      lstSelectedRows = selectedRows(currView);
      // Adding the rows is possible allways, when a visible
      // QTableView object exists 
      emit addingRowsEnabled(true);
    }
  
  if (lstSelectedRows.size() == 0)
    {
      emit removingRowsEnabled(false);
      emit movingRowsUpEnabled(false);
      emit movingRowsDownEnabled(false);
      emit changeRuleTypesEnabled(false);
    }
  else
    {
      emit changeRuleTypesEnabled(true);
      emit removingRowsEnabled(true);
      if (lstSelectedRows[0].first == 0)
        emit movingRowsUpEnabled(false);
      else
        movingRowsUpEnabled(true);

      //The following variable stores the last selected row added by one.
      int lastSelRowInc = lstSelectedRows[lstSelectedRows.size()-1].first + lstSelectedRows[lstSelectedRows.size()-1].second;
      if (lastSelRowInc == currView->model()->rowCount())
        emit movingRowsDownEnabled(false);
      else
        emit movingRowsDownEnabled(true);
    }

  emit contentsChanged(hasChanged());
      
}


/* This is a private slot, which is called, when the selections has
   been changed in the view.
 */
void PiiTableLabelerConfigurator::selectionChanged(const QItemSelection & /*selected*/, const QItemSelection & /*deselected*/ )
{
  updateControllers();
}

/* This private slot is called, when the currently visible tab has
 *  been changed.
 */
void PiiTableLabelerConfigurator::currentTabChanged(int /*index*/)
{
  updateControllers();
}

void PiiTableLabelerConfigurator::setChangedState(bool status)
{
  setChanged(status);
}

///////////////// PiiTableLabelerToolBar implementation //////////////////
PiiTableLabelerToolBar::PiiTableLabelerToolBar(QWidget* parent) : QWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSizeConstraint ( QLayout::SetMinimumSize );
  createButtons();
  
  layout->addStretch(1);
  
  //The signals are connected in createUiComponent() function of operation.
}

/* 
   Private function, which creates the too buttons, and adds them into
   the toolbar. */
void PiiTableLabelerToolBar::createButtons()
{
  _pAddButton = createButton();
  _pAddButton->setText(tr("New rule"));
  _pRemoveButton = createButton();
  _pRemoveButton->setText(tr("Delete rule"));
  _pMoveUpButton = createButton();
  _pMoveUpButton->setText(tr("Move up"));
  _pMoveDownButton = createButton();
  _pMoveDownButton->setText(tr("Move down"));
  _pChangeRuleButton = createButton();
  _pChangeRuleButton->setText(tr("Switch rule type"));
}

/* 
   Private function, which creates the too buttons, and adds them into
   the toolbar. */
QPushButton* PiiTableLabelerToolBar::createButton()
{
  QPushButton* button = new QPushButton(this);
  button->setMinimumSize(QSize(32, 32));
  //button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  this->layout()->addWidget(button);
  button->setEnabled(false);
  return button;
}


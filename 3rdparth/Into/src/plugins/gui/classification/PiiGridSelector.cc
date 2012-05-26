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

#include "PiiGridSelector.h"

PiiGridSelector::PiiGridSelector(int cols,int rows) : _gridSize(0,0),
                                                      _popupCell(0,0),
                                                      _cellSize(50),
                                                      _iCurrentX(0),
                                                      _iCurrentY(0),
                                                      _iStartX(0),
                                                      _iStartY(0),
                                                      _copyValue(0.0),
                                                      _pImagePopup(0),
                                                      _pPopup(0),
                                                      _bDragging(false),
                                                      _bMultiselecting(false),
                                                      _bShowCellInfo(true),
                                                      _bShowCellValue(false),
                                                      _bShowOverlayColoring(false),
                                                      _mousePressed(0,0),
                                                      _mouseCurrent(0,0),
                                                      _pFitActionGroup(0),
                                                      _pFitToViewAction(0),
                                                      _pFillViewAction(0),
                                                      _pNoFitAction(0),
                                                      _pSelectAllAction(0),
                                                      _pShowCellValuesAction(0),
                                                      _pShowCellInfoAction(0),
                                                      _pShowOverlayColoringAction(0),
                                                      _unitScale(1.0,1.0),
                                                      _bShowBufferingProperties(false)
{
  setGridSize(QSize(cols,rows));
  setCellSize(50);     // Default size for cells
    
  _pFitToViewAction = new QAction(tr("Fit to View"), this);
  _pFitToViewAction->setCheckable(true);
  _pFitToViewAction->setShortcut(QKeySequence("Ctrl+1"));
  connect(_pFitToViewAction, SIGNAL(triggered(bool)), this, SLOT(setFitToView(bool)));
    
  _pFillViewAction = new QAction(tr("Fit Visible"), this);
  _pFillViewAction->setCheckable(true);
  _pFillViewAction->setShortcut(QKeySequence("Ctrl+2"));
  connect(_pFillViewAction, SIGNAL(triggered(bool)), this, SLOT(setFillView(bool)));
    
  _pNoFitAction = new QAction(tr("No Fit"), this);
  _pNoFitAction->setCheckable(true);
  _pNoFitAction->setShortcut(QKeySequence("Ctrl+3"));
  connect(_pNoFitAction, SIGNAL(triggered()), this, SLOT(setNoFit()));
    
  _pFitActionGroup = new QActionGroup(this);
  _pFitActionGroup->addAction(_pFitToViewAction);
  _pFitActionGroup->addAction(_pFillViewAction);
  //_pFitActionGroup->addAction(_pNoFitAction);
  _pFitActionGroup->setExclusive(true);

  //setFitToView(true);
    
  _pShowCellValuesAction = new QAction(tr("Cell Values"), this);
  _pShowCellValuesAction->setCheckable(true);
  _pShowCellValuesAction->setChecked (_bShowCellValue);
  connect(_pShowCellValuesAction, SIGNAL(triggered(bool)), this, SLOT(cellValueVisible(bool)));
    
  _pSelectAllAction = new QAction(tr("All"), this);
  _pSelectAllAction->setShortcut(QKeySequence("Ctrl+a"));
  connect(_pSelectAllAction, SIGNAL(triggered(bool)), this, SLOT(selectAll()));
    
  _pShowCellInfoAction = new QAction(tr("Cell Info"), this);
  _pShowCellInfoAction->setCheckable(true);
  _pShowCellInfoAction->setChecked (_bShowCellInfo);
  connect(_pShowCellInfoAction, SIGNAL(triggered(bool)), this, SLOT(cellInfoVisible(bool)));
    
  _pShowOverlayColoringAction = new QAction(tr("Overlay Coloring"), this);
  _pShowOverlayColoringAction->setCheckable(true);
  _pShowOverlayColoringAction->setChecked (_bShowOverlayColoring);
  connect(_pShowOverlayColoringAction, SIGNAL(triggered(bool)), this, SLOT(setShowOverlayColoring(bool)));

  //_pOpenOutputValuesAction = new QAction(tr("Output values..."),this);
  //connect(_pOpenOutputValuesAction, SIGNAL(triggered()), this, SIGNAL(outputValuesTriggered()));
  
  selectXYArea(0,0,0,0);
}

PiiGridSelector::~PiiGridSelector()
{
  delete _pImagePopup;
  delete _pCurrentLayer;
  delete _pPopup;
}

void PiiGridSelector::setGridSize(QSize val)
{
  if(val.height()>_gridSize.height())
    {
      int n_rows = val.height()-_gridSize.height(); // new rows
      for(int i=0;i<n_rows;i++)
        {
          QList<PiiMultiImage*> row;
          for(int x=0;x<_gridSize.width();x++)
            {
              PiiMultiImage* w =  new PiiMultiImage();
              w->setParent(this);
              row.append(w);
            }
          _grid.append(row);
        }
    }
  else if(val.height() < _gridSize.height())
    {
      int n_rows = _gridSize.height() - val.height();   
      for(int i=0;i<n_rows;i++)
        {
          _grid.takeLast(); // Take out last row
        }
    }
  
  _gridSize.setHeight(val.height());
  
  if(val.width()>_gridSize.width())
    {
      for(int r=0;r< _gridSize.height();r++)
        {
          int n_cols = val.width()-_gridSize.width();
          for(int i=0; i< n_cols ;i++)
            {
              PiiMultiImage* w = new PiiMultiImage();
              w->setParent(this);
              _grid[r].append(w);
            }
        }
    }
  else if(val.width()<_gridSize.width())
    {
      for(int r=0;r< _gridSize.height();r++)
        {
          int n_cols = _gridSize.width()-val.width();
          for(int i=0; i< n_cols ;i++)
            {
              delete _grid[r].takeLast();
            }
        }
    }
  
  // go through _layers, and see if they are grid layers and resize them
  PiiSelectorLayer* layer;
  if(_layers.size()>0)
    {
      foreach(layer,_layers)
        {
          //if(layer->inherits("PiiGridLayer"))
          //  ((PiiGridLayer*)layer)->setGridSize(val);
          layer->setLimits(val); //testi
        }
    }
  _gridSize = val;
  
  int basew = _gridSize.width()*_cellSize;
  int baseh = _gridSize.height()*_cellSize;
  setBaseSize (basew,baseh);
  setMinimumSize(100,100);

  if ( _pFitToViewAction != 0 )
    updateFitMode();

  updateGridDisplays();
  // RESIZE TO WIDGET
  //resize(_gridSize.width()*_cellSize,_gridSize.height()*_cellSize); // does update();
}

QWidget* PiiGridSelector::createEditorWidget(int x, int y)
{
  QWidget* editors = new QWidget;
  QGridLayout* gridLayout = new QGridLayout;
  int pointIndex = y * _gridSize.width() + x;
  
  for ( int i=0; i<_layers.size(); i++ )
    {
      if ( _layers[i]->isEnabled() )
        {
          QString labelString = QString("%1:").arg(_layers[i]->info()->name());
          gridLayout->addWidget(new QLabel(labelString), i, 0);
          gridLayout->addWidget(_layers[i]->info()->editorWidget(pointIndex, _layers[i]->valueAt(x,y)), i, 1);
        }
    }
  editors->setLayout(gridLayout);
  gridLayout->setSizeConstraint(QLayout::SetFixedSize);

  if (_layers.size() <= 1)
    {
      editors->setContentsMargins(0,0,0,0);
      gridLayout->setContentsMargins(0,0,0,0);
      return editors;
    }
  else
    {
      QVBoxLayout *vlayout = new QVBoxLayout;
      vlayout->addWidget(editors);
      vlayout->addStretch();
      
      QWidget *w = new QWidget;
      w->setLayout(vlayout);
      
      QScrollArea* scroll = new QScrollArea;
      scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      scroll->setWidget(w);
      scroll->setBackgroundRole(QPalette::Mid);
      scroll->setMinimumWidth(editors->width() + 20);
      scroll->setMinimumHeight(1);
      scroll->setFocusPolicy(Qt::NoFocus);
      
      return scroll;
    }
}

QMenu *PiiGridSelector::createMenu()
{
  QMenu *menu = new QMenu;
  
  QMenu *viewMenu = menu->addMenu(tr("&View"));
  viewMenu->addAction(tr("Current image"),this,SLOT(showImagePopup()));
  viewMenu->addAction(tr("Node images"),this,SLOT(showContentOnCurrentLocation()));

  QMenu *displayMenu = menu->addMenu(tr("&Display"));
  
  /*
    displayMenu->addAction(tr("&Zoom In"), this, SLOT(zoomIn()),
    QKeySequence(tr("Ctrl++", "View|Zoom In")));
    displayMenu->addAction(tr("&Zoom Out"), this, SLOT(zoomOut()),
    QKeySequence(tr("Ctrl+-", "View|Zoom Out")));
  */
  displayMenu->addAction(_pFitToViewAction);
  displayMenu->addAction(_pFillViewAction);
  //Not shown!!  displayMenu->addAction(_pNoFitAction);
  displayMenu->addSeparator();
  displayMenu->addAction(_pShowCellValuesAction);
  displayMenu->addAction(_pShowCellInfoAction);
  displayMenu->addAction(_pShowOverlayColoringAction);
  // UPDATE CHECKING
  
  _pShowCellValuesAction->setChecked (_bShowCellValue);
  _pShowCellInfoAction->setChecked (_bShowCellInfo);
  _pShowOverlayColoringAction->setChecked(_bShowOverlayColoring );
  
  QMenu *selectMenu = menu->addMenu(tr("&Select"));
  selectMenu->addAction(tr("Unselect All"),this,SLOT(unselectAll()),
                        QKeySequence(tr("Ctrl+u", "Select|Unselect All")));

  selectMenu->addAction(_pSelectAllAction);//tr("All"),this,SLOT(selectAll()));  
  
  selectMenu->addAction(tr("&Invert"), this, SLOT(invertSelection()),
                        QKeySequence(tr("Ctrl+I", "Select|Invert")));

  if ( _pCurrentLayer != 0 )
    {
      QMenu *subSelectMenu = _pCurrentLayer->info()->selectionMenu();
      if ( subSelectMenu != 0 )
        { 
          subSelectMenu->setTitle(tr("By Class"));
          selectMenu->addMenu(subSelectMenu);
        }
    }

  if ( _bShowBufferingProperties )
    menu->addAction(tr("&Buffering..."), this, SIGNAL(bufferingActionTriggered()));

  //selectMenu->addAction(tr("Invert"),this,SLOT(invertSelection()));
  menu->addSeparator();
  PiiSelectorLayer *layer;
  foreach(layer,_layers)
    {
      if ( layer->isEnabled() )
        {
          QMenu *submenu = layer->info()->menu();
          //submenu->addSeparator();
          //submenu->addAction(_pOpenOutputValuesAction);
          menu->addMenu(submenu);
        }
    }

  
  return menu;
}

// ***************************************************************

void PiiGridSelector::removeImage(int index, int x, int y, bool silent)
{
  if(x<0 || x>=_gridSize.width() || y<0 || y >= _gridSize.height())
    return;
  _grid[y][x]->removeAt(index);
  
  if (!silent)
    update(cellArea(x,y));
}

void PiiGridSelector::removeImage(QImage* im, bool silent)
{  
  for(int y=0;y<_gridSize.height();y++)
    { 
      for(int x=0;x<_gridSize.width();x++)
        {    
          _grid[y][x]->removeImage(im);
          if (!silent)
            update(cellArea(x,y));
        }
    }
}

void PiiGridSelector::removeImages(int x, int y, bool silent)
{
  if(x<0 || x>=_gridSize.width() || y<0 || y >= _gridSize.height()) return;
  _grid[y][x]->removeImages();
  if (!silent)
    update(cellArea(x,y));
}

void PiiGridSelector::removeImages(double x, double y, bool silent)
{
  removeImages( (int)x, (int)y, silent);
}

void PiiGridSelector::removeImage(double x, double y, bool silent)
{
  int ix = (int)x;
  int iy = (int)y;
  if (ix < 0 || ix >= _gridSize.width() || iy < 0 || iy >= _gridSize.height()) return;
  _grid[iy][ix]->removeImage();

  if (!silent)
    update(cellArea(ix,iy));
}

void PiiGridSelector::removeAllImages()
{
  for(int y=0;y<_gridSize.height();y++)
    for(int x=0;x<_gridSize.width();x++)
      _grid[y][x]->removeImages();
  update();
}

// ***************************************************************************
 
void PiiGridSelector::setCellSize(int size)
{
  if (size < 5)
    size = 5;
  
  resize(_gridSize.width()*size,_gridSize.height()*size);
  _cellSize = size;
  update();
}

void PiiGridSelector::zoomIn()
{
  zoom(5);
}
void PiiGridSelector::zoomOut()
{
  zoom(-5);
}

void PiiGridSelector::zoom(int factor)
{
  if ( _pFitToViewAction->isChecked() || _pFillViewAction->isChecked())
    setNoFit();

  setCellSize(_cellSize+factor);
}

void PiiGridSelector::resizeEvent ( QResizeEvent * event )
{
  QSize newsize = event->size();
  int newcellsize = newsize.width() / _gridSize.width();
  
  if(_pFitToViewAction->isChecked())
    {
      if( newcellsize > (newsize.height() / _gridSize.height()))
        newcellsize = (newsize.height() / _gridSize.height());
    } else if(_pFillViewAction->isChecked())    
    if( newcellsize < (newsize.height() / _gridSize.height()))
      newcellsize = (newsize.height() / _gridSize.height());
  _cellSize = newcellsize;
}

void PiiGridSelector::updateFitMode()
{
  // Update scrollarea mode
  if(_pScrollarea!=0)
    {
      if(_pFitToViewAction->isChecked() == true)
        _pScrollarea->setAspectRatioMode(Qt::KeepAspectRatio);
      else if(_pFillViewAction->isChecked() == true)
        _pScrollarea->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);
      else _pScrollarea->setAspectRatioMode(Qt::IgnoreAspectRatio);
    }
  
  if(_pFitToViewAction->isChecked() || _pFillViewAction->isChecked())
    if(parent()!= 0)
      {
        // This causes recalculation for _cellSize according aspect mode
        resizeEvent(new QResizeEvent(((QWidget*)parent())->size(),
                                     ((QWidget*)parent())->size()+QSize(1,1)));
      }
  
  // This causes update of components size to real size after mode has changed
  resize(_gridSize.width()*_cellSize,_gridSize.height()*_cellSize);
  update();
}

// AS FOLLOWING ARE GROUPED ACTIONS Setting one disables other two.
void PiiGridSelector::setFitToView(bool val)
{
  if(val == false)
    setNoFit();
  else {
    _pFitToViewAction->setChecked(val);
    _pFillViewAction->setChecked(false); // manual exclusive as both must be able to disable
    _pNoFitAction->setChecked(false);    
    updateFitMode();
  }
}
void PiiGridSelector::setFillView(bool val)
{
  if(val == false)
    setNoFit();
  else {
    _pFillViewAction->setChecked(val);
    _pFitToViewAction->setChecked(false); // manual exclusive as both must be able to disable
    _pNoFitAction->setChecked(false);
    updateFitMode();
  }
}
void PiiGridSelector::setNoFit()
{
  _pNoFitAction->setChecked(true);
  _pFitToViewAction->setChecked(false); // manual exclusive as both must be able to disable
  _pFillViewAction->setChecked(false); // manual exclusive as both must be able to disable  
  updateFitMode();
}

void PiiGridSelector::cellInfoVisible(bool val)
{
  _bShowCellInfo  = val;
  update();
}

void PiiGridSelector::setShowOverlayColoring(bool val)
{
  _bShowOverlayColoring = val;
  update();
}


void PiiGridSelector::cellValueVisible(bool val)
{ 
  _bShowCellValue = val;
  update();
}

// Returns the preferred height for this widget, given the width w.
int PiiGridSelector::heightForWidth ( int w ) const {
  return (int)(w * (double)_gridSize.height()/(double)_gridSize.width());
}

QRect PiiGridSelector::cellArea(int x, int y) const {
  return QRect(x*_cellSize,y*_cellSize,_cellSize,_cellSize);
}

QPoint PiiGridSelector::cellAt(QPoint p) const {
  if(_cellSize>0)
    {
      int x = p.x()/_cellSize;
      int y = p.y()/_cellSize;
      return QPoint(x,y);
    }
  return QPoint(-1,-1);
}

int PiiGridSelector::imageCount(int x, int y) const{
  if(x<0 || x>=_gridSize.width() || y<0 || y >= _gridSize.height()) return -1;
  return _grid[y][x]->imageCount();
}

double PiiGridSelector::value(double _x,double _y) const {
  int x = (int)_x;
  int y = (int)_y;
  if(x<0 || x>=_gridSize.width() || y<0 || y >= _gridSize.height()) return 0;
  if(_pCurrentLayer != 0)
    return _pCurrentLayer->valueAt(x,y);
  return 0;
}

double PiiGridSelector::value(QImage* image,PiiSelectorLayer* layer) const{
  if(layer==0) layer = _pCurrentLayer;
  
  for(int y=0;y<_gridSize.height();y++)
    for(int x=0;x<_gridSize.width();x++)
      {
        if(_grid[y][x]->contains(image))
          return layer->valueAt(x,y);
      }
  return 0;
}

void PiiGridSelector::setShownImage(int x, int y, int index)
{
  _grid[y][x]->setCurrentIndex(index);
}

void PiiGridSelector::addImage(QImage* im, double x, double y, bool silent)
{
  addImage(im,(int)x,(int)y,silent);
}

void PiiGridSelector::addImage(QImage* im,int x,int y, bool silent)
{
  if(x<0 || x >= _gridSize.width() || y <0 || y >= _gridSize.height()) return;
  else
    _grid[y][x]->addImage(im);
  if(!silent)
    update(cellArea(x,y));
}

void PiiGridSelector::keyPressEvent ( QKeyEvent * e )
{
  if (  Qt::ControlModifier & e->modifiers() )
    {
      if ( e->key() == Qt::Key_Left )
        decreaseValueToSelected();
      else if ( e->key() == Qt::Key_Down )
        decreaseValueToSelected();
      else if ( e->key() == Qt::Key_Right )
        increaseValueToSelected();
      else if ( e->key() == Qt::Key_Up )
        increaseValueToSelected();
      else if ( e->key() == Qt::Key_A ) 
        selectAll();
      else if ( e->key() == Qt::Key_I )
        invertSelection();
      else if ( e->key() == Qt::Key_U )
        unselectAll();
      else if ( e->key() == Qt::Key_1 )
        setFitToView(!_pFitToViewAction->isChecked());
      else if ( e->key() == Qt::Key_2 )
        setFillView(!_pFillViewAction->isChecked());
      else if ( e->key() == Qt::Key_C )
        _copyValue = firstValue();
      else if ( e->key() == Qt::Key_V )
        pasteCopyValue();
      else if ( e->key() == Qt::Key_S )
        emit acceptChanges();
        
    }
  else
    {
      switch ( e->key())
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          if ( _pCurrentLayer != 0)
            _pCurrentLayer->info()->showMenuWidget(QString(e->key()).toDouble());
          break;
        }

      if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return )
        {
          if ( Qt::ShiftModifier & e->modifiers())
            showContent(_iCurrentX, _iCurrentY);
          else
            showImagePopup(_iCurrentX, _iCurrentY);
        }
      else if ( e->key() == Qt::Key_Left )
        {
          _iCurrentX--;
          if ( _iCurrentX < 0 )
            _iCurrentX = 0;
          if(! (Qt::ShiftModifier & e->modifiers()))
            {
              _iStartX = _iCurrentX;
              _iStartY = _iCurrentY;
            }
          selectXYArea(_iStartX, _iStartY, _iCurrentX, _iCurrentY);
        }
      else if ( e->key() == Qt::Key_Right )
        {
          _iCurrentX++;
          if ( _iCurrentX >= _gridSize.width() )
            _iCurrentX = _gridSize.width() - 1;
          if(! (Qt::ShiftModifier & e->modifiers()))
            {
              _iStartX = _iCurrentX;
              _iStartY = _iCurrentY;
            }
          selectXYArea(_iStartX, _iStartY, _iCurrentX,_iCurrentY);
        }
      else if ( e->key() == Qt::Key_Up )
        {
          _iCurrentY--;
          if ( _iCurrentY < 0 )
            _iCurrentY = 0;
          if(! (Qt::ShiftModifier & e->modifiers()))
            {
              _iStartX = _iCurrentX;
              _iStartY = _iCurrentY;
            }
          selectXYArea(_iStartX, _iStartY, _iCurrentX,_iCurrentY);
        }
      else if ( e->key() == Qt::Key_Down )
        {
          _iCurrentY++;
          if ( _iCurrentY >= _gridSize.height() )
            _iCurrentY = _gridSize.height() - 1;
          if(! (Qt::ShiftModifier & e->modifiers()))
            {
              _iStartX = _iCurrentX;
              _iStartY = _iCurrentY;
            }
          selectXYArea(_iStartX, _iStartY, _iCurrentX,_iCurrentY);
        }
      else if ( e->key() == Qt::Key_Home )
        {
          _iCurrentX = 0;
          if(! (Qt::ShiftModifier & e->modifiers()))
            {
              _iStartX = _iCurrentX;
              _iStartY = _iCurrentY;
            }
          selectXYArea(_iStartX, _iStartY, _iCurrentX,_iCurrentY);
        }
      else if ( e->key() == Qt::Key_End )
        {
          _iCurrentX = _gridSize.width() - 1;
          if(! (Qt::ShiftModifier & e->modifiers()))
            {
              _iStartX = _iCurrentX;
              _iStartY = _iCurrentY;
            }
          selectXYArea(_iStartX, _iStartY, _iCurrentX,_iCurrentY);
        }
  
      else if(e->key() ==Qt::Key_PageUp)
        nextImageOnSelected(); 
      else if(e->key() ==Qt::Key_PageDown)
        prevImageOnSelected();

      
      // THESE shold work with QAction's shortkey, but it didn't. Remove these when its working...
      // Probably the reason is implementation of this keyPressEvent method...
      
      
      if(e->key() == '+')
        {
          //if(Qt::ShiftModifier & e->modifiers())
          //if(!_fitToView)
          setNoFit();
          setCellSize(_cellSize+5);
          //else 
          // increaseValueToSelected();
        }
      else if(e->key() == '-')
        {
          //    if(Qt::ShiftModifier & e->modifiers())
          // if(!_fitToView)
          setNoFit();
          setCellSize(_cellSize-5);
          //else 
          // decreaseValueToSelected();
        }
    }
  
  
  // Using this will proceed key presses to parent widget
  // QWidget::keyPressEvent(e);
}

void PiiGridSelector::mousePressEvent( QMouseEvent *e )
{
  setFocus();
  if(_pImagePopup!=0)
    _pImagePopup->hide();
  if(_pPopup!=0 && _pPopup->isVisible()) 
    _pPopup->hide();
  
  _mousePressed = e->pos();
  _mouseCurrent = e->pos();

  _bDragging = false;
  if((e->button()&Qt::RightButton))
    {
      _popupCell = cellAt(e->pos());
    
      // MENU TYPE 1: traditional menu
      // New menu is all ways create, as its content may change (layers)
      QMenu *menu = createMenu();
      menu->exec(e->globalPos());
      delete menu;
    }
  if ((e->button()&Qt::LeftButton))
    {
      if(_pPopup!=0) _pPopup->close();
      _bMultiselecting = true;
    
      if(Qt::ControlModifier & e->modifiers())
        invertSelectionAt(_mouseCurrent);
      else{
      
        unselectAll();
        selectArea(_mousePressed,_mouseCurrent, false);
        //invertSelectionAt(_mouseCurrent);
      }
      update();
    }
}

void PiiGridSelector::mouseReleaseEvent( QMouseEvent * )
{
  _bMultiselecting = false;
  _bDragging = false;
  update();
}

void PiiGridSelector::mouseDoubleClickEvent(QMouseEvent * e )
{
  _mouseCurrent = e->pos();
  _mousePressed = e->pos();
  if (e->button() & Qt::LeftButton)
    {
      if(e->modifiers() & Qt::ShiftModifier)
        showImagePopup();
      else
        showContent(e->pos());
    }
}

void PiiGridSelector::mouseMoveEvent(QMouseEvent *e)
{
  _mouseCurrent = e->pos();
  //if(Qt::ShiftModifier & e->modifiers())
  //  ;
  
  if(_bMultiselecting)
    {
      if(Qt::ControlModifier & e->modifiers())
        selectArea(_mousePressed,_mouseCurrent, false);
      else selectArea(_mousePressed,_mouseCurrent);
    }

  update();
}


void PiiGridSelector::wheelEvent( QWheelEvent * e )
{
  setFocus();
  
  int delta = e->delta();
  if(Qt::ControlModifier & e->modifiers())
    {
      if(delta<0)
        zoomOut();
      else if(delta>0)
        zoomIn();
    }
  else if(Qt::ShiftModifier & e->modifiers())
    {
      if(delta>0)      nextImageOnLocation(e->pos()); // or use nextImageOnSelected() 
      else if(delta<0) prevImageOnLocation(e->pos()); // or use prevImageOnSelected()
    }
  else
    {
      if(delta>0) increaseValueToSelected();
      else if(delta<0) decreaseValueToSelected();
    }
}

void PiiGridSelector::showImagePopup()
{
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(QRect(x*_cellSize,y*_cellSize,_cellSize,_cellSize).contains(_mouseCurrent))
            showImagePopup(x,y);
        }
    }
  //update();
}

void PiiGridSelector::showImagePopup(int x, int y)
{
  _grid[y][x]->showImageDisplay();
}

void PiiGridSelector::setUnitScale(const QSizeF& unitScale)
{
  //update unit scale to the grid cells
  for (int i=0; i<_grid.size(); i++)
    {
      for (int j=0; j<_grid[i].size(); j++)
        {
          if (_grid[i][j])
            _grid[i][j]->setUnitScale(unitScale);
        }
    }

  _unitScale = unitScale;
}


void PiiGridSelector::nextImageOnSelected()
{
  /*
  // FOR ONLY MOUSE TARGET CELL
  for(int y=0;y<_gridSize.height();y++)
  {
  for(int x=0;x<_gridSize.width();x++)
  {
  if(QRect(x*_cellSize,y*_cellSize,_cellSize,_cellSize).contains(_mousePressed))
  {
	_grid[y][x]->nextImage();
  }
  }
  }
  */
  // FOR ALL SELECTED:
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(_grid[y][x]->isSelected())
            _grid[y][x]->nextImage();
        }
    }
  update();
}

void PiiGridSelector::nextImageOnLocation(QPoint loc)
{

  // FOR ONLY MOUSE TARGET CELL
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(QRect(x*_cellSize,y*_cellSize,_cellSize,_cellSize).contains(loc))
            {
              _grid[y][x]->nextImage();
            }
        } 
    } 
  update();
}



void PiiGridSelector::prevImageOnSelected()
{
  // FOR ALL SELECTED:
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(_grid[y][x]->isSelected())
            _grid[y][x]->prevImage();
        }
    }
  update();
}

void PiiGridSelector::prevImageOnLocation(QPoint loc)
{
  // FOR ONLY MOUSE TARGET CELL
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(QRect(x*_cellSize,y*_cellSize,_cellSize,_cellSize).contains(loc))
            {
              _grid[y][x]->prevImage();
              break;
            }
        }
    }
  update();
}

void PiiGridSelector::showContentOnCurrentLocation()
{
  showContent(_mouseCurrent);
}

void PiiGridSelector::showContent(QPoint p1)
{
  QRect sel(p1,p1);
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(sel.intersects(QRect(x*_cellSize,y*_cellSize,_cellSize,_cellSize)))
            showContent(x,y);
        }
    }
}

void PiiGridSelector::updateGridDisplays()
{
  //update grid displays
  for (int i=0; i<_grid.size(); i++)
    {
      for (int j=0; j<_grid[i].size(); j++)
        {
          if (_grid[i][j]->hasIconListWidget())
            _grid[i][j]->updateIconListWidget(createEditorWidget(j,i));
        }
    }
}

void PiiGridSelector::showContent(int x, int y)
{
  _grid[y][x]->showIconListWidget(createEditorWidget(x,y), tr("Node images (%1,%2)").arg(x).arg(y));
}

// ************ SELECTION ************************************************

/*void PiiGridSelector::componentSelected(PiiMultiImage* w, bool selected)
{
  qDebug("PiiGridSelector::componentSelected");
  emit imagesSelected(w->images(),selected);
  checkSelectedValues();
  }*/

void PiiGridSelector::reselect()
{
  reselectCells();
}

void PiiGridSelector::reselectCells()
{
  emit imageSelectionChanged(selectedImages(),selectedValues(),true);
  checkSelectedValues();
}

QList<PiiMultiImage*> PiiGridSelector::selected() const
{
  QList<PiiMultiImage*> tl;
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(_grid[y][x]->isSelected())
            tl.append(_grid[y][x]);
        }
    }
  return tl;
}

QList<QImage*> PiiGridSelector::selectedImages() const
{
  QList<QImage*> tl;
  for (int y=0;y<_gridSize.height();y++)
    {
      for (int x=0;x<_gridSize.width();x++)
        {
          if (_grid[y][x]->isSelected())
            tl << _grid[y][x]->images();
        }
    }
  return tl;
}

QList<double> PiiGridSelector::selectedValues() const
{
  QList<double> tl;
  if(_pCurrentLayer == 0)
    return tl;
  
  for (int y=0;y<_gridSize.height();y++)
    {
      for (int x=0;x<_gridSize.width();x++)
        {
          if (_grid[y][x]->isSelected())
            {
              double val = _pCurrentLayer->valueAt(x,y);
              for (int i=_grid[y][x]->imageCount(); i--; )
                tl << val;
            }
        }
    }
  return tl;
}

double PiiGridSelector::firstValue() const
{
  for(int y=0;y<_gridSize.height();y++)
    for(int x=0;x<_gridSize.width();x++)
      if(_grid[y][x]->isSelected())
        return _pCurrentLayer->valueAt(x,y);

  return 0;
}

void PiiGridSelector::selectXYArea(int sx, int sy, int cx, int cy)
{
  QList<QImage*> unselectedimages;
  
  int minx, maxx, miny, maxy;
  if ( sx < cx )
    {
      minx = sx;
      maxx = cx;
    }
  else
    {
      minx = cx;
      maxx = sx;
    }

  if ( sy < cy )
    {
      miny = sy;
      maxy = cy;
    }
  else
    {
      miny = cy;
      maxy = sy;
    }
  
  for(int y=0;y<_gridSize.height();y++)
    for(int x=0;x<_gridSize.width();x++)
      if ( (y >= miny && y <= maxy) &&
           (x >= minx && x <= maxx))
        _grid[y][x]->select(true);
      else
        {
          unselectedimages << _grid[y][x]->images();
          _grid[y][x]->select(false);
        }

  emit imageSelectionChanged(unselectedimages, selectedValues(), false);
  reselectCells();
  update();
}

void PiiGridSelector::unselectAll()
{

  for(int y=0;y<_gridSize.height();y++)
    for(int x=0;x<_gridSize.width();x++)
      _grid[y][x]->select(false);
  emit allImagesUnselected();
  checkSelectedValues();
  update();
}

void PiiGridSelector::selectAll()
{
  for(int y=0;y<_gridSize.height();y++)
    for(int x=0;x<_gridSize.width();x++)
      {
        _grid[y][x]->select(true);
      }
  reselectCells();
  update();
}

void PiiGridSelector::selectByClass(double value, int layer)
{
  unselectAll();
  bool found = false;
  for(int y=0;y<_gridSize.height();y++)
    for(int x=0;x<_gridSize.width();x++)
      {
        if ( _layers[layer]->valueAt(x,y) == value )
          {
            _grid[y][x]->select(true);
            found = true;
          }
      }
  if ( found )
    {
      reselectCells();
      update();
    }
}

void PiiGridSelector::selectArea(QPoint p1, QPoint p2, bool do_unselect)
{
  QRect sel(p1,p2);
  QList<QImage*> unselectedimages;
  
  for(int y=_gridSize.height();y--;)
    {
      for(int x=_gridSize.width();x--;)
        {
          if(sel.intersects(QRect(x*_cellSize,y*_cellSize,_cellSize,_cellSize)))
            {
              _grid[y][x]->select(true);
            }
          else
            {
              if (do_unselect)
                {
                  if(_grid[y][x]->isSelected())
                    unselectedimages << _grid[y][x]->images();
                  _grid[y][x]->select(false);
                }
            }
        }
    }

  _iCurrentX = p2.x() / _cellSize;
  _iCurrentY = p2.y() / _cellSize;
  _iStartX = _iCurrentX;
  _iStartY = _iCurrentY;

  emit imageSelectionChanged(unselectedimages,selectedValues(),false);  
  reselectCells();
  update();
}

void PiiGridSelector::pasteCopyValue()
{
  //qDebug("paste");
  bool state = false;
  
  for(int y=_gridSize.height();y--;)
    for(int x=_gridSize.width();x--;)
      if ( _grid[y][x]->isSelected() )
        {
          _pCurrentLayer->setValueAt(x,y,_copyValue);
          state = true;
        }

  update();
  reselectCells();
  if ( state )
    {
      emit stateChanged(true);
    }
}

QList<PiiMultiImage*> PiiGridSelector::cellsContaining(QPoint p) const {
  QList<PiiMultiImage*> list;
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {       
          if(_grid[y][x]->imagesContain(p))
            list.append(_grid[y][x]);
        }
    }
  return list;
}

void PiiGridSelector::selectImagesContaining(QPoint p, int modifiers)
{
  bool normalSelection = (modifiers & Qt::ControlModifier) == 0;
  for(int y=_gridSize.height(); y--; )
    for(int x=_gridSize.width(); x--; )
      {
        if (normalSelection)
          _grid[y][x]->select(_grid[y][x]->imagesContain(p));
        else if (_grid[y][x]->imagesContain(p))
          _grid[y][x]->select(!_grid[y][x]->isSelected());
      }

  emit imagesSelected(selectedImages(),selectedValues());
  checkSelectedValues();
  update();
}

void PiiGridSelector::selectImagesIntersecting(QRect r, int modifiers)
{
  bool normalSelection = (modifiers & Qt::ControlModifier) == 0;
  for(int y=_gridSize.height(); y--; )
    for(int x=_gridSize.width(); x--; )
      {
        if (normalSelection)
          _grid[y][x]->select(_grid[y][x]->imagesIntersect(r));
        else if (_grid[y][x]->imagesIntersect(r))
          _grid[y][x]->select(!_grid[y][x]->isSelected());
      }

  emit imagesSelected(selectedImages(),selectedValues());
  checkSelectedValues();
  update();
}

void PiiGridSelector::invertSelectionAt(QPoint p1)
{
  invertSelectionAtArea(p1,p1);
}

void PiiGridSelector::invertSelectionAtArea(QPoint p1,QPoint p2)
{
  QRect sel(p1,p2);
  QList<QImage*> unselectedimages;
  
  for (int y=0;y<_gridSize.height();y++)
    {
      for (int x=0;x<_gridSize.width();x++)
        {
          if (sel.intersects(QRect(x*_cellSize,y*_cellSize,_cellSize,_cellSize)))
            {
              if(_grid[y][x]->isSelected())
                {
                  unselectedimages << _grid[y][x]->images();
                  _grid[y][x]->select(false);
                }
              else
                {
                  _grid[y][x]->select(true);
                }
            }
        }
    }
  emit imageSelectionChanged(unselectedimages,selectedValues(),false);  
  reselectCells();
  update();
}

void PiiGridSelector::invertSelection()
{
  QList<QImage*> unselectedimages;

  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(_grid[y][x]->isSelected())
            {
              unselectedimages << _grid[y][x]->images();
              _grid[y][x]->select(false);
            } else {	  
            _grid[y][x]->select(true);
	  
          }
        }
    }
  emit imageSelectionChanged(unselectedimages,selectedValues(),false);
  reselectCells();
  update();
}

void PiiGridSelector::checkSelectedValues()
{
  //qDebug("PiiGridSelector::checkSelectedValues");
  bool multivalue = false;
  bool first_sel_found = false;
  double selvalue = 0.0;
  
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(_grid[y][x]->isSelected())
            {
              
              if(!first_sel_found)
                {
                  first_sel_found=true;
                  if(_pCurrentLayer != 0)
                    {
                      for ( int i=0; i<_layers.size(); i++ )
                        _layers[i]->info()->setCurrentValue(_layers[i]->valueAt(x,y));
                      selvalue = _pCurrentLayer->valueAt(x,y);
                    }  
                }
              else
                {
                  if(_pCurrentLayer != 0)
                    if(selvalue != _pCurrentLayer->valueAt(x,y))
                      multivalue = true;
                }
            }
        }
    }
  if(first_sel_found)
    {
      if(multivalue)
        emit selectionStateMultivalue(); //tämä ei toimi vielä kunnolla
      else
        {
          if ( _pCurrentLayer != 0 )
            _pCurrentLayer->info()->setEditorValue(selvalue);
        }
    }
  
}

// ************** PAINTING ***********************************************

void PiiGridSelector::paintEvent(QPaintEvent* /*e*/)
{
  QPainter p(this);
  
  int min_x = 0;
  int max_x = _gridSize.width();
  int min_y = 0;
  int max_y = _gridSize.height();
  int cap = 5;
  int cap2 = cap * 2;

  for(int y=min_y; y<max_y; y++)
    {
      for(int x=min_x; x< max_x; x++)
        {
          QImage *im = _grid[y][x]->currentImage();
          if(im!=0)
            {
              //scale the image
              double aspectRatio = _unitScale.width() / _unitScale.height();
              QSize scaledSize(im->width(), (int)(im->height() / aspectRatio + 0.5));

              int imageCellWidth = _cellSize-cap2;
              int imageCellHeight = _cellSize-cap2;
              int imageX = x*_cellSize+cap;
              int imageY = y*_cellSize+cap;
              
              QImage image = im->scaled(scaledSize).scaled(imageCellWidth, imageCellHeight,Qt::KeepAspectRatio);

              if (image.width() > image.height())
                imageY = imageY + (imageCellHeight - image.height()) / 2;
              else if (image.width() < image.height())
                imageX = imageX + (imageCellWidth - image.width()) / 2;
              
              p.drawImage(imageX, imageY, image);
              
            } else {
            p.setPen(Qt::black);

            p.setBrush(Qt::NoBrush);
            p.drawRect(x*_cellSize+cap,y*_cellSize+cap,_cellSize-cap2,_cellSize-cap2);
          }
          if(_pCurrentLayer != 0)
            {
              double cell_value = _pCurrentLayer->valueAt(x,y);
	  
              QPen temp = _pCurrentLayer->info()->pen(cell_value);
			  temp.setWidth(2);
			  p.setPen(temp);
				
              if(_bShowOverlayColoring)
                p.setBrush(_pCurrentLayer->info()->brush(cell_value));
              else 
                p.setBrush(Qt::NoBrush);
	  
              p.drawRect(x*_cellSize+cap,y*_cellSize+cap,_cellSize-cap2,_cellSize-cap2);
	  
              if(_bShowCellValue)
                {
                  p.setPen(Qt::black);

                  p.drawText(x*_cellSize+cap+3,y*_cellSize+cap+12+_cellSize-cap2-13,QString("%1").arg(cell_value));
                  p.setPen(Qt::white);
                  p.drawText(x*_cellSize+cap+2,y*_cellSize+cap+11+_cellSize-cap2-13,QString("%1").arg(cell_value));
                }
            }
          if(_bShowCellInfo)
            {
              int isize = _grid[y][x]->imageCount();
              p.setPen(Qt::black);
              p.drawText(x*_cellSize+cap+3,y*_cellSize+cap+12,QString("%1").arg(isize));
              p.setPen(Qt::white);
              p.drawText(x*_cellSize+cap+2,y*_cellSize+cap+11,QString("%1").arg(isize));
            }
      
          if(_grid[y][x]->isSelected())
            {
              p.setPen(Qt::white);
              p.setBrush(Qt::NoBrush);
              p.drawRect(x*_cellSize+cap-2,y*_cellSize+cap-2,_cellSize-cap2+3,_cellSize-cap2+3);
            } else {
            p.setBrush(QBrush(Qt::blue,Qt::NoBrush));
            //p.drawRect(x*_cellSize,y*_cellSize,_cellSize,_cellSize);
          }
        }
    }
  
  if(_bMultiselecting)
    {
      p.setPen(Qt::DashLine);
      p.setBrush(Qt::NoBrush);
	  p.setPen(QPen (Qt::white,1,Qt::DotLine));
      p.setRenderHint(QPainter::Antialiasing);
      
      paintSquare(p,_mousePressed,_mouseCurrent);
    }

  if ( !isEnabled() )
    p.fillRect(QRect(min_x, min_y, max_x * _cellSize, max_y * _cellSize),QBrush(QColor(125,125,125,125)));
}

/**
 * Draws Square between two points
 **/
void PiiGridSelector::paintSquare(QPainter& p,QPoint p1,QPoint p2)
{
  p.drawLine( p1.x(),p1.y(),p1.x(),p2.y()); 
  p.drawLine( p1.x(),p2.y(),p2.x(),p2.y()); 
  p.drawLine( p2.x(),p2.y(),p2.x(),p1.y()); 
  p.drawLine( p2.x(),p1.y(),p1.x(),p1.y());
}

QGradient PiiGridSelector::createGradientBrush(QColor color, int radius)
{
  QRadialGradient gradient(radius/2,radius/2, radius,(int)(0.4*radius),(int)(0.4*radius));
  QColor c1 = Qt::white; c1.setAlphaF(color.alphaF());
  QColor c2 = Qt::black; c2.setAlphaF(color.alphaF());
  gradient.setColorAt(0.0, c1);
  gradient.setColorAt(0.5, color);
  gradient.setColorAt(1, c2);
  return gradient;
}

void PiiGridSelector::paintGradientButton(QPainter& p,QPoint loc,int radius,QColor color)
{  
  QRadialGradient gradient(radius/2,radius/2, radius,(int)(0.4*radius),(int)(0.4*radius));
  QColor c1 = Qt::white; c1.setAlphaF(color.alphaF());
  QColor c2 = Qt::black; c2.setAlphaF(color.alphaF());
  
  gradient.setColorAt(0.0, c1);
  gradient.setColorAt(0.5, color);
  gradient.setColorAt(1, c2);
  p.setBrush(gradient);
  
  p.save();
  p.translate(loc.x()-radius/2,loc.y()-radius/2);

  p.drawEllipse(0, 0, radius,radius);
  p.restore();
}

void PiiGridSelector::setShowCellInfo(bool value)
{
  _bShowCellInfo = value;
  update();
}

// ** METHODS RELATED TO LAYERS *****************************
/*void PiiGridSelector::assignValueToSelected(int value,int layer)
  {
  //qDebug("PiiGridSelector::assignValueToSelected(int value, int layer) %i %i", value, layer);
  if(_pCurrentLayer==0)
  return;
  for(int y=0;y<_gridSize.height();y++)
  {
  for(int x=0;x<_gridSize.width();x++)
  {
  if(_grid[y][x]->isSelected())
  {
  if(layer==-1)
  _pCurrentLayer->setValueAt(x,y,value);
  else
  {
  if(layer >= 0 && layer<_layers.size())
  _layers[layer]->setValueAt(x,y,value);
  }
  }
  }
  }
  update();
  reselectCells();
  }*/

/**
 * Set new value on current layer to each cells that is selected
 **/
void PiiGridSelector::assignValueToSelected(double value, int layer)
{
  bool state = false;
  
  if(_pCurrentLayer==0)
    return;
  for(int y=0;y<_gridSize.height();y++)
    {
      for(int x=0;x<_gridSize.width();x++)
        {
          if(_grid[y][x]->isSelected())
            {
              if(layer==-1)
                {
                  if ( _pCurrentLayer->valueAt(x,y) != value)
                    state = true;
                  _pCurrentLayer->setValueAt(x,y,value);
                }
              else
                {
                  if(layer >= 0 && layer<_layers.size())
                    {
                      if ( _layers[layer]->valueAt(x,y) != value)
                        state = true;
                      _layers[layer]->setValueAt(x,y,value);
                    }
                }
            }
        }
    }
  
  update();
  reselectCells();
  if ( state )
    {
      emit stateChanged(true);
    }
}

void PiiGridSelector::assignValueToPoint(double value, int layer, int pointIndex)
{
  bool state = false;

  int y = pointIndex / _gridSize.width();
  int x = pointIndex % _gridSize.width();

  if (_pCurrentLayer==0)
    return;
  if (layer >= 0 && layer<_layers.size())
    {
      if ( _layers[layer]->valueAt(x,y) != value)
        state = true;
      _layers[layer]->setValueAt(x,y,value);
    }

  update();
  reselectCells();
  if (state)
    emit stateChanged(true);
}

void PiiGridSelector::increaseValueToSelected()
{
  bool state = false;
  bool multivalue = false;
  bool first_sel_found = false;
  double selvalue = 0.0, value = 0.0;

  if(_pCurrentLayer==0)
    return;
  for(int y=0;y<_gridSize.height();y++)
    for(int x=0;x<_gridSize.width();x++)
      {
        if(_grid[y][x]->isSelected())
          {
            if ( !first_sel_found )
              {
                first_sel_found = true;
                selvalue = _pCurrentLayer->valueAt(x,y);
              }
            else
              {
                if ( selvalue != _pCurrentLayer->valueAt(x,y) )
                  multivalue = true;
              }

            //qDebug("_pCurrentLayer->increaseValue(%i,%i)", x,y);
            _pCurrentLayer->increaseValueAt(x,y);
            value = _pCurrentLayer->valueAt(x,y);
            //qDebug("value = _pCurrentLayer->valueAt(%i,%i) ==
            //%f", x,y, value);
            state = true;
                  
          }
      }

  if ( !multivalue )
    {
      //qDebug("!multivalue");
      //qDebug("value == %f", value);
      _pCurrentLayer->info()->setEditorValue(value);
    }
  else
    {
      //qDebug("multivalue");
      _pCurrentLayer->info()->unselectEditor();
      reselectCells();
    }

  //qDebug("update");
  update();
  //qDebug("updated");
  if ( state )
    {
      emit stateChanged(true);
    }
}

void PiiGridSelector::decreaseValueToSelected()
{
  //qDebug("PiiGridSelector::decreaseValueToSelected...");
  bool state = false;
  bool multivalue = false;
  bool first_sel_found = false;
  double selvalue = 0.0, value = 0.0;

  if(_pCurrentLayer==0)
    return;
  for(int y=0;y<_gridSize.height();y++)
    for(int x=0;x<_gridSize.width();x++)
      {
        if(_grid[y][x]->isSelected())
          {
            if ( !first_sel_found )
              {
                first_sel_found = true;
                selvalue = _pCurrentLayer->valueAt(x,y);
              }
            else
              {
                if ( selvalue != _pCurrentLayer->valueAt(x,y) )
                  multivalue = true;
              }
                
            _pCurrentLayer->decreaseValueAt(x,y);
            value = _pCurrentLayer->valueAt(x,y);
            state = true;
          }
      }

  if ( !multivalue )
    {
      _pCurrentLayer->info()->setEditorValue(value);
    }
  else
    {
      _pCurrentLayer->info()->unselectEditor();
      reselectCells();
    }

  update();
  if ( state )
    {
      emit stateChanged(true);
    }
}

void PiiGridSelector::setScrollArea(PiiScrollArea* scrollarea)
{
  _pScrollarea = scrollarea;

  if(_pScrollarea != 0)
    {
      if(_pScrollarea->aspectRatioMode() == Qt::KeepAspectRatioByExpanding)
        setFillView();
      else if(_pScrollarea->aspectRatioMode() == Qt::KeepAspectRatio)
        setFitToView();
      else setNoFit();
    }
}

void PiiGridSelector::updateView()
{
  updateGridDisplays();
  update();
}

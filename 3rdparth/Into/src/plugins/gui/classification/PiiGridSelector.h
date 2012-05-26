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

#ifndef _PIIGRIDSELECTOR_H
#define _PIIGRIDSELECTOR_H

#include <QRect>
#include <QList>
#include <QPointer>

#include "PiiVisualSelector.h"
#include "PiiMultiImage.h"
#include "PiiGridLayer.h"
/**
 * Selector that arranges images to grid. Each cell on the 
 * grid contains multiImage class that can store several 
 * images.
 **/
class PiiGridSelector : public PiiVisualSelector
{
  Q_OBJECT

  /**
   * Property, which defines the unit scale in the x and y directions.
   * It determines for both directions, how much a pixel is in units.
   * The default value is QSizeF(1.0, 1.0). The unit scale is needed
   * in some classifiers to scale the image.
   */
  Q_PROPERTY(QSizeF unitScale READ unitScale WRITE setUnitScale);

  /**
   * If this property is true, we will show the buffering properties
   * selection in the popup menu. The default value is false.
   */
  Q_PROPERTY(bool showBufferingProperties READ showBufferingProperties WRITE setShowBufferingProperties STORED false);

  Q_PROPERTY(QSize gridSize  READ gridSize    WRITE setGridSize);
  Q_PROPERTY(QSize slotSize  READ slotSize    WRITE setSlotSize);
  Q_PROPERTY(int gridSpacing READ gridSpacing WRITE setGridSpacing);

public:
  
  PiiGridSelector(int cols = 4,int rows = 5);

  ~PiiGridSelector();
  
  /**
   * Return number of images on given coordinate
   **/
  virtual int imageCount(int x, int y) const;
  
  /**
   * Sets a image number "index" in cell of given coordinate
   **/
  virtual void setShownImage(int x, int y, int index);
  
  /**
   * Returns all images on given coordinate
   **/
  QList<QImage*> images(double x, double y) const;

  /**
   * Returns a value of given coordinate in currently active layer
   **/
  virtual double value(double x,double y) const;

  /**
   * Returns a value given for cell that contains given image.
   * If layer is not given (=0) current layer is used.
   **/
  virtual double value(QImage* image,PiiSelectorLayer* layer=0) const;

  /**
   * returns a list of pointers to all PiiMultiImages that are currently selected
   **/
  virtual QList<PiiMultiImage*> selected() const;

  /**
   * Method for getting all images that are currenlty selected.
   **/ 
  virtual QList<QImage*> selectedImages() const;
  
  virtual QList<double> selectedValues() const;

  double firstValue() const;
  
  /**
   * RENAME!!!
   **/
  virtual QList< QList<PiiMultiImage*> > grid() const { return _grid;}
  
  QSize gridSize() const {return _gridSize;}
  void setGridSize(QSize val) ;
  
  QSize slotSize() const {return _slotSize;}
  void  setSlotSize(QSize val){_slotSize = val;}
  
  int  gridSpacing() const {return  _gridSpacing;}
  void setGridSpacing(int val){ _gridSpacing = val;}
  
  int cols() const { return _gridSize.width();}
  int rows() const { return _gridSize.height();}
  
  QList<PiiMultiImage*> cellsContaining(QPoint p) const;
  
  //  virtual bool event(QEvent *event);
  void mousePressEvent  ( QMouseEvent *e);
  void mouseReleaseEvent( QMouseEvent *e);
  void mouseMoveEvent   ( QMouseEvent *e);
  void mouseDoubleClickEvent(QMouseEvent * e );
  void wheelEvent( QWheelEvent * e );
  
  void paintEvent(QPaintEvent* e);
  
  // MOVE TO PiiPainter class
  void paintSquare(QPainter& p,QPoint p1,QPoint p2);
  void paintGradientButton(QPainter& p,QPoint loc,int radius,QColor color=Qt::green);
  QGradient createGradientBrush(QColor color, int radius);
  // moveto end
  
  void selectArea(QPoint p1, QPoint p2, bool do_unselect = true);
  void pasteCopyValue();
  void changeValue(PiiSelectorLayer* layer);
  
  /**
   * Overriding method for setting a value
   **/
  //virtual void assignValueToSelected(double value, int layer=-1);

  virtual void reselect();
  /**
   * Do reselection to all cells.
   **/
  void reselectCells();

  QPoint cellAt(QPoint) const ;

  virtual int heightForWidth (int w) const ;
  
  virtual void setScrollArea(PiiScrollArea* scrollarea);

  void setUnitScale(const QSizeF& unitScale);
  QSizeF unitScale() const { return _unitScale; }

  void setShowBufferingProperties(bool para) { _bShowBufferingProperties = para; }
  bool showBufferingProperties() { return _bShowBufferingProperties; }
  
protected:
  QList<QList<PiiMultiImage*> > _grid;
  
  QSize _gridSize;    // size of the grid, cols and rows
  QSize _slotSize;    // NOT USED!!!!

  /**
   * Cell in the grid that is under the current popup open
   * Used in several places.
   **/
  QPoint _popupCell;
  
  int   _cellSize;    // Size (width and height) of each cell on the grid
  int   _gridSpacing; // NOT USED!!!!
  int   _iCurrentX, _iCurrentY, _iStartX, _iStartY;     // For moving on the grid with arrows.
  double _copyValue;
  
  QFrame* _pImagePopup; // NOT USED!!!
  QFrame* _pPopup;      // menu
  
  bool   _bDragging;       
  bool   _bMultiselecting;
  bool   _bShowCellInfo;
  bool   _bShowCellValue;
  bool   _bShowOverlayColoring;
  // MOVED TO ACTION:bool   _fitToView;
  QPoint _mousePressed;
  QPoint _mouseCurrent;
  
  void createPopup();
  QWidget* createEditorWidget(int x, int y);
  QMenu* createMenu();
  
  void showContent(QPoint p1);
  void showContent(int x, int y);
  
  void keyPressEvent(QKeyEvent * e);

  virtual void resizeEvent ( QResizeEvent * event );

  QRect cellArea(int x, int y) const;

  /**
   * Check what values has been selected (on current layer) and
   * does emits.
   **/
  void checkSelectedValues();
  
  // ACTIONS
  QActionGroup* _pFitActionGroup;
  QAction* _pFitToViewAction;
  QAction* _pFillViewAction;
  QAction* _pNoFitAction;
  
  QAction* _pSelectAllAction;
  QAction* _pShowCellValuesAction;
  QAction* _pShowCellInfoAction;
  QAction* _pShowOverlayColoringAction;

  QSizeF _unitScale;

  bool _bShowBufferingProperties;
  
  //QAction* _pOpenOutputValuesAction;
signals:
  void stateChanged(bool);
  void bufferingActionTriggered();
  
public slots:

  void selectByClass(double value, int layer);
  
  //void componentSelected(PiiMultiImage* w, bool selected);
 
  virtual void addImage(QImage*, double x, double y, bool silent=false);
  virtual void addImage(QImage*, int x, int y, bool silent=false);
 
  virtual void removeImage(int index, int x, int y, bool silent=false);
  virtual void removeImages(int x, int y, bool silent=false);
  virtual void removeImages(double x, double y, bool silent=false);
  
  virtual void removeImage(QImage*, bool silent=false);
  virtual void removeAllImages();

  void selectXYArea(int sx, int sy, int cx, int cy);
  virtual void selectAll();
  virtual void unselectAll();
  virtual void invertSelection();
  virtual void invertSelectionAt(QPoint p1);
  virtual void invertSelectionAtArea(QPoint p1,QPoint p2);
  
  void showContentOnCurrentLocation();
  void showImagePopup();
  void showImagePopup(int x, int y);
  void nextImageOnSelected(); 
  void nextImageOnLocation(QPoint loc);
  void prevImageOnSelected();
  void prevImageOnLocation(QPoint loc);
  
  void cellInfoVisible(bool val);
  void cellValueVisible(bool val);
  
  // for changing fitting
  void setFitToView(bool val=true);
  void setFillView(bool val=true);
  void setNoFit();
  void updateFitMode();
  // void setAspectRatioMode(Qt::AspectRatioMode aspectMode);
  /**
   * Set cell size in pixels. Cells are allways square
   **/
  virtual void setCellSize(int size);
  void zoom(int factor);
  void zoomIn();
  void zoomOut();
  
  /**
   * Removes currently selected image on that coordinate
   **/
  virtual void removeImage(double x, double y, bool silent=false);
  
  void selectImagesContaining(QPoint p, int modifiers);
  void selectImagesIntersecting(QRect r, int modifiers);

  /**
   * whether or not cell information shall be seen.
   **/
  void setShowCellInfo(bool);
  
  void setShowOverlayColoring(bool);
  
  void updateGridDisplays();
  void updateView();
  
  void assignValueToPoint(double value, int layer, int pointIndex);
  
  //virtual void assignValueToSelected(int value, int layer=-1);
  virtual void assignValueToSelected(double value, int layer=-1);
  
  virtual void decreaseValueToSelected();
  virtual void increaseValueToSelected();
  
};
#endif //_PIIGRIDSELECTOR_H

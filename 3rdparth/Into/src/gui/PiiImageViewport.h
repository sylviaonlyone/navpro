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

#ifndef _PIIIMAGEVIEWPORT_H
#define _PIIIMAGEVIEWPORT_H

#include <QWidget>
#include <QThread>
#include <QMutex>
#include <PiiWaitCondition.h>

#include "PiiGui.h"

class QAction;
class QMenu;
class PiiImageOverlay;
class PiiRectangleOverlay;
class PiiImageViewport;

/**
 * @internal
 *
 * A thread that updates the scaled image.
 */
class PII_GUI_EXPORT PiiImageViewportUpdater : public QThread
{
  Q_OBJECT

public:
  PiiImageViewportUpdater(PiiImageViewport* parent);

  void startThread();
  void stop();
  void refresh();
  void setEnabled(bool enabled) { _bEnabled = enabled; }

signals:
  void imageReady();
  
protected:
  void run();

private:
  void updateImage();
  
  PiiImageViewport *_pParent;
  volatile bool _bRunning;
  volatile bool _bEnabled;
  PiiWaitCondition _updateCondition;
};

/**
 * A widget for viewing QImages.
 *
 * Main features:
 *
 * @li Zooming with keys + and -, and using mouse wheel.
 *
 * @li Mouse dragging (This requires that the widget is placed into a
 * PiiScrollArea, see #setScrollArea(PiiScrollArea*))
 *
 * @li Three states: Fit to view, Fill view and No fit (default
 * state).
 *
 * @ingroup Gui
 */
class PII_GUI_EXPORT PiiImageViewport : public QWidget
{
  Q_OBJECT

  /**
   * The fit mode of the image in the viewport. The default is @p
   * NoFit.
   */
  Q_PROPERTY(FitMode fitMode READ fitMode WRITE setFitMode);
  Q_ENUMS(FitMode);
  
  /**
   * The unit system for displaying distance and area measurements in
   * the image. See PiiUnitConverter for valid values. -1 (the
   * default) means no unit system. Measurements on the image are
   * shown in the selected unit system, to which pixels are scaled
   * with #pixelSize.
   */
  Q_PROPERTY(int unitSystem READ unitSystem WRITE setUnitSystem);

  /**
   * The physical size of a pixel in millimeters.
   */
  Q_PROPERTY(QSizeF pixelSize READ pixelSize WRITE setPixelSize);
   
  friend class PiiImageDisplay;
  friend class PiiImageViewportUpdater;

public:

  /**
   * Enumeration, which is used for determining the fit mode of the
   * image in the viewport. With the fit modes @p FitToView and @p
   * FillView the zoom factor is changed automatically, when the
   * image viewport is resized. Regardless of the fit mode, the aspect
   * ratio of the image remains the same. The following values are determined:
   *
   * @lip NoFit - there is no fit, the image is not rescaled,
   * when the image viewport is resized.
   *
   * @lip FitToView - when viewport  is resized, the image is rescaled
   * as big as possible inside the viewport widget area, preserving the
   * aspect ratio, and still fitting in inside the image viewport widget. 
   *
   * @lip FillView - when image is resized, the image is scaled as
   * small as possible outside the image viewport, filling the whole
   * image viewport area and preserving the aspect ratio. 
   */
  enum FitMode { NoFit = 0x00,
                 FitToView,
                 FillView };

  /**
   * Enumeration, which is used for determining the focus point for
   * example in zooming or resizing. Has the following possible values:
   *
   * @lip FocusPointMouseCursor - the image is focused to the mouse cursor.
   *
   * @lip FocusPointTopLeft - the image is focused to the top left
   * corner of the widget.
   *
   * @lip FocusPointTopLeft - the image is focused to the center
   * of the widget.
   *
   * @lip FocusPointTopLeft - the image is focused to the center
   * of the image.
   */
  enum FocusPolicy
    {
      FocusToMouseCursor = 0x00,
      FocusToWidgetTopLeft,
      //                      FocusToWidgetCenter,
      //                      FocusToImageCenter,
    };


  /**
   * PiiImageViewport constructor. Use this constructor, if you don't
   * want to use Qt's implicit memory sharing mechanism for the QImage
   * object, and you are sure that the QImage object, whose content is
   * shown on the image viewport, is stored somewhere else, when the
   * image is in the viewport. If these conditions are not fullfilled,
   * use the other constructor in stead.
   */
  PiiImageViewport(QImage* pImage, QWidget *parent = 0);

  /**
   * PiiImageViewport constructor. Use this version of the constructor,
   * if you want to use Qt's implicit memory sharing mechanism
   * for the QImage object. When using this constructor, you don't have
   * to care about references to the QImage object outside the
   * PiiImageViewport class.
   */
  PiiImageViewport(const QImage& image, QWidget *parent = 0);

  PiiImageViewport(QWidget *parent = 0);
  
  ~PiiImageViewport();

  /**
   * Adds overlay to the viewport. See the documentation of @p
   * PiiImageOverlay for more details about the overlays.
   */
  void addOverlay(PiiImageOverlay* overlay);
   
  /**
   * Changes current set of overlays with new one.
   * NOTE: ALL old overlays are removed.
   */
  void setOverlays(const QList<PiiImageOverlay*>& overlays);
   
  /**
   * Remove given overlay from image.
   * If parameter is 0, ALL overlays are removed
   */
  void removeOverlay(PiiImageOverlay* overlay=0);
  
  /**
   * Set current image on the viewport, resize the component.
   * Use this version of the function, if you don't
   * want to use Qt's implicit memory sharing mechanism for the QImage
   * object, and you are sure that the QImage object, whose content is
   * shown on the image viewport, is stored somewhere else, when the
   * image is in the viewport. If these conditions are not fullfilled,
   * use the other version of the setImage function in stead.   
   */
  void setImage(QImage* image, int layer = 0);
  
  /**
   * Works similarly than the other setImage function, except the memory
   * management works differently. Use this version of the function,
   * if you want to use Qt's implicit memory sharing mechanism
   * for the QImage object. When using this function, you don't have
   * to care about references to the QImage object outside the
   * PiiImageViewport class.
   */
  void setImage(const QImage& image, int layer = 0);

  void setOpacity(double opacity, int layer);
  double opacity(int layer) const;
  
  void setLayerVisible(bool visible, int layer);
  bool isLayerVisible(int layer) const;

  void addLayer(QImage* image = 0, qreal opacity = 1.0);
  void addLayer(const QImage& image, qreal opacity = 1.0);

  void removeLayer(int index);

  int layerCount() const;

  void setFitMode(FitMode mode);
  FitMode fitMode() const;
  
  /**
   * Returns pointer to current image on viewport
   */
  QImage* image(int layer = 0) const;

  //int heightForWidth ( int w ) const ;
  
  /**
   * Returns the zoom factor.
   */
  double zoom();

  void setPixelSize(QSizeF pixelSize);
  QSizeF pixelSize();

  void setUnitSystem(int unitSystem);
  int unitSystem();
  
signals:
  /**
   * Notify the location of mouse click on image coordinate system.
   *
   * @param p the clicked location, in image coordinates
   *
   * @param modifiers active keyboard modifiers
   */
  void clicked(QPoint p, int modifiers);

  /**
   * Emitted when the user has finished selecting a rectangular area
   * from the image. The coordinates are represented in image domain.
   *
   * @param r the selected area, in image coordinates
   *
   * @param modifiers active keyboard modifiers
   */
  void areaSelected(const QRect& r, int modifiers);

  /**
   * Emit signals pageUp, pageDown and esc when them buttons pressed.
   */
  void pageUpPressed();
  void pageDownPressed();
  void escPressed();
  
  /**
   * This signal is emitted, once the visible area compared to the
   * parent widget changes in this widget. The signal is emitted only
   * if the size of widget is greater than the size of the parent
   * widget. If the parent widget is QScrollArea (or class derived
   * from that), widget being greater than the parent widget means,
   * that there are scroll bars in the scroll area.
   *
   * @param x the x-coordinate of the left border of the new visible area.
   *
   * @param y the y-coordinate of the top border of the new visible
   * area.
   *
   * @param width the with of the new visible area.
   *
   * @param width the with of the new visible area.
   */
   void visibleAreaChanged(int x, int y, int width, int height);

public slots:
  /**
   * Requests a redraw of the image. This function is useful for
   * example if you modify the appearance of an overlay.
   */
  void updateImage();

  /**
   * This slot zooms in by the factor determined by the property @p
   * zoomStep (property not implemented yet).
   */
  void zoomIn();

  /**
   * This slot zooms out by the factor determined by the property @p
   * zoomStep (property not implemented yet).
   */
  void zoomOut();

  /**
   * This slot sets the zoom factor to 1.
   */
  void actualSize();

  /**
   * This slot sets the zoom factor as double. 1.0 is the image in the
   * actual size.
   */
  void setZoom(double val);

  /**
   * This slot sets the state of the image viewport to "Fit to View".
   */
  void fitToView();
  
  /**
   * This slot sets the state of the image viewport to "Fit to View" .
   * When "Fit to View" state is on, the image is scaled automatically
   * so that it fits entirely in the image viewport in the both directions,
   * keeping the original aspect ratio. The state remains, until the
   * image is zoomed in or zoomed out. In that case the state changes
   * to "No Fit". Resizing doesn't change the state.
   */
  void setFitToView(bool checked);

  /**
   * This slot sets the state of the image viewport to "Fill View" .
   * When "Fill View" state is on, the image is scaled automatically
   * so, that it takes all the room in the image viewport either in the
   * x or y direction, keeping the original aspect ratio. It means
   * that not necessarily everything of the image is shown in the
   * image viewport, because in one direction not everything is shown.
   * The state remains, until the image is zoomed in or zoomed out.
   * In that case the state changes to "No Fit". Resizing doesn't
   * change the state.
   */
  void setFillView(bool checked);

  /**
   * This slot sets overlay coloring on/off.
   */
  void setShowOverlayColoring(bool val);
  
protected:
  void keyPressEvent(QKeyEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void paintEvent(QPaintEvent* event);
  void resizeEvent(QResizeEvent* event);
  void wheelEvent(QWheelEvent* event);
  void showEvent(QShowEvent* event);
  void hideEvent(QHideEvent* event);

  /// @cond null
  struct Layer
  {
    Layer() : pImage(&image), dOpacity(1), bVisible(true) {}
    Layer(QImage* i, qreal o) : pImage(i != 0 ? i : &image), dOpacity(o), bVisible(true) {}
    Layer(const QImage& i, qreal o) : image(i), pImage(&image), dOpacity(o), bVisible(true) {}

    void setImage(QImage* im)
    {
      image = QImage();
      if (im != 0)
        pImage = im;
      else
        pImage = &image;
    }

    void setImage(const QImage& im)
    {
      image = im;
      pImage = &image;
    }
    
    /*
     * A (possibly shared) copy of the image, if implicit sharing is in
     * use. If the user gave the image as a pointer, this image will be
     * a null image.
     */
    QImage image;
    /*
     * A pointer to the image to be displayed. If the user gave the
     * image as a copy, or the pointer given by the user is zero, this
     * points to image. Otherwise it is the pointer given by the user.
     */
    QImage* pImage;
    qreal dOpacity;
    bool bVisible;
  };
  
  class Data
  {
  public:
    Data();
    virtual ~Data();
    
    /// Visible portion of the image, scaled to the correct size.
    QImage prescaledImage;
    /// The area of the original image prescaledImage represents.
    QRect prescaledArea;
    /// Protects against concurrent writes to pImage and prescaledImage.
    QMutex imageLock;

    // Image rect
    QRect imageRect;
    
    /**
     * How much the viewport is scaled
     */
    double dZoomFactor;
    
    /**
     * A Single zoom step. When zooming out, the current zoom factor is
     * divided by this zoom step. When zooming in, the zoom factor is
     * multiplied by the zoom step.
     */
    double dZoomStep;
    
    /**
     * Minimum value of the zoom factor.
     */
    double dMinZoomFactor;
    
    /**
     * Maximum value of the zoom factor.
     */
    double dMaxZoomFactor;
    
    QSizeF pixelSize;
    int iUnitSystem;
    
    // Horizontal and vertical scaling. Takes pixelSize into account.
    double dXScale, dYScale, dAspectRatio;
    
    // Contains the region (in pixels) of the original image (pixmap),
    // which will be painted on the screen.
    QRect visibleArea;
    
    // This member variable is used in focusing zooming to cursor. It
    // contains the widget position of the cursor, when the zooming is
    // done by mouse wheel and control key.
    QPoint mouseCurrPoint;
    
    /**
     * The location, in widget coordinates, where the mouse button has
     * been pressed down.
     */
    QPoint mousePressPoint;
    
    QAction *pFitToViewAction, *pFillViewAction, *pNoFitAction, *pZoomInAction, *pZoomOutAction, *pActualSizeAction, *pShowOverlayColoringAction;
    
    QList<PiiImageOverlay*> overlays;
    bool bShowOverlayColoring;
    
    // Contains the area, which will be selected by the mouse, by
    // dragging the left mouse button. If the mouse button is not
    // pressed, contains a null value (QRect()).
    QRect selectionArea;

    QList<Layer*> lstLayers;
    
    PiiImageViewportUpdater* pUpdater;
  } *d;

  PiiImageViewport(Data *d, QImage* pImage, QWidget *parent = 0);
  PiiImageViewport(Data *d, const QImage& image, QWidget *parent = 0);
  PiiImageViewport(Data *d, QWidget *parent = 0);
  /// @endcond
  
private:
  
  void init();
  QMenu *createPopupMenu();
  void checkFitMode();   
  void focusImage(PiiImageViewport::FocusPolicy focusPolicy = FocusToWidgetTopLeft);
  void setCurrX(int x);
  void setCurrY(int y);
  void moveCurrX(int dx);
  void moveCurrY(int dy);
  
  
  void updateZoomFactors();

  int xFromWidget2Image(int x) const;
  int yFromWidget2Image(int y) const;
  QPoint pointFromWidget2Image(const QPoint &pos) const;

  double widthFromWidget2Image(int width) const;
  double heightFromWidget2Image(int length) const;
  QRect rectFromWidget2Image(const QRect &rect) const;
  int xFromImage2Widget(int x) const;
  int yFromImage2Widget(int y) const;
  QPoint pointFromImage2Widget(const QPoint &pos) const;
  QRect rectFromImage2Widget(const QRect &rect) const;

  QString toolTipText(const QPoint&);
  QString toolTipTextForSelectionArea(const QPoint& imagePoint);

  QRect startRendering();
  void endRendering(QRect visibleArea);
};

#endif //_PIIIMAGEVIEWPORT_H

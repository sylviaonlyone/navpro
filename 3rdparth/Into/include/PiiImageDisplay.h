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

#ifndef _PIIIMAGEDISPLAY_H
#define _PIIIMAGEDISPLAY_H

#include <QAbstractScrollArea>
#include <QImage>

#include <PiiVariant.h>
#include <PiiMatrix.h>
#include <PiiGui.h>

class PiiImageViewport;
class QWidget;

/**
 * A class provides an image display window.
 *
 * The display automatically converts image colors to 8 bits per
 * channel. If the actual dynamic range of the color channels is
 * greater, one must scale the color appropriately before sending them
 * to the display. Floating point color channels are assumed to be at
 * the range [0,1] and scaled accordingly.
 *
 * @ingroup Gui
 */
class PII_GUI_EXPORT PiiImageDisplay : public QAbstractScrollArea
{
  Q_OBJECT

  /**
   * The display type. Color and gray-scale images are correctly shown
   * with the @p Normal type. To automatically scale images to the
   * full gray-scale range, set the displayType to @p AutoScale.
   */
  Q_PROPERTY(DisplayType displayType READ displayType WRITE setDisplayType);
  Q_ENUMS(DisplayType);

public:
  /**
   * Display types.
   *
   * @li @p Normal - images will be displayed without modification.
   *
   * @li @p AutoScale - gray-level images will be scaled to fill the
   * full 8-bit dynamic range. This is handy if you need to display
   * binary images or floating-point images that are not in the
   * correct scale.
   */
  enum DisplayType { Normal, AutoScale };

  PiiImageDisplay(QImage *image, QWidget* parent = 0);
  PiiImageDisplay(const QImage& image, QWidget* parent = 0);
  PiiImageDisplay(QWidget *parent = 0);
  
  ~PiiImageDisplay();
  
  void setDisplayType( DisplayType type );
  DisplayType displayType();

  PiiImageViewport* imageViewport() const;
  
protected:
  void mouseMoveEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void resizeEvent(QResizeEvent* event);
  void wheelEvent(QWheelEvent *event);
  void scrollContentsBy ( int dx, int dy );


  /// @internal
  class Data
  {
  public:
    Data();
    virtual ~Data();

    QList<QImage*> lstImages;
    
    PiiImageViewport* pImageViewport;
    
    // A flag, which determines, if the image display can be moved, if
    // the values of the scrollbars changes.
    bool bCanMoveDisplay;
    
    // Contains the dragging (moving the scrolls with the middle mouse
    // button) state of the display.
    bool bDragging;
    
    // This member variable is used in dragging functionality (moving
    // the image when the mouse middle button is pressed.
    QPoint mouseCurrPoint;
    
    // The display type.
    DisplayType displayType;
  } *d;

  /// @internal
  PiiImageDisplay(Data *d, QImage *image, QWidget* parent = 0);
  /// @internal
  PiiImageDisplay(Data *d, const QImage& image, QWidget* parent = 0);
  /// @internal
  PiiImageDisplay(Data *d, QWidget *parent = 0);
  
signals:
  /**
   * Refer to the documentation of the corresponding signal in the class
   * PiiImageViewport. 
   */
  void areaSelected(const QRect&,int);

  /**
   * Refer to the documentation of the corresponding signal in the class
   * PiiImageViewport. 
   */
  void clicked(const QPoint&,int);

public slots:
  /**
   * Sets the displayed image. If @p image is not of a recognized
   * type, this function does nothing.
   */
  void setImage(const PiiVariant& image, int layer = 0);

protected slots:
  void visibleAreaChanged(int x, int y, int width, int height);

private:
  template <class T> void grayImage(const PiiVariant& obj, int layer);
  template <class T> void floatImage(const PiiVariant& obj, int layer);
  template <class T> void colorImage(const PiiVariant& obj, int layer);
  template <class T> void scaledImage(const PiiMatrix<T>& image, int layer);
  template <class T> void view(const PiiMatrix<T>& image, int layer);
  
  /**
   * Initialize a viewport widget and variables.
   */
  void initViewport();
  
};

#endif //_PIIIMAGEDISPLAY_H

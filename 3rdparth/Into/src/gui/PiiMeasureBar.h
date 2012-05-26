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

#ifndef _PIIMEASUREBAR_H
#define _PIIMEASUREBAR_H

#include <QFrame>
#include <QVector>
#include <PiiUnitConverter.h>
#include <QStringList>
#include <QVariant>

#include "PiiGui.h"

class QRect;

/**
 * This widget provides a measuring tape for other widgets, like
 * PiiImageDisplay and PiiDefectMap.
 *
 * @ingroup Gui
 */
class PII_GUI_EXPORT PiiMeasureBar : public QFrame
{
  Q_OBJECT

  /**
   * This property holds the orientation of the measure widget.
   *
   * The orientation must be Qt::Vertical (the default) or Qt::Horizontal
   */
  Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation);

  /**
   * TODO: REVIEW THIS COMMENT BLOCK.
   * Defines the logical pixel length as millimieters. In this class
   * only one dimension of the pixel size is needed. That's why its
   * type is double. The value of this property, together with the
   * scale factor set by the function setScale(),  affect to the scale,
   * how the units are shown in the measure bar. It depends also on
   * the property @p unitSystem in which units the coordinates are
   * ultimately shown for the user.
   *
   * There is also corresponding property @p pixelSize in the class @p
   * PiiDefectMapOperation, which affects in two dimensions. During
   * creation of this UI-component, the value of the property in this
   * class is initialized according to the value in the operation. And
   * each time the value of the property changes in the operation, it
   * is changed also in this class. However, in this class, it is
   * possible to overwrite the property.
   */
  // TODO: RENAME THIS TO pixelSize
  Q_PROPERTY(double pixelLength READ pixelLength WRITE setPixelLength);

  /**
   * TODO: COMPLETE THIS COMMENT BLOCK!!
   */
  Q_PROPERTY(QVariantList minTickDistances READ minTickDistances WRITE setMinTickDistances);

  /**
   * TODO: COMPLETE THIS COMMENT BLOCK!!
   */
  Q_PROPERTY(QVariantList tickScales READ tickScales WRITE setTickScales);

  /**
   * TODO: COMPLETE THIS COMMENT BLOCK!!
   */
  Q_PROPERTY(QStringList tickNames READ tickNames WRITE setTickNames);

  /**
   * This property determines, whether the units are drawn in the
   * measure bar after the unit numbers, or not. The default value is
   * @ true.
   */
  Q_PROPERTY(bool drawUnitNames READ drawUnitNames WRITE setDrawUnitNames);

  /**
   * TODO: COMPLETE THIS COMMENT BLOCK!!
   */
  Q_PROPERTY(DrawTicks drawTicks READ drawTicks WRITE setDrawTicks);
  
  /**
   * TODO: COMPLETE THIS COMMENT BLOCK!!
   */
  Q_PROPERTY(QBrush brush READ brush WRITE setBrush);
  
  Q_ENUMS(UnitSystem);
  Q_ENUMS(DrawTicks);
  
public:
  /**
   * An enumeration used in determinining the unit system used in
   * measuer widget. The following values are determined: 
   *
   * @lip Metric - metric system (m, mm)
   *
   * @lip Imperial - imperial system (inches, feet)
   *
   * This enumeration is identical to the enumeration defined in class
   * #PiiUnitConverter. The reasons, why we cannot use that enumeration
   * directly, are the limitations in Qt and moc.
   */
  enum UnitSystem { Metric = PiiUnitConverter::Metric, Imperial = PiiUnitConverter::Imperial };
  
  /**
   * Enumeration, which determines the side, where the scale is drawn.
   * The following values are possible:
   *
   * @lip UndefinedDrawSide - the scale side drawn is undefined.
   *
   * @lip DrawTopOrRight - the scale is drawn either in the right or
   * the top side of the measure bar, depengind on the orientation of the
   * measure bar.
   *
   * @lip DrawBottomOrLeft - the scale is drawn either in the left or
   * the bottom side of the measure bar, depengind on the orientation of
   * the measure bar.
   *
   * @lip DrawBoth - the scale is drawn in both sides of the measure bar.
   *
   */
  enum DrawTicks
  {
    DrawNone = 0x00,
    DrawTopOrRight = 0x01,
    DrawBottomOrLeft = 0x02,
    DrawBoth = 0x03
  };

  
  PiiMeasureBar(QWidget* parent = 0);
  PiiMeasureBar(Qt::Orientation orientation, QWidget* parent = 0);
  ~PiiMeasureBar();

  void setOrientation(const Qt::Orientation& orientation);
  Qt::Orientation orientation() const;

  void setMinTickDistances(const QVariantList& distances);
  QVariantList minTickDistances() const;

  QVariantList tickScales() const;
  void setTickScales(const QVariantList& tickScales);

  void setTickNames(const QStringList& tickNames);
  QStringList tickNames() const;
  
  void setDrawUnitNames(bool status);
  bool drawUnitNames() const;

  DrawTicks drawTicks() const;
  void setDrawTicks(DrawTicks drawTicks);

  QBrush brush() const;
  void setBrush(const QBrush& brush);

public slots:
  /**
   * Sets the logicl pixel length either in the horizontal or vertical
   * direction, depending on the orientation of the measure bar.
   */
  void setPixelLength(double length);

  double pixelLength() const;
  
  /**
   * Sets the start location of the measure bar in pixels.
   */
  void setLocation(int location);

  /**
   * Sets the location of the mouse cursor in pixels. In general this
   * is the location from the tracking widget (in pixels) without any
   * scaling.
   */
  void setMouseLocation(const QPoint& location);

  /**
   * If this is true, we will draw mouse location.
   */
  void drawMouseLocation(bool para);
  
  /**
   * Sets the scale of the measure bar (in pixels). E.g. scale 1.0
   * means that a logical pixel in the measure bar corresponds to
   * a pixel on the screen.
   */
  void setScale(double scale);

protected:
  void paintEvent(QPaintEvent *event);
  
private:
  class Data
  {
  public:
    Data(Qt::Orientation);
    
    Qt::Orientation orientation;
    PiiMeasureBar::DrawTicks drawTicks;
    int iTop; // either in x or y direction
    QPoint mouseLocation;
    bool bDrawMouseLocation;
    double dScaleFactor;
    double dPixelLength;
  
    QVector<int> lstMinTickDistances;
    QVector<double> lstTickScales;
    QStringList lstTickNames;
    bool bDrawUnitNames;
  
    QVector<int> lstLineLengths;
    QVector<double> lstSteps;

    // Contains the index of the biggest scale.
    int iCurrBigScaleIndex;
    // Contains the index of the basic unit. Cannot contain an empty
    // string or unit starting with '-'- or '+'-sign. Has the 
    int iCurrUnitIndex;
    QBrush brush;
  } *d;
  
  void init();
  void updateLineLengths();
  void updateStepList();
  void updateUnitIndex();
};

#endif //_PIIMEASUREBAR_H

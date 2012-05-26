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

#ifndef PIITHERMOMETER_H
#define PIITHERMOMETER_H

#include <QWidget>
#include <QPen>
#include "PiiGui.h"

/**
 * A class provides a simple thermometer display.
 *
 * @ingroup Gui
 */
class PII_GUI_EXPORT PiiThermoMeter : public QWidget
{
  Q_OBJECT

  /**
   * Set the orientation of the thermometer. Default is Vertical.
   */
  Q_PROPERTY(Orientation orientation READ orientation WRITE setOrientation);
  Q_ENUMS(Orientation);

  /**
   * Set the scale position. Default is Left. If you don't want to see result values,
   * you can se scalePosition as None.
   */
  Q_PROPERTY(ScalePosition scalePosition READ scalePosition WRITE setScalePosition);
  Q_ENUMS(ScalePosition);

  /**
   * Set the maximum number of major and minor tick intervals.
   */
  Q_PROPERTY(int maxMajorTicks READ maxMajorTicks WRITE setmaxMajorTicks);
  Q_PROPERTY(int maxMinorTicks READ maxMinorTicks WRITE setmaxMinorTicks);

  /**
   * Change the width of the pipe.
   */
  Q_PROPERTY(int pipeWidth READ pipeWidth WRITE setPipeWidth);

  /**
   * Enable or disable the alarm threshold.
   */
  Q_PROPERTY(bool alarmEnabled READ alarmEnabled WRITE setAlarmEnabled);

  /**
   * Specify the alarm threshold.
   */
  Q_PROPERTY(double alarmThreshold READ alarmThreshold WRITE setAlarmThreshold);

  Q_PROPERTY(QBrush alarmBrush READ alarmBrush WRITE setAlarmBrush);
  Q_PROPERTY(QBrush backgroundBrush READ backgroundBrush WRITE setBackgroundBrush);
  Q_PROPERTY(QBrush liquidBrush READ liquidBrush WRITE setLiquidBrush);

  /**
   * Set a pen of the scales.
   */
  Q_PROPERTY(QPen scalePen READ scalePen WRITE setScalePen);

  /**
   * Set a pen of the pipe.
   */
  Q_PROPERTY(QPen pipePen READ pipePen WRITE setPipePen);

  /**
   * Set a precision of the scale numbers.
   */
  Q_PROPERTY(int scalePrecision READ scalePrecision WRITE setScalePrecision);

  /**
   * Set min and max values.
   */
  Q_PROPERTY(double min READ min WRITE setMin);
  Q_PROPERTY(double max READ max WRITE setMax);

public:

  PiiThermoMeter(QWidget *parent = 0);
  ~PiiThermoMeter();

  /**
   * Possible orientation.
   *
   * @li @p Horizontal -
   *
   * @li @p Vertical -
   */
  enum Orientation{Horizontal, Vertical};

  /**
   * Possible scale positions.
   *
   * @li @p Top -
   *
   * @li @p Bottom -
   *
   * @li @p Left -
   *
   * @li @p Right -
   *
   * @li @p None -
   */
  enum ScalePosition{Top, Bottom, Left, Right, None};

  Orientation orientation() const;
  void setOrientation(Orientation orientation);

  ScalePosition scalePosition() const;
  void setScalePosition(ScalePosition scalePosition);

  int maxMajorTicks() const;
  void setmaxMajorTicks(int maxMajorTicks);

  int maxMinorTicks() const;
  void setmaxMinorTicks(int maxMinorTicks);

  int pipeWidth() const;
  void setPipeWidth(int pipeWidth);

  bool alarmEnabled() const;
  void setAlarmEnabled(bool alarmEnabled);

  double alarmThreshold() const;
  void setAlarmThreshold(double alarmThreshold);

  QBrush alarmBrush() const;
  void setAlarmBrush(const QBrush& alarmBrush);

  QBrush backgroundBrush() const;
  void setBackgroundBrush(const QBrush& backgroundBrush);

  QBrush liquidBrush() const;
  void setLiquidBrush(const QBrush& liquidBrush);

  QPen scalePen() const;
  void setScalePen(const QPen& scalePen);

  QPen pipePen() const;
  void setPipePen(const QPen& pipePen);

  int scalePrecision() const;
  void setScalePrecision(int prec);

  double min() const;
  void setMin(double min);

  double max() const;
  void setMax(double max);

public slots:
  void setValue(double value);

protected:
  void paintEvent(QPaintEvent *e);

private:
  void updateValues();
  void calculatePipeValues(int *pipeX, int *pipeY, int *pipeWidth, int *pipeHeight,
                           int *liquidX, int *liquidY, int *liquidWidth, int *liquidHeight,
                           int *alarmX, int *alarmY, int *alarmWidth, int *alarmHeight,
                           int *scaleX1, int *scaleY1, int *scaleX2, int *scaleY2,
                           int *majorTickWidth, int *majorTickHeight, int *minorTickWidth, int *minorTickHeight,
                           int *textXOffset, int *textYOffset);

  class Data
  {
  public:
    Data();
    
    Orientation orientation;
    ScalePosition scalePosition;
    int iMaxMajorTicks, iMaxMinorTicks, iPipeWidth;
    bool bAlarmEnabled;
    QBrush alarmBrush, backgroundBrush, liquidBrush;
    QPen scalePen, pipePen;
    int iScalePrecision;
    double dAlarmThreshold, dMin, dMax, dValue;
    int iTextHeight, iTextWidth;
  } *d;
};

#endif // PIITHERMOMETER_H

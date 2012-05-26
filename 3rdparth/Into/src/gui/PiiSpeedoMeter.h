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

#ifndef PIISPEEDOMETER_H
#define PIISPEEDOMETER_H

#include <QWidget>
#include <QPen>
#include <QResizeEvent>

#include "PiiGui.h"

/**
 * A class provides a simple speedometer display.
 *
 * @ingroup Gui
 */
class PII_GUI_EXPORT PiiSpeedoMeter : public QWidget
{
  Q_OBJECT

  /**
   * Set the maximum number of major and minor tick intervals.
   */
  Q_PROPERTY(int maxMajorTicks READ maxMajorTicks WRITE setmaxMajorTicks);
  Q_PROPERTY(int maxMinorTicks READ maxMinorTicks WRITE setmaxMinorTicks);


  Q_PROPERTY(QBrush outerBorderBrush READ outerBorderBrush WRITE setOuterBorderBrush);
  Q_PROPERTY(QBrush innerBorderBrush READ innerBorderBrush WRITE setInnerBorderBrush);
  Q_PROPERTY(QBrush meterBrush READ meterBrush WRITE setMeterBrush);
  Q_PROPERTY(QBrush scaleBrush READ scaleBrush WRITE setScaleBrush);
  Q_PROPERTY(QBrush alarmBrush READ alarmBrush WRITE setAlarmBrush);
  Q_PROPERTY(QBrush needleBrush READ needleBrush WRITE setNeedleBrush);
  Q_PROPERTY(QBrush nailBrush READ nailBrush WRITE setNailBrush);
  Q_PROPERTY(QBrush valueBrush READ valueBrush WRITE setValueBrush);

  Q_PROPERTY(QPen outerBorderPen READ outerBorderPen WRITE setOuterBorderPen);
  Q_PROPERTY(QPen innerBorderPen READ innerBorderPen WRITE setInnerBorderPen);
  Q_PROPERTY(QPen meterPen READ meterPen WRITE setMeterPen);
  Q_PROPERTY(QPen scalePen READ scalePen WRITE setScalePen);
  Q_PROPERTY(QPen needlePen READ needlePen WRITE setNeedlePen);
  Q_PROPERTY(QPen nailPen READ nailPen WRITE setNailPen);
  Q_PROPERTY(QPen valuePen READ valuePen WRITE setValuePen);

  /**
   * Set a precision of the scale numbers.
   */
  Q_PROPERTY(int scalePrecision READ scalePrecision WRITE setScalePrecision);

  /**
   * Specify the alarm threshold.
   */
  Q_PROPERTY(double alarmThreshold READ alarmThreshold WRITE setAlarmThreshold);

  /**
   * Set min and max values.
   */
  Q_PROPERTY(double min READ min WRITE setMin);
  Q_PROPERTY(double max READ max WRITE setMax);

  /**
   * Set start and stop angle.
   */
  Q_PROPERTY(double startAngle READ startAngle WRITE setStartAngle);
  Q_PROPERTY(double stopAngle READ stopAngle WRITE setStopAngle);

  /**
   * The flags for the visibility. In default all flags are in use.
   */
  Q_PROPERTY(VisibilityFlags visibilityFlags READ visibilityFlags WRITE setVisibilityFlags);
  Q_FLAGS(VisibilityFlags);

  /**
   * Set a measurement type (QString).
   */
  Q_PROPERTY(QString measurementType READ measurementType WRITE setMeasurementType);

public:

  PiiSpeedoMeter(QWidget *parent = 0);
  ~PiiSpeedoMeter();

  /**
   * Visibility flags.
   *
   * @lip ShowMeasurementType - draw the type
   * @lip ShowValue - draw the value
   * @lip ShowBorders - draw the inner and outer borders
   * @lip ShowScaleBrush -
   * @lip ShowAlarmBrush -
   */
  enum VisibilityFlag { ShowMeasurementType = 1,
                        ShowValue = 2,
                        ShowBorders = 4,
                        ShowScaleBrush = 8,
                        ShowAlarmBrush = 16 };
  Q_DECLARE_FLAGS(VisibilityFlags, VisibilityFlag);
  
  void setVisibilityFlags(VisibilityFlags visibilityFlags) { _visibilityFlags = visibilityFlags; }
  VisibilityFlags visibilityFlags() const { return _visibilityFlags; }

  int maxMajorTicks() const { return _iMaxMajorTicks; }
  int maxMinorTicks() const { return _iMaxMinorTicks; }
  QBrush outerBorderBrush() const { return _outerBorderBrush; }
  QBrush innerBorderBrush() const { return _innerBorderBrush; }
  QBrush meterBrush() const { return _meterBrush; }
  QBrush scaleBrush() const { return _scaleBrush; }
  QBrush alarmBrush() const { return _alarmBrush; }
  QBrush needleBrush() const { return _needleBrush; }
  QBrush nailBrush() const { return _nailBrush; }
  QBrush valueBrush() const { return _valueBrush; }
  QPen outerBorderPen() const { return _outerBorderPen; }
  QPen innerBorderPen() const { return _innerBorderPen; }
  QPen scalePen() const { return _scalePen; }
  QPen meterPen() const { return _meterPen; }
  QPen needlePen() const { return _needlePen; }
  QPen nailPen() const { return _nailPen; }
  QPen valuePen() const { return _valuePen; }
  int scalePrecision() const { return _iScalePrecision; }
  double alarmThreshold() const { return _dAlarmThreshold; }
  double min() const { return _dMin; }
  double max() const { return _dMax; }
  double startAngle() const { return _dStartAngle; }
  double stopAngle() const { return _dStopAngle; }
  QString measurementType() const { return _strMeasurementType; }

  void setmaxMajorTicks(int maxMajorTicks) { _iMaxMajorTicks = qMax(2,maxMajorTicks); }
  void setmaxMinorTicks(int maxMinorTicks) { _iMaxMinorTicks = qMax(0,maxMinorTicks); }
  void setOuterBorderBrush(const QBrush& brush) { _outerBorderBrush = brush; }
  void setInnerBorderBrush(const QBrush& brush) { _innerBorderBrush = brush; }
  void setMeterBrush(const QBrush& meterBrush) { _meterBrush = meterBrush; }
  void setScaleBrush(const QBrush& scaleBrush) { _scaleBrush = scaleBrush; }
  void setAlarmBrush(const QBrush& alarmBrush) { _alarmBrush = alarmBrush; }
  void setNeedleBrush(const QBrush& needleBrush) { _needleBrush = needleBrush; }
  void setNailBrush(const QBrush& nailBrush) { _nailBrush = nailBrush; }
  void setValueBrush(const QBrush& valueBrush) { _valueBrush = valueBrush; }
  void setOuterBorderPen(const QPen& pen) { _outerBorderPen = pen; }
  void setInnerBorderPen(const QPen& pen) { _innerBorderPen = pen; }
  void setScalePen(const QPen& scalePen) { _scalePen = scalePen; }
  void setMeterPen(const QPen& meterPen) { _meterPen = meterPen; }
  void setNeedlePen(const QPen& needlePen) { _needlePen = needlePen; }
  void setNailPen(const QPen& nailPen) { _nailPen = nailPen; }
  void setValuePen(const QPen& valuePen) { _valuePen = valuePen; }
  void setScalePrecision(int prec) { _iScalePrecision = qMax(0,prec); updateValues(); }
  void setAlarmThreshold(double alarmThreshold) { _dAlarmThreshold = alarmThreshold; }
  void setMin(double min) { _dMin = min; }
  void setMax(double max) { _dMax = max; }
  void setStartAngle(double angle) { _dStartAngle = angle; }
  void setStopAngle(double angle) { _dStopAngle = angle; }
  void setMeasurementType(const QString& type) { _strMeasurementType = type; updateValues(); }

public slots:
  void setValue(double value);

protected:
  void paintEvent(QPaintEvent *e);
  void resizeEvent(QResizeEvent* e);

private:
  void updateValues();
  int calculateFontSize(double width, double height, const QString& text);

  VisibilityFlags _visibilityFlags;
  
  int _iMaxMajorTicks, _iMaxMinorTicks;
  QBrush _outerBorderBrush,_innerBorderBrush, _meterBrush, _scaleBrush, _alarmBrush, _needleBrush, _nailBrush, _valueBrush;
  QPen _outerBorderPen, _innerBorderPen, _meterPen, _scalePen, _needlePen, _nailPen, _valuePen;
  int _iScalePrecision;
  double _dAlarmThreshold, _dMin, _dMax, _dValue, _dStartAngle, _dStopAngle;

  QString _strMeasurementType;

  /// @internal
  class Data
  {
  public:
    Data();
    ~Data();

    double dValue;
    double dRadius;
    double dUnit;
    double dNeedleUnit;
    double dNailRadius;
    double dValueWidth;
    double dValueHeight;
    double dNumberWidth;
    double dNumberHeight;
    double dMeterCap;
    double dScaleCap;
    int iTypeFontSize;
    int iValueFontSize;
    int iNumberFontSize;
    QPointF center;
    QRectF rect;

  } *d;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(PiiSpeedoMeter::VisibilityFlags);

#endif // PIISPEEDOMETER_H

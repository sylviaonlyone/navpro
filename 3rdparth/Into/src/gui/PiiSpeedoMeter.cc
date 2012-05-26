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

#include "PiiSpeedoMeter.h"
#include <QPainter>
#include <QFontMetrics>
#include <QTransform>

PiiSpeedoMeter::Data::Data() :
   dValue(0),
   dRadius(50),
   dUnit(1),
   dNeedleUnit(0.8),
   dNailRadius(15),
   dValueWidth(20),
   dValueHeight(10),
   dNumberWidth(15),
   dNumberHeight(10),
   dMeterCap(4),
   dScaleCap(5),
   iTypeFontSize(8),
   iValueFontSize(8),
   iNumberFontSize(5),
   center(25,25),
   rect(0,0,50,50)
{
}


PiiSpeedoMeter::Data::~Data()
{
}

PiiSpeedoMeter::PiiSpeedoMeter(QWidget *parent) : QWidget(parent),
                                                  _visibilityFlags(31),
                                                  _iMaxMajorTicks(12),
                                                  _iMaxMinorTicks(9),
                                                  _outerBorderBrush(Qt::gray),
                                                  _innerBorderBrush(Qt::gray),
                                                  _meterBrush(QColor(230,230,230)),
                                                  _scaleBrush(QColor(0,255,0)),
                                                  _alarmBrush(QColor(255,0,0)),
                                                  _needleBrush(QColor(0,0,255)),
                                                  _nailBrush(Qt::gray),
                                                  _valueBrush(Qt::white),
                                                  _outerBorderPen(Qt::NoPen),
                                                  _innerBorderPen(Qt::NoPen),
                                                  _meterPen(Qt::black),
                                                  _scalePen(Qt::black),
                                                  _needlePen(Qt::NoPen),
                                                  _nailPen(Qt::NoPen),
                                                  _valuePen(Qt::black),
                                                  _iScalePrecision(0),
                                                  _dAlarmThreshold(80),
                                                  _dMin(0),
                                                  _dMax(220),
                                                  _dStartAngle(-40),
                                                  _dStopAngle(220),
                                                  _strMeasurementType(tr("km/h")),
                                                  d(new Data)
{
    QLinearGradient outerBorderGradient(0, 0, 0, 1);
    outerBorderGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    outerBorderGradient.setColorAt(0, Qt::white);
    outerBorderGradient.setColorAt(1, Qt::black);
    setOuterBorderBrush(QBrush(outerBorderGradient));

    QLinearGradient innerBorderGradient(1, 0, 0, 1);
    innerBorderGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    innerBorderGradient.setColorAt(0, Qt::black);
    innerBorderGradient.setColorAt(1, Qt::white);
    setInnerBorderBrush(QBrush(innerBorderGradient));

    QRadialGradient nailGradient(QPointF(0.5, 0.5), 0.5);
    nailGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    nailGradient.setColorAt(0, Qt::white);
    nailGradient.setColorAt(1, Qt::darkGray);
    setNailBrush(QBrush(nailGradient));

    updateValues();
}

PiiSpeedoMeter::~PiiSpeedoMeter()
{
    delete d;
}

void PiiSpeedoMeter::updateValues()
{
    double w = width();
    double h = height();

    d->dRadius = qMin(w/2, h/2) - 2;
    d->dUnit = (double)(d->dRadius) / 50.0;
    d->center = rect().center();
    d->rect = QRectF(d->center.x() - d->dRadius, d->center.y() - d->dRadius, 2*d->dRadius, 2*d->dRadius);
    d->dNeedleUnit = 0.8*d->dUnit;
    d->dNailRadius = 5*d->dUnit;
    d->dValueWidth = 20*d->dUnit;
    d->dValueHeight = 10*d->dUnit;
    d->dNumberWidth = 12*d->dUnit;
    d->dNumberHeight = 7*d->dUnit;
    d->dMeterCap = 4*d->dUnit;
    d->dScaleCap = 5*d->dUnit;

    // Check the size of the value and number fonts
    QString strText = QString::number(_dMax, 'f', _iScalePrecision);
    d->iTypeFontSize = calculateFontSize(d->dValueWidth, d->dValueHeight, _strMeasurementType);
    d->iValueFontSize = calculateFontSize(d->dValueWidth, d->dValueHeight, strText);
    d->iNumberFontSize = calculateFontSize(d->dNumberWidth, d->dNumberHeight, strText);
}

int PiiSpeedoMeter::calculateFontSize(double width, double height, const QString& text)
{
    QFont f = font();
    f.setPixelSize(1);
    int iPixelSize = 1;
    while(true)
    {
        QFontMetrics fm(f);
        if (fm.width(text) > width || fm.height() > height)
            break;
        else
        {
            iPixelSize++;
            f.setPixelSize(iPixelSize);
        }
    }

    return qMax(1,iPixelSize-2);
}

void PiiSpeedoMeter::setValue(double value)
{
    _dValue = value;
    update();
}

void PiiSpeedoMeter::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    updateValues();
    update();
}

void PiiSpeedoMeter::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Draw borders if necessary
    if (_visibilityFlags & ShowBorders)
      {
        p.setPen(_outerBorderPen);
        p.setBrush(_outerBorderBrush);
        p.drawEllipse(d->rect);
        
        double dInnerBorderAdjust = d->dMeterCap/2;
        p.setPen(_innerBorderPen);
        p.setBrush(_innerBorderBrush);
        p.drawEllipse(d->rect.adjusted(dInnerBorderAdjust,dInnerBorderAdjust,-dInnerBorderAdjust,-dInnerBorderAdjust));
      }
    
    // Draw meter circle
    p.setPen(_meterPen);
    p.setBrush(_meterBrush);
    p.drawEllipse(d->rect.adjusted(d->dMeterCap,d->dMeterCap,-d->dMeterCap,-d->dMeterCap));

    double dSweepLength = _dStopAngle-_dStartAngle;

    double dAlarmThresholdFactor = (_dAlarmThreshold-_dMin) / (_dMax - _dMin);
    double dScaleStartAngle = (180-_dStartAngle);
    double dScaleSweepLength = dAlarmThresholdFactor*dSweepLength;

    double dAlarmSweepLength = dSweepLength - dScaleSweepLength;
    double dAlarmStartAngle = dScaleStartAngle - dScaleSweepLength;


    // Draw scale and alarm backgrounds
    QPainterPath path(d->center);
    if (_visibilityFlags & ShowScaleBrush)
      {
        path.arcTo(d->rect.adjusted(d->dScaleCap,d->dScaleCap,-d->dScaleCap,-d->dScaleCap),
                   dScaleStartAngle, -dScaleSweepLength);
        path.arcTo(d->rect.adjusted(2*d->dScaleCap, 2*d->dScaleCap, -2*d->dScaleCap, -2*d->dScaleCap),
                   dScaleStartAngle-dScaleSweepLength, dScaleSweepLength);
        path.closeSubpath();
        p.setPen(Qt::NoPen);
        p.setBrush(_scaleBrush);
        p.drawPath(path);
      }

    if (_visibilityFlags & ShowAlarmBrush)
      {
        path = QPainterPath(d->center);
        path.arcTo(d->rect.adjusted(d->dScaleCap,d->dScaleCap,-d->dScaleCap,-d->dScaleCap),
                   dAlarmStartAngle, -dAlarmSweepLength);
        path.arcTo(d->rect.adjusted(2*d->dScaleCap,2*d->dScaleCap,-2*d->dScaleCap,-2*d->dScaleCap),
                   dAlarmStartAngle-dAlarmSweepLength, dAlarmSweepLength);
        path.closeSubpath();
        p.setPen(Qt::NoPen);
        p.setBrush(_alarmBrush);
        p.drawPath(path);
      }
    
    // Draw scale ticks and numbers
    p.setPen(_scalePen);
    p.setBrush(Qt::NoBrush);
    p.drawArc(d->rect.adjusted(d->dScaleCap,d->dScaleCap,-d->dScaleCap,-d->dScaleCap),
              (180-_dStartAngle)*16, -(_dStopAngle-_dStartAngle)*16);

    double dMajorStep = dSweepLength / double(_iMaxMajorTicks-1);
    double dMinorStep = dMajorStep / double(_iMaxMinorTicks+1);
    double dNumberStep = (_dMax - _dMin) / double(_iMaxMajorTicks-1);

    double dRotatedAngle = _dStartAngle;

    QFont f = font();
    f.setPixelSize(d->iNumberFontSize);
    p.setFont(f);

    QTransform transform;
    transform.translate(d->center.x(), d->center.y());
    transform.rotate(_dStartAngle);
    p.setTransform(transform);

    for (int i=0; i<_iMaxMajorTicks; i++)
    {
        p.drawLine(-d->dRadius + d->dScaleCap,0,-d->dRadius + 2*d->dScaleCap,0);

        // Rotate numbers so that there are in horizontal
        int iTextTranslation = -d->dRadius + 2*d->dScaleCap+3 + d->dNumberWidth/2;
        transform.translate(iTextTranslation, 0);
        transform.rotate(-dRotatedAngle);
        p.setTransform(transform);

        double dNumber = _dMin + (double)i * dNumberStep;
        p.drawText(QRect(-d->dNumberWidth/2,-d->dNumberHeight/2, d->dNumberWidth, d->dNumberHeight),
                   Qt::AlignCenter, QString::number(dNumber,'f',_iScalePrecision));

        transform.rotate(dRotatedAngle);
        transform.translate(-iTextTranslation, 0);
        p.setTransform(transform);

        // Rotate one minor tick
        transform.rotate(dMinorStep);
        p.setTransform(transform);

        dRotatedAngle += dMinorStep;

        if (i < _iMaxMajorTicks-1)
        {
            for (int j=0; j<_iMaxMinorTicks; j++)
            {
                p.drawLine(-d->dRadius + d->dScaleCap,0,-d->dRadius + 1.2*d->dScaleCap,0);
                transform.rotate(dMinorStep);
                p.setTransform(transform);

                dRotatedAngle += dMinorStep;
            }
        }
    }
    transform.reset();
    p.setTransform(transform);

    // Draw measurement type and value
    p.setPen(_valuePen);
    p.setBrush(_valueBrush);

    QRect textRect(d->center.x()-d->dValueWidth/2, d->rect.y() + 0.63*d->rect.height(), d->dValueWidth, d->dValueHeight);

    if (_visibilityFlags & ShowMeasurementType)
      {
        f.setPixelSize(d->iTypeFontSize);
        p.setFont(f);
        QRect typeRect(d->center.x()-d->dValueWidth/2, d->rect.y() + 0.3*d->rect.height(), d->dValueWidth, d->dValueHeight);
        p.drawText(typeRect, Qt::AlignCenter, _strMeasurementType);
      }

    if (_visibilityFlags & ShowValue)
      {
        f.setPixelSize(d->iValueFontSize);
        p.setFont(f);
        p.drawRoundedRect(textRect, 3, 3);
        p.drawText(textRect, Qt::AlignCenter, QString::number(_dValue,'f',_iScalePrecision));
      }

    // Draw needle
    p.setPen(_needlePen);
    p.setBrush(_needleBrush);
    QPainterPath needlePath(QPointF(0,-2*d->dNeedleUnit));
    needlePath.lineTo(-d->dRadius + 4*d->dScaleCap,-d->dNeedleUnit);
    needlePath.lineTo(-d->dRadius + 4*d->dScaleCap,d->dNeedleUnit);
    needlePath.lineTo(0,2*d->dNeedleUnit);
    needlePath.lineTo(2*d->dNailRadius,d->dNeedleUnit);
    needlePath.lineTo(2*d->dNailRadius,-d->dNeedleUnit);
    needlePath.closeSubpath();

    transform.translate(d->center.x(), d->center.y());
    transform.rotate((_dValue-_dMin)/(_dMax-_dMin)*dSweepLength + _dStartAngle);
    p.setTransform(transform);
    p.drawPath(needlePath);

    transform.reset();
    p.setTransform(transform);


    // Draw nail
    p.setPen(_nailPen);
    p.setBrush(_nailBrush);
    p.drawEllipse(d->center, d->dNailRadius, d->dNailRadius);


    p.end();
}

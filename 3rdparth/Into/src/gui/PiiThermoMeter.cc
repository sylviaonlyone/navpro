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

#include "PiiThermoMeter.h"
#include <QPainter>
#include <QFontMetrics>

PiiThermoMeter::Data::Data() :
  orientation(Vertical),
  scalePosition(None),
  iMaxMajorTicks(10),
  iMaxMinorTicks(5),
  iPipeWidth(20),
  bAlarmEnabled(false),
  alarmBrush(Qt::red),
  backgroundBrush(Qt::gray),
  liquidBrush(Qt::blue),
  scalePen(Qt::gray),
  pipePen(Qt::black),
  iScalePrecision(1),
  dAlarmThreshold(80),
  dMin(0),
  dMax(100),
  dValue(0.0),
  iTextHeight(30),
  iTextWidth(100)
{
}

PiiThermoMeter::PiiThermoMeter(QWidget *parent) :
  QWidget(parent),
  d(new Data)
{
  updateValues();
}

PiiThermoMeter::~PiiThermoMeter()
{
  delete d;
}

void PiiThermoMeter::updateValues()
{
  QFontMetrics fm(font());
  d->iTextWidth = fm.width(QString::number(d->dMax, 'f', d->iScalePrecision));
  d->iTextHeight = fm.height();

  if (d->orientation == Horizontal)
    {
      setMinimumHeight(d->scalePosition == None ? d->iPipeWidth : d->iTextHeight + 20 + d->iPipeWidth);
      setMinimumWidth(50);
    }
  else
    {
      setMinimumWidth(d->scalePosition == None ? d->iPipeWidth : d->iTextWidth + 20 + d->iPipeWidth);
      setMinimumHeight(50);
    }


}

void PiiThermoMeter::setValue(double value)
{
  d->dValue = value;
  update();
}

void PiiThermoMeter::calculatePipeValues(int *pipeX, int *pipeY, int *pipeWidth, int *pipeHeight,
                                         int *liquidX, int *liquidY, int *liquidWidth, int *liquidHeight,
                                         int *alarmX, int *alarmY, int *alarmWidth, int *alarmHeight,
                                         int *scaleX1, int *scaleY1, int *scaleX2, int *scaleY2,
                                         int *majorTickWidth, int *majorTickHeight, int *minorTickWidth, int *minorTickHeight,
                                         int *textXOffset, int *textYOffset)
{
  int w = width();
  int h = height();
  int iMajorTickLength= 8;
  int iMinorTickLength = 4;
  int iScaleCap = 10;

  double dValueFactor = 1.0;
  if (d->dMin < d->dMax)
    dValueFactor = (d->dValue - d->dMin) / (d->dMax - d->dMin);

  double dAlarmFactor = 1.0;
  if (d->bAlarmEnabled && d->dMin < d->dMax)
    dAlarmFactor = (d->dAlarmThreshold - d->dMin) / (d->dMax - d->dMin);

  if (d->orientation == Vertical)
    {
      int iMaxWidth = d->scalePosition == None ? d->iPipeWidth : d->iTextWidth + iScaleCap*2 + d->iPipeWidth;
      *pipeX = qMax(0,w/2-iMaxWidth/2);
      *pipeY = d->iTextHeight / 2;
      *pipeWidth = d->iPipeWidth;
      *pipeHeight = h-d->iTextHeight;
      *scaleY1 = *pipeY;
      *scaleY2 = *pipeY + *pipeHeight;
      *majorTickWidth = iMajorTickLength;
      *minorTickWidth = iMinorTickLength;
      *majorTickHeight = *minorTickHeight = 0;
      *textYOffset = -*pipeY;
      *textXOffset = iScaleCap;

      switch(d->scalePosition)
        {
        case None:
          *scaleX1 = *scaleX2 = *pipeX;
          break;
        case Left:
          *pipeX += d->iTextWidth + iScaleCap * 2 - 1;
          *scaleX1 = *scaleX2 = *pipeX - iScaleCap;
          *majorTickWidth = -iMajorTickLength;
          *minorTickWidth = -iMinorTickLength;
          *textXOffset = -d->iTextWidth - iScaleCap;
          break;
        default:
          *scaleX1 = *scaleX2 = *pipeX + d->iPipeWidth + iScaleCap;
          break;
        }

      *liquidX = *pipeX;
      *liquidWidth = *pipeWidth;
      *liquidHeight = static_cast<int>((h-d->iTextHeight)*dValueFactor);
      *liquidY = h - *liquidHeight - *pipeY;

      int iAlarmThresholdY = h - static_cast<int>((h-d->iTextHeight)*dAlarmFactor) - *pipeY;
      *alarmX = *pipeX;
      *alarmWidth = *pipeWidth;

      if (dValueFactor > dAlarmFactor)
        {
          *alarmY = *liquidY;
          *alarmHeight = iAlarmThresholdY - *liquidY;

          *liquidY = iAlarmThresholdY;
          *liquidHeight -= *alarmHeight;
        }
      else
        {
          *alarmY = iAlarmThresholdY;
          *alarmHeight = 1;
        }
    }
  else
    {
      int iMaxHeight = d->scalePosition == None ? d->iPipeWidth : d->iTextHeight + iScaleCap*2 + d->iPipeWidth;
      *pipeY = qMax(0,h/2-iMaxHeight/2);
      *pipeX = d->iTextWidth / 2;
      *pipeWidth = w-d->iTextWidth;
      *pipeHeight = d->iPipeWidth;
      *scaleX1 = *pipeX;
      *scaleX2 = *pipeX + *pipeWidth;
      *majorTickHeight = iMajorTickLength;
      *minorTickHeight = iMinorTickLength;
      *majorTickWidth = *minorTickWidth = 0;
      *textXOffset = -(*pipeX);
      *textYOffset = iScaleCap;

      switch(d->scalePosition)
        {
        case None:
          *scaleY1 = *scaleY2 = *pipeY;
          break;
        case Top:
          *pipeY += d->iTextHeight + iScaleCap*2 - 1;
          *scaleY1 = *scaleY2 = *pipeY - iScaleCap;
          *majorTickHeight = -iMajorTickLength;
          *minorTickHeight = -iMinorTickLength;
          *textYOffset = -d->iTextHeight - iScaleCap;
          break;
        default:
          *scaleY1 = *scaleY2 = *pipeY + d->iPipeWidth + iScaleCap;
          break;
        }

      *liquidX = *pipeX;
      *liquidY = *pipeY;
      *liquidWidth = static_cast<int>((w-d->iTextWidth)*dValueFactor);
      *liquidHeight = *pipeHeight;

      int iAlarmStartX = static_cast<int>((w-d->iTextWidth)*dAlarmFactor);
      *alarmX = iAlarmStartX + *pipeX;
      *alarmY = *pipeY;
      *alarmHeight = *pipeHeight;

      if (dValueFactor > dAlarmFactor)
        {
          *alarmWidth = *liquidWidth - iAlarmStartX;
          *liquidWidth -= *alarmWidth;
        }
      else
        {
          *alarmWidth = 1;
        }
    }
}

void PiiThermoMeter::paintEvent(QPaintEvent *e)
{
  QWidget::paintEvent(e);

  int iPipeX, iPipeY, iPipeWidth, iPipeHeight,
    iLiquidX, iLiquidY, iLiquidWidth, iLiquidHeight,
    iAlarmX, iAlarmY, iAlarmWidth, iAlarmHeight,
    iScaleX1, iScaleY1, iScaleX2, iScaleY2,
    iMajorTickWidth, iMajorTickHeight, iMinorTickWidth, iMinorTickHeight,
    iTextXOffset, iTextYOffset;

  calculatePipeValues(&iPipeX, &iPipeY, &iPipeWidth, &iPipeHeight,
                      &iLiquidX, &iLiquidY, &iLiquidWidth, &iLiquidHeight,
                      &iAlarmX, &iAlarmY, &iAlarmWidth, &iAlarmHeight,
                      &iScaleX1, &iScaleY1, &iScaleX2, &iScaleY2,
                      &iMajorTickWidth, &iMajorTickHeight, &iMinorTickWidth, &iMinorTickHeight,
                      &iTextXOffset, &iTextYOffset);

  QPainter p(this);

  // Draw background
  p.fillRect(iPipeX, iPipeY, iPipeWidth, iPipeHeight, d->backgroundBrush);

  // Draw liquid (and alarm liquid if necessary)
  p.fillRect(iLiquidX, iLiquidY, iLiquidWidth, iLiquidHeight, d->liquidBrush);
  if (d->bAlarmEnabled)
    p.fillRect(iAlarmX, iAlarmY, iAlarmWidth, iAlarmHeight, d->alarmBrush);

  // Draw borders of the pipe
  p.drawRect(iPipeX, iPipeY, iPipeWidth, iPipeHeight);

  // Draw scales
  if (d->scalePosition != None)
    {
      double dMajorLength = d->orientation == Vertical ? double(iPipeHeight) : double(iPipeWidth);
      double dMajorStep = dMajorLength / double(d->iMaxMajorTicks-1);
      double dMinorStep = dMajorStep / double(d->iMaxMinorTicks+1);
      double dNumberStep = (d->dMax - d->dMin) / double(d->iMaxMajorTicks-1);

      // Draw scale line
      p.setPen(d->scalePen);
      p.drawLine(iScaleX1, iScaleY1, iScaleX2, iScaleY2);

      for (int i=0; i<d->iMaxMajorTicks; i++)
        {
          double dMajorAdd = (double)i * dMajorStep + 0.5;
          int iMajorX1 = d->orientation == Vertical ? iScaleX1 : iScaleX1 + dMajorAdd;
          int iMajorY1 = d->orientation == Horizontal ? iScaleY1 : iScaleY1 + dMajorAdd;

          int iMajorX2 = iMajorX1 + iMajorTickWidth;
          int iMajorY2 = iMajorY1 + iMajorTickHeight;

          p.drawLine(iMajorX1, iMajorY1, iMajorX2, iMajorY2);

          double dNumber = (double)i * dNumberStep;
          dNumber = d->orientation == Vertical ? d->dMax - dNumber : d->dMin + dNumber;

          p.drawText(QRect(iTextXOffset + iMajorX1, iTextYOffset + iMajorY1, d->iTextWidth, d->iTextHeight),Qt::AlignCenter, QString::number(dNumber,'f',d->iScalePrecision));

          if (i < d->iMaxMajorTicks-1)
            {
              for (int j=1; j<=d->iMaxMinorTicks; j++)
                {
                  double dMinorAdd = (double)j * dMinorStep + 0.5;
                  int iMinorX1 = d->orientation == Vertical ? iMajorX1 : iMajorX1 + dMinorAdd;
                  int iMinorY1 = d->orientation == Horizontal ? iMajorY1 : iMajorY1 + dMinorAdd;

                  int iMinorX2 = iMinorX1 + iMinorTickWidth;
                  int iMinorY2 = iMinorY1 + iMinorTickHeight;

                  p.drawLine(iMinorX1, iMinorY1, iMinorX2, iMinorY2);
                }
            }
        }
    }

  p.end();
}

PiiThermoMeter::Orientation PiiThermoMeter::orientation() const { return d->orientation; }
void PiiThermoMeter::setOrientation(Orientation orientation) { d->orientation = orientation; updateValues(); }
PiiThermoMeter::ScalePosition PiiThermoMeter::scalePosition() const { return d->scalePosition; }
void PiiThermoMeter::setScalePosition(ScalePosition scalePosition) { d->scalePosition = scalePosition; }
int PiiThermoMeter::maxMajorTicks() const { return d->iMaxMajorTicks; }
void PiiThermoMeter::setmaxMajorTicks(int maxMajorTicks) { d->iMaxMajorTicks = qMax(2,maxMajorTicks); }
int PiiThermoMeter::maxMinorTicks() const { return d->iMaxMinorTicks; }
void PiiThermoMeter::setmaxMinorTicks(int maxMinorTicks) { d->iMaxMinorTicks = qMax(0,maxMinorTicks); }
int PiiThermoMeter::pipeWidth() const { return d->iPipeWidth; }
void PiiThermoMeter::setPipeWidth(int pipeWidth) { d->iPipeWidth = pipeWidth; }
bool PiiThermoMeter::alarmEnabled() const { return d->bAlarmEnabled; }
void PiiThermoMeter::setAlarmEnabled(bool alarmEnabled) { d->bAlarmEnabled = alarmEnabled; }
double PiiThermoMeter::alarmThreshold() const { return d->dAlarmThreshold; }
void PiiThermoMeter::setAlarmThreshold(double alarmThreshold) { d->dAlarmThreshold = alarmThreshold; }
QBrush PiiThermoMeter::alarmBrush() const { return d->alarmBrush; }
void PiiThermoMeter::setAlarmBrush(const QBrush& alarmBrush) { d->alarmBrush = alarmBrush; }
QBrush PiiThermoMeter::backgroundBrush() const { return d->backgroundBrush; }
void PiiThermoMeter::setBackgroundBrush(const QBrush& backgroundBrush) { d->backgroundBrush = backgroundBrush; }
QBrush PiiThermoMeter::liquidBrush() const { return d->liquidBrush; }
void PiiThermoMeter::setLiquidBrush(const QBrush& liquidBrush) { d->liquidBrush = liquidBrush; }
QPen PiiThermoMeter::scalePen() const { return d->scalePen; }
void PiiThermoMeter::setScalePen(const QPen& scalePen) { d->scalePen = scalePen; }
QPen PiiThermoMeter::pipePen() const { return d->pipePen; }
void PiiThermoMeter::setPipePen(const QPen& pipePen) { d->pipePen = pipePen; }
int PiiThermoMeter::scalePrecision() const { return d->iScalePrecision; }
void PiiThermoMeter::setScalePrecision(int prec) { d->iScalePrecision = qMax(0,prec); updateValues(); }
double PiiThermoMeter::min() const { return d->dMin; }
void PiiThermoMeter::setMin(double min) { d->dMin = min; }
double PiiThermoMeter::max() const { return d->dMax; }
void PiiThermoMeter::setMax(double max) { d->dMax = max; }

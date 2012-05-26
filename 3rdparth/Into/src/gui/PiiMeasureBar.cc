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

#include "PiiMeasureBar.h"

#include <PiiMath.h>
#include <PiiUtil.h>

#include <math.h>

#include <QPainter>
#include <QPen>
#include <QRect>
#include <QRegExp>
#include <QtDebug>

PiiMeasureBar::Data::Data(Qt::Orientation o) :
  orientation(o)
{
}


PiiMeasureBar::PiiMeasureBar(QWidget* parent) :
  QFrame(parent),
  d(new Data(Qt::Vertical))
{
  init();
}

PiiMeasureBar::PiiMeasureBar(Qt::Orientation orientation, QWidget* parent) :
  QFrame(parent),
  d(new Data(orientation))
{
  init();
}

PiiMeasureBar::~PiiMeasureBar()
{
  delete d;
}

void PiiMeasureBar::init()
{
  d->dPixelLength = 1.0;
  d->dScaleFactor = 1.0;
  d->iCurrBigScaleIndex = -1;
  d->iCurrUnitIndex = -1;
  d->iTop = 0;
  d->mouseLocation = QPoint(0,0);
  d->bDrawMouseLocation = false;
  d->bDrawUnitNames = true;
  d->drawTicks = PiiMeasureBar::DrawBoth;
  
  setMinimumSize(24,24);
  QPalette pal = palette();
  pal.setColor(QPalette::Inactive, QPalette::Window, Qt::yellow);
  pal.setColor(QPalette::Normal, QPalette::Window, Qt::yellow);
  setPalette(pal);
  setBackgroundRole(QPalette::Window);
  setAutoFillBackground(true);

}

QVariantList PiiMeasureBar::minTickDistances() const
{
  return Pii::vectorToVariants<int>(d->lstMinTickDistances);
}

void PiiMeasureBar::setMinTickDistances(const QVariantList& minDistances)
{
  d->lstMinTickDistances = Pii::variantsToVector<int>(minDistances);
  updateLineLengths();
}

QVariantList PiiMeasureBar::tickScales() const
{
  return Pii::vectorToVariants<double>(d->lstTickScales);
}

void PiiMeasureBar::setTickScales(const QVariantList& tickScales)
{
  d->lstTickScales = Pii::variantsToVector<double>(tickScales);
}

void PiiMeasureBar::setTickNames(const QStringList& tickNames)
{
  d->lstTickNames = tickNames;
  update();
}

void PiiMeasureBar::setDrawUnitNames(bool status)
{
  d->bDrawUnitNames = status;
  update();
}

void PiiMeasureBar::setDrawTicks(DrawTicks drawTicks)
{
  d->drawTicks = drawTicks;
  update();
}

void PiiMeasureBar::setBrush(const QBrush& brush)
{
  d->brush = brush;
  if (brush.color().isValid())
    {
      QPalette pal = palette();
      pal.setColor(QPalette::Inactive, QPalette::Window, brush.color());
      pal.setColor(QPalette::Normal, QPalette::Window, brush.color());
      setPalette(pal);
      setBackgroundRole(QPalette::Window);
      setAutoFillBackground(true);
    }
  update();
}

void PiiMeasureBar::setPixelLength(double length)
{
  d->dPixelLength = length;
  update();  
}

void PiiMeasureBar::setLocation(int location)
{
  d->iTop = location + contentsRect().x();
  update();
}

void PiiMeasureBar::setMouseLocation(const QPoint& location)
{
  d->mouseLocation = location;
  update();
}

void PiiMeasureBar::setScale(double scale)
{
  d->dScaleFactor = scale;
  updateStepList();
  update();
}

/* This function updates the member variable d->lstLineLengths based on
   the content of the member variable d->lstMinTickDistances.
   d->lstLineLengths is filled with the numbers starting from 2 upwards
   with the interval of 2. The amount of numbers in the list will be
   equal to the amount of numbers in d->lstMinTickDistances. */
void PiiMeasureBar::updateLineLengths()
{
  d->lstLineLengths.clear();
  for (int i = 0; i<d->lstMinTickDistances.size(); i++)
    d->lstLineLengths << i*2+2;
}

/* This function updates the member variable d->lstSteps. */
void PiiMeasureBar::updateStepList()
{
  double realScale = d->dScaleFactor/d->dPixelLength;
  d->lstSteps.clear();
  // Initialize the big scale index to -1 meaning that it doesn't
  // contain any real value. If the new big scale that meet
  // the requirements, is not found (e.g. d->lstTickScales is empty) -1
  // is the final value.
  int tempBigScaleIndex = -1;
  
  // Find out the greatest scale
  for (int j = 0; j<d->lstTickScales.size(); j++)
    {
      if ((double(d->lstTickScales[j])*realScale > double(d->lstMinTickDistances[d->lstMinTickDistances.size()-1])) && (d->lstTickNames.isEmpty() || false == d->lstTickNames[j].isEmpty()))
        {
          //           qDebug("PiiMeasureBar::setScale() found greatest scale: %f", d->lstTickScales[j]);
//           qDebug("PiiMeasureBar::setScale()      d->lstTickScales[j] = %f", d->lstTickScales[j]);
//           qDebug("PiiMeasureBar::setScale()      realScale = %f", realScale);
//           qDebug("PiiMeasureBar::setScale()      double(d->lstTickScales[j])*realScale = %f", double(d->lstTickScales[j])*realScale);
          d->lstSteps << d->lstTickScales[j];
          tempBigScaleIndex = j;
//           qDebug() << "PiiMeasureBar::setScale()      d->lstTickNames[tempUnitIndex] = " << d->lstTickNames[tempUnitIndex];
          break;
        }
    }

  // If no scale fullfilled the requirement for the greatest scale,
  // take just the last one from the list.
  if (d->lstTickScales.size()>0 && d->lstSteps.isEmpty())
    {
      d->lstSteps << d->lstTickScales.last();
      tempBigScaleIndex = d->lstTickScales.size()-1;
    }

  // Find out the smaller ticks.

  for (int i=d->lstMinTickDistances.size()-2; i>=0; i--)
    {
      for (int j=0; j<d->lstTickScales.size(); j++)
        {
          //d->lstTickScales[j] = d->dPixelLength*d->lstSmallestStep[i]
          // Check if the tick scale fullfills the minimum pixel distance requirement.
          if (double(d->lstTickScales[j])*realScale > double(d->lstMinTickDistances[i]))
            {
              // Check, if all the bigger ticks scales are divisible with
              // this tick, and it is also smaller than all other tick
              // scales.
              bool bIsSmaller = true; //
                               //tells, if this tick is smaller than
                               //all previous ticks
              bool bIsDivisible = true; // 
                                        // tell, if this tick is
                                        // divisible with all stored steps.
              for (int k=0; k<d->lstSteps.size(); k++)
                {
                  if (d->lstTickScales[j] >= d->lstSteps[k])
                    {
                      // greater than equal than other tick, not valid.
                      bIsSmaller = false;
                      break;
                    }
                  double stepRatio = d->lstSteps[k]/d->lstTickScales[j];
                  if (false == Pii::almostEqualRel(floor(stepRatio)-stepRatio,0.0))
                    {
                      // step not divisible by greater step, not valid
                      bIsDivisible = false;
                      break;
                    }
                }
              // Because the ticks are ordered in d->lstTickScales, we
              // can stop searching for this round of j-loop
              if (bIsSmaller == false)
                break;
              if (bIsDivisible)
                {
                  d->lstSteps.insert(0, d->lstTickScales[j]);
                  break;
                }
              // If the tick wasn't divisible, we can still continue
              // the j-loop, until there is a match or the tick is too
              // big.
            }
        }
    }
  
  d->iCurrBigScaleIndex = tempBigScaleIndex;
  updateUnitIndex();
//   qDebug("PiiMeasureBar::updateStepList() END");
}

/* This function updates the member variable d->iUnitIndex. In the
   function it is assumed that the member variable d->iCurrBigScaleIndex
   has been correctly set.*/
void PiiMeasureBar::updateUnitIndex()
// void PiiMeasureBar::updateUnitIndex(int tempBigScaleIndex)
{
//   qDebug("PiiMeasureBar::updateUnitIndex() BEGIN, d->iCurrUnitIndex = %d", d->iCurrUnitIndex);
  // Negative value as a unit index means that
  // no unit will be shown.
  d->iCurrUnitIndex = -1;
  
  // Make sure that the big scale index is valid and initialized (not -1).
  if (d->iCurrBigScaleIndex<0 || d->iCurrBigScaleIndex >= d->lstTickNames.size())
    return;

  int tempUnitIndex = d->iCurrBigScaleIndex;
  
  // Find out the unit index.
  if (false == d->lstTickNames[tempUnitIndex].isEmpty())
    {
      if (d->lstTickNames[tempUnitIndex].startsWith('-'))
        {
          //'-' sign in front of the unit occured. Search the
          //corresponding unit without a sign going backward in the list.
          
          // Parse the '-' away from the unit name
          QString tempUnitName = d->lstTickNames[tempUnitIndex].mid(1);
          
          if (tempUnitName.length() > 0)
            {
              for (tempUnitIndex--; tempUnitIndex>=0; tempUnitIndex--)
                {
                  if (d->lstTickNames[tempUnitIndex] == tempUnitName)
                    {
                      d->iCurrUnitIndex = tempUnitIndex;
                      break;
                    }
                }
            }
        }
      else if (d->lstTickNames[tempUnitIndex].startsWith('+'))
        {
          //'+' sign in front of the unit occured. Search the
          //corresponding unit without a sign going forward in the list.
          
          // Parse the '+' away from the unit name
          QString tempUnitName = d->lstTickNames[tempUnitIndex].mid(1);
          if (tempUnitName.length() > 0)
            {
              for (tempUnitIndex++; tempUnitIndex<d->lstTickNames.size(); tempUnitIndex++)
                {
                  if (d->lstTickNames[tempUnitIndex] == tempUnitName)
                    {
                      d->iCurrUnitIndex = tempUnitIndex;
                      break;
                    }
                }
            }
        }
      else
        {
          // Here it is assumed that the unit name corresponding to
          // tempUnitIndex is not empty.
          d->iCurrUnitIndex = tempUnitIndex;
        }
    }
}

void PiiMeasureBar::paintEvent(QPaintEvent* event)
{
  QRect cRect = contentsRect();
  
  QPainter p;
  p.begin(this);
  p.save();

  QPen pen = QPen(Qt::SolidLine);
  pen.setWidth(1);
  pen.setColor(QColor(0, 0, 0));
  p.setPen(pen);
  p.setFont(QFont("Times", 8, QFont::Bold));

  int smallLines = 0;
  int barWidth;
  int barLength;
  
  double realScale = d->dScaleFactor/d->dPixelLength;
  if (d->orientation == Qt::Horizontal)
    {
      barWidth = cRect.height();
      barLength = width();

      if (d->lstSteps.size() > 0)
        smallLines = int(double(cRect.width())/(realScale*double(d->lstSteps[0])));
      // Else amount of small lines remains zero.
    }
  else // vertical
    {
      barWidth = cRect.width();
      barLength = height();
      if (d->lstSteps.size() > 0)
        smallLines = int(double(cRect.height())/(realScale*double(d->lstSteps[0])));
      // Else amount of small lines remains zero.
      p.translate(QPoint(width(),0));
      p.rotate(90.0);
    }

  // If the brush has a valid color, it has been set to the background
  // color of the widget in the function setBrush(), and it is not
  // drawn explicitely here. If the brush contains e.g. a gradient, it
  // is drawn here.
  if ( (d->brush.style() != Qt::NoBrush) && (false == d->brush.color().isValid()) )
    {
      p.setBrush(d->brush);
      p.drawRect(QRect(0, 0, barLength, barWidth));
    }

  if (smallLines > 0)
    {
      //int start = int((double(d->iTop)*d->dPixelLength/d->lstSteps[0]+1)*d->lstSteps[0]);
      int start = int((int(double(d->iTop)*d->dPixelLength/d->lstSteps[0]+1))*d->lstSteps[0]);
      double offset = d->lstSteps[0]-fmod(double(d->iTop)*d->dPixelLength,d->lstSteps[0]);

      for (int i = 0; i<smallLines; i++)
        {
          //       qDebug("PiiMeasureBar::paintEvent in first loop, i =  %d", i);
          int loc = int((double(i)*d->lstSteps[0]+offset)*realScale);
          bool bIsSmall = true;
          for (int j = 1; j<d->lstSteps.size(); j++)
            {
              //             qDebug("PiiMeasureBar::paintEvent in second loop, j =  %d", j);
              // Here it is checked, if the bigger step must be drawn. The
              // function almostEqualRel checks, wheter the result of the
              // function fmod() equals to zero.
              
              if (Pii::almostEqualRel(fmod(start+i*d->lstSteps[0], d->lstSteps[j]), 0.0))
                {
                  // Draw the line
                  bIsSmall = false;
                  // The linewidht of the frames are taken into account
                  // when drawing the lines.
                  if (d->drawTicks & DrawTopOrRight)
                    p.drawLine(loc, lineWidth(), loc, lineWidth()+d->lstLineLengths[j]);
                  if (d->drawTicks & DrawBottomOrLeft)
                    p.drawLine(loc, barWidth-d->lstLineLengths[j]-lineWidth(), loc, barWidth-lineWidth());
                  
                  // Draw the number, if it is needed. The number is drawn
                  // only in the location of the biggest step.
                  if (j == d->lstSteps.size()-1)
                    {
                      int firstTextMargin = lineWidth();
                      int secondTextMargin = lineWidth();
                      // Draw the line
                      if (d->drawTicks & DrawTopOrRight)
                        firstTextMargin +=d->lstLineLengths[j];
                      if (d->drawTicks & DrawBottomOrLeft)
                        secondTextMargin +=d->lstLineLengths[j];
                      
                      QString text;
                      QString unitName;
                      if (d->bDrawUnitNames && d->iCurrUnitIndex >= 0)
                        unitName = d->lstTickNames[d->iCurrUnitIndex];
                      text = QString("%1 %2").arg(double(start+i*d->lstSteps[0])/d->lstTickScales[d->iCurrUnitIndex]).arg(unitName);
                      
                      // The text is centered vertically in the empty
                      // space between the longest scale lines. If in
                      // either side drawing the scale lines are disabled,
                      // extra space is given for the text.
                      
                      int textOffset = (fontMetrics().width(text))/2;
                      QRect rect = QRect(loc-textOffset, firstTextMargin, loc+textOffset+10, barWidth-(firstTextMargin+secondTextMargin));
                      p.drawText (rect, Qt::AlignVCenter, text);
                      //                   qDebug("PiiMeasureBar::paintEvent firstTextMargin = %d", firstTextMargin);
                      //                   qDebug("PiiMeasureBar::paintEvent secondTextMargin = %d", secondTextMargin);
                    }
                }
            }
          if (bIsSmall)
            {
              //draw only the small line
              if ((d->drawTicks & DrawTopOrRight))
                p.drawLine(loc, lineWidth(), loc, d->lstLineLengths[0]+lineWidth());
              if ((d->drawTicks & DrawBottomOrLeft))
                p.drawLine(loc, barWidth-d->lstLineLengths[0]-lineWidth(), loc, barWidth-lineWidth()); 
            }
        }
    }

  if (d->bDrawMouseLocation)
    {
      p.setBrush(QBrush(QColor(Qt::black)));
      int h = barWidth - 2;
      int step = barWidth / 5;
      int x = d->mouseLocation.x() + cRect.x();
      
      if (d->orientation == Qt::Vertical)
        {
          x = d->mouseLocation.y() + cRect.y();
          h = 2;
          step = -step;
        }

      QPolygon pol;
      pol << QPoint(x-step, h-step) << QPoint(x+step, h-step) << QPoint(x, h);
      p.drawPolygon(pol);
    }
  


  p.restore();
  p.end();
  QFrame::paintEvent(event);    
  //   qDebug("PiiMeasureBar::paintEvent END");
}


void PiiMeasureBar::setOrientation(const Qt::Orientation& orientation) { d->orientation = orientation; }
Qt::Orientation PiiMeasureBar::orientation() const { return d->orientation; }
QStringList PiiMeasureBar::tickNames() const { return d->lstTickNames; }
bool PiiMeasureBar::drawUnitNames() const { return d->bDrawUnitNames;}
PiiMeasureBar::DrawTicks PiiMeasureBar::drawTicks() const { return d->drawTicks; }
QBrush PiiMeasureBar::brush() const { return d->brush; }
double PiiMeasureBar::pixelLength() const { return d->dPixelLength; }
void PiiMeasureBar::drawMouseLocation(bool para) { d->bDrawMouseLocation = para; update(); }

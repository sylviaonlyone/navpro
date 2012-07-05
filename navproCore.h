/*=============================================================================
**                            MODULE SPECIFICATION
===============================================================================
**
**  Title :
**
**  Description :
**
**
===============================================================================
**  Author            :     
**  Creation Date     :     2011
-------------------------------------------------------------------------------
**  CVS Identification:     $Revision: $
**                          $Author: $
**                          $Date: $
===============================================================================
**/

#include <QtGui>
#include <QApplication>
//#include <cstdio>
#include <iostream>

#include <sched.h>
#include <unistd.h>
#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QtGlobal>

#include <PiiEngine.h>
#include <PiiImageDisplay.h>
#include <PiiProbeInput.h>
#include "ui_navpro.h"

#define LIB_QT

//#define DEBUG_LOG
/*
const static int TOLERANT_COLOR_DEVIATION = 20;
const static int MEANINGLESS_COLOR_MIN = 70;
const static int MEANINGLESS_COLOR_MAX = 220;
const static int DEFAULT_BLOCKS = 16;

struct block
{
    quint8 hitRate;
};


class home : public QWidget
{
    //Q_OBJECT

    static const int DEFAULT_WIDTH = 320;
    static const int DEFAULT_HEIGHT = 240;

  public:
    home(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);
    bool isGray(const QRgb&);
    bool isRoad(const QRgb&);
  protected:
    void paintEvent(QPaintEvent *event);
  private:
    QImage image;
    qint32 realWidth;
    qint32 realHeight;
};

home::home(int width, int height):
    image("./road.jpg") 
{
    realWidth = (width <= image.width() ? width : image.width()) * 2;
    realHeight = height <= image.height() ? height : image.height();

    resize(realWidth + 10, realHeight + 10);
}

bool home::isGray(const QRgb& pixel)
{
    quint8 red = qRed(pixel);
    quint8 green = qGreen(pixel);
    quint8 blue = qBlue(pixel);

#ifdef DEBUG_LOG
    std::cout<<"r: "<<red<<" ";
    std::cout<<"g: "<<green<<" ";
    std::cout<<"b: "<<blue<<std::endl;
#endif

    if (green < MEANINGLESS_COLOR_MIN || green > MEANINGLESS_COLOR_MAX)
      return false;

    if (qAbs(green - red) <= TOLERANT_COLOR_DEVIATION && 
        qAbs(green - blue)<= TOLERANT_COLOR_DEVIATION )
      return true;
    else
      return false;
}

void home::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 1));

    QImage output(image);

    int i, j;
    for (i = 0; i < image.width(); ++i)
    {
        for (j = 0; j < image.height(); ++j)
        {
            if (isGray(image.pixel(i,j)))
            {
               output.setPixel(i,j, 0); 
               //output.setPixel(i,j, QColor(Qt::red).rgb()); 
            }
        }
    }
    //std::cout<<image.depth()<<std::endl;
    //std::cout<<image.width()<<std::endl;
    //std::cout<<image.height()<<std::endl;
    //std::cout<<pixels.size() <<std::endl;
    //std::cout<<"Format: "<<image.format()<<std::endl;

    // Draw the background pixmap
    painter.drawPixmap(QPoint(0,0), QPixmap::fromImage(image));
    painter.drawPixmap(QPoint(image.width() + 1,0), QPixmap::fromImage(output));
    //painter.drawRect(0,0,realWidth/2,realHeight);
}
*/

class navproCore : public QWidget
{
  static const int DEFAULT_WIDTH = 640;
  static const int DEFAULT_FULL_WIDTH = 1200;
  static const int DEFAULT_HEIGHT = 480;
  enum {
    HUE = 0,
    SATURATION,
    CB,
    CR
  };

signals:
  void updateImage(int);

public:
  navproCore(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT);
  //navproCore(QWidget *parent = 0);
  void searchRoad();
  void changeThresholdFrom(const int threshold);
  void changeThresholdTo(const int threshold);
  void showSliderValue(QSlider *pSlider, const QString& text);

protected:
  void paintEvent(QPaintEvent *event);
  void keyPressEvent(QKeyEvent * e);
private:
  void init();
  void setHueFrom(int hue){hueFrom = hue;}
  void setHueTo(int hue){hueTo = hue;}

  PiiEngine *pEngine;
  PiiProbeInput *pSourceProbeInput, *pResultProbeInput;
  PiiOperation *pImageFileReader, *pImageFileWriter;
  PiiOperation *pEdgeDetector;
  PiiOperation *pHoughTransform;
  PiiOperation *pImageAnnotator;
  PiiImageDisplay *pSourceImageDisplay;
  PiiImageDisplay *pResultImageDisplay;
  QHBoxLayout *pLayout;
  int hueFrom, hueTo;
  int saturationFrom, saturationTo;
  int cbFrom,cbTo;
  int crFrom,crTo;
  int *activeFrom, *activeTo;
  char active;
#ifdef LIB_QT
  QStringList fileList;
  QImage image;
#endif
};

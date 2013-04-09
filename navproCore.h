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
===============================================================================
**/

#ifndef NAVPRO_CORE_H_
#define NAVPRO_CORE_H_

#include <cassert>
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

//#include "ui_navpro.h"
#include "environment.h"
#include "laneTracker.h"
#include "particleFilter.h"
#include "inputManager.h"

//#define DEBUG_LOG

class navproCore : public QWidget
{
  // 10% of pixels from center will be accounted as sample
  static const int DEFAULT_SAMPLING_RANGE_PERCENTAGE = 10;

  // center x,y of sample square
  static const float DEFAULT_X_PROPOTION = 0.5;
  static const float DEFAULT_Y_PROPOTION = 0.75;
  
  enum {
    HUE = 0,
    SATURATION,
    CB,
    CR
  };

signals:
  void updateImage(int);

public:
  navproCore(laneTracker* tracker, particleFilter* filter, inputManager* input);
  ~navproCore();
  void changeThresholdFrom(const int threshold);
  void changeThresholdTo(const int threshold);
  void showSliderValue(QSlider *pSlider, const QString& text);
  //void probe(const QString& path);
  void probe();
  void move();

  QImage* getOriginImage() const {return p_image_origin_;};
  QImage* getEdgeImage() const {return p_image_edge_;};
  QImage* getMarkerImage() const {return p_image_marker_;};
  QImage* getColorImage() const {return p_image_color_;};

  const M_Prob* getParticles()
  {
    assert(pFilter);
    return pFilter->getParticles();
  }

protected:
  void paintEvent(QPaintEvent *event);
  void keyPressEvent(QKeyEvent * e);
private:
  // HSV + Cb Cr method
  void setHueFrom(int hue){hueFrom = hue;}
  void setHueTo(int hue){hueTo = hue;}
  bool singalFilter(QRgb clr);
  bool multiFilters(QRgb clr);

  // Centra point sampling method
  void getRange();

  bool getStdDeviation(int rangeX, int rangeY, int *hue, int *sat, int *cb, int *cr);

  QHBoxLayout *pLayout;

  // HSV + Cb Cr method
  int hueFrom, hueTo;
  int saturationFrom, saturationTo;
  int cbFrom,cbTo;
  int crFrom,crTo;
  int *activeFrom, *activeTo;
  char active;

  int posX;
  int posY;

  //Image from laneTracker class
  laneTracker* pTracker;
  particleFilter* pFilter;
  inputManager* p_input_manager_;
  std::vector<cv::Mat>* p_histogram_;

  //Images for display
  QImage *p_image_origin_;
  QImage *p_image_edge_;
  QImage *p_image_marker_;
  QImage *p_image_color_;

  //color table for lane marker index8 QImage
  QVector<QRgb> colorTable;

  //Images for processing
  cv::Mat cv_edge_;
  cv::Mat cv_maker_;
  cv::Mat cv_color_;
};
#endif  //NAVPRO_CORE_H_

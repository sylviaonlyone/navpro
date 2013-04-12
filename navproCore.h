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
  
signals:
  void updateImage(int);

public:

  navproCore(laneTracker* tracker, inputManager* input);
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

  const M_Prob* getParticles(int type);

protected:
  void paintEvent(QPaintEvent *event);
  void keyPressEvent(QKeyEvent * e);

private:
  bool getStdDeviation(int rangeX, int rangeY, int *hue, int *sat, int *cb, int *cr);

  int posX;
  int posY;

  //Image from laneTracker class
  laneTracker* pTracker;

  particleFilter* p_particle_edge_;
  particleFilter* p_particle_marker_;
  particleFilter* p_particle_color_;

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

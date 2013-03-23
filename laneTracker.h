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
**  Creation Date     :     2012.11.19
===============================================================================
**/
#ifndef NAVPRO_LANETRACKER_H_
#define NAVPRO_LANETRACKER_H_

#include <QString>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include "cueBase.h"
#include "particleFilter.h"
#include <iostream>

class laneTracker 
{
public:
  laneTracker();
  ~laneTracker();

  int preprocess             (const char* path);
  cv::Mat edgeDetect              ();
  std::vector<cv::Mat>& roadColorDetect ();
  cv::Mat laneMarkerDetect ();
private:
  cv::Mat cvLaplicain();
  cv::Mat src_;
  cv::Mat gray_;
  std::vector<cv::Mat>* pHistVector_;
};
#endif //NAVPRO_LANETRACKER_H_

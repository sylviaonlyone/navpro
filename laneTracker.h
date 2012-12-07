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

using namespace cv;

class laneTracker 
{
public:
  laneTracker();
  ~laneTracker();

  int preprocess             (const char* path);
  Mat edgeDetect              ();
  vector<Mat>& roadColorDetect ();
private:
  Mat src_;
  Mat gray_;
  vector<Mat>* pHistVector_;
};
#endif //NAVPRO_LANETRACKER_H_

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
#include <QtDebug>
#include "laneTracker.h"

laneTracker::laneTracker()
{
}

laneTracker::~laneTracker()
{

}

Mat laneTracker::preprocess(const char* path)
{
  Mat src, gray, edges, dstRGB;
  //const char* window_name = "Edge Map";
  src = imread(path);
  if (!src.data)
    qDebug()<<"src image NULL Error!";

  std::cout<<"image size:"<<src.size()<<" type:"<<src.type()<<std::endl;

  //namedWindow(window_name, CV_WINDOW_AUTOSIZE );

  cvtColor(src, gray, CV_BGR2GRAY);

  blur(gray, edges, Size(3,3));
  int lowThreshold = 100;
  int ratio = 3;
  int kernel_size = 3;
  Canny(edges, edges, lowThreshold, lowThreshold*ratio, kernel_size);
  Mat dst;
  dst = Scalar::all(0);

  //! copies "src" elements to "dst" that are marked with non-zero "edges" elements.
  src.copyTo(dst, edges);

  cvtColor(dst,dstRGB, CV_BGR2RGB);
  return dstRGB;
  //imshow(window_name, dst);
}

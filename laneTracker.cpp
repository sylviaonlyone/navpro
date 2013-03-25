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
  try{
    pHistVector_ = new std::vector<cv::Mat>;
  }
  catch(std::bad_alloc& ba)
  {
    std::cerr<<"alloc failed: "<<ba.what()<<std::endl;
  }
}

laneTracker::~laneTracker()
{
  delete pHistVector_;
}

int laneTracker::preprocess(const char* path)
{
  cv::Mat dstRGB;
  //const char* window_name = "Edge Map";
  src_ = cv::imread(path);
  if (!src_.data)
  {
    std::cerr<<"src image NULL Error!";
    return -1;
  }

  std::cout<<"image size:"<<src_.size()<<" type:"<<src_.type()<<std::endl;

  cvtColor(src_, gray_, CV_BGR2GRAY);

  return 0;
}

std::vector<cv::Mat>* laneTracker::roadColorDetect()
{
  //Rect(x, y ,width, height)
  //Rect roi(0, src_.rows/2, src_.cols, src_.rows/2);  

  cv::Mat roadRegion = src_(ROAD_RECT(src_.cols, src_.rows));

  /// Separate the image in 3 places ( B, G and R )
  std::vector<cv::Mat> bgr_planes;
  split(roadRegion, bgr_planes);

  /// Establish the number of bins, x-axis
  int histSize = 256;

  /// Set the ranges ( for B,G,R) ), y-axis
  // for example, there are N blue pixels at 150 [0, 255], and N is the max
  // from [0, 255], then N is max of the range:100
  float range[] = { 0, 100 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  cv::Mat b_hist, g_hist, r_hist;
  cv::Mat b_blur, g_blur, r_blur;

  GaussianBlur(bgr_planes[0], b_blur, cv::Size(5,5), 0, 0);
  GaussianBlur(bgr_planes[1], g_blur, cv::Size(5,5), 0, 0);
  GaussianBlur(bgr_planes[2], r_blur, cv::Size(5,5), 0, 0);
  //
  /// Compute the histograms:
  calcHist( &b_blur, 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &g_blur, 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &r_blur, 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );


  /// Normalize the result to [ 0, 100 ], percentage representation
  normalize(b_hist, b_hist, 0, 100, cv::NORM_MINMAX, -1, cv::Mat() );
  normalize(g_hist, g_hist, 0, 100, cv::NORM_MINMAX, -1, cv::Mat() );
  normalize(r_hist, r_hist, 0, 100, cv::NORM_MINMAX, -1, cv::Mat() );

  // clear vector first
  pHistVector_->clear();
  pHistVector_->push_back(b_hist);
  pHistVector_->push_back(g_hist);
  pHistVector_->push_back(r_hist);

  return pHistVector_;
}

cv::Mat laneTracker::edgeDetect()
{
  cv::Mat edges, dstRGB;

  GaussianBlur(gray_, edges, cv::Size(5,5), 0, 0);

  int lowThreshold = 100;
  int ratio = 3;
  int kernel_size = 3;
  Canny(edges, edges, lowThreshold, lowThreshold*ratio, kernel_size);
  cv::Mat dst;
  dst = cv::Scalar::all(0);

  //! copies "src" elements to "dst" that are marked with non-zero "edges" elements.
  src_.copyTo(dst, edges);

  cvtColor(dst,dstRGB, CV_BGR2RGB);

  std::cout<<"edge image size:"<<dstRGB.size()<<" type:"<<dstRGB.type()<<std::endl;
  return dstRGB;
}

cv::Mat laneTracker::cvLaplicain()
{
  cv::Mat src, dst, abs_dst;

  GaussianBlur(gray_, src, cv::Size(5,5), 0, 0);

  int kernel_size = 3;
  int scale = 1;
  int delta = 0;
  int ddepth = CV_16S;
  Laplacian(src, dst, ddepth, kernel_size, scale, delta, cv::BORDER_DEFAULT);
  convertScaleAbs(dst, abs_dst);

  return abs_dst;
}

cv::Mat laneTracker::laneMarkerDetect()
{
  int kernel_size = 11;
  float k[kernel_size] ;

  //calc 1-D kernel;
  int i,j;
  for(i = 0, j = -(kernel_size/2); i <= kernel_size; ++i, ++j)
  {
      k[i] = LoG(j);
      //std::cout<<" k["<<j<<"]: "<<k[i];
  }
  //std::cout<<std::endl;

  cv::Mat src, dst;
  cv::Mat roadRegion = gray_(ROAD_RECT(gray_.cols, gray_.rows));
  // blur gray source image
  // src region only has down-half size of origin gray image
  GaussianBlur(roadRegion, src, cv::Size(5,5), 0, 0);

  // dst has same size as origin gray image
  dst.create(gray_.size(), CV_MAKETYPE(CV_8U, src.channels()));
  dst = cv::Scalar::all(0);

  uchar *src_row, *src_pos_in_row, *dst_row;
  
  // so far, size of Mats are (if image w = 1, h = 1):
  // gray_      (1, 1)
  // roadRegion (1, 1/2)
  // src        (1, 1/2)
  // dst        (1, 1)
  std::cout<<"dst size: "<<dst.size()<<" cols:"<<dst.cols<<" rows:"<<dst.rows<<std::endl;
  std::cout<<"src size: "<<src.size()<<" cols:"<<src.cols<<" rows:"<<src.rows<<std::endl;
  float sum_f;
  // y-th for dst and src
  int yd,ys;
  for(yd = gray_.rows - src.rows, ys = 0; yd < gray_.rows; ++yd, ++ys)
  {
      // get pointer to y-th row
      src_row = src.ptr(ys); 
      dst_row = dst.ptr(yd); 
      for(int x = gray_.cols - src.cols; x < src.cols - kernel_size/2; ++x)
      {
          sum_f = 0.0;
          src_pos_in_row = src_row + x;
          for (int kx = 0; kx < kernel_size; ++kx)
          {
              sum_f += *(src_pos_in_row++) * k[kx];
          }
          *(dst_row + x + kernel_size/2) = static_cast<int>(sum_f);
      }
  }

  return dst;
}


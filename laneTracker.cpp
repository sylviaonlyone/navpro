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
    pHistVector_ = new vector<Mat>;
  }
  catch(std::bad_alloc& ba)
  {
    std::cerr<<"alloc failed: "<<ba.what()<<std::endl;
  }
}

laneTracker::~laneTracker()
{

}

int laneTracker::preprocess(const char* path)
{
  Mat dstRGB;
  //const char* window_name = "Edge Map";
  src_ = imread(path);
  if (!src_.data)
  {
    std::cerr<<"src image NULL Error!";
    return -1;
  }

  //std::cout<<"image size:"<<src.size()<<" type:"<<src.type()<<std::endl;

  cvtColor(src_, gray_, CV_BGR2GRAY);

  return 0;
}

vector<Mat>& laneTracker::roadColorDetect()
{
  // TODO Set ROI(Rect of interests) to down-half of image, where I assume 
  // road would be. May use Road Region Model with more accuratcy

  //Rect(x, y ,width, height)
  Rect roi(0, src_.rows/2, src_.cols, src_.rows/2);  

  Mat roadRegion = src_(roi);

  /// Separate the image in 3 places ( B, G and R )
  vector<Mat> bgr_planes;
  split(roadRegion, bgr_planes);

  /// Establish the number of bins
  int histSize = 256;

  /// Set the ranges ( for B,G,R) )
  float range[] = { 0, 256 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  Mat b_hist, g_hist, r_hist;
  Mat b_blur, g_blur, r_blur;

  GaussianBlur(bgr_planes[0], b_blur, Size(5,5), 0, 0);
  GaussianBlur(bgr_planes[1], g_blur, Size(5,5), 0, 0);
  GaussianBlur(bgr_planes[2], r_blur, Size(5,5), 0, 0);
  //
  /// Compute the histograms:
  calcHist( &b_blur, 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &g_blur, 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &r_blur, 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );


  /// Normalize the result to [ 0, 100 ], percentage representation
  normalize(b_hist, b_hist, 0, 100, NORM_MINMAX, -1, Mat() );
  normalize(g_hist, g_hist, 0, 100, NORM_MINMAX, -1, Mat() );
  normalize(r_hist, r_hist, 0, 100, NORM_MINMAX, -1, Mat() );

  // TODO remember to clear vector in the next around
  pHistVector_->push_back(b_hist);
  pHistVector_->push_back(g_hist);
  pHistVector_->push_back(r_hist);

  return *pHistVector_;
}

Mat laneTracker::edgeDetect()
{
  Mat edges, dstRGB;

  GaussianBlur(gray_, edges, Size(5,5), 0, 0);

  int lowThreshold = 100;
  int ratio = 3;
  int kernel_size = 3;
  Canny(edges, edges, lowThreshold, lowThreshold*ratio, kernel_size);
  Mat dst;
  dst = Scalar::all(0);

  //! copies "src" elements to "dst" that are marked with non-zero "edges" elements.
  src_.copyTo(dst, edges);

  cvtColor(dst,dstRGB, CV_BGR2RGB);

  return dstRGB;
}

Mat laneTracker::cvLaplicain()
{
  Mat src, dst, abs_dst;

  GaussianBlur(gray_, src, Size(5,5), 0, 0);

  int kernel_size = 3;
  int scale = 1;
  int delta = 0;
  int ddepth = CV_16S;
  Laplacian(src, dst, ddepth, kernel_size, scale, delta, BORDER_DEFAULT);
  convertScaleAbs(dst, abs_dst);

  return abs_dst;
}

Mat laneTracker::laneMarkerDetect()
{
  int kernel_size = 11;
  float k[kernel_size] ;
  //calc 1-D kernel;
  int i,j;
  for(i = 0, j = -(kernel_size/2); i <= kernel_size; ++i, ++j)
  {
      k[i] = LoG(j);
      std::cout<<" k["<<j<<"]: "<<k[i];
  }
  std::cout<<std::endl;
  Mat kernel(3, 1, CV_8U, k);

  Mat src, dst;
  src = gray_;
  dst.create( src.size(), CV_MAKETYPE(CV_8U, src.channels()));
  std::cout<<"size: "<<src.size()<<" cols:"<<src.cols<<" rows:"<<src.rows<<std::endl;

  uchar *src_row, *src_pos_in_row, *dst_row;
  
  float sum_f;
  for(int y = 0; y < src.rows; ++y)
  {
      // get pointer to y-th row
      src_row = src.ptr(y); 
      dst_row = dst.ptr(y); 
      for(int x = 0; x < gray_.cols - kernel_size/2; ++x)
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


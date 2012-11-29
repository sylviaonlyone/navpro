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

vector<Mat>& laneTracker::roadColorDetect(float array3D[][256][256])
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

  //create RGB 3D array
  float sum = 0.0;
  for(int r = 0; r < 256; ++r)
  {
      for(int g = 0; g < 256; ++g)
      {
          for(int b = 0; b < 256; ++b)
          {
              array3D[r][g][b] = static_cast<float>(r_hist.at<int>(r) * g_hist.at<int>(g) * b_hist.at<int>(b)) / 
                                 static_cast<float>(100 * 100 * 100);
              sum += array3D[r][g][b];
          }
      }
  }

  //normalize result
  for(int r = 0; r < 256; ++r)
  {
      for(int g = 0; g < 256; ++g)
      {
          for(int b = 0; b < 256; ++b)
          {
              array3D[r][g][b] = array3D[r][g][b]/sum;
          }
      }
  }

  std::cout<<"sum: "<<sum<<std::endl;

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

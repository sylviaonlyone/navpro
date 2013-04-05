/*=============================================================================
**                            MODULE SPECIFICATION
===============================================================================
**
**  Title : Particle filter
**
**  Description : Particle filter class define, the original code borrowed from
**                from Sebastian's Code at:
**                http://www.udacity.com/
**                Udacity CS373: Programming a Robotic Car
**
**
===============================================================================
**  Author            :     Xin Zhang
**  Creation Date     :     2012.11.18
===============================================================================
**/

#ifndef NAVPRO_PARTICLEfILTER_H_
#define NAVPRO_PARTICLEfILTER_H_

#include <QImage>

#include <opencv2/core/core.hpp>

#include "environment.h"


static int randomInt(const int low, const int high)
{
    Q_ASSERT(high > low);
    return qrand()%((high + 1) - low) + low;
}

static int Distance(const int X1, const int Y1, const int X2, const int Y2)
{                        
    return static_cast<int>(sqrt(pow((X1-X2), 2) + pow((Y1-Y2), 2)));
}


struct measurement
{
  unsigned int x;
  unsigned int y;
  float probabilityEdge;
  float probabilityMarker;
  float probabilityRoad;
  float probabilityNonRoad;

  measurement()
    : x(randomInt(0, FRAME_WIDTH)),
    y(randomInt(0, FRAME_HEIGHT)),
    probabilityEdge(0.0),
    probabilityMarker(0.0),
    probabilityRoad(0.0),
    probabilityNonRoad(0.0)
  {
  }

  measurement& operator= (const measurement& M)
  {
      this->x = M.x;
      this->y = M.y;
      this->probabilityEdge = M.probabilityEdge;
      this->probabilityMarker = M.probabilityMarker;
      this->probabilityRoad = M.probabilityRoad;
      this->probabilityNonRoad = M.probabilityNonRoad;
      return *this;
  }
};

typedef measurement M_Prob;

class particleFilter
{
  public:
    const static int NUMBER_OF_PARTICLES = 1000;

    particleFilter();
    ~particleFilter();
    void resample();
    // update road color cue
    void measurementUpdate(const std::vector<cv::Mat>& rgbHistogram, const QImage& rawImage);
    // update road edge cue
    void measurementUpdate(const QImage&, bool isEdge);
    const M_Prob* getParticles() { return pMeasureArray;}
    void move(const int pixels);

  private:
    particleFilter            (const particleFilter &);
    particleFilter& operator= (const particleFilter &);

    void printParticles(const char* header = NULL);
    //pointer to robot
    float globleNoise;
    M_Prob* pMeasureArray;
};

#endif //NAVPRO_PARTICLEfILTER_H_

/*=============================================================================
**                            MODULE SPECIFICATION
===============================================================================
**
**  Title : mathematic functionsformulas Particle filter
**
**  Description : Most common used math formulas define
**
**
===============================================================================
**  Author            :     Xin Zhang
**  Creation Date     :     2012.11.18
===============================================================================
**/

#ifndef NAVPRO_ENVIRONMENT_H_
#define NAVPRO_ENVIRONMENT_H_

#include <math.h>
#include <QtGlobal>

// Create road rectangle from image width, height
// Return down-half of image
//   (0, 0)  +------------+
//           |            |
//(0, y/2) ->|            |
//           |            |
//           +------------+(x, y)
#define ROAD_RECT(WIDTH, HEIGHT) (Rect(0, HEIGHT/2, WIDTH, HEIGHT/2))

#define PI		3.14159265358979323846	// pi
#define Gaussian(MU, SIGMA, X) \
        (                      \
          exp(-(pow(((MU) - (X)), 2) / (2 * pow((SIGMA), 2)))) / ((SIGMA) * sqrt(2.0 * PI)) \
        )      

//1-D Lanplician Of Gaussian kernel calculation
//LoG(r) = c(1-r^2/a^2)exp^(-r^2/2a^2)
//a = 2, c= 2/(3^(1/2)Pi^(1/4))
#define LoG(R) \
        (      \
          (2 / (sqrt(3) * pow(PI, 0.25))) * (1 - ((R) * (R))/(2.0 * 2.0)) * exp(-(((R) * (R)) / (2.0 * 2.0 * 2.0))) \
        )      
#if 0
#define Distance(X1, Y1, X2, Y2) \
        (                        \
          sqrt(pow((X1-X2), 2) + pow((Y1-Y2), 2)) \
        )
#endif            

#define RGB2CB(clr) (0.148*qRed((clr))-0.291*qGreen((clr))+0.439*qBlue((clr))+128)

#define RGB2CR(clr) (0.439*qRed((clr))-0.368*qGreen((clr))+0.071*qBlue((clr))+128)

#endif  //NAVPRO_ENVIRONMENT_H_

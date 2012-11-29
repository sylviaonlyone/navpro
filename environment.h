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

#define PI		3.14159265358979323846	// pi
#define Gaussian(MU, SIGMA, X) \
        (                      \
          exp(-(pow(((MU) - (X)), 2) / (2 * pow((SIGMA), 2)))) / ((SIGMA) * sqrt(2.0 * PI)) \
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

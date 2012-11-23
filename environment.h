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

#define PI		3.14159265358979323846	// pi
#define GAUSSIAN(MU, SIGMA, X) \
        {                      \
          exp(-(pow(((MU) - (X)), 2) / (2 * pow((SIGMA), 2)))) / ((SIGMA) * sqrt(2.0 * PI)) \
        }      
#endif  //NAVPRO_ENVIRONMENT_H_

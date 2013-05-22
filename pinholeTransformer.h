/*=============================================================================
**                            MODULE SPECIFICATION
===============================================================================
**
**  Title : 
**
**  Description : 
**                
**                Source from: 
**                Vision based lane tracking using multiple cues and particle 
**                filtering
**                --Nicholas Apostoloff ANU
**
**
===============================================================================
**  Author            :     Xin Zhang
**  Creation Date     :     2013.05.15
===============================================================================
**/

#ifndef PINHOLE_TRANSFORMER_H
#define PINHOLE_TRANSFORMER_H

//#include <QString>
#include <cmath>

#include "environment.h"
#include "point.h"

class pinholeTransformer
{
  public:

    static Point translation(const float F, const float fx, const float fy, const float x, const float y, const HomoPoint3D& PA)
    {
        //Pi is 3-D vector, should eventually downgrade to 2-D
        Point3D Pi = Point3D((x*PA.getX() - fx*PA.getY())/F, (y*PA.getX() - fy*PA.getY())/F, PA.getX()/F);

        return Point(Pi.getX(), Pi.getY());
    }
};

#endif  //PINHOLE_TRANSFORMER_H

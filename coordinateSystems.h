/*=============================================================================
**                            MODULE SPECIFICATION
===============================================================================
**
**  Title : Coordinate systems for modelling Road 
**
**  Description : 
**              
**
**
**
===============================================================================
**  Author            :     Xin Zhang
**  Creation Date     :     2013.05.22
===============================================================================
**/

#ifndef COORDINATE_SYSTEMS_H
#define COORDINATE_SYSTEMS_H

#include "environment.h"
#include "point.h"
#include "eulerTransformer.h"
#include "pinholeTransformer.h"

//TODO: The assumpation is: there is not lateral offset and yaw offset from 
//the skeletal line, at inital stage don't have to care about Ysr and ψ.
//So far coordinate system I only count:
//road coordinate system(RCS): centered on the skeletal line of the lane
//image plane coordinate system(IPCS)
//formula a point from RCS to IPCS
//Pi = Mic * Pr
//Mic downgrade to pinhole transform
Point RCS2IPCS(const Point3D& Pr)
{
   return pinholeTransformer::translation(Focal, Fx, Fx, principleX, principleY, Pr);
}

#endif  //COORDINATE_SYSTEMS_H

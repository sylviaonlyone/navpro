/*=============================================================================
**                            MODULE SPECIFICATION
===============================================================================
**
**  Title : Homogeneous coordinate transformation
**
**  Description : Homogenous coordinate transformation includes Euler translation
**                and Euler roation.
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

#ifndef EULER_TRANSFORMER_H
#define EULER_TRANSFORMER_H

//#include <QString>
#include <cmath>

#include "environment.h"
#include "point.h"

class eulerTransformer
{
  public:
    eulerTransformer();
    ~eulerTransformer();
    
//Euler homogeneous translation
//P{B}=T(x<BA>, y<BA>, z<BA>)P{A}
//
//                | 1 0 0 tx |
//T(tx, ty, tz) = | 0 1 0 ty |
//                | 0 0 1 tz |
//                | 0 0 0 1  |
//       | x |
//P{A} = | y |
//       | z |
//       | 1 |
//
//       | x+tx |
//p{B} = | y+ty |
//       | z+tz |
//       |  1   |
//
//NOTE: T is not a Point, its shift along the X,Y and Z respecitvely, 
//but it's easy to represent as a Point
    static HomoPoint3D translation(const Point3D& T, const HomoPoint3D& PA)
    {
        return HomoPoint3D(PA.getX() + T.getX(),
                           PA.getY() + T.getY(),
                           PA.getZ() + T.getZ());
    }

//Euler homogeneous rotation
//P{B}=Rx<BA>, Ry<BA>, Rz<BA>)P{A}
//
//         | 1    0      0     0 |
//Rx(φ) =  | 0 cos(φ) - sin(φ) 0 |
//         | 0 sin(φ) cos(φ)   0 |
//         | 0    0      0     1 |
//
//         | cos(θ)  0  sin(θ) 0 |
//Ry(θ) =  |    0    1   0     0 |
//         | -sin(θ) 0  cos(θ) 0 |
//         |    0    0   0     1 |
//
//         | cos(ψ) -sin(ψ) 0 0 |
//Rz(ψ) =  | sin(ψ) cos(ψ)  0 0 |
//         |  0      0      1 0 |
//         |  0      0      0 1 |
//
//       |            cos(θ)cos(ψ)                    cos(θ)*-sin(ψ)                   sin(θ)     0 |   | x |
//p{B} = | -sin(φ)*-sin(θ)cos(ψ)+cos(φ)sin(ψ)   -sin(φ)*-sin(θ)*-sin(ψ)+cos(φ)cos(ψ)     0        0 |   | y |
//       | cos(φ)*-sin(θ)cos(ψ)+sin(φ)sin(ψ)    cos(φ)*-sin(θ)*-sin(ψ)+sin(φ)cos(ψ)  cos(φ)cos(θ) 1 | * | z |
//       |                 0                                0                            0        1 |   | 1 |
//rx, ry,rz are rotation along the X, Y and Z respecitvely, range is [0, 2Pi]
    static HomoPoint3D rotation(const float rx, const float ry, const float rz, const HomoPoint3D& PA)
    {
        return HomoPoint3D(cos(ry)*cos(rz)*PA.getX() + cos(ry)*(-sin(rz))*PA.getY() + sin(ry)*PA.getZ(),
                           ((-sin(rx))*(-sin(ry))*cos(rz)+cos(rx)*sin(rz))*PA.getX() + ((-sin(rx))*(-sin(ry))*(-sin(rz))+cos(rx)*cos(rz))*PA.getY(),
                           (cos(rx)*(-sin(ry))*cos(rz)+sin(rx)*sin(rz))*PA.getX() + (cos(rx)*(-sin(ry))*(-sin(rz))+sin(rx)*cos(rz))*PA.getY() + cos(rx)*cos(ry)*PA.getZ() + 1);
    }

    static HomoPoint3D translationRotation(const Point3D& T, const float rx, const float ry, const float rz, const HomoPoint3D& PA)
    {
        return translation(T, PA) + rotation(rx, ry, rz, PA);
    }
};

#endif  //EULER_TRANSFORMER_H

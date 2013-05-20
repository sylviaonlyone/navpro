/*=============================================================================
**                            MODULE SPECIFICATION
===============================================================================
**
**  Title : point defines in 2D, 3D and homogeneous coordinate
**
**  Description : 
**
**
===============================================================================
**  Author            :     Xin Zhang
**  Creation Date     :     2013.05.16
===============================================================================
**/

#ifndef POINT_H
#define POINT_H

//#include "environment.h"

class Point
{
  public:
    Point(float x = 0.0, float y = 0.0)
      : x_(x),
        y_(y)
    {
    }

    Point(const Point& PA)
      : x_(PA.x_),
        y_(PA.y_)
    {
    }

    Point& operator= (const Point& P)
    {
        this->x_ = P.x_;
        this->y_ = P.y_;

        return *this;
    }

    float getX() const {return x_;}
    float getY() const {return y_;}

  private:
    float x_;
    float y_;
};

class Point3D : public Point
{
  public:
    Point3D(float x = 0.0, float y = 0.0, float z = 0.0)
      : Point(x, y),
        z_(z)
    {
    }

    Point3D(const Point3D& PA)
      : Point(PA),
        z_(PA.z_)
    {
    }

    Point3D& operator= (const Point3D& P)
    {
        this->z_ = P.z_;

        return *this;
    }

    float getZ() const {return z_;}

  private:
    float z_;

};

struct HomoPoint3D : public Point3D
{

  public:
    HomoPoint3D(float x = 0.0, float y = 0.0, float z = 0.0)
      : Point3D(x, y, z),
        homo_(1.0)
    {
    }

    HomoPoint3D(const Point3D& PA)
      : Point3D(PA),
        homo_(1.0)
    {
    }

    HomoPoint3D& operator= (const HomoPoint3D& P)
    {
        this->homo_ = P.homo_;

        return *this;
    }

    HomoPoint3D& operator+ (const HomoPoint3D& P)
    {
    }

  private:
    float homo_;
};

HomoPoint3D operator+ (const HomoPoint3D& PA, const HomoPoint3D& PB)
{
    return HomoPoint3D(PA.getX() + PB.getX(),
                       PA.getY() + PB.getY(),
                       PA.getZ() + PB.getZ());
}

#endif  //POINT_H

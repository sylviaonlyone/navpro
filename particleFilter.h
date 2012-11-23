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

class particleFilter
{
  public:
    particleFilter();
    ~particleFilter();
    void resample();
    void measurement_update();
  private:
    particleFilter            (const particleFilter &);
    particleFilter& operator= (const particleFilter &);
    //pointer to robot
};

#endif //NAVPRO_PARTICLEfILTER_H_

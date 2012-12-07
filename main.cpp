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
**  Creation Date     :     2011
-------------------------------------------------------------------------------
**  CVS Identification:     $Revision: $
**                          $Author: $
**                          $Date: $
===============================================================================
**/

#include <QApplication>
#include "navproCore.h"
#include "laneTracker.h"
#include "particleFilter.h"
//#define DEBUG_LOG

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    laneTracker tracker;
    particleFilter filter;
    navproCore instance(&tracker, &filter);

    instance.probe(QString("./road/1.JPG"));
    instance.show();
    return a.exec();
}

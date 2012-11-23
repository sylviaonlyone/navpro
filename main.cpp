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
//#define DEBUG_LOG

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    laneTracker tracker;
    navproCore instance(&tracker);

    instance.show();
    return a.exec();
}

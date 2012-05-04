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
//#define DEBUG_LOG

int main(int argc, char *argv[])
{
    int retValue;

    QApplication a(argc, argv);

    navproCore* instance = new navproCore;
    instance->show();

    retValue = a.exec();
    delete instance;
    return retValue;
}

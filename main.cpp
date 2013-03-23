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
#include "inputManager.h"
#include "mainwindow.h"
#define DEBUG_LOG

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString path = QString("road/");

    //opencv image processing class
    laneTracker tracker;
    particleFilter filter;
    inputManager input(path);

    navproCore core(&tracker, &filter, &input);

    //main window should know core for display
    mainwindow window(&core);

    //core.probe();
    window.show();
    //core.show();
    return a.exec();
}

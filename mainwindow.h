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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "navproCore.h"
#include "particleFilter.h"

namespace Ui {
class MainWindow;
}

class mainwindow : public QWidget
{
    Q_OBJECT
    const static int MAIN_WINDOW_WIDTH = 800;
    const static int MAIN_WINDOW_HEIGHT = 600;
    //singal image size 400 x 300
    const static int WIDTH = 400;
    const static int HEIGHT = 300;
    
    //edge display offset from mainwindow(0,0)
    const static int EDGE_OFFSET_X = 400;
    const static int EDGE_OFFSET_Y = 0;
    
    //lane marker display offset from mainwindow(0,0)
    const static int MARKER_OFFSET_X = 400;
    const static int MARKER_OFFSET_Y = 300;

    //color display offset from mainwindow(0,0)
    const static int COLOR_OFFSET_X = 0;
    const static int COLOR_OFFSET_Y = 300;

public:
    mainwindow(navproCore *core, QWidget *parent = 0);
    ~mainwindow();
    
protected:
    void keyPressEvent(QKeyEvent * e);

private:
    class widgetParticle : public QWidget
    {
      public:
        widgetParticle(QWidget *parent, const M_Prob* pArray);
        ~widgetParticle();
      protected:
        void paintEvent(QPaintEvent *event);
      private:
        const M_Prob *p_particle_array_;
    };
    void updateUi();

    Ui::MainWindow *pUi;
    navproCore *p_Core_;
    QImage *p_origin_;
    QImage *p_edge_;
    QImage *p_mark_;
    QImage *p_color_;
    widgetParticle *p_widget_particle_;

    bool show_particles_;
};
#endif // MAINWINDOW_H

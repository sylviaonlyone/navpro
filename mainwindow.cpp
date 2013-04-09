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

#include <cassert>
#include "mainwindow.h"
#include "ui_mainwindow.h"

mainwindow::mainwindow(navproCore *core, QWidget *parent) :
    QWidget(parent),
    pUi(new Ui::MainWindow),
    p_Core_(core),
    show_particles_(true)
{
    pUi->setupUi(this);

    assert(p_Core_);
    p_origin_ = p_Core_->getOriginImage();
    p_edge_ = p_Core_->getEdgeImage();
    p_mark_ = p_Core_->getMarkerImage();
    p_color_ = p_Core_->getColorImage();

    const M_Prob *pProb = p_Core_->getParticles();
    assert(pProb);
    p_widget_particle_ = new widgetParticle(this, pProb);
    p_widget_particle_->resize(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    //make sure p_particles widget is on top
    p_widget_particle_->raise();

    updateUi();
}

mainwindow::~mainwindow()
{
    delete pUi;
}

void mainwindow::keyPressEvent(QKeyEvent * e)
{
    switch(e->key())
    {
      case Qt::Key_N:
      {
          std::cout<<"move 10"<<std::endl;
          for(int i = 0; i < 10; i++)
          {
            p_Core_->move();
            p_widget_particle_->update();
            //updateUi();
          }
      }
      break;
      default:
      break;
    }
}

void mainwindow::updateUi()
{
    pUi->origin->setPixmap(QPixmap::fromImage(p_origin_->scaledToWidth(WIDTH)));
    pUi->edge->setPixmap(QPixmap::fromImage(p_edge_->scaledToWidth(WIDTH)));
    pUi->marker->setPixmap(QPixmap::fromImage(p_mark_->scaledToWidth(WIDTH)));
    pUi->color->setPixmap(QPixmap::fromImage(p_color_->scaledToWidth(WIDTH)));

    p_widget_particle_->update();
}

mainwindow::widgetParticle::widgetParticle(QWidget *parent, const M_Prob *pArray) :
    QWidget(parent),
    p_particle_array_(pArray)
{
    assert(p_particle_array_);
}

mainwindow::widgetParticle::~widgetParticle()
{
}

void mainwindow::widgetParticle::paintEvent(QPaintEvent *event)
{
    std::cout<<"widget particle paintEvent()"<<std::endl;

    (void)event;
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(QBrush(Qt::red));

    //map particle (640x480) to display(400x300)
    int particle_x, particle_y, ui_x, ui_y;
    for(int i = 0; i < particleFilter::NUMBER_OF_PARTICLES; ++i)
    {
        particle_x = p_particle_array_[i].x;
        particle_y = p_particle_array_[i].y;
        std::cout<<"i:"<<i<<" x:"<<particle_x<<" y:"<<particle_y<<std::endl;
        ui_x = particle_x * static_cast<double>(WIDTH)/FRAME_WIDTH;
        ui_y = particle_y * static_cast<double>(HEIGHT)/FRAME_HEIGHT;

        //draw ellipse on origin image
        //painter.drawEllipse(QPoint(ui_x, ui_y), 2, 2);
        //draw ellipse on edge image
        painter.drawEllipse(QPoint(ui_x + EDGE_OFFSET_X , ui_y + EDGE_OFFSET_Y), 2, 2);
        //draw ellipse on lane marker image
        //painter.drawEllipse(QPoint(ui_x + MARKER_OFFSET_X, ui_y + MARKER_OFFSET_Y), 2, 2);
        //draw ellipse on color image
        //painter.drawEllipse(QPoint(ui_x + COLOR_OFFSET_X, ui_y + COLOR_OFFSET_Y), 2, 2);
    }
}

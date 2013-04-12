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

    p_widget_particle_ = new widgetParticle(this);
    p_widget_particle_->resize(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT);
    // hide widget display not stop drawing particles
    if (show_particles_)
      //make sure p_particles widget is on top
      p_widget_particle_->raise();
    else
      p_widget_particle_->lower();

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

mainwindow::widgetParticle::widgetParticle(mainwindow *parent) :
    QWidget(parent),
    p_parent_(parent)
{
    assert(p_parent_);
}

mainwindow::widgetParticle::~widgetParticle()
{
}

void mainwindow::widgetParticle::paintEvent(QPaintEvent *event)
{
    std::cout<<"widget particle paintEvent()"<<std::endl;

    (void)event;

    const M_Prob *prob = p_parent_->getParticles(particleFilter::EDGE);
    assert(prob);
    paintParticles(prob, EDGE_OFFSET_X, EDGE_OFFSET_Y);

    prob = p_parent_->getParticles(particleFilter::LANE_MARKER);
    assert(prob);
    paintParticles(prob, MARKER_OFFSET_X, MARKER_OFFSET_Y);

    prob = p_parent_->getParticles(particleFilter::COLOR);
    assert(prob);
    paintParticles(prob, COLOR_OFFSET_X, COLOR_OFFSET_Y);
}

void mainwindow::widgetParticle::paintParticles(const M_Prob* prob, int offset_x, int offset_y)
{
    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(QBrush(Qt::red));
 
    //map particle (640x480) to display(400x300)
    int particle_x, particle_y, ui_x, ui_y;
    for(int i = 0; i < particleFilter::NUMBER_OF_PARTICLES; ++i)
    {
        particle_x = prob[i].x;
        particle_y = prob[i].y;
        std::cout<<"i:"<<i<<" x:"<<particle_x<<" y:"<<particle_y<<std::endl;
        ui_x = particle_x * static_cast<double>(WIDTH)/FRAME_WIDTH;
        ui_y = particle_y * static_cast<double>(HEIGHT)/FRAME_HEIGHT;

        //draw ellipse on origin image
        painter.drawEllipse(QPoint(ui_x + offset_x, ui_y + offset_y), 1, 1);
    }
}

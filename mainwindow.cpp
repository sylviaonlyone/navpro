#include "mainwindow.h"
#include "ui_mainwindow.h"

mainwindow::mainwindow(navproCore *core, QWidget *parent) :
    QWidget(parent),
    pUi(new Ui::mainwindow),
    p_Core_(core)
{
    p_origin_ = p_Core_->getOriginImage();
    p_edge_ = p_Core_->getEdgeImage();
    p_mark_ = p_Core_->getMarkerImage();
    p_color_ = p_Core_->getColorImage();
    std::cout<<"edge image(mainwindow) size:"<<p_edge_->width()<<" "<<p_edge_->height()<<std::endl;
    std::cout<<"mark image(mainwindow) size:"<<p_mark_->width()<<" "<<p_mark_->height()<<std::endl;

    pUi->setupUi(this);

    pUi->origin->setPixmap(QPixmap::fromImage(p_origin_->scaledToWidth(WIDTH)));
    pUi->edge->setPixmap(QPixmap::fromImage(p_edge_->scaledToWidth(WIDTH)));
    pUi->marker->setPixmap(QPixmap::fromImage(p_mark_->scaledToWidth(WIDTH)));
    pUi->color->setPixmap(QPixmap::fromImage(p_color_->scaledToWidth(WIDTH)));
}

mainwindow::~mainwindow()
{
    delete pUi;
}

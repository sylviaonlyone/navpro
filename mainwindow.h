#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "navproCore.h"

namespace Ui {
class mainwindow;
}

class mainwindow : public QWidget
{
    Q_OBJECT
    //singal image size 400 x 300
    const static int WIDTH = 400;
    const static int HEIGTH = 300;
    
public:
    mainwindow(navproCore *core, QWidget *parent = 0);
    ~mainwindow();
    
private:
    Ui::mainwindow *pUi;
    navproCore *p_Core_;
    QImage *p_origin_;
    QImage *p_edge_;
    QImage *p_mark_;
    QImage *p_color_;

};

#endif // MAINWINDOW_H

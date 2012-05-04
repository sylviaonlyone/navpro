/********************************************************************************
** Form generated from reading UI file 'navpro.ui'
**
** Created: Fri May 4 19:44:54 2012
**      by: Qt User Interface Compiler version 4.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NAVPRO_H
#define UI_NAVPRO_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_NavProClass
{
public:

    void setupUi(QWidget *NavProClass)
    {
        if (NavProClass->objectName().isEmpty())
            NavProClass->setObjectName(QString::fromUtf8("NavProClass"));
        NavProClass->resize(320, 240);

        retranslateUi(NavProClass);

        QMetaObject::connectSlotsByName(NavProClass);
    } // setupUi

    void retranslateUi(QWidget *NavProClass)
    {
        NavProClass->setWindowTitle(QApplication::translate("NavProClass", "NavPro", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class NavProClass: public Ui_NavProClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NAVPRO_H

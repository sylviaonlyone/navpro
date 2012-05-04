/********************************************************************************
** Form generated from reading UI file 'navpro.ui'
**
** Created: Wed Mar 7 23:07:26 2012
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

class Ui_CtxPodGuiClass
{
public:

    void setupUi(QWidget *CtxPodGuiClass)
    {
        if (CtxPodGuiClass->objectName().isEmpty())
            CtxPodGuiClass->setObjectName(QString::fromUtf8("CtxPodGuiClass"));
        CtxPodGuiClass->resize(320, 240);

        retranslateUi(CtxPodGuiClass);

        QMetaObject::connectSlotsByName(CtxPodGuiClass);
    } // setupUi

    void retranslateUi(QWidget *CtxPodGuiClass)
    {
        CtxPodGuiClass->setWindowTitle(QApplication::translate("CtxPodGuiClass", "CtxPodGui", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class CtxPodGuiClass: public Ui_CtxPodGuiClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NAVPRO_H

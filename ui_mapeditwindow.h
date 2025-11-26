/********************************************************************************
** Form generated from reading UI file 'mapeditwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAPEDITWINDOW_H
#define UI_MAPEDITWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MapEditWindow
{
public:
    QWidget *centralwidget;

    void setupUi(QMainWindow *MapEditWindow)
    {
        if (MapEditWindow->objectName().isEmpty())
            MapEditWindow->setObjectName("MapEditWindow");
        MapEditWindow->resize(800, 600);
        MapEditWindow->setMinimumSize(QSize(800, 600));
        MapEditWindow->setMaximumSize(QSize(800, 600));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/res/destination.jpg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        MapEditWindow->setWindowIcon(icon);
        centralwidget = new QWidget(MapEditWindow);
        centralwidget->setObjectName("centralwidget");
        MapEditWindow->setCentralWidget(centralwidget);

        retranslateUi(MapEditWindow);

        QMetaObject::connectSlotsByName(MapEditWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MapEditWindow)
    {
        MapEditWindow->setWindowTitle(QCoreApplication::translate("MapEditWindow", "\347\274\226\350\276\221\350\277\267\345\256\253\345\234\260\345\233\276", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MapEditWindow: public Ui_MapEditWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAPEDITWINDOW_H

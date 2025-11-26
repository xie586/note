/********************************************************************************
** Form generated from reading UI file 'chooselevelwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHOOSELEVELWINDOW_H
#define UI_CHOOSELEVELWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChooseLevelWindow
{
public:
    QWidget *centralwidget;

    void setupUi(QMainWindow *ChooseLevelWindow)
    {
        if (ChooseLevelWindow->objectName().isEmpty())
            ChooseLevelWindow->setObjectName("ChooseLevelWindow");
        ChooseLevelWindow->resize(800, 600);
        ChooseLevelWindow->setMinimumSize(QSize(800, 600));
        ChooseLevelWindow->setMaximumSize(QSize(800, 600));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/res/destination.jpg"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
        ChooseLevelWindow->setWindowIcon(icon);
        centralwidget = new QWidget(ChooseLevelWindow);
        centralwidget->setObjectName("centralwidget");
        ChooseLevelWindow->setCentralWidget(centralwidget);

        retranslateUi(ChooseLevelWindow);

        QMetaObject::connectSlotsByName(ChooseLevelWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ChooseLevelWindow)
    {
        ChooseLevelWindow->setWindowTitle(QCoreApplication::translate("ChooseLevelWindow", "\351\200\211\346\213\251\345\205\263\345\215\241", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChooseLevelWindow: public Ui_ChooseLevelWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHOOSELEVELWINDOW_H

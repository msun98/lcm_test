/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.8
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *bt_Test;
    QPushButton *bt_SendMap;
    QLabel *lb_Screen1;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1132, 1000);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        bt_Test = new QPushButton(centralwidget);
        bt_Test->setObjectName(QString::fromUtf8("bt_Test"));
        bt_Test->setGeometry(QRect(1020, 10, 89, 41));
        bt_SendMap = new QPushButton(centralwidget);
        bt_SendMap->setObjectName(QString::fromUtf8("bt_SendMap"));
        bt_SendMap->setGeometry(QRect(1020, 60, 89, 41));
        lb_Screen1 = new QLabel(centralwidget);
        lb_Screen1->setObjectName(QString::fromUtf8("lb_Screen1"));
        lb_Screen1->setGeometry(QRect(0, 0, 1000, 1000));
        lb_Screen1->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);"));
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        bt_Test->setText(QApplication::translate("MainWindow", "Test", nullptr));
        bt_SendMap->setText(QApplication::translate("MainWindow", "Send map", nullptr));
        lb_Screen1->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

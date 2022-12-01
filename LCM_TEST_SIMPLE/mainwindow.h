#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <chrono>
#include <thread>

// lcm
#include <lcm/lcm-cpp.hpp>
#include "lcm_types/example_t.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    double get_time();

    // lcm
    lcm::LCM lcm;
    void example_callback(const lcm::ReceiveBuffer *rbuf, const std::string &chan, const example_t *msg);

    // lcm message loop
    std::atomic<bool> bFlag;
    std::thread* bThread = NULL;
    void bLoop();

private:
    Ui::MainWindow *ui;

private slots:
    void bt_Test();

};
#endif // MAINWINDOW_H

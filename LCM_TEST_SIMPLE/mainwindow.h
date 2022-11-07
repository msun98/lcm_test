#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <chrono>
#include <thread>

// opencv
#include <opencv2/opencv.hpp>
#include "cv_to_qt.h"

// lcm
#include <lcm/lcm-cpp.hpp>
#include "lcm_types/example_t.hpp"
#include "lcm_types/map_data_t.hpp"

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
    void map_data_callback(const lcm::ReceiveBuffer *rbuf, const std::string &chan, const map_data_t *msg);

    // lcm message loop
    std::atomic<bool> lcmFlag;
    std::thread* lcmThread = NULL;
    void lcmLoop();

private:
    Ui::MainWindow *ui;

private slots:
    void bt_Test();
    void bt_SendMap();

};
#endif // MAINWINDOW_H

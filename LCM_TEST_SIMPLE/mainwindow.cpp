#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lcm("udpm://239.255.76.67:7667?ttl=1")
{
    ui->setupUi(this);
    connect(ui->bt_Test, SIGNAL(clicked()), this, SLOT(bt_Test()));

    if (bThread == NULL)
    {
        bFlag = true;
        bThread = new std::thread(&MainWindow::bLoop, this);
    }
}

MainWindow::~MainWindow()
{
    if(bThread != NULL)
    {
        bFlag = false;
        bThread->join();
    }

    delete ui;
}

double MainWindow::get_time()
{
    std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count();
    return timestamp*1.0e-9; // nano to sec
}

void MainWindow::bt_Test()
{
    example_t send_msg;
    send_msg.time = get_time();
    lcm.publish("EXAMPLE", &send_msg);
    printf("PUB: %f\n", send_msg.time);
}

void MainWindow::example_callback(const lcm::ReceiveBuffer *rbuf, const std::string &chan, const example_t *msg)
{
    printf("SUB: %f\n", msg->time);
}

void MainWindow::bLoop()
{
    /*
    sudo ifconfig lo multicast
    sudo route add -net 224.0.0.0 netmask 240.0.0.0 dev lo
    */

    // lcm init
    if(lcm.good())
    {
        lcm.subscribe("EXAMPLE", &MainWindow::example_callback, this);
    }
    else
    {
        printf("lcm init failed\n");
    }

    while(bFlag)
    {
        lcm.handleTimeout(1);
    }
}

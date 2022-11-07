#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , lcm("udpm://239.255.76.67:7667?ttl=1")
{
    ui->setupUi(this);
    connect(ui->bt_Test, SIGNAL(clicked()), this, SLOT(bt_Test()));
    connect(ui->bt_SendMap, SIGNAL(clicked()), this, SLOT(bt_SendMap()));

    if (lcmThread == NULL)
    {
        lcmFlag = true;
        lcmThread = new std::thread(&MainWindow::lcmLoop, this);
    }
}

MainWindow::~MainWindow()
{
    if(lcmThread != NULL)
    {
        lcmFlag = false;
        lcmThread->join();
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
    printf("PUB(EXAMPLE): %f\n", send_msg.time);
}

void MainWindow::bt_SendMap()
{
    cv::Mat map = cv::imread("raw_map.png", cv::IMREAD_GRAYSCALE);
    printf("loaded map w:%d, h:%d\n", map.cols, map.rows);

    // send structure
    map_data_t send_msg;

    // map metadata
    send_msg.map_name = "test_map";
    send_msg.map_grid_w = 0.025; // 2.5cm
    send_msg.map_w = map.cols;
    send_msg.map_h = map.rows;
    send_msg.map_origin[0] = map.cols/2;
    send_msg.map_origin[1] = map.rows/2;

    // map data
    send_msg.len = send_msg.map_w*send_msg.map_h;
    send_msg.data.resize(send_msg.len, 0);
    memcpy(send_msg.data.data(), map.data, map.rows*map.cols);

    // set via points for test
    send_msg.via_num = 2;
    send_msg.via_pos.resize(send_msg.via_num);
    for(int p = 0; p < send_msg.via_num; p++)
    {
        send_msg.via_pos[p].resize(3, 0);
    }

    send_msg.via_pos[0][0] = 0;
    send_msg.via_pos[0][1] = 0;
    send_msg.via_pos[0][2] = 0;

    send_msg.via_pos[1][0] = 1.0;
    send_msg.via_pos[1][1] = 0;
    send_msg.via_pos[1][2] = 0;

    // set loc points for test
    send_msg.loc_num = 1;
    send_msg.loc_name.resize(send_msg.loc_num);
    send_msg.loc_pos.resize(send_msg.loc_num);
    send_msg.loc_name[0] = "table_1";

    send_msg.loc_pos[0].resize(3);
    send_msg.loc_pos[0][0] = -1.0;
    send_msg.loc_pos[0][1] = 0.0;
    send_msg.loc_pos[0][2] = M_PI/4;

    lcm.publish("MAP_DATA", &send_msg);
    printf("PUB(MAP_DATA): %s, %d, %d\n", send_msg.map_name.data(), send_msg.data[send_msg.len-2], send_msg.data[send_msg.len-1]);
}

void MainWindow::example_callback(const lcm::ReceiveBuffer *rbuf, const std::string &chan, const example_t *msg)
{
    printf("SUB(EXAMPLE): %f\n", msg->time);
}

void MainWindow::map_data_callback(const lcm::ReceiveBuffer *rbuf, const std::string &chan, const map_data_t *msg)
{
    printf("SUB(MAP_DATA): %s, %d, %d\n", msg->map_name.data(), msg->data[msg->len-2], msg->data[msg->len-1]);

    // draw received map
    cv::Mat map(msg->map_h, msg->map_w, CV_8U, cv::Scalar::all(0));
    memcpy(map.data, msg->data.data(), msg->len);

    cv::Mat plot_map;
    cv::cvtColor(map, plot_map, cv::COLOR_GRAY2BGR);

    // draw received metadata
    {
        int u0 = std::round((msg->via_pos[0][0]/msg->map_grid_w) + msg->map_origin[0]);
        int v0 = std::round((msg->via_pos[0][1]/msg->map_grid_w) + msg->map_origin[1]);

        int u1 = std::round((msg->via_pos[1][0]/msg->map_grid_w) + msg->map_origin[0]);
        int v1 = std::round((msg->via_pos[1][1]/msg->map_grid_w) + msg->map_origin[1]);

        cv::line(plot_map, cv::Point(u0,v0), cv::Point(u1,v1), cv::Scalar(0,255,0), 2);
    }

    {
        int u0 = std::round((msg->loc_pos[0][0]/msg->map_grid_w) + msg->map_origin[0]);
        int v0 = std::round((msg->loc_pos[0][1]/msg->map_grid_w) + msg->map_origin[1]);

        cv::circle(plot_map, cv::Point(u0,v0), 10, cv::Scalar(0,0,255), 1);
    }

    // flip and rotation
    cv::flip(plot_map, plot_map, 0);
    cv::rotate(plot_map, plot_map, cv::ROTATE_90_COUNTERCLOCKWISE); // image north is +x axis

    // simple drawing
    ui->lb_Screen1->setPixmap(QPixmap::fromImage(mat_to_qimage_cpy(plot_map)));
    ui->lb_Screen1->setScaledContents(true);
    ui->lb_Screen1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

void MainWindow::lcmLoop()
{
    /*
    when pc reboot type to terminal

    sudo ifconfig lo multicast
    sudo route add -net 224.0.0.0 netmask 240.0.0.0 dev lo
    sudo sysctl -w net.core.rmem_max=20000000
    sudo sysctl -w net.core.netdev_max_backlog=2000

    http://lcm-proj.github.io/multicast_setup.html
    */

    // lcm init
    if(lcm.good())
    {
        lcm.subscribe("EXAMPLE", &MainWindow::example_callback, this);
        lcm.subscribe("MAP_DATA", &MainWindow::map_data_callback, this);
    }
    else
    {
        printf("lcm init failed\n");
    }

    while(lcmFlag)
    {
        lcm.handleTimeout(1);
    }
}

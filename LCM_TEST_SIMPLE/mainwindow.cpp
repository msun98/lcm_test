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
    connect(ui->bt_robot, SIGNAL(clicked()), this, SLOT(bt_robot()));
    connect(ui->bt_target_pose, SIGNAL(clicked()), this, SLOT(bt_target_pose()));
    connect(ui->bt_status, SIGNAL(clicked()), this, SLOT(bt_status()));
    connect(ui->bt_command, SIGNAL(clicked()), this, SLOT(bt_command()));

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
    //data 에 메모리 할당을 하기 위함.0 <- 초기화의 의미, 쓰레기 값 들어가지 않도록 하기 위함.
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
//    send_msg.via_pos[0][2] = 0;

    send_msg.via_pos[1][0] = 1.0;
    send_msg.via_pos[1][1] = 0;
//    send_msg.via_pos[1][2] = 0;

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
    //msg->data[msg->len-2], msg->data[msg->len-1] 잘 날아가는지 확인하기 위해 만든 내용이므로 없어도 됨.

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
    sudo sysctl -w net.core.rmem_max=20000000 //메모리 늘리는 부분 (만약 데이터가 커서 제대로 안날아가면 터미널 창에 치면 늘어난다.)
    sudo sysctl -w net.core.netdev_max_backlog=2000 //메모리 늘리는 부분 (만약 데이터가 커서 제대로 안날아가면 터미널 창에 치면 늘어난다.)

    http://lcm-proj.github.io/multicast_setup.html
    */

    // lcm init
    if(lcm.good())
    {
        lcm.subscribe("EXAMPLE", &MainWindow::example_callback, this);
        lcm.subscribe("MAP_DATA", &MainWindow::map_data_callback, this);
        lcm.subscribe("ROBOT_DATA", &MainWindow::robot_data_callback, this);
        lcm.subscribe("TARGET_DATA", &MainWindow::target_pose_callback, this);
        lcm.subscribe("STATUS_DATA", &MainWindow::robot_status_callback, this);
        lcm.subscribe("COMMAND_DATA", &MainWindow::command_callback, this);

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
///////////////////////////////////////////////////////////////////////
void MainWindow::bt_robot()
{
    // send structure
    robot_pose send_msg;

    // map metadata
    send_msg.robot_x = 50;
    send_msg.robot_y = 10; // 2.5cm
    send_msg.robot_theta = 0.5;


    lcm.publish("ROBOT_DATA", &send_msg);
    printf("PUB(ROBOT_DATA): %f \n", send_msg.robot_x);

//    std::cout<<send_msg.robot_x <<std::endl;

}


void MainWindow::robot_data_callback(const lcm::ReceiveBuffer *rbuf, const std::string &chan, const robot_pose *msg)
{
    printf("SUB(ROBOT_DATA): %.4f,%.4f,%.4f \n", msg->robot_x, msg->robot_y, msg->robot_theta);
}

///////////////////////////////////////////////////////////////////////
void MainWindow::bt_target_pose()
{
    // send structure
    target_pose send_msg;

    // map metadata
    send_msg.robot_target_x = 50;
    send_msg.robot_target_y = 100; // 2.5cm
    send_msg.robot_target_theta = 0.5;


    lcm.publish("TARGET_DATA", &send_msg);
    printf("PUB(TARGET_DATA): %f \n", send_msg.robot_target_x);

//    std::cout<<send_msg.robot_x <<std::endl;

}


void MainWindow::target_pose_callback(const lcm::ReceiveBuffer *rbuf, const std::string &chan, const target_pose *msg)
{
    printf("SUB(TARGET_DATA): %.4f,%.4f,%.4f \n", msg->robot_target_x, msg->robot_target_y, msg->robot_target_theta);
}


///////////////////////////////////////////////////////////////////////
void MainWindow::bt_status()
{
    // send structure
    robot_status send_msg;

    // map metadata
    send_msg.bat = 50;
    send_msg.state = 3; // 2.5cm
    send_msg.err_code = 3;

    lcm.publish("STATUS_DATA", &send_msg);
//    printf("PUB(STATUS_DATA): %f \n", send_msg.bat);

//    std::cout<<send_msg.robot_x <<std::endl;

}


void MainWindow::robot_status_callback(const lcm::ReceiveBuffer *rbuf, const std::string &chan, const robot_status *msg)
{
    printf("SUB(STATUS_DATA): %.4f, %d, %d\n", msg->bat , msg->state, msg->err_code);
}

///////////////////////////////////////////////////////////////////////
void MainWindow::bt_command()
{

    // send structure
    command send_msg;

//    qDebug()<<ui->cb_cmd->currentIndex();
    // map metadata
    send_msg.cmd = ui->cb_cmd->currentIndex()+1;
    std::fill(send_msg.params,send_msg.params+255,0); //배열 0으로 초기화

    if (send_msg.cmd == 1)
    {
        send_msg.params[0] = 2;
    }

    else if (send_msg.cmd == 2)
    {

        //256이 넘어가게 되면 배열의 칸을 하나 더 넘어가게 되므로 memcpy를 써서 칸 수를 맞춰주는 작업을 진행
        int params_1=256;
        int params_2=2;
        int params_3=5;

        QByteArray bytes1 = QByteArray::fromRawData(reinterpret_cast<const char *>(&params_1), sizeof(params_1));
        QByteArray bytes2 = QByteArray::fromRawData(reinterpret_cast<const char *>(&params_2), sizeof(params_2));
        QByteArray bytes3 = QByteArray::fromRawData(reinterpret_cast<const char *>(&params_3), sizeof(params_3));

        memcpy(&(send_msg.params[0]),&params_1,4);
        memcpy(&(send_msg.params[4]),&params_2,4);
        memcpy(&(send_msg.params[8]),&params_3,4);
////        memcpy(&(send_msg.params[0]),bytes1.data(),4);
//        memcpy(&(send_msg.params[4]),bytes2.data(),4);
//        memcpy(&(send_msg.params[8]),bytes3.data(),4);


//        printf("PUB(COMMAND_DATA): %d\n", send_msg.params);

        printf("PUB(COMMAND_DATA): %d, %d, %d\n", send_msg.params[0],send_msg.params[4],send_msg.params[8]);

    }


    else if (send_msg.cmd == 3)
    {
        float params_1=1.1;
        float params_2=2.5;
        float params_3=5.5;

        QByteArray bytes1;
        memcpy(&bytes1,&params_1,sizeof(params_1));
//        QByteArray bytes1 = QByteArray::fromRawData(reinterpret_cast<const char *>(&temp), sizeof(temp));
//        QByteArray bytes2 = QByteArray::fromRawData(reinterpret_cast<const char *>(&params_2), sizeof(params_2));
//        QByteArray bytes3 = QByteArray::fromRawData(reinterpret_cast<const char *>(&params_3), sizeof(params_3));

//        memcpy(&(send_msg.params[0]),bytes1.data(),4);
//        memcpy(&(send_msg.params[4]),bytes2.data(),4);
//        memcpy(&(send_msg.params[8]),bytes3.data(),4);

        qDebug()<<bytes1;

//        printf("PUB(COMMAND_DATA): %f, %f, %f\n", send_msg.params[0],send_msg.params[4],send_msg.params[8]);
    }
//    send_msg.params[2] = 5;

    //data 에 메모리 할당을 하기 위함.0 <- 초기화의 의미, 쓰레기 값 들어가지 않도록 하기 위함.
//    memcpy(send_msg.data.data(), map.data, map.rows*map.cols);

    lcm.publish("COMMAND_DATA", &send_msg);
//    printf("PUB(COMMAND_DATA): %d, %f, %f\n", send_msg.params[2],send_msg.params[3],send_msg.params[8]);

//    std::cout<<send_msg.robot_x <<std::endl;

}

void MainWindow::command_callback(const lcm::ReceiveBuffer *rbuf, const std::string &chan, const command *msg)
{

    if (msg->cmd == 1)
    {
        if(msg->params[0] == 1)
        {
            printf("SUB(COMMAND_DATA): charge \n");
        }

        else if(msg->params[0] == 2)
        {
            printf("SUB(COMMAND_DATA): wait point \n");
        }

        else if(msg->params[0] == 3)
        {
            printf("SUB(COMMAND_DATA): tables \n");
        }
    }

    else if(msg->cmd == 2)
    {
//        QByteArray bytes1 = QByteArray::fromRawData(reinterpret_cast<const char *>(&params_1), sizeof(params_1));
//        QByteArray bytes2 = QByteArray::fromRawData(reinterpret_cast<const char *>(&params_2), sizeof(params_2));
//        QByteArray bytes3 = QByteArray::fromRawData(reinterpret_cast<const char *>(&params_3), sizeof(params_3));

//        memcpy(&(send_msg.params[0]),bytes1.data(),4);
//        memcpy(&(send_msg.params[4]),bytes2.data(),4);
//        memcpy(&(send_msg.params[8]),bytes3.data(),4);
         QByteArray bytes[4];
         printf("SUB(COMMAND_DATA): %d, %d, %d\n", msg->params[0],msg->params[4],msg->params[8]);

//         memcpy(bytes.data(),&(send_msg.params),4);
//         printf("%d\n", bytes);

//         printf("SUB(COMMAND_DATA): %d ,%d, %d \n",msg->params[0],msg->params[4],msg->params[8]);
    }


}


//    printf("SUB(COMMAND_DATA): %d \n", msg->cmd);



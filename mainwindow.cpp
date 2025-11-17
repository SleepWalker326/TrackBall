// MainWindow.cpp
#include "mainwindow.h"
#include <QTime>
#include <QDebug>
const char* MainWindow::REMOTE_IP = "192.168.1.100";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_videoPlayer(new VideoPlayer()),
      m_isVideoPlaying(false)
{
    // 初始化UDP
    udpSocket = new QUdpSocket(this);
    if (!udpSocket->bind(QHostAddress::AnyIPv4, UDP_PORT_LOCAL)) {
        QMessageBox::critical(this, "错误", "无法绑定UDP端口: " + QString::number(UDP_PORT_LOCAL));
    }
    createUI();
    createConnections();

    m_mediaRecorder = new MediaRecorder(this);

    setWindowTitle("智能光电球显示界面");
    resize(1000, 715);
}

MainWindow::~MainWindow()
{
    delete m_videoPlayer;
    delete udpSocket;

}

void MainWindow::createUI()
{
    // 创建中心部件
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);

    // 创建主显示区域和右侧控制面板
    createMainArea();
    mainLayout->addWidget(m_mainDisplayWidget);

    // 创建底部控制按钮区域
    createControlButtons();
    QWidget *bottomControlWidget = new QWidget();
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomControlWidget);
    bottomLayout->addWidget(m_connectionBtn);
    bottomLayout->addWidget(m_startDetectionBtn);
    bottomLayout->addWidget(m_multiTargetTrackBtn);
    bottomLayout->addWidget(m_singleTargetTrackBtn);
    bottomLayout->addWidget(m_startRecordBtn);
    bottomLayout->addWidget(m_screenshotBtn);

    mainLayout->addWidget(bottomControlWidget);

    // 创建状态栏
    statusBar();

}

void MainWindow::createMainArea()
{
    // 创建主显示区域容器
    m_mainDisplayWidget = new QWidget();
    QHBoxLayout *mainDisplayLayout = new QHBoxLayout(m_mainDisplayWidget);

    // 创建左侧显示区域
    QWidget *leftDisplayWidget = new QWidget();
    leftDisplayWidget->setFixedSize(671, 591);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftDisplayWidget);

    // 创建信息显示区域
    QWidget *infoArea = createInfoArea();
    leftLayout->addWidget(infoArea);

    // 创建视频显示区域
    createDisplayArea();
    leftLayout->addWidget(m_displayImageLabel, 0, Qt::AlignCenter);

    // 创建右侧控制面板
    createControlPanel();

    // 添加到主布局
    mainDisplayLayout->addWidget(leftDisplayWidget);
    mainDisplayLayout->addWidget(m_controlPanelWidget);
}

QWidget* MainWindow::createInfoArea()
{
    QWidget *infoWidget = new QWidget();
    infoWidget->setFixedSize(641, 61);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoWidget);

    // 帧率信息
    QWidget *frameRateWidget = new QWidget();
    frameRateWidget->setFixedSize(75, 60);
    QVBoxLayout *frameRateLayout = new QVBoxLayout(frameRateWidget);
    m_frameRateLabel = new QLabel("帧率");
    m_frameRateValueLabel = new QLabel("0fps");
    m_frameRateLabel->setAlignment(Qt::AlignCenter);
    m_frameRateValueLabel->setAlignment(Qt::AlignCenter);
    frameRateLayout->addWidget(m_frameRateLabel);
    frameRateLayout->addWidget(m_frameRateValueLabel);
    // 分辨率信息
    QWidget *resolutionWidget = new QWidget();
    resolutionWidget->setFixedSize(100, 60);
    QVBoxLayout *resolutionLayout = new QVBoxLayout(resolutionWidget);
    m_resolutionLabel = new QLabel("分辨率");
    m_resolutionValueLabel = new QLabel("640*512");
    m_resolutionValueLabel->setFixedWidth(95);
    m_resolutionLabel->setAlignment(Qt::AlignCenter);
    m_resolutionValueLabel->setAlignment(Qt::AlignCenter);
    resolutionLayout->addWidget(m_resolutionLabel);
    resolutionLayout->addWidget(m_resolutionValueLabel);

    // 时间戳信息
    QWidget *timestampWidget = new QWidget();
    timestampWidget->setFixedSize(100, 60);
    QVBoxLayout *timestampLayout = new QVBoxLayout(timestampWidget);
    m_timestampLabel = new QLabel("时间戳");
    m_timestampValueLabel = new QLabel("00：00：00");
    m_timestampLabel->setAlignment(Qt::AlignCenter);
    m_timestampValueLabel->setAlignment(Qt::AlignCenter);
    timestampLayout->addWidget(m_timestampLabel);
    timestampLayout->addWidget(m_timestampValueLabel);

    infoLayout->addWidget(frameRateWidget);
    infoLayout->addWidget(resolutionWidget);
    infoLayout->addWidget(timestampWidget);

    return infoWidget;
}

void MainWindow::createDisplayArea()
{
    QWidget *displayContainer = new QWidget();
    displayContainer->setFixedSize(640, 512);
    QVBoxLayout *containerLayout = new QVBoxLayout(displayContainer);

    m_displayImageLabel = new QLabel("显示图像");
    m_displayImageLabel->setFixedSize(640, 512);
    m_displayImageLabel->setAlignment(Qt::AlignCenter);
    m_displayImageLabel->setStyleSheet("border: 1px solid gray; background-color: #f0f0f0;");

    containerLayout->addWidget(m_displayImageLabel);
}

void MainWindow::createControlPanel()
{
    m_controlPanelWidget = new QWidget();
    m_controlPanelWidget->setFixedSize(291, 591);
    QVBoxLayout *controlLayout = new QVBoxLayout(m_controlPanelWidget);

    // 创建云台控制
    createGimbalControl();
    controlLayout->addWidget(m_gimbalGroup);

    // 创建镜头控制
    createLensControl();
    controlLayout->addWidget(m_lensGroup);

    // 创建图像控制
    createImageControl();
    controlLayout->addWidget(m_imageGroup);

    controlLayout->addStretch();
}

void MainWindow::createGimbalControl()
{
    m_gimbalGroup = new QGroupBox("云台控制");
    m_gimbalGroup->setFixedSize(271, 251);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_gimbalGroup);

    // 方向控制区域
    QWidget *directionWidget = new QWidget();
    directionWidget->setFixedSize(251, 90);
    QGridLayout *directionLayout = new QGridLayout(directionWidget);

    m_gimbalUpBtn = new QPushButton("上");
    m_gimbalLeftBtn = new QPushButton("左");
    m_gimbalDownBtn = new QPushButton("下");
    m_gimbalRightBtn = new QPushButton("右");
    m_gimbalStopBtn = new QPushButton("停止");

    // 设置方形按钮
    m_gimbalUpBtn->setFixedSize(40, 40);
    m_gimbalLeftBtn->setFixedSize(40, 40);
    m_gimbalDownBtn->setFixedSize(40, 40);
    m_gimbalRightBtn->setFixedSize(40, 40);
    m_gimbalStopBtn->setFixedSize(90, 40);

    // 设置按钮位置
    directionLayout->addWidget(m_gimbalUpBtn, 0, 1);
    directionLayout->addWidget(m_gimbalLeftBtn, 1, 0);
    directionLayout->addWidget(m_gimbalDownBtn, 1, 1);
    directionLayout->addWidget(m_gimbalRightBtn, 1, 2);
    directionLayout->addWidget(m_gimbalStopBtn, 1, 3);

    // 参数设置区域
    QWidget *paramWidget = new QWidget();
    paramWidget->setFixedSize(251, 90);
    QGridLayout *paramLayout = new QGridLayout(paramWidget);

    // 移动速度
    m_moveSpeedLabel = new QLabel("移动速度");
    m_moveSpeedLabel->setFixedSize(70, 25);
    m_moveSpeedLabel->setAlignment(Qt::AlignCenter);
    m_moveSpeedEdit = new QLineEdit();
    m_moveSpeedEdit->setFixedSize(160, 25);

    // 角度步进
    m_angleStepLabel = new QLabel("角度步进");
    m_angleStepEdit = new QLineEdit();
    m_angleStepLabel->setFixedSize(70, 25);
    m_angleStepLabel->setAlignment(Qt::AlignCenter);
    m_angleStepEdit->setFixedSize(160, 25);
    paramLayout->addWidget(m_moveSpeedLabel,0,0);
    paramLayout->addWidget(m_moveSpeedEdit,0,1);
    paramLayout->addWidget(m_angleStepLabel,1,0);
    paramLayout->addWidget(m_angleStepEdit,1,1);

    // 功能按钮区域
    QWidget *functionWidget = new QWidget();
    functionWidget->setFixedSize(251, 41);
    QHBoxLayout *functionLayout = new QHBoxLayout(functionWidget);

    m_gimbalResetBtn = new QPushButton("云台归零");
    m_autoScanBtn = new QPushButton("自动扫描");

    functionLayout->addWidget(m_gimbalResetBtn);
    functionLayout->addWidget(m_autoScanBtn);

    // 添加到主布局
    mainLayout->addWidget(directionWidget);
    mainLayout->addWidget(paramWidget);
    mainLayout->addWidget(functionWidget);
}

void MainWindow::createLensControl()
{
    m_lensGroup = new QGroupBox("镜头控制");
    m_lensGroup->setFixedSize(271, 151);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_lensGroup);

    // 变焦和视场控制
    QWidget *controlWidget = new QWidget();
    controlWidget->setFixedSize(251, 71);
    QGridLayout *controlLayout = new QGridLayout(controlWidget);

    // 变焦控制
    m_zoomLabel = new QLabel("变焦");
    m_zoomLabel->setFixedSize(45, 30);
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    m_zoomInBtn = new QPushButton("+");
    m_zoomOutBtn = new QPushButton("-");
    m_zoomInBtn->setFixedSize(28, 28);
    m_zoomOutBtn->setFixedSize(28, 28);

    // 视场控制
    m_fovLabel = new QLabel("视场");
    m_fovLabel->setFixedSize(45, 30);
    m_fovLabel->setAlignment(Qt::AlignCenter);
    m_fovLargeBtn = new QPushButton("大");
    m_fovSmallBtn = new QPushButton("小");
    m_fovLargeBtn->setFixedSize(28, 28);
    m_fovSmallBtn->setFixedSize(28, 28);

    controlLayout->addWidget(m_zoomLabel,0,0);
    controlLayout->addWidget(m_zoomInBtn,0,1);
    controlLayout->addWidget(m_zoomOutBtn,0,2);
    controlLayout->addWidget(m_fovLabel,1,0);
    controlLayout->addWidget(m_fovLargeBtn,1,1);
    controlLayout->addWidget(m_fovSmallBtn,1,2);

    // 功能按钮
    QWidget *functionWidget = new QWidget();
    functionWidget->setFixedSize(251, 41);
    QHBoxLayout *functionLayout = new QHBoxLayout(functionWidget);

    m_autoFocusBtn = new QPushButton("自动聚焦");
    m_lensResetBtn = new QPushButton("一键复位");

    functionLayout->addWidget(m_autoFocusBtn);
    functionLayout->addWidget(m_lensResetBtn);

    // 添加到主布局
    mainLayout->addWidget(controlWidget);
    mainLayout->addWidget(functionWidget);
}

void MainWindow::createImageControl()
{
    m_imageGroup = new QGroupBox("图像控制");
    m_imageGroup->setFixedSize(271, 161);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_imageGroup);

    // 模式选择
    QWidget *modeWidget = new QWidget();
    modeWidget->setFixedSize(251, 51);
    QHBoxLayout *modeLayout = new QHBoxLayout(modeWidget);

    m_imageTypeCombo = new QComboBox();
    m_imageTypeCombo->addItem("红外");
    m_imageTypeCombo->addItem("可见光");

    m_videoSourceCombo = new QComboBox();
    m_videoSourceCombo->addItem("推流视频");
    m_videoSourceCombo->addItem("本地视频");

    modeLayout->addWidget(m_imageTypeCombo);
    modeLayout->addWidget(m_videoSourceCombo);

    // 亮度和对比度控制
    QWidget *adjustWidget = new QWidget();
    adjustWidget->setFixedSize(251, 71);
    QGridLayout *adjustLayout = new QGridLayout(adjustWidget);

    // 亮度控制
    m_brightnessLabel = new QLabel("亮度");
    m_brightnessLabel->setFixedSize(67, 30);
    m_brightnessLabel->setAlignment(Qt::AlignCenter);
    m_brightnessUpBtn = new QPushButton("+");
    m_brightnessDownBtn = new QPushButton("-");
    m_brightnessUpBtn->setFixedSize(28, 28);
    m_brightnessDownBtn->setFixedSize(28, 28);

    // 对比度控制
    m_contrastLabel = new QLabel("对比度");
    m_contrastLabel->setFixedSize(67, 30);
    m_contrastLabel->setAlignment(Qt::AlignCenter);
    m_contrastUpBtn = new QPushButton("+");
    m_contrastDownBtn = new QPushButton("-");
    m_contrastUpBtn->setFixedSize(28, 28);
    m_contrastDownBtn->setFixedSize(28, 28);

    adjustLayout->addWidget(m_brightnessLabel,0,0);
    adjustLayout->addWidget(m_brightnessUpBtn,0,1);
    adjustLayout->addWidget(m_brightnessDownBtn,0,2);
    adjustLayout->addWidget(m_contrastLabel,1,0);
    adjustLayout->addWidget(m_contrastUpBtn,1,1);
    adjustLayout->addWidget(m_contrastDownBtn,1,2);
    // 添加到主布局
    mainLayout->addWidget(adjustWidget);
    mainLayout->addWidget(modeWidget);
}

void MainWindow::createControlButtons()
{
    m_connectionBtn = new QPushButton("连接");
    m_startDetectionBtn = new QPushButton("开始检测");
    m_multiTargetTrackBtn = new QPushButton("多目标跟踪");
    m_singleTargetTrackBtn = new QPushButton("单目标跟踪");
    m_startRecordBtn = new QPushButton("开始录制");
    m_screenshotBtn = new QPushButton("截图");
}

void MainWindow::createConnections()
{
    connect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::readPendingDatagrams);
    connect(m_startDetectionBtn, &QPushButton::clicked, this, &MainWindow::onStartDetectClicked);

//    // 测试用：点击按钮更新信息
//    connect(m_startDetectionBtn, &QPushButton::clicked, [this]() {
//        updateFrameRate("30fps");
//        updateResolution("640*512");
//        updateTimestamp(QTime::currentTime().toString("hh：mm：ss"));
//    });

    // 视频相关连接
    connect(m_connectionBtn, &QPushButton::clicked, [this]() {
        if (!m_isVideoPlaying) {
            // 从JSON文件读取RTSP地址 (假设配置文件为rtsp_config.json)
            QString rtspUrl = m_videoPlayer->readRtspUrlFromJson(filePath);
            if (rtspUrl.isEmpty()) return;

                m_videoPlayer->startPlayback(rtspUrl);
                auto thread = m_videoPlayer->getCaptureThread();
                if (!thread) return;

                connect(thread, &VideoCaptureThread::frameCaptured,
                        this, &MainWindow::onFrameCaptured);
                connect(thread, &VideoCaptureThread::errorOccurred,
                        this, &MainWindow::onVideoError);

                m_connectionBtn->setText("断开");
                m_isVideoPlaying = true;
        }
        else {
//            if (m_frameConnection) {
//                disconnect(m_frameConnection);
//                m_frameConnection = QMetaObject::Connection();
//            }
//            if (m_errorConnection) {
//                disconnect(m_errorConnection);
//                m_errorConnection = QMetaObject::Connection();
//            }
            m_videoPlayer->stopPlayback();
            // 清除显示的图像，恢复初始文本
//            m_displayImageLabel->setPixmap(QPixmap());  // 清除现有图像
            m_displayImageLabel->clear();
            m_displayImageLabel->setText("显示图像");
            m_connectionBtn->setText("连接");
            m_isVideoPlaying = false;
        }
    });

    // 截图按钮
    connect(m_screenshotBtn, &QPushButton::clicked, [this]() {
        if (m_displayImageLabel->pixmap()) {
            QImage image = m_displayImageLabel->pixmap()->toImage();
            bool success = m_mediaRecorder->saveScreenshot(image);
            if (success) {
                QMessageBox::information(this, "成功", "截图保存成功");
            } else {
                QMessageBox::warning(this, "失败", "截图保存失败");
            }
        }
    });

    // 录制按钮
    connect(m_startRecordBtn, &QPushButton::clicked, [this]() {
        if (!m_isVideoPlaying) {
            QMessageBox::warning(this, "提示", "请先连接视频");
            return;
        }

        if (!m_mediaRecorder->isRecording()) {
            // 设置录制参数
            auto thread = m_videoPlayer->getCaptureThread();
            if (thread) {
                m_mediaRecorder->setRecordParams(
                    thread->getFrameWidth(),
                    thread->getFrameHeight(),
                    thread->getFrameRate()
                );
            }
            bool success = m_mediaRecorder->startRecording();
            if (success) {
                m_startRecordBtn->setText("停止录制");
            }
        } else {
            m_mediaRecorder->stopRecording();
            m_startRecordBtn->setText("开始录制");
            QMessageBox::information(this, "成功", "视频录制已保存");
        }
    });


}

// 更新帧率显示
void MainWindow::updateFrameRate(const QString &frameRate)
{
    m_frameRateValueLabel->setText(frameRate);
}

// 更新分辨率显示
void MainWindow::updateResolution(const QString &resolution)
{
    m_resolutionValueLabel->setText(resolution);
}

// 更新时间戳显示
void MainWindow::updateTimestamp(const QString &timestamp)
{
    m_timestampValueLabel->setText(timestamp);
}


void MainWindow::onFrameCaptured(cv::Mat frame) {
//    if (frame.empty()) return;

//    // 转换为QImage
//    QImage img = VideoPlayer::cvMatToQImage(frame);
//    if (img.isNull()) return;

//    // 转换为QPixmap并调整大小
//    QPixmap pixmap = QPixmap::fromImage(img);
//    pixmap = pixmap.scaled(m_displayImageLabel->size(),
//                          Qt::KeepAspectRatio,
//                          Qt::SmoothTransformation);

//    // 显示图像
//    m_displayImageLabel->setPixmap(pixmap);

    //QLable是640*512,如果大于该尺寸则只显示正中心的640*512
    // 假设frame是1920x1080的cv::Mat
    if (frame.empty()) return;

    // 计算裁剪区域
    int cropWidth = m_displayImageLabel->width();
    int cropHeight = m_displayImageLabel->height();
    int startX = (frame.cols - cropWidth) / 2;
    int startY = (frame.rows - cropHeight) / 2;

    // 确保裁剪区域有效
    startX = std::max(0, startX);
    startY = std::max(0, startY);
    cropWidth = std::min(cropWidth, frame.cols - startX);
    cropHeight = std::min(cropHeight, frame.rows - startY);

    // 裁剪中心区域
    cv::Mat croppedFrame = frame(cv::Rect(startX, startY, cropWidth, cropHeight));

    // 转换为QImage并显示
    QImage img = VideoPlayer::cvMatToQImage(croppedFrame);
    if (!img.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(img);
        m_displayImageLabel->setPixmap(pixmap);
    }

    // 更新视频信息
    if (m_videoPlayer->getCaptureThread()) {
        updateFrameRate(QString("%1fps").arg((int)m_videoPlayer->getCaptureThread()->getFrameRate()));
        updateResolution(QString("%1*%2")
                           .arg(m_videoPlayer->getCaptureThread()->getFrameWidth())
                           .arg(m_videoPlayer->getCaptureThread()->getFrameHeight()));
    }
    updateTimestamp(QTime::currentTime().toString("hh：mm：ss"));

    // 如果正在录制，写入帧数据
    if (m_mediaRecorder->isRecording()) {
        m_mediaRecorder->writeFrame(frame);
    }
}

void MainWindow::onVideoError(QString error) {
    QMessageBox::critical(this, "视频错误", error);
    m_connectionBtn->setText("连接");
    m_isVideoPlaying = false;
}

//开始检测按钮的实现
/*
void MainWindow::onStartDetectClicked()
{
    if (!isDetecting) {
        // 开始检测
        sendStartDetectCommand();
        m_startDetectionBtn->setText("停止检测");

        isDetecting = true;
    } else {
        // 停止检测
        sendStopDetectCommand();
        m_startDetectionBtn->setText("开始检测");

        isDetecting = false;
    }
}
*/

void MainWindow::sendStartDetectCommand()
{
    unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};

    // 帧头
    sendBuffer[0] = 0xC1;
    // 命令ID - 开始检测
    sendBuffer[1] = tarDetectBack;

    sendBuffer[2] = startDetect;

    // 计算校验和 (从第1字节开始，共SENDBUFFER_SIZE_UDP-2字节)
    sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[1], SENDBUFFER_SIZE_UDP - 2);

    // 发送UDP数据报并检查返回值
    qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                               QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);

    if (bytesSent == SENDBUFFER_SIZE_UDP) {
        qDebug() << "UDP发送成功，发送字节数:" << bytesSent;
        qDebug() << "目标IP:" << REMOTE_IP << "端口:" << UDP_PORT_REMOTE;
        qDebug() << "命令: tarDetectBack - startDetect";
    } else if (bytesSent == -1) {
        qDebug() << "UDP发送失败! 错误:" << udpSocket->errorString();
    } else {
        qDebug() << "UDP发送不完整，期望发送:" << SENDBUFFER_SIZE_UDP << "实际发送:" << bytesSent;
    }
}

void MainWindow::sendStopDetectCommand()
{
    unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};

    // 帧头
    sendBuffer[0] = 0xC1;
    // 命令ID - 停止检测
    sendBuffer[1] = tarDetectBack;
    sendBuffer[2] = stopDetect;

    // 计算校验和
    sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[1], SENDBUFFER_SIZE_UDP - 2);

    // 发送UDP数据报
    udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                            QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
}

unsigned char MainWindow::CalCheckNum(unsigned char data[], int num)
{
    unsigned char res = 0;
    for (int i = 0; i < num; i++) {
        res += data[i];
    }
    return res;
}

void MainWindow::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        parseReceivedData(datagram);
    }
}

void MainWindow::parseReceivedData(const QByteArray &data)
{
    if (data.size() < 64) return; // 数据长度不足

    const unsigned char *buffer = reinterpret_cast<const unsigned char*>(data.constData());

    // 验证帧头
    if (buffer[0] != 0xC3) return;

    // 验证校验和
    unsigned char checksum = CalCheckNum(const_cast<unsigned char*>(&buffer[1]), data.size() - 2);
    if (checksum != buffer[data.size() - 1]) return;

    // 解析命令ID
    unsigned char cmdId = buffer[1];

    if (cmdId == StateSend_DC) {
        // 解析视场角信息
        FLOAT2CHAR ftemp;

        // 可见光视场角
        memcpy(ftemp.arr, &buffer[2], 4);
        visFov = ftemp.val;

        // 红外视场角
        memcpy(ftemp.arr, &buffer[6], 4);
        irFov = ftemp.val;

        // 目标数量
        int targetCount = buffer[10];
        detectedTargets.clear();

        // 解析每个目标信息
        for (int i = 0; i < targetCount && i < 20; i++) { // 限制最大20个目标
            TargetInfo target;
            int baseIndex = 11 + i * 19;

            if (baseIndex + 18 >= data.size()) break; // 防止越界

            target.id = i + 1;
            target.targetClass = buffer[baseIndex + 1];

            // 解析X坐标
            unsigned int cx = 0;
            cx |= (unsigned int)buffer[baseIndex + 3] << 24;
            cx |= (unsigned int)buffer[baseIndex + 4] << 16;
            cx |= (unsigned int)buffer[baseIndex + 5] << 8;
            cx |= (unsigned int)buffer[baseIndex + 6];
            target.cx = cx;

            // 解析Y坐标
            unsigned int cy = 0;
            cy |= (unsigned int)buffer[baseIndex + 7] << 24;
            cy |= (unsigned int)buffer[baseIndex + 8] << 16;
            cy |= (unsigned int)buffer[baseIndex + 9] << 8;
            cy |= (unsigned int)buffer[baseIndex + 10];
            target.cy = cy;

            // 解析方位角
            memcpy(ftemp.arr, &buffer[baseIndex + 11], 4);
            target.az = ftemp.val;

            // 解析俯仰角
            memcpy(ftemp.arr, &buffer[baseIndex + 15], 4);
            target.pi = ftemp.val;

            detectedTargets.append(target);

            QString info = QString("目标 %1: 类别=%2, 坐标=(%3, %4), 方位角=%5, 俯仰角=%6")
                            .arg(target.id)
                            .arg(target.targetClass)
                            .arg(target.cx)
                            .arg(target.cy)
                            .arg(target.az)
                            .arg(target.pi);
            qDebug() << info;
        }
    }
}

//切换视频按钮
void MainWindow::onStartDetectClicked()
{
    if (!m_isVideoPlaying) {
        QMessageBox::warning(this, "提示", "请先连接视频");
        return;
    } else {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};

        // 帧头
        sendBuffer[0] = 0xC1;
        // 命令ID - 开始检测
        sendBuffer[1] = (enum SendEnum)0x0A;

        sendBuffer[2] = (enum videoDisplayEnum)0x02;//1是红外，2是可见光

        // 计算校验和 (从第1字节开始，共SENDBUFFER_SIZE_UDP-2字节)
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[1], SENDBUFFER_SIZE_UDP - 2);

        // 发送UDP数据报并检查返回值
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                                   QHostAddress("192.168.1.100"), 40213);

        if (bytesSent == SENDBUFFER_SIZE_UDP) {
            qDebug() << "UDP发送成功，发送字节数:" << bytesSent;
            qDebug() << "目标IP:" << REMOTE_IP << "端口:" << UDP_PORT_REMOTE;
            qDebug() << "命令: videoDisplayBack - irDis";
        } else if (bytesSent == -1) {
            qDebug() << "UDP发送失败! 错误:" << udpSocket->errorString();
        } else {
            qDebug() << "UDP发送不完整，期望发送:" << SENDBUFFER_SIZE_UDP << "实际发送:" << bytesSent;
        }
    }
}



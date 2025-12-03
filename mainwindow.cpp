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
    resize(1500, 815);
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

    // 创建右侧控制面板和表格区域
    createControlPanel();

    // 添加到主布局
    mainDisplayLayout->addWidget(leftDisplayWidget);
    mainDisplayLayout->addWidget(m_rightPanelWidget);  // 改为右侧整体面板
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
    // 创建右侧整体面板
    m_rightPanelWidget = new QWidget();
    m_rightPanelWidget->setFixedSize(601, 591);
    QVBoxLayout *rightPanelLayout = new QVBoxLayout(m_rightPanelWidget);
    rightPanelLayout->setSpacing(10);  // 设置间距

    // 创建TabWidget
    QTabWidget *controlTabWidget = new QTabWidget();
    controlTabWidget->setFixedSize(591, 291);  // 调整高度，为表格留出空间

    // 创建各个控制模块的Tab页
    createGimbalControl();
    createLensControl();
    createImageControl();

    // 将各个控制模块添加到Tab页
    controlTabWidget->addTab(m_gimbalWidget, "云台控制");
    controlTabWidget->addTab(m_lensWidget, "镜头控制");
    controlTabWidget->addTab(m_imageWidget, "图像控制");

    // 创建表格区域
    QWidget *tableWidget = new QWidget();
    tableWidget->setFixedSize(591, 300);  // 表格区域高度
    QVBoxLayout *tableLayout = new QVBoxLayout(tableWidget);

    // 创建表格标题
    QLabel *tableTitle = new QLabel("目标信息");
    tableTitle->setAlignment(Qt::AlignCenter);
    tableTitle->setStyleSheet("font-weight: bold; font-size: 14px; margin: 5px;");

    // 创建表格（暂时留空，不编辑内容）
    m_dataTable = new QTableWidget();
    m_dataTable->setFixedSize(581, 300);
    m_dataTable->setColumnCount(4);  // 设置4列
    m_dataTable->setRowCount(5);     // 设置5行

    // 设置表头
    QStringList headers;
    headers << "时间" << "参数1" << "参数2" << "状态";
    m_dataTable->setHorizontalHeaderLabels(headers);

    // 设置表格样式
    m_dataTable->setStyleSheet("QTableWidget { border: 1px solid #ccc; background-color: white; }"
                              "QHeaderView::section { background-color: #f0f0f0; padding: 5px; border: 1px solid #ddd; }");

    // 暂时禁用编辑
    m_dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 添加到表格布局
    tableLayout->addWidget(tableTitle);
    tableLayout->addWidget(m_dataTable);

    // 将TabWidget和表格添加到右侧面板布局
    rightPanelLayout->addWidget(controlTabWidget);
    rightPanelLayout->addWidget(tableWidget);
}

void MainWindow::createGimbalControl()
{
    // 改为创建普通Widget而不是GroupBox
    m_gimbalWidget = new QWidget();
    m_gimbalWidget->setFixedSize(591, 281); // 调整高度以适应Tab页

    QGridLayout *mainLayout = new QGridLayout(m_gimbalWidget);

    //步长、方位速度、俯仰速度
    QWidget *window1 = new QWidget();
    QHBoxLayout *win1layout = new QHBoxLayout(window1);
    step = new QLabel("步长");
    step->setAlignment(Qt::AlignCenter);
    step->setFixedSize(50,40);
    stepValue = new QSpinBox();
    stepValue->setFixedSize(70,30);
    stepValue->setAlignment(Qt::AlignCenter);
    stepValue->setRange(1,100);
    QHBoxLayout *steplayout = new QHBoxLayout();
    steplayout->addWidget(step);
    steplayout->addWidget(stepValue);
    aziRate = new QLabel("方位速度");
    aziRate->setFixedSize(70,40);
    aziRateValue = new QLabel("0");
    aziRateValue->setFixedSize(70,40);
    QHBoxLayout *aziRatelayout = new QHBoxLayout();
    aziRatelayout->addWidget(aziRate);
    aziRatelayout->addWidget(aziRateValue);
    phiRate = new QLabel("俯仰速度");
    phiRate->setFixedSize(70,40);
    phiRateValue = new QLabel("0");
    phiRateValue->setFixedSize(70,40);
    QHBoxLayout *phiRatelayout = new QHBoxLayout();
    phiRatelayout->addWidget(phiRate);
    phiRatelayout->addWidget(phiRateValue);
    QVBoxLayout *ratelayout = new QVBoxLayout();
    ratelayout->addLayout(aziRatelayout);
    ratelayout->addLayout(phiRatelayout);
    win1layout->addLayout(steplayout);
    win1layout->addLayout(ratelayout);
    mainLayout->addWidget(window1,0,0);

    //前视模式
    QWidget *window2 = new QWidget();
    QHBoxLayout *win2layout = new QHBoxLayout(window2);
    frontView = new QPushButton("前视模式");
    frontView->setFixedSize(100,40);
    frontViewAzi = new QLabel("前视方位");
    frontViewAzi->setFixedSize(70,40);
    frontViewAziValue = new QLineEdit();
    frontViewAziValue->setFixedSize(70,25);
    frontViewPhi = new QLabel("前视俯仰");
    frontViewPhi->setFixedSize(70,40);
    frontViewPhiValue = new QLineEdit();
    frontViewPhiValue->setFixedSize(70,25);
    QHBoxLayout *frontViewAzilayout = new QHBoxLayout();
    frontViewAzilayout->addWidget(frontViewAzi);
    frontViewAzilayout->addWidget(frontViewAziValue);
    QHBoxLayout *frontViewPhilayout = new QHBoxLayout();
    frontViewPhilayout->addWidget(frontViewPhi);
    frontViewPhilayout->addWidget(frontViewPhiValue);
    QVBoxLayout *frontViewlayout = new QVBoxLayout();
    frontViewlayout->addLayout(frontViewAzilayout);
    frontViewlayout->addLayout(frontViewPhilayout);
    win2layout->addWidget(frontView);
    win2layout->addLayout(frontViewlayout);
    mainLayout->addWidget(window2,0,1);

    //方向控制
    QWidget *window3 = new QWidget();
    QHBoxLayout *win3layout = new QHBoxLayout(window3);
    QWidget *directionWidget = new QWidget();
    m_gimbalUpBtn = new QPushButton("上");
    m_gimbalUpBtn->setFixedSize(40,40);
    m_gimbalLeftBtn = new QPushButton("左");
    m_gimbalLeftBtn->setFixedSize(40,40);
    m_gimbalDownBtn = new QPushButton("下");
    m_gimbalDownBtn->setFixedSize(40,40);
    m_gimbalRightBtn = new QPushButton("右");
    m_gimbalRightBtn->setFixedSize(40,40);
    QGridLayout *directionlayout = new QGridLayout(directionWidget);
    directionlayout->addWidget(m_gimbalUpBtn,0,1);
    directionlayout->addWidget(m_gimbalLeftBtn,1,0);
    directionlayout->addWidget(m_gimbalDownBtn,1,1);
    directionlayout->addWidget(m_gimbalRightBtn,1,2);
    azi = new QLabel("方位位置");
    azi->setFixedSize(70,40);
    aziValue = new QLabel("0");
    aziValue->setFixedSize(70,40);
    QHBoxLayout *azilayout = new QHBoxLayout();
    azilayout->addWidget(azi);
    azilayout->addWidget(aziValue);
    phi = new QLabel("俯仰位置");
    phi->setFixedSize(70,40);
    phiValue = new QLabel("0");
    phiValue->setFixedSize(70,40);
    QHBoxLayout *philayout = new QHBoxLayout();
    philayout->addWidget(phi);
    philayout->addWidget(phiValue);
    QVBoxLayout *positionlayout = new QVBoxLayout();
    positionlayout->addLayout(azilayout);
    positionlayout->addLayout(philayout);
    win3layout->addWidget(directionWidget);
    win3layout->addLayout(positionlayout);
    mainLayout->addWidget(window3,1,0);

    //扫描模式
    QWidget *window4 = new QWidget();
    QHBoxLayout *win4layout = new QHBoxLayout(window4);
    sectorScan = new QPushButton("扇扫模式");
    sectorScan->setFixedSize(100,40);
    circularScan = new QPushButton("周扫模式");
    circularScan->setFixedSize(100,40);
    QVBoxLayout *scanBtnlayout = new QVBoxLayout();
    scanBtnlayout->addWidget(sectorScan);
    scanBtnlayout->addWidget(circularScan);
    scanRate = new QLabel("扫描速度");
    scanRate->setFixedSize(70,40);
    scanRateValue = new QLineEdit();
    scanRateValue->setFixedSize(70,25);
    QHBoxLayout *scanRatelayout = new QHBoxLayout();
    scanRatelayout->addWidget(scanRate);
    scanRatelayout->addWidget(scanRateValue);
    scanRange = new QLabel("扫描范围");
    scanRange->setFixedSize(70,40);
    scanRangeValue = new QLineEdit();
    scanRangeValue->setFixedSize(70,25);
    QHBoxLayout *scanRangelayout = new QHBoxLayout();
    scanRangelayout->addWidget(scanRange);
    scanRangelayout->addWidget(scanRangeValue);
    scanCenter = new QLabel("扫描中心");
    scanCenter->setFixedSize(70,40);
    scanCenterValue = new QLineEdit();
    scanCenterValue->setFixedSize(70,25);
    QHBoxLayout *scanCenterlayout = new QHBoxLayout();
    scanCenterlayout->addWidget(scanCenter);
    scanCenterlayout->addWidget(scanCenterValue);
    QVBoxLayout *scanlayout = new QVBoxLayout();
    scanlayout->addLayout(scanRatelayout);
    scanlayout->addLayout(scanRangelayout);
    scanlayout->addLayout(scanCenterlayout);
    win4layout->addLayout(scanBtnlayout);
    win4layout->addLayout(scanlayout);
    mainLayout->addWidget(window4,1,1);
}

void MainWindow::createLensControl()
{
    m_lensWidget = new QWidget();
    m_lensWidget->setFixedSize(591, 320);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_lensWidget);

    // 变焦和视场控制
    QWidget *controlWidget = new QWidget();
    controlWidget->setFixedSize(591, 150);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlWidget);

    // 变焦控制
    QWidget *zoomWidget = new QWidget();
    zoomWidget->setFixedSize(591, 70);
    QHBoxLayout *zoomLayout = new QHBoxLayout(zoomWidget);

    m_zoomLabel = new QLabel("变焦控制");
    m_zoomLabel->setFixedSize(100, 40);
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    m_zoomOutBtn = new QPushButton("-");
    m_zoomInBtn = new QPushButton("+");
    m_zoomInBtn->setFixedSize(50, 50);
    m_zoomOutBtn->setFixedSize(50, 50);

    zoomLayout->addStretch();
    zoomLayout->addWidget(m_zoomOutBtn);
    zoomLayout->addSpacing(20);
    zoomLayout->addWidget(m_zoomLabel);
    zoomLayout->addSpacing(20);
    zoomLayout->addWidget(m_zoomInBtn);
    zoomLayout->addStretch();

    // 视场控制
    QWidget *fovWidget = new QWidget();
    fovWidget->setFixedSize(591, 70);
    QHBoxLayout *fovLayout = new QHBoxLayout(fovWidget);

    m_fovLabel = new QLabel("视场控制");
    m_fovLabel->setFixedSize(100, 40);
    m_fovLabel->setAlignment(Qt::AlignCenter);
    m_fovSmallBtn = new QPushButton("小");
    m_fovLargeBtn = new QPushButton("大");
    m_fovLargeBtn->setFixedSize(50, 50);
    m_fovSmallBtn->setFixedSize(50, 50);

    fovLayout->addStretch();
    fovLayout->addWidget(m_fovSmallBtn);
    fovLayout->addSpacing(20);
    fovLayout->addWidget(m_fovLabel);
    fovLayout->addSpacing(20);
    fovLayout->addWidget(m_fovLargeBtn);
    fovLayout->addStretch();

    controlLayout->addWidget(zoomWidget);
    controlLayout->addWidget(fovWidget);

    // 功能按钮
    QWidget *functionWidget = new QWidget();
    functionWidget->setFixedSize(591, 80);
    QHBoxLayout *functionLayout = new QHBoxLayout(functionWidget);

    m_autoFocusBtn = new QPushButton("自动聚焦");
    m_lensResetBtn = new QPushButton("一键复位");

    m_autoFocusBtn->setFixedSize(120, 40);
    m_lensResetBtn->setFixedSize(120, 40);

    functionLayout->addStretch();
    functionLayout->addWidget(m_autoFocusBtn);
    functionLayout->addSpacing(40);
    functionLayout->addWidget(m_lensResetBtn);
    functionLayout->addStretch();

    // 添加到主布局
    mainLayout->addSpacing(30);
    mainLayout->addWidget(controlWidget);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(functionWidget);
    mainLayout->addStretch();
}

void MainWindow::createImageControl()
{
    m_imageWidget = new QWidget();
    m_imageWidget->setFixedSize(591, 320);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_imageWidget);

    // 模式选择
    QWidget *modeWidget = new QWidget();
    modeWidget->setFixedSize(591, 100);
    QVBoxLayout *modeLayout = new QVBoxLayout(modeWidget);

    // 图像类型选择
    QWidget *imageTypeWidget = new QWidget();
    imageTypeWidget->setFixedSize(591, 45);
    QHBoxLayout *imageTypeLayout = new QHBoxLayout(imageTypeWidget);

    QLabel *imageTypeTitle = new QLabel("图像类型:");
    imageTypeTitle->setFixedSize(80, 30);
    m_imageType_ir = new QPushButton("红外");
    m_imageType_vis = new QPushButton("可见光");
    m_imageType_ir->setFixedSize(100, 35);
    m_imageType_vis->setFixedSize(100, 35);

    imageTypeLayout->addStretch();
    imageTypeLayout->addWidget(imageTypeTitle);
    imageTypeLayout->addWidget(m_imageType_ir);
    imageTypeLayout->addWidget(m_imageType_vis);
    imageTypeLayout->addStretch();

    // 视频源选择
    QWidget *videoSourceWidget = new QWidget();
    videoSourceWidget->setFixedSize(591, 45);
    QHBoxLayout *videoSourceLayout = new QHBoxLayout(videoSourceWidget);

    QLabel *videoSourceTitle = new QLabel("视频源:");
    videoSourceTitle->setFixedSize(80, 30);
    m_videoSource_stream = new QPushButton("推流视频");
    m_videoSource_local = new QPushButton("本地视频");
    m_videoSource_stream->setFixedSize(100, 35);
    m_videoSource_local->setFixedSize(100, 35);

    videoSourceLayout->addStretch();
    videoSourceLayout->addWidget(videoSourceTitle);
    videoSourceLayout->addWidget(m_videoSource_stream);
    videoSourceLayout->addWidget(m_videoSource_local);
    videoSourceLayout->addStretch();

    modeLayout->addWidget(imageTypeWidget);
    modeLayout->addWidget(videoSourceWidget);

    // 亮度和对比度控制
    QWidget *adjustWidget = new QWidget();
    adjustWidget->setFixedSize(591, 150);
    QVBoxLayout *adjustLayout = new QVBoxLayout(adjustWidget);

    // 亮度控制
    QWidget *brightnessWidget = new QWidget();
    brightnessWidget->setFixedSize(591, 70);
    QHBoxLayout *brightnessLayout = new QHBoxLayout(brightnessWidget);

    m_brightnessLabel = new QLabel("亮度控制");
    m_brightnessLabel->setFixedSize(100, 40);
    m_brightnessLabel->setAlignment(Qt::AlignCenter);
    m_brightnessDownBtn = new QPushButton("-");
    m_brightnessUpBtn = new QPushButton("+");
    m_brightnessUpBtn->setFixedSize(50, 50);
    m_brightnessDownBtn->setFixedSize(50, 50);

    brightnessLayout->addStretch();
    brightnessLayout->addWidget(m_brightnessDownBtn);
    brightnessLayout->addSpacing(20);
    brightnessLayout->addWidget(m_brightnessLabel);
    brightnessLayout->addSpacing(20);
    brightnessLayout->addWidget(m_brightnessUpBtn);
    brightnessLayout->addStretch();

    // 对比度控制
    QWidget *contrastWidget = new QWidget();
    contrastWidget->setFixedSize(591, 70);
    QHBoxLayout *contrastLayout = new QHBoxLayout(contrastWidget);

    m_contrastLabel = new QLabel("对比度控制");
    m_contrastLabel->setFixedSize(100, 40);
    m_contrastLabel->setAlignment(Qt::AlignCenter);
    m_contrastDownBtn = new QPushButton("-");
    m_contrastUpBtn = new QPushButton("+");
    m_contrastUpBtn->setFixedSize(50, 50);
    m_contrastDownBtn->setFixedSize(50, 50);

    contrastLayout->addStretch();
    contrastLayout->addWidget(m_contrastDownBtn);
    contrastLayout->addSpacing(20);
    contrastLayout->addWidget(m_contrastLabel);
    contrastLayout->addSpacing(20);
    contrastLayout->addWidget(m_contrastUpBtn);
    contrastLayout->addStretch();

    adjustLayout->addWidget(brightnessWidget);
    adjustLayout->addWidget(contrastWidget);

    // 添加到主布局
    mainLayout->addSpacing(20);
    mainLayout->addWidget(modeWidget);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(adjustWidget);
    mainLayout->addStretch();
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

    //可见光
    connect(m_imageType_vis, &QPushButton::clicked, [this]() {
            unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
            sendBuffer[0] = 0xCB;
            sendBuffer[1] = 0x03;
            sendBuffer[2] = 0x04;
            sendBuffer[3] = 0x02;
            sendBuffer[4] = 0x01;//可见光
            sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
            qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                                       QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
        }
    );

    //红外
    connect(m_imageType_ir, &QPushButton::clicked, [this]() {
            unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
            sendBuffer[0] = 0xCB;
            sendBuffer[1] = 0x03;
            sendBuffer[2] = 0x04;
            sendBuffer[3] = 0x02;
            sendBuffer[4] = 0x02;//红外
            sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
            qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                                       QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
        }
    );

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

    //前视模式
    connect(frontView, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x03;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                                   QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });


    //上
    connect(m_gimbalUpBtn, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x03;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = ((qRound(phiValue->text().toFloat())+stepValue->value())*32767/360) & 0xFF;
        sendBuffer[5] = (((qRound(phiValue->text().toFloat())+stepValue->value())*32767/360) >> 8) & 0xFF;
        sendBuffer[6] = (qRound(aziValue->text().toFloat())*32767/360) & 0xFF;
        sendBuffer[7] = ((qRound(aziValue->text().toFloat())*32767/360) >> 8) & 0xFF;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                                   QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //下
    connect(m_gimbalDownBtn, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xC1;
        sendBuffer[1] = 0xFE;
        sendBuffer[2] = 1;
        sendBuffer[3] = stepValue->value();
        // 计算校验和 (从第1字节开始，共SENDBUFFER_SIZE_UDP-2字节)
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[1], SENDBUFFER_SIZE_UDP - 2);
        // 发送UDP数据报并检查返回值
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                                   QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //左
    connect(m_gimbalDownBtn, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x03;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = qRound(phiValue->text().toFloat()) & 0xFF;
        sendBuffer[5] = (qRound(phiValue->text().toFloat()) >> 8) & 0xFF;
        sendBuffer[6] = qRound(aziValue->text().toFloat()+stepValue->value()) & 0xFF;
        sendBuffer[7] = ((qRound(aziValue->text().toFloat()+stepValue->value())%360) >> 8) & 0xFF;

        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                                   QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //右
    connect(m_gimbalDownBtn, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xC1;
        sendBuffer[1] = 0xFE;
        sendBuffer[2] = 1;
        sendBuffer[3] = stepValue->value();
        // 计算校验和 (从第1字节开始，共SENDBUFFER_SIZE_UDP-2字节)
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[1], SENDBUFFER_SIZE_UDP - 2);
        // 发送UDP数据报并检查返回值
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                                   QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //前视模式

    //扇扫模式

    //周扫模式
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

//开始检测按钮
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

void MainWindow::sendStartDetectCommand()
{
    unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};

    // 帧头
    sendBuffer[0] = 0xCB;
    // 命令ID - 开始检测
    sendBuffer[1] = 0x01;
    sendBuffer[2] = 0x03;
    // 计算校验和 (从第1字节开始，共SENDBUFFER_SIZE_UDP-2字节)
    sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
    qDebug() << sendBuffer[15] ;
    qDebug()<<QString("0x%1").arg(static_cast<quint8>(sendBuffer[15]),2,16,QLatin1Char('0'));
    // 发送UDP数据报并检查返回值
    qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,
                                               QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
}

void MainWindow::sendStopDetectCommand()
{
    unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};

    // 帧头
    sendBuffer[0] = 0xC1;
    // 命令ID - 停止检测
    sendBuffer[1] = 0x02;

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
    StatusFeedbackPacket packet;

    if (data.size() != sizeof(StatusFeedbackPacket)) {
        qWarning() << "数据长度不匹配，期望:" << sizeof(StatusFeedbackPacket)
                   << "实际:" << data.size();
        return;
    }

    // 拷贝数据到结构体
    memcpy(&packet, data.constData(), sizeof(StatusFeedbackPacket));

    // 验证帧头
    if (packet.frameHeader != 0xC0) {
//        qWarning() << "帧头验证失败，期望:0xC0 实际:" << QString::number(packet.frameHeader, 16);
        return; // 返回空对象
    }

    // 验证校验和
    uint8_t calculatedChecksum = 0;
    const uint8_t *temp = reinterpret_cast<const uint8_t*>(&packet);

    // 计算前63个字节的和
    for (int i = 0; i < 63; ++i) {
        calculatedChecksum += data[i];
    }
    if (calculatedChecksum != packet.checksum) {
        qWarning() << "校验和验证失败";
        return;
    }
    aziValue->setText(QString::number(((float)packet.aziPosition/10000), 'f', 4));
    phiValue->setText(QString::number(((float)packet.pitchPosition/10000), 'f', 4));
    aziRateValue->setText(QString::number(((float)packet.aziVelocity*1024/32767), 'f', 4));
    phiRateValue->setText(QString::number(((float)packet.pitchVelocity*1024/32767), 'f', 4));
    qDebug() << "当前俯仰：" << (float)packet.pitchPosition/10000 << "度";
    qDebug() << "当前方位：" << (float)packet.aziPosition/10000 << "度";
    qDebug() << qRound(phiValue->text().toFloat()+stepValue->value());
    qDebug() << qRound(aziValue->text().toFloat()+stepValue->value());
}



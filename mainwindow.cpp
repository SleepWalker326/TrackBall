// MainWindow.cpp
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDateTime>
#include <QFrame>
#include <QTime>
#include <QDebug>
const char* MainWindow::REMOTE_IP = "192.168.1.100";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_videoPlayer(new VideoPlayer()),
      m_isVideoPlaying(false)
{
    setWindowTitle("光电球跟踪系统");

    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // 1. 创建顶部状态栏
    createTopBar();
    mainLayout->addWidget(topBar);

    // 2. 创建主体内容区域
    QHBoxLayout *bodyLayout = new QHBoxLayout();
    bodyLayout->setSpacing(0);
    bodyLayout->setContentsMargins(0, 0, 0, 0);

    // 左侧导航
    createNavigation();
    bodyLayout->addWidget(navigationBar);  // 1份宽度

    // 右侧内容
    createContentArea();
    bodyLayout->addWidget(contentStack);  // 4份宽度
    createContent2Area();
    bodyLayout->addWidget(contentStack2);  // 4份宽度

    mainLayout->addLayout(bodyLayout, 1);  // 可伸缩部分

    // 3. 创建底部按钮栏
    createBottomBar();
    mainLayout->addWidget(bottomBar);

    // 设置样式
    setupStyles();

    // 初始化时间更新定时器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateDateTime);
    timer->start(1000);  // 每秒更新一次时间

    // 初始显示设置页面
    contentStack2->setCurrentWidget(m_gimbalWidget);
    updateNavigationStyle(0);  // 高亮设置按钮

    // 初始化UDP
    udpSocket = new QUdpSocket(this);
    if (!udpSocket->bind(QHostAddress::AnyIPv4, UDP_PORT_LOCAL)) {
        QMessageBox::critical(this, "错误", "无法绑定UDP端口: " + QString::number(UDP_PORT_LOCAL));
    }
    createConnections();

    m_mediaRecorder = new MediaRecorder(this);

}

MainWindow::~MainWindow(){
    delete m_videoPlayer;
    delete udpSocket;
}

//顶部设计
void MainWindow::createTopBar()
{
    topBar = new QWidget();
    topBar->setObjectName("topBar");
    topBar->setFixedHeight(80);

    QHBoxLayout *layout = new QHBoxLayout(topBar);
    layout->setContentsMargins(20, 10, 20, 10);

    // 左侧：标题
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(5);

    titleLabel = new QLabel("第一研究所");
    titleLabel->setObjectName("titleLabel");
    leftLayout->addWidget(titleLabel);

    statusLabel = new QLabel("智能软件组");
    statusLabel->setObjectName("statusLabel");
    leftLayout->addWidget(statusLabel);

    // 中间：时间
    QWidget *centerWidget = new QWidget();
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setAlignment(Qt::AlignCenter);

    timeLabel = new QLabel();
    timeLabel->setObjectName("timeLabel");
    timeLabel->setAlignment(Qt::AlignCenter);
    centerLayout->addWidget(timeLabel);
    updateDateTime();

    // 右侧：电源状态
    QWidget *rightWidget = new QWidget();
    QHBoxLayout *rightLayout = new QHBoxLayout(rightWidget);
    rightLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    ServoStatusLabel = new QLabel("伺服系统");
    ServoStatusLabel->setProperty("status", "unknown");  // 初始未知状态
    rightLayout->addWidget(ServoStatusLabel);
    trackStatusLabel = new QLabel("跟踪系统");
    trackStatusLabel->setProperty("status", "unknown");  // 初始未知状态
    rightLayout->addWidget(trackStatusLabel);
    irStatusLabel = new QLabel("热像系统");
    irStatusLabel->setProperty("status", "unknown");  // 初始未知状态
    rightLayout->addWidget(irStatusLabel);
    visStatusLabel = new QLabel("可见光系统");
    visStatusLabel->setProperty("status", "unknown");  // 初始未知状态
    rightLayout->addWidget(visStatusLabel);
    laserStatusLabel = new QLabel("激光系统");
    laserStatusLabel->setProperty("status", "unknown");  // 初始未知状态
    rightLayout->addWidget(laserStatusLabel);
    layout->addWidget(leftWidget);
    layout->addWidget(centerWidget, 1);
    layout->addWidget(rightWidget);
}

//导航栏
void MainWindow::createNavigation()
{
    navigationBar = new QWidget();
    navigationBar->setObjectName("navigationBar");
    navigationBar->setFixedWidth(150);

    navLayout = new QVBoxLayout(navigationBar);
    navLayout->setSpacing(15);
    navLayout->addStretch();

    navLayout->setContentsMargins(20, 40, 20, 40);

    // 导航按钮
    QStringList navItems = {
        "云台控制",
        "热像控制",
        "白光控制",
        "激光控制"
    };

    for (int i = 0; i < navItems.size(); ++i) {
        QPushButton *button = new QPushButton(navItems[i]);
        button->setObjectName(QString("navButton%1").arg(i));
        button->setCheckable(true);
        button->setFixedHeight(60);
        button->setCursor(Qt::PointingHandCursor);

        connect(button, &QPushButton::clicked, [this, i]() {
            onNavigationClicked(i);
        });

        navButtons.append(button);
        navLayout->addWidget(button);
        navLayout->setSpacing(50);

    }

    navLayout->addStretch();
}

//中间显示
void MainWindow::createContentArea()
{
    // 创建左侧显示区域
    contentStack = new QWidget();
    contentStack->setFixedWidth(1200);
    QVBoxLayout *leftLayout = new QVBoxLayout(contentStack);

    // 创建信息显示区域
    QWidget *infoArea = createInfoArea();
    leftLayout->addWidget(infoArea,0, Qt::AlignCenter);

    // 创建视频显示区域
    createDisplayArea();
    leftLayout->addWidget(m_displayImageLabel, 0, Qt::AlignCenter);
    leftLayout->addStretch();
}

//信息显示
QWidget* MainWindow::createInfoArea()
{
    QWidget *infoWidget = new QWidget();
    infoWidget->setFixedSize(1000, 70);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoWidget);

    // 帧率信息
    QWidget *frameRateWidget = new QWidget();
    frameRateWidget->setFixedSize(75, 60);
    QVBoxLayout *frameRateLayout = new QVBoxLayout(frameRateWidget);
    m_frameRateLabel = new QLabel("帧率");
    QFont font("黑体", 16, QFont::Bold);  // 黑体，16px，加粗
    m_frameRateLabel->setFont(font);
    m_frameRateLabel->setFixedHeight(30);
    m_frameRateValueLabel = new QLabel("0fps");
    m_frameRateValueLabel->setFont(font);
    m_frameRateValueLabel->setFixedHeight(30);
    m_frameRateLabel->setAlignment(Qt::AlignCenter);
    m_frameRateValueLabel->setAlignment(Qt::AlignCenter);
    frameRateLayout->addWidget(m_frameRateLabel);
    frameRateLayout->addWidget(m_frameRateValueLabel);
    // 分辨率信息
    QWidget *resolutionWidget = new QWidget();
    resolutionWidget->setFixedSize(100, 60);
    QVBoxLayout *resolutionLayout = new QVBoxLayout(resolutionWidget);
    m_resolutionLabel = new QLabel("分辨率");
    m_resolutionLabel->setFont(font);
    m_resolutionLabel->setFixedHeight(30);
    m_resolutionValueLabel = new QLabel("640*512");
    m_resolutionValueLabel->setFont(font);
    m_resolutionValueLabel->setFixedHeight(30);
    m_resolutionLabel->setAlignment(Qt::AlignCenter);
    m_resolutionValueLabel->setAlignment(Qt::AlignCenter);
    resolutionLayout->addWidget(m_resolutionLabel);
    resolutionLayout->addWidget(m_resolutionValueLabel);

    // 时间戳信息
    QWidget *timestampWidget = new QWidget();
    timestampWidget->setFixedSize(150, 60);
    QVBoxLayout *timestampLayout = new QVBoxLayout(timestampWidget);
    m_timestampLabel = new QLabel("时间戳");
    m_timestampLabel->setFont(font);
    m_timestampLabel->setFixedHeight(30);
    m_timestampValueLabel = new QLabel("00：00：00");
    m_timestampValueLabel->setFont(font);
    m_timestampValueLabel->setFixedHeight(30);
    m_timestampLabel->setAlignment(Qt::AlignCenter);
    m_timestampValueLabel->setAlignment(Qt::AlignCenter);
    timestampLayout->addWidget(m_timestampLabel);
    timestampLayout->addWidget(m_timestampValueLabel);

    infoLayout->addWidget(frameRateWidget);
    infoLayout->addWidget(resolutionWidget);
    infoLayout->addWidget(timestampWidget);

    return infoWidget;
}

//视频显示
void MainWindow::createDisplayArea()
{
    QWidget *displayContainer = new QWidget();
    displayContainer->setFixedSize(1200, 650);
    QVBoxLayout *containerLayout = new QVBoxLayout(displayContainer);

    m_displayImageLabel = new QLabel("显示图像");
    m_displayImageLabel->setFixedSize(1130, 635);
    m_displayImageLabel->setAlignment(Qt::AlignCenter);
    m_displayImageLabel->setStyleSheet("border: 1px solid gray; background-color: white;");
    containerLayout->addWidget(m_displayImageLabel);
}

void MainWindow::createContent2Area()
{
    contentStack2 = new QStackedWidget();
    contentStack2->setObjectName("contentStack");

    // 1. 云台控制
    m_gimbalWidget = createGimbalControl();
    contentStack2->addWidget(m_gimbalWidget);

    // 2. 热像控制
    m_irImagWidget = createIrImagControl();
    contentStack2->addWidget(m_irImagWidget);

    // 3. 可见光控制
    m_visImageWidget = createVisImageControl();
    contentStack2->addWidget(m_visImageWidget);

    // 4. 激光控制
    m_laserWidget = createLaserControl();
    contentStack2->addWidget(m_laserWidget);
}

void MainWindow::createBottomBar()
{
    bottomBar = new QWidget();
    bottomBar->setObjectName("settingsItemButton");
    bottomBar->setFixedHeight(60);

    QHBoxLayout *layout = new QHBoxLayout(bottomBar);

    // 保存按钮
    m_connectionBtn = new QPushButton("连接");
    m_connectionBtn->setObjectName("settingsItem1Button");
    m_connectionBtn->setFixedSize(80, 45);
    m_connectionBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_connectionBtn);

    // 取消按钮
    m_startDetectionBtn = new QPushButton("开始检测");
    m_startDetectionBtn->setObjectName("settingsItem1Button");
    m_startDetectionBtn->setFixedSize(120, 45);
    m_startDetectionBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_startDetectionBtn);

    m_multiTargetTrackBtn = new QPushButton("多目标跟踪");
    m_multiTargetTrackBtn->setFixedSize(120,45);
    m_multiTargetTrackBtn->setObjectName("settingsItem1Button");
    m_multiTargetTrackBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_multiTargetTrackBtn);

    m_singleTargetTrackBtn = new QPushButton("单目标跟踪");
    m_singleTargetTrackBtn->setFixedSize(120,45);
    m_singleTargetTrackBtn->setObjectName("settingsItem1Button");
    m_singleTargetTrackBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_singleTargetTrackBtn);

    m_startRecordBtn = new QPushButton("开始录制");
    m_startRecordBtn->setFixedSize(120,45);
    m_startRecordBtn->setObjectName("settingsItem1Button");
    m_startRecordBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_startRecordBtn);

    m_screenshotBtn = new QPushButton("截图");
    m_screenshotBtn->setFixedSize(80,45);
    m_screenshotBtn->setObjectName("settingsItem1Button");
    m_screenshotBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_screenshotBtn);

    m_imageType_ir = new QPushButton("红外图像");
    m_imageType_ir->setFixedSize(120, 45);
    m_imageType_ir->setObjectName("settingsItem1Button");
    m_imageType_ir->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_imageType_ir);

    m_imageType_vis = new QPushButton("可见光图像");
    m_imageType_vis->setFixedSize(120, 45);
    m_imageType_vis->setObjectName("settingsItem1Button");
    m_imageType_vis->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_imageType_vis);

    m_videoSource_stream = new QPushButton("推流视频");
    m_videoSource_stream->setFixedSize(120, 45);
    m_videoSource_stream->setObjectName("settingsItem1Button");
    m_videoSource_stream->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_videoSource_stream);

    m_videoSource_local = new QPushButton("本地视频");
    m_videoSource_local->setFixedSize(120, 45);
    m_videoSource_local->setObjectName("settingsItem1Button");
    m_videoSource_local->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_videoSource_local);

}

QWidget* MainWindow::createGimbalControl()
{
    m_gimbalWidget = new QWidget();
    m_gimbalWidget->setFixedSize(400, 600); // 调整高度以适应Tab页

    QVBoxLayout *mainLayout = new QVBoxLayout(m_gimbalWidget);

    //步长、方位速度、俯仰速度
    QWidget *window1 = new QWidget();
    QHBoxLayout *win1layout = new QHBoxLayout(window1);
    step = new QLabel("步长");
    step->setObjectName("label");
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
    aziRate->setObjectName("label");
    aziRate->setFixedSize(70,40);
    aziRateValue = new QLabel("0");
    aziRateValue->setFixedSize(70,40);
    QHBoxLayout *aziRatelayout = new QHBoxLayout();
    aziRatelayout->addWidget(aziRate);
    aziRatelayout->addWidget(aziRateValue);
    phiRate = new QLabel("俯仰速度");
    phiRate->setObjectName("label");
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

    mainLayout->addWidget(window1);

    //前视模式
    QWidget *window2 = new QWidget();
    QHBoxLayout *win2layout = new QHBoxLayout(window2);
    frontView = new QPushButton("前视模式");
    frontView->setObjectName("settingsItemButton");
    frontView->setFixedSize(100,50);
    frontViewAzi = new QLabel("前视方位");
    frontViewAzi->setObjectName("label");
    frontViewAzi->setFixedSize(70,40);
    frontViewAziValue = new QLineEdit("0");
    frontViewAziValue->setAlignment(Qt::AlignCenter);
    frontViewAziValue->setFixedSize(70,25);
    frontViewPhi = new QLabel("前视俯仰");
    frontViewPhi->setObjectName("label");
    frontViewPhi->setFixedSize(70,40);
    frontViewPhiValue = new QLineEdit("0");
    frontViewPhiValue->setAlignment(Qt::AlignCenter);
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
    mainLayout->addWidget(window2);

    //方向控制
    QWidget *window3 = new QWidget();
    QHBoxLayout *win3layout = new QHBoxLayout(window3);
    QWidget *directionWidget = new QWidget();
    m_gimbalUpBtn = new QPushButton("上");
    m_gimbalUpBtn->setObjectName("settingsItemButton");
    m_gimbalUpBtn->setFixedSize(50,50);
    m_gimbalLeftBtn = new QPushButton("左");
    m_gimbalLeftBtn->setObjectName("settingsItemButton");
    m_gimbalLeftBtn->setFixedSize(50,50);
    m_gimbalDownBtn = new QPushButton("下");
    m_gimbalDownBtn->setObjectName("settingsItemButton");
    m_gimbalDownBtn->setFixedSize(50,50);
    m_gimbalRightBtn = new QPushButton("右");
    m_gimbalRightBtn->setObjectName("settingsItemButton");
    m_gimbalRightBtn->setFixedSize(50,50);
    QGridLayout *directionlayout = new QGridLayout(directionWidget);
    directionlayout->addWidget(m_gimbalUpBtn,0,1);
    directionlayout->addWidget(m_gimbalLeftBtn,1,0);
    directionlayout->addWidget(m_gimbalDownBtn,1,1);
    directionlayout->addWidget(m_gimbalRightBtn,1,2);
    azi = new QLabel("方位位置");
    azi->setObjectName("label");
    azi->setFixedSize(70,40);
    aziValue = new QLabel("0");
    aziValue->setFixedSize(70,40);
    QHBoxLayout *azilayout = new QHBoxLayout();
    azilayout->addWidget(azi);
    azilayout->addWidget(aziValue);
    phi = new QLabel("俯仰位置");
    phi->setObjectName("label");
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
    mainLayout->addWidget(window3);

    //扫描模式
    QWidget *window4 = new QWidget();
    QHBoxLayout *win4layout = new QHBoxLayout(window4);
    sectorScan = new QPushButton("扇扫模式");
    sectorScan->setObjectName("settingsItemButton");
    sectorScan->setFixedSize(100,50);
    circularScan = new QPushButton("周扫模式");
    circularScan->setObjectName("settingsItemButton");
    circularScan->setFixedSize(100,50);
    stopScan = new QPushButton("停止扫描");
    stopScan->setObjectName("settingsItemButton");
    stopScan->setFixedSize(100,50);
    QVBoxLayout *scanBtnlayout = new QVBoxLayout();
    scanBtnlayout->addWidget(sectorScan);
    scanBtnlayout->addWidget(circularScan);
    scanBtnlayout->addWidget(stopScan);
    scanPhi = new QLabel("扫描俯仰");
    scanPhi->setObjectName("label");
    scanPhi->setFixedSize(70,40);
    scanPhiValue = new QLineEdit("0");
    scanPhiValue->setAlignment(Qt::AlignCenter);
    scanPhiValue->setFixedSize(70,25);
    QHBoxLayout *scanPhilayout = new QHBoxLayout();
    scanPhilayout->addWidget(scanPhi);
    scanPhilayout->addWidget(scanPhiValue);
    scanRate = new QLabel("扫描速度");
    scanRate->setObjectName("label");
    scanRate->setFixedSize(70,40);
    scanRateValue = new QLineEdit("0");
    scanRateValue->setAlignment(Qt::AlignCenter);
    scanRateValue->setFixedSize(70,25);
    QHBoxLayout *scanRatelayout = new QHBoxLayout();
    scanRatelayout->addWidget(scanRate);
    scanRatelayout->addWidget(scanRateValue);
    scanRange = new QLabel("扫描范围");
    scanRange->setObjectName("label");
    scanRange->setFixedSize(70,40);
    scanRangeValue = new QLineEdit("0");
    scanRangeValue->setAlignment(Qt::AlignCenter);
    scanRangeValue->setFixedSize(70,25);
    QHBoxLayout *scanRangelayout = new QHBoxLayout();
    scanRangelayout->addWidget(scanRange);
    scanRangelayout->addWidget(scanRangeValue);
    scanCenter = new QLabel("扫描中心");
    scanCenter->setObjectName("label");
    scanCenter->setFixedSize(70,40);
    scanCenterValue = new QLineEdit("0");
    scanCenterValue->setAlignment(Qt::AlignCenter);
    scanCenterValue->setFixedSize(70,25);
    QHBoxLayout *scanCenterlayout = new QHBoxLayout();
    scanCenterlayout->addWidget(scanCenter);
    scanCenterlayout->addWidget(scanCenterValue);
    QVBoxLayout *scanlayout = new QVBoxLayout();
    scanlayout->addLayout(scanPhilayout);
    scanlayout->addLayout(scanRatelayout);
    scanlayout->addLayout(scanRangelayout);
    scanlayout->addLayout(scanCenterlayout);
    win4layout->addLayout(scanBtnlayout);
    win4layout->addLayout(scanlayout);
    mainLayout->addWidget(window4);

    return m_gimbalWidget;
}

QWidget* MainWindow::createIrImagControl()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setAlignment(Qt::AlignCenter);

    //自检
    irSelfCheck = new QPushButton("自检");
    irSelfCheck->setObjectName("settingsItemButton");
    irSelfCheck->setFixedSize(150,50);

    //非均匀校正
    NUC = new QPushButton("非均匀校正");
    NUC->setObjectName("settingsItemButton");
    NUC->setFixedSize(150,50);
    NUCValue = new QComboBox();
    NUCValue->setFixedSize(70,40);
    NUCValue->addItem("手动",0x00);
    NUCValue->addItem("自动",0x01);
    QHBoxLayout *NUClayout = new QHBoxLayout();
    NUClayout->addWidget(NUC);
    NUClayout->addWidget(NUCValue);

    // 快门校正
    ShutterCorrection = new QPushButton("快门校正");
    ShutterCorrection->setObjectName("settingsItemButton");
    ShutterCorrection->setFixedSize(150,50);
    ShutterCorrectionValue = new QComboBox();
    ShutterCorrectionValue->setFixedSize(105,40);
    ShutterCorrectionValue->addItem("背景校正",0x00);
    ShutterCorrectionValue->addItem("双稳态快门",0x01);
    QHBoxLayout *ShutterCorrectionlayout = new QHBoxLayout();
    ShutterCorrectionlayout->addWidget(ShutterCorrection);
    ShutterCorrectionlayout->addWidget(ShutterCorrectionValue);

    // 极性
    imagePolarity = new QPushButton("极性");
    imagePolarity->setObjectName("settingsItemButton");
    imagePolarity->setFixedSize(150,50);
    imagePolarityValue = new QComboBox();
    imagePolarityValue->setFixedSize(70,40);
    imagePolarityValue->addItem("白热",0x00);
    imagePolarityValue->addItem("黑热",0x01);
    QHBoxLayout *imagePolaritylayout = new QHBoxLayout();
    imagePolaritylayout->addWidget(imagePolarity);
    imagePolaritylayout->addWidget(imagePolarityValue);

    // 图像翻转
    imageFlip = new QPushButton("图像翻转");
    imageFlip->setObjectName("settingsItemButton");
    imageFlip->setFixedSize(150,50);
    imageFlipValue = new QComboBox();
    imageFlipValue->setFixedSize(95,40);
    imageFlipValue->addItem("不翻转",0x00);
    imageFlipValue->addItem("上下翻转",0x01);
    imageFlipValue->addItem("左右翻转",0x02);
    imageFlipValue->addItem("对角翻转",0x03);
    QHBoxLayout *imageFliplayout = new QHBoxLayout();
    imageFliplayout->addWidget(imageFlip);
    imageFliplayout->addWidget(imageFlipValue);

    //对比度
    irContrast = new QPushButton("对比度");
    irContrast->setObjectName("settingsItemButton");
    irContrast->setFixedSize(150,50);
    irContrastValue = new QLineEdit("128");
    irContrastValue->setAlignment(Qt::AlignCenter);
    irContrastValue->setFixedSize(70,40);
    irContrastValue->setPlaceholderText("0～255");
    QHBoxLayout *irContrastlayout = new QHBoxLayout();
    irContrastlayout->addWidget(irContrast);
    irContrastlayout->addWidget(irContrastValue);

    //亮度
    irBrightness = new QPushButton("亮度");
    irBrightness->setObjectName("settingsItemButton");
    irBrightness->setFixedSize(150,50);
    irBrightnessValue = new QLineEdit("128");
    irBrightnessValue->setAlignment(Qt::AlignCenter);
    irBrightnessValue->setFixedSize(70,40);
    irBrightnessValue->setPlaceholderText("0～255");
    QHBoxLayout *irBrightnesslayout = new QHBoxLayout();
    irBrightnesslayout->addWidget(irBrightness);
    irBrightnesslayout->addWidget(irBrightnessValue);

    // DDE增强
    DDEEnhancement = new QPushButton("DDE增强");
    DDEEnhancement->setObjectName("settingsItemButton");
    DDEEnhancement->setFixedSize(150,50);
    DDEEnhancementValue = new QComboBox();
    DDEEnhancementValue->setFixedSize(70,40);
    DDEEnhancementValue->addItem("1",0x01);
    DDEEnhancementValue->addItem("2",0x02);
    DDEEnhancementValue->addItem("3",0x03);
    DDEEnhancementValue->addItem("4",0x04);
    DDEEnhancementValue->addItem("5",0x05);
    DDEEnhancementValue->addItem("6",0x06);
    DDEEnhancementValue->addItem("7",0x07);
    DDEEnhancementValue->addItem("8",0x08);
    QHBoxLayout *DDEEnhancementlayout = new QHBoxLayout();
    DDEEnhancementlayout->addWidget(DDEEnhancement);
    DDEEnhancementlayout->addWidget(DDEEnhancementValue);

    //图像实域滤波
    temporalFiltering = new QPushButton("图像实域滤波");
    temporalFiltering->setObjectName("settingsItemButton");
    temporalFiltering->setFixedSize(150,50);
    temporalFilteringValue = new QComboBox();
    temporalFilteringValue->setFixedSize(80,40);
    temporalFilteringValue->addItem("滤波关",0x00);
    temporalFilteringValue->addItem("滤波开",0x01);
    QHBoxLayout *temporalFilteringlayout = new QHBoxLayout();
    temporalFilteringlayout->addWidget(temporalFiltering);
    temporalFilteringlayout->addWidget(temporalFilteringValue);

    // 电子变倍
    irElectronicZoom = new QPushButton("电子变倍");
    irElectronicZoom->setObjectName("settingsItemButton");
    irElectronicZoom->setFixedSize(150,50);
    irElectronicZoomValue = new QComboBox();
    irElectronicZoomValue->setFixedSize(70,40);
    irElectronicZoomValue->addItem("1");
    irElectronicZoomValue->addItem("2");
    irElectronicZoomValue->addItem("3");
    irElectronicZoomValue->addItem("4");
    QHBoxLayout *irElectronicZoomlayout = new QHBoxLayout();
    irElectronicZoomlayout->addWidget(irElectronicZoom);
    irElectronicZoomlayout->addWidget(irElectronicZoomValue);

    //图像空域滤波
    spatialFiltering = new QPushButton("图像空域滤波");
    spatialFiltering->setObjectName("settingsItemButton");
    spatialFiltering->setFixedSize(150,50);
    spatialFilteringValue = new QComboBox();
    spatialFilteringValue->setFixedSize(80,40);
    spatialFilteringValue->addItem("滤波关",0x00);
    spatialFilteringValue->addItem("滤波开",0x01);
    QHBoxLayout *spatialFilteringlayout = new QHBoxLayout();
    spatialFilteringlayout->addWidget(spatialFiltering);
    spatialFilteringlayout->addWidget(spatialFilteringValue);

    //热像十字光标显隐
    irCrossHair = new QPushButton("十字光标显隐");
    irCrossHair->setObjectName("settingsItemButton");
    irCrossHair->setFixedSize(150,50);
    irCrossHairValue = new QComboBox();
    irCrossHairValue->setFixedSize(70,40);
    irCrossHairValue->addItem("隐藏",0x00);
    irCrossHairValue->addItem("显示",0x01);
    QHBoxLayout *irCrossHairlayout = new QHBoxLayout();
    irCrossHairlayout->addWidget(irCrossHair);
    irCrossHairlayout->addWidget(irCrossHairValue);

    layout->addWidget(irSelfCheck);
    layout->addLayout(NUClayout);
    layout->addLayout(ShutterCorrectionlayout);
    layout->addLayout(imagePolaritylayout);
    layout->addLayout(imageFliplayout);
    layout->addLayout(irContrastlayout);
    layout->addLayout(irBrightnesslayout);
    layout->addLayout(DDEEnhancementlayout);
    layout->addLayout(irElectronicZoomlayout);
    layout->addLayout(temporalFilteringlayout);
    layout->addLayout(spatialFilteringlayout);
    layout->addLayout(irCrossHairlayout);


    return page;
}

QWidget* MainWindow::createVisImageControl()
{
    QWidget *page = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(page);

    //亮度
    visBrightness = new QPushButton("亮度");
    visBrightness->setObjectName("settingsItemButton");
    visBrightness->setFixedSize(70,50);
    visBrightnessValue = new QLineEdit("50");
    visBrightnessValue->setAlignment(Qt::AlignCenter);
    visBrightnessValue->setFixedSize(50,40);
    visBrightnessValue->setPlaceholderText("0～100");
    QHBoxLayout *visBrightnesslayout = new QHBoxLayout();
    visBrightnesslayout->addWidget(visBrightness);
    visBrightnesslayout->addWidget(visBrightnessValue);

    //对比度
    visContrast = new QPushButton("对比度");
    visContrast->setObjectName("settingsItemButton");

    visContrast->setFixedSize(100,50);
    visContrastValue = new QLineEdit("50");
    visContrastValue->setAlignment(Qt::AlignCenter);
    visContrastValue->setFixedSize(50,40);
    visContrastValue->setPlaceholderText("0～100");
    QHBoxLayout *visContrastlayout = new QHBoxLayout();
    visContrastlayout->addWidget(visContrast);
    visContrastlayout->addWidget(visContrastValue);

    //清晰度
    sharpness = new QLabel("清晰度");
    sharpness->setObjectName("label");
    sharpness->setFixedSize(70,50);
    sharpnessReset = new QPushButton("恢复");
    sharpnessReset->setObjectName("settingsItemButton");
    sharpnessReset->setFixedSize(80,50);
    sharpnessIncrease = new QPushButton("+");
    sharpnessIncrease->setObjectName("settingsItemButton");
    sharpnessIncrease->setFixedSize(50,50);
    sharpnessDecrease = new QPushButton("-");
    sharpnessDecrease->setObjectName("settingsItemButton");
    sharpnessDecrease->setFixedSize(50,50);
    QHBoxLayout *sharpnesslayout = new QHBoxLayout();
    sharpnesslayout->addWidget(sharpness);
    sharpnesslayout->addWidget(sharpnessReset);
    sharpnesslayout->addWidget(sharpnessIncrease);
    sharpnesslayout->addWidget(sharpnessDecrease);

    //视场
    FOV = new QLabel("视场");
    FOV->setObjectName("label");
    FOV->setFixedSize(70,50);
    FOVstop = new QPushButton("停止");
    FOVstop->setObjectName("settingsItemButton");
    FOVstop->setFixedSize(80,50);
    FOVNarrow = new QPushButton("窄");
    FOVNarrow->setObjectName("settingsItemButton");
    FOVNarrow->setFixedSize(50,50);
    FOVWide = new QPushButton("宽");
    FOVWide->setObjectName("settingsItemButton");
    FOVWide->setFixedSize(50,50);
    QHBoxLayout *FOVlayout = new QHBoxLayout();
    FOVlayout->addWidget(FOV);
    FOVlayout->addWidget(FOVstop);
    FOVlayout->addWidget(FOVNarrow);
    FOVlayout->addWidget(FOVWide);

    // 电子变倍
    visElectronicZoom = new QLabel("电子变倍");
    visElectronicZoom->setObjectName("label");
    visElectronicZoom->setFixedSize(70,50);
    visElectronicZoomOff = new QPushButton("关");
    visElectronicZoomOff->setObjectName("settingsItemButton");
    visElectronicZoomOff->setFixedSize(50,50);
    visElectronicZoomOn = new QPushButton("开");
    visElectronicZoomOn->setObjectName("settingsItemButton");
    visElectronicZoomOn->setFixedSize(50,50);
    QHBoxLayout *visElectronicZoomlayout = new QHBoxLayout();
    visElectronicZoomlayout->addWidget(visElectronicZoom);
    visElectronicZoomlayout->addWidget(visElectronicZoomOff);
    visElectronicZoomlayout->addWidget(visElectronicZoomOn);

    //调焦
    focus = new QLabel("调焦");
    focus->setObjectName("label");
    focus->setFixedSize(70,50);
    focusStop = new QPushButton("停止");
    focusStop->setObjectName("settingsItemButton");
    focusStop->setFixedSize(80,50);
    focusIncrease = new QPushButton("+");
    focusIncrease->setObjectName("settingsItemButton");
    focusIncrease->setFixedSize(50,50);
    focusDecrease = new QPushButton("-");
    focusDecrease->setObjectName("settingsItemButton");
    focusDecrease->setFixedSize(50,50);
    QHBoxLayout *focuslayout = new QHBoxLayout();
    focuslayout->addWidget(focus);
    focuslayout->addWidget(focusStop);
    focuslayout->addWidget(focusIncrease);
    focuslayout->addWidget(focusDecrease);

    //增益
    gain = new QLabel("增益");
    gain->setObjectName("label");
    gain->setFixedSize(70,50);
    gainReset = new QPushButton("恢复");
    gainReset->setObjectName("settingsItemButton");
    gainReset->setFixedSize(80,50);
    gainIncrease = new QPushButton("+");
    gainIncrease->setObjectName("settingsItemButton");
    gainIncrease->setFixedSize(50,50);
    gainDecrease = new QPushButton("-");
    gainDecrease->setObjectName("settingsItemButton");
    gainDecrease->setFixedSize(50,50);
    QHBoxLayout *gainlayout = new QHBoxLayout();
    gainlayout->addWidget(gain);
    gainlayout->addWidget(gainReset);
    gainlayout->addWidget(gainIncrease);
    gainlayout->addWidget(gainDecrease);

    //白光十字光标显隐
    visCrossHair = new QLabel("十字光标显隐");
    visCrossHair->setObjectName("label");
    visCrossHair->setFixedSize(100,50);
    visCrossHairOff = new QPushButton("关");
    visCrossHairOff->setObjectName("settingsItemButton");
    visCrossHairOff->setFixedSize(50,50);
    visCrossHairOn = new QPushButton("开");
    visCrossHairOn->setObjectName("settingsItemButton");
    visCrossHairOn->setFixedSize(50,50);
    QHBoxLayout *visCrossHairlayout = new QHBoxLayout();
    visCrossHairlayout->addWidget(visCrossHair);
    visCrossHairlayout->addWidget(visCrossHairOff);
    visCrossHairlayout->addWidget(visCrossHairOn);

    //指定倍数视场切换
    FOVSwitch = new QPushButton("指定倍数视场切换");
    FOVSwitch->setObjectName("settingsItemButton");
    FOVSwitch->setFixedSize(160,50);
    FOVSwitchValue = new QLineEdit("1");
    FOVSwitchValue->setAlignment(Qt::AlignCenter);
    FOVSwitchValue->setFixedSize(50,50);
    FOVSwitchValue->setPlaceholderText("1～38");
    QHBoxLayout *FOVSwitchlayout = new QHBoxLayout();
    FOVSwitchlayout->addWidget(FOVSwitch);
    FOVSwitchlayout->addWidget(FOVSwitchValue);

    visSelfCheck = new QPushButton("自检");
    visSelfCheck->setObjectName("settingsItemButton");
    visSelfCheck->setFixedSize(120,50);
    manualFocus = new QPushButton("手动聚焦");
    manualFocus->setObjectName("settingsItemButton");
    manualFocus->setFixedSize(120,50);
    autoFocus = new QPushButton("自动聚焦");
    autoFocus->setObjectName("settingsItemButton");
    autoFocus->setFixedSize(120,50);
    semiAutoFocus = new QPushButton("半自动聚焦");
    semiAutoFocus->setObjectName("settingsItemButton");
    semiAutoFocus->setFixedSize(120,50);

    mainLayout->addLayout(visElectronicZoomlayout);
    mainLayout->addLayout(visCrossHairlayout);
    mainLayout->addLayout(sharpnesslayout);
    mainLayout->addLayout(FOVlayout);
    mainLayout->addLayout(gainlayout);
    mainLayout->addLayout(focuslayout);
    mainLayout->addLayout(visBrightnesslayout);
    mainLayout->addLayout(visContrastlayout);
    mainLayout->addLayout(FOVSwitchlayout);
    QHBoxLayout *lay1 = new QHBoxLayout();
    lay1->addWidget(visSelfCheck);
    lay1->addWidget(manualFocus);
    QHBoxLayout *lay2 = new QHBoxLayout();
    lay2->addWidget(autoFocus);
    lay2->addWidget(semiAutoFocus);
    mainLayout->addLayout(lay1);
    mainLayout->addLayout(lay2);


    return page;
}

QWidget* MainWindow::createLaserControl() {
    QWidget *page = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(page);
    laserSelfcheck = new QPushButton("自检");
    laserSelfcheck->setObjectName("settingsItemButton");
    laserSelfcheck->setGeometry(50,50,100,50);
    laserSelfcheck->setFixedSize(100, 50);
    QHBoxLayout *laserDislayout = new QHBoxLayout();
    laserDis = new QPushButton("测距");
    laserDis->setObjectName("settingsItemButton");
    laserDis->setFixedSize(100, 50);
    laserDisValue = new QComboBox();
    laserDisValue->setFixedSize(100, 50);
    laserDisValue->addItem("停止测距",0x00);
    laserDisValue->addItem("单次测距",0x01);
    laserDisValue->addItem("1Hz重频",0x02);
    laserDisValue->addItem("5Hz重频",0x03);
    laserDislayout->addWidget(laserDis);
    laserDislayout->addWidget(laserDisValue);
    mainLayout->addWidget(laserSelfcheck);
    mainLayout->addLayout(laserDislayout);
    return page;
}

void MainWindow::onNavigationClicked(int index)
{
    // 切换到对应页面
    switch (index) {
    case 0:
        contentStack2->setCurrentWidget(m_gimbalWidget);
        break;
    case 1:
        contentStack2->setCurrentWidget(m_irImagWidget);
        break;
    case 2:
        contentStack2->setCurrentWidget(m_visImageWidget);
        break;
    case 3:
        contentStack2->setCurrentWidget(m_laserWidget);
        break;
    }

    // 更新导航按钮样式
    updateNavigationStyle(index);
}

void MainWindow::updateNavigationStyle(int activeIndex)
{
    for (int i = 0; i < navButtons.size(); ++i) {
        QPushButton *button = navButtons[i];
        bool isActive = (i == activeIndex);
        button->setChecked(isActive);

        // 更新样式
        QString style = QString(
            "QPushButton#navButton%1 {"
            "    background-color: %2;"
            "    color: %3;"
            "    border: 2px solid %4;"
            "}"
        ).arg(i)
         .arg(isActive ? "#3498db" : "#f5f5f5")
         .arg(isActive ? "white" : "#333")
         .arg(isActive ? "#2980b9" : "#ddd");

        button->setStyleSheet(style);
    }
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
            qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
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
            qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
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
        sendBuffer[3] = 0x00;
        sendBuffer[4] = ((frontViewPhiValue->text().toInt())*32767/360) & 0xFF;
        sendBuffer[5] = (((frontViewPhiValue->text().toInt())*32767/360) >> 8) & 0xFF;
        sendBuffer[6] = ((frontViewAziValue->text().toInt())*32767/360) & 0xFF;
        sendBuffer[7] = (((frontViewAziValue->text().toInt())*32767/360) >> 8) & 0xFF;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
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
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //下
    connect(m_gimbalDownBtn, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x03;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = ((qRound(phiValue->text().toFloat())-stepValue->value())*32767/360) & 0xFF;
        sendBuffer[5] = (((qRound(phiValue->text().toFloat())-stepValue->value())*32767/360) >> 8) & 0xFF;
        sendBuffer[6] = (qRound(aziValue->text().toFloat())*32767/360) & 0xFF;
        sendBuffer[7] = ((qRound(aziValue->text().toFloat())*32767/360) >> 8) & 0xFF;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //左
    connect(m_gimbalLeftBtn, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x03;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = (qRound(phiValue->text().toFloat())*32767/360) & 0xFF;
        sendBuffer[5] = ((qRound(phiValue->text().toFloat())*32767/360) >> 8) & 0xFF;
        sendBuffer[6] = ((qRound(aziValue->text().toFloat())+stepValue->value())%360*32767/360) & 0xFF;
        sendBuffer[7] = ((qRound(aziValue->text().toFloat()+stepValue->value())%360*32767/360) >> 8) & 0xFF;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //右
    connect(m_gimbalRightBtn, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x03;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = (qRound(phiValue->text().toFloat())*32767/360) & 0xFF;
        sendBuffer[5] = ((qRound(phiValue->text().toFloat())*32767/360) >> 8) & 0xFF;
        sendBuffer[6] = ((qRound(aziValue->text().toFloat())-stepValue->value()+360)%360*32767/360) & 0xFF;
        sendBuffer[7] = ((qRound(aziValue->text().toFloat()-stepValue->value()+360)%360*32767/360) >> 8) & 0xFF;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //扇扫模式
    connect(sectorScan, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = (scanPhiValue->text().toInt()*32767/360) & 0xFF;
        sendBuffer[5] = ((scanPhiValue->text().toInt()*32767/360) >> 8) & 0xFF;
        sendBuffer[6] = (scanRateValue->text().toInt()*32767/360) & 0xFF;
        sendBuffer[7] = ((scanRateValue->text().toInt()*32767/360) >> 8) & 0xFF;
        sendBuffer[8] = (scanRangeValue->text().toInt()*32767/360) & 0xFF;
        sendBuffer[9] = ((scanRangeValue->text().toInt()*32767/360) >> 8) & 0xFF;
        sendBuffer[10] = (scanCenterValue->text().toInt()*32767/360) & 0xFF;
        sendBuffer[11] = ((scanCenterValue->text().toInt()*32767/360) >> 8) & 0xFF;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //周扫模式
    connect(circularScan, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x06;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = (scanPhiValue->text().toInt()*32767/360) & 0xFF;
        sendBuffer[5] = ((scanPhiValue->text().toInt()*32767/360) >> 8) & 0xFF;
        sendBuffer[6] = (scanRateValue->text().toInt()*32767/360) & 0xFF;
        sendBuffer[7] = ((scanRateValue->text().toInt()*32767/360) >> 8) & 0xFF;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //停止扫描
    connect(stopScan, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x01;
        sendBuffer[2] = 0x06;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = (scanPhiValue->text().toInt()*32767/360) & 0xFF;
        sendBuffer[5] = ((scanPhiValue->text().toInt()*32767/360) >> 8) & 0xFF;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //热像自检
    connect(irSelfCheck, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //非均匀校正
    connect(NUC, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = NUCValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //快门校正
    connect(ShutterCorrection, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x01;
        sendBuffer[4] = ShutterCorrectionValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //极性
    connect(imagePolarity, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x03;
        sendBuffer[4] = imagePolarityValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //图像翻转
    connect(imageFlip, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x04;
        sendBuffer[4] = imageFlipValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //对比度
    connect(irContrast, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x05;
        sendBuffer[4] = irContrastValue->text().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //亮度
    connect(irBrightness, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x06;
        sendBuffer[4] = irBrightnessValue->text().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //DDE增强
    connect(DDEEnhancement, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x07;
        sendBuffer[4] = DDEEnhancementValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //图像实域滤波
    connect(temporalFiltering, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x08;
        sendBuffer[4] = temporalFilteringValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //电子变倍
    connect(irElectronicZoom, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x09;
        sendBuffer[4] = irElectronicZoomValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //图像空域滤波
    connect(spatialFiltering, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x01;
        sendBuffer[3] = 0x0A;
        sendBuffer[4] = spatialFilteringValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //图像空域滤波
    connect(irCrossHair, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x02;
        sendBuffer[3] = 0x00;
        sendBuffer[4] = irCrossHairValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光自检
    connect(visSelfCheck, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光亮度
    connect(visBrightness, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x01;
        sendBuffer[4] = visBrightnessValue->text().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光对比度
    connect(visContrast, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x02;
        sendBuffer[4] = visContrastValue->text().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光清晰度恢复
    connect(sharpnessReset, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x03;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光清晰度+
    connect(sharpnessIncrease, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x03;
        sendBuffer[4] = 0x01;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光清晰度-
    connect(sharpnessDecrease, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x03;
        sendBuffer[4] = 0x02;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光视场停止
    connect(FOVstop, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x04;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光视场窄
    connect(FOVNarrow, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x04;
        sendBuffer[4] = 0x01;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光视场宽
    connect(FOVWide, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x04;
        sendBuffer[4] = 0x02;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光电子变倍关
    connect(visElectronicZoomOff, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x05;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光电子变倍开
    connect(visElectronicZoomOn, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x05;
        sendBuffer[4] = 0x01;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光调焦停止
    connect(focusStop, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x06;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光调焦+
    connect(focusIncrease, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x06;
        sendBuffer[4] = 0x01;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光调焦-
    connect(focusDecrease, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x06;
        sendBuffer[4] = 0x02;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光增益恢复
    connect(gainReset, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x07;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光增益+
    connect(gainIncrease, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x07;
        sendBuffer[4] = 0x01;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光增益-
    connect(gainDecrease, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x07;
        sendBuffer[4] = 0x02;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //白光十字光标隐
    connect(visCrossHairOff, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x08;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //白光十字光标显
    connect(visCrossHairOn, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x08;
        sendBuffer[4] = 0x01;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //指定倍数视场切换
    connect(FOVSwitch, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x09;
        sendBuffer[4] = FOVSwitchValue->text().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光手动聚焦
    connect(manualFocus, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x0A;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光自动聚焦
    connect(autoFocus, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x0B;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //可见光半自动聚焦
    connect(semiAutoFocus, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x04;
        sendBuffer[3] = 0x0C;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //激光系统自检
    connect(laserSelfcheck, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x05;
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
    });

    //激光系统测距
    connect(laserDis, &QPushButton::clicked, [this]() {
        unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
        sendBuffer[0] = 0xCB;
        sendBuffer[1] = 0x04;
        sendBuffer[2] = 0x05;
        sendBuffer[3] = 0x01;
        sendBuffer[4] = laserDisValue->currentData().toUInt();
        sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
        qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
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
//    int cropWidth = m_displayImageLabel->width();
//    int cropHeight = m_displayImageLabel->height();
//    int startX = (frame.cols - cropWidth) / 2;
//    int startY = (frame.rows - cropHeight) / 2;

//    // 确保裁剪区域有效
//    startX = std::max(0, startX);
//    startY = std::max(0, startY);
//    cropWidth = std::min(cropWidth, frame.cols - startX);
//    cropHeight = std::min(cropHeight, frame.rows - startY);

//    // 裁剪中心区域
//    cv::Mat croppedFrame = frame(cv::Rect(startX, startY, cropWidth, cropHeight));
    QImage img = VideoPlayer::cvMatToQImage(frame);
    QPixmap pixmap = QPixmap::fromImage(img);
    int scaledWidth = frame.cols / 1.7;
    int scaledHeight = frame.rows / 1.7;

    QPixmap finalPixmap = pixmap.scaled(
        scaledWidth,
        scaledHeight,
        Qt::KeepAspectRatio,  // 保持宽高比
        Qt::SmoothTransformation
    );

    m_displayImageLabel->setPixmap(finalPixmap);

    m_displayImageLabel->setAlignment(Qt::AlignCenter);
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
    sendBuffer[0] = 0xCB;
    sendBuffer[1] = 0x01;
    sendBuffer[2] = 0x05;
    sendBuffer[3] = 0x02;
    sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
    qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
}

void MainWindow::sendStopDetectCommand()
{
    unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};
    sendBuffer[0] = 0xCB;
    sendBuffer[1] = 0x01;
    sendBuffer[2] = 0x05;
    sendBuffer[3] = 0x03;
    sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);
    qint64 bytesSent = udpSocket->writeDatagram((char*)sendBuffer, SENDBUFFER_SIZE_UDP,QHostAddress(REMOTE_IP), UDP_PORT_REMOTE);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    // 只有在检测状态下才能选择区域
    if (isDetecting && m_displayImageLabel->geometry().contains(event->pos())) {
        // 将全局坐标转换为显示区域的局部坐标
        QPoint localPos = m_displayImageLabel->mapFrom(this, event->pos());
        selectionStart = localPos;
        isSelectingRegion = true;
        selectedRegion = QRect(localPos, QSize(0, 0));

        // 更新显示
        m_displayImageLabel->update();
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (isSelectingRegion && isDetecting) {
        isSelectingRegion = false;
        // 将全局坐标转换为显示区域的局部坐标
        QPoint localPos = m_displayImageLabel->mapFrom(this, event->pos());
        selectedRegion.setBottomRight(localPos);

        // 确保区域有效
        if (selectedRegion.width() > 10 && selectedRegion.height() > 10) {
            // 发送手动跟踪指令
            sendManualTrackCommand(selectedRegion);

            // 在列表中显示信息
            QString info = QString("发送手动跟踪指令 - 区域: (%1, %2, %3, %4)")
                            .arg(selectedRegion.x())
                            .arg(selectedRegion.y())
                            .arg(selectedRegion.width())
                            .arg(selectedRegion.height());
            qDebug() << info;
        }

        // 更新显示
        m_displayImageLabel->update();
    }
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isSelectingRegion && isDetecting) {
        QPoint localPos = m_displayImageLabel->mapFrom(this, event->pos());
        selectedRegion.setBottomRight(localPos);
        m_displayImageLabel->update(); // 触发重绘
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::sendManualTrackCommand(const QRect &region)
{
    unsigned char sendBuffer[SENDBUFFER_SIZE_UDP] = {0};

    // 帧头
    sendBuffer[0] = 0xCB;
    // 命令ID - 手动跟踪
    sendBuffer[1] = 0x01;
    sendBuffer[2] = 0x05;
    sendBuffer[3] = 0x00;

    // 填充区域信息 (x, y, width, height)
    // 注意：这里需要根据实际协议格式调整字节顺序
    sendBuffer[4] = int(region.x()*1.7) & 0xFF;
    sendBuffer[5] = (int(region.x()*1.7) >> 8) & 0xFF;

    sendBuffer[6] = int(region.y()*1.7) & 0xFF;
    sendBuffer[7] = (int(region.y()*1.7) >> 8) & 0xFF;

    sendBuffer[8] = region.width() & 0xFF;
    sendBuffer[9] = (region.width() >> 8) & 0xFF;

    sendBuffer[10] = region.height() & 0xFF;
    sendBuffer[11] = (region.height() >> 8) & 0xFF;

    // 计算校验和
    sendBuffer[SENDBUFFER_SIZE_UDP - 1] = CalCheckNum(&sendBuffer[0], SENDBUFFER_SIZE_UDP - 2);

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
    if (packet.frameHeader == 0xC0) {
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

        if (packet.systemFaultInfo.bits.servoFault) {
            ServoStatusLabel->setProperty("status", "fault");  // 故障状态
            ServoStatusLabel->setText("伺服故障");
        } else {
            ServoStatusLabel->setProperty("status", "normal");  // 正常状态
            ServoStatusLabel->setText("伺服正常");
        }
        ServoStatusLabel->style()->unpolish(ServoStatusLabel);
        ServoStatusLabel->style()->polish(ServoStatusLabel);
        ServoStatusLabel->update();

        if (!packet.systemFaultInfo.bits.trackerFault){
            trackStatusLabel->setProperty("status", "fault");  // 故障状态
            trackStatusLabel->setText("跟踪故障");
        }
        else {
            trackStatusLabel->setProperty("status", "normal");  // 正常状态
            trackStatusLabel->setText("跟踪正常");
        }
        trackStatusLabel->style()->unpolish(trackStatusLabel);
        trackStatusLabel->style()->polish(trackStatusLabel);
        trackStatusLabel->update();

        if (packet.systemFaultInfo.bits.thermalFault) {
            irStatusLabel->setProperty("status", "fault");  // 故障状态
            irStatusLabel->setText("热像故障");
        }
        else{
            irStatusLabel->setProperty("status", "normal");  // 正常状态
            irStatusLabel->setText("热像正常");
        }
        irStatusLabel->style()->unpolish(irStatusLabel);
        irStatusLabel->style()->polish(irStatusLabel);
        irStatusLabel->update();

        if (packet.systemFaultInfo.bits.whiteLightFault) {
            visStatusLabel->setProperty("status", "fault");  // 故障状态
            visStatusLabel->setText("可见光故障");
        }
        else{
            visStatusLabel->setProperty("status", "normal");  // 正常状态
            visStatusLabel->setText("可见光正常");
        }
        visStatusLabel->style()->unpolish(visStatusLabel);
        visStatusLabel->style()->polish(visStatusLabel);
        visStatusLabel->update();

        if (packet.systemFaultInfo.bits.laserFault){
            laserStatusLabel->setProperty("status", "fault");  // 故障状态
            laserStatusLabel->setText("激光故障");
        }
        else{
            laserStatusLabel->setProperty("status", "normal");  // 正常状态
            laserStatusLabel->setText("激光正常");
        }
        laserStatusLabel->style()->unpolish(laserStatusLabel);
        laserStatusLabel->style()->polish(laserStatusLabel);
        laserStatusLabel->update();

//        qDebug() << packet.systemFaultInfo.bits.servoFault <<
//                    packet.systemFaultInfo.bits.trackerFault <<
//                    packet.systemFaultInfo.bits.thermalFault <<
//                    packet.systemFaultInfo.bits.whiteLightFault <<
//                    packet.systemFaultInfo.bits.laserFault;

        aziValue->setText(QString::number(((float)packet.aziPosition/10000), 'f', 4));
        phiValue->setText(QString::number(((float)packet.pitchPosition/10000), 'f', 4));
        aziRateValue->setText(QString::number(((float)packet.aziVelocity*1024/32767), 'f', 4));
        phiRateValue->setText(QString::number(((float)packet.pitchVelocity*1024/32767), 'f', 4));
    }
//    qDebug() << "开始接收数据：";
//    for (int i = 0; i < 8; i++) {
//        qDebug()<<QString("0x%1").arg(static_cast<quint8>(data[i*8+0]),2,16,QLatin1Char('0'))<<
//              QString("0x%1").arg(static_cast<quint8>(data[i*8+1]),2,16,QLatin1Char('0'))<<
//              QString("0x%1").arg(static_cast<quint8>(data[i*8+2]),2,16,QLatin1Char('0'))<<
//              QString("0x%1").arg(static_cast<quint8>(data[i*8+3]),2,16,QLatin1Char('0'))<<
//              QString("0x%1").arg(static_cast<quint8>(data[i*8+4]),2,16,QLatin1Char('0'))<<
//              QString("0x%1").arg(static_cast<quint8>(data[i*8+5]),2,16,QLatin1Char('0'))<<
//              QString("0x%1").arg(static_cast<quint8>(data[i*8+6]),2,16,QLatin1Char('0'))<<
//              QString("0x%1").arg(static_cast<quint8>(data[i*8+7]),2,16,QLatin1Char('0'));
//    }

}

void MainWindow::updateDateTime()
{
    QString currentTime = QDateTime::currentDateTime().toString("hh:mm AP");
    timeLabel->setText(currentTime);
}

void MainWindow::setupStyles()
{
    // 应用全局样式表
    QString styleSheet = R"(
        /* 主窗口 */
        QMainWindow {
            background-color: #f8f9fa;
        }

        /* 顶部状态栏 */
        QWidget#topBar {
            background-color: white;
            border-bottom: 2px solid #e0e0e0;
        }

        QLabel#titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #2c3e50;
        }

        QLabel#statusLabel {
            font-size: 14px;
            color: #e74c3c;
            font-weight: bold;
        }

        QLabel#timeLabel {
            font-size: 20px;
            font-weight: bold;
            color: #3498db;
        }

         QLabel[status="fault"] {
             font-size: 16px;
             color: #e74c3c;           /* 红色文字 */
             font-weight: bold;
             padding: 5px 15px;
             background-color: #fdedec;  /* 浅红色背景 */
             border-radius: 4px;
             border: 1px solid #e74c3c;
         }

         QLabel[status="normal"] {
             font-size: 16px;
             color: #27ae60;           /* 绿色文字 */
             font-weight: bold;
             padding: 5px 15px;
             background-color: #d5f4e6;  /* 浅绿色背景 */
             border-radius: 4px;
             border: 1px solid #27ae60;
         }

         QLabel[status="unknown"] {
             font-size: 16px;
             color: #7f8c8d;           /* 灰色文字 */
             font-weight: bold;
             padding: 5px 15px;
             background-color: #ecf0f1;  /* 浅灰色背景 */
             border-radius: 4px;
             border: 1px dashed #bdc3c7;
         }

        /* 左侧导航栏 */
        QWidget#navigationBar {
            background-color: white;
            border-right: 2px solid #e0e0e0;
        }

        /* 导航按钮基础样式 */
        QPushButton[objectName^="navButton"] {
            text-align: left;
            padding-left: 20px;
            font-size: 16px;
            font-weight: bold;
            border-radius: 8px;
            border: 2px solid #ddd;
            background-color: #f5f5f5;
            color: #333;
        }

        QPushButton[objectName^="navButton"]:hover {
            background-color: #e8f4fc;
            border-color: #3498db;
        }

        QPushButton[objectName^="navButton"]:checked {
            background-color: #3498db;
            color: white;
            border-color: #2980b9;
        }

        /* 内容区域 */
        QWidget#contentStack {
            background-color: white;
            border-radius: 10px;
            margin: 20px;
        }

        /* 页面标题 */
        QLabel#pageTitle {
            font-size: 28px;
            font-weight: bold;
            color: #2c3e50;
            padding-bottom: 10px;
            border-bottom: 3px solid #3498db;
        }

        QLabel#label {
            font-weight: bold;
        }


        /* 设置项按钮 */
        QPushButton#settingsItemButton {
            text-align: left;
            padding-left: 15px;
            font-size: 16px;
            font-weight: bold;
            background-color: #f8f9fa;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
        }

        QPushButton#settingsItemButton:hover {
            background-color: #e3f2fd;
            border-color: #3498db;
            transform: translateY(-2px);
        }

        QPushButton#settingsItemButton:pressed {
            background-color: #bbdefb;
        }

        QPushButton#settingsItem1Button {
            text-align: left;
            padding-left: 20px;
            font-size: 16px;
            font-weight: bold;
            background-color: #f8f9fa;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
        }

        QPushButton#settingsItem1Button:hover {
            background-color: #e3f2fd;
            border-color: #3498db;
            transform: translateY(-2px);
        }

        QPushButton#settingsItem1Button:pressed {
            background-color: #bbdefb;
        }


        /* 底部按钮栏 */
        QWidget#bottomBar {
            background-color: white;
            border-top: 2px solid #e0e0e0;
        }

        /* 保存按钮 */
        QPushButton#saveButton {
            background-color: #27ae60;
            color: white;
            font-size: 18px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
        }

        QPushButton#saveButton:hover {
            background-color: #219653;
        }

        QPushButton#saveButton:pressed {
            background-color: #1e8449;
        }

        /* 取消按钮 */
        QPushButton#cancelButton {
            background-color: #e74c3c;
            color: white;
            font-size: 18px;
            font-weight: bold;
            border: none;
            border-radius: 8px;
        }

        QPushButton#cancelButton:hover {
            background-color: #c0392b;
        }

        QPushButton#cancelButton:pressed {
            background-color: #a93226;
        }

        /* 页面标签 */
        QLabel#pageLabel {
            font-size: 24px;
            color: #7f8c8d;
        }
    )";

    setStyleSheet(styleSheet);
}

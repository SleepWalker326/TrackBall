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
    resize(1500, 715);
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
    m_rightPanelWidget->setFixedSize(601, 701);
    QVBoxLayout *rightPanelLayout = new QVBoxLayout(m_rightPanelWidget);
    rightPanelLayout->setSpacing(10);  // 设置间距

    // 创建TabWidget
    QTabWidget *controlTabWidget = new QTabWidget();
    controlTabWidget->setFixedSize(591, 341);  // 调整高度，为表格留出空间

    // 创建各个控制模块的Tab页
    createGimbalControl();
    createIrImagControl();
    createVisImageControl();
    createLaserControl();
    createControl();
    // 将各个控制模块添加到Tab页
    controlTabWidget->addTab(m_gimbalWidget, "云台控制");
    controlTabWidget->addTab(m_irImagWidget, "热像图像控制");
    controlTabWidget->addTab(m_visImageWidget, "白光操作控制");
    controlTabWidget->addTab(m_laserWidget, "激光操作控制");
    controlTabWidget->addTab(m_controlWidget, "操作控制");

    // 创建表格区域
    QWidget *tableWidget = new QWidget();
    tableWidget->setFixedSize(591, 300);  // 表格区域高度
    QVBoxLayout *tableLayout = new QVBoxLayout(tableWidget);

    // 创建表格标题
    QLabel *tableTitle = new QLabel();
    tableTitle->setAlignment(Qt::AlignCenter);
    tableTitle->setStyleSheet("font-weight: bold; font-size: 14px; margin: 5px;");

    // 创建表格（暂时留空，不编辑内容）
    m_dataTable = new QTableWidget();
    m_dataTable->setFixedSize(581, 280);
    m_dataTable->setColumnCount(4);  // 设置4列
    m_dataTable->setRowCount(20);     // 设置5行

    // 设置表头
    QStringList headers;
    headers << "目标类别" << "目标俯仰角" << "目标方位角" << "目标状态";
    m_dataTable->setHorizontalHeaderLabels(headers);

    // 设置表格样式
    m_dataTable->setStyleSheet("QTableWidget { border: 1px solid #ccc; background-color: white; }"
                              "QHeaderView::section { background-color: #f0f0f0; padding: 5px; border: 1px solid #ddd; }");

    // 暂时禁用编辑
    m_dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 添加到表格布局
//    tableLayout->addWidget(tableTitle);
    tableLayout->addWidget(m_dataTable);

    // 将TabWidget和表格添加到右侧面板布局
    rightPanelLayout->addStretch();
    rightPanelLayout->addWidget(controlTabWidget);
    rightPanelLayout->addStretch();
    rightPanelLayout->addWidget(tableWidget);
    rightPanelLayout->addStretch();

}

void MainWindow::createGimbalControl()
{
    // 改为创建普通Widget而不是GroupBox
    m_gimbalWidget = new QWidget();
    m_gimbalWidget->setFixedSize(591, 301); // 调整高度以适应Tab页

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
    frontViewAziValue = new QLineEdit("0");
    frontViewAziValue->setFixedSize(70,25);
    frontViewPhi = new QLabel("前视俯仰");
    frontViewPhi->setFixedSize(70,40);
    frontViewPhiValue = new QLineEdit("0");
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
    stopScan = new QPushButton("停止扫描");
    stopScan->setFixedSize(100,40);
    QVBoxLayout *scanBtnlayout = new QVBoxLayout();
    scanBtnlayout->addWidget(sectorScan);
    scanBtnlayout->addWidget(circularScan);
    scanBtnlayout->addWidget(stopScan);
    scanPhi = new QLabel("扫描俯仰");
    scanPhi->setFixedSize(70,40);
    scanPhiValue = new QLineEdit("0");
    scanPhiValue->setFixedSize(70,25);
    QHBoxLayout *scanPhilayout = new QHBoxLayout();
    scanPhilayout->addWidget(scanPhi);
    scanPhilayout->addWidget(scanPhiValue);
    scanRate = new QLabel("扫描速度");
    scanRate->setFixedSize(70,40);
    scanRateValue = new QLineEdit("0");
    scanRateValue->setFixedSize(70,25);
    QHBoxLayout *scanRatelayout = new QHBoxLayout();
    scanRatelayout->addWidget(scanRate);
    scanRatelayout->addWidget(scanRateValue);
    scanRange = new QLabel("扫描范围");
    scanRange->setFixedSize(70,40);
    scanRangeValue = new QLineEdit("0");
    scanRangeValue->setFixedSize(70,25);
    QHBoxLayout *scanRangelayout = new QHBoxLayout();
    scanRangelayout->addWidget(scanRange);
    scanRangelayout->addWidget(scanRangeValue);
    scanCenter = new QLabel("扫描中心");
    scanCenter->setFixedSize(70,40);
    scanCenterValue = new QLineEdit("0");
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
    mainLayout->addWidget(window4,1,1);
}

void MainWindow::createIrImagControl()
{
    m_irImagWidget = new QWidget();
    m_irImagWidget->setFixedSize(591, 330);

    QGridLayout *mainLayout = new QGridLayout(m_irImagWidget);

    //自检
    irSelfCheck = new QPushButton("自检");
    irSelfCheck->setFixedSize(80,30);

    //非均匀校正
    NUC = new QPushButton("非均匀校正");
    NUC->setFixedSize(80,30);
    NUCValue = new QComboBox();
    NUCValue->setFixedSize(70,30);
    NUCValue->addItem("手动",0x00);
    NUCValue->addItem("自动",0x01);
    QHBoxLayout *NUClayout = new QHBoxLayout();
    NUClayout->addWidget(NUC);
    NUClayout->addWidget(NUCValue);

    // 快门校正
    ShutterCorrection = new QPushButton("快门校正");
    ShutterCorrection->setFixedSize(70,30);
    ShutterCorrectionValue = new QComboBox();
    ShutterCorrectionValue->setFixedSize(105,30);
    ShutterCorrectionValue->addItem("背景校正",0x00);
    ShutterCorrectionValue->addItem("双稳态快门",0x01);
    QHBoxLayout *ShutterCorrectionlayout = new QHBoxLayout();
    ShutterCorrectionlayout->addWidget(ShutterCorrection);
    ShutterCorrectionlayout->addWidget(ShutterCorrectionValue);

    // 极性
    imagePolarity = new QPushButton("极性");
    imagePolarity->setFixedSize(70,30);
    imagePolarityValue = new QComboBox();
    imagePolarityValue->setFixedSize(70,30);
    imagePolarityValue->addItem("白热",0x00);
    imagePolarityValue->addItem("黑热",0x01);
    QHBoxLayout *imagePolaritylayout = new QHBoxLayout();
    imagePolaritylayout->addWidget(imagePolarity);
    imagePolaritylayout->addWidget(imagePolarityValue);

    // 图像翻转
    imageFlip = new QPushButton("图像翻转");
    imageFlip->setFixedSize(70,30);
    imageFlipValue = new QComboBox();
    imageFlipValue->setFixedSize(95,30);
    imageFlipValue->addItem("不翻转",0x00);
    imageFlipValue->addItem("上下翻转",0x01);
    imageFlipValue->addItem("左右翻转",0x02);
    imageFlipValue->addItem("对角翻转",0x03);
    QHBoxLayout *imageFliplayout = new QHBoxLayout();
    imageFliplayout->addWidget(imageFlip);
    imageFliplayout->addWidget(imageFlipValue);

    //对比度
    irContrast = new QPushButton("对比度");
    irContrast->setFixedSize(70,30);
    irContrastValue = new QLineEdit("128");
    irContrastValue->setFixedSize(70,30);
    irContrastValue->setPlaceholderText("0～255");
    QHBoxLayout *irContrastlayout = new QHBoxLayout();
    irContrastlayout->addWidget(irContrast);
    irContrastlayout->addWidget(irContrastValue);

    //亮度
    irBrightness = new QPushButton("亮度");
    irBrightness->setFixedSize(70,30);
    irBrightnessValue = new QLineEdit("128");
    irBrightnessValue->setFixedSize(70,30);
    irBrightnessValue->setPlaceholderText("0～255");
    QHBoxLayout *irBrightnesslayout = new QHBoxLayout();
    irBrightnesslayout->addWidget(irBrightness);
    irBrightnesslayout->addWidget(irBrightnessValue);

    // DDE增强
    DDEEnhancement = new QPushButton("DDE增强");
    DDEEnhancement->setFixedSize(70,30);
    DDEEnhancementValue = new QComboBox();
    DDEEnhancementValue->setFixedSize(70,30);
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
    temporalFiltering->setFixedSize(100,30);
    temporalFilteringValue = new QComboBox();
    temporalFilteringValue->setFixedSize(80,30);
    temporalFilteringValue->addItem("滤波关",0x00);
    temporalFilteringValue->addItem("滤波开",0x01);
    QHBoxLayout *temporalFilteringlayout = new QHBoxLayout();
    temporalFilteringlayout->addWidget(temporalFiltering);
    temporalFilteringlayout->addWidget(temporalFilteringValue);

    // 电子变倍
    irElectronicZoom = new QPushButton("电子变倍");
    irElectronicZoom->setFixedSize(70,30);
    irElectronicZoomValue = new QComboBox();
    irElectronicZoomValue->setFixedSize(70,30);
    irElectronicZoomValue->addItem("1");
    irElectronicZoomValue->addItem("2");
    irElectronicZoomValue->addItem("3");
    irElectronicZoomValue->addItem("4");
    QHBoxLayout *irElectronicZoomlayout = new QHBoxLayout();
    irElectronicZoomlayout->addWidget(irElectronicZoom);
    irElectronicZoomlayout->addWidget(irElectronicZoomValue);

    //图像空域滤波
    spatialFiltering = new QPushButton("图像空域滤波");
    spatialFiltering->setFixedSize(100,30);
    spatialFilteringValue = new QComboBox();
    spatialFilteringValue->setFixedSize(80,30);
    spatialFilteringValue->addItem("滤波关",0x00);
    spatialFilteringValue->addItem("滤波开",0x01);
    QHBoxLayout *spatialFilteringlayout = new QHBoxLayout();
    spatialFilteringlayout->addWidget(spatialFiltering);
    spatialFilteringlayout->addWidget(spatialFilteringValue);

    //热像十字光标显隐
    irCrossHair = new QPushButton("十字光标显隐");
    irCrossHair->setFixedSize(100,30);
    irCrossHairValue = new QComboBox();
    irCrossHairValue->setFixedSize(70,30);
    irCrossHairValue->addItem("隐藏",0x00);
    irCrossHairValue->addItem("显示",0x01);
    QHBoxLayout *irCrossHairlayout = new QHBoxLayout();
    irCrossHairlayout->addWidget(irCrossHair);
    irCrossHairlayout->addWidget(irCrossHairValue);

    mainLayout->addWidget(irSelfCheck,0,0);
    mainLayout->addLayout(irCrossHairlayout,0,1);
    mainLayout->addLayout(NUClayout,1,0);
    mainLayout->addLayout(ShutterCorrectionlayout,1,1);
    mainLayout->addLayout(imagePolaritylayout,2,0);
    mainLayout->addLayout(imageFliplayout,2,1);
    mainLayout->addLayout(irContrastlayout,3,0);
    mainLayout->addLayout(irBrightnesslayout,3,1);
    mainLayout->addLayout(DDEEnhancementlayout,4,0);
    mainLayout->addLayout(irElectronicZoomlayout,4,1);
    mainLayout->addLayout(temporalFilteringlayout,5,0);
    mainLayout->addLayout(spatialFilteringlayout,5,1);

}

void MainWindow::createVisImageControl()
{
    m_visImageWidget = new QWidget();
    m_visImageWidget->setFixedSize(591, 330);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_visImageWidget);

    //亮度
    visBrightness = new QPushButton("亮度");
    visBrightness->setFixedSize(70,30);
    visBrightnessValue = new QLineEdit("50");
    visBrightnessValue->setFixedSize(50,30);
    visBrightnessValue->setPlaceholderText("0～100");
    QHBoxLayout *visBrightnesslayout = new QHBoxLayout();
    visBrightnesslayout->addWidget(visBrightness);
    visBrightnesslayout->addWidget(visBrightnessValue);

    //对比度
    visContrast = new QPushButton("对比度");
    visContrast->setFixedSize(70,30);
    visContrastValue = new QLineEdit("50");
    visContrastValue->setFixedSize(50,30);
    visContrastValue->setPlaceholderText("0～100");
    QHBoxLayout *visContrastlayout = new QHBoxLayout();
    visContrastlayout->addWidget(visContrast);
    visContrastlayout->addWidget(visContrastValue);

    //清晰度
    sharpness = new QLabel("清晰度");
    sharpness->setFixedSize(70,30);
    sharpnessReset = new QPushButton("恢复");
    sharpnessReset->setFixedSize(50,30);
    sharpnessIncrease = new QPushButton("+");
    sharpnessIncrease->setFixedSize(30,30);
    sharpnessDecrease = new QPushButton("-");
    sharpnessDecrease->setFixedSize(30,30);
    QHBoxLayout *sharpnesslayout = new QHBoxLayout();
    sharpnesslayout->addWidget(sharpness);
    sharpnesslayout->addWidget(sharpnessReset);
    sharpnesslayout->addWidget(sharpnessIncrease);
    sharpnesslayout->addWidget(sharpnessDecrease);

    //视场
    FOV = new QLabel("视场");
    FOV->setFixedSize(70,30);
    FOVstop = new QPushButton("停止");
    FOVstop->setFixedSize(50,30);
    FOVNarrow = new QPushButton("窄");
    FOVNarrow->setFixedSize(30,30);
    FOVWide = new QPushButton("宽");
    FOVWide->setFixedSize(30,30);
    QHBoxLayout *FOVlayout = new QHBoxLayout();
    FOVlayout->addWidget(FOV);
    FOVlayout->addWidget(FOVstop);
    FOVlayout->addWidget(FOVNarrow);
    FOVlayout->addWidget(FOVWide);

    // 电子变倍
    visElectronicZoom = new QLabel("电子变倍");
    visElectronicZoom->setFixedSize(70,30);
    visElectronicZoomOff = new QPushButton("关");
    visElectronicZoomOff->setFixedSize(30,30);
    visElectronicZoomOn = new QPushButton("开");
    visElectronicZoomOn->setFixedSize(30,30);
    QHBoxLayout *visElectronicZoomlayout = new QHBoxLayout();
    visElectronicZoomlayout->addWidget(visElectronicZoom);
    visElectronicZoomlayout->addWidget(visElectronicZoomOff);
    visElectronicZoomlayout->addWidget(visElectronicZoomOn);

    //调焦
    focus = new QLabel("调焦");
    focus->setFixedSize(70,30);
    focusStop = new QPushButton("停止");
    focusStop->setFixedSize(50,30);
    focusIncrease = new QPushButton("+");
    focusIncrease->setFixedSize(30,30);
    focusDecrease = new QPushButton("-");
    focusDecrease->setFixedSize(30,30);
    QHBoxLayout *focuslayout = new QHBoxLayout();
    focuslayout->addWidget(focus);
    focuslayout->addWidget(focusStop);
    focuslayout->addWidget(focusIncrease);
    focuslayout->addWidget(focusDecrease);

    //增益
    gain = new QLabel("增益");
    gain->setFixedSize(70,30);
    gainReset = new QPushButton("恢复");
    gainReset->setFixedSize(50,30);
    gainIncrease = new QPushButton("+");
    gainIncrease->setFixedSize(30,30);
    gainDecrease = new QPushButton("-");
    gainDecrease->setFixedSize(30,30);
    QHBoxLayout *gainlayout = new QHBoxLayout();
    gainlayout->addWidget(gain);
    gainlayout->addWidget(gainReset);
    gainlayout->addWidget(gainIncrease);
    gainlayout->addWidget(gainDecrease);

    //白光十字光标显隐
    visCrossHair = new QLabel("十字光标显隐");
    visCrossHair->setFixedSize(100,30);
    visCrossHairOff = new QPushButton("关");
    visCrossHairOff->setFixedSize(30,30);
    visCrossHairOn = new QPushButton("开");
    visCrossHairOn->setFixedSize(30,30);
    QHBoxLayout *visCrossHairlayout = new QHBoxLayout();
    visCrossHairlayout->addWidget(visCrossHair);
    visCrossHairlayout->addWidget(visCrossHairOff);
    visCrossHairlayout->addWidget(visCrossHairOn);

    //指定倍数视场切换
    FOVSwitch = new QPushButton("指定倍数视场切换");
    FOVSwitch->setFixedSize(130,30);
    FOVSwitchValue = new QLineEdit("1");
    FOVSwitchValue->setFixedSize(50,30);
    FOVSwitchValue->setPlaceholderText("1～38");
    QHBoxLayout *FOVSwitchlayout = new QHBoxLayout();
    FOVSwitchlayout->addWidget(FOVSwitch);
    FOVSwitchlayout->addWidget(FOVSwitchValue);

    visSelfCheck = new QPushButton("自检");
    visSelfCheck->setFixedSize(120,30);
    manualFocus = new QPushButton("手动聚焦");
    manualFocus->setFixedSize(120,30);
    autoFocus = new QPushButton("自动聚焦");
    autoFocus->setFixedSize(120,30);
    semiAutoFocus = new QPushButton("半自动聚焦");
    semiAutoFocus->setFixedSize(120,30);

    QHBoxLayout *layout1 = new QHBoxLayout();
    layout1->addStretch();
    layout1->addLayout(visElectronicZoomlayout);
    layout1->addStretch();
    layout1->addLayout(visCrossHairlayout);
    layout1->addStretch();
    QHBoxLayout *layout2 = new QHBoxLayout();
    layout2->addStretch();
    layout2->addLayout(sharpnesslayout);
    layout2->addStretch();
    layout2->addLayout(FOVlayout);
    layout2->addStretch();
    QHBoxLayout *layout3 = new QHBoxLayout();
    layout3->addStretch();
    layout3->addLayout(gainlayout);
    layout3->addStretch();
    layout3->addLayout(focuslayout);
    layout3->addStretch();
    QHBoxLayout *layout4 = new QHBoxLayout();
    layout4->addLayout(visBrightnesslayout);
    layout4->addLayout(visContrastlayout);
    layout4->addLayout(FOVSwitchlayout);
    QHBoxLayout *visBtnlayout = new QHBoxLayout();
    visBtnlayout->addWidget(visSelfCheck);
    visBtnlayout->addWidget(manualFocus);
    visBtnlayout->addWidget(autoFocus);
    visBtnlayout->addWidget(semiAutoFocus);

    mainLayout->addStretch();
    mainLayout->addLayout(layout1);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(layout2);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(layout3);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(layout4);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(visBtnlayout);
    mainLayout->addStretch();
}

void MainWindow::createLaserControl() {
    m_laserWidget = new QWidget();
    m_laserWidget->setFixedSize(591, 330);

    QHBoxLayout *mainLayout = new QHBoxLayout(m_laserWidget);
    laserSelfcheck = new QPushButton("自检");
    laserSelfcheck->setFixedSize(100, 35);
    QHBoxLayout *laserDislayout = new QHBoxLayout();
    laserDis = new QPushButton("测距");
    laserDis->setFixedSize(100, 35);
    laserDisValue = new QComboBox();
    laserDisValue->setFixedSize(100, 35);
    laserDisValue->addItem("停止测距",0x00);
    laserDisValue->addItem("单次测距",0x01);
    laserDisValue->addItem("1Hz重频",0x02);
    laserDisValue->addItem("5Hz重频",0x03);
    laserDislayout->addWidget(laserDis);
    laserDislayout->addWidget(laserDisValue);
    mainLayout->addWidget(laserSelfcheck);
    mainLayout->addLayout(laserDislayout);

}

void MainWindow::createControl()
{
    m_controlWidget = new QWidget();
    m_controlWidget->setFixedSize(591, 320);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_controlWidget);

    // 模式选择
    QWidget *modeWidget = new QWidget();
    modeWidget->setFixedSize(591, 100);
    QVBoxLayout *modeLayout = new QVBoxLayout(modeWidget);

    // 图像类型选择
    QWidget *imageTypeWidget = new QWidget();
    imageTypeWidget->setFixedSize(591, 45);
    QHBoxLayout *imageTypeLayout = new QHBoxLayout(imageTypeWidget);

    QLabel *imageTypeTitle = new QLabel("图像类型");
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

    QLabel *videoSourceTitle = new QLabel("视频源");
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
    QWidget *controlWidget = new QWidget();
    controlWidget->setFixedSize(591, 150);
    QGridLayout *controlLayout = new QGridLayout(controlWidget);

    createControlButtons();

    controlLayout->addWidget(m_connectionBtn,0,0);
    controlLayout->addWidget(m_singleTargetTrackBtn,0,1);
    controlLayout->addWidget(m_screenshotBtn,0,2);
    controlLayout->addWidget(m_startDetectionBtn,1,0);
    controlLayout->addWidget(m_multiTargetTrackBtn,1,1);
    controlLayout->addWidget(m_startRecordBtn,1,2);

    // 添加到主布局
    mainLayout->addSpacing(20);
    mainLayout->addWidget(modeWidget);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(controlWidget);
    mainLayout->addStretch();

}

void MainWindow::createControlButtons()
{
    m_connectionBtn = new QPushButton("连接");
    m_connectionBtn->setFixedSize(120,35);
    m_startDetectionBtn = new QPushButton("开始检测");
    m_startDetectionBtn->setFixedSize(120,35);
    m_multiTargetTrackBtn = new QPushButton("多目标跟踪");
    m_multiTargetTrackBtn->setFixedSize(120,35);
    m_singleTargetTrackBtn = new QPushButton("单目标跟踪");
    m_singleTargetTrackBtn->setFixedSize(120,35);
    m_startRecordBtn = new QPushButton("开始录制");
    m_startRecordBtn->setFixedSize(120,35);
    m_screenshotBtn = new QPushButton("截图");
    m_screenshotBtn->setFixedSize(120,35);
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
    //    if (packet.systemFaultInfo.bits.servoFault)
    //        QMessageBox::information(this, "伺服故障", "伺服系统发生故障！");
    //    if (packet.systemFaultInfo.bits.trackerFault)
    //        QMessageBox::information(this, "跟踪故障", "跟踪系统发生故障！");
    //    if (packet.systemFaultInfo.bits.thermalFault)
    //        QMessageBox::information(this, "热像故障", "热像系统发生故障！");
    //    if (packet.systemFaultInfo.bits.whiteLightFault)
    //        QMessageBox::information(this, "白光故障", "白光系统发生故障！");
    //    if (packet.systemFaultInfo.bits.laserFault)
    //        QMessageBox::information(this, "激光故障", "激光系统发生故障！");
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



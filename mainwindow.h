// MainWindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QUdpSocket>
#include <QSpinBox>
#include <QTableWidget>
#include <QtGlobal>
#include <QIntValidator>

#include "videoplayer.h"  // 添加头文件包含
#include "mediarecorder.h"
#include "statusfeedbackpacket.h"


//#define UDP_AVAILDADA_SIZE 389
//#define SENDBUFFER_SIZE_UDP 392

/*
上位机给板卡发送指令的端口为40213
上位机接收板卡发送数据"192.168.1.211:40212"
*/


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // 用于更新显示信息的槽函数
    void updateFrameRate(const QString &frameRate);
    void updateResolution(const QString &resolution);
    void updateTimestamp(const QString &timestamp);
    void onFrameCaptured(cv::Mat frame);
    void onVideoError(QString error);
    void onStartDetectClicked();
    void readPendingDatagrams();

private:
    void createUI();
    void createMainArea();
    void createDisplayArea();
    QWidget* createInfoArea();
    void createControlButtons();
    void createGimbalControl();
    void createIrImagControl();
    void createVisImageControl();
    void createLaserControl();
    void createControl();
    void createConnections();
    void createControlPanel();
    void sendStartDetectCommand();
    void sendStopDetectCommand();
    unsigned char CalCheckNum(unsigned char data[], int num);
    void parseReceivedData(const QByteArray &data);

    QWidget *m_centralWidget;

    // 主显示区域组件
    QWidget *m_mainDisplayWidget;
    QLabel *m_displayImageLabel;

    // 信息显示区域
    QLabel *m_frameRateLabel;
    QLabel *m_frameRateValueLabel;
    QLabel *m_resolutionLabel;
    QLabel *m_resolutionValueLabel;
    QLabel *m_timestampLabel;
    QLabel *m_timestampValueLabel;

    QWidget *m_gimbalWidget;
    QWidget *m_irImagWidget;
    QWidget *m_visImageWidget;
    QWidget *m_laserWidget;
    QWidget *m_controlWidget;
    QWidget *m_rightPanelWidget;
    QTableWidget  *m_dataTable;
    // 底部控制按钮
    QPushButton *m_connectionBtn;
    QPushButton *m_startDetectionBtn;
    QPushButton *m_multiTargetTrackBtn;
    QPushButton *m_singleTargetTrackBtn;
    QPushButton *m_startRecordBtn;
    QPushButton *m_screenshotBtn;

    // 右侧控制面板
    QWidget *m_controlPanelWidget;

    // 云台控制组件
    QLabel *step;
    QSpinBox *stepValue;
    QLabel *aziRate;
    QLabel *aziRateValue;
    QLabel *phiRate;
    QLabel *phiRateValue;
    QPushButton *frontView;
    QLabel *frontViewAzi;
    QLineEdit *frontViewAziValue;
    QLabel *frontViewPhi;
    QLineEdit *frontViewPhiValue;
    QPushButton *m_gimbalUpBtn;
    QPushButton *m_gimbalLeftBtn;
    QPushButton *m_gimbalDownBtn;
    QPushButton *m_gimbalRightBtn;
    QPushButton *m_gimbalStopBtn;
    QLabel *azi;
    QLabel *aziValue;
    QLabel *phi;
    QLabel *phiValue;
    QPushButton *sectorScan;
    QPushButton *circularScan;
    QPushButton *stopScan;
    QLabel *scanPhi;
    QLineEdit *scanPhiValue;
    QLabel *scanRate;
    QLineEdit *scanRateValue;
    QLabel *scanRange;
    QLineEdit *scanRangeValue;
    QLabel *scanCenter;
    QLineEdit *scanCenterValue;
    // 热像图像操作组件
    QPushButton *irSelfCheck;
    QPushButton *NUC;
    QComboBox *NUCValue;
    QPushButton *ShutterCorrection;
    QComboBox *ShutterCorrectionValue;
    QPushButton *imagePolarity;
    QComboBox *imagePolarityValue;
    QPushButton *imageFlip;
    QComboBox *imageFlipValue;
    QPushButton *irContrast;
    QLineEdit *irContrastValue;
    QPushButton *irBrightness;
    QLineEdit *irBrightnessValue;
    QPushButton *DDEEnhancement;
    QComboBox *DDEEnhancementValue;
    QPushButton *temporalFiltering;
    QComboBox *temporalFilteringValue;
    QPushButton *irElectronicZoom;
    QComboBox *irElectronicZoomValue;
    QPushButton *spatialFiltering;
    QComboBox *spatialFilteringValue;
    QPushButton *irCrossHair;
    QComboBox *irCrossHairValue;
    // 白光操作组件
    QPushButton *visSelfCheck;
    QPushButton *visBrightness;
    QLineEdit *visBrightnessValue;
    QPushButton *visContrast;
    QLineEdit *visContrastValue;
    QLabel *sharpness;
    QPushButton *sharpnessReset;
    QPushButton *sharpnessDecrease;
    QPushButton *sharpnessIncrease;
    QLabel *FOV;
    QPushButton *FOVstop;
    QPushButton *FOVNarrow;
    QPushButton *FOVWide;
    QLabel *visElectronicZoom;
    QPushButton *visElectronicZoomOff;
    QPushButton *visElectronicZoomOn;
    QLabel *focus;
    QPushButton *focusStop;
    QPushButton *focusIncrease;
    QPushButton *focusDecrease;
    QLabel *gain;
    QPushButton *gainReset;
    QPushButton *gainIncrease;
    QPushButton *gainDecrease;
    QLabel *visCrossHair;
    QPushButton *visCrossHairOff;
    QPushButton *visCrossHairOn;
    QPushButton *FOVSwitch;
    QLineEdit *FOVSwitchValue;
    QPushButton *manualFocus;
    QPushButton *autoFocus;
    QPushButton *semiAutoFocus;

    // 控制组件
    QPushButton *m_imageType_vis;
    QPushButton *m_imageType_ir;
    QPushButton *m_videoSource_stream;
    QPushButton *m_videoSource_local;
    //激光控制组件
    QPushButton *laserSelfcheck;
    QPushButton *laserDis;
    QComboBox *laserDisValue;
    // 视频播放相关
    VideoPlayer *m_videoPlayer;
    bool m_isVideoPlaying;
    QString filePath = "/home/sleepwalker/EOSTRACKER/assets/configs/rtsp2webrtc.json";
    QMetaObject::Connection m_frameConnection;  // 保存帧捕获信号的连接
    QMetaObject::Connection m_errorConnection;  // 保存错误信号的连接
    MediaRecorder *m_mediaRecorder;  // 媒体录制器

    //检测跟踪
    bool isDetecting;

    // 网络通信
    QUdpSocket *udpSocket;
    // 目标信息
    float visFov;
    float irFov;

    // 常量
    static const int UDP_PORT_LOCAL = 40210;
    static const int UDP_PORT_REMOTE = 40211;
    static const char* REMOTE_IP;
    static const int SENDBUFFER_SIZE_UDP = 16;  // 命令包大小
    unsigned char crossOrient;
    unsigned char crossStep;
};

#endif // MAINWINDOW_H

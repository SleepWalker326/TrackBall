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
#include "videoplayer.h"  // 添加头文件包含
#include "mediarecorder.h"

/*
上位机给板卡发送指令的端口为40213
上位机接收板卡发送数据"192.168.1.211:40212"
*/
// 命令枚举定义
enum SendEnum
{
    SelfCheckBack0 = 0x01,
    idleBack,
    manualTrackBack,
    tarDetectBack,
    videoStabilityBack,
    specifyTrackBack,
    idTrackBack,
    cancelTrackBack,
    chaBlankBack,
    videoDisplayBack,
    videoSavedBack,
    gateSetBack,
    videoCompressBack,
    infoDisplayBack
};
enum videoDisplayEnum
{
    visibleDis = 0x01,
    irDis = 0x02,
    mwirDis = 0x03,
    none
};
//目标检测
enum tarDetecteEnumBack
{
    stopDetect = 1,
    startDetect,
    DetectPro
};

// 协议帧结构定义
typedef struct {
    unsigned char m_RsvHeader;      // 帧头
    unsigned char m_rsvID;          // 命令ID
    unsigned char m_rData[510];     // 数据区域
    unsigned char rsv_Check;        // 校验和
} ReceiveFrame;

// 发送帧结构定义
typedef struct {
    unsigned char m_ComponentID;    // 组件ID
    unsigned char m_Avail[509];     // 有效数据
    unsigned char u_Check;          // 校验和
} SendFrameUDP;

// 目标信息结构
typedef struct {
    int id;                         // 目标ID
    int targetClass;                // 目标类别
    unsigned int cx;                // 中心X坐标
    unsigned int cy;                // 中心Y坐标
    float az;                       // 方位角
    float pi;                       // 俯仰角
} TargetInfo;

// 枚举定义

enum RecvEnum_DC
{
    StateSend_DC=0x01,
};


// 数据转换联合体
union SHORT2CHAR {
    short val;
    unsigned char arr[2];
};

union USHORT2CHAR {
    unsigned short val;
    unsigned char arr[2];
};

union FLOAT2CHAR {
    float val;
    unsigned char arr[4];
};



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
    void createLensControl();
    void createImageControl();
    void createConnections();
    void createControlPanel();
    void sendStartDetectCommand();
    void sendStopDetectCommand();
    unsigned char CalCheckNum(unsigned char data[], int num);
    void parseReceivedData(const QByteArray &data);

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
    QGroupBox *m_gimbalGroup;
    QPushButton *m_gimbalUpBtn;
    QPushButton *m_gimbalLeftBtn;
    QPushButton *m_gimbalDownBtn;
    QPushButton *m_gimbalRightBtn;
    QPushButton *m_gimbalStopBtn;
    QLabel *m_moveSpeedLabel;
    QLineEdit *m_moveSpeedEdit;
    QLabel *m_angleStepLabel;
    QLineEdit *m_angleStepEdit;
    QPushButton *m_gimbalResetBtn;
    QPushButton *m_autoScanBtn;

    // 镜头控制组件
    QGroupBox *m_lensGroup;
    QLabel *m_zoomLabel;
    QPushButton *m_zoomInBtn;
    QPushButton *m_zoomOutBtn;
    QLabel *m_fovLabel;
    QPushButton *m_fovLargeBtn;
    QPushButton *m_fovSmallBtn;
    QPushButton *m_autoFocusBtn;
    QPushButton *m_lensResetBtn;

    // 图像控制组件
    QGroupBox *m_imageGroup;
    QComboBox *m_imageTypeCombo;
    QComboBox *m_videoSourceCombo;
    QLabel *m_brightnessLabel;
    QPushButton *m_brightnessUpBtn;
    QPushButton *m_brightnessDownBtn;
    QLabel *m_contrastLabel;
    QPushButton *m_contrastUpBtn;
    QPushButton *m_contrastDownBtn;
    QWidget *m_centralWidget;

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
    QList<TargetInfo> detectedTargets;
    float visFov;
    float irFov;

    // 常量
    static const int UDP_PORT_LOCAL = 40212;
    static const int UDP_PORT_REMOTE = 40213;
    static const char* REMOTE_IP;
    static const int SENDBUFFER_SIZE_UDP = 64;  // 命令包大小

};

#endif // MAINWINDOW_H

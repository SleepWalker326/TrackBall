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
#include "videoplayer.h"  // 添加头文件包含
#include "mediarecorder.h"


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

};

#endif // MAINWINDOW_H

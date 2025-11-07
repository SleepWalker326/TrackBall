#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QThread>
#include <QMutex>
#include <QString>
#include <opencv2/opencv.hpp>
#include <QImage>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include <QDebug>

class VideoCaptureThread : public QThread {
    Q_OBJECT
public:
    VideoCaptureThread(QString rtspUrl, QObject *parent = nullptr);
    ~VideoCaptureThread();
    void stop();
    double getFrameRate() const;
    int getFrameWidth() const;
    int getFrameHeight() const;

signals:
    void frameCaptured(cv::Mat frame);
    void errorOccurred(QString error);

protected:
    void run() override;

private:
    QString m_rtspUrl;
    cv::VideoCapture m_cap;
    bool m_running;
    QMutex m_mutex;
};

class VideoPlayer {
public:
    VideoPlayer();
    ~VideoPlayer();

    bool startPlayback(const QString &rtspUrl);
    void stopPlayback();
    QString readRtspUrlFromJson(const QString &filePath);
    static QImage cvMatToQImage(const cv::Mat &mat);

    VideoCaptureThread* getCaptureThread() const { return m_captureThread; }

private:
    VideoCaptureThread *m_captureThread;
    QString m_rtspAddress;       // RTSP地址

};

#endif // VIDEOPLAYER_H

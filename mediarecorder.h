#ifndef MEDIARECORDER_H
#define MEDIARECORDER_H

#include <QObject>
#include <QString>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <QDateTime>

class MediaRecorder : public QObject {
    Q_OBJECT
public:
    explicit MediaRecorder(QObject *parent = nullptr);
    ~MediaRecorder();

    // 设置录像参数
    void setRecordParams(int width, int height, double fps);

    // 截图
    bool saveScreenshot(const QImage &image, const QString &savePath = "");

    // 开始录像
    bool startRecording(const QString &videoPath = "");

    // 停止录像
    void stopRecording();

    // 写入视频帧
    void writeFrame(const cv::Mat &frame);

    // 获取录制状态
    bool isRecording() const { return m_isRecording; }

private:
    cv::VideoWriter m_videoWriter;  // OpenCV视频写入器
    bool m_isRecording;             // 录制状态
    int m_frameWidth;               // 帧宽度
    int m_frameHeight;              // 帧高度
    double m_frameRate;             // 帧率
    QString generateTimestampFilename(const QString &prefix, const QString &suffix);
    const QString SCREENSHOT_DIR = "screenshots";   // 截图子目录
    const QString RECORDING_DIR = "recordings";     // 录像子目录
};

#endif // MEDIARECORDER_H

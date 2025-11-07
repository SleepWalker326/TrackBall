#include "mediarecorder.h"
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QCoreApplication>  // 新增：用于获取应用程序路径

MediaRecorder::MediaRecorder(QObject *parent)
    : QObject(parent),
      m_isRecording(false),
      m_frameWidth(0),
      m_frameHeight(0),
      m_frameRate(30.0) {}

MediaRecorder::~MediaRecorder() {
    if (m_isRecording) {
        stopRecording();
    }
}

void MediaRecorder::setRecordParams(int width, int height, double fps) {
    m_frameWidth = width;
    m_frameHeight = height;
    m_frameRate = fps;
}

bool MediaRecorder::saveScreenshot(const QImage &image, const QString &savePath) {
    if (image.isNull()) {
        qWarning() << "无法保存截图：图像为空";
        return false;
    }
    // 创建存储目录
    QString appDir = QCoreApplication::applicationDirPath(); // 获取build目录路径
    QString screenshotDir = appDir + "/" + SCREENSHOT_DIR;

    // 递归创建目录（如果不存在）
    if (!QDir().mkpath(screenshotDir)) {
        qWarning() << "无法创建截图目录：" << screenshotDir;
        return false;
    }

    // 生成默认路径（当前目录+时间戳）
    QString filePath = savePath;
    if (filePath.isEmpty()) {
        QString fileName = generateTimestampFilename("screenshot", ".png");
        filePath = screenshotDir + "/" + fileName;
    }

    // 保存图像
    bool success = image.save(filePath);
    if (success) {
        qDebug() << "截图保存成功：" << filePath;
    } else {
        qWarning() << "截图保存失败：" << filePath;
    }
    return success;
}

bool MediaRecorder::startRecording(const QString &videoPath) {
    if (m_isRecording) {
        qWarning() << "已经在录制中";
        return false;
    }

    if (m_frameWidth <= 0 || m_frameHeight <= 0) {
        qWarning() << "请先设置有效的视频参数";
        return false;
    }

    // 创建存储目录
    QString appDir = QCoreApplication::applicationDirPath();
    QString recordingDir = appDir + "/" + RECORDING_DIR;
    if (!QDir().mkpath(recordingDir)) {
        qWarning() << "无法创建录像目录：" << recordingDir;
        return false;
    }

    // 生成默认视频路径
    QString filePath = videoPath;
    if (filePath.isEmpty()) {
        QString fileName = generateTimestampFilename("recording", ".avi");
        filePath = recordingDir + "/" + fileName;
    }

    // 打开视频写入器（使用MJPG编码）
    int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    m_videoWriter.open(filePath.toStdString(), fourcc, m_frameRate,
                      cv::Size(m_frameWidth, m_frameHeight));

    if (!m_videoWriter.isOpened()) {
        qWarning() << "无法打开视频文件进行录制：" << filePath;
        return false;
    }

    m_isRecording = true;
    qDebug() << "开始录制视频：" << filePath;
    return true;
}

void MediaRecorder::stopRecording() {
    if (!m_isRecording) return;

    m_videoWriter.release();
    m_isRecording = false;
    qDebug() << "停止录制视频";
}

void MediaRecorder::writeFrame(const cv::Mat &frame) {
    if (!m_isRecording || frame.empty()) return;

    // 确保帧尺寸与录制参数一致
    if (frame.cols != m_frameWidth || frame.rows != m_frameHeight) {
        cv::Mat resizedFrame;
        cv::resize(frame, resizedFrame, cv::Size(m_frameWidth, m_frameHeight));
        m_videoWriter.write(resizedFrame);
    } else {
        m_videoWriter.write(frame);
    }
}

QString MediaRecorder::generateTimestampFilename(const QString &prefix, const QString &suffix) {
    // 生成带时间戳的文件名（格式：prefix_yyyyMMdd_hhmmss_suffix）
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    return QString("%1_%2%3").arg(prefix).arg(timestamp).arg(suffix);
}





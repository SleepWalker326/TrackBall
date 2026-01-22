#include "videoplayer.h"
#include <QTime>

VideoCaptureThread::VideoCaptureThread(QString rtspUrl, QObject *parent)
    : QThread(parent), m_rtspUrl(rtspUrl), m_running(false) {}

VideoCaptureThread::~VideoCaptureThread() {
    stop();
}

void VideoCaptureThread::stop() {
    m_mutex.lock();
    m_running = false;
    if (m_cap.isOpened()) {
        m_cap.release();
    }
    m_mutex.unlock();
    wait();
}

double VideoCaptureThread::getFrameRate() const {
    if (m_cap.isOpened()) {
        return m_cap.get(cv::CAP_PROP_FPS);
    }
    return 0.0;
}

int VideoCaptureThread::getFrameWidth() const {
    if (m_cap.isOpened()) {
        return m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
    }
    return 0;
}

int VideoCaptureThread::getFrameHeight() const {
    if (m_cap.isOpened()) {
        return m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    }
    return 0;
}

void VideoCaptureThread::run() {
    m_mutex.lock();
    m_running = true;
    m_mutex.unlock();

    // 显式指定FFmpeg后端打开RTSP流
    if (!m_cap.open(m_rtspUrl.toStdString(), cv::CAP_FFMPEG)) {
        emit errorOccurred("无法打开RTSP流: " + m_rtspUrl);
        return;
    }

    // 可选：设置缓存大小，减少网络波动影响
    m_cap.set(cv::CAP_PROP_BUFFERSIZE, 2);

    cv::Mat frame;
    while (m_running) {
        m_mutex.lock();
        bool readSuccess = m_cap.read(frame);
        m_mutex.unlock();  // 提前释放锁
        if (readSuccess) {
            emit frameCaptured(frame.clone());
        } else {
            emit errorOccurred("无法读取视频帧");
            m_mutex.lock();
            m_running = false;
            m_mutex.unlock();
        }
        msleep(30); // 控制帧率
    }
}

VideoPlayer::VideoPlayer() : m_captureThread(nullptr) {}

VideoPlayer::~VideoPlayer() {
    stopPlayback();
}

bool VideoPlayer::startPlayback(const QString &rtspUrl) {
    stopPlayback(); // 确保之前的线程已停止

    if (rtspUrl.isEmpty()) {
        return false;
    }

    m_captureThread = new VideoCaptureThread(rtspUrl);
    m_captureThread->start();
    return true;
}

void VideoPlayer::stopPlayback() {
    if (m_captureThread) {
        m_captureThread->stop();
        delete m_captureThread;
        m_captureThread = nullptr;
    }
}

QString VideoPlayer::readRtspUrlFromJson(const QString &filePath) {
//    QFile file(filePath);

//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        QMessageBox::critical(nullptr, "错误", "无法打开配置文件: " + filePath);
//        return "";
//    }

//    QByteArray data = file.readAll();
//    file.close();

//    QJsonDocument doc = QJsonDocument::fromJson(data);
//    if (doc.isNull()) {
//        QMessageBox::critical(nullptr, "错误", "配置文件格式错误");
//        return "";
//    }

//    QJsonObject obj = doc.object();
//    if (obj.contains("rtsp_url")) {
//        return obj["rtsp_url"].toString();
//    } else {
//        QMessageBox::critical(nullptr, "错误", "配置文件中未找到rtsp_url");
//        return "";
//    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(nullptr, "错误", "无法打开配置文件 rtsp2webrtc.json");
        return "";
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        QMessageBox::warning(nullptr, "错误", "配置文件格式错误");
        return "";
    }

    QJsonObject rootObj = doc.object();

    // 1. 解析streams对象（根对象下的"streams"字段）
    if (!rootObj.contains("streams")) {
        QMessageBox::warning(nullptr, "错误", "配置文件中未找到streams节点");
        return "";
    }
    QJsonObject streamsObj = rootObj["streams"].toObject();
    if (streamsObj.isEmpty()) {
        QMessageBox::warning(nullptr, "错误", "streams节点格式错误");
        return "";
    }

    // 2. 解析RTSP_TEST对象（streams下的"RTSP_TEST"字段）
    if (!streamsObj.contains("RTSP_TEST")) {
        QMessageBox::warning(nullptr, "错误", "streams中未找到RTSP_TEST节点");
        return "";
    }
    QJsonObject rtspTestObj = streamsObj["RTSP_TEST"].toObject();
    if (rtspTestObj.isEmpty()) {
        QMessageBox::warning(nullptr, "错误", "RTSP_TEST节点格式错误");
        return "";
    }

    // 3. 解析url（RTSP_TEST下的"url"字段）
    m_rtspAddress = rtspTestObj["url"].toString();
    if (m_rtspAddress.isEmpty()) {
        QMessageBox::warning(nullptr, "错误", "配置文件中未找到RTSP地址（url）");
        return "";
    }

    qDebug() << "加载RTSP地址:" << m_rtspAddress;

    return m_rtspAddress;

}

QImage VideoPlayer::cvMatToQImage(const cv::Mat &mat) {
    if (mat.empty()) {
        return QImage();
    }

    // 转换颜色空间 (OpenCV默认是BGR格式，Qt需要RGB格式)
    cv::Mat rgbMat;
    if (mat.type() == CV_8UC3) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
    } else if (mat.type() == CV_8UC1) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_GRAY2RGB);
    } else {
        return QImage();
    }

    // 转换为QImage
    return QImage(rgbMat.data, rgbMat.cols, rgbMat.rows,
                 rgbMat.step,
                 QImage::Format_RGB888).copy();
}

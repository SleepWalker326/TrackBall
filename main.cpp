#include "mainwindow.h"
#if 1
#include <QMetaType>  // 包含元类型头文件
#include <opencv2/opencv.hpp>  // 包含cv::Mat定义
#include <QApplication>
#include <QUdpSocket>
#include <QThread>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 注册cv::Mat类型，确保跨线程信号槽传递正常
    qRegisterMetaType<cv::Mat>("cv::Mat");
    // 可选：注册const引用类型（如果信号使用const引用）
    qRegisterMetaType<cv::Mat>("const cv::Mat&");
    MainWindow w;
    w.show();
    return a.exec();
}
#endif

#if 0
#include <QCoreApplication>
#include <QUdpSocket>
#include <QTimer>
#include <QTime>
#include <QDebug>
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QUdpSocket *udpSocket = new QUdpSocket();

    // 绑定到具体IP
    if (!udpSocket->bind(QHostAddress("192.168.1.211"), 40212)) {
        qDebug() << "绑定失败:" << udpSocket->errorString();
        return -1;
    }

    qDebug() << "🚀 UDP发送器已启动";
    qDebug() << "本地地址:" << udpSocket->localAddress().toString();
    qDebug() << "本地端口:" << udpSocket->localPort();
    qDebug() << "目标地址: 192.168.1.100:40213";
    qDebug() << "========================================";

    QTimer timer;
    int sendCount = 0;

    QObject::connect(&timer, &QTimer::timeout, [udpSocket, &sendCount]() {
        sendCount++;
        QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");


        // 二进制数据
        unsigned char binaryData[] = {
            0x48, 0x45, 0x4C, 0x4C, 0x4F,  // HELLO
            0x00, 0x01, 0x02, 0x03,        // 一些二进制
            0xAA, 0xBB, 0xCC, 0xDD         // 更多二进制
        };
        qint64 bytesSent = udpSocket->writeDatagram(
            reinterpret_cast<char*>(binaryData),
            sizeof(binaryData),
            QHostAddress("192.168.1.100"),
            40213
        );
        qDebug() << "🔢 发送二进制数据 #" << sendCount << ":" << (bytesSent > 0 ? "成功" : "失败") << "- 大小:" << sizeof(binaryData) << "字节";


        if (sendCount >= 15) {
            qDebug() << "========================================";
            qDebug() << "✅ 测试完成，共发送15次数据";
            QCoreApplication::quit();
        }
    });

    timer.start(1000);  // 每秒发送一次

    return app.exec();
}
#endif

#if 0
#include <QCoreApplication>
#include <QUdpSocket>
#include <QTimer>
#include <QTime>
#include <QDebug>
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QUdpSocket *udpSocket = new QUdpSocket();

    // 绑定到具体IP
    if (!udpSocket->bind(QHostAddress("192.168.1.211"), 40212)) {
        qDebug() << "绑定失败:" << udpSocket->errorString();
        return -1;
    }

    qDebug() << "🚀 UDP发送器已启动";
    qDebug() << "本地地址:" << udpSocket->localAddress().toString();
    qDebug() << "本地端口:" << udpSocket->localPort();
    qDebug() << "目标地址: 192.168.1.100:40213";
    qDebug() << "========================================";

    while(1)
    {
//        if (udpSocket->hasPendingDatagrams()) {
//            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "接收成功！";
//            break;
//        }
//        else
//            qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "接收失败！";
        if (udpSocket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(udpSocket->pendingDatagramSize());

            udpSocket->readDatagram(datagram.data(), datagram.size());
            const unsigned char *buffer = reinterpret_cast<const unsigned char*>(datagram.constData());
            qDebug() << buffer[0];
        }
    }

    return app.exec();
}
#endif




// main.cpp
#include "mainwindow.h"
#include <QMetaType>  // 包含元类型头文件
#include <opencv2/opencv.hpp>  // 包含cv::Mat定义
#include <QApplication>
#include <QUdpSocket>
#include <QThread>
// 加载样式表
QString loadStyleSheet() {
    return R"(
        QWidget {
            font-family: "Microsoft YaHei", "Segoe UI";
            font-size: 14px;
        }
        QPushButton {
            border-radius: 8px;
            padding: 10px;
            font-size: 16px;
            font-weight: bold;
        }
    )";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // 注册cv::Mat类型，确保跨线程信号槽传递正常
    qRegisterMetaType<cv::Mat>("cv::Mat");
    // 可选：注册const引用类型（如果信号使用const引用）
    qRegisterMetaType<cv::Mat>("const cv::Mat&");

    // 设置应用程序样式
    app.setStyleSheet(loadStyleSheet());

    MainWindow window;
    window.showMaximized();  // 平板设备全屏显示

    return app.exec();
}



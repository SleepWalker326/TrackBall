#include "mainwindow.h"
#include <QMetaType>  // 包含元类型头文件
#include <opencv2/opencv.hpp>  // 包含cv::Mat定义
#include <QApplication>

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

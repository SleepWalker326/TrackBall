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

//int main() {
//    // 1. 输出OpenCV版本（确认是否是刚编译的4.11.0）
//    std::cout << "OpenCV版本: " << CV_VERSION << std::endl;

//    // 2. 获取编译信息
//    std::string buildInfo = cv::getBuildInformation();

//    // 3. 更可靠的FFmpeg检查逻辑：先找"FFmpeg:"，再看后面是否有"YES"
//    size_t ffmpegPos = buildInfo.find("FFmpeg:");
//    bool hasFFmpeg = false;
//    if (ffmpegPos != std::string::npos) {
//        // 从"FFmpeg:"位置开始，往后查找"YES"
//        hasFFmpeg = (buildInfo.find("YES", ffmpegPos) != std::string::npos);
//    }

//    // 4. 输出结果
//    std::cout << "是否支持FFmpeg: " << (hasFFmpeg ? "是" : "否") << std::endl;

//    // 5. （可选）更直观的验证：尝试读取一个视频文件（如MP4）
//    cv::VideoCapture cap("/home/sleepwalker/QtPrj/build-TrackBall-Desktop_Qt_5_12_9_GCC_64bit-Debug/records/record_20251027_152937.mp4"); // 替换为你的视频文件路径
//    if (cap.isOpened()) {
//        std::cout << "成功打开视频文件，FFmpeg功能正常！" << std::endl;
//        cap.release();
//    } else {
//        std::cout << "无法打开视频文件（可能文件不存在，或格式不支持）" << std::endl;
//    }

//    return 0;
//}





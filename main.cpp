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
#include <cstdint>   // 必须包含：定义uint16_t/uint8_t
#include <QDebug>    // Qt打印（非必需，可替换为printf）

/**
 * @brief 判断当前系统是否为小端序（Little Endian）
 * @return true: 小端序（x86/ARM主流架构）; false: 大端序（部分嵌入式/网络协议）
 */
bool isLittleEndian() {
    // 定义16位测试值：0x0102（高位字节0x01，低位字节0x02）
    const uint16_t testValue = 0x0102;
    // 强制转换为uint8_t*，读取第一个字节（低地址字节）
    const uint8_t firstByte = *reinterpret_cast<const uint8_t*>(&testValue);

    // 小端序：低地址存低位字节（0x02）；大端序：低地址存高位字节（0x01）
    return (firstByte == 0x02);
}

// 扩展：直接返回字节序类型（更易读）
enum EndianType {
    LittleEndian,  // 小端
    BigEndian,     // 大端
    UnknownEndian  // 罕见：混合端（极少遇到）
};

EndianType getSystemEndian() {
    uint32_t testValue = 0x01020304; // 32位测试值，覆盖更多字节场景
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&testValue);

    if (bytes[0] == 0x04 && bytes[1] == 0x03 && bytes[2] == 0x02 && bytes[3] == 0x01) {
        return LittleEndian; // 小端：低地址→04 03 02 01
    } else if (bytes[0] == 0x01 && bytes[1] == 0x02 && bytes[2] == 0x03 && bytes[3] == 0x04) {
        return BigEndian;    // 大端：低地址→01 02 03 04
    } else {
        return UnknownEndian;// 混合端（几乎不会遇到）
    }
}
int main() {
    // 极简判断
    if (isLittleEndian()) {
        qDebug() << "当前系统是小端序（Little Endian）";
    } else {
        qDebug() << "当前系统是大端序（Big Endian）";
    }

    // 详细类型判断
    EndianType endian = getSystemEndian();
    switch (endian) {
        case LittleEndian:
            qDebug() << "字节序：小端（主流x86/ARM架构）";
            break;
        case BigEndian:
            qDebug() << "字节序：大端（部分嵌入式/网络设备）";
            break;
        case UnknownEndian:
            qDebug() << "字节序：混合端（罕见）";
            break;
    }

    return 0;
}
#endif




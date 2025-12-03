

#include <QByteArray>
#include <QDataStream>
#include <QDebug>
#include <cstdint>
#pragma pack(push, 1) // 保证1字节对齐

/**
 * @brief 主控到显控状态反馈报文结构体 (64字节)
 * 对应协议中的 "主控到显控状态反馈1" 表格
 */
struct StatusFeedbackPacket
{
    // 0. 帧头
    uint8_t frameHeader;           // 0xC0

    // 1. 系统主状态
    enum SystemMainMode : uint8_t {
        MODE_INIT = 0x00,          // 初始模式
        MODE_PARKING = 0x01,       // 停车模式
        MODE_SELF_TEST = 0x02,     // 自检模式
        MODE_FORWARD_VIEW = 0x03,  // 前视模式
        MODE_SECTOR_SCAN = 0x04,   // 扇扫模式
        MODE_TRACKING = 0x05,      // 跟踪模式
        MODE_PERIMETER_SCAN = 0x06,// 周扫模式
        MODE_DEBUG = 0x07,         // 调试模式
        MODE_OTHER = 0xFF          // 其它模式
    };
    uint8_t systemMainMode;

    // 2. 系统子状态
    uint8_t systemSubMode;

    // 3. 系统样机标识码
    uint8_t turretId;              // 转塔编号

    // 4. 系统总故障信息 (bit位)
    union {
        struct {
            uint8_t servoFault : 1;    // bit0: 伺服故障
            uint8_t trackerFault : 1;  // bit1: 跟踪故障
            uint8_t thermalFault : 1;  // bit2: 热像故障
            uint8_t whiteLightFault : 1;// bit3: 白光故障
            uint8_t laserFault : 1;    // bit4: 激光故障
            uint8_t reserved1 : 2;     // bit5-6: 备用
            uint8_t otherFault : 1;    // bit7: 其它故障
        } bits;
        uint8_t byteValue;
    } systemFaultInfo;

    // 5-6. 系统自检信息 (16位)
    union {
        struct {
            uint16_t servoSelfTest : 2;     // bit0-1: 伺服自检 (00:未完成,01:成功,10:失败)
            uint16_t trackerSelfTest : 2;   // bit2-3: 跟踪自检
            uint16_t thermalSelfTest : 2;   // bit4-5: 热像自检
            uint16_t ccdSelfTest : 2;       // bit6-7: CCD自检
            uint16_t laserSelfTest : 2;     // bit8-9: 激光自检
            uint16_t reserved2 : 6;         // bit10-15: 备用
        } bits;
        uint16_t wordValue;
    } selfTestInfo;

    // 7-8. 伺服故障码 (16位)
    union {
        struct {
            // 高字节 (7)
            uint8_t pitchControlFault : 1;      // bit0: 俯仰控制异常
            uint8_t pitchPositionFault : 1;     // bit1: 俯仰驱动位置异常
            uint8_t pitchOvercurrentFault : 1;  // bit2: 俯仰驱动过流故障
            uint8_t pitchOutputFault : 1;       // bit3: 俯仰驱动输出故障
            uint8_t aziControlFault : 1;        // bit4: 方位控制异常
            uint8_t aziPositionFault : 1;       // bit5: 方位驱动位置异常
            uint8_t aziOvercurrentFault : 1;    // bit6: 方位驱动过流故障
            uint8_t aziOutputFault : 1;         // bit7: 方位驱动输出故障
            // 低字节 (8)
            uint8_t commFault : 1;              // bit8: 驱动通讯异常
            uint8_t gyroFault : 1;              // bit9: 陀螺信息异常
            uint8_t reserved3 : 6;              // bit10-15: 其它故障/备用
        } bits;
        uint16_t wordValue;
    } servoFaultCode;

    // 9. 跟踪故障码
    union {
        struct {
            uint8_t reserved4 : 2;         // bit0-1: 备用
            uint8_t visibleOutputFault : 1;// bit2: 可见光输出故障
            uint8_t infraredOutputFault : 1;// bit3: 红外输出故障
            uint8_t receiveTimeoutFault : 1;// bit4: 接收时间故障
            uint8_t reserved5 : 3;         // bit5-7: 其它故障/备用
        } bits;
        uint8_t byteValue;
    } trackerFaultCode;

    // 10. 热像故障码
    union {
        struct {
            uint8_t imageOutputFault : 1;  // bit0: 图像输出故障
            uint8_t reserved6 : 7;         // bit1-7: 备用
        } bits;
        uint8_t byteValue;
    } thermalFaultCode;

    // 11. 白光故障码
    union {
        struct {
            uint8_t whiteImageOutputFault : 1; // bit0: 图像输出故障
            uint8_t reserved7 : 7;             // bit1-7: 备用
        } bits;
        uint8_t byteValue;
    } whiteLightFaultCode;

    // 12. 激光故障码
    uint8_t laserFaultCode;              // 全部备用

    // 13. 通信故障码
    union {
        struct {
            uint8_t commandError : 1;     // bit0: 命令字错误
            uint8_t timeoutError : 1;     // bit1: 工作超时
            uint8_t checksumError : 1;    // bit2: 校验错误
            uint8_t uartTimeout : 1;      // bit3: 串口通信超时
            uint8_t reserved8 : 4;        // bit4-7: 备用
        } bits;
        uint8_t byteValue;
    } commFaultCode;

    // 14. 伺服主状态
    enum ServoMainMode : uint8_t {
        SERVO_OFF = 0x00,                // 伺服关机
        SERVO_POSITION = 0x03,           // 电机位置
        SERVO_TRACKING = 0x04,           // 电机跟踪
        SERVO_SELF_TEST = 0x05,          // 电机自检
        SERVO_OTHER = 0xFF               // 其它模式
    };
    uint8_t servoMainMode;

    // 15. 伺服位置子状态
    union {
        struct {
            uint8_t pitchMode : 4;       // 低四位: 俯仰向状态 (0:位置指向,1:预定速度)
            uint8_t aziMode : 4;         // 高四位: 方位向状态 (0:位置指向,1:预定速度,2:扇扫搜索,3:周扫搜索)
        } bits;
        uint8_t byteValue;
    } servoPositionSubMode;

    // 16. 使能状态字
    union {
        struct {
            uint8_t pitchEnable : 1;     // bit0: 俯仰向电机使能
            uint8_t aziEnable : 1;       // bit1: 方位向电机使能
            uint8_t reserved9 : 6;       // bit2-7: 备用
        } bits;
        uint8_t byteValue;
    } enableStatus;

    // 17-20. 俯仰角位置 (4字节, 小端序)
    int32_t pitchPosition;                 // 单位: 度, LSB=0.0001°

    // 21-24. 方位角位置 (4字节, 小端序)
    int32_t aziPosition;                   // 单位: 度, LSB=0.0001°

    // 25-26. 俯仰角速度 (2字节, 小端序)
    int16_t pitchVelocity;               // 单位: °/s, LSB=1024/32767

    // 27-28. 方位角速度 (2字节, 小端序)
    int16_t aziVelocity;                 // 单位: °/s, LSB=1024/32767

    // 29. 跟踪器状态
    enum TrackerStatus : uint8_t {
        TRACKER_IDLE = 0x00,             // 空闲模式
        TRACKER_TRACKING = 0x01,         // 跟踪状态
        TRACKER_DETECTION = 0x02,        // 目标粗检测
        TRACKER_LOST = 0x03,             // 目标丢失
        TRACKER_OTHER = 0xFF             // 其它
    };
    uint8_t trackerStatus;

    // 30-31. 热像状态信息 (16位)
    union {
        struct {
            uint16_t nucMode : 2;        // bit0-1: 非均匀性状态 (0:手动,1:自动)
            uint16_t ddeLevel : 4;       // bit2-5: 图像DDE档位 (1~8)
            uint16_t timeFilter : 1;     // bit6: 图像时域滤波开关 (0:关,1:开)
            uint16_t reserved10 : 2;     // bit7-8: 备用
            uint16_t shutterCorrection : 1;// bit9: 快门校正 (0:背景校正,1:双稳态快门)
            uint16_t imagePolarity : 1;  // bit10: 图像极性 (0:白热,1:黑热)
            uint16_t saveStatus : 1;     // bit11: 参数保存状态 (0:失败,1:成功)
            uint16_t reserved11 : 4;     // bit12-15: 备用
        } bits;
        uint16_t wordValue;
    } thermalStatus;

    // 32. 热像亮度
    uint8_t thermalBrightness;           // 范围: 0-255, LSB=1

    // 33. 热像对比度
    uint8_t thermalContrast;             // 范围: 0-255, LSB=1

    // 34-35. 保留
    uint8_t reserved12[2];

    // 36-37. 保留
    uint8_t reserved13[2];

    // 38. CCD状态信息
    union {
        struct {
            uint8_t targetVisible : 1;   // bit0: 靶标显隐 (0:隐藏,1:显示)
            uint8_t reserved14 : 7;      // bit1-7: 备用
        } bits;
        uint8_t byteValue;
    } ccdStatus;

    // 39. CCD亮度
    uint8_t ccdBrightness;               // 范围: 0-255, LSB=1

    // 40. CCD对比度
    uint8_t ccdContrast;                 // 范围: 0-127, LSB=2 (实际值需*2)

    // 41. 激光工作模式
    union {
        struct {
            uint8_t rangingMode : 4;     // bit0-3: 测距模式 (0:停止,1:单次,2:连续)
            uint8_t continuousFreq : 4;  // bit4-7: 连续测距频率
        } bits;
        uint8_t byteValue;
    } laserMode;

    // 42-45. 激光测距信息 (4字节, 小端序)
    float laserDistance;                 // 单位: 米, LSB=1

    // 46-47. X向脱靶量 (2字节, 小端序)
    int16_t xMissDistance;               // 方位向脱靶量, LSB=1

    // 48-49. Y向脱靶量 (2字节, 小端序)
    int16_t yMissDistance;               // 俯仰向脱靶量, LSB=1

    // 50. 视频源状态
    enum VideoSource : uint8_t {
        VIDEO_WHITE_LIGHT = 0,           // 白光
        VIDEO_THERMAL = 1                // 热像
    };
    uint8_t videoSource;

    // 51-52. 扫描范围 (2字节, 小端序)
    int16_t scanRange;                   // 单位: 度, LSB=360/32767 → 实际值=scanRange*(360.0/32767.0)

    // 53-54. 中心位置 (2字节, 小端序)
    int16_t scanCenter;                  // 单位: 度, LSB=360/32767 → 实际值=scanCenter*(360.0/32767.0)

    // 55-62. 备用
    uint8_t reserved15[8];

    // 63. 和校验
    uint8_t checksum;
};

#pragma pack(pop) // 恢复默认对齐

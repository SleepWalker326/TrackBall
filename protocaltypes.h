//#ifndef PROTOCALTYPES_H
//#define PROTOCALTYPES_H

////#define RMPROTOCAL  0 //是否是6k协议,0为6k
//#define SENDBUFFER_SIZE 16
//#define COM_AVAILDADA_SIZE 13
//#define RSV_AVAILDATA_SIZE 13
////////////////////*************************类型转换联合体*******************************///////////////////////////
//union SHORT2CHAR
//{
//    short val;
//    char  arr[2];
//};

//union USHORT2CHAR
//{
//    unsigned short val;
//    unsigned char arr[2];
//};
//union FLOAT2CHAR
//{
//    float val;
//    char  arr[4];
//};
//union UINT2CHAR
//{
//    unsigned int val;
//    char  arr[4];
//};
//#if 0
////几种基本的发送数据帧类型
//enum SendEnum
//{
//    SelfCheck=0x01,
//    State
//};



////发送控制面板的Buffer：16Bytes
//struct Str_COMSendBuff
//{
//    unsigned char      m_Header;                       //1 Bytes:帧头
//    enum SendEnum      m_ComponentID;                  //1 Byte:组件标识码
//    unsigned char      m_Avail[COM_AVAILDADA_SIZE];    //13Bytes：有效数据字
//    unsigned char      u_Check;                        //1 Bytes：校验和
//};

////接收结构体
//struct Str_COMRsvBuff
//{
//    unsigned char      m_RsvHeader;
//    unsigned char      m_rsvID;                  //1 Byte:组件标识码
//    unsigned char      m_rData[RSV_AVAILDATA_SIZE];
//    unsigned char      rsv_Check;
//};

///////****************************************发送*************************************///////////////////
///// \brief The H6Protocal class
////自检信息：0自检正常，1故障状态
//enum selfCheckEnum
//{
//    normal,
//    fault
//};
//struct Str_FaultInfo
//{
//    unsigned char visFault:1;
//    unsigned char mirFault:1;
//    unsigned char lirFault:1;
//    unsigned char reserved:5;
//};
//union U_FaultInfo
//{
//    struct Str_FaultInfo bit;
//    unsigned char all;
//};
//struct Str_SelfCheck
//{
//    enum selfCheckEnum check;
//    union U_FaultInfo fault;
//};

//enum workMode
//{
//    idle,
//    track,
//    trackLoss,
//    imgFreeze
//};
//enum imgTypeEnum
//{
//    visible,
//    mir,
//    lir
//};
//struct Str_State
//{
//    enum workMode mode;
//    unsigned short pitOffset;
//    unsigned short aziOffset;
//    unsigned short diffLineX;
//    unsigned short diffLineY;
//    enum imgTypeEnum imgType;
//};

///////****************************************接收*************************************///////////////////
//struct Str_ManualBack
//{
//    unsigned short tarPosX;
//    unsigned short tarPosY;
//    unsigned short tarWidth;
//    unsigned short tarHeight;
//};
//enum ImgFreezeEnumBack
//{
//    freeze,
//    unFreeze
//};
//struct Str_TrackingBoxBack
//{
//    unsigned short boxAzi;
//    unsigned short boxPit;
//};
//enum CrossDisplayBack
//{
//    Display,
//    Blank
//};

//struct Str_rsvFeedBack
//{

//    struct Str_ManualBack r_manualBack;
//    enum ImgFreezeEnumBack r_Freeze;
//    struct Str_TrackingBoxBack r_trackBox;
//    enum imgTypeEnum r_imgDisplay;
//    enum CrossDisplayBack r_crossDis;
//    struct Str_CrossCorrectBack r_diffLine;
//};

//#endif RMPROTOCAL


//#define COM_AVAILDADA_SIZE 61
//#define SENDBUFFER_SIZE 64
//#define UDP_AVAILDADA_SIZE 389
//#define SENDBUFFER_SIZE_UDP 392
////////////////////*************************类型转换联合体*******************************///////////////////////////

//union INT2CHAR
//{
//    unsigned int val;
//    char arr[4];
//};

////几种基本的发送数据帧类型
//enum SendEnum
//{
//    SelfCheck=0x01,
//    StateSend,
//};

//enum SendEnum_DC
//{
//    StateSend_DC=0x01,
//};

//enum ImgFreezeEnumBack
//{
//    freeze,
//    unFreeze
//};
//#ifdef PROTOCAL6K
////几种基本的接收数据帧类型
////enum RecvEnum
////{
////    SelfCheckBack0 = 0x01,
////    manualTrackBack,
////    autoTrackBack,
////    cancelTrackBack,
////    imgFrezBack,
////    CorrectBack,
////    imgDisplayBack,
////    crossDisplayBack,
////    crossCorrBack
////};
//#else
////几种基本的接收数据帧类型
//enum RecvEnum
//{
//    SelfCheckBack0 = 0x01,
//    idleBack,
//    manualTrackBack,
//    tarDetectBack,
//    videoStabilityBack,
//    specifyTrackBack,
//    idTrackBack,
//    cancelTrackBack,
//    chaBlankBack,
//    videoDisplayBack,
//    videoSavedBack,
//    gateSetBack,
//    videoCompressBack,
//    infoDisplayBack
//};
//#endif
//struct Str_CrossCorrectBack
//{
//    unsigned short diffLineX;
//    unsigned short diffLineY;
//};
////发送控制面板的Buffer：16Bytes
//struct Str_COMSendBuff
//{
//    unsigned char      m_Header;                       //1 Bytes:帧头
//    enum SendEnum      m_ComponentID;                  //1 Byte:组件标识码
//    unsigned char      m_Avail[COM_AVAILDADA_SIZE];    //61Bytes：有效数据字
//    unsigned char      u_Check;                        //1 Bytes：校验和
//};

////发送控制面板的Buffer：16Bytes
//struct Str_UDPSendBuff
//{
//    unsigned char      m_Header;                       //1 Bytes:帧头
//    enum SendEnum_DC      m_ComponentID;                  //1 Byte:组件标识码
//    unsigned char      m_Avail[UDP_AVAILDADA_SIZE];    //61Bytes：有效数据字
//    unsigned char      u_Check;                        //1 Bytes：校验和
//};



///////****************************************发送*************************************///////////////////
///// \brief The H6Protocal class
////自检信息：0自检正常，1故障状态
//enum selfCheckEnum
//{
//    normal,
//    fault
//};
//struct Str_FaultInfo
//{
//    unsigned char visInputFault:1;
//    unsigned char irInputFault:1;
//    unsigned char visOutputFault:1;
//    unsigned char irOutputFault:1;
//    unsigned char timeFault:1;
//    unsigned char reserved:3;
//};
//union U_FaultInfo
//{
//    struct Str_FaultInfo bit;
//    unsigned char all;
//};
//struct Str_SelfCheck
//{
//    enum selfCheckEnum check;
//    union U_FaultInfo fault;
//};
////状态信息
//enum workMode
//{
//    idle,
//    track,
//    ObDetect,
//    trackLoss,
//    ImgStability,
//    UnImgStablity,
//    MaintainState,
//    detectPro
//};
//enum imgTypeEnum
//{
//    visible,
//    ir,
//};
//enum videoSavedEnum
//{
//    noSaved,
//    Saving
//};
//struct Str_State
//{
//    enum workMode mode;
//    short pitOffset;
//    short aziOffset;
//    unsigned char TarNum;
//    char TarReserved[40];
//    enum imgTypeEnum videoType;
//    enum videoSavedEnum videoSaveState;
//    unsigned short diffLineX;
//    unsigned short diffLineY;
//    float sharpnessVal;
//};

///////****************************************接收*************************************///////////////////
///// \brief The rmEosProtocal class
///// 手动跟踪
//struct Str_ManualBack
//{
//    unsigned short tarPosX;
//    unsigned short tarPosY;
//    unsigned short tarWidth;
//    unsigned short tarHeight;
//};
////目标检测
//enum tarDetecteEnumBack
//{
//    stopDetect = 0x01,
//    startDetect = 0x02,
//    DetectPro = 0x03
//};
////图像稳像
//enum videoStableEnumBack
//{
//    startStable,
//    stopStable
//};
////字符消隐
//struct Str_ChaBlankInfo
//{
//    unsigned char allCha:1;
//    unsigned char ScaleBar:1;
//    unsigned char gps:1;
//    unsigned char trackBox:1;
//    unsigned char targetBox:1;
//    unsigned char rectBox:1;
//    unsigned char reserved:2;
//};
//union U_ChaBlank
//{
//    struct Str_ChaBlankInfo bit;
//    unsigned char all;
//};
////显示视频
//enum videoDisplayEnum
//{
//    visibleDis = 0x01,
//    irDis = 0x02,
//    mwirDis = 0x03,
//    none
//};
////视频保存
//enum videoSavedBackEnum
//{
//    defualtSave,
//    saveTwo,
//    stopSave
//};
////波门调整
//struct Str_GateAdjst
//{
//    unsigned short gateWidth;
//    unsigned short gateHeight;
//};

//enum LongDirectionEnum
//{
//    east,
//    Weat
//};
//enum LatitudeDirctEnum
//{
//    north,
//    South
//};
////显示信息
//struct Str_InfoDisplay
//{
//    unsigned short year;
//    unsigned char mouth;
//    unsigned char day;
//    unsigned char hour;
//    unsigned char minute;
//    unsigned char second;
//    enum LongDirectionEnum longDirct;
//    int longitude;
//    enum LatitudeDirctEnum latitudDirct;
//    int latitude;
//    float visFov;
//    float irFov;
//    float pitch;
//    float azi;
//    float pitAngVelo;
//    float aziAngVelo;
//    int lasDistance;
//};
//enum CrossDisplayBack
//{
//    Display,
//    Blank
//};
//struct Str_TrackingBoxBack
//{
//    unsigned short boxAzi;
//    unsigned short boxPit;
//};
//struct Str_rsvFeedBack
//{
//    struct Str_ManualBack r_manualBack;
//    enum tarDetecteEnumBack r_TarDetectBack;
//    enum videoStableEnumBack r_VideoStableBack;
//    unsigned char r_AssignedTargetBack;
//    unsigned char r_IDTargetBack;
//    union U_ChaBlank r_chaBlank;
//    enum videoDisplayEnum r_videoDisplay;
//    enum videoSavedBackEnum r_videoSaved;
//    struct Str_GateAdjst r_gateAdjst;
//    unsigned char r_videoComprss;
//    struct Str_InfoDisplay r_infoDisplay;

//    enum ImgFreezeEnumBack r_Freeze;
//    enum CrossDisplayBack r_crossDis;
//    struct Str_CrossCorrectBack r_diffLine;
//    struct Str_TrackingBoxBack r_trackBox;
//    unsigned char crossOrient;
//    unsigned char crossStep;
//    unsigned short crossX;
//    unsigned short crossY;
//    int jumpState=0;
//    int jumpPos=0;
//    int systemMod=0;
//};


//#endif // PROTOCALTYPES_H

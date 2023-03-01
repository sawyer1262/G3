#ifndef __UART_FACE_H_
#define __UART_FACE_H_


#include <stdint.h>




#define       FACE_POWERON_DELAY   500 
#define       FACE_SCAN_INTVAL     500               //人脸扫描
#define       FACE_SCAN_TIMEOUT    2000
#define       ORB_FACE_MAX         100                //最大人脸数量
#define       ORB_FACE_TX_Q_SIZE   10
#define       ORB_TX_FAIL_RTY      3 
#define       FACE_ADD_TIMEOUT_MAX 60000
#define       FACE_VCC_PIN         I2C_SDA


/***********************************测试条件编译选项*****************************************/
//人脸串口测试，
//#define 	UART_FACE_TEST

/***********************************测试条件编译选项*****************************************/
#define        UART_FACE_INDEX      1
#define        UART_FACE_SCI        SCI2

#define        ORB_HEAD_16          0xAEF5
#define        ORB_HEAD_8_H         0xAE
#define        ORB_HEAD_8_L         0xF5

#define        ORB_HEAD_IDX         0
#define        ORB_SRC_IDX         (ORB_HEAD_IDX+2)
#define        ORB_TYPE_IDX        (ORB_SRC_IDX+1)
#define        ORB_LEN_IDX         (ORB_TYPE_IDX+1)
#define        ORB_DAT_IDX         (ORB_LEN_IDX+2)


typedef enum{
		HOST=0x01,
		SLAVE
}ObFrameSrc_t;

typedef enum{
		CMD=0x01,
		DATA,
		QUERY,
		ERR,
		UPDATE=0x05,
		IMG
}ObFrameType_t;

//注册或识别人脸返回的错误表
typedef enum{
		ORBBEC_ERR_NOT_INIT=0x00,                   //未初始化或初始化失败 
		ORBBEC_ERR_LICENSE,                         //未授权  
		ORBBEC_ERR_INTERNAL,                        //SDK内部错误
		ORBBEC_ERR_NO_FACE,                         //未检测到人脸
	
		ORBBEC_ERR_FACE_DISTANCE_NEAR,              //人脸过近
		ORBBEC_ERR_FACE_DISTANCE_FAR,               //人脸过远
		ORBBEC_ERR_FACE_YAW_LEFT,                   //人脸太偏左
		ORBBEC_ERR_FACE_YAW_RIGHT,                  //人脸太偏右
		ORBBEC_ERR_FACE_PITCH_TOP,                  //人脸偏上
		ORBBEC_ERR_FACE_PITCH_BOTTOM,               //人脸偏下
		ORBBEC_ERR_FACE_ROLL_CLOCKWISE,             //人脸顺时针旋转过大
		ORBBEC_ERR_FACE_ROLL_ANTI_CLOCKWISE,        //人脸逆时针旋转过大
		ORBBEC_ERR_FACE_NEAR_LEFT,                  //人脸靠左边沿太近
		ORBBEC_ERR_FACE_NEAR_RIGHT,                 //人脸靠有边沿太近
		ORBBEC_ERR_FACE_NEAR_TOP,                   //人脸靠上边沿太近
		ORBBEC_ERR_FACE_NEAR_BOTTOM,                //人脸靠下边沿太近
	
		ORBBEC_ERR_FACE_LIVE=0x11,                       //非活体人脸
		ORBBEC_ERR_DB_FULL=0x13,                        //数据库已满
		ORBBEC_ERR_NOT_IN_DB=0x14,                      //未在数据库中
		ORBBEC_ERR_BAD_ARGUMENTS=0x15,                   //传入参数不正确
		ORBBEC_ERR_FACE_ID_EXIST,                   //待注册人脸ID已存在
		ORBBEC_ERR_FACE_DEPTH_QUALITY=0x18,              //人脸深度不合适
		ORBBEC_ERR_FACE_OCCLUSION=0x1A,                  //人脸有遮挡
		//0x20-0x24,调整合适位置
		READSENSE_ERR_NOFACE=0xF3                      //详见协议文档
}FaceRegisterErrCode_t;

typedef struct{
		uint8_t Ack;
		uint8_t Cmd;
		uint8_t Stat;
		uint8_t ErrC;
}OrbFaceAckStr_t;


typedef enum{
		FACE_IDLE=0,                    //空闲
		FACE_CHECK_SENSOR,              //验证传感器
		FACE_IDY,                       //识别人脸
}FaceIdyStat_t;

typedef enum{
		FACEADD_READUSER=0,             //等待应答
		FACEADD_READY,
    FACEADD_RUN,
		FACEADD_OK,
		FACEADD_FAIL,
		FACEADD_TIMEOUT
}FaceAddStat_t;

typedef enum{
		FACEDEL_READUSER=0,             //等待应答
    FACEDEL_RUN,
		FACEDEL_OK,
		FACEDEL_FAIL,
		FACEDEL_TIMEOUT
}FaceDelStat_t;

typedef enum{
		WAIT_RESP=0,                    //等待应答
		OP_READY,
		OP_SUCCESS,                     //添加成功
		OP_FULL,                        //已满
		OP_FAIL,                        //添加失败
		OP_TIMEOUT                      //超时无应答       
}FaceAck_t;

typedef union{
		uint8_t IdyReady;
		uint8_t AddReady;
		uint8_t DelReady;
		uint8_t Ready;
}FaceReady_t;

typedef struct{
		FaceIdyStat_t FsStat;
		uint8_t IdyRetry;
		uint32_t StTime;
}FaceIdy_t;

typedef struct{
		FaceAddStat_t AddStat;
		uint8_t Retry;
		uint32_t StTime;
		uint32_t VolFristTime;
}FaceAdd_t;

typedef struct{
		FaceDelStat_t DelStat;
		uint8_t Retry;
		uint32_t StTime;
}FaceDel_t;


typedef struct{
		uint8_t Sum;                     //有效人脸总数
		uint8_t IndexTab[13];            //人脸数据存储阵列
}FaceUserStr_t;


/************************************************************************************************************/
/*
串口基本参数：

波特率 数据位 奇偶校验 停止位  数据流控
115200   8       无      1      无


一：查询设备信息相关指令

包类型：ORB_QUERY

数据1决定包内容：

0x55-----算法板准备完成，设备上电就绪主动上报（无需查询）：用于确定何时向模块发送操作请求
0x57-----上报注册过程中的中间状态：一共5帧数据，前4帧0x57加帧序号，第5帧0x01加成功失败标志：可用于增强交互体验
0x50-----查询当前设备所有ID，返回所有存储的人脸ID

//下列操作不会使用
0x56-----上报串口升级状态，1S一次（此功能暂时不用）
0x60-----查询系统设备版本号
0x61-----查询算法板软件版本号
0x62-----查询摄像头模组版本号
0x64-----查询chipID
0x65-----查询DeviceKey
0x66-----分配特定大小内存直到死机（测试用）

二：发送设备控制命令

包类型：ORB_CMD

数据1决定命令类型：

0x01-----添加人脸：期间有上报中间状态（不多于10次）和结果1次
0x02-----删除人脸：后跟人脸ID，FF为删除所有人脸
0x40-----识别人脸：后跟可设的识别次数，次数范围1-5，默认5次


0x51-----演示识别：识别成功返回FF
0x70-----设置WIFI信息（具有wifi模块的才具有此功能）：本产品不具有此模块
0x71-----控制WIFI电源（具有wifi模块的才具有此功能）：本产品不具有此模块
0x73-----抓取一张JPEG压缩后的抓拍图片
0x77-----告知主机图片（抓拍的图片）文件的信息：此处协议上表述有问题，待跟厂家沟通


三：设备上电初始化错误代码

bit 0：代表 3D 模块 I2C 通信异常；
bit 1：代表算法 license 异常；
bit 2：代表算法 weight 异常；
bit 3：DB error -1，人脸特征数据库错误，无法初始化 SDK；6.9.0 之后的版本支持。
bit 4：代表人脸算法的其他异常；

所以可能的异常是:					
0x1 ： 只有 3D 模组异常
0x3 ： license 异常并同时导致了 3D 模组的异常
0x5 ： weight 异常并同时导致了 3D 模组的异常
0x8 ：人脸特征数据库异常，可以单独出现
0x9： 人脸数据库 DB 异常并同时导致了 3D 模组的异常


docklight测试数据如下：

//添加人脸
2021/12/3 11:26:42.053 [TX] - F5 AE 01 01 02 00 01 06 52 FE 
2021/12/3 11:26:42.543 [RX] - F5 AE 02 01 03 00 57 01 03 FC FD         //第一帧结果
                              F5 AE 02 01 03 00 57 02 03 FB FD         //第二帧结果   
															F5 AE 02 01 03 00 57 03 03 FA FD         //第三帧结果
															F5 AE 02 01 03 00 57 04 03 F9 FD         //第四帧结果
															F5 AE 02 01 03 00 57 05 07 F4 FD         //第五帧结果
															F5 AE 02 01 03 00 57 06 08 F2 FD         //第六帧结果
															F5 AE 02 01 03 00 01 01 06 4F FE 
//中间状态帧结果可能有1-10条，可以忽略中间状态，只处理结果

//识别人脸
2021/12/3 11:27:44.384 [TX] - F5 AE 01 01 02 00 40 05 14 FE 
2021/12/3 11:27:45.709 [RX] - F5 AE 02 01 03 00 40 01 06 10 FE 
//删除人脸
2021/12/3 13:32:13.533 [TX] - F5 AE 01 01 02 00 02 05 52 FE 
2021/12/3 13:32:13.655 [RX] - F5 AE 02 01 02 00 02 01 55 FE 
//读取所有ID
2021/12/3 13:32:19.926 [TX] - F5 AE 01 03 01 00 50 08 FE 
2021/12/3 13:32:20.024 [RX] - F5 AE 02 03 06 00 50 01 02 03 04 06 F2 FD 
//查询硬件版本
2021/12/3 13:35:18.675 [TX] - F5 AE 01 03 01 00 60 F8 FD 
2021/12/3 13:35:18.746 [RX] - F5 AE 02 03 04 00 60 02 00 00 F2 FD 
//查询软件版本
2021/12/3 13:35:47.938 [TX] - F5 AE 01 03 01 00 61 F7 FD 
2021/12/3 13:35:48.008 [RX] - F5 AE 02 03 04 00 61 07 00 18 D4 FD 
//查询命令状态，此指令无应答，可能在特殊场合才使用
2021/12/3 13:35:44.076 [TX] - F5 AE 01 03 01 00 01 57 FE 
//查询摄像头版本
2021/12/3 13:37:55.415 [TX] - F5 AE 01 03 01 00 62 F6 FD 
2021/12/3 13:37:55.474 [RX] - F5 AE 02 03 04 00 62 02 07 05 E4 FD 
//查询芯片ID
2021/12/3 13:38:13.660 [TX] - F5 AE 01 03 01 00 64 F4 FD 
2021/12/3 13:38:13.752 [RX] - F5 AE 02 03 09 00 64 31 2D 0F 35 38 32 35 32 78 FC 
//抓拍图片，图片为红外图，此功能暂时不用
2021/12/3 13:49:47.846 [TX] - F5 AE 01 01 02 00 73 00 E6 FD 
2021/12/3 13:49:48.501 [RX] - F5 AE 02 01 06 00 77 B3 23 00 D0 FA 3D FB   //应答，图片大小与校验
															F5 AE 02 04 00 04 FF D8 FF E0 ......        //省略号后为抓拍的图片数据，		
//注意其数据类型为04，可能不对，如需使用该功能，应与奥比核对
*/                              
/************************************************************************************************************/


extern FaceAdd_t FaceAdd;
extern FaceIdy_t FaceIdy;
extern FaceDel_t FaceDel;
extern OrbFaceAckStr_t OrdFace;
extern FaceReady_t FaceReady;
extern FaceUserStr_t FaceUser;


void UartFace_DatStrInit(void);
void UartFace_PowerOn(void);
void UartFace_PowerOff(void);
void UartFace_PowerReset(void);
void UartFace_Init(void);
void UartFace_Deinit(void);
void UartFace_RtxTimeoutoutHandle(void);
void UartFace_SendNbytes(uint8_t* buf,uint16_t len);
void UartFace_RxDataHandle(void);
void UartFace_RtxTimeoutoutHandle(void); 
void UartFace_DataSendHandle(void);
void UartFace_TxQueueReset(void);

void UartFace_SendCmdData(ObFrameType_t type,uint8_t *buf,uint8_t len,uint8_t ack,uint32_t timeout);



void OrbAddFace(uint8_t id);
void OrbIdfyFace(void);
void OrbReadFaceID(void);
void OrbFaceScanHandle(void);

uint8_t GetOrbFaceFreeSlot(void);
uint8_t FaceAddHandle(uint8_t type);
uint8_t FaceDelHandle(uint8_t id);
void OrbFaceUpdateUser(uint8_t type,uint8_t id);

#endif


#ifndef  __UART_FP_H_
#define  __UART_FP_H_



#include <stdint.h>


#define        FP_MAX_NUM              100
#define        FP_POWERON_DELAY        100       //
#define        FP_ACK_FLAG             0x80
#define        FP_STEP_RTY_MAX         3

#define        FP_TX_Q_SIZE            5
#define        FP_REG_PRESS_TIMES      6         //注册时按压手指次数

#define        FP_ADD_TIMEOUT_MAX      60000
#define        FP_LEAVE_WAIT_TIME      5000


//指纹模组MCU供电电源
#define        UART_FP_INDEX          2
#define        UART_FP_SCI            SCI3

#define        FP_MCU_VCC_PIN      I2C_SDA                //指纹MCU电压开关
#define        FP_TOUCHOUT_PIN     EPORT_PIN30            //指纹触摸唤醒引脚，检测到指纹为高电平



#define  	FP_SUCCESS            0x00           //添加成功  
#define	  FP_FAIL               0x01           //添加失败


//指纹模组传感器供电暂时是一直供电


//包头
#define        FP_HEAD              0xEF01
#define        FP_HEAD_H            0xEF
#define        FP_HEAD_L            0x01

//包表示
typedef enum{
		FP_CMD=0x01,                    // 指令包,主机发送的指令 
		FP_DAT_CONTINUED,               // 数据包还有后续数据包 
		FP_ACK=0x07,                    // 应答包
		FP_DAT_END,                     // 最后一个数据包 
}FpPktType_t;

//指纹模块系统信息
typedef struct{
		uint16_t SysStat;                //系统状态寄存器值
		uint16_t SenType;                //传感器类型
		uint16_t MaxFpSum;               //指纹最大容量
		uint16_t ActFpSum;               //有效指纹模板总数
		uint16_t SecLevel;               //安全等级
		uint16_t PktSize;                //数据包大小
		uint16_t BaudRate;               //串口波特率
		uint32_t DevAddr;                //设备地址
		uint8_t  IndexTab[13];           //指纹模板索引，由于只有100指纹，一个字节8位，所以100/8+1=13，1表示存有指纹，0表示没有  
}LdFpStr_t;


//指纹模块工作状态机
typedef enum{
		FP_IDLE=0,                //空闲
		FP_CHECK_SENSOR,          //检测传感器是否正常
		FP_READ_IDXTABLE,         //读指纹索引表
		FP_GET_IMG,               //获取图像
		FP_GEN_CHAR,              //生成特征值
		FP_SEARCH_LIB,            //搜索指纹库 
		FP_POWER_DOWN             //指纹头休眠
}LdFpStat_t;


typedef enum{
		PD_START=0,                //power down start
		PD_WAIT_PD,                //等待指纹掉电
		PD_WAIT_WK                 //休眠等待唤醒
}LdFpPdStat_t;

typedef struct{
		LdFpStat_t LdFpStat;
		uint8_t ComRetry;
		uint32_t StTime;
}FpScan_t;


typedef enum{
		FP_ADD_READ_USER=0,       //读取用户数 
		FP_ADD_GET_IMG,           //获取图像
		FP_ADD_GEN_CHAR,          //生成特征
		FP_ADD_REG_MODE,          //生成模板
		FP_ADD_STORE_MODE,        //存储模板
}LdFpAddStat_t;


typedef struct{
		LdFpAddStat_t LdFpAddStat;      //指纹添加状态机
		uint8_t AlreadGetIMG;           //成功获取图像标志
		uint8_t GenCharCnt;             //生成特征数量
		uint8_t ComRetry;               //通讯失败次数
		uint32_t VolStime;              //语音提示时间 
		uint32_t StTime;                //命令发送时间
}FpAdd_t;

typedef enum{
		FP_DEL_READ_USER=0,       //读取用户数 
		FP_DEL_RUN                //执行删除
}LdFpDelStat_t;

typedef struct{
		LdFpDelStat_t LdFpDelStat;
		uint8_t ComRetry;
		uint32_t StTime;
}FpDel_t;

typedef struct{
		uint32_t Stime;
}FpPd_t;

//帧结构
#define    FP_HEAD_IDX        0
#define    FP_ADDR_IDX        (FP_HEAD_IDX+2)
#define    FP_TYPE_IDX        (FP_ADDR_IDX+4)
#define    FP_LEN_IDX         (FP_TYPE_IDX+1)


//////////////////错误返回码////////////////////
#define PS_OK                	0x00         //OK
#define PS_COMM_ERR          	0x01         //数据包接收错误
#define PS_NO_FINGER         	0x02         //传感器上没有手指
#define PS_GET_IMG_ERR       	0x03         //录入指纹图像失败
#define PS_FP_TOO_DRY        	0x04         //表示指纹图像太干、太淡而生不成特征
#define PS_FP_TOO_WET       	0x05         //表示指纹图像太湿、太糊而生不成特征
#define PS_FP_DISORDER       	0x06         //表示指纹图像太乱而生不成特征
#define PS_LITTLE_FEATURE    	0x07         //表示指纹图像正常，但特征点太少（或面积太小）而生不成特征
#define PS_NOT_MATCH         	0x08         //表示指纹不匹配 
#define PS_NOT_SEARCHED      	0x09         //表示没搜索到指纹
#define PS_MERGE_ERR         	0x0a         //表示特征合并失败
#define PS_ADDRESS_OVER      	0x0b         //表示访问指纹库时地址序号超出指纹库范围
#define PS_READ_ERR          	0x0c         //表示从指纹库读模板出错或无效
#define PS_UP_TEMP_ERR       	0x0d         //表示上传特征失败
#define PS_RECV_ERR          	0x0e         //表示模块不能接收后续数据包
#define PS_UP_IMG_ERR        	0x0f         //表示上传图像失败  
#define PS_DEL_TEMP_ERR      	0x10         //表示删除模板失败 
#define PS_CLEAR_TEMP_ERR    	0x11         //表示清空指纹库失败
#define PS_SLEEP_ERR         	0x12         //表示不能进入低功耗状态
#define PS_INVALID_PASSWORD  	0x13         //表示口令不正确
#define PS_RESET_ERR         	0x14         //表示系统复位失败 
#define PS_INVALID_IMAGE     	0x15         //表示缓冲区内没有有效原始图而生不成图像
#define PS_UPDATE_ERROR     	0x16         //表示在线升级失败
#define PS_HANGOVER_UNREMOVE 	0X17         //表示残留指纹或两次采集之间手指没有移动过
#define PS_FLASH_ERR         	0x18         //表示读写 FLASH 出错 
#define PS_TRNG_ERR         	0x19         //随机数生成失败
#define PS_INVALID_REG       	0x1a         //无效寄存器号 
#define PS_REG_CONTENT_ERR    0x1b         //寄存器设定内容错误号
#define PS_NOTEADDR_ERR      	0x1c         //记事本页码指定错误 
#define PS_PORT_ERR      		  0x1d         //端口操作失败
#define PS_ENROLL_ERR        	0x1e         //自动注册（enroll）失败
#define PS_LIB_FULL_ERR      	0x1f         //指纹库满
#define PS_DEVICE_ADDR_ERR   	0x20         //设备地址错误
#define PS_MUST_VERIFY_PWD   	0x21         //密码有误 
#define PS_TMPL_NOT_EMPTY    	0x22         //指纹模板非空
#define PS_TMPL_EMPTY    	 	  0x23         //指纹模板为空
#define PS_LIB_EMPTY_ERR      0x24         //指纹库为空
#define PS_TMPL_NUM_ERR		 	  0x25         //录入次数设置错误
#define PS_TIME_OUT			 	    0x26         //超时
#define PS_FP_DUPLICATION	 	  0x27         //指纹已存在 
#define PS_RELATE         		0x28         //指纹模板有关联
#define PS_SENSOR_ERR         0x29         //传感器初始化失败


//通用指令
#define PS_GetImage				    0x01         //验证用获取图像	
#define PS_GenChar            0x02         //生成特征
#define PS_Match              0x03         //精确比对两枚指纹特征
#define PS_Search             0x04         //搜索指纹
#define PS_RegModel           0x05         //合并特征（生成模板） 
#define PS_StoreChar          0x06         //储存模板
#define PS_LoadChar           0x07         //读出模板
#define PS_UpChar             0x08         //上传特征或模板
#define PS_DownChar           0x09         //下载特征或模板
#define PS_UpImage				    0x0a         //上传图像
#define PS_DownImage          0x0b         //下载图像
#define PS_DeletChar          0x0c         //删除模板
#define PS_Empty              0x0d         //清空指纹库 
#define PS_WriteReg           0x0e         //写系统寄存器
#define PS_ReadSysPara        0x0f         //读系统基本参数
#define PS_SetPwd             0x12         //设置口令
#define PS_VfyPwd							0x13         //验证口令
#define PS_GetRandomCode      0x14         //采样随机数
#define PS_SetChipAddr        0x15         //设置芯片地址
#define PS_ReadINFpage				0x16         //读 flash 信息页
#define PS_Port_Contro        0x17         //端口控制
#define PS_WriteNotepad       0x18         //写记事本
#define PS_ReadNotepad        0x19         //读记事本
#define PS_BurnCode				    0x1a         //烧写片内 FLASH
#define PS_GenBinImage        0x1c         //生成细化指纹图像
#define PS_ValidTempleteNum	  0x1d         //读有效模板个数
#define PS_ReadIndexTable     0x1f         //读取模版索引表 
#define PS_GetEnrollImage     0x29         //注册用获取图像
#define PS_AutoEnroll         0x31         //自动注册模板
#define PS_AutoIdentify       0x32         //自动验证指纹
#define PS_Sleep              0x33         //休眠指令
#define PS_ReadUUID           0x34         //读芯片唯一序列号
#define PS_HandShake		      0x35         //握手指令




/***********************************************************************/
/*
//指纹模块默认地址FFFFFFFF，波特率57600

波特率 数据位 奇偶校验 停止位  数据流控
57600    8       无      1       无


**************************************注意**********************************************
注意事项：指纹传感器中断信号TOUCH-OUT:只有在休眠指令后才会触发（低到高），触发后一直为高
不论手指是否离开，要再次触发必须发休眠指令，所以正确的处理方式就是：先发休眠指令，收到中
断信号就开始处理，处理完后休眠传感器。主要用于验证指纹，注册指纹时可以用来检测手指是否按
下，然后采集图像。
**************************************注意**********************************************


主要功能：

1.添加指纹
2.删除指纹
3.验证指纹

//测试数据
2021/12/16 09:12:23.585 [RX] - 55                //上电握手                                                                  
2021/12/16 09:15:04.397 [TX] - EF 01 FF FF FF FF 01 00 03 0F 00 13        //读系统信息 
2021/12/16 09:15:04.413 [RX] - EF 01 FF FF FF FF 07 00 13 00 00 0C 2E C8 00 64 00 03 FF FF FF FF 00 02 00 06 05 87 

			00 0C                    //状态寄存器
			2E C8                    //传感器类型
			00 64                    //指纹库容量：序号从0-99
			00 03                    //安全等级
			FF FF FF FF              //设备地址 
			00 02                    //数据包大小：0-32,1-64,2-128,3-258
			00 06                    //波特率：9600*N=9600*6=57600

//自动注册：自动注册后续不用
1.>2次按压，注册成功
2021/12/16 10:11:09.817 [TX] - EF 01 FF FF FF FF 01 00 08 31 00 01 02 00 4B 00 88    //
2021/12/16 10:11:09.830 [RX] - EF 01 FF FF FF FF 07 00 05 00 00 00 00 0C             //指令正确
2021/12/16 10:11:11.488 [RX] - EF 01 FF FF FF FF 07 00 05 00 01 01 00 0E             //第一次采图成功
2021/12/16 10:11:11.775 [RX] - EF 01 FF FF FF FF 07 00 05 00 02 01 00 0F             //第一次生成特征成功
2021/12/16 10:11:12.846 [RX] - EF 01 FF FF FF FF 07 00 05 00 03 01 00 10             //手指离开，第一次录入成功 
2021/12/16 10:11:13.964 [RX] - EF 01 FF FF FF FF 07 00 05 00 01 02 00 0F             //第二次采图成功 
2021/12/16 10:11:14.268 [RX] - EF 01 FF FF FF FF 07 00 05 00 02 02 00 10             //第二次生成特征成功
															 EF 01 FF FF FF FF 07 00 05 00 04 F0 01 00             //合成模板
															 EF 01 FF FF FF FF 07 00 05 00 05 F1 01 02             //已注册检测
															 EF 01 FF FF FF FF 07 00 05 00 06 F2 01 04             //模板存储结果
															 
2.->注册超时：8S超时
2021/12/16 10:18:37.927 [RX] - EF 01 FF FF FF FF 07 00 05 00 00 00 00 0C 
2021/12/16 10:18:45.860 [RX] - EF 01 FF FF FF FF 07 00 05 26 01 01 00 34 


//读有效模板个数
2021/12/16 11:01:20.793 [TX] - EF 01 FF FF FF FF 01 00 03 1D 00 21 
2021/12/16 11:01:20.805 [RX] - EF 01 FF FF FF FF 07 00 05 00 00 01 00 0D 




//添加指纹流程：获取图像生成特征次数多一点，6-8次。不需要精确比对：特征文件缓存有多个（资料上写4个，未更新）
//录指纹时手指按2次，每次采4次图像，一共存8个特征值到特征文件缓存，对应的PS_GenChar命令的BufferID为1-8，然后
//通过8个特征生成指纹数据，存入指纹库。

2021/12/16 14:55:16.131 [TX] - EF 01 FF FF FF FF 01 00 03 01 00 05                        //第一次获取图像
2021/12/16 14:55:16.315 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A                        //返回成功标志 
2021/12/16 14:55:19.023 [TX] - EF 01 FF FF FF FF 01 00 04 02 01 00 08                     //第一次图像生成特征
2021/12/16 14:55:19.319 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A                        //返回成功标志
2021/12/16 14:55:25.948 [TX] - EF 01 FF FF FF FF 01 00 03 01 00 05                        //第二次获取图像
2021/12/16 14:55:26.141 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A                        //返回成功标志   
2021/12/16 14:55:28.411 [TX] - EF 01 FF FF FF FF 01 00 04 02 02 00 09                     //第二次图像生成特征
2021/12/16 14:55:28.713 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A                        //返回成功标志
//2021/12/16 14:55:31.161 [TX] - EF 01 FF FF FF FF 01 00 03 03 00 07                        //精确对比两枚指纹特征：模块厂家说不要精确对比指纹特征
//2021/12/16 14:55:31.189 [RX] - EF 01 FF FF FF FF 07 00 05 00 01 13 00 20                  //返回对比结果
2021/12/16 14:56:01.662 [TX] - EF 01 FF FF FF FF 01 00 03 05 00 09                        //合并两次特征
2021/12/16 14:56:01.674 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A                        //返回结果
2021/12/16 14:56:07.061 [TX] - EF 01 FF FF FF FF 01 00 06 06 01 00 00 00 0E               //存储指纹数据
2021/12/16 14:56:07.119 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A                        //返回结果

//刷指纹流程

2021/12/16 15:33:25.430 [TX] - EF 01 FF FF FF FF 01 00 03 01 00 05                        //获取图像
2021/12/16 15:33:25.620 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A                        //返回成功标志
2021/12/16 15:33:34.574 [TX] - EF 01 FF FF FF FF 01 00 04 02 01 00 08                     //生成指纹特征 
2021/12/16 15:33:34.869 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A                        //生成指纹特征应答 
2021/12/16 15:33:36.835 [TX] - EF 01 FF FF FF FF 01 00 08 04 01 00 01 00 10 00 1F         //搜索指纹库
2021/12/16 15:33:36.882 [RX] - EF 01 FF FF FF FF 07 00 07 00 00 01 00 50 00 5F            //返回搜索结果

//删除指纹

2021/12/16 11:08:08.695 [TX] - EF 01 FF FF FF FF 01 00 07 0C 00 01 00 01 00 16 
2021/12/16 11:08:08.761 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A 

//清空指纹库

2021/12/16 11:10:06.391 [TX] - EF 01 FF FF FF FF 01 00 03 0D 00 11 
2021/12/16 11:10:06.461 [RX] - EF 01 FF FF FF FF 07 00 03 00 00 0A 

*/
/***********************************************************************/





extern FpScan_t FpScan;
extern FpAdd_t FpAdd;
extern FpDel_t FpDel;
extern LdFpStr_t LdFp;
extern uint8_t FpPowerDown;

void UartFp_DatStrInit(void);
void UartFp_Init(void);
void UartFp_Deinit(void);
void UartFp_RtxTimeoutoutHandle(void);
void UartFp_SendNbytes(uint8_t* buf,uint16_t len);
void UartFp_RxDataHandle(void);
void UartFp_RtxTimeoutoutHandle(void); 
void UartFp_DataSendHandle(void);


void UartFp_SendCmdData(FpPktType_t type,uint8_t *buf,uint16_t len,uint8_t ack,uint32_t timeout);

void FpMcuPowerIoInit(void);
void FpMcuPowerOn(void);
void FpMcuPowerOff(void);


void LdFpStatHandle(void);
uint8_t Fp_GetValidFpSum(void);

uint8_t Fp_GetFpFristFreeSlot(void);
uint8_t FpAddHandle(uint8_t type);
uint8_t FpDelHandle(uint8_t id);
uint8_t Fp_PowerDown(void);
void FpUpdateUser(uint8_t type,uint8_t id,uint8_t isCoerce);


#endif


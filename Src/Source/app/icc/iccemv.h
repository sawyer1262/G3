/*
 * iccemv.h
 *
 *  Created on: 2017年5月26日
 *      Author: YangWenfeng
 */

#ifndef ICCEMV_H_
#define ICCEMV_H_

#include "usi_drv.h"
#include "icc_conf.h"
#include "main.h"

#ifdef BCTC_DEMO
#define USI_PIT
#endif


/*******************************************
             IC卡处理状态
 *******************************************/
#define ICC_SUCCESS                   0
#define ICC_VCCMODEERR            (-2500)       /* 电压模式错误*/
#define ICC_INPUTSLOTERR          (-2501)       /* 选择通道口错误*/
#define ICC_VCCOPENERR            (-2502)       /* */
#define ICC_ICCMESERR             (-2503)       /* 卡通讯失败*/

#define ICC_T0_TIMEOUT            (-2200)       /* 等待卡片响应超时  */
#define ICC_T0_MORESENDERR        (-2201)       /* 重发错误*/
#define ICC_T0_MORERECEERR        (-2202)       /* 重收错误*/
#define ICC_T0_PARERR             (-2203)       /* 字符奇偶错误*/
#define ICC_T0_INVALIDSW          (-2204)       /* 状态字节无效*/

#define ICC_DATA_LENTHERR         (-2400)       /* 数据长度错误  */
#define ICC_PARERR                (-2401)       /* 奇偶错误      */
#define ICC_PARAMETERERR          (-2402)       /* 参数值为空*/
#define ICC_SLOTERR               (-2403)       /* 卡通道错误   */
#define ICC_PROTOCALERR           (-2404)       /* 协议错误     */
#define ICC_CARD_OUT              (-2405)       /* 卡拨出       */
#define ICC_NO_INITERR            (-2406)       /* 没有初始化   */
#define ICC_ICCMESSOVERTIME       (-2407)       /* 卡通讯超时*/

#define ICC_ATR_TSERR             (-2100)       /* 正反向约定错误,TS错误*/
#define ICC_ATR_TCKERR            (-2101)       /* 复位校验(T=1,TCK错误)错误    */
#define ICC_ATR_TIMEOUT           (-2102)       /* 复位等待超时    */
#define ICC_TS_TIMEOUT            (-2115)       /* 字符间隔超时    */
#define ICC_ATR_TA1ERR            (-2103)       /* TA1错误         */
#define ICC_ATR_TA2ERR            (-2104)       /* TA2错误         */
#define ICC_ATR_TA3ERR            (-2105)       /* TA3错误         */
#define ICC_ATR_TB1ERR            (-2106)       /* TB1错误         */
#define ICC_ATR_TB2ERR            (-2107)       /* TB2错误         */
#define ICC_ATR_TB3ERR            (-2108)       /* TB3错误         */
#define ICC_ATR_TC1ERR            (-2109)       /* TC1错误         */
#define ICC_ATR_TC2ERR            (-2110)       /* TC2错误         */
#define ICC_ATR_TC3ERR            (-2111)       /* TC3错误         */
#define ICC_ATR_TD1ERR            (-2112)       /* TD1错误         */
#define ICC_ATR_TD2ERR            (-2113)       /* TD2错误         */
#define ICC_ATR_LENGTHERR         (-2114)       /* ATR数据长度错误  */

#define ICC_T1_BWTERR             (-2300)       /* T=1字组等待时间错误  */
#define ICC_T1_CWTERR             (-2301)       /* T=1字符等待时间错误  */
#define ICC_T1_ABORTERR           (-2302)       /* 异常(ABORT)通信错误 */
#define ICC_T1_EDCERR             (-2303)       /* 字组校验码(EDC)错误 */
#define ICC_T1_SYNCHERR           (-2304)       /* 同步通信错误*/
#define ICC_T1_EGTERR             (-2305)       /* 字符保护时间错误    */
#define ICC_T1_BGTERR             (-2306)       /* 字组保护时间错误    */
#define ICC_T1_NADERR             (-2307)       /* 字组中NAD错误       */
#define ICC_T1_PCBERR             (-2308)       /* 字组PCB错误         */
#define ICC_T1_LENGTHERR          (-2309)       /* 字组LEN错误         */
#define ICC_T1_IFSCERR            (-2310)       /* IFSC错误            */
#define ICC_T1_IFSDERR            (-2311)       /* IFSD错误            */
#define ICC_T1_MOREERR            (-2312)       /* 多次传送错误并放弃  */
#define ICC_T1_PARITYERR          (-2313)       /* 字符奇偶错误*/
#define ICC_T1_INVALIDBLOCK       (-2314)       /* 无效的字组*/



#define   IC_MAXSLOT       1   //define the 7816_x
#define   USER_CARD        0

//#define debug_print
#define ISO7816_AUTO_RETRANSMIT
#define ISO7816_PBOC_CARD	// 7816 PBOC卡,Level1测试需要打开
//#define JTAG_MODE

#define   ICMAXBUF         512

typedef struct
{
	UINT8   	    Command[ 4 ]; // CLA INS  P1 P2
	UINT16  	    Lc;
	UINT8   	    DataIn[512];
	UINT16  	    Le;
} APDU_SEND;

typedef struct
{
	UINT16       LenOut;    // length of dataout
	UINT8 	    DataOut[512];
	UINT8        SWA;
	UINT8        SWB;
} APDU_RESP;


typedef struct
{
	UINT8 open;         /* 0 关闭 1 打开  */
	UINT8 existent;     /* 0 无卡 1 有卡  */
	UINT8 resetstatus;  /* 0-没有复位；1-已冷复位；2-已热复位 */
	UINT8 autoresp;     /* 自动应答设置：0=自动应答  1=非自动应答  */
#ifndef ISO7816_PBOC_CARD
	UINT16 F;
#endif	
	UINT8 D;            /* 修改异步通信波特率变量因子   */
	UINT8 T;            /* 卡片协议类型  值为0或1 */
	UINT8 TS;           /* 卡正、反向约定  3B=正向 3F=反向 */
	UINT8 ATR_len;      /*卡片复位应答长度 */
	UINT8 ATR[40];      /*卡片复位应答内容 */
	UINT8 TA;       /* 低4位有效，对应于TA1、TA2、TA3,TA4的存在   */
	UINT8 TA1;      /* 卡片复位接口字符   */
	UINT8 TA2;      /* 卡片复位接口字符   */
	UINT8 TA3;      /* 卡片复位接口字符   */
	UINT8 TA4;      /* 卡片复位接口字符   */
	UINT8 TB;       /* 低4位有效，对应于TB1、TB2、TB3,TB4的存在   */
	UINT8 TB1;      /* 卡片复位接口字符    */
	UINT8 TB2;      /* 卡片复位接口字符   */
	UINT8 TB3;      /* 卡片复位接口字符   */
	UINT8 TB4;      /* 卡片复位接口字符   */
	UINT8 TC;       /* 低4位有效，对应于TC1、TC2、TC3,TC4的存在   */
	UINT8 TC1;      /* 卡片复位接口字符   */
	UINT8 TC2;      /* 卡片复位接口字符   */
	UINT8 TC3;      /* 卡片复位接口字符   */
	UINT8 TC4;      /* 卡片复位接口字符   */
	UINT8 TD;       /* 低4位有效，对应于TD1、TD2、TD3,TD4的存在   */
	UINT8 TD1;      /* 卡片复位接口字符   */
	UINT8 TD2;      /* 卡片复位接口字符   */
	UINT8 TD3;      /* 卡片复位接口字符   */
	UINT8 IFSC;     /* 卡片允许最大字组信息长度   */
	UINT8 IFSD;     /* 终端允许最大字组信息长度   */
	UINT8 term_pcb; /* T=1协议下终端发送下个字段格式与序列号  */
	UINT8 card_pcb; /* T=1协议下卡发送下个字段格式与序列号    */
	UINT8 sw1;      /* 状态字节1  */
	UINT8 sw2;      /* 状态字节2  */
}ASYNCARD_INFO;


extern volatile UINT32          k_Ic_TotalTimeCount;         //字符组总超时计数中间变量
extern volatile UINT32          k_Ic_TimeCount;              //字符间隔超时计数中间变量
extern volatile UINT32          k_IcMax_TotalTimeCount;      //字组总超时上限值
extern volatile UINT32          k_IcMax_TimeCount;           //字符间隔超时上限值
extern volatile int             k_timeover,k_total_timeover; //字符超时，总超时标志
extern volatile UINT32          k_IcMax_EtuCount;           //字符间隔超时上限值，单位ETU

//static   UINT8                  card_Outputdata[ICMAXBUF];
//static   UINT8                  rstbuf[40];
volatile static UINT8           WTX;
volatile static UINT32          WWT,CWT,BWT;
extern volatile UINT8           current_slot;
extern volatile  UINT8          Parity_Check_Enable; /* 偶校验错误单个字符重发允许标志  */
extern volatile  UINT8          k_IccErrPar;       /* 奇偶校验    */
extern ASYNCARD_INFO            asyncard_info[IC_MAXSLOT];
extern SC_ATR                   SC_A2R;
extern SC_ADPU_Commands         SC_ADPU;
extern SC_ADPU_Responce         SC_Responce;
extern SC_State                 *locState;
extern SC_READER_SET            SC_Slot_Default;
extern SC_READER_SET            *SC_Slot_Active;
extern volatile UINT8           SC_ATR_Table[40];
extern volatile UINT8           SC_ATR_Len;
extern volatile UINT8           guc_hotrstFlag;
extern volatile UINT8           guc_Leflag;
extern volatile UINT8           SC_T0_T1_Status;
extern volatile UINT8           changeTa1;          //20110729
extern volatile UINT8           guc_AdjustETU;		//发送字符间的ETU需要加1减1来调整
extern volatile UINT8           guc_GuardTime;		//20131220，必须为int类型(与原值类型匹配)，否则配置完GT寄存器为默认12ETU
extern volatile UINT16          respLength;
extern volatile UINT8           guc_Class;
extern volatile UINT8           ICCardResetOK;      //表示IC卡复位成功
extern volatile UINT8           ICCardInsert;       //表示卡已经插入
extern volatile UINT8           ICCardInfoSent;      //ic卡插拔信息发送：00 初始状态   01 卡插入   02 卡移除
extern volatile UINT8           g_usbCommOK;

//icc_hardware.c

/*******************************************************************************
* Function Name  : delayETU
* Description    : Delay a ETU
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void delay1ETU(void);

/*******************************************************************************
* Function Name  : DelayNETU
* Description    : Delay N ETUs
* Input          : count: Delay Parame
* Output         : None
* Return         : None
*******************************************************************************/
void DelayNETU(UINT16 count);

/*******************************************************************************
* Function Name  : SC_VoltageConfig
* Description    : Configures the card power voltage.
* Input          : - SC_Voltage: specifies the card power voltage.
*                    This parameter can be one of the following values:
*                        - MB525 SC_Voltage_5V: 5V cards.
*                        - MB525 SC_Voltage_3V: 3V cards.
*                        - NCN6001 0x01 1.8V
*                        - NCN6001 0x02 3V
*                        - NCN6001 0x03 5V
* Output         : None
* Return         : None
*******************************************************************************/
void SC_VoltageConfig(UINT32 SC_Voltage);

/*******************************************************************************
* Function Name  : Open_ICCard_Vcc_3V
* Description    : open ic card vcc = 5V
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Open_ICCard_Vcc_5V(void);

/*******************************************************************************
* Function Name  : Open_ICCard_Vcc_3V
* Description    : open ic card vcc = 3V
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Open_ICCard_Vcc_3V(void);

/*******************************************************************************
* Function Name  : Open_ICCard_Vcc_1V8
* Description    : open ic card vcc = 1V8
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Open_ICCard_Vcc_1V8(void);

/*******************************************************************************
* Function Name  : Close_ICCard_Vcc
* Description    : close ic card vcc
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Close_ICCard_Vcc(void);

/*******************************************************************************
* Function Name  : Open_ICCard_VCC
* Description    : open ic card vcc
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Open_ICCard_VCC(void);

/*******************************************************************************
* Function Name  : SC_Init
* Description    : Initializes all peripheral used for Smartcard interface.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SC_Init(void);

/*******************************************************************************
* Function Name  : SC_HotInit
* Description    : Smart card 热复位初始化
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SC_HotInit(void);

/*******************************************************************************
* Function Name  : SC_DeInit
* Description    : Deinitializes all ressources used by the Smartcard interface.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SC_DeInit(void);

/*******************************************************************************
* Function Name  : SC_Reset
* Description    : SIM Card 复位
* Input          : mode  复位方式：热复位或者冷复位
*
* Output         : None
* Return         : 返回复位状态  0：复位成功   -1复位失败
******************************************************************************/
char SC_Reset(UINT8 mode);

/*******************************************************************************
* Function Name  : SC_RSTSET
* Description    : Sets or clears the Smartcard reset pin.
* Input          : - ResetState: this parameter specifies the state of the Smartcard
*                    reset pin.
*                    BitVal must be one of the BitAction enum values:
*                       - Bit_RESET: to clear the port pin.
*                       - Bit_SET: to set the port pin.
* Output         : None
* Return         : None
*******************************************************************************/
void SC_RSTSET(UINT8 ResetState);

/*******************************************************************************
* Function Name  : SC_Init
* Description    : Initializes all peripheral used for Smartcard interface.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SC_RegInit(USI_TypeDef* USIx);


//iccemv.c
/*******************************************************************************
* Function Name  : USART_ByteReceive
* Description    : Receives a new data while the time out not elapsed.
* Input          : None
* Output         : None
* Return         : An ErrorStatus enumuration value:
*                         - SUCCESS: New data has been received
*                         - ERROR: time out was elapsed and no further data is
*                                  received
*******************************************************************************/
ErrorStatus USART_ByteReceive(UINT8 *Data, UINT32 TimeOut);

/*******************************************************************************
* Function Name  : USART_TsReceive
* Description    : Receives a new data while the time out not elapsed.
* Input          : None
* Output         : None
* Return         : An ErrorStatus enumuration value:
*                         - SUCCESS: New data has been received
*                         - ERROR: time out was elapsed and no further data is
*                                  received
*******************************************************************************/
ErrorStatus USART_TsReceive(UINT8 *Data, UINT32 TimeOut);

/*******************************************************************************
* Function Name  : SC_GetByte
* Description    : Get byte from IO
* Input          : Data pointer
* Output         : None
* Return         : None
*******************************************************************************/

ErrorStatus SC_GetByte(UINT8 *Data);

/*******************************************************************************
* Function Name  : SC_PutByte
* Description    : Put byte
* Input          : Data to send
* Output         : None
* Return         : None
*******************************************************************************/
UINT8 SC_PutByte(UINT8 Data);

/*******************************************************************************
* Function Name  : SC_Command
* Description    : Handles all Smartcard states and serves to send and receive all
*                  communication data between Smartcard and reader.
* Input          : - locadpu: pointer to an adpu buffer.
*                : adpulen : adpu'length
* Output         : - locresp: pointer to a Responce buffer.
*                : resplen : response's length which includes SW1&SW2
* Return         : SW1 & SW2
*******************************************************************************/
UINT16 SC_Command(UINT16 adpulen, UINT8 * adpubuf, UINT16 * resplen, UINT8 * respbuf);

/*******************************************************************************
* Function Name  : SC_Handler
* Description    : Handles all Smartcard states and serves to send and receive all
*                  communication data between Smartcard and reader.
* Input          : - SCState: pointer to an SC_State enumeration that will contain
*                    the Smartcard state.
*                  - SC_ADPU: pointer to an SC_ADPU_Commands structure that will be
*                    initialized.
*                  - SC_Response: pointer to a SC_ADPU_Responce structure which will
*                    be initialized.
* Output         : None
* Return         : None
*******************************************************************************/
void SC_Handler(SC_ADPU_Commands *SC_ADPU, SC_ADPU_Responce *SC_Response);
/*******************************************************************************
* Function Name  : Icc_Reset
* Description    :初始化SLOT通道的IC卡，包括对卡上电，复位，判断卡的协议类型：
*                 （T=1或T=0），并将ATR（复位应答）读入*ATR指针外。
* Input          : slot 需要初始化卡通道号
*                  ATR  ATR保持BUF
*
* Output         : None
* Return         : None
******************************************************************************/
int Icc_Reset(UINT8 slot,UINT8 *ATR);
/*******************************************************************************
* Function Name  : exp2_n
* Description    :计算2的N次方函数
* Input          :en : 幂数
*
* Output         : None
* Return         : 返回操作结果
******************************************************************************/
ushort exp2_n(UINT8 en);
/*******************************************************************************
* Function Name  : GetAtr
* Description    :读取IC卡的复位应答信号ATR
* Input          :Buff : ATR的存放BUFF
*
* Output         : None
* Return         : 返回操作结果：0x00：成功
******************************************************************************/
int GetAtr(UINT8 *Buff);
/*******************************************************************************
* Function Name  : ClearAtr
* Description    :清除ATR BUF
* Input          :Channel : 需要清理的ATRbuf的逻辑号
*
* Output         : None
* Return         : None
******************************************************************************/
void ClearAtr(UINT8 Channel);
/*******************************************************************************
* Function Name  : Icc_Command
* Description    :发送命令
*                 支持T0协议与T1协议
*                 不支持CASE2类型的命令发送
* Input          : slot     被操作端口
*                  ApduSend APDU命令
*                  ApduResp 应答数据
*
* Output         : None
* Return         : 返回操作结果
******************************************************************************/
int Icc_Command(UINT8 slot,SC_ADPU_Commands * ApduSend,SC_ADPU_Responce * ApduResp);

/*******************************************************************************
* Function Name  : IccSendCommand
* Description    :分别根据T=1或T=0协议将CLA+INS+P1+P2+LC（LE）+INDATA的命令数据包格式
*                 进行 转换，并按相应的格式与IC卡进行交换，结果数据与状态保存在OUTDATA中
*                 T=1时： OUTDATA： LEN（2）+DATA+SW（1，2）。
*                 T=0时： OUTDATA： LEN（2）+DATA。 SW保存到全局变量中
* Input          :cla : CLA
*                 ins : INS
*                 p1  : P1
*                 p2  : P2
*                 p3  : P3
*                 indata:data
*                 le   :Le
*                 outdata:结果BUF
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int IccSendCommand(UINT8 cla, 
									 UINT8 ins,
									 UINT8 p1,
									 UINT8 p2,
									 UINT16 p3,
									 UINT8 *indata,
									 UINT16 le,
									 UINT8 *outdata);

/*******************************************************************************
* Function Name  : T0_send_command
* Description    :T=0协议下发送命令头标CLA+INS+P1+P2+P3与数据包（DATABUF），并读取IC卡响应的数据（OUTBUF）与状态字节（RSLT）。
* Input          :Class : CLA
*                 Ins : INS
*                 p1  : P1
*                 p2  : P2
*                 p3  : P3
*                 databuf：从终端发出的数据包，（其长度为P3；）
*                 outbuf:从IC卡读出的数据内容，[0]为数据长度；
*                 relt:表CASE命令号（1、2、3、4）
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败       返回时存入SW1与SW2状态字节。
******************************************************************************/
int T0_send_command(UINT8 cla,
	                  UINT8 ins,
                    UINT8 p1,
                    UINT8 p2,
                    UINT8 p3,
                    UINT8 *databuf,
                    UINT8 *outbuf,
                    UINT8 *rslt);

/*******************************************************************************
* Function Name  : T1_send_command
* Description    :T=1协议下的字组数据发送与接收响应数据。
*                   注意: T1不支持扩展指令，如果要支持则需要修改
* Input          :NAD : 结点地址（初定为0X00）；
*                 INBUF : 发送的信息字段LEN+INF，INBUF[0]为长度；
*                 OUTBUF: 接收应答数据单元：格式NAD+LEN（2字节）+DATA+SW1+SW2
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int T1_send_command(UINT8 nad,UINT8 *inbuf,UINT8 *outbuf);
/*******************************************************************************
* Function Name  : T1_Send_Block
* Description    :T=1协议下发送一字组
* Input          :Inbuf : 待发送字组的buf
*
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int  T1_Send_Block(UINT8 *Inbuf);
/*******************************************************************************
* Function Name  : T1_Rece_Block
* Description    :T=1协议下接收一字组
* Input          :Outbuf : 接收数据缓存buf
*
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int  T1_Rece_Block(UINT8 *Outbuf);
/*******************************************************************************
* Function Name  : T1_IFSD_command
* Description    :在收到正确的复位应答后，对于T=1卡片须发送一个IFSD请求，以指示终端能收取最大字组长度。
* Input          :slot : 卡槽逻辑号
*
* Output         : None
* Return         : 命令执行结果：0x00：成功  其他：失败
******************************************************************************/
int  T1_IFSD_command(UINT8 slot);
/*******************************************************************************
* Function Name  : SC_PollCard_Init
* Description    : SIM card insertion init detection
* Input          : None
*
* Output         : None
* Return         : None
*******************************************************************************/
void SC_PollCard(void);

#endif /* ICCEMV_H_ */

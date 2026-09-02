#ifndef _APP_EVEN_H_
#define _APP_EVEN_H_

//#include "AppCfg.h"
//#include "AppRunLog.h"
//#include "AppRunTime.h"
#include "Libinclude.h"
//#include "AppStorage.h"
//#include "ota_Interface.h"


#define EVE_GUN_CNT				2


/*------------------------------------------------------------------------------
Section: 故障告警级别枚举
------------------------------------------------------------------------------*/
typedef enum
{
    eErr_LvN                     = 0x00,     // 无效值
    eErr_Lv1                     = 0x01,     // (严重故障) 产生后停机,根据物理量消除
    eErr_Lv2                     = 0x02,     // (一般故障) 产生后停机,结束充电后消除
    eErr_Lv3                     = 0x03,     // (严重告警) 充电后可继续充电，空闲时不可启动充电
    eErr_Lv4                     = 0x04,     // (一般告警) 产生后仍可继续充电且可再启动充电
    eErr_Lv5                     = 0x05,     // (正常停止) 产生后停机且可再启动充电
    eErr_LvNum,
}EVE_LEVEL_E;

/*------------------------------------------------------------------------------
Section: 设备状态枚举
------------------------------------------------------------------------------*/
typedef enum
{
    eDevState_Normal             = 0x00,     // 正常状态
    eDevState_Alarm              = 0x01,     // 告警状态
    eDevState_MajorAlarm         = 0x02,     // 严重告警状态-不可启动，继续充电
    eDevState_Fault              = 0x03,     // 故障状态-不可启动，停止充电
//    eDevState_MajorFault         = 0x04,     // 严重故障状态-不可启动，停止充电
}DEVICE_STATE_E;

/*------------------------------------------------------------------------------
Section: 故障属性
------------------------------------------------------------------------------*/
typedef enum
{
    eEveResult_Null     = 0x00,		// 维持当前状态
    eEveResult_Off      = 0x01,     // 满足故障恢复条件(非置位)
    eEveResult_On       = 0x02,     // 满足故障产生条件(非置位)
}EVE_CHECK_RESULT;

/*------------------------------------------------------------------------------
Section: 故障处理方式枚举
------------------------------------------------------------------------------*/
typedef enum
{
    eHandleType_N                = 0x00,     // 无效值 */
    eHandleType_A                = 0x01,     // 50ms内断K1K2
    eHandleType_B                = 0x02,     // 100ms内断K1K2
    eHandleType_C                = 0x03,     // 300ms内断K1K2
    eHandleType_D                = 0x04,     // 1s内断K1K2
    eHandleType_E                = 0x05,     // 5A下断 K1K2
    eHandleType_Num              = 0x06,
}ERR_HANDLE_E;

/*------------------------------------------------------------------------------
Section: 故障/停止原因分类枚举
------------------------------------------------------------------------------*/
typedef enum
{
    eErr_none,								 // 0

    eErr_CpVoltAbnor = 1,                        // CP电压异常    
    eErr_CpGroundFault,                      // CP对地短路
    eErr_PEBreakFault,                       // PE接地故障
    eErr_GunDisConnErr,                      // 充电中控制导引故障 
    eErr_EmergencyStop,                      // 急停故障       
    eErr_InputLineReversed,                  // 火零反接
    eErr_LeakageCurrErr,                     // 漏电流故障
    eErr_DiodeStop,                          // 不存在二极管停止   
    eErr_ShortCircleErro,                    // 短路故障

    eErr_AphaseInputOverVol = 10,            // 交流A相输入过压
    eErr_BphaseInputOverVol,                 // 交流B相输入过压
    eErr_CphaseInputOverVol,                 // 交流C相输入过压
    eErr_AphaseInputLessVol,                 // 交流A相输入欠压
    eErr_BphaseInputLessVol,                 // 交流B相输入欠压
    eErr_CphaseInputLessVol,                 // 交流C相输入欠压
    eErr_OutputOverCurr,                     // 交流输出过流
    eErr_JcqMaloperation,                    // 交流输出接触器误动拒动
    eErr_JcqSynechiaFault,                   // 交流输出接触器粘连
    eErr_HmiCommErr,                         // 人机交互通信故障

    eErr_ReaderCommErr = 20,                 // 读卡器通信故障
    eErr_8209CommErr,                        // RN8209通信故障
    eErr_MeterCommErr,                       // 电表通信故障
    eErr_EnvOverTempErr,                     // 环境过温故障
    eErr_GunOverTempErr,                     // 枪过温故障
    eErr_POverTempErr,                       // 插头过温 
    eErr_DatabaseErr,                        // 数据库存储错误
    eErr_MeterCalcErr,                       // 电能表计量故障
    eErr_PowerApplyFail,                     // 功率申请失败告警
    eErr_ChgStartTimeout,                    // 启动超时

    eErr_EnvOverTempWarn = 30,                // 环境过温警告
    eErr_GunOverTempWarn,                     // 枪过温警告
    eErr_PlugOverTempWarn,                    // 插头过温警告
    eErr_LittleCurr,                         // 小电流
    eSrc_ChargeFullStop,                     // 充满停止
    eSrc_AutoStop,                           // 自动停止（车辆） 
    eSrc_ManualCtrlStop,                     // 主动控制停止
    eSrc_SetKeyStop,                         // 按键停止

    //下面为网络单元扩展故障
    eErr_CCUSCUCommErr = 38,                  //与充电单元通讯故障
    eErr_PhaseLossErr,                       //缺相故障 
    eErr_InputOverVol,                       // 交流输入过压   
    eErr_InputLessVol,                       // 交流输入欠压  

    eErr_NetNoSIMErr,                        // 网络连接异常，sim卡未插入
    eErr_NetSIMErr,                          // 网络连接异常，离线-SIM卡异常 
    eErr_NetIPErr,                           // 网络连接正常，ip连接异常 
    eErr_PlatformOffline,                    // 平台未正常上线，平台通信异常
    eErr_feemodelErr,                        // 费率模型异常
    
    eErr_Num,                                // 故障数(含告警) 
}E_ErrCode;

#ifdef __cplusplus
extern "C" {
#endif
#define EVE_QUEUE_CNT			8
typedef struct
{
    E_ErrCode          eErr;           	// 故障类型
	uint8_t				state;
}REPORT_EVE_QUEUE;
//====================================================================
//故障管理
void fgv_AppEventCycleTask(void);

uint8_t dev_getErrExsit(uint8_t u8Port, E_ErrCode eErr);

uint8_t dev_setErrExsit(uint8_t u8Port, E_ErrCode eErr, uint16_t line);
uint8_t dev_setErrExsit_all(E_ErrCode eErr, uint16_t line);

uint8_t dev_clrErrExsit(uint8_t u8Port, E_ErrCode eErr, uint16_t line);
uint8_t dev_clrErrExsit_all(E_ErrCode eErr, uint16_t line);

//=====================================================================
//充电管理
//用于判断是否有故障，是否可以启动
uint8_t dev_getDevHardfalt(uint8_t u8Port);
//用于判断是否需要停止，正常停止无故障
uint8_t EVE_isDevStop(uint8_t u8Port);

DEVICE_STATE_E dev_getDevState(uint8_t u8Port);


//最高等级的停止方式
ERR_HANDLE_E dev_getHighlistErrHandleType(uint8_t u8Port);
//第一个导致停机的事件，停止原因
E_ErrCode dev_getFirstStopErr(uint8_t u8Port);


char *dev_getErrString(E_ErrCode eErr);
uint8_t EVE_PopQueue(uint8_t u8Port, REPORT_EVE_QUEUE *pEveInfo);
uint8_t EVE_IsQueueEmpty(uint8_t u8Port);
//=====================================================================

#ifdef __cplusplus
}
#endif

#endif



#ifndef	__COMM_INTERFACE_H__
#define	__COMM_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "Libinclude.h"
#include "Communication.h"
#include "Gprslib.h"
#include "SIM900A.h"

enum {
    eNet_Connecting,    //连接中
    eNet_SimErro,       //SIM异常
    eNet_IPErro,        //IP异常
    eNet_CommErro,      //通信异常
};

U8 Comm_getNetStep(eNetSocket SocketID);

void Comm_JSHttpsParamStart(void);

U8 Comm_getNetAbnormalSta(eNetSocket SocketID);

U8 Comm_getIpSuces(eNetSocket SocketID);

void Comm_PlatReconnect(eNetSocket SocketID, uint16_t line);
void AT_ImtlyReconnct(eNetSocket SocketID, uint16_t line);
void Comm_HttpsStart(void);
void Comm_BleReset(void);

//
void Comm_BleQueryList(void);
void Comm_BleQueryPin(void);
void Comm_BleSetPin(void);

void Comm_Init(void);

#ifdef __cplusplus
}
#endif

#endif

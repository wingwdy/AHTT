#include "mbsDataUpdate.h"
#include "AppStorage.h"
#include "CommInterface.h"
#include "AppMidDataTrans.h"

GN_PLATMOD  sg_platmod;


static GN_PLATMOD  *lpmUpd = &sg_platmod;	//用于本文件中

//获取枪数量
uint8_t fgv_GetPileCfgGunNum()
{
    return lpmUpd->pileCfgInfo.gunNum;
}
//获取是否即插即充状态
uint8_t fgv_GetPileCfgOffLinChrg()
{
    return lpmUpd->pileCfgInfo.fct_cfg.bit.cgfPlugChrg;
}
//获取国企标状态
uint8_t fgv_GetPileCfgNationalStandard()
{
    return lpmUpd->pileCfgInfo.fct_cfg.bit.cgfModeCmp;
}

/**
 * @brief 判断IPv4地址是否合法
 * @param ip: 待校验的ip地址
 * @return true: 校验成功	false：校验失败（IP地址不合法）
 * @author PJW
 */
uint8_t IPv4_verify(char *ip, uint8_t *ipOut) {
    int a,b,c,d;
    char t;
	if (4 == sscanf(ip,"%d.%d.%d.%d%c",&a,&b,&c,&d,&t)){
        if (0<=a && a<=255
            && 0<=b && b<=255
            && 0<=c && c<=255
            && 0<=d && d<=255)
            {
                ipOut[0] = a;
                ipOut[1] = b;
                ipOut[2] = c;
                ipOut[3] = d;
                return 1;
            }
    }
    return 0;
}

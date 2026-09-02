#ifndef __VERSION_H__
#define __VERSION_H__


///////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//产品平台特征字，用于升级区分 (请勿删除)
#define PLAT_NAME                   "P65N"

#define RTU_NAME                    "appC"
#define appA                        0
#define PROGRAM_NAME                appC


#define P65N                        0
#define RTU_TYPE                    P65N
#define HARDWARE_VER                0x100            //定义硬件版本号
#define SOFTWARE_VER                0x103            //定义软件版本号
#define SOFTWARE_STRVERSION  		"1.0.1_0003"     //软件程序版本号

#define SOFTWARE_VER1               0x31            //程序修改日期：日
#define SOFTWARE_VER2               0x08            //程序修改日期：月
#define SOFTWARE_VER3               0x23            //程序修改日期：年


//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#define TEST_FLAG                   FALSE


#define DEVICETYPE					0				//0-单相交流充电桩 1-三相交流充电桩
#define DEVICEMODEL					"D3-D32B"		//设备型号
#define RATETOTALPOWER				7				//额定总功率


#endif  //__VERSION_H__


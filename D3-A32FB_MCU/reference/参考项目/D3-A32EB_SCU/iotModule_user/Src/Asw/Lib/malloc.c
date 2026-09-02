///////////////////////////////////////////////////////////////////////////
//Copyright (C), 2010-2050, 公牛集团
//文件名称: malloc.c
//文件功能: 内存管理相关定义、结构与函数
//文件作者: 网络
//文件版本: V1.0
//创建日期: 2021-03-15
//函数列表:
//修改记录:
//////////////////////////////////////////////////////////////////////////
//头文件

#include "malloc.h"
#include "FreeRTOS.h"

//释放内存(外部调用)
//ptr:内存首地址
void myfree(void *ptr)
{
//	U32 offset;
//    if(ptr==NULL)return;//地址为0.
// 		offset=(U32)ptr-(U32)mallco_dev.membase;
//    mem_free(offset);	//释放内存

	vPortFree(ptr);
	
}
//分配内存(外部调用)
//size:内存大小(字节)
//返回值:分配到的内存首地址.
void *mymalloc(size_t sz)
{
//    U32 offset;
//	offset=mem_malloc(sz);
//    if(offset==0XFFFFFFFF)return NULL;
//    else return (void*)((U32)mallco_dev.membase+offset);

	return pvPortMalloc(sz);
}
//重新分配内存(外部调用)
//*ptr:旧内存首地址
//size:要分配的内存大小(字节)
//返回值:新分配到的内存首地址.
//void *myrealloc(void *ptr,U32 size)
//{
//    U32 offset;
//    offset=mem_malloc(size);
//    if(offset==0XFFFFFFFF)return NULL;
//    else
//    {
//	    mymemcpy((void*)((U32)mallco_dev.membase+offset),ptr,size);	//拷贝旧内存内容到新内存
//        myfree(ptr);  											  	//释放旧内存
//        return (void*)((U32)mallco_dev.membase+offset);  			//返回新内存首地址
//    }
	
//	return NULL;

//}

size_t myFreeHeapSize(void)
{
	return xPortGetFreeHeapSize();
}

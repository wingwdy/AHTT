
#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include "cmsis_os2.h"
#include "freeRTOS.h"


#define AT_RSP_GSM_MSG_MAX  256
#define AT_RSP_GSM_MSG_CUN  5


struct Node_s
{
	uint8_t  srvID;
    uint16_t uLen;                     ///<<Property>长度
    uint8_t  aucData[AT_RSP_GSM_MSG_MAX];  ///<<Property>数据区
	Node_s   *next;
};

class CLinkedList
{
	private:
		Node_s *sHead;
		uint16_t Lenth;
		osMemoryPoolId_t pool_Id;
	public:
		CLinkedList(void);
		uint8_t addNode(uint8_t ID, uint16_t Len, uint8_t *pucData);
		uint8_t readNode(uint8_t ID, Node_s *sNode);//根据id号取节点

};
	
#endif

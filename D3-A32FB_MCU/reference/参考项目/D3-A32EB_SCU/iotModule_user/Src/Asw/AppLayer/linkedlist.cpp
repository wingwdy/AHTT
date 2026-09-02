
#include "linkedlist.h"
#include <string.h>
#include "stdio.h"
#include "cmsis_os2.h"

CLinkedList::CLinkedList(void)
{
	this->sHead = 0;
	this->Lenth = 0;

	//初始化内存空间
	this->pool_Id = osMemoryPoolNew(AT_RSP_GSM_MSG_CUN, 8 + AT_RSP_GSM_MSG_MAX, NULL);
	if (this->pool_Id == 0)//<<Plan B
	{
		;
	}
}

//增加节点，存满时取干掉最早一个
//返回错误值：0，无错误 //1，缓存满
uint8_t CLinkedList::addNode(uint8_t ID, uint16_t Len, uint8_t *pucData)
{
	//p1.建立节点
	Node_s *target = 0;
	Node_s *temp;
    
    if(ID > 9)
        return ID;
	
	if (this->Lenth < AT_RSP_GSM_MSG_CUN)
	{
		target = (Node_s *)osMemoryPoolAlloc (this->pool_Id, 0);//从内存池中取一个空间
	}
	else
	{
		//PlanB
		target = this->sHead;
		this->sHead = this->sHead->next;
		this->Lenth--;
	}

    target->srvID = ID;
    target->uLen = Len;
    memcpy(target->aucData, pucData, Len);
    target->next = 0;
	
	//p2.判断节点加入位置
	if (this->sHead == 0)
	{
		this->sHead = target;
	}
	else
	{
		temp = this->sHead;
		while (temp->next != 0)
		{
			temp = temp->next;
		}
        //p3.加入节点并修改相关信息
        temp->next = target;
	}

	this->Lenth++;
	
	return 0;
	
}

//根据id号取节点,返回1失败，0成功
uint8_t CLinkedList::readNode(uint8_t ID, Node_s *sNode)
{
	Node_s *temp;
	Node_s *target;
	osStatus_t status;
	if (this->sHead == 0)
	{	return 1;}
	
	temp = this->sHead;
	target = temp->next;
	if (temp->srvID != ID)
	{
		while (target != 0)
		{
			if (target->srvID == ID)
			{
				temp->next = target->next;
				memcpy(sNode, target, sizeof(*sNode));
				status = osMemoryPoolFree (this->pool_Id, target);
				this->Lenth--;
				return 0;
			}
			else
			{
				//target 的ID不是要的ID
				target = target->next;
				temp = temp->next;
			}
		}
	}
	else//头是要找的目标
	{
		this->sHead = temp->next;
		memcpy(sNode, temp, sizeof(*sNode));
		status = osMemoryPoolFree (this->pool_Id, target);
		this->Lenth--;
		return 0;
	}
	
	return 1;
	
}



#ifndef __IDWORLD_H
#define __IDWORLD_H

extern unsigned char RxCon;
extern unsigned char RxBuf[];
extern unsigned int  AddNewFlag;	    //添加新指纹标志
extern unsigned int  IdentifyFlag;	//识别标志
//extern volatile unsigned char FPMXX_RECEICE_BUFFER[];

void SEND_CMD_GET_IMAGE(void);
void SEND_CMD_GENERATE0(void);
void SEND_CMD_GENERATE1(void);
void SEND_CMD_GENERATE2(void);
void SEND_CMD_GET_EMPTY_ID(void);
void SEND_CMD_MERGE(void);
void SEND_CMD_DEL_CHAR(void);
void SEND_CMD_SEARCH(void);

void SEND_CMD_STORE_CHAR(unsigned char storeID); 
void SEND_CMD_SLED_CTRL(unsigned char sled);


void RxBuf2Clr(void);	  //接收缓存清0


#endif


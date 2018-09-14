#ifndef __ZIGBEE_H
#define __ZIGBEE_H

extern unsigned char ZigRxCon;
extern unsigned char ZigRxBuf[];
extern unsigned char ZigTxBuf[];
void ZigRxBufClr(void);
void Zig_ReadInfo(void);
void Zig_BasicPost(unsigned char len,unsigned char event,unsigned char mark);
void Zig_Set(void);

#endif

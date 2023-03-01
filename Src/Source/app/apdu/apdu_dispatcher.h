/*
 * apdu_dispatcher.h
 *
 *  Created on: 2018年8月13日
 *      Author: YangWenfeng
 */

#ifndef APDU_DISPATCHER_H_
#define APDU_DISPATCHER_H_

#define INS_READ_VERSION			0x5C//获取软件和硬件版本号
#define INS_TEST_SLEEP              0x5D //进入sleep
#define INS_MEM_READ				0xA0
#define INS_READ_SN					0xE1
#define INS_GET_RESPONSE			0xC0


extern void usi_main(void);
#endif /* APDU_DISPATCHER_H_ */

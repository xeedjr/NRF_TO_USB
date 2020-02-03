/*
 * RFBL.h
 *
 *  Created on: 30 ��. 2020 �.
 *      Author: Bogdan
 */

#ifndef RFBL_H_
#define RFBL_H_

#include "cmsis_os.h"
#include "RF24.h"
#include "RF24HALChibios.h"
#include "MailBox.h"

class RFBL {
	RF24* radio;

	void thread(void);
	static void RFBL_thread(const void* thisp) {
		RFBL* p = (RFBL*)thisp;
		p->thread();
	};
	osThreadDef (RFBL_thread, osPriorityNormal, 1024, 0);              // define Thread and specify to allow three instances

	void thread_serial(void);
	static void RFBL_thread_serial(const void* thisp) {
		RFBL* p = (RFBL*)thisp;
		p->thread_serial();
	};
	osThreadDef (RFBL_thread_serial, osPriorityNormal, 1024, 0);              // define Thread and specify to allow three instances

	typedef struct {
		enum msgid {
			kA,
		};
		union data {
			struct a{

			};
		};
	} Message;
	MailBox<Message, 10> mb;

	// Radio pipe addresses for the 2 nodes to communicate.
	const uint64_t pipes[4] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL, 0xF0F0F0F0D3LL, 0xF0F0F0F0D4LL};

	uint8_t read_buffer[256];

	uint8_t Tostr(uint8_t* data, int len, char* str);
public:
	RFBL(RF24* radio);
	virtual ~RFBL();

};

#endif /* RFBL_H_ */

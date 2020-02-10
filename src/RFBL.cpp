/*
 * RFBL.cpp
 *
 *  Created on: 30 ñ³÷. 2020 ð.
 *      Author: Bogdan
 */

#include <string>
#include <string.h>
#include "ch.h"
#include "hal.h"
#include "cmsis_os.h"
#include "usbcfg.h"
#include "RFBL.h"

RFBL::RFBL(RF24* radio) {
	this->radio = radio;
	// TODO Auto-generated constructor stub
	this->radio->begin();                           // Setup and configure rf radio
	this->radio->setChannel(2);
	this->radio->setPALevel(RF24_PA_MAX);
	this->radio->setDataRate(RF24_1MBPS);
	this->radio->setAutoAck(1);                     // Ensure autoACK is enabled
	this->radio->setRetries(2,15);                  // Optionally, increase the delay between retries & # of retries
	this->radio->setCRCLength(RF24_CRC_8);          // Use 8-bit CRC for performance
	this->radio->enableDynamicPayloads();
	auto rate = this->radio->getCRCLength();
	if (rate != RF24_CRC_8) {
		while (1) {}
	}
	this->radio->openWritingPipe(pipes[0]);
	this->radio->openReadingPipe(0, pipes[0]);
	this->radio->openReadingPipe(1, pipes[1]);
	this->radio->openReadingPipe(2, pipes[2]);
	this->radio->openReadingPipe(3, pipes[3]);
	this->radio->startListening();

	mutexRf_id = osMutexCreate  (osMutex (MutexRF));
	if (mutexRf_id == NULL)  {
		while (1) {};
	}

	thread_id = osThreadCreate (osThread (RFBL_thread), this);
	auto id2 = osThreadCreate (osThread (RFBL_thread_serial), this);

	auto timer_id = osTimerCreate (osTimer(Timer1), osTimerPeriodic, this);
	if (timer_id == NULL)  {
		while (1) {};
	}
	osTimerStart(timer_id, 10);
}

RFBL::~RFBL() {
	// TODO Auto-generated destructor stub
}


#define StartBlock()	(code_ptr = dst++, code = 1)
#define FinishBlock()	(*code_ptr = code)

size_t RFBL::StuffData(const uint8_t *ptr, size_t length, uint8_t *dst)
{
	const uint8_t *start = dst, *end = ptr + length;
	uint8_t code, *code_ptr; /* Where to insert the leading count */

	StartBlock();
	while (ptr < end) {
		if (code != 0xFF) {
			uint8_t c = *ptr++;
			if (c != 0) {
				*dst++ = c;
				code++;
				continue;
			}
		}
		FinishBlock();
		StartBlock();
	}
	FinishBlock();
	return dst - start;
}

/*
 * UnStuffData decodes "length" bytes of data at
 * the location pointed to by "ptr", writing the
 * output to the location pointed to by "dst".
 *
 * Returns the length of the decoded data
 * (which is guaranteed to be <= length).
 */
size_t RFBL::UnStuffData(const uint8_t *ptr, size_t length, uint8_t *dst)
{
	const uint8_t *start = dst, *end = ptr + length;
	uint8_t code = 0xFF, copy = 0;

	for (; ptr < end; copy--) {
		if (copy != 0) {
			*dst++ = *ptr++;
		} else {
			if (code != 0xFF)
				*dst++ = 0;
			copy = code = *ptr++;
			if (code == 0)
				break; /* Source length too long */
		}
	}
	return dst - start;
}

void RFBL::send_frame(uint8_t* data, uint8_t len) {
	buffer_index = StuffData(data, len, &buffer[1]);
	buffer[0] = 0;
	buffer_index++;
	buffer[buffer_index++] = 0;
	chnWrite(&SDU1, (const uint8_t*)buffer, buffer_index);
}

void RFBL::process_frame(void) {
	/// parse array
	switch(read_buffer[0]) {
	case 1: // test
		{
			AnswMessage answ = {0};
			answ.s.cmdid = read_buffer[0];
			answ.s.type = 1;
			send_frame((uint8_t*)&answ.s, sizeof(answ.s));
		}
		break;
	case 2:  /// Write to node
		{
			osStatus status;
			uint8_t pipe_id = read_buffer[1];
			uint8_t data_len =  read_buffer_index - 2;
			uint8_t* data_p = &read_buffer[2];

			status  = osMutexWait(mutexRf_id, osWaitForever);
			if (status != osOK)  {
			  // handle failure code
			}
			// First, stop listening so we can talk
			radio->stopListening();

			// Open the correct pipe for writing
			radio->openWritingPipe(pipes[pipe_id]);

			// Send the final one back.
			bool res = radio->write( data_p, data_len );

			// Now, resume listening so we catch the next packets.
			radio->startListening();
		    status = osMutexRelease(mutexRf_id);
		    if (status != osOK)  {
		      // handle failure code
		    }

			/// form nswer
			AnswMessage answ = {0};
			answ.s.cmdid = read_buffer[0];
			answ.s.type = 1;
			answ.s.result = res;
			send_frame((uint8_t*)&answ.s, sizeof(answ.s));
		}
		break;
	case 3:
		break;
	}
}

void RFBL::thread_serial(void) {

	while(1) {
		uint8_t ch = 0;
		int len = 0;

		/// wait 0xOA
		len = chnRead(&SDU1, (uint8_t*)&ch, 1);
		if (len != 1) {
			osThreadYield();
			buffer_index = 0;
			continue;
		}
		if (ch == 0x00) {
			/// scip start frame chracter
			if (buffer_index > 0) {
				// we have data process it
				read_buffer_index = UnStuffData(buffer, buffer_index, read_buffer);
				if (read_buffer_index != 0) {
					process_frame();
				} else {
					//// TODO some errror of decoding previous data len
				};
			};
			buffer_index = 0;
			continue;
		} else {
			buffer[buffer_index++] = ch;
		}
	}
}


void RFBL::thread(void) {
	while(1) {
		osStatus status;
		osEvent ev;

		ev = osSignalWait(0, osWaitForever);
		if ((ev.value.signals & kTimer1) > 0) {
			uint8_t pipe_num = 0;

			status  = osMutexWait(mutexRf_id, osWaitForever);
			if (status != osOK)  {
			  // handle failure code
			}
			if ( radio->available(&pipe_num) ) {
				uint8_t data [25] = {0};
				// Fetch the payload, and see if this was the last one.
				bool done = false;
				while (!done)
				{
					uint8_t len = radio->getDynamicPayloadSize();
					done = radio->read( data, len );

					EventMessage evmsg;
					evmsg.s.type = 2;
					evmsg.s.pipe_num = pipe_num;
					memcpy(evmsg.s.data, data, len);
					send_frame((uint8_t*)&evmsg.s, (len + sizeof(evmsg.s.type) + sizeof(evmsg.s.pipe_num)));
				}

				osSignalSet(thread_id, kTimer1);
			}
			status = osMutexRelease(mutexRf_id);
			if (status != osOK)  {
			  // handle failure code
			}
		};
	}
}


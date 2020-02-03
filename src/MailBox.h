/*
 * MailBox.h
 *
 *  Created on: 20 марта 2016 г.
 *      Author: Bogdan
 */

#ifndef MAILBOX_H_
#define MAILBOX_H_

#include <array>

#include "cmsis_os.h"

template <typename T, int N>
class MailBox
{
private:
	osMessageQId		mail_box__ID;
	const msg_t os_messageQ_buf_mail_box__ID[N] = {0};                        \
	mailbox_t os_messageQ_obj_mail_box__ID;                                    \
	const osMessageQDef_t os_messageQ_def_mail_box__ID = {                            \
	  (N),                                                               \
	  sizeof (unsigned char),                                                             \
	  (mailbox_t *)&os_messageQ_obj_mail_box__ID,                                          \
	  (void *)&os_messageQ_buf_mail_box__ID[0]                                        \
	};

	osMessageQId		free_mail_box__ID;
	const msg_t os_messageQ_buf_free_mail_box__ID[N] = {0};                        \
	mailbox_t os_messageQ_obj_free_mail_box__ID;                                    \
	const osMessageQDef_t os_messageQ_def_free_mail_box__ID = {                            \
	  (N),                                                               \
	  sizeof (unsigned char),                                                             \
	  (mailbox_t *)&os_messageQ_obj_free_mail_box__ID,                                          \
	  (void *)&os_messageQ_buf_free_mail_box__ID[0]                                        \
	};
	std::array<T, N> buffers;

 public:
	MailBox() {
		mail_box__ID = osMessageCreate(osMessageQ(mail_box__ID), NULL);
	    if ( 0 == free_mail_box__ID )
	    {
	    	/// error

	    }
	    free_mail_box__ID = osMessageCreate(osMessageQ(free_mail_box__ID), NULL);
	    if ( 0 == free_mail_box__ID )
	    {
	    	/// error

	    }
		for (int i = 0; i < N; i++) {
			//free_mail_box_.post(i, TIME_INFINITE);
			osMessagePut(free_mail_box__ID, i, osWaitForever);
		}
	}

	void pop(T& item)
	{
		osEvent  peEvent_;
		peEvent_ = osMessageGet(mail_box__ID, osWaitForever);
	    if( osEventMessage != peEvent_.status)
	    {
	    	/// error
	    	return;
	    }
		item = buffers.at(peEvent_.value.v);

		osMessagePut(free_mail_box__ID, peEvent_.value.v, osWaitForever);
	}

	void push(T& item)
	{
		osEvent  peEvent_;
		peEvent_ = osMessageGet(free_mail_box__ID, osWaitForever);
	    if( osEventMessage != peEvent_.status)
	    {
	    	/// error
	    	return;
	    }
		buffers.at(peEvent_.value.v) = item;
		osMessagePut(mail_box__ID, peEvent_.value.v, osWaitForever);
	}

	void pushI(T& item)
	{
		osEvent  peEvent_;
		peEvent_ = osMessageGet(free_mail_box__ID, 0);
	    if( osEventMessage != peEvent_.status)
	    {
	    	/// error
	    	return;
	    }
		peEvent_.value.v;
		buffers.at(peEvent_.value.v) = item;
		osMessagePut(mail_box__ID, peEvent_.value.v, 0);
	}
};

#endif /* MAILBOX_H_ */
